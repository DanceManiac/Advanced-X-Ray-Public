#include "common.h"
#include "skin.h"

v2p_bumped _main( v_model I )
{
	v2p_bumped O;
	
	//Hpos and texcoord
	O.hpos = mul(m_WVP, I.P);
	O.tcdh = float4(I.tc.xyyy);

	//Hemi
	float3 Nw = mul((float3x3)m_W, I.N.xyz);
	float3 hc_pos = (float3)hemi_cube_pos_faces;
	float3 hc_neg = (float3)hemi_cube_neg_faces;
	float3 hc_mixed = (Nw.y < 0.0) ? hc_neg : hc_pos;
	float hemi_val = saturate(dot(hc_mixed, abs(Nw)));

	//TBN
	float3 N = I.N;	
	float3 T = I.T;
	T = normalize(T - dot(T, N) * N);	
	float3 B = cross(N,T);
	
	float3x3 xform = mul((float3x3)m_WV, float3x3(
	2.0 * T.x, 2.0 * B.x, 2.0 * N.x,
	2.0 * T.y, 2.0 * B.y, 2.0 * N.y,
	2.0 * T.z, 2.0 * B.z, 2.0 * N.z
	)); 

	//Feed this TBN to pixel shader
	O.M1 = xform[0]; 
	O.M2 = xform[1]; 
	O.M3 = xform[2]; 
	
	//Position
	float3 Pe = mul(m_WV, I.P);
	O.position = float4(Pe, hemi_val);
	
	return O;
}

/////////////////////////////////////////////////////////////////////////
#ifdef SKIN_NONE
v2p_bumped	main(v_model v)
{ 
	return _main(v);
}
#endif

#ifdef SKIN_0
v2p_bumped main(v_model_skinned_0 v)
{
	return _main(skinning_0(v)); 
}
#endif

#ifdef SKIN_1
v2p_bumped main(v_model_skinned_1 v)
{
	return _main(skinning_1(v));
}
#endif

#ifdef SKIN_2
v2p_bumped main(v_model_skinned_2 v)
{
	return _main(skinning_2(v));
}
#endif

#ifdef SKIN_3
v2p_bumped main(v_model_skinned_3 v)
{
	return _main(skinning_3(v));
}
#endif

#ifdef SKIN_4
v2p_bumped main(v_model_skinned_4 v)
{
	return _main(skinning_4(v));
}
#endif