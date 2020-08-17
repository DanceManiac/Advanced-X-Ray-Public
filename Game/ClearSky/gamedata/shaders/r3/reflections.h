#ifndef SSLR_INCLUDED
#define SSLR_INCLUDED

	#include "common.h"
	#include "hmodel.h"

	//--------------------------------------------------------------
	// Screen Space Reflections, moved to G.S.W.R. Addon by DWM Team, VK: https://vk.com/mihan323
	// Source: https://habr.com/ru/post/244367/
	//--------------------------------------------------------------

		// SSR_SAMPLES --> r4_ssr_samples, is integer

		#if 	((SSR_SAMPLES>3) && (SSR_SAMPLES<6)) 	// [4..5]
			#define SSR_QUALITY 1
		#elif 	((SSR_SAMPLES>5) && (SSR_SAMPLES<8)) 	// [6..7]
			#define SSR_QUALITY 2
		#elif 	((SSR_SAMPLES>7) && (SSR_SAMPLES<16))	// [8..15]
			#define SSR_QUALITY 3
		#elif 	((SSR_SAMPLES>15) && (SSR_SAMPLES<21))	// [16..20]
			#define SSR_QUALITY 4
		#else											// [0..3] or [20+] - not rendering SRR
			#undef SSR_QUALITY
		#endif

		#if defined(SSR_ROAD)
			float3 eye_direction;
			#define s_pptemp s_image
		#else
			#if defined(USE_MSAA)
				Texture2DMS <float4, MSAA_SAMPLES> s_pptemp;
			#else
				Texture2D <float3> s_pptemp; // For DX10.0
			#endif
		#endif

		#if defined(SSR_QUALITY)
			struct dt_ssr
			{
				float3 ssposi;
				float3 wposi;
				float3 wreflect;

				float tap;
				float error;

				float3 ssposi_tap;
				float2 tc;
			};

			void p_trace(inout dt_ssr ref)
			{
				ref.tc = ss_tc_from_wo_posi(ref.wposi + ref.wreflect * ref.tap); // Convert to screen space tc

				#if (SSR_QUALITY>3)
					ref.ssposi_tap = gbuf_get_position(ref.tc); // Read new screen space position

					if(ref.ssposi_tap.z == gbuf_transform_depth(ref.ssposi_tap.z)) // Compare depth, cut sky
						ref.error = 0;
				#else
					ref.ssposi_tap = gbuf_get_position(ref.tc, false); // Read new screen space position with fast sky depth
				#endif

				if((ref.tc.x > 1) || (ref.tc.x < 0) || (ref.tc.y > 1) || (ref.tc.y < 0) || (ref.ssposi_tap.z > 140)) // Fix sky/clamping artefacts
					ref.error = 0;

				ref.tap = length(ref.ssposi_tap - ref.ssposi); // Calc screen space position differnce, step of new sample

				#if (SSR_QUALITY==1)
					ref.error = (ref.tap > 12.5) ? (0) : (1);
				#endif
			}

			#if defined(SSR_ROAD)
				float4 depth_traced_ssr(in float3 wposi, in float3 wnorm, in float3 ssposi)
			#else
				float3 depth_traced_ssr(in float3 wposi, in float3 wnorm, in float3 pposi)
			#endif
			{
				#if !defined(SSR_ROAD)
					float3 ssposi = float3((pposi.xy * float2(2.0, -2.0) - float2(1.0, -1.0)) * InvFocalLen * pposi.z, pposi.z);
				#endif

				float3 to_point  = normalize(wposi - eye_position);

				// Fill refl struct (not all)
				dt_ssr	ref;
						ref.ssposi 		= ssposi;
						ref.wposi 		= wposi;
						ref.wreflect	= normalize(reflect(to_point, wnorm));
						ref.tap			= 0.25;
						ref.error		= 1;

				float sun = pow(abs(dot(normalize(to_point + L_sun_dir_w), wnorm)), 192.0) * 0.75; // Sun specular (LVutner)

				#if (SSR_QUALITY==1)
					float3 sky = sky_fake_reflection(sky_s0, sky_s1, ref.wreflect); // Get fast sky cubemap
				#else
					float3 sky = sky_true_reflection(sky_s0, sky_s1, ref.wreflect); // Get true sky cubemap
				#endif

				#if defined(SSR_ROAD)
					sky *= clamp(u_weather, 0, 0.5);
				#else
					sky *= u_weather.xyz;
				#endif

				// DT
				[unroll(SSR_SAMPLES)] for(int i = 0; i <= SSR_SAMPLES; i++)
					p_trace(ref); // Main target --> remaped tc

				ref.error *= saturate(2.8 * pow(1 + dot(eye_direction, wnorm), 2));

				#if (SSR_QUALITY>2)
					// Additional sky lerper (fog distance)
					ref.error *= 1 - saturate(pow(saturate(length(ref.ssposi_tap) * fog_params.w + fog_params.x),2));

					if((ssposi.z - ref.ssposi_tap.z) > 0) ref.error = 0; // Cut shit pixels !!!
				#endif

				#if defined(USE_MSAA) && !defined(SSR_ROAD)
					float3 reflection = s_pptemp.Load(ref.tc * screen_res_alt.xy, 0);
				#else
					float3 reflection = s_pptemp.Sample(smp_nofilter, ref.tc);
				#endif

				#if defined(SSR_ROAD)
					return float4((reflection * ref.error) + (sky * (1 - ref.error)), sun * (1 - ref.error));
				#else
					return (reflection * ref.error * 0.75 + (sky + (sun * L_sun_color.rgb)) * (1 - ref.error)) * 0.5; // Mix
				#endif
			}
		#endif
	//--------------------------------------------------------------
#endif