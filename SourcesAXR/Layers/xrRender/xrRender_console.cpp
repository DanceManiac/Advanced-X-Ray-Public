#include	"stdafx.h"
#pragma		hdrstop

#include	"xrRender_console.h"
#include	"dxRenderDeviceRender.h"
#include	"../../xrEngine/x_ray.h"
#include "../../build_config_defines.h"

// SSR quality option
u32			dt_ssr_samp = 16;
xr_token							qdt_ssr_samp_token[] = {
	{ "dtssr_off",					0												},
	{ "dtssr_lowest",				1												},
	{ "dtssr_low",					2												},
	{ "dtssr_medium",				3												},
	{ "dtssr_high",					4												},
	{ 0,							0												}
};

// AA Modes
u32			r2_aa_mode = 1;
xr_token							r2_aa_mode_token[] = {
	{ "opt_noaa",					1												},
	{ "opt_fxaa",					2												},
	{ "opt_dlaa",					3												},
#ifdef USE_DX11
	{ "opt_smaa",					4												},
#endif
	{ 0,							0												}
};

u32 ps_r_aa_quality = 3;
xr_token qaa_token[] =
{
	{ "st_aa_off",					0 },
	{ "st_aa_low",					1 },
	{ "st_aa_medium",				2 },
	{ "st_aa_high",					3 },
	{ "st_aa_ultra",				4 },
	{ 0,							0 }
};

// SMAP Control

u32 ps_r2_smapsize = 2048;
xr_token qsmapsize_token[] =
{
    { "1024",						1024										},
    { "1536",						1536										},
    { "2048",						2048										},
    { "2560",						2560										},
    { "3072",						3072										},
    { "4096",						4096										},
   // { "8192",						8192										},
    { 0,							0											}
};

u32			ps_Preset				=	2	;
xr_token							qpreset_token							[ ]={
	{ "Minimum",					0											},
	{ "Low",						1											},
	{ "Default",					2											},
	{ "High",						3											},
	{ "Extreme",					4											},
	{ 0,							0											}
};

u32			ps_r_ssao_mode			=	2;
xr_token							qssao_mode_token						[ ]={
	{ "disabled",					0											},
	{ "default",					1											},
	{ "hdao",						2											},
	{ "hbao",						3											},
	{ "ssdo",						4											},
	{ 0,							0											}
};

u32			ps_r_sun_shafts				=	2;
xr_token							qsun_shafts_token							[ ]={
	{ "st_opt_off",					0												},
	{ "st_opt_low",					1												},
	{ "st_opt_medium",				2												},
	{ "st_opt_high",				3												},
	{ 0,							0												}
};

//ogse sunshafts
u32 ps_sunshafts_mode = 0;
xr_token sunshafts_mode_token[] = {
	{ "volumetric", 0 },
	{ "screen_space", 1 },
	{ "combine_sunshafts", 2 },
	{ NULL, 0 }
};

u32			ps_r_ssao				=	3;
xr_token							qssao_token									[ ]={
	{ "st_opt_off",					0												},
	{ "st_opt_low",					1												},
	{ "st_opt_medium",				2												},
	{ "st_opt_high",				3												},
#ifdef USE_DX11
	{ "st_opt_ultra",				4												},
#endif
	{ 0,							0												}
};

u32			ps_r_sun_quality		=	1;			//	=	0;
xr_token							qsun_quality_token							[ ]={
	{ "st_opt_low",					0												},
	{ "st_opt_medium",				1												},
	{ "st_opt_high",				2												},
#ifdef USE_DX11
	{ "st_opt_ultra",				3												},
	{ "st_opt_extreme",				4												},
#endif	//	USE_DX11
	{ 0,							0												}
};

u32			ps_r3_msaa				=	0;			//	=	0;
xr_token							qmsaa_token							[ ]={
	{ "st_opt_off",					0												},
	{ "2x",							1												},
	{ "4x",							2												},
	{ "8x",							3												},
	{ 0,							0												}
};

u32			ps_r3_msaa_atest		=	2;			//	=	0;
xr_token							qmsaa__atest_token					[ ]={
	{ "st_opt_off",					0												},
	{ "st_opt_atest_msaa_dx10_0",	1												},
	{ "st_opt_atest_msaa_dx10_1",	2												},
	{ 0,							0												}
};

u32			ps_r3_minmax_sm			=	3;			//	=	0;
xr_token							qminmax_sm_token					[ ]={
	{ "off",						0												},
	{ "on",							1												},
	{ "auto",						2												},
	{ "autodetect",					3												},
	{ 0,							0												}
};

//M.F.S. Team Color Drag Preset
u32 ps_clr_preset;
xr_vector <xr_token> qclrdrag_token;

u32			ps_r2_flares = 2;			//	=	0;
xr_token							qflares_token[] = {
	{ "st_opt_off",					0												},
	{ "st_opt_always",				1												},
	{ "st_opt_on_helmet",			2												},
	{ 0,							0												}
};

//Тип низинного тумана
u32 ps_lowland_fog_type = 0;
xr_token lowland_fog_type_token[] = {
	{ "st_gswr", 0 },
	{ "st_screen_space_shaders", 1 },
	{ 0, 0 }
};

//Пресет настроек для шейдеров
u32 ps_ShaderPreset = 0;
xr_token							qshader_preset_token[] = {
	{"original_shaders_preset",		0												},
	{"es_shaders_preset",			1												},
	{ 0,							0												}
};

//	“Off”
//	“DX10.0 style [Standard]”
//	“DX10.1 style [Higher quality]”

// Common
extern int			psSkeletonUpdate;
extern float		r__dtex_range;

//int		ps_r__Supersample			= 1		;
int			ps_r__LightSleepFrames		= 10	;

float		ps_r__Detail_l_ambient		= 0.9f	;
float		ps_r__Detail_l_aniso		= 0.25f	;
float		ps_r__Detail_density		= 0.3f	;
float		ps_r__Detail_rainbow_hemi	= 0.75f	;

float		ps_r__Tree_w_rot			= 10.0f	;
float		ps_r__Tree_w_speed			= 1.00f	;
float		ps_r__Tree_w_amp			= 0.005f;
Fvector		ps_r__Tree_Wave				= {.1f, .01f, .11f};
float		ps_r__Tree_SBC				= 1.5f	;	// scale bias correct

float		ps_r__WallmarkTTL			= 50.f	;
float		ps_r__WallmarkSHIFT			= 0.0001f;
float		ps_r__WallmarkSHIFT_V		= 0.0001f;

float		ps_r__GLOD_ssa_start		= 256.f	;
float		ps_r__GLOD_ssa_end			=  64.f	;
float		ps_r__LOD					=  0.75f	;
//. float		ps_r__LOD_Power				=  1.5f	;
float		ps_r__ssaDISCARD			=  3.5f	;					//RO
float		ps_r__ssaDONTSORT			=  32.f	;					//RO
float		ps_r__ssaHZBvsTEX			=  96.f	;					//RO

int			ps_r__tf_Anisotropic		= 8		;

// R1
float		ps_r1_ssaLOD_A				= 64.f	;
float		ps_r1_ssaLOD_B				= 48.f	;
float		ps_r1_tf_Mipbias			= 0.0f	;
Flags32		ps_r1_flags					= { R1FLAG_DLIGHTS };		// r1-only
float		ps_r1_lmodel_lerp			= 0.1f	;
float		ps_r1_dlights_clip			= 40.f	;
float		ps_r1_pps_u					= 0.f	;
float		ps_r1_pps_v					= 0.f	;

// R1-specific
int			ps_r1_GlowsPerFrame			= 16	;					// r1-only
float		ps_r1_fog_luminance			= 1.1f	;					// r1-only
int			ps_r1_SoftwareSkinning		= 0		;					// r1-only

// R2
float		ps_r2_ssaLOD_A				= 64.f	;
float		ps_r2_ssaLOD_B				= 48.f	;
float		ps_r2_tf_Mipbias			= 0.0f	;

// R2-specific
Flags32		ps_r2_ls_flags				= { R2FLAG_SUN 
	//| R2FLAG_SUN_IGNORE_PORTALS
	| R2FLAG_EXP_DONT_TEST_UNSHADOWED 
	| R2FLAG_USE_NVSTENCIL | R2FLAG_EXP_SPLIT_SCENE 
	| R2FLAG_EXP_MT_CALC | R3FLAG_DYN_WET_SURF
	| R3FLAG_VOLUMETRIC_SMOKE
	//| R3FLAG_MSAA 
	| R3FLAG_MSAA_OPT
	| R3FLAG_GBUFFER_OPT
	|R2FLAG_DETAIL_BUMP
	|R2FLAG_DOF
	|R2FLAG_SOFT_PARTICLES
	|R2FLAG_SOFT_WATER
	|R2FLAG_STEEP_PARALLAX
	|R2FLAG_SUN_FOCUS
	|R2FLAG_SUN_TSM
	|R2FLAG_TONEMAP
	|R2FLAG_VOLUMETRIC_LIGHTS
	};	// r2-only

Flags32		ps_r2_ls_flags_ext			= {
		/*R2FLAGEXT_SSAO_OPT_DATA |*/ R2FLAGEXT_SSAO_HALF_DATA
		|R2FLAGEXT_ENABLE_TESSELLATION
	};

int			ps_no_scale_on_fade			= 0;
float		ps_r2_df_parallax_h			= 0.02f;
float		ps_r2_df_parallax_range		= 75.f;

float		ps_r2_tonemap_middlegray	= READ_IF_EXISTS(pAdvancedSettings, r_float, "start_settings", "tonemap_middlegray", 0.95f);
float		ps_r2_tonemap_adaptation	= READ_IF_EXISTS(pAdvancedSettings, r_float, "start_settings", "tonemap_adaptation", 1.f);
float		ps_r2_tonemap_low_lum		= READ_IF_EXISTS(pAdvancedSettings, r_float, "start_settings", "tonemap_low_lum", 0.0035f);
float		ps_r2_tonemap_amount		= READ_IF_EXISTS(pAdvancedSettings, r_float, "start_settings", "tonemap_amount", 0.7f);

