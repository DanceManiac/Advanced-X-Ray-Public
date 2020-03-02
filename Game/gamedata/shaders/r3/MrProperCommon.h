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

// Rain drops
Texture2D s_rain_drops0;
uniform float2 rain_drops_params0;
//#define RAIN_DROPS_DEBUG

#endif//EXTCOMMON_H_INCLUDED 