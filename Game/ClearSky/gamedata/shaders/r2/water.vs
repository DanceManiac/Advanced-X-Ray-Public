#include "common.h"
#include "shared\waterconfig.h"
#include "shared\watermove.h"

struct        v_vert
{
        float4         P        : POSITION;        // (float,float,float,1)
        float4        N        : NORMAL;        // (nx,ny,nz,hemi occlusion)
        float4         T        : TANGENT;
        float4         B        : BINORMAL;
        float4        color        : COLOR0;        // (r,g,b,dir-occlusion)
        float2         uv        : TEXCOORD0;        // (u0,v0)
};
struct   vf
{
        float4      hpos        :         POSITION        ;
        float2  	tbase        :           TEXCOORD0        ;  // base
        float2      tnorm0        :        TEXCOORD1        ;  // nm0
        float2      tnorm1        :        TEXCOORD2        ;  // nm1
        half3       M1        :        TEXCOORD3        ;
        half3       M2        :        TEXCOORD4        ;
        half3       M3        	:        TEXCOORD5        ;
        half3       v2point     :        TEXCOORD6        ;
#ifdef	USE_SOFT_WATER
#ifdef	NEED_SOFT_WATER
	float4      tctexgen    :         TEXCOORD7        ;
#endif	//	USE_SOFT_WATER
#endif	//	NEED_SOFT_WATER	
        half4        c0        :          COLOR0                ;
        float          fog        :         FOG                ;
};

uniform float4 ogse_c_rain; // x - rmap dist, y - flood level, z - wet level, w - rain intensity
uniform float4 c_depth;

vf main(v_vert v)
{
        vf                 o;

	float4 P = v.P;                // world

#if defined(USE_PUDDLES) && defined(NEED_PUDDLES)
	P.xyz = mul(m_W, P);
	float speed = PUDDLES_GROW_SPEED;
	P.y += saturate(ogse_c_rain.x*ogse_c_rain.w*speed)*c_depth.x;
	P.w = 1.f;
#endif

	float3 NN = unpack_normal(v.N);
    P = watermove(P)        ;

    o.v2point = P - eye_position;

#if defined(USE_PUDDLES) && defined(NEED_PUDDLES)
	o.tbase = v.uv;
	float3 N = v.N.xyz; 
	float3 T = v.T.xyz;
	float3 B = v.B.xyz;        
#else 
    o.tbase = unpack_tc_base(v.uv,v.T.w,v.B.w); // copy tc
	float3 N = unpack_bx2(v.N.xyz); // just scale (assume normal in the -.5f, .5f)
	float3 T = unpack_bx2(v.T.xyz); //
	float3 B = unpack_bx2(v.B.xyz); //
#endif

    o.tnorm0 = watermove_tc(o.tbase*W_DISTORT_BASE_TILE_0, P.xz, W_DISTORT_AMP_0);
    o.tnorm1 = watermove_tc(o.tbase*W_DISTORT_BASE_TILE_1, P.xz, W_DISTORT_AMP_1);
	
	// Calculate the 3x3 transform from tangent space to eye-space
	// TangentToEyeSpace = object2eye * tangent2object
	//                   = object2eye * transpose(object2tangent) (since the inverse of a rotation is its transpose)
	float3x3 xform = mul((float3x3)m_W, float3x3(
                                                T.x,B.x,N.x,
                                                T.y,B.y,N.y,
                                                T.z,B.z,N.z
						));
	// The pixel shader operates on the bump-map in [0..1] range
	// Remap this range in the matrix, anyway we are pixel-shader limited :)
	// ...... [ 2  0  0  0]
	// ...... [ 0  2  0  0]
	// ...... [ 0  0  2  0]
	// ...... [-1 -1 -1  1]
	// issue: strange, but it's slower :(
	// issue: interpolators? dp4? VS limited? black magic?

	// Feed this transform to pixel shader
	o.M1 = xform[0];
	o.M2 = xform[1];
	o.M3 = xform[2];

	float3 L_rgb 	= v.color.xyz;                                                // precalculated RGB lighting
	float3 L_hemi 	= v_hemi(N) * v.N.w;                                        // hemisphere
	float3 L_sun 	= v_sun(N) * v.color.w;                                        // sun
	float3 L_final 	= L_rgb + L_hemi + L_sun + L_ambient;

	o.hpos                 = mul                        (m_VP, P);                        // xform, input in world coords
	o.fog = saturate(calc_fogging  (v.P));

	o.c0 = float4		(L_final,o.fog);

//	Igor: for additional depth dest
#ifdef	USE_SOFT_WATER
#ifdef	NEED_SOFT_WATER
	o.tctexgen = mul( m_texgen, P);
	float3	Pe	= mul		(m_V,  P);
	o.tctexgen.z = Pe.z;
#endif	//	USE_SOFT_WATER
#endif	//	NEED_SOFT_WATER
        return o;
}
