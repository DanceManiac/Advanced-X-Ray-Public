////////////////////////////////////////////////////////////////////////////
//	Module 		: stalker_death_actions.cpp
//	Created 	: 25.03.2004
//  Modified 	: 26.03.2004
//	Author		: Dmitriy Iassenev
//	Description : Stalker death action classes
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "stalker_death_actions.h"
#include "ai/stalker/ai_stalker.h"
#include "stalker_decision_space.h"
#include "script_game_object.h"
#include "movement_manager_space.h"
#include "detail_path_manager_space.h"
#include "stalker_movement_manager_smart_cover.h"
#include "inventory.h"
#include "weapon.h"
#include "xr_level_controller.h"
#include "clsid_game.h"
#include "characterphysicssupport.h"
#include "weaponmagazined.h"

using namespace StalkerDecisionSpace;

//////////////////////////////////////////////////////////////////////////
// CStalkerActionDead
//////////////////////////////////////////////////////////////////////////

CStalkerActionDead::CStalkerActionDead	(CAI_Stalker *object, LPCSTR action_name) :
	inherited							(object,action_name)
{
}

bool CStalkerActionDead::fire			() const
{
	if (object().inventory().TotalWeight() <= 0)
		return							(false);
	
	CWeapon								*weapon = smart_cast<CWeapon*>(object().inventory().ActiveItem());
	if (!weapon)
		return							(false);

	if (!weapon->GetAmmoElapsed())
		return							(false);

	if (!object().hammer_is_clutched())
		return							(false);

	if (!object().character_physics_support()->can_drop_active_weapon())
		return							(true);

	if (Device.dwTimeGlobal - object().GetLevelDeathTime() > 500)
		return							(false);

	return								(true);
}

void CStalkerActionDead::initialize		()
{
	inherited::initialize				();

	if (object().getDestroy())
		return;

	if (!fire())
		return;

	object().inventory().Action			(kWPN_FIRE,CMD_START);

	int active_slot						= object().inventory().GetActiveSlot();
	if (active_slot == 2) {
		CInventoryItem*					item = object().inventory().m_slots[active_slot].m_pIItem;
		if (item) {
			CWeaponMagazined*			weapon = smart_cast<CWeaponMagazined*>(item);
			VERIFY						(weapon);
			weapon->SetQueueSize		(weapon->GetAmmoElapsed());
		}
	}

	typedef xr_vector<CInventorySlot>	SLOTS;

	SLOTS::iterator						I = object().inventory().m_slots.begin(), B = I;
	SLOTS::iterator						E = object().inventory().m_slots.end();
	for ( ; I != E; ++I) {
		if ((I - B) == (int)object().inventory().GetActiveSlot())
			continue;

		if (!(*I).m_pIItem)
			continue;

		if ((*I).m_pIItem->object().CLS_ID == CLSID_IITEM_BOLT)
			continue;

		object().inventory().Ruck		((*I).m_pIItem);
	}
}

void CStalkerActionDead::execute		()
{
	inherited::execute					();

	if (object().getDestroy())
		return;

	object().movement().enable_movement	(false);

	if (fire())
		return;

	if (!object().character_physics_support()->can_drop_active_weapon())
		return;

	typedef xr_vector<CInventorySlot>	SLOTS;

	SLOTS::iterator						I = object().inventory().m_slots.begin(), B = I;
	SLOTS::iterator						E = object().inventory().m_slots.end();
	for ( ; I != E; ++I) {
		if (!(*I).m_pIItem)
			continue;
		
		if ((*I).m_pIItem->object().CLS_ID == CLSID_IITEM_BOLT)
			continue;

		if ((I - B) == (int)object().inventory().GetActiveSlot()) {
			(*I).m_pIItem->SetDropManual	(TRUE);
			continue;
		}

		object().inventory().Ruck		((*I).m_pIItem);
	}

	set_property						(eWorldPropertyDead,true);
}
