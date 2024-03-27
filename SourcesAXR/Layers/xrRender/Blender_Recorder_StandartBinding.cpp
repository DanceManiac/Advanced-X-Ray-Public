#include "stdafx.h"
#pragma hdrstop

#pragma warning(push)
#pragma warning(disable:4995)
#include <d3dx9.h>
#pragma warning(pop)

#include "ResourceManager.h"
#include "blenders\Blender_Recorder.h"
#include "blenders\Blender.h"

#include "../../xrEngine/igame_persistent.h"
#include "../../xrEngine/environment.h"
#include "../../xrEngine/DiscordRichPresense.h"
#include "../../xrEngine/x_ray.h"

#include "dxRenderDeviceRender.h"

// matrices
#define	BIND_DECLARE(xf)	\
class cl_xform_##xf	: public R_constant_setup {	virtual void setup (R_constant* C) { RCache.xforms.set_c_##xf (C); } }; \
	static cl_xform_##xf	binder_##xf
BIND_DECLARE(w);
BIND_DECLARE(invw);
BIND_DECLARE(v);
BIND_DECLARE(p);
BIND_DECLARE(wv);
BIND_DECLARE(vp);
BIND_DECLARE(wvp);

#define DECLARE_TREE_BIND(c)	\
	class cl_tree_##c: public R_constant_setup	{virtual void setup(R_constant* C) {RCache.tree.set_c_##c(C);} };	\
	static cl_tree_##c	tree_binder_##c

DECLARE_TREE_BIND(m_xform_v);
DECLARE_TREE_BIND(m_xform);
DECLARE_TREE_BIND(consts);
DECLARE_TREE_BIND(wave);
DECLARE_TREE_BIND(wind);
DECLARE_TREE_BIND(c_scale);
DECLARE_TREE_BIND(c_bias);
DECLARE_TREE_BIND(c_sun);

class cl_hemi_cube_pos_faces: public R_constant_setup
{
	virtual void setup(R_constant* C) {RCache.hemi.set_c_pos_faces(C);}
};

static cl_hemi_cube_pos_faces binder_hemi_cube_pos_faces;

class cl_hemi_cube_neg_faces: public R_constant_setup
{
	virtual void setup(R_constant* C) {RCache.hemi.set_c_neg_faces(C);}
};

static cl_hemi_cube_neg_faces binder_hemi_cube_neg_faces;

class cl_material: public R_constant_setup
{
	virtual void setup(R_constant* C) {RCache.hemi.set_c_material(C);}
};

static cl_material binder_material;

class cl_texgen : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		Fmatrix mTexgen;

#ifdef USE_DX11
		Fmatrix			mTexelAdjust		= 
		{
			0.5f,				0.0f,				0.0f,			0.0f,
			0.0f,				-0.5f,				0.0f,			0.0f,
			0.0f,				0.0f,				1.0f,			0.0f,
			0.5f,				0.5f,				0.0f,			1.0f
		};
#else	//	USE_DX11
		float	_w						= float(RDEVICE.dwWidth);
		float	_h						= float(RDEVICE.dwHeight);
		float	o_w						= (.5f / _w);
		float	o_h						= (.5f / _h);
		Fmatrix			mTexelAdjust		= 
		{
			0.5f,				0.0f,				0.0f,			0.0f,
			0.0f,				-0.5f,				0.0f,			0.0f,
			0.0f,				0.0f,				1.0f,			0.0f,
			0.5f + o_w,			0.5f + o_h,			0.0f,			1.0f
		};
#endif

		mTexgen.mul	(mTexelAdjust,RCache.xforms.m_wvp);

		RCache.set_c( C, mTexgen);
	}
};
static cl_texgen		binder_texgen;

class cl_VPtexgen : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		Fmatrix mTexgen;

#ifdef USE_DX11
		Fmatrix			mTexelAdjust		= 
		{
			0.5f,				0.0f,				0.0f,			0.0f,
			0.0f,				-0.5f,				0.0f,			0.0f,
			0.0f,				0.0f,				1.0f,			0.0f,
			0.5f,				0.5f,				0.0f,			1.0f
		};
