#include "common.h"
#include "skin.h"

#if defined(ALLOW_AMD_RETICLE_FIX) || defined(USE_AMD_RETICLE_FIX)
struct vf
{
	float2 tc0	: TEXCOORD0;		// base
	float4 c0	: COLOR0;		// color
	float4 hpos	: SV_Position;
};

vf 	_main (v_model v)
{
	vf 		o;

	o.hpos 		= mul			(m_WVP, v.P);		// xform, input in world coords
	o.tc0		= v.tc.xy;					// copy tc

	// calculate fade
	float3  dir_v 	= normalize		(mul(m_WV,v.P));
	float3 	norm_v 	= normalize 		(mul(m_WV,v.N));
	float 	fade 	= abs			(dot(dir_v,norm_v));
	o.c0		= fade;

	return o;
}
#endif
#if !defined(ALLOW_AMD_RETICLE_FIX) || defined(USE_AMD_RETICLE_FIX)
struct vf
{
 	float2 tc0		: TEXCOORD0;	//Unused texcoord
	float3 position	: TEXCOORD1;	//View space position
	float3 normal	: TEXCOORD2;	//View space normal
	float3 tangent	: TEXCOORD3;	//View space tangent
	
	float4 c0		: COLOR0;		//Color
	float4 hpos		: SV_Position;
};

vf 	_main (v_model v)
{
	vf o;

	o.hpos = mul(m_WVP, v.P);		// xform, input in world coords
	o.tc0 = v.tc.xy;					// copy tc

	//Transform our content into view space
	o.position = mul(m_WV,v.P);
	o.normal = mul(m_WV,v.N);
	o.tangent = mul(m_WV, v.T);
	
	//No fade cuz we got a collimator lens, not some shit :)
	o.c0 = 1.0;
	return o;
}
#endif




/////////////////////////////////////////////////////////////////////////
#ifdef 	SKIN_NONE
vf	main(v_model v) 		{ return _main(v); 		}
#endif

#ifdef 	SKIN_0
vf	main(v_model_skinned_0 v) 	{ return _main(skinning_0(v)); }
#endif

#ifdef	SKIN_1
vf	main(v_model_skinned_1 v) 	{ return _main(skinning_1(v)); }
#endif

#ifdef	SKIN_2
vf	main(v_model_skinned_2 v) 	{ return _main(skinning_2(v)); }
#endif

#ifdef	SKIN_3
vf	main(v_model_skinned_3 v) 	{ return _main(skinning_3(v)); }
#endif

#ifdef	SKIN_4
vf	main(v_model_skinned_4 v) 	{ return _main(skinning_4(v)); }
#endif
