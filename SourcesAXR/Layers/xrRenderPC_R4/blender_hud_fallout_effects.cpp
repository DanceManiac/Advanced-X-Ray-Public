#include "stdafx.h"
#pragma hdrstop

#include "blender_hud_fallout_effects.h"

CBlender_Hud_Acid_Rain::CBlender_Hud_Acid_Rain() { description.CLS = 0; }
CBlender_Hud_Acid_Rain::~CBlender_Hud_Acid_Rain() {}

void CBlender_Hud_Acid_Rain::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);
	switch (C.iElement)
	{
		case 0:
			C.r_Pass("stub_screen_space", "hud_fallout_acid_rain", FALSE, FALSE, FALSE);
			C.r_dx10Texture("s_image", r2_RT_generic0);
			C.r_dx10Texture("s_hud_acid_rain", "shaders\\hud_mask\\hud_fallout_acid_rain");
			C.r_dx10Sampler("smp_rtlinear");
			C.r_End();
		break;
	}
}

CBlender_Hud_Radiation_Rain::CBlender_Hud_Radiation_Rain() { description.CLS = 0; }
CBlender_Hud_Radiation_Rain::~CBlender_Hud_Radiation_Rain() {}

void CBlender_Hud_Radiation_Rain::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);
	switch (C.iElement)
	{
	case 1:
		C.r_Pass("stub_screen_space", "hud_fallout_radiation_rain", FALSE, FALSE, FALSE);
		C.r_dx10Texture("s_image", r2_RT_generic0);
		C.r_dx10Texture("s_hud_radiation_rain", "shaders\\hud_mask\\hud_fallout_radiation_rain");
		C.r_dx10Sampler("smp_rtlinear");
		C.r_End();
		break;
	}
}