#else	//	USE_DX11
		float	_w						= float(RDEVICE.dwWidth);
		float	_h						= float(RDEVICE.dwHeight);
		float	o_w						= (.5f / _w);
		float	o_h						= (.5f / _h);
		Fmatrix			mTexelAdjust		= 
		{
			0.5f,				0.0f,				0.0f,			0.0f,
			0.0f,				-0.5f,				0.0f,			0.0f,
			0.0f,				0.0f,				1.0f,			0.0f,
			0.5f + o_w,			0.5f + o_h,			0.0f,			1.0f
		};
#endif

		mTexgen.mul	(mTexelAdjust,RCache.xforms.m_vp);

		RCache.set_c( C, mTexgen);
	}
};
static cl_VPtexgen		binder_VPtexgen;

// fog
#ifndef _EDITOR
class cl_fog_plane	: public R_constant_setup {
	u32			marker;
	Fvector4	result;
	virtual void setup(R_constant* C)
	{
		if (marker!=Device.dwFrame)
		{
			// Plane
			Fvector4		plane;
			Fmatrix&	M	= Device.mFullTransform;
			plane.x			= -(M._14 + M._13);
			plane.y			= -(M._24 + M._23);
			plane.z			= -(M._34 + M._33);
			plane.w			= -(M._44 + M._43);
			float denom		= -1.0f / _sqrt(_sqr(plane.x)+_sqr(plane.y)+_sqr(plane.z));
			plane.mul		(denom);

			// Near/Far
			float A			= g_pGamePersistent->Environment().CurrentEnv->fog_near;
			float B			= 1/(g_pGamePersistent->Environment().CurrentEnv->fog_far - A);
			result.set		(-plane.x*B, -plane.y*B, -plane.z*B, 1 - (plane.w-A)*B	);								// view-plane
		}
		RCache.set_c	(C,result);
	}
};
static cl_fog_plane		binder_fog_plane;

// fog-params
class cl_fog_params	: public R_constant_setup {
	u32			marker;
	Fvector4	result;
	virtual void setup(R_constant* C)
	{
		if (marker!=Device.dwFrame)
		{
			// Near/Far
			float	n		= g_pGamePersistent->Environment().CurrentEnv->fog_near	;
			float	f		= g_pGamePersistent->Environment().CurrentEnv->fog_far		;
			float	r		= 1/(f-n);
			result.set(-n * r, n, f, r);
		}
		RCache.set_c	(C,result);
	}
};	static cl_fog_params	binder_fog_params;

// fog-color
class cl_fog_color	: public R_constant_setup {
	u32			marker;
	Fvector4	result;
	virtual void setup	(R_constant* C)	{
		if (marker!=Device.dwFrame)	{
			CEnvDescriptor&	desc	= *g_pGamePersistent->Environment().CurrentEnv;
			result.set				(desc.fog_color.x,	desc.fog_color.y, desc.fog_color.z, desc.fog_density);
		}
		RCache.set_c	(C,result);
	}
};	static cl_fog_color		binder_fog_color;
#endif

//Lowland fog params
class cl_lowland_fog_params : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		CEnvDescriptor&	desc = *g_pGamePersistent->Environment().CurrentEnv;
		float low_fog_height = desc.lowland_fog_height;
		float low_fog_density = desc.lowland_fog_density;
		float low_fog_base_height = g_discord.LowlandFogBaseHeight;
		RCache.set_c(C, low_fog_height, low_fog_density, low_fog_base_height, 0);
	}
};	static cl_lowland_fog_params binder_lowland_fog_params;

class cl_m_v2w : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		Fmatrix m_v2w;
		m_v2w.invert(Device.mView);
		RCache.set_c(C, m_v2w);
	}
};

static cl_m_v2w binder_m_v2w;

// DWM: set weather params

class cl_rain_params : public R_constant_setup 
{
    virtual void setup(R_constant* C)
    {
		float rainDensity = g_pGamePersistent->Environment().CurrentEnv->rain_density;
        float wetness_accum    = g_pGamePersistent->Environment().wetness_accum;

        LPCSTR wetness_comment = "Wetness accumulator:";

            //Log(wetness_comment, wetness_accum);

        RCache.set_c (C, rainDensity, wetness_accum, 0, 0);
    }
};
static cl_rain_params binder_rain_params;

