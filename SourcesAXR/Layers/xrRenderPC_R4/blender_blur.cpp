#include "stdafx.h"

#include "blender_blur.h"

CBlender_blur::CBlender_blur() { description.CLS = 0; }

CBlender_blur::~CBlender_blur()
{
}

void CBlender_blur::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);

	switch (C.iElement)
	{
	case 0:	//Fullres Horizontal
		C.r_Pass("stub_screen_space", "pp_blur", FALSE, FALSE, FALSE);
		C.r_dx10Texture("s_image", r2_RT_generic0);
		C.r_dx10Texture("s_position", r2_RT_P);		
		C.r_dx10Texture("s_lut_atlas", "shaders\\lut_atlas");

		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_rtlinear");
		C.r_End();
		break;
	case 1:	//Fullres Vertical
		C.r_Pass("stub_screen_space", "pp_blur", FALSE, FALSE, FALSE);
		C.r_dx10Texture("s_image", r2_RT_blur_h_2);
		C.r_dx10Texture("s_position", r2_RT_P);		
		C.r_dx10Texture("s_lut_atlas", "shaders\\lut_atlas");

		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_rtlinear");
		C.r_End();
		break;
	case 2: //Halfres Horizontal
		C.r_Pass("stub_screen_space", "pp_blur", FALSE, FALSE, FALSE);
		C.r_dx10Texture("s_image", r2_RT_generic0);
		C.r_dx10Texture("s_position", r2_RT_P);		
		C.r_dx10Texture("s_lut_atlas", "shaders\\lut_atlas");

		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_rtlinear");
		C.r_End();
		break;
	case 3: //Halfres Vertical
		C.r_Pass("stub_screen_space", "pp_blur", FALSE, FALSE, FALSE);
		C.r_dx10Texture("s_image", r2_RT_blur_h_4);
		C.r_dx10Texture("s_position", r2_RT_P);		
		C.r_dx10Texture("s_lut_atlas", "shaders\\lut_atlas");

		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_rtlinear");
		C.r_End();
		break;		
	case 4: //Quarterres Horizontal
		C.r_Pass("stub_screen_space", "pp_blur", FALSE, FALSE, FALSE);
		C.r_dx10Texture("s_image", r2_RT_generic0);
		C.r_dx10Texture("s_position", r2_RT_P);		
		C.r_dx10Texture("s_lut_atlas", "shaders\\lut_atlas");

		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_rtlinear");
		C.r_End();
		break;
	case 5: //Quarterres Vertical
		C.r_Pass("stub_screen_space", "pp_blur", FALSE, FALSE, FALSE);
		C.r_dx10Texture("s_image", r2_RT_blur_h_8);
		C.r_dx10Texture("s_position", r2_RT_P);		
		C.r_dx10Texture("s_lut_atlas", "shaders\\lut_atlas");

		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_rtlinear");
		C.r_End();
		break;				
	}
}

CBlender_ssfx_ssr::CBlender_ssfx_ssr() { description.CLS = 0; }

CBlender_ssfx_ssr::~CBlender_ssfx_ssr()
{
}

