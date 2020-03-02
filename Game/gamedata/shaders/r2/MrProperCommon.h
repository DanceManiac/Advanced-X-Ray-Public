#ifndef EXTCOMMON_H_INCLUDED
#define EXTCOMMON_H_INCLUDED

//////////////////////////////////////////////////////////////////////////////////////////
//
#define		MrProperDepthConstant		half(100)//COP

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
uniform		sampler2D		s_rain_drops0;  //rain drops du/dv map

//////////////////////////////////////////////////////////////////////////////////////////
//
//uniform		float4		screenParams;
//uniform		half4		SunShaftsRadius;
//uniform		half4		SampleStepParams;
//uniform		half4		BlendParams;
//uniform		half4		SSIntensity;
uniform 	float2 			rain_drops_params0; //x-weather control,y-debug intensity
uniform		half4		SSParams;
uniform		half4		SSParamsDISPLAY;

//////////////////////////////////////////////////////////////////////////////////////////
//
half		NormalizeDepth(half Depth)
{
	return saturate(Depth/MrProperDepthConstant);
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

#endif//EXTCOMMON_H_INCLUDED