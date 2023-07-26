//////////////////////////////////////////////////////////////////////////
//-' CryRay Engine x64 Shaders
//-' Shaders are based on the Call of Pripyat version 1.6.02
//-' To contact the developers and get additional information
//-' Follow this link: https://discord.gg/Ry8snVzgu7
//-' OldSerpskiStalker, CryRay Team 2022 - 2023
//////////////////////////////////////////////////////////////////////////

#include "common.h"
//////////////////////////////////////////////////////////////////////////////////////////
#define SMAA_HLSL_4_1

#define SMAA_INCLUDE_VS 1
#define SMAA_RT_METRICS screen_res.zwxy
//////////////////////////////////////////////////////////////////////////////////////////

#include "smaa.h"

//Struct
struct p_smaa
{
	float4 hpos			: SV_Position;
	float2 tc0			: TEXCOORD0;        // Texture coordinates         (for sampling maps)
	float2 pixcoord		: TEXCOORD1;	
	float4 offset[3]	: TEXCOORD2;
};

struct	v2p_smaa
{
	float2 tc0 : TEXCOORD0;	
	float4 HPos : POSITIONT;  	// Clip-space position 	(for rasterization)
};

//////////////////////////////////////////////////////////////////////////////////////////
// Vertex
p_smaa main(v2p_smaa I)
{
	p_smaa O;
	// Transform to screen space (in d3d9 it was done automatically)
	O.hpos.x 	=  (I.HPos.x * screen_res.z * 2 - 1);
	O.hpos.y 	= -(I.HPos.y * screen_res.w * 2 - 1);
	O.hpos.zw 	=	I.HPos.zw;
	
    O.tc0 		= 	I.tc0;
  
	SMAABlendingWeightCalculationVS( I.tc0, O.pixcoord, O.offset );

    return O; 
}