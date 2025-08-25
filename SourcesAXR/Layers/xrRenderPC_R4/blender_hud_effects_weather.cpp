#include "stdafx.h"
#pragma hdrstop

#include "blender_hud_effects_weather.h"

CBlender_Hud_Rainfall::CBlender_Hud_Rainfall() { description.CLS = 0; }
CBlender_Hud_Rainfall::~CBlender_Hud_Rainfall() {}

CBlender_Hud_Sweated::CBlender_Hud_Sweated() { description.CLS = 0; }
CBlender_Hud_Sweated::~CBlender_Hud_Sweated() {}

CBlender_Hud_Rainfall_Acid::CBlender_Hud_Rainfall_Acid() { description.CLS = 0; }
CBlender_Hud_Rainfall_Acid::~CBlender_Hud_Rainfall_Acid() {}

void CBlender_Hud_Rainfall::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);
	switch (C.iElement)
	{
		case 0:
			C.r_Pass("stub_screen_space", "hud_weather_rainfall", FALSE, FALSE, FALSE);
			C.r_dx10Texture("s_image", r2_RT_generic0);
			C.r_dx10Texture("s_hud_rainfall", "shaders\\hud_mask\\hud_rainfall");
			C.r_dx10Texture("s_hud_raindrop_small", "shaders\\hud_mask\\hud_raindrop_small");
			C.r_dx10Sampler("SamplerLinearClamp");
			C.r_End();
		break;
	}
}

void CBlender_Hud_Sweated::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);
	switch (C.iElement)
	{
	case 1:
		C.r_Pass("stub_screen_space", "hud_sweat", FALSE, FALSE, FALSE);
		C.r_dx10Texture("s_image", r2_RT_generic0);
		C.r_dx10Texture("s_hud_sweated", "shaders\\hud_mask\\hud_sweated");
		C.r_dx10Sampler("smp_rtlinear");
		C.r_End();
		break;
	}
}

void CBlender_Hud_Rainfall_Acid::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);
	switch (C.iElement)
	{
	case 2:
		C.r_Pass("stub_screen_space", "hud_weather_rainfall_acid", FALSE, FALSE, FALSE);
		C.r_dx10Texture("s_image", r2_RT_generic0);
		C.r_dx10Texture("s_hud_rainfall_acid", "shaders\\hud_mask\\hud_rainfall_acid");
		C.r_dx10Texture("s_hud_raindrop_acid_small", "shaders\\hud_mask\\hud_raindrop_acid_small");
		C.r_dx10Sampler("SamplerLinearClamp");
		C.r_End();
		break;
	}
}