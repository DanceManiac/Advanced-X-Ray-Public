#include "stdafx.h"
#pragma hdrstop

#include "blender_hud_frost.h"

CBlender_Hud_Frost::CBlender_Hud_Frost() { description.CLS = 0; }
CBlender_Hud_Frost::~CBlender_Hud_Frost() {}

void CBlender_Hud_Frost::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);
	switch (C.iElement)
	{
		case 0:
			C.r_Pass("null", "hud_frost", FALSE, FALSE, FALSE);
			C.r_Sampler_clf("s_image", r2_RT_generic0);
			C.r_Sampler_clf("s_hud_frost", "shaders\\hud_mask\\hud_frost");
			C.r_End();
		break;
	}
}