void CBlender_ssfx_ssr::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);

	switch (C.iElement)
	{
	case 0:	// Do SSR
		C.r_Pass("stub_screen_space", "ssfx_ssr", FALSE, FALSE, FALSE);

		C.r_dx10Texture("s_position", r2_RT_P);
		C.r_dx10Texture("ssr_image", r2_RT_ssfx); // Prev Frame

		C.r_dx10Texture("s_rimage", "$user$generic_temp");
		C.r_dx10Texture("s_ssfx_hud_mask", r2_RT_ssfx_hud);
		C.r_dx10Texture("s_prev_pos", r2_RT_ssfx_prevPos);
		C.r_dx10Texture("s_gloss_data", r2_RT_ssfx_temp3);

		C.r_dx10Texture("blue_noise", "shaders\\blue_noise");

		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_rtlinear");
		C.r_dx10Sampler("smp_linear");

		C.r_End();
		break;

	case 1:	// Blur Phase 1
		C.r_Pass("stub_screen_space", "ssfx_ssr_blur", FALSE, FALSE, FALSE);

		C.r_dx10Texture("ssr_image", r2_RT_ssfx);
		C.r_dx10Texture("s_diffuse", r2_RT_albedo);

		C.r_dx10Sampler("smp_nofilter");
		C.r_End();
		break;

	case 2:	// Blur Phase 2
		C.r_Pass("stub_screen_space", "ssfx_ssr_blur", FALSE, FALSE, FALSE);

		C.r_dx10Texture("ssr_image", r2_RT_ssfx_temp);
		C.r_dx10Texture("s_diffuse", r2_RT_albedo);

		C.r_dx10Sampler("smp_nofilter");
		C.r_End();
		break;

	case 3:	// Combine
		C.r_Pass("stub_screen_space", "ssfx_ssr_combine", FALSE, FALSE, FALSE);

		C.r_dx10Texture("s_position", r2_RT_P);
		C.r_dx10Texture("s_rimage", "$user$generic_temp");
		C.r_dx10Texture("ssr_image", r2_RT_ssfx_temp2);
		C.r_dx10Texture("s_gloss_data", r2_RT_ssfx_temp3);

		C.r_dx10Sampler("smp_linear");
		C.r_dx10Sampler("smp_nofilter");
		C.r_End();

		break;

	case 4:	// No blur just direct to [r2_RT_ssfx_temp2]
		C.r_Pass("stub_screen_space", "ssfx_ssr_noblur", FALSE, FALSE, FALSE);

		C.r_dx10Texture("ssr_image", r2_RT_ssfx);

		C.r_dx10Sampler("smp_nofilter");
		C.r_End();
		break;

	case 5:	// Copy from [r2_RT_ssfx_temp2] to [r2_RT_ssfx]
		C.r_Pass("stub_screen_space", "ssfx_ssr_gloss", FALSE, FALSE, FALSE);

		C.r_dx10Texture("s_position", r2_RT_P);
		C.r_dx10Texture("s_diffuse", r2_RT_albedo);
		C.r_dx10Texture("s_ssfx_hud_mask", r2_RT_ssfx_hud);
		C.r_dx10Texture("env_s0", r2_T_envs0);
		C.r_dx10Texture("env_s1", r2_T_envs1);
		C.r_dx10Texture("sky_s0", r2_T_sky0);
		C.r_dx10Texture("sky_s1", r2_T_sky1);

		C.r_dx10Sampler("smp_nofilter");
		C.r_End();
		break;
	}
}

CBlender_ssfx_volumetric_blur::CBlender_ssfx_volumetric_blur() { description.CLS = 0; }

CBlender_ssfx_volumetric_blur::~CBlender_ssfx_volumetric_blur()
{
}

void CBlender_ssfx_volumetric_blur::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);

	switch (C.iElement)
	{
	case 0:	// Blur Phase 1
		C.r_Pass("stub_screen_space", "ssfx_volumetric_blur", FALSE, FALSE, FALSE);

		C.r_dx10Texture("vol_buffer", r2_RT_ssfx_volumetric);

		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_linear");
		C.r_End();
		break;

	case 1:	// Blur Phase 2
		C.r_Pass("stub_screen_space", "ssfx_volumetric_blur", FALSE, FALSE, FALSE);

		C.r_dx10Texture("vol_buffer", r2_RT_ssfx_volumetric_tmp);

		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_linear");
		C.r_End();
		break;

	case 2:	// Blur Phase 1
		C.r_Pass("stub_screen_space", "ssfx_volumetric_blur", FALSE, FALSE, FALSE);
		C.r_dx10Texture("vol_buffer", r2_RT_generic2);
		
		C.r_dx10Sampler("smp_rtlinear");
		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_linear");
		C.r_End();
		break;

	case 3:	// Blur Phase 2
		C.r_Pass("stub_screen_space", "ssfx_volumetric_blur", FALSE, FALSE, FALSE);
		C.r_dx10Texture("vol_buffer", r2_RT_ssfx_accum);
		
		C.r_dx10Sampler("smp_rtlinear");
		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_linear");
		C.r_End();
		break;

	case 5:	// Combine
		C.r_Pass("stub_screen_space", "ssfx_volumetric_combine", FALSE, FALSE, FALSE);
		C.r_dx10Texture("vol_buffer", r2_RT_generic2);
		C.r_dx10Texture("vol_point", r2_RT_ssfx_volumetric);

		C.r_dx10Sampler("smp_rtlinear");
		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_linear");
		C.r_End();
		break;
	}
}
CBlender_ssfx_ao::CBlender_ssfx_ao() { description.CLS = 0; }

CBlender_ssfx_ao::~CBlender_ssfx_ao()
{
}

