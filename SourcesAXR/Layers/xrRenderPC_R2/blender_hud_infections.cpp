#include "stdafx.h"
#pragma hdrstop

#include "blender_hud_infections.h"

CBlender_Hud_Infections::CBlender_Hud_Infections() { description.CLS = 0; }
CBlender_Hud_Infections::~CBlender_Hud_Infections() {}

void CBlender_Hud_Infections::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);
	switch (C.iElement)
	{
	case 0:
		C.r_Pass("null", "hud_infections", FALSE, FALSE, FALSE);
		C.r_Sampler_clf("s_image", r2_RT_generic0);
		C.r_Sampler_clf("s_hud_infections", "shaders\\hud_mask\\hud_infections");
		C.r_End();
		break;
	}
}