class cl_wind_params : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		Fvector4 result;
		CEnvDescriptor& E = *g_pGamePersistent->Environment().CurrentEnv;
		result.set(E.wind_direction, E.wind_velocity, E.m_fTreeAmplitudeIntensity, 0.0f);
		RCache.set_c(C, result);
	}
};
static cl_wind_params binder_wind_params;

class pp_image_corrections : public R_constant_setup
{
	virtual void setup(R_constant* C) override
	{
		RCache.set_c(C, ps_r2_img_exposure, ps_r2_img_gamma, ps_r2_img_saturation, 1.f);
	}
};
static pp_image_corrections binder_image_corrections;

class pp_color_grading : public R_constant_setup
{
	virtual void setup(R_constant* C) override
	{
		RCache.set_c(C, ps_r2_img_cg.x, ps_r2_img_cg.y, ps_r2_img_cg.z, 1.f);
	}
};
static pp_color_grading binder_color_grading;

class cl_sky_color : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		Fvector4	result;
		CEnvDescriptor& desc = *g_pGamePersistent->Environment().CurrentEnv;
		result.set(desc.sky_color.x, desc.sky_color.y, desc.sky_color.z, desc.sky_rotation);
		RCache.set_c(C, result);
	}
};
static cl_sky_color binder_sky_color;

class cl_temperature_params : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		Fvector4 result;
		CEnvDescriptor& desc = *g_pGamePersistent->Environment().CurrentEnv;
		RCache.set_c(C, desc.m_fAirTemperature, 0, 0, 0);
	}
};
static cl_temperature_params binder_temperature_params;

// times
class cl_times		: public R_constant_setup {
	virtual void setup(R_constant* C)
	{
		float 		t	= RDEVICE.fTimeGlobal;
		RCache.set_c	(C,t,t*10,t/10,_sin(t))	;
	}
};
static cl_times		binder_times;

// eye-params
class cl_eye_P		: public R_constant_setup {
	virtual void setup(R_constant* C)
	{
		Fvector&		V	= RDEVICE.vCameraPosition;
		RCache.set_c	(C,V.x,V.y,V.z,1);
	}
};
static cl_eye_P		binder_eye_P;

// eye-params
class cl_eye_D		: public R_constant_setup {
	virtual void setup(R_constant* C)
	{
		Fvector&		V	= RDEVICE.vCameraDirection;
		RCache.set_c	(C,V.x,V.y,V.z,0);
	}
};
static cl_eye_D		binder_eye_D;

// eye-params
class cl_eye_N		: public R_constant_setup {
	virtual void setup(R_constant* C)
	{
		Fvector&		V	= RDEVICE.vCameraTop;
		RCache.set_c	(C,V.x,V.y,V.z,0);
	}
};
static cl_eye_N		binder_eye_N;

#ifndef _EDITOR
// D-Light0
class cl_sun0_color	: public R_constant_setup {
	u32			marker;
	Fvector4	result;
	virtual void setup	(R_constant* C)	{
		if (marker!=Device.dwFrame)	{
			CEnvDescriptor&	desc	= *g_pGamePersistent->Environment().CurrentEnv;
			result.set				(desc.sun_color.x,	desc.sun_color.y, desc.sun_color.z,	0);
		}
		RCache.set_c	(C,result);
	}
};	static cl_sun0_color		binder_sun0_color;
class cl_sun0_dir_w	: public R_constant_setup {
	u32			marker;
	Fvector4	result;
	virtual void setup	(R_constant* C)	{
		if (marker!=Device.dwFrame)	{
			CEnvDescriptor&	desc	= *g_pGamePersistent->Environment().CurrentEnv;
			result.set				(desc.sun_dir.x,	desc.sun_dir.y, desc.sun_dir.z,	0);
		}
		RCache.set_c	(C,result);
	}
};	static cl_sun0_dir_w		binder_sun0_dir_w;
class cl_sun0_dir_e	: public R_constant_setup {
	u32			marker;
	Fvector4	result;
	virtual void setup	(R_constant* C)	{
		if (marker!=Device.dwFrame)	{
			Fvector D;
			CEnvDescriptor&	desc		= *g_pGamePersistent->Environment().CurrentEnv;
			Device.mView.transform_dir	(D,desc.sun_dir);
			D.normalize					();
			result.set					(D.x,D.y,D.z,0);
		}
		RCache.set_c	(C,result);
	}
};	static cl_sun0_dir_e		binder_sun0_dir_e;

