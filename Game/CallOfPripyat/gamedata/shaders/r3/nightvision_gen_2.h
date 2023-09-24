#include "common.h"

//Night Vision Type 2 Shader
//Ported from ShaderToy by Dance Maniac for Advanced X-Ray and G.S.W.R.
//https://www.shadertoy.com/view/XlsGzs#
//https://www.shadertoy.com/view/Xsf3RN

#define NVG_WARP_LINE_PERIOD_NVG_2 0.1
#define NVG_WARP_LINE_INTENSITY_NVG_2 5.0

float4 process_night_vision_2(p_screen I) : SV_Target
{
	float4 jitter = float4(
		frac(sin(dot(I.tc0, float2(12.0, 78.0) + (timers.x) )) * 12345.0),
		frac(sin(dot(I.tc0, float2(12.0, 78.0) + (timers.x) )) * 67890.0),
		frac(sin(dot(I.tc0, float2(12.0, 78.0) + (timers.x) )) * 78372.0),
		frac(sin(dot(I.tc0, float2(12.0, 78.0) + (timers.x) )) * 37857.0));
		
	float2 uv = I.hpos.xy * screen_res.zw;
	
	float wide = screen_res.x / screen_res.y;
	float high = 1.0;
	
	float2 position = float2(uv.x * wide, uv.y);
	
	float distanceFromCenter = length( uv - float2(0.5,0.5) );
    
    float vignetteAmount = 1.0 - distanceFromCenter;
    vignetteAmount = smoothstep(0.1, 1.0, vignetteAmount);
	
	//Circle circle_a = Circle(float2(0.5, 0.5), 0.5);
	//Circle circle_b = Circle(float2(wide - 0.5, 0.5), 0.5);
	//float4 mask_a = circle_mask_color(circle_a, position);
	//float4 mask_b = circle_mask_color(circle_b, position);
	//float4 mask = mask_blend(mask_a, mask_b);
	
	float greenness = 0.4;
	float4 coloring = float4(1.0, 2.0, 1.3, 1.0);
	
	float warpLine = frac(+timers.x * (NVG_WARP_LINE_PERIOD_NVG_2 + (nightvision_params.x * 10)));
	
	/** debug
	if(abs(uv.y - warpLine) < 0.003)
	{
		fragColor = vec4(1.0, 1.0, 1.0, 1.0);
		return;
	}
    */
	
	float warpLen = 0.1;
	float warpArg01 = remap(clamp((position.y - warpLine) - warpLen * 0.5, 0.0, warpLen), 0.0, warpLen, 0.0, 1.0);
	float offset = sin(warpArg01 * (NVG_WARP_LINE_INTENSITY_NVG_2 + (nightvision_params.x * 10)))  * f1(warpArg01);
	
	
	float4 lineNoise = float4(1.0, 1.0, 1.0, 1.0);
	
	if(abs(uv.y - frac(+timers.x * (19.0 + (nightvision_params.x * 100)))) < 0.0005 + (nightvision_params.x / 250.0))
	{
		lineNoise = float4(0.5, 0.5, 0.5, 3.0);
	}
	
	float3 color = s_image.Sample(smp_base, I.tc0 + float2(offset * 0.02, 0.0)).xyz;
	
	// APPLY NOISE
	color += jitter.y * (0.075 + (nightvision_params.x / 3.0)); // Add the noise to the image
	
	    // vignetting
    color *=  vignetteAmount*0.75;
	
	return float4(color * 2.0 * coloring * lineNoise, 1.0);

}