#include "stdafx.h"
#pragma hdrstop

#include "blender_vignette.h"

CBlender_Vignette::CBlender_Vignette() { description.CLS = 0; }
CBlender_Vignette::~CBlender_Vignette() { }

void CBlender_Vignette::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);

	switch (C.iElement)
	{
	case 0:	// Vignette
		C.r_Pass("null", "vignette", false, FALSE, FALSE);
		C.r_Sampler("s_image", r2_RT_generic0);
		C.r_End();
		break;
	}
}