void CBlender_ssfx_ao::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);
	switch (C.iElement)
	{
	case 0:	// AO
		C.r_Pass("stub_screen_space", "ssfx_ao", FALSE, FALSE, FALSE);
		C.r_dx10Texture("s_position", r2_RT_P);
		C.r_dx10Texture("s_ssfx_hud_mask", r2_RT_ssfx_hud);
		C.r_dx10Texture("ssfx_ao", r2_RT_ssfx_ao);
		C.r_dx10Texture("s_prev_pos", r2_RT_ssfx_prevPos);
		C.r_dx10Texture("jitter0", JITTER(0));

		C.r_dx10Sampler("smp_rtlinear");
		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_linear");
		C.r_dx10Sampler("smp_jitter");
		C.r_End();
		break;

	case 1:	// Blur Phase 1
		C.r_Pass("stub_screen_space", "ssfx_ao_blur", FALSE, FALSE, FALSE);
		C.r_dx10Texture("ao_image", r2_RT_ssfx_temp);
		C.r_dx10Texture("s_ssfx_hud_mask", r2_RT_ssfx_hud);

		C.r_dx10Sampler("smp_rtlinear");
		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_linear");
		C.r_End();
		break;

	case 2:	// Blur Phase 2
		C.r_Pass("stub_screen_space", "ssfx_ao_blur", FALSE, FALSE, FALSE);
		C.r_dx10Texture("ao_image", r2_RT_ssfx_temp3);
		C.r_dx10Texture("s_ssfx_hud_mask", r2_RT_ssfx_hud);

		C.r_dx10Sampler("smp_rtlinear");
		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_linear");
		C.r_End();
		break;

	case 3:	// IL
		C.r_Pass("stub_screen_space", "ssfx_il", FALSE, FALSE, FALSE);
		C.r_dx10Texture("s_accumulator", r2_RT_accum);
		C.r_dx10Texture("s_position", r2_RT_P);
		C.r_dx10Texture("s_ssfx_hud_mask", r2_RT_ssfx_hud);
		C.r_dx10Texture("ssfx_ao", r2_RT_ssfx_il);
		C.r_dx10Texture("s_prev_pos", r2_RT_ssfx_prevPos);
		C.r_dx10Texture("jitter0", JITTER(0));

		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_linear");
		C.r_dx10Sampler("smp_jitter");
		C.r_End();
		break;

	case 4:	// Blur Phase 1
		C.r_Pass("stub_screen_space", "ssfx_il_blur", FALSE, FALSE, FALSE);
		C.r_dx10Texture("ao_image", r2_RT_ssfx_temp2);
		C.r_dx10Texture("s_ssfx_hud_mask", r2_RT_ssfx_hud);

		C.r_dx10Sampler("smp_rtlinear");
		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_linear");
		C.r_End();
		break;

	case 5:	// Blur Phase 2
		C.r_Pass("stub_screen_space", "ssfx_il_blur", FALSE, FALSE, FALSE);
		C.r_dx10Texture("ao_image", r2_RT_ssfx_temp3);
		C.r_dx10Texture("s_ssfx_hud_mask", r2_RT_ssfx_hud);

		C.r_dx10Sampler("smp_rtlinear");
		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_linear");
		C.r_End();
		break;
	}
}

CBlender_ssfx_sss::CBlender_ssfx_sss() { description.CLS = 0; }

CBlender_ssfx_sss::~CBlender_ssfx_sss()
{
}

void CBlender_ssfx_sss::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);

	switch (C.iElement)
	{
	case 0:	// SSS
		C.r_Pass("stub_screen_space", "ssfx_sss", FALSE, FALSE, FALSE);

		C.r_dx10Texture("s_position", r2_RT_P);
		C.r_dx10Texture("s_hud_mask", r2_RT_ssfx_hud);
		C.r_dx10Texture("sss_image", r2_RT_ssfx_sss);

		C.r_dx10Texture("s_prev_pos", r2_RT_ssfx_prevPos);

		C.r_dx10Texture("jitter0", JITTER(0));

		C.r_dx10Sampler("smp_rtlinear");
		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_linear");
		C.r_dx10Sampler("smp_jitter");

		C.r_End();
		break;

	case 1:	// SSS Blur Occ Mask - Phase 1
		C.r_Pass("stub_screen_space", "ssfx_sss_blur", FALSE, FALSE, FALSE);

		C.r_dx10Texture("sss_image", r2_RT_ssfx);

		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_linear");
		C.r_dx10Sampler("smp_jitter");

		C.r_End();
		break;

	case 2:	// SSS Blur Occ Mask - Phase 2
		C.r_Pass("stub_screen_space", "ssfx_sss_blur", FALSE, FALSE, FALSE);

		C.r_dx10Texture("sss_image", r2_RT_ssfx_temp);

		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_linear");
		C.r_dx10Sampler("smp_jitter");

		C.r_End();
		break;

	}
}


