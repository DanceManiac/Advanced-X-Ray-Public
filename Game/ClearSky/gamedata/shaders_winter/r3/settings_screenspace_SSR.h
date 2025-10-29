// [ SETTINGS ] [ SCREEN SPACE REFLECTIONS ]

#define G_SSR_VERTICAL_SCREENFADE		4.0f	// Vertical fade. ( higher values = sharp gradient )

#define G_SSR_MAX_INTENSITY				0.1f	// Global reflection MAX intensity.
#define G_SSR_FLORA_INTENSITY 			0.2f	// Adjust grass and tree branches intensity
#define G_SSR_WEAPON_MAX_LENGTH			1.3f	// Maximum distance to apply G_SSR_WEAPON_INTENSITY.
#define G_SSR_WEAPON_RAIN_FACTOR		0.2f	// Weapons reflections boost when raining ( 0 to disable ) ( Affected by rain intensity )
#define G_SSR_FALLOFF_RATE				0.15f	// Controls the rate of reflection attenuation as you move away from the camera. Smaller values will make the attenuation slower, and larger values will make it faster.

#if defined(SSFX_WPN_SSR_ALLWAYS)
	#define G_SSR_WEAPON_REFLECT_ONLY_WITH_RAIN		// Uncomment this if you don't want weapon reflections unless is raining
#endif

//#define G_SSR_CHEAP_SKYBOX						// Use a faster skybox to improve some performance.