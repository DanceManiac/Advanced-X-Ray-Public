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
			C.r_Pass("stub_notransform_aa_AA", "hud_power", FALSE, FALSE, FALSE);
			C.r_dx10Texture("s_image", r2_RT_generic0);
			C.r_dx10Texture("s_hud_power", "shaders\\hud_mask\\hud_power");
			C.r_dx10Sampler("smp_rtlinear");
			C.r_End();
		break;
	}
}