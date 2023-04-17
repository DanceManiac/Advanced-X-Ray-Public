#include "common.h"
#include "skin.h"

uniform float4x4 m_v2w:register(ps,c3);

struct vf
{
	float4 hpos	: POSITION;
	float2 tc0	: TEXCOORD0;		// base
	float4 pos2d: TEXCOORD1;
	float4 posW : TEXCOORD2;
	float4 c0	: COLOR0;		// color
};

vf 	_main (v_model v)
{
	vf 		o;

	o.hpos = mul(m_WVP, v.pos);	// xform, input in world coords
	o.posW.xyz = mul(m_v2w, v.pos);
	o.posW.w = 1;
	o.tc0 = v.tc.xy; // copy tc
	o.pos2d.x = (o.hpos.x + o.hpos.w)*0.5;
	o.pos2d.y = (o.hpos.w - o.hpos.y)*0.5;
	o.pos2d.z = o.hpos.z;
	o.pos2d.w = o.hpos.w;
	o.pos2d.xyz = o.pos2d.xyz / o.pos2d.w;
	
	// calculate fade
	float3  dir_v 	= normalize		(mul(m_WV,v.pos));
	float3 	norm_v 	= normalize 		(mul(m_WV,v.norm));
	float 	fade 	= abs			(dot(dir_v,norm_v));
	o.c0		= fade;

#ifdef SKIN_COLOR
	o.c0.rgb	*= v.rgb_tint;
#endif
	return o;
}

/////////////////////////////////////////////////////////////////////////
#define SKIN_LQ
#define SKIN_VF vf
#include "skin_main.h"