float		ps_r2_ls_bloom_kernel_g		= READ_IF_EXISTS(pAdvancedSettings, r_float, "start_settings", "bloom_kernel_g", 3.0f);
float		ps_r2_ls_bloom_kernel_b		= READ_IF_EXISTS(pAdvancedSettings, r_float, "start_settings", "bloom_kernel_b", 0.7f);
float		ps_r2_ls_bloom_speed		= READ_IF_EXISTS(pAdvancedSettings, r_float, "start_settings", "bloom_speed", 100.0f);
float		ps_r2_ls_bloom_kernel_scale = READ_IF_EXISTS(pAdvancedSettings, r_float, "start_settings", "bloom_kernel_scale", 0.7f);
float		ps_r2_ls_bloom_threshold	= READ_IF_EXISTS(pAdvancedSettings, r_float, "start_settings", "bloom_threshold", 0.00001f);

float		ps_r2_ls_dsm_kernel			= .7f;				// r2-only
float		ps_r2_ls_psm_kernel			= .7f;				// r2-only
float		ps_r2_ls_ssm_kernel			= .7f;				// r2-only
Fvector		ps_r2_aa_barier				= { .8f, .1f, 0};	// r2-only
Fvector		ps_r2_aa_weight				= { .25f,.25f,0};	// r2-only
float		ps_r2_aa_kernel				= .5f;				// r2-only
float		ps_r2_mblur					= .1f;				// .5f
int			ps_r2_GI_depth				= 1;				// 1..5
int			ps_r2_GI_photons			= 16;				// 8..64
float		ps_r2_GI_clip				= EPS_L;			// EPS
float		ps_r2_GI_refl				= .9f;				// .9f
float		ps_r2_ls_depth_scale		= 1.00001f;			// 1.00001f
float		ps_r2_ls_depth_bias			= -0.0003f;			// -0.0001f
float		ps_r2_ls_squality			= 1.0f;				// 1.00f
float		ps_r2_sun_tsm_projection	= 0.3f;			// 0.18f
float		ps_r2_sun_tsm_bias			= -0.01f;			// 
float		ps_r2_sun_near				= 20.f;				// 12.0f

extern float OLES_SUN_LIMIT_27_01_07;	//	actually sun_far

float		ps_r2_sun_near_border		= 0.75f;			// 1.0f
float		ps_r2_sun_depth_far_scale	= 1.00000f;			// 1.00001f
float		ps_r2_sun_depth_far_bias	= -0.00002f;			// -0.0000f
float		ps_r2_sun_depth_near_scale	= 1.0000f;			// 1.00001f
float		ps_r2_sun_depth_near_bias	= 0.00001f;		// -0.00005f
float		ps_r2_sun_lumscale			= READ_IF_EXISTS(pAdvancedSettings, r_float, "start_settings", "sun_lumscale", 1.0f);
float		ps_r2_sun_lumscale_hemi		= READ_IF_EXISTS(pAdvancedSettings, r_float, "start_settings", "sun_lumscale_hemi", 1.0f);
float		ps_r2_sun_lumscale_amb		= READ_IF_EXISTS(pAdvancedSettings, r_float, "start_settings", "sun_lumscale_amb", 1.0f);
float		ps_r2_gmaterial				= 2.2f;				// 
float		ps_r2_zfill					= 0.25f;				// .1f

float		ps_r2_dhemi_sky_scale		= 0.08f;				// 1.5f
float		ps_r2_dhemi_light_scale     = 0.2f	;
float		ps_r2_dhemi_light_flow      = 0.1f	;
int			ps_r2_dhemi_count			= 5;				// 5
int			ps_r2_wait_sleep			= 0;

float		ps_r2_lt_smooth				= 1.f;				// 1.f
float		ps_r2_slight_fade			= 0.5f;				// 1.f

//	x - min (0), y - focus (1.4), z - max (100)
Fvector3	ps_r2_dof					= Fvector3().set(-1.25f, 1.4f, 10000.f);
float		ps_r2_dof_sky				= 30;				//	distance to sky
float		ps_r2_dof_kernel_size		= 5.0f;						//	7.0f

extern		ENGINE_API float ps_r3_dyn_wet_surf_near;
extern		ENGINE_API float ps_r3_dyn_wet_surf_far;
extern		ENGINE_API int ps_r3_dyn_wet_surf_sm_res;

int			ps_r__detail_radius = 49;
u32			dm_size = 24;
u32 		dm_cache1_line = 12;	//dm_size*2/dm_cache1_count
u32			dm_cache_line = 49;	//dm_size+1+dm_size
u32			dm_cache_size = 2401;	//dm_cache_line*dm_cache_line
float		dm_fade = 47.5;	//float(2*dm_size)-.5f;
u32			dm_current_size = 24;
u32 		dm_current_cache1_line = 12;	//dm_current_size*2/dm_cache1_count
u32			dm_current_cache_line = 49;	//dm_current_size+1+dm_current_size
u32			dm_current_cache_size = 2401;	//dm_current_cache_line*dm_current_cache_line
float		dm_current_fade = 47.5;	//float(2*dm_current_size)-.5f;
float		ps_current_detail_density = 0.6;
float		ps_current_detail_scale = 1.f;

//ogse sunshafts
float		ps_r2_ss_sunshafts_length = 1.f;
float		ps_r2_ss_sunshafts_radius = 1.f;
float		droplets_power_debug = 0.f;

Fvector4	ps_color_grading = { 1.0f, 1.0f, 1.0f, 0.0f };

float		ps_r2_rain_drops_intensity = 0.00025f;
float		ps_r2_rain_drops_speed = 1.25f;

Flags32		ps_actor_shadow_flags = { 0 };

Flags32		ps_r2_postscreen_flags = { R_FLAG_HUD_MASK
	| R_FLAG_HUD_DYN_EFFECTS
	| R2FLAG_RAIN_DROPS
	| R_FLAG_CHROMATIC_ABERRATION
};

Flags32		ps_r_textures_flags = { R3_NO_RAM_TEXTURES };

int ps_force_enable_lens_flares = 0;

float ps_r2_gloss_factor = READ_IF_EXISTS(pAdvancedSettings, r_float, "start_settings", "gloss_factor", 10.0f);
float ps_r2_gloss_min = READ_IF_EXISTS(pAdvancedSettings, r_float, "start_settings", "gloss_min", 0.0f);

Fvector4 ps_pp_bloom_thresh = { .7, .8f, .9f, .0f };
Fvector4 ps_pp_bloom_weight = { .33f, .33f, .33f, .0f };

//debug
Fvector4 ps_dev_param_1 = { .0f, .0f, .0f, .0f };
Fvector4 ps_dev_param_2 = { .0f, .0f, .0f, .0f };
Fvector4 ps_dev_param_3 = { .0f, .0f, .0f, .0f };
Fvector4 ps_dev_param_4 = { .0f, .0f, .0f, .0f };
Fvector4 ps_dev_param_5 = { .0f, .0f, .0f, .0f };
Fvector4 ps_dev_param_6 = { .0f, .0f, .0f, .0f };
Fvector4 ps_dev_param_7 = { .0f, .0f, .0f, .0f };
Fvector4 ps_dev_param_8 = { .0f, .0f, .0f, .0f };

//Geometry optimization from Anomaly
int opt_static = 0;
int opt_dynamic = 0;

//SFZ Lens Flares
int ps_r2_lfx = 1;

//Многопоточная загрузка текстур
int ps_mt_texture_load = 1;

//AO Debug
int ps_r2_ao_debug = 0;

float ps_r2_reflections_distance = 300.0f;

Flags32 psDeviceFlags2 = { 0 };

//Static on R2+
Flags32	ps_r2_static_flags = { R2FLAG_USE_BUMP };

Flags32	ps_r4_shaders_flags = { R4FLAG_SSS_ADDON | R4FLAG_ES_ADDON };

//Screen Space Shaders Stuff

// Anomaly
float ps_r2_img_exposure = 1.0f;
float ps_r2_img_gamma = 1.0f;
float ps_r2_img_saturation = 1.0f;
Fvector ps_r2_img_cg = READ_IF_EXISTS(pAdvancedSettings, r_fvector3, "start_settings", "color_grading_es", Fvector3().set(0.5f, 0.5f, 0.5f));

// Ascii1457's Screen Space Shaders
extern ENGINE_API Fvector3 ps_ssfx_shadow_cascades;
extern ENGINE_API Fvector4 ps_ssfx_grass_shadows;
extern ENGINE_API Fvector4 ps_ssfx_grass_interactive;
extern ENGINE_API Fvector4 ps_ssfx_int_grass_params_1;
extern ENGINE_API Fvector4 ps_ssfx_int_grass_params_2;
extern ENGINE_API Fvector4 ps_ssfx_hud_drops_1;
extern ENGINE_API Fvector4 ps_ssfx_hud_drops_2;
extern ENGINE_API Fvector4 ps_ssfx_hud_drops_1_cfg;
extern ENGINE_API Fvector4 ps_ssfx_hud_drops_2_cfg;
extern ENGINE_API Fvector4 ps_ssfx_blood_decals;
extern ENGINE_API Fvector4 ps_ssfx_rain_1;
extern ENGINE_API Fvector4 ps_ssfx_rain_2;
extern ENGINE_API Fvector4 ps_ssfx_rain_3;
extern ENGINE_API Fvector4 ps_ssfx_florafixes_1;
extern ENGINE_API Fvector4 ps_ssfx_florafixes_2;
extern ENGINE_API Fvector4 ps_ssfx_wetsurfaces_1;
extern ENGINE_API Fvector4 ps_ssfx_wetsurfaces_2;
extern ENGINE_API int	   ps_ssfx_is_underground;
extern ENGINE_API int	   ps_ssfx_gloss_method;
extern ENGINE_API float	   ps_ssfx_gloss_factor;
extern ENGINE_API Fvector3 ps_ssfx_gloss_minmax;
extern ENGINE_API Fvector4 ps_ssfx_lightsetup_1;
extern ENGINE_API Fvector3 ps_ssfx_shadows;
extern ENGINE_API Fvector3 ps_ssfx_volumetric;
extern ENGINE_API Fvector3 ps_ssfx_shadow_bias;
extern ENGINE_API Fvector4 ps_ssfx_lut;
extern ENGINE_API Fvector4 ps_ssfx_wind_grass;
extern ENGINE_API Fvector4 ps_ssfx_wind_trees;

