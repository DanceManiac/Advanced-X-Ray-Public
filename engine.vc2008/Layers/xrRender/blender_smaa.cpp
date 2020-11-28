#include "stdafx.h"
#pragma hdrstop

#include "blender_smaa.h"

CBlender_SMAA::CBlender_SMAA() { description.CLS = 0; }
CBlender_SMAA::~CBlender_SMAA() { }

void CBlender_SMAA::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);

	switch (C.iElement)
	{
	case 0:
		C.r_Pass("stub_screen_space", "smaa_edge_detect", FALSE, FALSE, FALSE);
		C.r_dx10Texture("s_image", r2_RT_generic0);
		C.r_dx10Texture("s_position", r2_RT_P);

		C.r_dx10Sampler("smp_rtlinear");
		C.r_dx10Sampler("smp_nofilter");

		C.r_End();

		break;
	case 1:
		C.r_Pass("stub_screen_space", "smaa_bweight_calc", FALSE, FALSE, FALSE);

		C.r_dx10Texture("s_edgetex", r2_RT_smaa_edgetex);
		C.r_dx10Texture("s_areatex", "shaders\\smaa_area_tex_dx10");
		C.r_dx10Texture("s_searchtex", "shaders\\smaa_search_tex");

		C.r_dx10Sampler("smp_rtlinear");
		C.r_dx10Sampler("smp_nofilter");

		C.r_End();

		break;
	case 2:
		C.r_Pass("stub_screen_space", "smaa_neighbour_blend", FALSE, FALSE, FALSE);

		C.r_dx10Texture("s_image", r2_RT_generic0);
		C.r_dx10Texture("s_blendtex", r2_RT_smaa_blendtex);

		C.r_dx10Sampler("smp_rtlinear");
		C.r_dx10Sampler("smp_nofilter");

		C.r_End();

		break;

	}
}