CBlender_ssfx_sss_ext::CBlender_ssfx_sss_ext() { description.CLS = 0; }

CBlender_ssfx_sss_ext::~CBlender_ssfx_sss_ext()
{
}

void CBlender_ssfx_sss_ext::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);

	switch (C.iElement)
	{
	case 0: // SSS Ext
		C.r_Pass("stub_screen_space", "ssfx_sss_ext", FALSE, FALSE, FALSE);

		C.r_dx10Texture("s_position", r2_RT_P);
		C.r_dx10Texture("s_hud_mask", r2_RT_ssfx_hud);
		C.r_dx10Texture("sss_image", r2_RT_ssfx_sss_ext);

		C.r_dx10Texture("s_prev_pos", r2_RT_ssfx_prevPos);

		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_linear");
		C.r_dx10Sampler("smp_jitter");

		C.r_End();
		break;

	case 1: // SSS Ext
		C.r_Pass("stub_screen_space", "ssfx_sss_ext", FALSE, FALSE, FALSE);

		C.r_dx10Texture("s_position", r2_RT_P);
		C.r_dx10Texture("s_hud_mask", r2_RT_ssfx_hud);
		C.r_dx10Texture("sss_image", r2_RT_ssfx_sss_ext2);

		C.r_dx10Texture("s_prev_pos", r2_RT_ssfx_prevPos);

		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_linear");
		C.r_dx10Sampler("smp_jitter");

		C.r_End();
		break;

	case 2: // SSS Ext Combine

		C.r_Pass("stub_screen_space", "ssfx_sss_ext_combine", FALSE, FALSE, FALSE);

		C.r_dx10Texture("sss_image_1", r2_RT_ssfx_sss_ext);
		C.r_dx10Texture("sss_image_2", r2_RT_ssfx_sss_ext2);

		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_linear");

		C.r_End();
		break;
	}
}

CBlender_ssfx_rain::CBlender_ssfx_rain() { description.CLS = 0; }

CBlender_ssfx_rain::~CBlender_ssfx_rain()
{
}

void CBlender_ssfx_rain::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);

	switch (C.iElement)
	{
	case 0:
		C.r_Pass("stub_screen_space", "ssfx_rain", FALSE, FALSE, FALSE);

		C.r_dx10Texture("s_position", r2_RT_P);
		C.r_dx10Texture("ssfx_color_buffer", r2_RT_generic0);
		C.r_dx10Texture("volumetric_buffer", r2_RT_ssfx_volumetric);

		C.r_dx10Sampler("smp_rtlinear");
		C.r_dx10Sampler("smp_nofilter");

		C.r_End();
		break;

	case 1: // MSAA
		C.r_Pass("stub_screen_space", "ssfx_rain", FALSE, FALSE, FALSE);

		C.r_dx10Texture("s_position", r2_RT_P);
		C.r_dx10Texture("ssfx_color_buffer", r2_RT_generic0_r);
		C.r_dx10Texture("volumetric_buffer", r2_RT_ssfx_volumetric);

		C.r_dx10Sampler("smp_rtlinear");
		C.r_dx10Sampler("smp_nofilter");

		C.r_End();
		break;
	}
}

CBlender_ssfx_water_blur::CBlender_ssfx_water_blur() { description.CLS = 0; }

CBlender_ssfx_water_blur::~CBlender_ssfx_water_blur()
{
}

void CBlender_ssfx_water_blur::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);

	switch (C.iElement)
	{
	case 0:	// Blur Phase 1
		C.r_Pass("stub_screen_space", "ssfx_water_blur", FALSE, FALSE, FALSE);

		C.r_dx10Texture("water_buffer", r2_RT_ssfx_temp);

		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_linear");
		C.r_End();
		break;

	case 1:	// Blur Phase 2
		C.r_Pass("stub_screen_space", "ssfx_water_blur", FALSE, FALSE, FALSE);

		C.r_dx10Texture("water_buffer", r2_RT_ssfx_temp2);

		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_linear");
		C.r_End();
		break;

	case 2:	// No Blur
		C.r_Pass("stub_screen_space", "ssfx_water_noblur", FALSE, FALSE, FALSE);

		C.r_dx10Texture("water_buffer", r2_RT_ssfx_temp2);

		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_linear");
		C.r_End();
		break;

	case 5:	// Water Waves
		C.r_Pass("stub_screen_space", "ssfx_water_waves", FALSE, FALSE, FALSE);

		C.r_dx10Texture("water_waves", "fx\\water_height");

		C.r_dx10Sampler("smp_linear");
		C.r_End();
		break;
	}
}