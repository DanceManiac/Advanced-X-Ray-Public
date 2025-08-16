// [ SETTINGS ] [ WATER SHADER - ICE VERSION ]

#define G_SSR_WATER_QUALITY				1		// Quality of the reflections. ( 0 = low | 1 = medium | 2 = high | 3 = Very High | 4 = Ultra )

#define G_SSR_ICE_TRANSPARENCY			0.5f	// How much you see through the ice. ( 0 = Solid | 1 = Liquid )
#define G_SSR_ICE_REFLECTION			0.3f	// Reflection intensity. ( 1.0f = 100% )
#define G_SSR_ICE_REFRACTION			1.0f	// Intensity of refraction distortion

#define G_SSR_ICE_BUMP_SIZE				6.0f	// Size of the bump texture
#define G_SSR_ICE_CRACKS_SIZE			5.0f	// Size of the ice cracks texture

#define G_SSR_ICE_BUMP_INTENSITY		2.0f	// Defines how much the ice surface interfere with the reflection, specular and lighting bounce.
#define G_SSR_ICE_CRACKS_INTESNITY		0.6f	// Intensity of the cracks

#define G_SSR_ICE_SKY_REFLECTION		1.0f	// Sky reflection factor. ( 1.0f = 100% )
#define G_SSR_ICE_MAP_REFLECTION		1.0f	// Objects reflection factor. ( 1.0f = 100% )

#define G_SSR_ICE_FRESNEL_COLOR_CLOSE	float3(0.6f,0.8f,1.0f)	// Tint for close ice surfaces ( 1,1,1 = White | 0,0,0 = Black )
#define G_SSR_ICE_FRESNEL_COLOR_FAR		float3(1.0f,1.0f,1.0f)	// Tint for far ice surfaces  ( 1,1,1 = White | 0,0,0 = Black )

#define G_SSR_ICE_USE_PARALLAX			true	// Use parallax to simulate ice tickness
#define G_SSR_ICE_PARALLAX_STEPS		6		// Number of steps to create the effect. More steps will smooth the result.
#define G_SSR_ICE_PARALLAX_DEPTH 		1.0f	// Depth of the effect
#define G_SSR_ICE_PARALLAX_INTENSITY 	0.8f	// Effect intensity

#define G_SSR_ICE_SPECULAR_INTENSITY	1.0f	// Sun/Moon specular intensity
#define G_SSR_ICE_SPECULAR_VIBRANCE		0.6f	// Adjust the vibrance of the specular color ( 0 = Grayscale | 1 = Full Color )

#define G_SSR_ICE_BORDER				0.25f	// Ice border factor. ( 0.0f = hard edge )