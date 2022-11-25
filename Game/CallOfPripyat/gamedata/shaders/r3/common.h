#ifndef        COMMON_H
#define        COMMON_H

#include "shared\common.h"

#include "common_defines.h"
#include "common_policies.h"
#include "common_iostructs.h"
#include "common_samplers.h"
#include "common_cbuffers.h"
#include "common_functions.h"

// #define USE_SUPER_SPECULAR
#define USE_SUNMASK                		//- shader defined
#define SKY_DEPTH	float(10000.f)

#ifdef        USE_R2_STATIC_SUN
#  define xmaterial float(1.0h/4.h)
#else
#  define xmaterial float(L_material.w)
#endif

#define FXPS technique _render{pass _code{PixelShader=compile ps_3_0 main();}}
#define FXVS technique _render{pass _code{VertexShader=compile vs_3_0 main();}}

uniform float4 		sky_color; // .xyz - sky color, .w - sky rotation
uniform float4 		lowland_fog_params; // x - low fog height, y - low fog density, z - base height, w - null
uniform float4 		screen_res_alt; // .xy - pos_decompression_params2.xy, .zw - screen_res.xy power to -1
uniform float4 		rain_params; // .x - rain density, .y - wetness accumulator, .zw = 0
uniform float4x4	m_view2world;
uniform float3x4	m_inv_V;
uniform float4		screen_res;
uniform float4 		fov;

// Глобальные параметры шейдеров --#SM+#--
uniform	float4		m_hud_params;	// zoom_rotate_factor, secondVP_zoom_factor, NULL, NULL
uniform	float4		m_blender_mode;	

float4 proj_to_screen(float4 proj) 
{ 
	float4 screen = proj; 
	screen.x = (proj.x + proj.w); 
	screen.y = (proj.w - proj.y); 
	screen.xy *= 0.5; 
	return screen; 
}

// Активен-ли двойной рендер --#SM+#--
inline bool isSecondVPActive()
{
	return (m_blender_mode.z == 1.f);
}
#endif