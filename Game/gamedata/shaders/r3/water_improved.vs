
	#define WATER_VERTEX
	#include "common.h"
	#include "water_pv.h"
	#include "shared\waterconfig.h"
	#include "shared\watermove.h"

	#if MAIN_VOID_STYLE!=1
		wivs_out main(wivs_in _in)
	#else
		void main(in wivs_in _in, out wivs_out _out)
	#endif
	{
			#if MAIN_VOID_STYLE!=1
				wivs_out _out;
			#endif

			_in.N		=	unpack_D3DCOLOR(_in.N);
			_in.T		=	unpack_D3DCOLOR(_in.T);
			_in.B		=	unpack_D3DCOLOR(_in.B);
			_in.color	=	unpack_D3DCOLOR(_in.color);

			float4 P         = _in.P;

			P   = watermove(P);

			_out.position_w = float4(P.xyz, 1.0	);
			_out.v2point          = P - eye_position;
			_out.tbase			  = unpack_tc_base        		 (_in.uv,_in.T.w,_in.B.w);                // copy tc
			_out.tnorm0.xy        = watermove_tc                 (_out.tbase * 1.0f, P.xz, 0.1875f);
			_out.tnorm0.zw        = watermove_tc                 (_out.tbase * 1.1f, P.xz, 0.55f);

			_out.for_pr		 	  = abs(eye_position.y - P.y);

			// Calculate the 3x3 transform from tangent space to eye-space
			// TangentToEyeSpace = object2eye * tangent2object
			//                     = object2eye * transpose(object2tangent) (since the inverse of a rotation is its transpose)
			float3          N         = unpack_bx2(_in.N);        // just scale (assume normal in the -.5f, .5f)
			float3          T         = unpack_bx2(_in.T);        //
			float3          B         = unpack_bx2(_in.B);        //
			float3x3 xform        = mul        ((float3x3)m_W, float3x3(
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
			_out.M1                 = xform        [0];
			_out.M2                 = xform        [1];
			_out.M3                 = xform        [2];

			float3         L_rgb         = _in.color.xyz;                                                // precalculated RGB lighting
			float3         L_hemi         = v_hemi(N)*_in.N.w;                                        // hemisphere
			float3         L_sun         = v_sun(N)*_in.color.w;                                        // sun
			float3         L_final        = L_rgb + L_hemi + L_sun + L_ambient;

			_out.fog       = saturate( calc_fogging  (_in.P));

			_out.c0		= float4(L_final,1);

			_out.c1		= s_tonemap.Load(int3(0,0,0));

			_out.hpos = mul(m_VP, P);                        // xform, input in world coords

			_out.tctexgen = mul(m_texgen, P);
			_out.tctexgen.z = mul(m_V,  P).z;

			#if MAIN_VOID_STYLE!=1
				return _out;
			#endif
	}
