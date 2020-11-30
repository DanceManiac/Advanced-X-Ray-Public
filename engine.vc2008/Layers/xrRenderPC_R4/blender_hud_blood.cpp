#include "stdafx.h"
#pragma hdrstop

#include "blender_hud_blood.h"

CBlender_Hud_Blood::CBlender_Hud_Blood() { description.CLS = 0; }
CBlender_Hud_Blood::~CBlender_Hud_Blood() {}

void CBlender_Hud_Blood::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);
	switch (C.iElement)
	{
		case 0:
			C.r_Pass("stub_notransform_aa_AA", "hud_blood", FALSE, FALSE, FALSE);
			C.r_dx10Texture("s_image", r2_RT_generic0);
			C.r_dx10Texture("s_hud_blood", "shaders\\hud_mask\\hud_blood");
			C.r_dx10Sampler("smp_rtlinear");
			C.r_End();
		break;
	}
}