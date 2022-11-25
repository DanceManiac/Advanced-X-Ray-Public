#ifndef REFLECTIONS_H
#define REFLECTIONS_H
#define WATER_SPLASHES_MAX_RADIUS 1.5 // Maximum number of cells a ripple can cross.
//#define WATER_SPLASHES_DOUBLE_HASH	  // дополнительный шум

TextureCube	s_env0;
TextureCube	s_env1;

float3 calc_envmap(float3 vreflect)
{
	float3 vreflectabs = abs(vreflect);
	float  vreflectmax = max(vreflectabs.x, max(vreflectabs.y, vreflectabs.z));
	vreflect /= vreflectmax;
	if (vreflect.y < 0.999)
			vreflect.y= vreflect.y * 2 - 1; // fake remapping

	float3 env0 = s_env0.SampleLevel(smp_base, vreflect.xyz, 0).xyz;
	float3 env1 = s_env1.SampleLevel(smp_base, vreflect.xyz, 0).xyz;
	env0 *= env0*2;
	env1 *= env1*2;
	return lerp(env0, env1, L_ambient.w);
}
//*******************************************************************************************************************
//*******************************************************************************************************************
float3 specular_phong(float3 pnt, float3 normal, float3 light_direction)
{
    return L_sun_color.rgb * pow( abs( dot(normalize(pnt + light_direction), normal) ), 256.0);
}
//*******************************************************************************************************************
//*******************************************************************************************************************

float hash12(float2 p)
{
	float3 p3  = frac(float3(p.xyx) * .1031);
    p3 += dot(p3, p3.yzx + 19.19);
    return frac((p3.x + p3.y) * p3.z);
}

float2 hash22(float2 p)
{
	float3 p3 = frac(float3(p.xyx) * float3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yzx+19.19);
    return frac((p3.xx+p3.yz)*p3.zy);
}

float3 calc_rain_splashes(float2 tc)
{
	float2 p0 = floor(tc * 35);

	float circles = 0;

	for (int j = -WATER_SPLASHES_MAX_RADIUS; j <= WATER_SPLASHES_MAX_RADIUS; ++j)
	{
		for (int i = -WATER_SPLASHES_MAX_RADIUS; i <= WATER_SPLASHES_MAX_RADIUS; ++i)
		{
			float2 pi = p0 + float2(i, j);
		#ifdef WATER_SPLASHES_DOUBLE_HASH
			float2 hsh = hash22(pi);
		#else
			float2 hsh = pi;
		#endif
			float2 p = pi + hash22(hsh);

			float t = frac(1.45f * timers.x + hash12(hsh));
			float2 v = p - tc * 35;

			float d = (length(v) * 2.0f) - (float(WATER_SPLASHES_MAX_RADIUS) + 1.0) * t;

			const float h = 1e-3;
			float d1 = d - h;
			float d2 = d + h;
			float p1 = sin(31. * d1) * smoothstep(-0.6, -0.3, d1) * smoothstep(0., -0.3, d1);
			float p2 = sin(31. * d2) * smoothstep(-0.6, -0.3, d2) * smoothstep(0., -0.3, d2);
			circles += 0.5 * normalize(v) * ((p2 - p1) / (2. * h) * (1. - t) * (1. - t));
		}
	}

	float c = float(WATER_SPLASHES_MAX_RADIUS * 2 + 1);
	circles /= c * c;

	return float3(circles.xx, sqrt(1.0f - dot(circles, circles)));
}

#endif