int ps_r4_ss_grass_collision = ps_r4_shaders_flags.test(R4FLAG_SSS_ADDON) ? 1 : 0;
int ps_r4_pseudo_pbr = 0;

//extern ENGINE_API Fvector4 ps_ssfx_wpn_dof_1;
//extern ENGINE_API float ps_ssfx_wpn_dof_2;

//Fvector4 ps_ssfx_wpn_dof_1 = { .0f, .0f, .0f, .0f };
//extern float ps_ssfx_wpn_dof_2 = 1.0f;

#ifndef _EDITOR
#include	"../../xrEngine/xr_ioconsole.h"
#include	"../../xrEngine/xr_ioc_cmd.h"

#ifdef USE_DX11
#include "../xrRenderDX10/StateManager/dx10SamplerStateCache.h"
#endif	//	USE_DX11

//-----------------------------------------------------------------------

class CCC_ssfx_cascades : public CCC_Vector3
{
public:
	void apply()
	{
		// TODO: Crash here when quitting game
#if defined(USE_DX11)
		RImplementation.init_cascades();
#endif
	}

	CCC_ssfx_cascades(LPCSTR N, Fvector3* V, const Fvector3 _min, const Fvector3 _max) : CCC_Vector3(N, V, _min, _max)
	{
	};

	virtual void Execute(LPCSTR args)
	{
		CCC_Vector3::Execute(args);
		apply();
	}

	virtual void GetStatus(TStatus& S)
	{
		CCC_Vector3::Status(S);
		apply();
	}
};

class CCC_detail_radius : public CCC_Integer
{
public:
	void	apply()
	{
		dm_current_size = iFloor((float)ps_r__detail_radius / 4) * 2;
		dm_current_cache1_line = dm_current_size * 2 / 4;		// assuming cache1_count = 4
		dm_current_cache_line = dm_current_size + 1 + dm_current_size;
		dm_current_cache_size = dm_current_cache_line * dm_current_cache_line;
		dm_current_fade = float(2 * dm_current_size) - .5f;
	}
	CCC_detail_radius(LPCSTR N, int* V, int _min = 0, int _max = 999) : CCC_Integer(N, V, _min, _max)
	{};
	virtual void Execute(LPCSTR args)
	{
		CCC_Integer::Execute(args);
		apply();
	}
	virtual void	Status(TStatus& S)
	{
		CCC_Integer::Status(S);
	}
};

class CCC_tf_Aniso		: public CCC_Integer
{
public:
	void	apply	()	{
		if (0==HW.pDevice)	return	;
		int	val = *value;	clamp(val,1,16);
#ifdef USE_DX11
		SSManager.SetMaxAnisotropy(val);
#else	//	USE_DX11
		for (u32 i=0; i<HW.Caps.raster.dwStages; i++)
			CHK_DX(HW.pDevice->SetSamplerState( i, D3DSAMP_MAXANISOTROPY, val	));
#endif	//	USE_DX11
	}
	CCC_tf_Aniso(LPCSTR N, int*	v) : CCC_Integer(N, v, 1, 16)		{ };
	virtual void Execute	(LPCSTR args)
	{
		CCC_Integer::Execute	(args);
		apply					();
	}
	virtual void	Status	(TStatus& S)
	{	
		CCC_Integer::Status		(S);
		apply					();
	}
};
class CCC_tf_MipBias: public CCC_Float
{
public:
	void	apply	()	{
		if (0==HW.pDevice)	return	;

#ifdef USE_DX11
		//	TODO: DX10: Implement mip bias control
		//VERIFY(!"apply not implmemented.");
#else	//	USE_DX11
		for (u32 i=0; i<HW.Caps.raster.dwStages; i++)
			CHK_DX(HW.pDevice->SetSamplerState( i, D3DSAMP_MIPMAPLODBIAS, *((LPDWORD) value)));
#endif	//	USE_DX11
	}

	CCC_tf_MipBias(LPCSTR N, float*	v) : CCC_Float(N, v, -0.5f, +0.5f)	{ };
	virtual void Execute(LPCSTR args)
	{
		CCC_Float::Execute	(args);
		apply				();
	}
	virtual void	Status	(TStatus& S)
	{	
		CCC_Float::Status	(S);
		apply				();
	}
};
class CCC_R2GM		: public CCC_Float
{
public:
	CCC_R2GM(LPCSTR N, float*	v) : CCC_Float(N, v, 0.f, 4.f) { *v = 0; };
	virtual void	Execute	(LPCSTR args)
	{
		if (0==xr_strcmp(args,"on"))	{
			ps_r2_ls_flags.set	(R2FLAG_GLOBALMATERIAL,TRUE);
		} else if (0==xr_strcmp(args,"off"))	{
			ps_r2_ls_flags.set	(R2FLAG_GLOBALMATERIAL,FALSE);
		} else {
			CCC_Float::Execute	(args);
			if (ps_r2_ls_flags.test(R2FLAG_GLOBALMATERIAL))	{
				static LPCSTR	name[4]	=	{ "oren", "blin", "phong", "metal" };
				float	mid		= *value	;
				int		m0		= iFloor(mid)	% 4;
				int		m1		= (m0+1)		% 4;
				float	frc		= mid - float(iFloor(mid));
				Msg		("* material set to [%s]-[%s], with lerp of [%f]",name[m0],name[m1],frc);
			}
		}
	}
};
class CCC_Screenshot : public IConsole_Command
{
public:
	CCC_Screenshot(LPCSTR N) : IConsole_Command(N)  { };
	virtual void Execute(LPCSTR args) {
		if (g_dedicated_server)
			return;

		string_path	name;	name[0]=0;
		sscanf		(args,"%s",	name);
		LPCSTR		image	= xr_strlen(name)?name:0;
		::Render->Screenshot(IRender_interface::SM_NORMAL,image);
	}
};

class CCC_RestoreQuadIBData : public IConsole_Command
{
public:
	CCC_RestoreQuadIBData(LPCSTR N) : IConsole_Command(N)  { };
	virtual void Execute(LPCSTR args) {
		RCache.RestoreQuadIBData();
	}
};

class CCC_ModelPoolStat : public IConsole_Command
{
public:
	CCC_ModelPoolStat(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = TRUE; };
	virtual void Execute(LPCSTR args) {
		RImplementation.Models->dump();
	}
};

class	CCC_SSAO_Mode		: public CCC_Token
{
public:
	CCC_SSAO_Mode(LPCSTR N, u32* V, xr_token* T) : CCC_Token(N,V,T)	{}	;

	virtual void	Execute	(LPCSTR args)	{
		CCC_Token::Execute	(args);
				
		switch	(*value)
		{
			case 0:
			{
				ps_r_ssao = 0;
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_HBAO, 0);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_HDAO, 0);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_SSDO, 0);
				break;
			}
			case 1:
			{
				if (ps_r_ssao==0)
				{
					ps_r_ssao = 1;
				}
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_HBAO, 0);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_HDAO, 0);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_HALF_DATA, 0);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_SSDO, 0);
				break;
			}
			case 2:
			{
				if (ps_r_ssao==0)
				{
					ps_r_ssao = 1;
				}
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_HBAO, 0);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_HDAO, 1);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_OPT_DATA, 0);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_HALF_DATA, 0);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_SSDO, 0);
				break;
			}
			case 3:
			{
				if (ps_r_ssao == 0)
				{
					ps_r_ssao = 1;
				}
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_HBAO, 1);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_HDAO, 0);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_OPT_DATA, 1);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_SSDO, 0);
				break;
			}
			case 4:
			{
				if (ps_r_ssao == 0)
				{
					ps_r_ssao = 1;
				}
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_HBAO, 0);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_HDAO, 0);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_OPT_DATA, 1);
				ps_r2_ls_flags_ext.set(R2FLAGEXT_SSAO_SSDO, 1);
				break;
			}
		}
	}
};

//-----------------------------------------------------------------------
class	CCC_Preset		: public CCC_Token
{
public:
	CCC_Preset(LPCSTR N, u32* V, xr_token* T) : CCC_Token(N,V,T)	{}	;

	virtual void	Execute	(LPCSTR args)	{
		CCC_Token::Execute	(args);
		string_path		_cfg;
		string_path		cmd;
		
		switch	(*value)	{
			case 0:		xr_strcpy(_cfg, "rspec_minimum.ltx");	break;
			case 1:		xr_strcpy(_cfg, "rspec_low.ltx");		break;
			case 2:		xr_strcpy(_cfg, "rspec_default.ltx");	break;
			case 3:		xr_strcpy(_cfg, "rspec_high.ltx");		break;
			case 4:		xr_strcpy(_cfg, "rspec_extreme.ltx");	break;
		}
		FS.update_path			(_cfg,"$game_config$",_cfg);
		strconcat				(sizeof(cmd),cmd,"cfg_load", " ", _cfg);
		Console->Execute		(cmd);
	}
};

