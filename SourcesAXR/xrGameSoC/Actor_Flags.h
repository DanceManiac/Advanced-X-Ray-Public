#pragma once

enum{
		AF_GODMODE			=(1<<0),
		AF_INVISIBLE		=(1<<1),
		AF_ALWAYSRUN		=(1<<2),
		AF_UNLIMITEDAMMO	=(1<<3),
		AF_RUN_BACKWARD		=(1<<4),
		AF_AUTOPICKUP		=(1<<5),
		AF_PSP				=(1<<6),
		AF_COLLISION		=(1<<10),
		AF_RIGHT_SHOULDER	=(1<<11),
		AF_3DSCOPE_ENABLE	=(1<<12),
		AF_PNV_W_SCOPE_DIS	=(1<<13),
		AF_CROUCH_TOGGLE	=(1<<14),
		AF_FP_DEATH			=(1<<15),
};

extern Flags32 psActorFlags;

extern BOOL		GodMode	();	

extern int		psActorSleepTime;
