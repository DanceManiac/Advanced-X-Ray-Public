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

#endif//EXTCOMMON_H_INCLUDED 