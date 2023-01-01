/**
 * @ Version: SCREEN SPACE SHADERS - UPDATE 13.2
 * @ Description: 2 Layers fog ( Distance + Height )
 * @ Modified time: 2022-12-28 02:09
 * @ Author: https://www.moddb.com/members/ascii1457
 * @ Mod: https://www.moddb.com/mods/stalker-anomaly/addons/screen-space-shaders
 */

#include "settings_screenspace_FOG.h"
#include "common.h"

float SSFX_HEIGHT_FOG(float3 P, float World_Py, inout float3 color)
{
	// Get Sun dir
	float3 Sun = saturate(dot(normalize(Ldynamic_dir), -normalize(P)));

	// Apply sun color
	Sun = lerp(fog_color, Ldynamic_color.rgb, Sun);

	// Distance Fog ( Default Anomaly Fog )
	float fog = saturate(length(P) * fog_params.w + fog_params.x);

	// Height Fog
	
#ifdef G_USE_PARAMS_FROM_WEATHER
	float fogheight = smoothstep(lowland_fog_params.x, -lowland_fog_params.x, World_Py) * lowland_fog_params.y;
#else
	float fogheight = smoothstep(G_FOG_HEIGHT, -G_FOG_HEIGHT, World_Py) * G_FOG_HEIGHT_INTENSITY;
#endif
	// Add the height fog to the distance fog
	float fogresult = saturate(fog + fogheight * fog);

	// Blend factor to mix sun color, fog color and adjust intensity.
	float FogBlend = fogheight * G_FOG_SUNCOLOR_INTENSITY;

	// Final fog color
	float3 FOG_COLOR = lerp(fog_color, Sun, FogBlend);

	// Apply fog to color
	color = lerp(color, FOG_COLOR, fogresult);

	// Return distance fog.
	return fog;
}

float SSFX_FOGGING(float Fog, float World_Py)
{
	// Height fog
#ifdef G_USE_PARAMS_FROM_WEATHER
	float fog_height = smoothstep(lowland_fog_params.x, -lowland_fog_params.x, World_Py);
#else
	float fog_height = smoothstep(G_FOG_HEIGHT, -G_FOG_HEIGHT, World_Py);
#endif

	// Add height fog to default distance fog.
	float fog_extra = saturate(Fog + fog_height * Fog);
	
	// Return distance fog & height fog.
	return 1.0f - fog_extra;
}