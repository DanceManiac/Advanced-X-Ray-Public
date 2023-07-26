#ifndef	LMODEL_H
#define LMODEL_H

#include "common.h"
#include "common_brdf.h"
#include "pbr_brdf.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Lighting formulas

#ifdef ES_PSEUDO_PBR
float4 compute_lighting(float3 N, float3 V, float3 L, float4 alb_gloss, float mat_id)
{
	float3 albedo = calc_albedo(alb_gloss, mat_id);
	float3 specular = calc_specular(alb_gloss, mat_id);
	float rough = calc_rough(alb_gloss, mat_id);
	//calc_rain(albedo, specular, rough, alb_gloss, mat_id, 1);
	calc_foliage(albedo, specular, rough, alb_gloss, mat_id);
	
	float3 light = Lit_BRDF(rough, albedo, specular, V, N, L );

	//if(mat_id == MAT_FLORA) //Be aware of precision loss/errors
	if(abs(mat_id-MAT_FLORA) <= MAT_FLORA_ELIPSON) //Be aware of precision loss/errors
	{
		//Simple subsurface scattering
		float subsurface = SSS(N,V,L);
		light.rgb += subsurface*albedo;
	}	

	return float4(light, 0);
}

float4 plight_infinity(float m, float3 pnt, float3 normal, float4 c_tex, float3 light_direction )
{
	//gsc vanilla stuff
	float3 N = normalize(normal);							// normal 
	float3 V = normalize(-pnt);					// vector2eye
	float3 L = normalize(-light_direction);						// vector2light

	float4 light = compute_lighting(N,V,L,c_tex,m);
	
	return light; // output (albedo.gloss)
}

float4 plight_local(float m, float3 pnt, float3 normal, float4 c_tex, float3 light_position, float light_range_rsq, out float rsqr )
{
	float atteps = 0.1;
	
	float3 L2P = pnt - light_position;                       		// light2point 
	rsqr = dot(L2P,L2P); // distance 2 light (squared)
	rsqr = max(rsqr, atteps);
	//rsqr = rsqr + 1.0;
	
	//vanilla atten - linear
	float att = saturate(1.0 - rsqr*light_range_rsq); // q-linear attenuate
	att = SRGBToLinear(att);
	/*
	//unity atten - quadtratic
	//catlikecoding.com/unity/tutorials/custom-srp/point-and-spot-lights/
	att = rsqr * light_range_rsq;
	att *= att;
	att = saturate(1.0 - att);
	att *= att;
	att = att / rsqr;
	*/
	float3 N = normalize(normal);							// normal 
	float3 V = normalize(-pnt);					// vector2eye
	float3 L = normalize(-L2P);					// vector2light

	float4 light = compute_lighting(N,V,L,c_tex,m);
	
	return att*light;		// output (albedo.gloss)
}

float3 specular_phong(float3 pnt, float3 normal, float3 light_direction)
{
	float3 H = normalize(pnt + light_direction );
	float nDotL = saturate(dot(normal, light_direction));
	float nDotH = saturate(dot(normal, H));
	float nDotV = saturate(dot(normal, pnt));
	float lDotH = saturate(dot(light_direction, H));
	//float vDotH = saturate(dot(pnt, H));
	return L_sun_color.rgb * Lit_Specular(nDotL, nDotH, nDotV, lDotH, 0.02, 0.1);
}

//	TODO: DX10: Remove path without blending
half4 blendp(half4 value, float4 tcp)
{
	return 	value;
}

half4 blend(half4 value, float2 tc)
{
	return 	value;
}

#else // NO ES_PSEUDO_PBR

