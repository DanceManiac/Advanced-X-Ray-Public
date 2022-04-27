#ifndef EXTCOMMON_H_INCLUDED
#define EXTCOMMON_H_INCLUDED

//////////////////////////////////////////////////////////////////////////////////////////
//
struct		AntiAliasingStruct
{
	float2		texCoord0		:		TEXCOORD0;
	float2		texCoord1		:		TEXCOORD1;
	float2		texCoord2		:		TEXCOORD2;
	float2		texCoord3		:		TEXCOORD3;
	float2		texCoord4		:		TEXCOORD4;
	float4		texCoord5		:		TEXCOORD5;
	float4		texCoord6		:		TEXCOORD6;
};

// Hud Mask
Texture2D s_hud_mask;

uniform float2 hud_mask_params;

//Gasmask settings
#define GM_DIST_INT 0.1 //Refraction intensity
#define GM_DIFF_INT 0.15 //Diffuse cracks intensity
#define GM_VIG_INT 0.5 //Vignette intensity

//Glass reflections settings
#define GM_VIS_NUM 16 //Reflection quality
#define GM_VIS_RADIUS 0.45 //Reflection radius
#define GM_VIS_INTENSITY 1.0 //Reflection intensity

#endif//EXTCOMMON_H_INCLUDED 