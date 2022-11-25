#ifndef	LMODEL_H
#define LMODEL_H

#include "common.h"

uniform float4 	pbr_settings; //x - roughness, y - intensity, zw - null

//////////////////////////////////////////////////////////////////////////////////////////
// Lighting formulas // 
//////////////////////////////////////////////////////////////////////////////////////////

float2 pseudopbr(float3 N, float V, float3 L, float3 H, float gloss)
{
	//Dot products
	float NL = saturate(dot(N, L));
	float LH = saturate(dot(L, H));
	float NH = saturate(dot(N, H));

	//Roughness
	float roughness = pbr_settings.x - saturate(gloss); //Glossiness->Roughness
	roughness *= roughness; //Remapping (Burley)
	roughness += 0.1; //Remapping (Just for fucks sake)
	roughness = clamp(roughness, 0.005, 1.0); //Better safe than sorry

	//Alpha
	float alpha = roughness * roughness;

	//GGX Normal distribution function
	float D_denominator = NH * NH * (alpha - 1.0) + 1.0;
	float Distribution = alpha / (D_denominator * D_denominator);

	//Somewhat cool vis
	float Visibility = rcp(LH + (1.0 / roughness));

	//Specular sum
	float specular = Distribution * Visibility * 0.3183 * pbr_settings.y;

	//Out: [NL] [D*V/PI]
	return float2(NL, specular);
}

float4 compute_lighting(float3 N, float3 V, float3 L, float4 alb_gloss, float mat_id, bool is_point_light)
{
	//Half vector
	float3 H = normalize(L+V);
	
	//Vanilla
	float4 light = s_material.Sample(smp_material, float3( dot(L,N), dot (H,N), mat_id)).xxxy;
	
	//Vanilla + pseudoPBR
	light = pseudopbr(N, V, L, H, alb_gloss.w).xxxy;
	return light;
}

float4 plight_infinity(float m, float3 pnt, float3 normal, float4 c_tex, float3 light_direction)
{
#ifdef USE_PBR
	float3 N = normalize(normal); //normal
	float3 V = -normalize(pnt); //vector2eye
	float3 L = -normalize(light_direction); //vector2light
	
	float4 light = compute_lighting(N,V,L,c_tex,m, false);
#else
  	float3 N = normal; // normal 
  	float3 V = -normalize	(pnt); // vector2eye
  	float3 L = -light_direction; // vector2light
  	float3 H = normalize	(L+V); // float-angle-vector
	
	float4 light	= s_material.Sample( smp_material, float3( dot(L,N), dot(H,N), m ) ).xxxy; // sample material
#endif
	return light; //output (albedo.gloss)
}

float4 plight_local(float m, float3 pnt, float3 normal, float4 c_tex, float3 light_position, float light_range_rsq, out float rsqr)
{
#ifdef USE_PBR
  	float3 N		= normalize(normal);							// normal 
  	float3 L2P 	= pnt-light_position;                         		// light2point 
  	float3 V 	= -normalize	(pnt);					// vector2eye
  	float3 L 	= -normalize	(L2P);					// vector2light
	
	rsqr	= dot		(L2P,L2P);					// distance 2 light (squared)
  	float  att 	= (saturate	(1 - rsqr*light_range_rsq));			// q-linear attenuate
	
	float4 light = compute_lighting(N,V,L,c_tex,m, true);
#else
  	float3 N	= normal; // normal 
  	float3 L2P 	= pnt-light_position; // light2point 
  	float3 V 	= -normalize	(pnt); // vector2eye
  	float3 L 	= -normalize	((float3)L2P); // vector2light
  	float3 H	= normalize	(L+V); // float-angle-vector
		rsqr	= dot		(L2P,L2P); // distance 2 light (squared)
  	float  att 	= saturate	(1 - rsqr*light_range_rsq); // q-linear attenuate
	float4 light	= s_material.Sample( smp_material, float3( dot(L,N), dot(H,N), m ) ).xxxy; // sample material
#endif
	
  	return att*light; //output (albedo.gloss)
}

//	TODO: DX10: Remove path without blending
float4 blendp(float4 value, float4 tcp)
{
	return 	value;
}

float4 blend(float4 value, float2 tc)
{
	return 	value;
}

#endif
