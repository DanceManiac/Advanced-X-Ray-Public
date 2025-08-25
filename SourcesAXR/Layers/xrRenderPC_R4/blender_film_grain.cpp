#include "stdafx.h"
#pragma hdrstop

#include "blender_film_grain.h"

CBlender_FilmGrain::CBlender_FilmGrain() { description.CLS = 0; }
CBlender_FilmGrain::~CBlender_FilmGrain() { }

void CBlender_FilmGrain::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);

	switch (C.iElement)
	{
	case 0:	// Vignette
		C.r_Pass("stub_screen_space", "hud_film_grain", false, FALSE, FALSE);
		C.r_dx10Texture("s_image", r2_RT_generic0);
		C.r_End();
		break;
	}
}