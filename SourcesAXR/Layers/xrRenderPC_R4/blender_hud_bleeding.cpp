#include "stdafx.h"
#pragma hdrstop

#include "blender_hud_bleeding.h"

CBlender_Hud_Bleeding::CBlender_Hud_Bleeding() { description.CLS = 0; }
CBlender_Hud_Bleeding::~CBlender_Hud_Bleeding() {}

void CBlender_Hud_Bleeding::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);
	switch (C.iElement)
	{
		case 0:
			C.r_Pass("stub_screen_space", "hud_bleeding", FALSE, FALSE, FALSE);
			C.r_dx10Texture("s_image", r2_RT_generic0);
			C.r_dx10Texture("s_hud_bleeding", "shaders\\hud_mask\\hud_bleeding");
			C.r_dx10Sampler("smp_rtlinear");
			C.r_End();
		break;
	}
}