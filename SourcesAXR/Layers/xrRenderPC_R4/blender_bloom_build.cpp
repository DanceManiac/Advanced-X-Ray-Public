#include "stdafx.h"
#pragma hdrstop

#include "Blender_bloom_build.h"

CBlender_bloom_build::CBlender_bloom_build	()	{	description.CLS		= 0;	}
CBlender_bloom_build::~CBlender_bloom_build	()	{	}
 
void	CBlender_bloom_build::Compile			(CBlender_Compile& C)
{
	IBlender::Compile		(C);

	switch (C.iElement)
	{
	case 0:		// transfer into bloom-target
		C.r_Pass			("stub_notransform_build","bloom_build",	FALSE,	FALSE,	FALSE, FALSE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);
		//C.r_Sampler_clf		("s_image",			r2_RT_generic1);
		C.r_dx10Texture		("s_image",			r2_RT_generic1);
		C.r_dx10Sampler		("smp_rtlinear");
		C.r_End				();
		break;
	case 1:		// X-filter
		C.r_Pass			("stub_notransform_filter","bloom_filter",		FALSE,	FALSE,	FALSE);
		//C.r_Sampler_clf		("s_bloom",			r2_RT_bloom1);
		C.r_dx10Texture		("s_bloom",			r2_RT_bloom1);
		C.r_dx10Sampler		("smp_rtlinear");
		C.r_End				();
		break;
	case 2:		// Y-filter
		C.r_Pass			("stub_notransform_filter","bloom_filter",		FALSE,	FALSE,	FALSE);
		//C.r_Sampler_clf		("s_bloom",			r2_RT_bloom2);
		C.r_dx10Texture		("s_bloom",			r2_RT_bloom2);
		C.r_dx10Sampler		("smp_rtlinear");
		C.r_End				();
		break;
	case 3:		// FF-filter_P0
		C.r_Pass			("stub_notransform_build","bloom_filter_f",	FALSE,	FALSE,	FALSE);
		//C.r_Sampler_clf		("s_bloom",			r2_RT_bloom1);
		C.r_dx10Texture		("s_bloom",			r2_RT_bloom1);
		C.r_dx10Sampler		("smp_rtlinear");
		C.r_End				();
		break;
	case 4:		// FF-filter_P1
		C.r_Pass			("stub_notransform_build","bloom_filter_f",	FALSE,	FALSE,	FALSE);
		//C.r_Sampler_clf		("s_bloom",			r2_RT_bloom2);
		C.r_dx10Texture		("s_bloom",			r2_RT_bloom2);
		C.r_dx10Sampler		("smp_rtlinear");
		C.r_End				();
		break;
	}
}

CBlender_bloom_build_msaa::CBlender_bloom_build_msaa	()	{	description.CLS		= 0;	}
CBlender_bloom_build_msaa::~CBlender_bloom_build_msaa	()	{	}

