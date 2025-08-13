#pragma once

enum{
	AF_GODMODE = (1 << 0),
	AF_NO_CLIP = (1 << 1),
	AF_ALWAYSRUN = (1 << 2),
	AF_UNLIMITEDAMMO = (1 << 3),
	AF_RUN_BACKWARD = (1 << 4),
	AF_AUTOPICKUP = (1 << 5),
	AF_PSP = (1 << 6),
	AF_DYNAMIC_MUSIC = (1 << 7),
	AF_GODMODE_RT = (1 << 8),
	AF_COLLISION = (1 << 10),
	AF_RIGHT_SHOULDER = (1 << 11),
	AF_3DSCOPE_ENABLE = (1 << 12),
	AF_PNV_W_SCOPE_DIS = (1 << 13),
	AF_SIMPLE_PDA = (1 << 14),
	AF_3D_PDA = (1 << 15),
	AF_CROUCH_TOGGLE = (1 << 16),
	AF_WPN_ZOOM_OUT_SHOOT = (1 << 17),
	AF_INV_ITEMCONDITION = (1 << 18),
	AF_UNLIMITED_BOLTS = (1 << 19),
	AF_USE_BATTERY = (1 << 20),
	AF_DBG_BATTERY_USE_CONSOLE = (1 << 21),		// for modding, you can see the battery uncharge in console
	AF_USE_FILTERS = (1 << 22),
};

extern Flags32 psActorFlags;
extern BOOL		GodMode	();	

extern int		psActorSleepTime;
extern int		psActorQuickSaveNumberCurrent;
extern int		psActorQuickSaveNumberMax;