//
class cl_amb_color	: public R_constant_setup {
	u32			marker;
	Fvector4	result;
	virtual void setup	(R_constant* C)	{
		if (marker!=Device.dwFrame)	{
			CEnvDescriptorMixer&	desc	= *g_pGamePersistent->Environment().CurrentEnv;
			result.set				(desc.ambient.x, desc.ambient.y, desc.ambient.z, desc.weight);
		}
		RCache.set_c	(C,result);
	}
};	static cl_amb_color		binder_amb_color;
class cl_hemi_color	: public R_constant_setup {
	u32			marker;
	Fvector4	result;
	virtual void setup	(R_constant* C)	{
		if (marker!=Device.dwFrame)	{
			CEnvDescriptor&	desc	= *g_pGamePersistent->Environment().CurrentEnv;
			result.set				(desc.hemi_color.x, desc.hemi_color.y, desc.hemi_color.z, desc.hemi_color.w);
		}
		RCache.set_c	(C,result);
	}
};	static cl_hemi_color		binder_hemi_color;
#endif

// SM_TODO: RCache.hemi    "" 
static class cl_hud_params : public R_constant_setup //--#SM+#--
{
	virtual void setup(R_constant* C) { RCache.set_c(C, g_pGamePersistent->m_pGShaderConstants->hud_params); }
} binder_hud_params;

static class cl_script_params : public R_constant_setup //--#SM+#--
{
	virtual void setup(R_constant* C) { RCache.set_c(C, g_pGamePersistent->m_pGShaderConstants->m_script_params); }
} binder_script_params;

static class cl_blend_mode : public R_constant_setup //--#SM+#--
{
	virtual void setup(R_constant* C) { RCache.set_c(C, g_pGamePersistent->m_pGShaderConstants->m_blender_mode); }
} binder_blend_mode;

static class cl_screen_res : public R_constant_setup		
{	
	virtual void setup	(R_constant* C)
	{
		RCache.set_c	(C, (float)RDEVICE.dwWidth, (float)RDEVICE.dwHeight, 1.0f/(float)RDEVICE.dwWidth, 1.0f/(float)RDEVICE.dwHeight);
	}
}	binder_screen_res;

//ogse sunshafts

static class cl_screen_params : public R_constant_setup
{
	Fvector4	result;
	virtual void setup(R_constant* C)
	{
		float fov = float(Device.fFOV);
		float aspect = float(Device.fASPECT);
		result.set(fov, aspect, tan(deg2rad(fov) / 2), g_pGamePersistent->Environment().CurrentEnv->far_plane*0.75f);
		RCache.set_c(C, result);
	}
};
static cl_screen_params binder_screen_params;

//Sneaky debug stuff
extern Fvector4 ps_dev_param_1;
extern Fvector4 ps_dev_param_2;
extern Fvector4 ps_dev_param_3;
extern Fvector4 ps_dev_param_4;
extern Fvector4 ps_dev_param_5;
extern Fvector4 ps_dev_param_6;
extern Fvector4 ps_dev_param_7;
extern Fvector4 ps_dev_param_8;

static class dev_param_1 : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		RCache.set_c(C, ps_dev_param_1.x, ps_dev_param_1.y, ps_dev_param_1.z, ps_dev_param_1.w);
	}
}    dev_param_1;

static class dev_param_2 : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		RCache.set_c(C, ps_dev_param_2.x, ps_dev_param_2.y, ps_dev_param_2.z, ps_dev_param_2.w);
	}
}    dev_param_2;

static class dev_param_3 : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		RCache.set_c(C, ps_dev_param_3.x, ps_dev_param_3.y, ps_dev_param_3.z, ps_dev_param_3.w);
	}
}    dev_param_3;

