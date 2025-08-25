#include "stdafx.h"
#pragma hdrstop

#include "blender_hud_effects_winter.h"

CBlender_Hud_Frost::CBlender_Hud_Frost() { description.CLS = 0; }
CBlender_Hud_Frost::~CBlender_Hud_Frost() {}

CBlender_Hud_Cold::CBlender_Hud_Cold() { description.CLS = 0; }
CBlender_Hud_Cold::~CBlender_Hud_Cold() {}

CBlender_Hud_Snowfall::CBlender_Hud_Snowfall() { description.CLS = 0; }
CBlender_Hud_Snowfall::~CBlender_Hud_Snowfall() {}

void CBlender_Hud_Frost::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);
	switch (C.iElement)
	{
		case 0:
			C.r_Pass("null", "hud_winter_frost", FALSE, FALSE, FALSE);
			C.r_Sampler_clf("s_image", r2_RT_generic0);
			C.r_Sampler_clf("s_hud_frost", "shaders\\hud_mask\\hud_frost");
			C.r_End();
		break;
	}
}

void CBlender_Hud_Cold::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);
	switch (C.iElement)
	{
		case 1:
			C.r_Pass("null", "hud_winter_cold", FALSE, FALSE, FALSE);
			C.r_Sampler_clf("s_image", r2_RT_generic0);
			C.r_Sampler_clf("s_hud_cold", "shaders\\hud_mask\\hud_cold");
			C.r_End();
		break;
	}
}

void CBlender_Hud_Snowfall::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);
	switch (C.iElement)
	{
		case 2:
			C.r_Pass("null", "hud_winter_snowfall", FALSE, FALSE, FALSE);
			C.r_Sampler_clf("s_image", r2_RT_generic0);
			C.r_Sampler_clf("s_hud_snowfall", "shaders\\hud_mask\\hud_snowfall");
			C.r_End();
		break;
	}
}