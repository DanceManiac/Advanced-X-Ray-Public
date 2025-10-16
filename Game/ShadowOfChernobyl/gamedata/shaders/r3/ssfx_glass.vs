/**
 * @ Version: SCREEN SPACE SHADERS - UPDATE 23
 * @ Description: Glass Shader - VS
 * @ Modified time: 2025-05-10 23:25:24
 * @ Author: https://www.moddb.com/members/ascii1457
 * @ Mod: https://www.moddb.com/mods/stalker-anomaly/addons/screen-space-shaders
 */

#include "common.h"
#include "skin.h"

#include "check_screenspace.h"

#ifdef SSFX_FOG
	#include "screenspace_fog.h"
#endif

struct	v_model_
{
	float4	P	: POSITION;		// (float,float,float,1)
	float3	N	: NORMAL;		// (nx,ny,nz)
	float3	T	: TANGENT;		// (nx,ny,nz)
	float3	B	: BINORMAL;		// (nx,ny,nz)
	float2	tc	: TEXCOORD0;	// (u,v)
};

struct 	vf
{
	float4 P	: POSITION;
	float2 tc0	: TEXCOORD0;		// base
	float3 tc1	: TEXCOORD1;		// environment
	float4 hpos	: SV_Position;
	float  fog	: FOG;
	
	float3 v2point 	: TEXCOORD2;
	float3 nor		: TEXCOORD3;
};

#ifdef 	SKIN_NONE
vf 	_main (v_model_ v, float3 psp)
#else
vf 	_main (v_model v, float3 psp)
#endif
{
	vf 		o;

	float4 	pos 	= v.P;
	float3  pos_w 	= mul(m_W, pos);
	float3 	norm_w 	= normalize(mul(m_W,v.N));

	o.hpos 		= mul(m_WVP, pos);		// xform, input in world coords
	o.tc0		= v.tc.xy;				// copy tc
	o.tc1		= calc_reflection(pos_w, norm_w);

	o.fog 		= saturate(calc_fogging(float4(pos_w,1)) );	// Vanilla fog, input in world coords

#ifdef SSFX_FOG
	o.fog		= SSFX_FOGGING(1.0 - o.fog, pos.y); // SSS Height Fog
#endif	

	o.v2point = normalize(pos_w - eye_position);
	o.nor = norm_w;
	
	if (!all(psp))
		o.P.xyz = pos.xyz;
	else
		o.P.xyz = psp;

	return o;
}

/////////////////////////////////////////////////////////////////////////
#ifdef 	SKIN_NONE
vf	main(v_model_ v) { return _main(v,0); }
#endif

#ifdef 	SKIN_0
vf	main(v_model_skinned_0 v) 	{ return _main(skinning_0(v), v.P); }
#endif

#ifdef	SKIN_1
vf	main(v_model_skinned_1 v) 	{ return _main(skinning_1(v), v.P); }
#endif

#ifdef	SKIN_2
vf	main(v_model_skinned_2 v) 	{ return _main(skinning_2(v), v.P); }
#endif

#ifdef	SKIN_3
vf	main(v_model_skinned_3 v) 	{ return _main(skinning_3(v), v.P); }
#endif

#ifdef	SKIN_4
vf	main(v_model_skinned_4 v) 	{ return _main(skinning_4(v), v.P); }
#endif