void	CBlender_bloom_build_msaa::Compile			(CBlender_Compile& C)
{
   IBlender::Compile		(C);

   switch (C.iElement)
   {
   case 0:		// transfer into bloom-target
      C.r_Pass			("stub_notransform_build","bloom_build",	FALSE,	FALSE,	FALSE, FALSE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);
      //C.r_Sampler_clf		("s_image",			r2_RT_generic1);
      C.r_dx10Texture		("s_image",			r2_RT_generic1);
      C.r_dx10Sampler		("smp_rtlinear");
      C.r_End				();
      break;
   case 1:		// X-filter
      C.r_Pass			("stub_notransform_filter","bloom_filter",		FALSE,	FALSE,	FALSE);
      //C.r_Sampler_clf		("s_bloom",			r2_RT_bloom1);
      C.r_dx10Texture		("s_bloom",			r2_RT_bloom1);
      C.r_dx10Sampler		("smp_rtlinear");
      C.r_End				();
      break;
   case 2:		// Y-filter
      C.r_Pass			("stub_notransform_filter","bloom_filter",		FALSE,	FALSE,	FALSE);
      //C.r_Sampler_clf		("s_bloom",			r2_RT_bloom2);
      C.r_dx10Texture		("s_bloom",			r2_RT_bloom2);
      C.r_dx10Sampler		("smp_rtlinear");
      C.r_End				();
      break;
   case 3:		// FF-filter_P0
      C.r_Pass			("stub_notransform_build","bloom_filter_f",	FALSE,	FALSE,	FALSE);
      //C.r_Sampler_clf		("s_bloom",			r2_RT_bloom1);
      C.r_dx10Texture		("s_bloom",			r2_RT_bloom1);
      C.r_dx10Sampler		("smp_rtlinear");
      C.r_End				();
      break;
   case 4:		// FF-filter_P1
      C.r_Pass			("stub_notransform_build","bloom_filter_f",	FALSE,	FALSE,	FALSE);
      //C.r_Sampler_clf		("s_bloom",			r2_RT_bloom2);
      C.r_dx10Texture		("s_bloom",			r2_RT_bloom2);
      C.r_dx10Sampler		("smp_rtlinear");
      C.r_End				();
      break;
   }
}

CBlender_postprocess_msaa::CBlender_postprocess_msaa	()	{	description.CLS		= 0;	}
CBlender_postprocess_msaa::~CBlender_postprocess_msaa	()	{	}

void	CBlender_postprocess_msaa::Compile			(CBlender_Compile& C)
{
	IBlender::Compile		(C);

	switch (C.iElement)
	{
	case 0:		// transfer into bloom-target
		C.r_Pass			("stub_notransform_postpr","postprocess",	FALSE,	FALSE,	FALSE, FALSE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);
		C.r_dx10Texture		("s_base0",	r2_RT_generic);
		C.r_dx10Texture		("s_base1",	r2_RT_generic);
		C.r_dx10Texture		("s_noise", "fx\\fx_noise2");

		C.r_dx10Sampler		("smp_rtlinear");
		C.r_dx10Sampler		("smp_linear");
		C.r_End				();
		break;

	case 4:		// use color map
		C.r_Pass			("stub_notransform_postpr","postprocess_CM",	FALSE,	FALSE,	FALSE, FALSE, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);
		C.r_dx10Texture		("s_base0",	r2_RT_generic);
		C.r_dx10Texture		("s_base1",	r2_RT_generic);
		C.r_dx10Texture		("s_noise", "fx\\fx_noise2");
		C.r_dx10Texture		("s_grad0", "$user$cmap0");
		C.r_dx10Texture		("s_grad1", "$user$cmap1");

		C.r_dx10Sampler		("smp_rtlinear");
		C.r_dx10Sampler		("smp_linear");
		C.r_End				();
		break;
	}
}

CBlender_ssfx_bloom_build::CBlender_ssfx_bloom_build() { description.CLS = 0; }

CBlender_ssfx_bloom_build::~CBlender_ssfx_bloom_build()
{
}

void CBlender_ssfx_bloom_build::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);

	switch (C.iElement)
	{
	case 0: // Bloom Build
	{
		C.r_Pass("stub_screen_space", "ssfx_bloom", FALSE, FALSE, FALSE);

		C.r_dx10Texture("s_position", r2_RT_P);
		C.r_dx10Texture("s_scene", r2_RT_blur_2);
		C.r_dx10Texture("s_emissive", r2_RT_ssfx_bloom_emissive);

		C.r_dx10Sampler("smp_linear");
		C.r_dx10Sampler("smp_rtlinear");
		C.r_dx10Sampler("smp_nofilter");

		C.r_End();
		break;
	}

	case 1: // Combine ( Finish )
		C.r_Pass("stub_screen_space", "ssfx_bloom_combine", FALSE, FALSE, FALSE);

		C.r_dx10Texture("s_bloom", r2_RT_ssfx_bloom_tmp2);
		C.r_dx10Texture("s_emissive", r2_RT_ssfx_bloom_emissive);

		C.r_dx10Texture("s_ssfx_lensdirt", "shaders\\lens_dirt");
		C.r_dx10Texture("s_starburst", "shaders\\starburst");
		C.r_dx10Texture("s_bloom_lens", r2_RT_ssfx_bloom_lens);
		C.r_dx10Texture("s_lenscolors", "shaders\\lens_colors");



		C.r_dx10Sampler("smp_point");
		C.r_dx10Sampler("smp_linear");
		C.r_dx10Sampler("smp_rtlinear");
		C.r_dx10Sampler("smp_nofilter");

		C.r_End();
		break;
	}
}