static class dev_param_4 : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		RCache.set_c(C, ps_dev_param_4.x, ps_dev_param_4.y, ps_dev_param_4.z, ps_dev_param_4.w);
	}
}    dev_param_4;

static class dev_param_5 : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		RCache.set_c(C, ps_dev_param_5.x, ps_dev_param_5.y, ps_dev_param_5.z, ps_dev_param_5.w);
	}
}    dev_param_5;

static class dev_param_6 : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		RCache.set_c(C, ps_dev_param_6.x, ps_dev_param_6.y, ps_dev_param_6.z, ps_dev_param_6.w);
	}
}    dev_param_6;

static class dev_param_7 : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		RCache.set_c(C, ps_dev_param_7.x, ps_dev_param_7.y, ps_dev_param_7.z, ps_dev_param_7.w);
	}
}    dev_param_7;

static class dev_param_8 : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		RCache.set_c(C, ps_dev_param_8.x, ps_dev_param_8.y, ps_dev_param_8.z, ps_dev_param_8.w);
	}
}    dev_param_8;

static class cl_pda_params : public R_constant_setup
{
	u32 marker;
	Fvector4 result;

	virtual void setup(R_constant* C)
	{
		float pda_factor = g_pGamePersistent->devices_shader_data.pda_display_factor;
		float pda_psy_factor = g_pGamePersistent->devices_shader_data.pda_psy_influence;
		float pda_display_brightness = g_pGamePersistent->devices_shader_data.pda_displaybrightness;
		RCache.set_c(C, pda_factor, pda_psy_factor, pda_display_brightness, 0.0f);
	}

} binder_pda_params;

static class cl_device_params : public R_constant_setup
{
	u32 marker;
	Fvector4 result;

	virtual void setup(R_constant* C)
	{
		float device_global_psy_factor = g_pGamePersistent->devices_shader_data.device_global_psy_influence;
		float device_psy_zone_factor = g_pGamePersistent->devices_shader_data.device_psy_zone_influence;
		float device_rad_zone_factor = g_pGamePersistent->devices_shader_data.device_radiation_zone_influence;
		RCache.set_c(C, device_global_psy_factor, device_psy_zone_factor, device_rad_zone_factor, 0.0f);
	}

} binder_device_params;

static class cl_inv_v : public R_constant_setup
{
	u32	marker;
	Fmatrix	result;

	virtual void setup(R_constant* C)
	{
		result.invert(Device.mView);

		RCache.set_c(C, result);
	}
} binder_inv_v;

// Ascii1457's Screen Space Shaders
extern ENGINE_API Fvector4 ps_ssfx_hud_drops_1;
extern ENGINE_API Fvector4 ps_ssfx_hud_drops_2;
extern ENGINE_API Fvector4 ps_ssfx_blood_decals;

extern ENGINE_API Fvector4 ps_ssfx_florafixes_1;
extern ENGINE_API Fvector4 ps_ssfx_florafixes_2;
extern ENGINE_API float ps_ssfx_gloss_factor;
extern ENGINE_API Fvector3 ps_ssfx_gloss_minmax;
extern ENGINE_API Fvector4 ps_ssfx_wetsurfaces_1;
extern ENGINE_API Fvector4 ps_ssfx_wetsurfaces_2;
extern ENGINE_API int ps_ssfx_is_underground;
extern ENGINE_API Fvector4 ps_ssfx_lightsetup_1;

extern ENGINE_API Fvector4 ps_ssfx_volumetric;
extern ENGINE_API Fvector4 ps_ssfx_ssr_2;
extern ENGINE_API Fvector4 ps_ssfx_terrain_offset;

extern ENGINE_API Fvector3 ps_ssfx_shadow_bias;
extern ENGINE_API Fvector4 ps_ssfx_lut;
extern ENGINE_API Fvector4 ps_ssfx_wind_grass;
extern ENGINE_API Fvector4 ps_ssfx_wind_trees;

