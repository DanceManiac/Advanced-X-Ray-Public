#include "stdafx.h"
#pragma hdrstop

#include "blender_hud_stamina.h"

CBlender_Hud_Stamina::CBlender_Hud_Stamina() { description.CLS = 0; }
CBlender_Hud_Stamina::~CBlender_Hud_Stamina() {}

void CBlender_Hud_Stamina::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);
	switch (C.iElement)
	{
		case 0:
			C.r_Pass("null", "hud_power", FALSE, FALSE, FALSE);
			C.r_Sampler_clf("s_image", r2_RT_generic0);
			C.r_Sampler_clf("s_hud_power", "shaders\\hud_mask\\hud_power");
			C.r_End();
		break;
	}
}