//M.F.S. Team Color Drag Preset------------------------------------------
class	CCC_ps_clr_preset		: public CCC_Token
{
public:
	CCC_ps_clr_preset(LPCSTR N, u32* V, xr_token* T) : CCC_Token(N,V,T)	{}	;

	virtual void	Execute	(LPCSTR args)
	{
		CCC_Token::Execute	(args);
		string256		file_name;
		string_path		_cfg = "mfs_team\\color_drag_settings\\";
		string_path		cmd;

		strconcat(sizeof(file_name), file_name, args, ".ltx");
		strconcat(sizeof(_cfg), _cfg, _cfg, file_name);

		FS.update_path			(_cfg,"$game_config$", _cfg);
		strconcat				(sizeof(cmd),cmd,"cfg_load", " ", _cfg);
		Console->Execute		(cmd);
	}
};


class CCC_memory_stats : public IConsole_Command
{
protected	:

public		:

	CCC_memory_stats(LPCSTR N) :	IConsole_Command(N)	{ bEmptyArgsHandled = true; };

	virtual void	Execute	(LPCSTR args)
	{
		u32 m_base = 0;
		u32 c_base = 0;
		u32 m_lmaps = 0; 
		u32 c_lmaps = 0;

		dxRenderDeviceRender::Instance().ResourcesGetMemoryUsage( m_base, c_base, m_lmaps, c_lmaps );

		Msg		("memory usage  mb \t \t video    \t managed      \t system \n" );

		float vb_video		= (float)HW.stats_manager.memory_usage_summary[enum_stats_buffer_type_vertex][D3DPOOL_DEFAULT]/1024/1024;
		float vb_managed	= (float)HW.stats_manager.memory_usage_summary[enum_stats_buffer_type_vertex][D3DPOOL_MANAGED]/1024/1024;
		float vb_system		= (float)HW.stats_manager.memory_usage_summary[enum_stats_buffer_type_vertex][D3DPOOL_SYSTEMMEM]/1024/1024;
		Msg		("vertex buffer      \t \t %f \t %f \t %f ",	vb_video, vb_managed, vb_system);

		float ib_video		= (float)HW.stats_manager.memory_usage_summary[enum_stats_buffer_type_index][D3DPOOL_DEFAULT]/1024/1024; 
		float ib_managed	= (float)HW.stats_manager.memory_usage_summary[enum_stats_buffer_type_index][D3DPOOL_MANAGED]/1024/1024; 
		float ib_system		= (float)HW.stats_manager.memory_usage_summary[enum_stats_buffer_type_index][D3DPOOL_SYSTEMMEM]/1024/1024; 
		Msg		("index buffer      \t \t %f \t %f \t %f ",	ib_video, ib_managed, ib_system);
		
		float textures_managed = (float)(m_base+m_lmaps)/1024/1024;
		Msg		("textures          \t \t %f \t %f \t %f ",	0.f, textures_managed, 0.f);

		float rt_video		= (float)HW.stats_manager.memory_usage_summary[enum_stats_buffer_type_rtarget][D3DPOOL_DEFAULT]/1024/1024;
		float rt_managed	= (float)HW.stats_manager.memory_usage_summary[enum_stats_buffer_type_rtarget][D3DPOOL_MANAGED]/1024/1024;
		float rt_system		= (float)HW.stats_manager.memory_usage_summary[enum_stats_buffer_type_rtarget][D3DPOOL_SYSTEMMEM]/1024/1024;
		Msg		("R-Targets         \t \t %f \t %f \t %f ",	rt_video, rt_managed, rt_system);									

		Msg		("\nTotal             \t \t %f \t %f \t %f ",	vb_video+ib_video+rt_video,
																textures_managed + vb_managed+ib_managed+rt_managed,
																vb_system+ib_system+rt_system);
	}

};


#if RENDER!=R_R1
#include "r__pixel_calculator.h"
class CCC_BuildSSA : public IConsole_Command
{
public:
	CCC_BuildSSA(LPCSTR N) : IConsole_Command(N)  { bEmptyArgsHandled = TRUE; };
	virtual void Execute(LPCSTR args) 
	{
#ifndef USE_DX11
		//	TODO: DX10: Implement pixel calculator
		r_pixel_calculator	c;
		c.run				();
#endif	//	USE_DX11
	}
};
#endif

class CCC_DofFar : public CCC_Float
{
public:
	CCC_DofFar(LPCSTR N, float* V, float _min=0.0f, float _max=10000.0f) 
		: CCC_Float( N, V, _min, _max){}

	virtual void Execute(LPCSTR args) 
	{
		float v = float(atof(args));

		if (v<ps_r2_dof.y+0.1f)
		{
			char	pBuf[256];
			_snprintf( pBuf, sizeof(pBuf)/sizeof(pBuf[0]), "float value greater or equal to r2_dof_focus+0.1");
			Msg("~ Invalid syntax in call to '%s'",cName);
			Msg("~ Valid arguments: %s", pBuf);
			Console->Execute("r2_dof_focus");
		}
		else
		{
			CCC_Float::Execute(args);
			if(g_pGamePersistent)
				g_pGamePersistent->SetBaseDof(ps_r2_dof);
		}
	}

	//	CCC_Dof should save all data as well as load from config
	virtual void	Save	(IWriter *F)	{;}
};

class CCC_DofNear : public CCC_Float
{
public:
	CCC_DofNear(LPCSTR N, float* V, float _min=0.0f, float _max=10000.0f) 
		: CCC_Float( N, V, _min, _max){}

	virtual void Execute(LPCSTR args) 
	{
		float v = float(atof(args));

		if (v>ps_r2_dof.y-0.1f)
		{
			char	pBuf[256];
			_snprintf( pBuf, sizeof(pBuf)/sizeof(pBuf[0]), "float value less or equal to r2_dof_focus-0.1");
			Msg("~ Invalid syntax in call to '%s'",cName);
			Msg("~ Valid arguments: %s", pBuf);
			Console->Execute("r2_dof_focus");
		}
		else
		{
			CCC_Float::Execute(args);
			if(g_pGamePersistent)
				g_pGamePersistent->SetBaseDof(ps_r2_dof);
		}
	}

	//	CCC_Dof should save all data as well as load from config
	virtual void	Save	(IWriter *F)	{;}
};

class CCC_DofFocus : public CCC_Float
{
public:
	CCC_DofFocus(LPCSTR N, float* V, float _min=0.0f, float _max=10000.0f) 
		: CCC_Float( N, V, _min, _max){}

	virtual void Execute(LPCSTR args) 
	{
		float v = float(atof(args));

		if (v>ps_r2_dof.z-0.1f)
		{
			char	pBuf[256];
			_snprintf( pBuf, sizeof(pBuf)/sizeof(pBuf[0]), "float value less or equal to r2_dof_far-0.1");
			Msg("~ Invalid syntax in call to '%s'",cName);
			Msg("~ Valid arguments: %s", pBuf);
			Console->Execute("r2_dof_far");
		}
		else if (v<ps_r2_dof.x+0.1f)
		{
			char	pBuf[256];
			_snprintf( pBuf, sizeof(pBuf)/sizeof(pBuf[0]), "float value greater or equal to r2_dof_far-0.1");
			Msg("~ Invalid syntax in call to '%s'",cName);
			Msg("~ Valid arguments: %s", pBuf);
			Console->Execute("r2_dof_near");
		}
		else{
			CCC_Float::Execute(args);
			if(g_pGamePersistent)
				g_pGamePersistent->SetBaseDof(ps_r2_dof);
			}
	}

	//	CCC_Dof should save all data as well as load from config
	virtual void	Save	(IWriter *F)	{;}
};

class CCC_Dof : public CCC_Vector3
{
public:
	CCC_Dof(LPCSTR N, Fvector* V, const Fvector _min, const Fvector _max) : 
	  CCC_Vector3(N, V, _min, _max) {;}

	virtual void	Execute	(LPCSTR args)
	{
		Fvector v;
		if (3!=sscanf(args,"%f,%f,%f",&v.x,&v.y,&v.z))	
			InvalidSyntax(); 
		else if ( (v.x > v.y-0.1f) || (v.z < v.y+0.1f))
		{
			InvalidSyntax();
			Msg("x <= y - 0.1");
			Msg("y <= z - 0.1");
		}
		else
		{
			CCC_Vector3::Execute(args);
			if(g_pGamePersistent)
				g_pGamePersistent->SetBaseDof(ps_r2_dof);
		}
	}
	virtual void	Status	(TStatus& S)
	{	
		xr_sprintf	(S,"%f,%f,%f",value->x,value->y,value->z);
	}
	virtual void	Info	(TInfo& I)
	{	
		xr_sprintf(I,"vector3 in range [%f,%f,%f]-[%f,%f,%f]",min.x,min.y,min.z,max.x,max.y,max.z);
	}

};

class CCC_DumpResources : public IConsole_Command
{
public:
	CCC_DumpResources(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = TRUE; };
	virtual void Execute(LPCSTR args) {
		RImplementation.Models->dump();
		dxRenderDeviceRender::Instance().Resources->Dump(false);
	}
};

//	Allow real-time fog config reload
#if	(RENDER == R_R4)
#ifdef	DEBUG

#include "../xrRenderDX10/3DFluid/dx103DFluidManager.h"

class CCC_Fog_Reload : public IConsole_Command
{
public:
	CCC_Fog_Reload(LPCSTR N) : IConsole_Command(N) { bEmptyArgsHandled = TRUE; };
	virtual void Execute(LPCSTR args) 
	{
		FluidManager.UpdateProfiles();
	}
};
#endif	//	DEBUG
#endif	//	(RENDER == R_R4)

static void LoadTokensFromIni(xr_vector<xr_token>& tokens, LPCSTR section)
{
	tokens.clear();

	for (auto& item : pAdvancedSettings->r_section(section).Data)
	{
		tokens.push_back({item.first.c_str(), atoi(item.second.c_str())});
	}
	tokens.push_back({nullptr, 0});
}