CBlender_ssfx_bloom_lens::CBlender_ssfx_bloom_lens() { description.CLS = 0; }

CBlender_ssfx_bloom_lens::~CBlender_ssfx_bloom_lens()
{
}

void CBlender_ssfx_bloom_lens::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);

	switch (C.iElement)
	{
	case 0: // Lens Build
		C.r_Pass("stub_screen_space", "ssfx_bloom_flares", FALSE, FALSE, FALSE);

		C.r_dx10Texture("s_emissive", r2_RT_ssfx_bloom_emissive);
		C.r_dx10Texture("s_lenscolors", "fx\\lens_colors");

		C.r_dx10Sampler("smp_linear");
		C.r_dx10Sampler("smp_rtlinear");
		C.r_dx10Sampler("smp_nofilter");

		C.r_End();
		break;

	case 1: // Flares Blur
		C.r_Pass("stub_screen_space", "ssfx_bloom_upsample", FALSE, FALSE, FALSE);

		C.r_dx10Texture("s_bloom", r2_RT_ssfx_bloom_tmp4);
		C.r_dx10Texture("s_downsample", r2_RT_ssfx_bloom_tmp4);

		C.r_dx10Sampler("smp_linear");
		C.r_dx10Sampler("smp_rtlinear");
		C.r_dx10Sampler("smp_nofilter");

		C.r_End();
		break;

	case 2: // Flares Blur
		C.r_Pass("stub_screen_space", "ssfx_bloom_upsample", FALSE, FALSE, FALSE);

		C.r_dx10Texture("s_bloom", r2_RT_ssfx_bloom_tmp4_2);
		C.r_dx10Texture("s_downsample", r2_RT_ssfx_bloom_tmp4_2);

		C.r_dx10Sampler("smp_linear");
		C.r_dx10Sampler("smp_rtlinear");
		C.r_dx10Sampler("smp_nofilter");

		C.r_End();
		break;
	}
}


CBlender_ssfx_bloom_downsample::CBlender_ssfx_bloom_downsample() { description.CLS = 0; }

CBlender_ssfx_bloom_downsample::~CBlender_ssfx_bloom_downsample()
{
}