static class ssfx_wpn_dof_1 : public R_constant_setup
{
	virtual void setup(R_constant * C)
	{
		RCache.set_c(C, ps_ssfx_wpn_dof_1.x, ps_ssfx_wpn_dof_1.y, ps_ssfx_wpn_dof_1.z, ps_ssfx_wpn_dof_1.w);
	}
} ssfx_wpn_dof_1;

static class ssfx_wpn_dof_2 : public R_constant_setup
{
	virtual void setup(R_constant * C)
	{
		RCache.set_c(C, ps_ssfx_wpn_dof_2, 0, 0, 0);
	}
} ssfx_wpn_dof_2;

// Reflections distance
extern float ps_r2_reflections_distance;

static class cl_refl_dist : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		RCache.set_c(C, ps_r2_reflections_distance, 0, 0, 0);
	}
} cl_refl_dist;

//AO Debug
static class cl_debug : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		RCache.set_c(C, ps_r2_ao_debug, 0, 0, 0);
	}
} binder_debug;

static class cl_spv_screen_res : public R_constant_setup //--#SM+#--
{
	virtual void setup(R_constant* C) { RCache.set_c(C, (float)Device.m_SecondViewport.screenWidth, (float)Device.m_SecondViewport.screenHeight, 0, 0); }
} binder_spv_screen_res;

class ssfx_blood_decals : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		RCache.set_c(C, ps_r4_shaders_flags.test(R4FLAG_SSS_ADDON) ? ps_ssfx_blood_decals : Fvector4().set(0, 0, 0, 0));
	}
};
static ssfx_blood_decals binder_ssfx_blood_decals;

class ssfx_hud_drops_1 : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		RCache.set_c(C, ps_r4_shaders_flags.test(R4FLAG_SSS_ADDON) ? ps_ssfx_hud_drops_1 : Fvector4().set(0, 0, 0, 0));
	}
};
static ssfx_hud_drops_1 binder_ssfx_hud_drops_1;

class ssfx_hud_drops_2 : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		RCache.set_c(C, ps_r4_shaders_flags.test(R4FLAG_SSS_ADDON) ? ps_ssfx_hud_drops_2 : Fvector4().set(0, 0, 0, 0));
	}
};
static ssfx_hud_drops_2 binder_ssfx_hud_drops_2;

class ssfx_lightsetup_1 : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		RCache.set_c(C, ps_ssfx_lightsetup_1);
	}
};
static ssfx_lightsetup_1 binder_ssfx_lightsetup_1;

class ssfx_is_underground : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		RCache.set_c(C, (float)ps_ssfx_is_underground, 0.f, 0.f, 0.f);
	}
};
static ssfx_is_underground binder_ssfx_is_underground;

class ssfx_wetsurfaces_1 : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		RCache.set_c(C, ps_ssfx_wetsurfaces_1);
	}
};
static ssfx_wetsurfaces_1 binder_ssfx_wetsurfaces_1;

class ssfx_wetsurfaces_2 : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		RCache.set_c(C, ps_ssfx_wetsurfaces_2);
	}
};
static ssfx_wetsurfaces_2 binder_ssfx_wetsurfaces_2;

class ssfx_gloss : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		RCache.set_c(C, ps_ssfx_gloss_minmax.x, ps_ssfx_gloss_minmax.y, ps_ssfx_gloss_factor, 0.f);
	}
};
static ssfx_gloss binder_ssfx_gloss;

class ssfx_florafixes_1 : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		RCache.set_c(C, ps_ssfx_florafixes_1);
	}
};
static ssfx_florafixes_1 binder_ssfx_florafixes_1;

class ssfx_florafixes_2 : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		RCache.set_c(C, ps_ssfx_florafixes_2);
	}
};
static ssfx_florafixes_2 binder_ssfx_florafixes_2;

static class ssfx_wind_grass : public R_constant_setup
{
	virtual void setup(R_constant * C)
	{
		RCache.set_c(C, ps_ssfx_wind_grass);
	}
} ssfx_wind_grass;

static class ssfx_wind_trees : public R_constant_setup
{
	virtual void setup(R_constant * C)
	{
		RCache.set_c(C, ps_ssfx_wind_trees);
	}
} ssfx_wind_trees;

