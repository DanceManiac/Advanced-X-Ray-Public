#include "stdafx.h"
#pragma hdrstop

#include "blender_lens_flares.h"

CBlender_LFX::CBlender_LFX() { description.CLS = 0; }
CBlender_LFX::~CBlender_LFX() {}

void CBlender_LFX::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);
	switch (C.iElement)
	{
	case 0:
		C.r_Pass("lfx_main", "lfx_main", false, FALSE, FALSE, TRUE, D3DBLEND_ONE, D3DBLEND_ONE);
		C.r_dx10Texture("s_base0", r2_RT_generic0);
		C.r_dx10Sampler("smp_rtlinear");
		C.r_End();
		break;
	}
}