float4 compute_lighting(float3 N, float3 V, float3 L, float4 alb_gloss, float mat_id)
{
	float3 albedo = alb_gloss;
	float spec = alb_gloss.www;
	float metalness = ceil(mat_id-0.75);

	spec *= Ldynamic_color.w;
	
	//gamma
	albedo = SRGBToLinear(albedo);
	spec = SRGBToLinear(spec);

	//spec *= Ldynamic_color.w;

	
	//float vector
  	float3 H = normalize(L+V);
  	float NdotL = saturate(dot(N,L));
  	float NdotH = saturate(dot(N,H));
  	float HdotL = saturate(dot(H,L));
  	float Pi = 3.14159265359;
	
	//material
	float4 light = s_material.Sample(smp_material, float3( NdotL, NdotH, mat_id)).xxxy;
	//light.w /= max(NdotL, 0.1);

	//gamma
	//light.rgb = SRGBToLinear(light.rgb);
	//light.w = SRGBToLinear(light.w);
	
	//non-LUT shading
	//vaguely energy conserving
	float gloss = saturate(mat_id/0.75);
	gloss -= 0.5*metalness;
	
	//diffuse
	float exponent = ((gloss*0.5)*0.8)+0.6; //source
	light.rgb = pow(NdotL, exponent) * ((exponent + 1.0) * 0.5); //source lambert

	//specular
	float glossiness = exp2(pow(gloss,1.5)*14); //source
	light.w = pow(NdotH, glossiness) * (glossiness+2/8) * saturate(4*NdotL);
	
	//fresnel
	float f0 = lerp(0.04,0.75,metalness);
	light.w *= lerp(f0, 1, pow(1-HdotL, 5)); //non-PBR fresnel

	//grass SSS
	if(abs(mat_id-MAT_FLORA) <= MAT_FLORA_ELIPSON) //Be aware of precision loss/errors
	{
		//Simple subsurface scattering
		float subsurface = SSS(N,V,L);
		light.rgb += subsurface;
	}	

	//textures
	light.rgb = light.rgb*albedo;
	light.w = light.w*spec;
	
	
	float3 metaltint = metalness ? (albedo / max(dot(LUMINANCE_VECTOR, albedo), 0.01)) : 1.0; //spec mask tinted by diffuse
	
	//add spec
	light.rgb += light.www * metaltint;
	
	return float4(light.rgb,0);
}

float4 plight_infinity(float m, float3 pnt, float3 normal, float4 c_tex, float3 light_direction )
{
	//gsc vanilla stuff
	float3 N = normalize(normal);							// normal 
	float3 V = -normalize(pnt);					// vector2eye
	float3 L = -normalize(light_direction);						// vector2light

	float4 light = compute_lighting(N,V,L,c_tex,m);
	
	return light; // output (albedo.gloss)
}

float4 plight_local(float m, float3 pnt, float3 normal, float4 c_tex, float3 light_position, float light_range_rsq, out float rsqr )
{
	float3 L2P = pnt - light_position;                       		// light2point 
	rsqr = dot(L2P,L2P); // distance 2 light (squared)
	
	//vanilla atten
	float att = saturate(1 - rsqr*light_range_rsq); // q-linear attenuate
	att = SRGBToLinear(att);
	
	float3 N = normalize(normal);							// normal 
	float3 V = normalize(-pnt);					// vector2eye
	float3 L = normalize(-L2P);					// vector2light
	float3 H = normalize(L+V);						// float-angle-vector

	float4 light = compute_lighting(N,V,L,c_tex,m);
	
	return att*light;		// output (albedo.gloss)
}

float3 specular_phong(float3 pnt, float3 normal, float3 light_direction)
{
	float3 H = normalize(pnt + light_direction );
	float nDotL = max(0, dot(normal, light_direction));
	float nDotH = max(0, dot(normal, H));
	float nDotV = max(0, dot(normal, pnt));
	float lDotH = max(0, dot(light_direction, H));
	return L_sun_color.rgb * pow(lDotH,128) * ((128+8)/8) * nDotL;
}

//	TODO: DX10: Remove path without blending
half4 blendp(half4 value, float4 tcp)
{
	return 	value;
}

half4 blend(half4 value, float2 tc)
{
	return 	value;
}

#endif // ES_PSEUDO_PBR

#endif