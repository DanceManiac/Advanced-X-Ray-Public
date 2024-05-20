#ifndef EXTCOMMON_H_INCLUDED
#define EXTCOMMON_H_INCLUDED

//////////////////////////////////////////////////////////////////////////////////////////
//
#define		HudMaskDepthConstant		half(100)//COP

//////////////////////////////////////////////////////////////////////////////////////////
//
struct		AntiAliasingStruct
{
	float4		texCoord0		:		TEXCOORD0;
	float4		texCoord1		:		TEXCOORD1;
	float4		texCoord2		:		TEXCOORD2;
	float4		texCoord3		:		TEXCOORD3;
	float4		texCoord4		:		TEXCOORD4;
	float4		texCoord5		:		TEXCOORD5;
	float4		texCoord6		:		TEXCOORD6;
};

struct		OUTStruct
{
	half4		Position		:		COLOR0;
	half4		Normal		:		COLOR1;
};

//////////////////////////////////////////////////////////////////////////////////////////
//

uniform		sampler2D		sScene;
uniform		sampler2D		sPosition;
uniform		sampler2D		sMask;          //sunshafts occlusion mask
uniform		sampler2D		sMaskBlur;      //smoothed mask
uniform		sampler2D		sSunShafts;     //
uniform		sampler2D		s_hud_mask;  	//hud mask du/dv map

//////////////////////////////////////////////////////////////////////////////////////////
//
uniform 	float2 		hud_mask_params; //x-weather control,y-debug intensity
uniform		half4		SSParams;
uniform		half4		SSParamsDISPLAY;

//////////////////////////////////////////////////////////////////////////////////////////
//
half		NormalizeDepth(half Depth)
{
	return saturate(Depth/HudMaskDepthConstant);
}

float4		proj2screen(float4 Project)
{
	float4 Screen;
	Screen.x = (Project.x+Project.w)*0.5;
	Screen.y = (Project.w-Project.y)*0.5;
	Screen.z = Project.z;
	Screen.w = Project.w;
	return Screen;
}

//Gasmask settings
#define GM_DIST_INT 0.1 //Refraction intensity
#define GM_DIFF_INT 0.15 //Diffuse cracks intensity
#define GM_VIG_INT 0.5 //Vignette intensity

//Glass reflections settings
#define GM_VIS_NUM 16 //Reflection quality
#define GM_VIS_RADIUS 0.45 //Reflection radius
#define GM_VIS_INTENSITY 1.0 //Reflection intensity

#endif//EXTCOMMON_H_INCLUDED