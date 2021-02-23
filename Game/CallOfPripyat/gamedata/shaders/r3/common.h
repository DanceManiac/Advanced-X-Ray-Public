#ifndef        COMMON_H
#define        COMMON_H

#include "shared\common.h"

#include "common_defines.h"
#include "common_policies.h"
#include "common_iostructs.h"
#include "common_samplers.h"
#include "common_cbuffers.h"
#include "common_functions.h"

// #define USE_SUPER_SPECULAR
#define USE_SUNMASK                		//- shader defined
#define SKY_DEPTH	float(10000.f)

#ifdef        USE_R2_STATIC_SUN
#  define xmaterial float(1.0h/4.h)
#else
#  define xmaterial float(L_material.w)
#endif

#define FXPS technique _render{pass _code{PixelShader=compile ps_3_0 main();}}
#define FXVS technique _render{pass _code{VertexShader=compile vs_3_0 main();}}

#endif

//#include "ogse_functions.h"

#ifndef DWM_GBUFFER
#define DWM_GBUFFER
#define mix lerp

	#define FARPLANE 180

	float4 proj_to_screen(float4 proj) 
	{ 
		float4 screen = proj; 
		screen.x = (proj.x + proj.w); 
		screen.y = (proj.w - proj.y); 
		screen.xy *= 0.5; 
		return screen; 
	}
	
	uniform float4 u_weather; // .xyz - sky color, .w - rain line param
	uniform float4 lowland_fog_params; // x - low fog height, y - low fog density, z - base height, w - null
	uniform float4 screen_res_alt; // .xy - pos_decompression_params2.xy, .zw - screen_res.xy power to -1
	uniform float4 puddles_accumulator; // .x - wetness accumulator, .yzw = 0
	uniform float4x4	m_view2world;
	
	// Глобальные параметры шейдеров --#SM+#--
	uniform	float4		m_hud_params;	// zoom_rotate_factor, secondVP_zoom_factor, NULL, NULL
	uniform	float4		m_blender_mode;	

	float gbuf_get_depth(float2 texcoord)
	{
		#if defined(USE_MSAA)
			return s_position.Load(int2(texcoord.xy * screen_res_alt.xy), 0).z;
		#else
			return s_position.Sample(smp_nofilter, texcoord).z;
		#endif
	}

	float gbuf_get_depth(float4 texcoord_proj)
	{
		float2 texcoord = texcoord_proj.xy/texcoord_proj.w;

		return gbuf_get_depth(texcoord);
	}

	float gbuf_normalize_depth(float depth)
	{
		return (saturate(depth/FARPLANE));
	}

	float gbuf_get_remapped_depth(float2 texcoord)
	{
		float depth = gbuf_get_depth(texcoord);

		return (depth > 0.0025) ? (depth) : (1000);
	}

	float gbuf_transform_depth(float depth)
	{
		return depth * (1 - step(0.001, abs(depth - 10000)));
	}

	uniform float4 fov;

	#define InvFocalLen float2(tan(0.5f * radians(81.5)) / (float)screen_res_alt.y * (float)screen_res_alt.x, tan(0.5f * radians(fov.x)))

	float3 gbuf_get_position(float2 uv)
	{
		float eye_z = gbuf_get_depth(uv);

		uv = (uv * float2(2.0, -2.0) - float2(1.0, -1.0));

		float3 pos = float3(uv * InvFocalLen, 1);

		return pos * eye_z;
	}

	float3 gbuf_get_position(float2 uv, bool normalize)
	{
		float eye_z;

		if(normalize==true) 
		{eye_z = gbuf_normalize_depth(gbuf_get_depth(uv));}
		else
		{eye_z = gbuf_get_remapped_depth(uv);}

		uv = (uv * float2(2.0, -2.0) - float2(1.0, -1.0));

		float3 pos = float3(uv * InvFocalLen, 1);

		return pos * eye_z;
	}

	float2 ss_tc_from_wo_posi(float3 wo_posi)
	{
		float4	p_posi	   = mul(m_VP, float4(wo_posi, 1));
		float4	ss_tc 	   = proj_to_screen(p_posi);
				ss_tc.xy  /= ss_tc.w;

		return ss_tc.xy;
	}

	// От 0 до 1
	float smooth_out_depth(in float depth, in float max_length, float contrast)
	{
		depth  = clamp(depth, 0, max_length);
		depth  = max_length - depth;
		depth /= max_length;
		depth *= contrast;

		return  saturate(depth);
	}

	float4 SampleMix(Texture2D s_input, sampler you_sampler, float2 texcoord1, float2 texcoord2)
	{
		float4 T1 = s_input.Sample(you_sampler, texcoord1);
		float4 T2 = s_input.Sample(you_sampler, texcoord2);

		return T1 + T2 - 1;
	}

	float3 sky_fake_reflection(TextureCube skytex1, TextureCube skytex2, float3 vreflect)
	{
		float3	env0	= skytex1.Sample(smp_base, vreflect.xyz).xyz;
		float3	env1	= skytex2.Sample(smp_base, vreflect.xyz).xyz;

		return mix(env0,env1,L_ambient.w);
	}

	float3 true_remapping(float3 vreflect)
	{
		float3 s_vreflect = vreflect / max(abs(vreflect.x), max(abs(vreflect.y), abs(vreflect.z)));

		if(s_vreflect.y < 0.99) {s_vreflect.y = s_vreflect.y * 2 - 1;}

		return s_vreflect;
	}

	float3 sky_true_reflection(TextureCube skytex1, TextureCube skytex2, float3 vreflect)
	{
		return sky_fake_reflection(skytex1, skytex2, true_remapping(vreflect));
	}
	
	// Активен-ли двойной рендер --#SM+#--
	inline bool isSecondVPActive()
	{
		return (m_blender_mode.z == 1.f);
	}

#endif
