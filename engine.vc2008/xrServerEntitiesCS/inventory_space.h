#pragma once

#define CMD_START	(1<<0)
#define CMD_STOP	(1<<1)

#define NO_ACTIVE_SLOT		0xffffffff
#define KNIFE_SLOT			0
#define PISTOL_SLOT			1
#define RIFLE_SLOT			2
#define GRENADE_SLOT		3
#define APPARATUS_SLOT		4
#define BOLT_SLOT			5
#define OUTFIT_SLOT			6
#define PDA_SLOT			7
#define DETECTOR_SLOT		8
#define TORCH_SLOT			9
#define ARTEFACT_SLOT		10
#define FLARE_SLOT			11
//.#define SLOTS_TOTAL			12


#define RUCK_HEIGHT			280
#define RUCK_WIDTH			7

class CInventoryItem;
class CInventory;

typedef CInventoryItem*				PIItem;
typedef xr_vector<PIItem>			TIItemContainer;


enum EItemPlace
{			
	EItemPlaceUndefined,
	EItemPlaceSlot,
	EItemPlaceBelt,
	EItemPlaceRuck
};

extern u32	INV_STATE_LADDER;
extern u32	INV_STATE_CAR;
extern u32	INV_STATE_BLOCK_ALL;
extern u32	INV_STATE_INV_WND;
extern u32	INV_STATE_BUY_MENU;
