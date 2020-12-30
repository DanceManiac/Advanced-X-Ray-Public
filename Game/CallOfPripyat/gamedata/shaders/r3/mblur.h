#ifndef	MBLUR_H
#define MBLUR_H
#include "common.h"

#ifndef USE_MBLUR
float3 mblur(float2 UV, float3 pos, float3 c_original)
{
	return c_original;
}
#else

uniform float4x4	m_current;
uniform float4x4	m_previous;
uniform float2		m_blur;		// x=scale_x/12, y=scale_y/12

#define MBLUR_SAMPLES	float(12.0f)
#define MBLUR_CLAMP		float(0.001f)

float3 mblur(float2 UV, float3 pos, float3 c_original)
{
	// The sky depth value is 0 since we're rendering it without z-test
	if (pos.z < 0.00001f)
		pos.z = SKY_DEPTH;

	float4	pos4		= float4(pos, 1.0f);

	float4	p_current	= mul(m_current, pos4);
	float4	p_previous 	= mul(m_previous, pos4);
	float2	p_velocity 	= m_blur * ((p_current.xy/p_current.w) - (p_previous.xy/p_previous.w));
			p_velocity	= clamp(p_velocity, -MBLUR_CLAMP, +MBLUR_CLAMP);

	// For each sample, sum up each sample's color in "Blurred" and then divide
	// to average the color after all the samples are added.
	float3	blurred 	= c_original;
	
	for (int i = 1; i < MBLUR_SAMPLES; ++i)
		blurred += s_image.Sample(smp_rtlinear, p_velocity * i + UV).xyz;

	return blurred/MBLUR_SAMPLES;
}
#endif
#endif // MBLUR_H
