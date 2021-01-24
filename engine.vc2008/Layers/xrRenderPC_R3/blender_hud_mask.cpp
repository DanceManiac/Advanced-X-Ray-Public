#include "stdafx.h"
#pragma hdrstop

#include "blender_hud_mask.h"

CBlender_hud_mask::CBlender_hud_mask() { description.CLS = 0; }
CBlender_hud_mask::~CBlender_hud_mask() { }

void CBlender_hud_mask::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);

	switch (C.iElement)
	{
		case 0:
			C.r_Pass("stub_screen_space", "hud_mask", FALSE, FALSE, FALSE);
			C.r_dx10Texture("s_image", r2_RT_generic0);
			C.r_dx10Texture("s_hud_mask", "shaders\\hud_mask\\ui_hud_mask_d_0");
			C.r_dx10Sampler("smp_rtlinear");
		break;

		case 1:
			C.r_Pass("stub_screen_space", "hud_mask", FALSE, FALSE, FALSE);
			C.r_dx10Texture("s_image", r2_RT_generic0);
			C.r_dx10Texture("s_hud_mask", "shaders\\hud_mask\\ui_hud_mask_d_1");
			C.r_dx10Sampler("smp_rtlinear");
		break;

		case 2:
			C.r_Pass("stub_screen_space", "hud_mask", FALSE, FALSE, FALSE);
			C.r_dx10Texture("s_image", r2_RT_generic0);
			C.r_dx10Texture("s_hud_mask", "shaders\\hud_mask\\ui_hud_mask_d_2");
			C.r_dx10Sampler("smp_rtlinear");
		break;

		case 3:
			C.r_Pass("stub_screen_space", "hud_mask", FALSE, FALSE, FALSE);
			C.r_dx10Texture("s_image", r2_RT_generic0);
			C.r_dx10Texture("s_hud_mask", "shaders\\hud_mask\\ui_hud_mask_d_3");
			C.r_dx10Sampler("smp_rtlinear");
		break;

		case 4:
			C.r_Pass("stub_screen_space", "hud_mask", FALSE, FALSE, FALSE);
			C.r_dx10Texture("s_image", r2_RT_generic0);
			C.r_dx10Texture("s_hud_mask", "shaders\\hud_mask\\ui_hud_mask_d_4");
			C.r_dx10Sampler("smp_rtlinear");
		break;

		case 5:
			C.r_Pass("stub_screen_space", "hud_mask", FALSE, FALSE, FALSE);
			C.r_dx10Texture("s_image", r2_RT_generic0);
			C.r_dx10Texture("s_hud_mask", "shaders\\hud_mask\\ui_hud_mask_d_5");
			C.r_dx10Sampler("smp_rtlinear");
		break;
	}
	C.r_End();
}