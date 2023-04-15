#include "stdafx.h"
#pragma hdrstop

#include "blender_chromatic_aberration.h"

CBlender_ChromaticAberration::CBlender_ChromaticAberration() { description.CLS = 0; }
CBlender_ChromaticAberration::~CBlender_ChromaticAberration() { }

void CBlender_ChromaticAberration::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);

	switch (C.iElement)
	{
	case 0:	// Vignette
		C.r_Pass("stub_screen_space", "chromatic_aberration", false, FALSE, FALSE);
		C.r_dx10Texture("s_image", r2_RT_generic0);
		C.r_End();
		break;
	}
}