void CBlender_ssfx_bloom_downsample::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);

	switch (C.iElement)
	{
	case 0:
		C.r_Pass("stub_screen_space", "ssfx_bloom_downsample", FALSE, FALSE, FALSE);

		C.r_dx10Texture("s_bloom", r2_RT_ssfx_bloom1);
		C.r_dx10Sampler("smp_linear");
		C.r_dx10Sampler("smp_rtlinear");
		C.r_dx10Sampler("smp_nofilter");
		C.r_End();
		break;
	case 1:
		C.r_Pass("stub_screen_space", "ssfx_bloom_downsample", FALSE, FALSE, FALSE);

		C.r_dx10Texture("s_bloom", r2_RT_ssfx_bloom_tmp2);
		C.r_dx10Sampler("smp_linear");
		C.r_dx10Sampler("smp_rtlinear");
		C.r_dx10Sampler("smp_nofilter");
		C.r_End();
		break;

	case 2:
		C.r_Pass("stub_screen_space", "ssfx_bloom_downsample", FALSE, FALSE, FALSE);

		C.r_dx10Texture("s_bloom", r2_RT_ssfx_bloom_tmp4);
		C.r_dx10Sampler("smp_linear");
		C.r_dx10Sampler("smp_rtlinear");
		C.r_End();
		break;
	case 3:
		C.r_Pass("stub_screen_space", "ssfx_bloom_downsample", FALSE, FALSE, FALSE);

		C.r_dx10Texture("s_bloom", r2_RT_ssfx_bloom_tmp8);
		C.r_dx10Sampler("smp_linear");
		C.r_dx10Sampler("smp_rtlinear");
		C.r_End();
		break;

	case 4:
		C.r_Pass("stub_screen_space", "ssfx_bloom_downsample", FALSE, FALSE, FALSE);

		C.r_dx10Texture("s_bloom", r2_RT_ssfx_bloom_tmp16);
		C.r_dx10Sampler("smp_linear");
		C.r_dx10Sampler("smp_rtlinear");
		C.r_End();
		break;

	case 5:
		C.r_Pass("stub_screen_space", "ssfx_bloom_downsample", FALSE, FALSE, FALSE);

		C.r_dx10Texture("s_bloom", r2_RT_ssfx_bloom_tmp32);
		C.r_dx10Sampler("smp_linear");
		C.r_dx10Sampler("smp_rtlinear");
		C.r_End();
		break;
	}
}


CBlender_ssfx_bloom_upsample::CBlender_ssfx_bloom_upsample() { description.CLS = 0; }

CBlender_ssfx_bloom_upsample::~CBlender_ssfx_bloom_upsample()
{
}

void CBlender_ssfx_bloom_upsample::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);

	switch (C.iElement)
	{
	case 0:
		C.r_Pass("stub_screen_space", "ssfx_bloom_upsample", FALSE, FALSE, FALSE);

		C.r_dx10Texture("s_downsample", r2_RT_ssfx_bloom_tmp64);
		C.r_dx10Texture("s_bloom", r2_RT_ssfx_bloom_tmp64);
		C.r_dx10Sampler("smp_linear");
		C.r_dx10Sampler("smp_rtlinear");
		C.r_End();
		break;

	case 1:
		C.r_Pass("stub_screen_space", "ssfx_bloom_upsample", FALSE, FALSE, FALSE);

		C.r_dx10Texture("s_downsample", r2_RT_ssfx_bloom_tmp32);
		C.r_dx10Texture("s_bloom", r2_RT_ssfx_bloom_tmp32_2);
		C.r_dx10Sampler("smp_linear");
		C.r_dx10Sampler("smp_rtlinear");
		C.r_End();
		break;

	case 2:
		C.r_Pass("stub_screen_space", "ssfx_bloom_upsample", FALSE, FALSE, FALSE);

		C.r_dx10Texture("s_downsample", r2_RT_ssfx_bloom_tmp16);
		C.r_dx10Texture("s_bloom", r2_RT_ssfx_bloom_tmp16_2);
		C.r_dx10Sampler("smp_linear");
		C.r_dx10Sampler("smp_rtlinear");
		C.r_End();
		break;

	case 3:
		C.r_Pass("stub_screen_space", "ssfx_bloom_upsample", FALSE, FALSE, FALSE);

		C.r_dx10Texture("s_downsample", r2_RT_ssfx_bloom_tmp8);
		C.r_dx10Texture("s_bloom", r2_RT_ssfx_bloom_tmp8_2);
		C.r_dx10Sampler("smp_linear");
		C.r_dx10Sampler("smp_rtlinear");
		C.r_End();
		break;

	case 4:
		C.r_Pass("stub_screen_space", "ssfx_bloom_upsample", FALSE, FALSE, FALSE);

		C.r_dx10Texture("s_downsample", r2_RT_ssfx_bloom_tmp4);
		C.r_dx10Texture("s_bloom", r2_RT_ssfx_bloom_tmp4_2);
		C.r_dx10Sampler("smp_linear");
		C.r_dx10Sampler("smp_rtlinear");
		C.r_End();
		break;
	}
}