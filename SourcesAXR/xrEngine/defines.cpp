﻿#include "stdafx.h"

#ifdef DEBUG
	ECORE_API BOOL bDebug	= FALSE;
	
#endif

// Video
u32			psCurrentVidMode[2] = { 0, 0 };
u32			psCurrentBPP		= 32;
// release version always has "mt_*" enabled
Flags32		psDeviceFlags		= {rsFullscreen|rsDetails|mtPhysics|mtSound|mtNetwork|rsDrawStatic|rsDrawDynamic|rsRefresh60hz};

// textures 
int			psTextureLOD		= 1;
