#include "stdafx.h"

#include "blender_cut.h"



void CBlender_cut::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);

	switch (C.iElement)
	{
	case 0:
		C.r_Pass("stub_notransform_t", "cut", FALSE, TRUE, TRUE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);
		C.r_dx10Texture("s_base", "wpn\\wpn_cut");
		C.r_dx10Sampler("smp_base");
		C.RS.SetRS(XRDX10RS_ALPHATOCOVERAGE, TRUE);
		C.r_End();
		break;
	}
}

CBlender_cut::CBlender_cut()
{
	description.CLS = 0;
}

CBlender_cut::~CBlender_cut()
{
}