class CCC_Shader_Preset : public CCC_Token
{
public:
	CCC_Shader_Preset(LPCSTR N, u32* V, xr_token* T) : CCC_Token(N, V, T) {};

	virtual void Execute(LPCSTR args)
	{
		CCC_Token::Execute(args);
		string_path _cfg;
		string_path cmd;

		switch (*value)
		{
		case 0: xr_strcpy(_cfg, "mfs_team\\shaders_presets\\shaders_original_preset.ltx"); break;
		case 1: xr_strcpy(_cfg, "mfs_team\\shaders_presets\\shaders_enchanted_preset.ltx"); break;
		}
		FS.update_path(_cfg, "$game_config$", _cfg);
		strconcat(sizeof(cmd), cmd, "cfg_load", " ", _cfg);
		Console->Execute(cmd);
	}
};

//-----------------------------------------------------------------------
void		xrRender_initconsole	()
{
	CMD3(CCC_Preset,	"_preset",				&ps_Preset,	qpreset_token	);

	LoadTokensFromIni(qclrdrag_token, "color_drag_presets");
	CMD3(CCC_ps_clr_preset,	"r2_clr_preset", &ps_clr_preset, qclrdrag_token.data());

	CMD4(CCC_Integer,	"rs_skeleton_update",	&psSkeletonUpdate,	2,		128	);
#ifdef	DEBUG
	CMD1(CCC_DumpResources,		"dump_resources");
#endif	//	 DEBUG

	CMD4(CCC_Float,		"r__dtex_range",		&r__dtex_range,		5,		175	);

// Common
	CMD1(CCC_Screenshot,"screenshot"			);

	//	Igor: just to test bug with rain/particles corruption
	CMD1(CCC_RestoreQuadIBData,	"r_restore_quad_ib_data");
#ifdef DEBUG
#if RENDER!=R_R1
	CMD1(CCC_BuildSSA,	"build_ssa"				);
#endif
	CMD4(CCC_Integer,	"r__lsleep_frames",		&ps_r__LightSleepFrames,	4,		30		);
	CMD4(CCC_Float,		"r__ssa_glod_start",	&ps_r__GLOD_ssa_start,		128,	512		);
	CMD4(CCC_Float,		"r__ssa_glod_end",		&ps_r__GLOD_ssa_end,		16,		96		);
	CMD4(CCC_Float,		"r__wallmark_shift_pp",	&ps_r__WallmarkSHIFT,		0.0f,	1.f		);
	CMD4(CCC_Float,		"r__wallmark_shift_v",	&ps_r__WallmarkSHIFT_V,		0.0f,	1.f		);
	CMD1(CCC_ModelPoolStat,"stat_models"		);
#endif // DEBUG
	CMD4(CCC_Float,		"r__wallmark_ttl",		&ps_r__WallmarkTTL,			1.0f,	5.f*60.f);

	CMD4(CCC_Integer,	"r__supersample",		&ps_r__Supersample,			1,		8		);

	Fvector	tw_min,tw_max;
	
	CMD4(CCC_Float,		"r__geometry_lod",		&ps_r__LOD,					0.1f,   3.f		); //AVO: extended from 1.2f to 3.f
//.	CMD4(CCC_Float,		"r__geometry_lod_pow",	&ps_r__LOD_Power,			0,		2		);

//.	CMD4(CCC_Float,		"r__detail_density",	&ps_r__Detail_density,		.05f,	0.99f	);
	CMD4(CCC_Float,		"r__detail_density",	&ps_current_detail_density, 0.3f, 1.5f		);
	CMD4(CCC_Float,		"r__detail_scale",		&ps_current_detail_scale,	0.2f, 1.6f		);

#ifdef DEBUG
	CMD4(CCC_Float,		"r__detail_l_ambient",	&ps_r__Detail_l_ambient,	.5f,	.95f	);
	CMD4(CCC_Float,		"r__detail_l_aniso",	&ps_r__Detail_l_aniso,		.1f,	.5f		);

	CMD4(CCC_Float,		"r__d_tree_w_amp",		&ps_r__Tree_w_amp,			.001f,	1.f		);
	CMD4(CCC_Float,		"r__d_tree_w_rot",		&ps_r__Tree_w_rot,			.01f,	100.f	);
	CMD4(CCC_Float,		"r__d_tree_w_speed",	&ps_r__Tree_w_speed,		1.0f,	10.f	);

	tw_min.set			(EPS,EPS,EPS);
	tw_max.set			(2,2,2);
	CMD4(CCC_Vector3,	"r__d_tree_wave",		&ps_r__Tree_Wave,			tw_min, tw_max	);
#endif // DEBUG

	CMD2(CCC_tf_Aniso,	"r__tf_aniso",			&ps_r__tf_Anisotropic		); //	{1..16}

	// R1
	CMD4(CCC_Float,		"r1_ssa_lod_a",			&ps_r1_ssaLOD_A,			16,		96		);
	CMD4(CCC_Float,		"r1_ssa_lod_b",			&ps_r1_ssaLOD_B,			16,		64		);
	CMD4(CCC_Float,		"r1_lmodel_lerp",		&ps_r1_lmodel_lerp,			0,		0.333f	);
	CMD2(CCC_tf_MipBias,"r1_tf_mipbias",		&ps_r1_tf_Mipbias			);//	{-3 +3}
	CMD3(CCC_Mask,		"r1_dlights",			&ps_r1_flags,				R1FLAG_DLIGHTS	);
	CMD4(CCC_Float,		"r1_dlights_clip",		&ps_r1_dlights_clip,		10.f,	150.f	);
	CMD4(CCC_Float,		"r1_pps_u",				&ps_r1_pps_u,				-1.f,	+1.f	);
	CMD4(CCC_Float,		"r1_pps_v",				&ps_r1_pps_v,				-1.f,	+1.f	);


	// R1-specific
	CMD4(CCC_Integer,	"r1_glows_per_frame",	&ps_r1_GlowsPerFrame,		2,		32		);
	CMD3(CCC_Mask,		"r1_detail_textures",	&ps_r2_ls_flags,			R1FLAG_DETAIL_TEXTURES);

	CMD4(CCC_Float,		"r1_fog_luminance",		&ps_r1_fog_luminance,		0.2f,	5.f	);

	// Software Skinning
	// 0 - disabled (renderer can override)
	// 1 - enabled
	// 2 - forced hardware skinning (renderer can not override)
	CMD4(CCC_Integer,	"r1_software_skinning",	&ps_r1_SoftwareSkinning,	0,		2	);

	// R2
	CMD4(CCC_Float,		"r2_ssa_lod_a",			&ps_r2_ssaLOD_A,			16,		96		);
	CMD4(CCC_Float,		"r2_ssa_lod_b",			&ps_r2_ssaLOD_B,			32,		64		);
	CMD2(CCC_tf_MipBias,"r2_tf_mipbias",		&ps_r2_tf_Mipbias			);

	// R2-specific
	CMD2(CCC_R2GM,		"r2em",					&ps_r2_gmaterial							);
	CMD3(CCC_Mask,		"r2_tonemap",			&ps_r2_ls_flags,			R2FLAG_TONEMAP	);
	CMD4(CCC_Float,		"r2_tonemap_middlegray",&ps_r2_tonemap_middlegray,	0.0f,	2.0f	);
	CMD4(CCC_Float,		"r2_tonemap_adaptation",&ps_r2_tonemap_adaptation,	0.01f,	10.0f	);
	CMD4(CCC_Float,		"r2_tonemap_lowlum",	&ps_r2_tonemap_low_lum,		0.0001f,1.0f	);
	CMD4(CCC_Float,		"r2_tonemap_amount",	&ps_r2_tonemap_amount,		0.0000f,1.0f	);
	CMD4(CCC_Float,		"r2_ls_bloom_kernel_scale",&ps_r2_ls_bloom_kernel_scale,	0.5f,	2.f);
	CMD4(CCC_Float,		"r2_ls_bloom_kernel_g",	&ps_r2_ls_bloom_kernel_g,	1.f,	7.f		);
	CMD4(CCC_Float,		"r2_ls_bloom_kernel_b",	&ps_r2_ls_bloom_kernel_b,	0.01f,	1.f		);
	CMD4(CCC_Float,		"r2_ls_bloom_threshold",&ps_r2_ls_bloom_threshold,	0.f,	1.f		);
	CMD4(CCC_Float,		"r2_ls_bloom_speed",	&ps_r2_ls_bloom_speed,		0.f,	100.f	);
	CMD3(CCC_Mask,		"r2_ls_bloom_fast",		&ps_r2_ls_flags,			R2FLAG_FASTBLOOM);
	CMD4(CCC_Float,		"r2_ls_dsm_kernel",		&ps_r2_ls_dsm_kernel,		.1f,	3.f		);
	CMD4(CCC_Float,		"r2_ls_psm_kernel",		&ps_r2_ls_psm_kernel,		.1f,	3.f		);
	CMD4(CCC_Float,		"r2_ls_ssm_kernel",		&ps_r2_ls_ssm_kernel,		.1f,	3.f		);
	CMD4(CCC_Float,		"r2_ls_squality",		&ps_r2_ls_squality,			.5f,	3.f		);

	CMD3(CCC_Mask,		"r2_zfill",				&ps_r2_ls_flags,			R2FLAG_ZFILL	);
	CMD4(CCC_Float,		"r2_zfill_depth",		&ps_r2_zfill,				.001f,	.5f		);
	CMD3(CCC_Mask,		"r2_allow_r1_lights",	&ps_r2_ls_flags,			R2FLAG_R1LIGHTS	);

	CMD3(CCC_Mask,		"r__actor_shadow",		&ps_actor_shadow_flags,		RFLAG_ACTOR_SHADOW);  //Swartz
    CMD3(CCC_Token, 	"r2_smap_size", 		&ps_r2_smapsize, 			qsmapsize_token	);
	
	CMD4(CCC_Float,		"r_color_r",			&ps_color_grading.x,		0.0f,	2.55f	);
	CMD4(CCC_Float,		"r_color_g",			&ps_color_grading.y,		0.0f,	2.55f	);
	CMD4(CCC_Float,		"r_color_b",			&ps_color_grading.z,		0.0f,	2.55f	);
	CMD4(CCC_Float,		"r_saturation",			&ps_color_grading.w,		-1.0f,	+1.0f	);

	Fvector4 clr_drag_min = { 0.f, 0.f, 0.f, -1.0f };
	Fvector4 clr_drag_max = { 2.55f, 2.55f, 2.55f, 1.f };
	CMD4(CCC_Vector4,	"r_color_grading",		&ps_color_grading,			clr_drag_min, clr_drag_max);

	//tw_min.set(0, 0, 0);
	//tw_max.set(1, 1, 1);

	//CMD4(CCC_Vector3,	"r_color_grading_es",	&ps_r2_img_cg,				tw_min, tw_max);

	CMD3(CCC_Mask,		"r2_raindrops",			&ps_r2_postscreen_flags,	R2FLAG_RAIN_DROPS	);
	CMD4(CCC_Float,		"r2_rain_drops_intensity",	&ps_r2_rain_drops_intensity, 0.f,	1.f	);
	CMD4(CCC_Float,		"r2_rain_drops_speed",	&ps_r2_rain_drops_speed, 	0.8f,	5.f		);
	// Vignette
	CMD3(CCC_Mask,		"r2_vignette",			&ps_r2_postscreen_flags,	R_FLAG_VIGNETTE);
	// No Ram Textures
	CMD3(CCC_Mask,		"r3_no_ram_textures",	&ps_r_textures_flags,		R3_NO_RAM_TEXTURES);
	// Hud Mask
	CMD3(CCC_Mask,		"r2_hud_mask",			&ps_r2_postscreen_flags,	R_FLAG_HUD_MASK);
	CMD3(CCC_Mask,		"r2_hud_dyn_effects",	&ps_r2_postscreen_flags,	R_FLAG_HUD_DYN_EFFECTS);
	//Chromatic Aberration
	CMD3(CCC_Mask,		"r4_chromatic_aberration", &ps_r2_postscreen_flags,	R_FLAG_CHROMATIC_ABERRATION);
	//Film Grain
	CMD3(CCC_Mask,		"r4_film_grain",		&ps_r2_postscreen_flags,	R_FLAG_FILM_GRAIN);

	CMD4(CCC_Float,		"r2_gloss_factor",		&ps_r2_gloss_factor,		.0f,	50.f	);
	CMD4(CCC_Float,		"r2_gloss_min",			&ps_r2_gloss_min,			.001f,	1.0f	);

#ifdef DEBUG
	CMD3(CCC_Mask,		"r2_use_nvdbt",			&ps_r2_ls_flags,			R2FLAG_USE_NVDBT);
	CMD3(CCC_Mask,		"r2_mt",				&ps_r2_ls_flags,			R2FLAG_EXP_MT_CALC);
#endif // DEBUG
	
	CMD3(CCC_Mask,		"r2_sun",				&ps_r2_ls_flags,			R2FLAG_SUN		);
	CMD3(CCC_Mask,		"r2_sun_details",		&ps_r2_ls_flags,			R2FLAG_SUN_DETAILS);
	CMD3(CCC_Mask,		"r2_sun_focus",			&ps_r2_ls_flags,			R2FLAG_SUN_FOCUS);
//	CMD3(CCC_Mask,		"r2_sun_static",		&ps_r2_ls_flags,			R2FLAG_SUN_STATIC);
//	CMD3(CCC_Mask,		"r2_exp_splitscene",	&ps_r2_ls_flags,			R2FLAG_EXP_SPLIT_SCENE);
//	CMD3(CCC_Mask,		"r2_exp_donttest_uns",	&ps_r2_ls_flags,			R2FLAG_EXP_DONT_TEST_UNSHADOWED);
	CMD3(CCC_Mask,		"r2_exp_donttest_shad",	&ps_r2_ls_flags,			R2FLAG_EXP_DONT_TEST_SHADOWED);
	
	CMD3(CCC_Mask,		"r2_sun_tsm",			&ps_r2_ls_flags,			R2FLAG_SUN_TSM	);
	CMD4(CCC_Float,		"r2_sun_tsm_proj",		&ps_r2_sun_tsm_projection,	.001f,	0.8f	);
	CMD4(CCC_Float,		"r2_sun_tsm_bias",		&ps_r2_sun_tsm_bias,		-0.5,	+0.5	);
	CMD4(CCC_Float,		"r2_sun_near",			&ps_r2_sun_near,			1.f,	100.f	);
#if RENDER!=R_R1
	CMD4(CCC_Float,		"r2_sun_far",			&OLES_SUN_LIMIT_27_01_07,	51.f,	180.f	);
#endif
	CMD4(CCC_Float,		"r2_sun_near_border",	&ps_r2_sun_near_border,		.5f,	1.0f	);
	CMD4(CCC_Float,		"r2_sun_depth_far_scale",&ps_r2_sun_depth_far_scale,0.5,	1.5		);
	CMD4(CCC_Float,		"r2_sun_depth_far_bias",&ps_r2_sun_depth_far_bias,	-0.5,	+0.5	);
	CMD4(CCC_Float,		"r2_sun_depth_near_scale",&ps_r2_sun_depth_near_scale,0.5,	1.5		);
	CMD4(CCC_Float,		"r2_sun_depth_near_bias",&ps_r2_sun_depth_near_bias,-0.5,	+0.5	);
	CMD4(CCC_Float,		"r2_sun_lumscale",		&ps_r2_sun_lumscale,		-1.0,	+3.0	);
	CMD4(CCC_Float,		"r2_sun_lumscale_hemi",	&ps_r2_sun_lumscale_hemi,	0.0,	+3.0	);
	CMD4(CCC_Float,		"r2_sun_lumscale_amb",	&ps_r2_sun_lumscale_amb,	0.0,	+3.0	);

	CMD3(CCC_Mask,		"r2_aa",				&ps_r2_ls_flags,			R2FLAG_AA);
	CMD4(CCC_Float,		"r2_aa_kernel",			&ps_r2_aa_kernel,			0.3f,	0.7f	);
	CMD3(CCC_Mask,		"r2_mblur_enable",		&ps_r2_ls_flags,			R2FLAG_MBLUR	);
	CMD4(CCC_Float,		"r2_mblur",				&ps_r2_mblur,				0.05f, 0.25f	);

	CMD3(CCC_Mask,		"r2_gi",				&ps_r2_ls_flags,			R2FLAG_GI);
	CMD4(CCC_Float,		"r2_gi_clip",			&ps_r2_GI_clip,				EPS,	0.1f	);
	CMD4(CCC_Integer,	"r2_gi_depth",			&ps_r2_GI_depth,			1,		5		);
	CMD4(CCC_Integer,	"r2_gi_photons",		&ps_r2_GI_photons,			8,		256		);
	CMD4(CCC_Float,		"r2_gi_refl",			&ps_r2_GI_refl,				EPS_L,	0.99f	);

	CMD4(CCC_Integer,	"r2_wait_sleep",		&ps_r2_wait_sleep,			0,		1		);

#ifndef MASTER_GOLD
	CMD4(CCC_Integer,	"r2_dhemi_count",		&ps_r2_dhemi_count,			4,		25		);
	CMD4(CCC_Float,		"r2_dhemi_sky_scale",	&ps_r2_dhemi_sky_scale,		0.0f,	100.f	);
	CMD4(CCC_Float,		"r2_dhemi_light_scale",	&ps_r2_dhemi_light_scale,	0,		100.f	);
	CMD4(CCC_Float,		"r2_dhemi_light_flow",	&ps_r2_dhemi_light_flow,	0,		1.f	);
	CMD4(CCC_Float,		"r2_dhemi_smooth",		&ps_r2_lt_smooth,			0.f,	10.f	);
	CMD3(CCC_Mask,		"rs_hom_depth_draw",	&ps_r2_ls_flags_ext,		R_FLAGEXT_HOM_DEPTH_DRAW);
	CMD3(CCC_Mask,		"r2_shadow_cascede_zcul",&ps_r2_ls_flags_ext,		R2FLAGEXT_SUN_ZCULLING);
#endif // DEBUG

	CMD3(CCC_Mask,		"r2_shadow_cascede_old", &ps_r2_ls_flags_ext,		R2FLAGEXT_SUN_OLD);


	CMD4(CCC_Float,		"r2_ls_depth_scale",	&ps_r2_ls_depth_scale,		0.5,	1.5		);
	CMD4(CCC_Float,		"r2_ls_depth_bias",		&ps_r2_ls_depth_bias,		-0.5,	+0.5	);

	CMD4(CCC_Float,		"r2_parallax_h",		&ps_r2_df_parallax_h,		.0f,	.5f		);
//	CMD4(CCC_Float,		"r2_parallax_range",	&ps_r2_df_parallax_range,	5.0f,	175.0f	);

	CMD4(CCC_Float,		"r2_slight_fade",		&ps_r2_slight_fade,			.2f,	1.f		);

	tw_min.set			(0,0,0);	tw_max.set	(1,1,1);
	CMD4(CCC_Vector3,	"r2_aa_break",			&ps_r2_aa_barier,			tw_min, tw_max	);

	tw_min.set			(0,0,0);	tw_max.set	(1,1,1);
	CMD4(CCC_Vector3,	"r2_aa_weight",			&ps_r2_aa_weight,			tw_min, tw_max	);

	//	Igor: Depth of field
	tw_min.set			(-10000,-10000,0);	tw_max.set	(10000,10000,10000);
	CMD4( CCC_Dof,		"r2_dof",		&ps_r2_dof, tw_min, tw_max);
	CMD4( CCC_DofNear,	"r2_dof_near",	&ps_r2_dof.x, tw_min.x, tw_max.x);
	CMD4( CCC_DofFocus,	"r2_dof_focus", &ps_r2_dof.y, tw_min.y, tw_max.y);
	CMD4( CCC_DofFar,	"r2_dof_far",	&ps_r2_dof.z, tw_min.z, tw_max.z);

	CMD4(CCC_Float,		"r2_dof_kernel",&ps_r2_dof_kernel_size,				.0f,	10.f);
	CMD4(CCC_Float,		"r2_dof_sky",	&ps_r2_dof_sky,						-10000.f,	10000.f);
	CMD3(CCC_Mask,		"r2_dof_enable",&ps_r2_ls_flags,	R2FLAG_DOF);
	
//	float		ps_r2_dof_near			= 0.f;					// 0.f
//	float		ps_r2_dof_focus			= 1.4f;					// 1.4f

	//ogse sunshafts
	CMD3(CCC_Token,		"r2_sunshafts_mode",			&ps_sunshafts_mode, sunshafts_mode_token);
	CMD4(CCC_Float,		"r2_ss_sunshafts_length",		&ps_r2_ss_sunshafts_length, .2f, 1.5f);
	CMD4(CCC_Float,		"r2_ss_sunshafts_radius",		&ps_r2_ss_sunshafts_radius, .5f, 2.f);
	//end ogse sunshafts 
	
	CMD3(CCC_Mask,		"r2_volumetric_lights",			&ps_r2_ls_flags,			R2FLAG_VOLUMETRIC_LIGHTS);
//	CMD3(CCC_Mask,		"r2_sun_shafts",				&ps_r2_ls_flags,			R2FLAG_SUN_SHAFTS);
	CMD3(CCC_Token,		"r2_sun_shafts",				&ps_r_sun_shafts,			qsun_shafts_token);
	CMD3(CCC_SSAO_Mode,	"r2_ssao_mode",					&ps_r_ssao_mode,			qssao_mode_token);
	CMD3(CCC_Token,		"r2_ssao",						&ps_r_ssao,					qssao_token);
	CMD3(CCC_Mask,		"r2_ssao_blur",                 &ps_r2_ls_flags_ext,		R2FLAGEXT_SSAO_BLUR);//Need restart
	CMD3(CCC_Mask,		"r2_ssao_opt_data",				&ps_r2_ls_flags_ext,		R2FLAGEXT_SSAO_OPT_DATA);//Need restart
	CMD3(CCC_Mask,		"r2_ssao_half_data",			&ps_r2_ls_flags_ext,		R2FLAGEXT_SSAO_HALF_DATA);//Need restart
	CMD3(CCC_Mask,		"r2_ssao_hbao",					&ps_r2_ls_flags_ext,		R2FLAGEXT_SSAO_HBAO);//Need restart
	CMD3(CCC_Mask,		"r2_ssao_hdao",					&ps_r2_ls_flags_ext,		R2FLAGEXT_SSAO_HDAO);//Need restart
	CMD3(CCC_Mask,		"r2_ssao_ssdo",					&ps_r2_ls_flags_ext,		R2FLAGEXT_SSAO_SSDO);//Need restart
	CMD3(CCC_Mask,		"r4_enable_tessellation",		&ps_r2_ls_flags_ext,		R2FLAGEXT_ENABLE_TESSELLATION);//Need restart
	CMD3(CCC_Mask,		"r4_wireframe",					&ps_r2_ls_flags_ext,		R2FLAGEXT_WIREFRAME);//Need restart
	CMD3(CCC_Mask,		"r2_steep_parallax",			&ps_r2_ls_flags,			R2FLAG_STEEP_PARALLAX);
	CMD3(CCC_Mask,		"r2_detail_bump",				&ps_r2_ls_flags,			R2FLAG_DETAIL_BUMP);

	//Static on R2+
	CMD3(CCC_Mask,		"r2_use_bump",					&ps_r2_static_flags,		R2FLAG_USE_BUMP);//Need restart
	CMD3(CCC_Mask,		"r2_static_sun",				&ps_r2_static_flags,		R2FLAG_STATIC_SUN);//Need restart

	CMD3(CCC_Token,		"r2_sun_quality",				&ps_r_sun_quality,			qsun_quality_token);

	// DWM: DT SSR quality option
	CMD3(CCC_Token,		"r4_ssr_samples",				&dt_ssr_samp,				qdt_ssr_samp_token);

	// AA Mode
	CMD3(CCC_Token,		"r2_aa_mode",					&r2_aa_mode,				r2_aa_mode_token);

	//Refactor
	Fvector4 twb_min = { 0.f, 0.f, 0.f, 0.f };
	Fvector4 twb_max = { 1.f, 1.f, 1.f, 1.f };
	CMD4(CCC_Vector4,	"r__bloom_weight",				&ps_pp_bloom_weight,		twb_min, twb_max);
	CMD4(CCC_Vector4,	"r__bloom_thresh",				&ps_pp_bloom_thresh,		twb_min, twb_max);

	Fvector4 tw2_min = { -100.f, -100.f, -100.f, -100.f };
	Fvector4 tw2_max = { 100.f, 100.f, 100.f, 100.f };

	if (bDeveloperMode)
	{
		// SMAA Quality
		CMD3(CCC_Token,		"r2_smaa_quality",				&ps_r_aa_quality,			qaa_token);
		//Shader param stuff
		CMD4(CCC_Vector4,	"shader_param_1",				&ps_dev_param_1,			tw2_min, tw2_max);
		CMD4(CCC_Vector4,	"shader_param_2",				&ps_dev_param_2,			tw2_min, tw2_max);
		CMD4(CCC_Vector4,	"shader_param_3",				&ps_dev_param_3,			tw2_min, tw2_max);
		CMD4(CCC_Vector4,	"shader_param_4",				&ps_dev_param_4,			tw2_min, tw2_max);
		CMD4(CCC_Vector4,	"shader_param_5",				&ps_dev_param_5,			tw2_min, tw2_max);
		CMD4(CCC_Vector4,	"shader_param_6",				&ps_dev_param_6,			tw2_min, tw2_max);
		CMD4(CCC_Vector4,	"shader_param_7",				&ps_dev_param_7,			tw2_min, tw2_max);
		CMD4(CCC_Vector4,	"shader_param_8",				&ps_dev_param_8,			tw2_min, tw2_max);

		//AO Debug
		CMD4(CCC_Integer,	"r2_ao_gebug",					&ps_r2_ao_debug,			0, 1);
	}

	//	Igor: need restart
	CMD3(CCC_Mask,		"r2_soft_water",				&ps_r2_ls_flags,			R2FLAG_SOFT_WATER);
	CMD3(CCC_Mask,		"r2_soft_particles",			&ps_r2_ls_flags,			R2FLAG_SOFT_PARTICLES);

	//CMD3(CCC_Mask,		"r3_msaa",						&ps_r2_ls_flags,			R3FLAG_MSAA);
	CMD3(CCC_Token,		"r3_msaa",						&ps_r3_msaa,				qmsaa_token);
	//CMD3(CCC_Mask,		"r3_msaa_hybrid",				&ps_r2_ls_flags,			R3FLAG_MSAA_HYBRID);
	//CMD3(CCC_Mask,		"r3_msaa_opt",					&ps_r2_ls_flags,			R3FLAG_MSAA_OPT);
	CMD3(CCC_Mask,		"r3_gbuffer_opt",				&ps_r2_ls_flags,			R3FLAG_GBUFFER_OPT);
	//CMD3(CCC_Token,		"r3_msaa_alphatest",			&ps_r3_msaa_atest,			qmsaa__atest_token);
	CMD3(CCC_Token,		"r3_minmax_sm",					&ps_r3_minmax_sm,			qminmax_sm_token);
	CMD4(CCC_detail_radius, "r__detail_radius",			&ps_r__detail_radius,		49, 300				);
	CMD4(CCC_Integer, "r__no_scale_on_fade",			&ps_no_scale_on_fade,		0, 1				); //Alundaio

	//	Allow real-time fog config reload
#if	(RENDER == R_R4)
#ifdef	DEBUG
	CMD1(CCC_Fog_Reload,"r3_fog_reload");
#endif	//	DEBUG
#endif	//	(RENDER == R_R4)

	CMD3(CCC_Mask,		"r3_dynamic_wet_surfaces",		&ps_r2_ls_flags,			R3FLAG_DYN_WET_SURF);
	CMD4(CCC_Float,		"r3_dynamic_wet_surfaces_near",	&ps_r3_dyn_wet_surf_near,	10,	70		);
	CMD4(CCC_Float,		"r3_dynamic_wet_surfaces_far",	&ps_r3_dyn_wet_surf_far,	30,	300		);
	CMD4(CCC_Integer,	"r3_dynamic_wet_surfaces_sm_res",&ps_r3_dyn_wet_surf_sm_res,64,	2048	);

	CMD3(CCC_Mask,			"r3_volumetric_smoke",			&ps_r2_ls_flags,			R3FLAG_VOLUMETRIC_SMOKE);
	CMD1(CCC_memory_stats,	"render_memory_stats" );
	
	// Geometry optimization
	CMD4(CCC_Integer,		"r__optimize_static_geom",		&opt_static,				0,	4		);
	CMD4(CCC_Integer,		"r__optimize_dynamic_geom",		&opt_dynamic,				0,	4		);
	psDeviceFlags2.set(rsOptShadowGeom, TRUE);
	CMD3(CCC_Mask,			"r__optimize_shadow_geom",		&psDeviceFlags2,			rsOptShadowGeom);

	CMD3(CCC_Token,			"r2_use_flares",				&ps_r2_flares,				qflares_token);
	CMD4(CCC_Integer,		"r2_lfx",						&ps_r2_lfx,					 0, 1		); //SFZ Lens Flares


	CMD4(CCC_Integer,		"r__mt_textures_load",			&ps_mt_texture_load,		0, 1); //Многопоточная загрузка текстур
	CMD3(CCC_Token,			"r3_lowland_fog_type",			&ps_lowland_fog_type,		lowland_fog_type_token); //Тип низинного тумана

	CMD4(CCC_Float,			"r3_reflections_dist",			&ps_r2_reflections_distance, 100.f, 1000.f); //Дальность отражений

	// Anomaly
	CMD4(CCC_Float,			"r__exposure",					&ps_r2_img_exposure,		0.5f, 4.0f);
	CMD4(CCC_Float,			"r__gamma",						&ps_r2_img_gamma,			0.5f, 2.2f);
	//CMD4(CCC_Float,			"r__saturation",				&ps_r2_img_saturation,		0.0f, 2.0f);

	//tw_min.set(0, 0, 0);
	//tw_max.set(1, 1, 1);

	//CMD4(CCC_Vector3,	"r_color_grading_es",	&ps_r2_img_cg,				tw_min, tw_max);

    // Screen Space Shaders
	CMD3(CCC_Mask,			"r4_enchanted_shaders",			&ps_r4_shaders_flags,		R4FLAG_ES_ADDON); //Need restart
	CMD3(CCC_Mask,			"r4_screen_space_shaders",		&ps_r4_shaders_flags,		R4FLAG_SSS_ADDON); //Need restart
	CMD3(CCC_Mask,			"r4_ss_sky_debanding",			&ps_r4_shaders_flags,		R4FLAG_SS_DEBANDING); //Need restart
	CMD3(CCC_Mask,			"r4_ss_flora_fix",				&ps_r4_shaders_flags,		R4FLAG_SS_FLORAFIX); //Need restart
	CMD3(CCC_Mask,			"r4_ss_fog",					&ps_r4_shaders_flags,		R4FLAG_SS_FOG); //Need restart
	CMD3(CCC_Mask,			"r4_ss_indirect_light",			&ps_r4_shaders_flags,		R4FLAG_SS_INDIRECT_LIGHT); //Need restart
	CMD3(CCC_Mask,			"r4_ss_new_gloss",				&ps_r4_shaders_flags,		R4FLAG_SS_NEW_GLOSS); //Need restart
	CMD3(CCC_Mask,			"r4_screen_space_shadows",		&ps_r4_shaders_flags,		R4FLAG_SS_SSS); //Need restart
	CMD3(CCC_Mask,			"r4_ss_shadows",				&ps_r4_shaders_flags,		R4FLAG_SS_SHADOWS); //Need restart
	CMD3(CCC_Mask,			"r4_ss_lut",					&ps_r4_shaders_flags,		R4FLAG_SS_LUT); //Need restart
	CMD3(CCC_Mask,			"r4_ss_wind",					&ps_r4_shaders_flags,		R4FLAG_SS_WIND); //Need restart
	CMD3(CCC_Shader_Preset, "shaders_preset",				&ps_ShaderPreset,			qshader_preset_token);

	CMD4(CCC_Vector4,		"ssfx_wpn_dof_1",				&ps_ssfx_wpn_dof_1,			tw2_min, tw2_max);
	CMD4(CCC_Float,			"ssfx_wpn_dof_2",				&ps_ssfx_wpn_dof_2,			0.0f, 1.0f);
    CMD4(CCC_Vector4,		"ssfx_grass_shadows",			&ps_ssfx_grass_shadows,		Fvector4().set(0, 0, 0, 0), Fvector4().set(3, 1, 100, 100));
	CMD4(CCC_Float,			"r_grass_shadows_dintance",		&ps_ssfx_grass_shadows.y,	0.01f, 1.0f);
    CMD4(CCC_ssfx_cascades, "ssfx_shadow_cascades",			&ps_ssfx_shadow_cascades,	Fvector3().set(1.0f, 1.0f, 1.0f), Fvector3().set(300, 300, 300));
    CMD4(CCC_Vector4,		"ssfx_grass_interactive",		&ps_ssfx_grass_interactive, Fvector4().set(0, 0, 0, 0), Fvector4().set(1, 15, 5000, 1));
    CMD4(CCC_Vector4,		"ssfx_int_grass_params_1",		&ps_ssfx_int_grass_params_1, Fvector4().set(0, 0, 0, 0), Fvector4().set(5, 5, 5, 60));
    CMD4(CCC_Vector4,		"ssfx_int_grass_params_2",		&ps_ssfx_int_grass_params_2, Fvector4().set(0, 0, 0, 0), Fvector4().set(5, 20, 1, 5));
	CMD4(CCC_Vector4,		"ssfx_hud_drops_1",				&ps_ssfx_hud_drops_1,		Fvector4().set(0, 0, 0, 0), Fvector4().set(100000, 100, 100, 100));
    CMD4(CCC_Vector4,		"ssfx_hud_drops_2",				&ps_ssfx_hud_drops_2,		Fvector4().set(0, 0, 0, 0), tw2_max);
    CMD4(CCC_Vector4,		"ssfx_blood_decals",			&ps_ssfx_blood_decals,		Fvector4().set(0, 0, 0, 0), Fvector4().set(5, 5, 0, 0));
    CMD4(CCC_Vector4,		"ssfx_rain_1",					&ps_ssfx_rain_1,			Fvector4().set(0, 0, 0, 0), Fvector4().set(10, 5, 5, 2));
    CMD4(CCC_Vector4,		"ssfx_rain_2",					&ps_ssfx_rain_2,			Fvector4().set(0, 0, 0, 0), Fvector4().set(1, 10, 10, 10));
    CMD4(CCC_Vector4,		"ssfx_rain_3",					&ps_ssfx_rain_3,			Fvector4().set(0, 0, 0, 0), Fvector4().set(1, 10, 10, 10));
	CMD4(CCC_Vector4,		"ssfx_florafixes_1",			&ps_ssfx_florafixes_1,		Fvector4().set(0.0, 0.0, 0.0, 0.0), Fvector4().set(1.0, 1.0, 1.0, 1.0));
	CMD4(CCC_Vector4,		"ssfx_florafixes_2",			&ps_ssfx_florafixes_2,		Fvector4().set(0.0, 0.0, 0.0, 0.0), Fvector4().set(10.0, 1.0, 1.0, 1.0));
	CMD4(CCC_Vector4,		"ssfx_wetsurfaces_1",			&ps_ssfx_wetsurfaces_1,		Fvector4().set(0.01, 0.01, 0.01, 0.01), Fvector4().set(2.0, 2.0, 2.0, 2.0));
	CMD4(CCC_Vector4,		"ssfx_wetsurfaces_2",			&ps_ssfx_wetsurfaces_2,		Fvector4().set(0.01, 0.01, 0.01, 0.01), Fvector4().set(2.0, 2.0, 2.0, 2.0));
	CMD4(CCC_Integer,		"ssfx_is_underground",			&ps_ssfx_is_underground,	0, 1);
	CMD4(CCC_Integer,		"ssfx_gloss_method",			&ps_ssfx_gloss_method,		0, 1);
	CMD4(CCC_Vector3,		"ssfx_gloss_minmax",			&ps_ssfx_gloss_minmax,		Fvector3().set(0, 0, 0), Fvector3().set(1.0, 1.0, 1.0));
	CMD4(CCC_Float,			"ssfx_gloss_factor",			&ps_ssfx_gloss_factor,		0.0f, 1.0f);
	CMD4(CCC_Vector4,		"ssfx_lightsetup_1",			&ps_ssfx_lightsetup_1,		Fvector4().set(0, 0, 0, 0), Fvector4().set(1.0, 1.0, 1.0, 1.0));
	CMD4(CCC_Vector3,		"ssfx_shadows",					&ps_ssfx_shadows,			Fvector3().set(128, 1536, 0), Fvector3().set(1536, 4096, 0));
	CMD4(CCC_Vector3,		"ssfx_volumetric",				&ps_ssfx_volumetric,		Fvector3().set(0, 0, 1.0), Fvector3().set(1.0, 1.0, 5.0));
	CMD4(CCC_Vector3,		"ssfx_shadow_bias",				&ps_ssfx_shadow_bias,		Fvector3().set(0, 0, 0), Fvector3().set(1.0, 1.0, 1.0));
	CMD4(CCC_Vector4,		"ssfx_lut",						&ps_ssfx_lut,				Fvector4().set(0.0, 0.0, 0.0, 0.0), tw2_max);
	CMD4(CCC_Vector4,		"ssfx_wind_grass",				&ps_ssfx_wind_grass,		Fvector4().set(0.0, 0.0, 0.0, 0.0), Fvector4().set(20.0, 5.0, 5.0, 5.0));
	CMD4(CCC_Vector4,		"ssfx_wind_trees",				&ps_ssfx_wind_trees,		Fvector4().set(0.0, 0.0, 0.0, 0.0), Fvector4().set(20.0, 5.0, 5.0, 1.0));

	CMD4(CCC_Integer,		"r4_ss_grass_collision",		&ps_r4_ss_grass_collision,	0, 1); //Screen Space Grass Shaders Collision
	CMD4(CCC_Integer,		"r4_es_pseudo_pbr",				&ps_r4_pseudo_pbr,			0, 1); //Enchanted Shaders Pseudo PBR
//	CMD3(CCC_Mask,		"r2_sun_ignore_portals",		&ps_r2_ls_flags,			R2FLAG_SUN_IGNORE_PORTALS);
}

void	xrRender_apply_tf		()
{
	Console->Execute	("r__tf_aniso"	);
#if RENDER==R_R1
	Console->Execute	("r1_tf_mipbias");
#else
	Console->Execute	("r2_tf_mipbias");
#endif
}

#endif