static class ssfx_wind_anim : public R_constant_setup
{
	virtual void setup(R_constant * C)
	{
		Fvector3 WindAni = g_pGamePersistent->Environment().wind_anim;
		RCache.set_c(C, WindAni.x, WindAni.y, WindAni.z, 0);
	}
} ssfx_wind_anim;

static class ssfx_lut : public R_constant_setup
{
	virtual void setup(R_constant * C)
	{
		RCache.set_c(C, ps_ssfx_lut);
	}
} ssfx_lut;

static class ssfx_shadow_bias : public R_constant_setup
{
	virtual void setup(R_constant * C)
	{
		RCache.set_c(C, ps_ssfx_shadow_bias.x, ps_ssfx_shadow_bias.y, 0, 0);
	}
} ssfx_shadow_bias;

static class ssfx_terrain_offset : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		RCache.set_c(C, ps_ssfx_terrain_offset);
	}
}    ssfx_terrain_offset;

static class ssfx_ssr_2 : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		RCache.set_c(C, ps_ssfx_ssr_2);
	}
}    ssfx_ssr_2;

static class ssfx_volumetric : public R_constant_setup
{
	virtual void setup(R_constant* C)
	{
		RCache.set_c(C, ps_ssfx_volumetric);
	}
}    ssfx_volumetric;

