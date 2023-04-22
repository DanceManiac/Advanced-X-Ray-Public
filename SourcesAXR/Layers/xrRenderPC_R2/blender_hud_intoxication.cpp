#include "stdafx.h"
#pragma hdrstop

#include "blender_hud_intoxication.h"

CBlender_Hud_Intoxication::CBlender_Hud_Intoxication() { description.CLS = 0; }
CBlender_Hud_Intoxication::~CBlender_Hud_Intoxication() {}

void CBlender_Hud_Intoxication::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);
	switch (C.iElement)
	{
		case 0:
			C.r_Pass("null", "hud_intoxication", FALSE, FALSE, FALSE);
			C.r_Sampler_clf("s_image", r2_RT_generic0);
			C.r_Sampler_clf("s_hud_intoxication", "shaders\\hud_mask\\hud_intoxication");
			C.r_End();
		break;
	}
}