// Standart constant-binding
void	CBlender_Compile::SetMapping	()
{

	//ogse sunshafts
	r_Constant("ogse_c_screen", &binder_screen_params);


	// DWM: out to shaders view to world mat, weather params, alternative screen res
	r_Constant("m_view2world", &binder_m_v2w);
	r_Constant("sky_color", &binder_sky_color);
	r_Constant("screen_res_alt", &binder_screen_res);
    r_Constant("rain_params", &binder_rain_params);
	r_Constant("wind_params", &binder_wind_params);

	r_Constant("temperature_params", &binder_temperature_params);

	// misc
	r_Constant("m_hud_params", &binder_hud_params);	//--#SM+#--
	r_Constant("m_script_params", &binder_script_params); //--#SM+#--
	r_Constant("m_blender_mode", &binder_blend_mode);	//--#SM+#--
	r_Constant("svp_screen_res", &binder_spv_screen_res);

	// matrices
	r_Constant				("m_W",				&binder_w);
	r_Constant				("m_invW",			&binder_invw);
	r_Constant				("m_V",				&binder_v);
	r_Constant				("m_P",				&binder_p);
	r_Constant				("m_WV",			&binder_wv);
	r_Constant				("m_VP",			&binder_vp);
	r_Constant				("m_WVP",			&binder_wvp);
	r_Constant				("m_inv_V",			&binder_inv_v);

	r_Constant				("m_xform_v",		&tree_binder_m_xform_v);
	r_Constant				("m_xform",			&tree_binder_m_xform);
	r_Constant				("consts",			&tree_binder_consts);
	r_Constant				("wave",			&tree_binder_wave);
	r_Constant				("wind",			&tree_binder_wind);
	r_Constant				("c_scale",			&tree_binder_c_scale);
	r_Constant				("c_bias",			&tree_binder_c_bias);
	r_Constant				("c_sun",			&tree_binder_c_sun);

	//hemi cube
	r_Constant				("L_material",			&binder_material);
	r_Constant				("hemi_cube_pos_faces",			&binder_hemi_cube_pos_faces);
	r_Constant				("hemi_cube_neg_faces",			&binder_hemi_cube_neg_faces);

	//	Igor	temp solution for the texgen functionality in the shader
	r_Constant				("m_texgen",			&binder_texgen);
	r_Constant				("mVPTexgen",			&binder_VPtexgen);

#ifndef _EDITOR
	// fog-params
	r_Constant				("fog_plane",		&binder_fog_plane);
	r_Constant				("fog_params",		&binder_fog_params);
	r_Constant				("fog_color",		&binder_fog_color);
	r_Constant				("lowland_fog_params",	&binder_lowland_fog_params);
#endif
	// time
	r_Constant				("timers",			&binder_times);

	// eye-params
	r_Constant				("eye_position",	&binder_eye_P);
	r_Constant				("eye_direction",	&binder_eye_D);
	r_Constant				("eye_normal",		&binder_eye_N);

#ifndef _EDITOR
	// global-lighting (env params)
	r_Constant				("L_sun_color",		&binder_sun0_color);
	r_Constant				("L_sun_dir_w",		&binder_sun0_dir_w);
	r_Constant				("L_sun_dir_e",		&binder_sun0_dir_e);
//	r_Constant				("L_lmap_color",	&binder_lm_color);
	r_Constant				("L_hemi_color",	&binder_hemi_color);
	r_Constant				("L_ambient",		&binder_amb_color);
#endif
	r_Constant				("screen_res",		&binder_screen_res);

	// Shader stuff
	r_Constant				("shader_param_1",	&dev_param_1);
	r_Constant				("shader_param_2",	&dev_param_2);
	r_Constant				("shader_param_3",	&dev_param_3);
	r_Constant				("shader_param_4",	&dev_param_4);
	r_Constant				("shader_param_5",	&dev_param_5);
	r_Constant				("shader_param_6",	&dev_param_6);
	r_Constant				("shader_param_7",	&dev_param_7);
	r_Constant				("shader_param_8",	&dev_param_8);

	// Anomaly
	r_Constant				("pp_img_corrections", &binder_image_corrections);
	r_Constant				("pp_img_cg",		&binder_color_grading);

	// PDA
	r_Constant				("pda_params",		&binder_pda_params);
	// Nightvision
	r_Constant				("device_influence",	&binder_device_params);
	//Screen Space Shaders
	r_Constant				("ssfx_wpn_dof_1",		&ssfx_wpn_dof_1);
	r_Constant				("ssfx_wpn_dof_2",		&ssfx_wpn_dof_2);
	r_Constant				("ssfx_blood_decals",	&binder_ssfx_blood_decals);
    r_Constant				("ssfx_hud_drops_1",	&binder_ssfx_hud_drops_1);
    r_Constant				("ssfx_hud_drops_2",	&binder_ssfx_hud_drops_2);
	r_Constant				("ssfx_lightsetup_1",	&binder_ssfx_lightsetup_1);
	r_Constant				("ssfx_is_underground", &binder_ssfx_is_underground);
	r_Constant				("ssfx_wetsurfaces_1",	&binder_ssfx_wetsurfaces_1);
	r_Constant				("ssfx_wetsurfaces_2",	&binder_ssfx_wetsurfaces_2);
	r_Constant				("ssfx_gloss",			&binder_ssfx_gloss);
	r_Constant				("ssfx_florafixes_1",	&binder_ssfx_florafixes_1);
	r_Constant				("ssfx_florafixes_2",	&binder_ssfx_florafixes_2);
	r_Constant				("ssfx_volumetric",		&ssfx_volumetric);
	r_Constant				("ssfx_ssr_2",			&ssfx_ssr_2);
	r_Constant				("ssfx_terrain_offset", &ssfx_terrain_offset);
	r_Constant				("ssfx_shadow_bias",	&ssfx_shadow_bias);
	r_Constant				("ssfx_wind_anim",		&ssfx_wind_anim);
	r_Constant				("ssfx_wsetup_grass",	&ssfx_wind_grass);
	r_Constant				("ssfx_wsetup_trees",	&ssfx_wind_trees);
	r_Constant				("ssfx_lut",			&ssfx_lut);
	//Reflections distance
	r_Constant				("reflections_distance", &cl_refl_dist);
	//AO Debug
	r_Constant				("debug",			&binder_debug);

	// detail
	//if (bDetail	&& detail_scaler)
	//	Igor: bDetail can be overridden by no_detail_texture option.
	//	But shader can be deatiled implicitly, so try to set this parameter
	//	anyway.
	if (detail_scaler)
		r_Constant			("dt_params",		detail_scaler);

	// other common
	for (u32 it=0; it<DEV->v_constant_setup.size(); it++)
	{
		std::pair<shared_str,R_constant_setup*>	cs	= DEV->v_constant_setup[it];
		r_Constant			(*cs.first,cs.second);
	}
}
