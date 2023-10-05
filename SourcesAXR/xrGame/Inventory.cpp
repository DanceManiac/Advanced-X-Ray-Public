#include "pch_script.h"
#include "inventory.h"
#include "actor.h"
#include "CustomOutfit.h"
#include "trade.h"
#include "weapon.h"

#include "ui/UIInventoryUtilities.h"
#include "ui/UIActorMenu.h"

#include "eatable_item.h"
#include "script_engine.h"
#include "xrmessages.h"
#include "xr_level_controller.h"
#include "level.h"
#include "ai_space.h"
#include "entitycondition.h"
#include "game_base_space.h"
#include "uigamecustom.h"
#include "clsid_game.h"
#include "static_cast_checked.hpp"
#include "player_hud.h"
#include "ai/stalker/ai_stalker.h"
#include "weaponmagazined.h"
#include "../xrPhysics/ElevatorState.h"
#include "CustomDetector.h"
#include "CustomBackpack.h"

using namespace InventoryUtilities;

extern bool g_block_all_except_movement;

std::atomic<bool> isHidingInProgressInv(false);
std::atomic<bool> TakeItemAnimNeeded(false);

// what to block
u16	INV_STATE_BLOCK_ALL		= 0xffff;
u16	INV_STATE_LADDER		= INV_STATE_BLOCK_ALL;
u16	INV_STATE_CAR			= INV_STATE_BLOCK_ALL;
u16	INV_STATE_INV_WND		= INV_STATE_BLOCK_ALL;
u16	INV_STATE_BUY_MENU		= INV_STATE_BLOCK_ALL;
u16 INV_STATE_HIDE_WEAPON	= (1 << KNIFE_SLOT | 1 << INV_SLOT_2 | 1 << INV_SLOT_3 | 1 << GRENADE_SLOT |  1 << PISTOL_SLOT);

CInventorySlot::CInventorySlot() 
{
	m_pIItem				= NULL;
	m_bAct					= true;
	m_bPersistent			= false;
}

CInventorySlot::~CInventorySlot() 
{
}

bool CInventorySlot::CanBeActivated() const 
{
	return (m_bAct);
};

CInventory::CInventory() 
{
	m_fMaxWeight								= pSettings->r_float	("inventory","max_weight");
	
	ReloadInv();
	
	m_iActiveSlot								= NO_ACTIVE_SLOT;
	m_iNextActiveSlot							= NO_ACTIVE_SLOT;
	m_iPrevActiveSlot							= NO_ACTIVE_SLOT;

	/*string256 temp;
	for(u16 i=FirstSlot(); i<=LastSlot(); ++i ) 
	{
		xr_sprintf(temp, "slot_persistent_%d", i);
		m_slots[i].m_bPersistent = !!pSettings->r_bool("inventory",temp);

		xr_sprintf			(temp, "slot_active_%d", i);
		m_slots[i].m_bAct	= !!pSettings->r_bool("inventory",temp);
	};*/

	m_bSlotsUseful								= true;
	m_bBeltUseful								= false;

	m_fTotalWeight								= -1.f;
	m_dwModifyFrame								= 0;
	m_drop_last_frame							= false;

	m_bTakeItemActivated						= false;
	m_bItemTaked								= false;
	m_iTakeAnimLength							= 0;
	m_iActionTiming								= 0;
	Object										= nullptr;
	
	InitPriorityGroupsForQSwitch				();
	m_next_item_iteration_time					= 0;

	for (u16 i = 0; i < LAST_SLOT+1; ++i)
	{
		m_blocked_slots[i] = 0;
	}
}

void CInventory::ReloadInv()
{
	m_slots.clear();

	u32 sz = pSettings->r_s32("inventory", "slots_count");
	m_slots.resize(sz + 1); //first is [1]

	string256 temp;
	for (u16 i = FirstSlot(); i <= LastSlot(); ++i)
	{
		xr_sprintf(temp, "slot_persistent_%d", i);
		m_slots[i].m_bPersistent = !!pSettings->r_bool("inventory", temp);

		xr_sprintf(temp, "slot_active_%d", i);
		m_slots[i].m_bAct = !!pSettings->r_bool("inventory", temp);
	};
}

CInventory::~CInventory() 
{
}

void CInventory::Clear()
{
	m_all.clear							();
	m_ruck.clear						();
	m_belt.clear						();
	
	for(u16 i=FirstSlot(); i<=LastSlot(); i++)
		m_slots[i].m_pIItem				= NULL;

	m_pOwner							= NULL;

	ReloadInv							();
	CalcTotalWeight						();
	InvalidateState						();
}

void CInventory::TakeItemAnimCheck(CGameObject* GameObj, CObject* Obj, bool use_pickup_anim)
{
	if (Actor()->m_bActionAnimInProcess)
		return;

	GameObject = GameObj;
	Object = Obj;
	m_bUsePickupAnim = use_pickup_anim;

	if (isHidingInProgressInv.load())
		return;

	CCustomDetector* pDet = smart_cast<CCustomDetector*>(Actor()->inventory().ItemFromSlot(DETECTOR_SLOT));

	if (!pDet || pDet->IsHidden())
	{
		TakeItemAnim(GameObj, Obj, use_pickup_anim);
		return;
	}
	else
	{
		if (!use_pickup_anim)
			TakeItemAnim(GameObj, Obj, use_pickup_anim);

		if (!pSettings->line_exist("actions_animations", "take_item_section"))
			return;
	}

	isHidingInProgressInv.store(true);

	std::thread hidingThread([&, pDet]
		{
			while (pDet && !pDet->IsHidden())
				pDet->HideDetector(true);

			isHidingInProgressInv.store(false);
			TakeItemAnimNeeded.store(true);
		});

	hidingThread.detach();
}

void CInventory::TakeItemAnim(CGameObject* GameObj, CObject* Obj, bool use_pickup_anim)
{
	LPCSTR anim_sect = READ_IF_EXISTS(pAdvancedSettings, r_string, "actions_animations", "take_item_section", nullptr);

	if (!anim_sect || !use_pickup_anim)
	{
		Obj->H_SetParent(smart_cast<CObject*>(Actor()));
		Take(GameObj, false, true);
		return;
	}

	Object = GameObj;

	CWeapon* Wpn = smart_cast<CWeapon*>(ActiveItem());

	if (Wpn && !(Wpn->GetState() == CWeapon::eIdle))
		return;

	m_bTakeItemActivated = true;

	int anim_timer = READ_IF_EXISTS(pSettings, r_u32, anim_sect, "anim_timing", 0);

	g_block_all_except_movement = true;
	g_actor_allow_ladder = false;

	LPCSTR use_cam_effector = READ_IF_EXISTS(pSettings, r_string, anim_sect, !Wpn ? "anim_camera_effector" : "anim_camera_effector_weapon", nullptr);
	float effector_intensity = READ_IF_EXISTS(pSettings, r_float, anim_sect, "cam_effector_intensity", 1.0f);
	float anim_speed = READ_IF_EXISTS(pSettings, r_float, anim_sect, "anim_speed", 1.0f);

	if (pSettings->line_exist(anim_sect, "anm_use"))
	{
		g_player_hud->script_anim_play(!GetActiveSlot() ? 2 : 1, anim_sect, !Wpn ? "anm_use" : "anm_use_weapon", true, anim_speed, GameObj->cNameVisual().c_str());

		if (use_cam_effector)
			g_player_hud->PlayBlendAnm(use_cam_effector, 0, anim_speed, effector_intensity, false);

		m_iTakeAnimLength = Device.dwTimeGlobal + g_player_hud->motion_length_script(anim_sect, !Wpn ? "anm_use" : "anm_use_weapon", anim_speed);
	}

	if (pSettings->line_exist(anim_sect, "snd_using"))
	{
		if (m_action_anim_sound._feedback())
			m_action_anim_sound.stop();

		shared_str snd_name = pSettings->r_string(anim_sect, "snd_using");
		m_action_anim_sound.create(snd_name.c_str(), st_Effect, sg_SourceType);
		m_action_anim_sound.play(NULL, sm_2D);
	}

	m_iActionTiming = Device.dwTimeGlobal + anim_timer;

	m_bItemTaked = false;
	Actor()->m_bActionAnimInProcess = true;
}

void CInventory::UpdateUseAnim(CActor* actor)
{
	if (TakeItemAnimNeeded.load())
	{
		TakeItemAnim(GameObject, Object, m_bUsePickupAnim);
		TakeItemAnimNeeded.store(false);
	}

	if (!m_bTakeItemActivated)
		return;

	if (!Actor()->m_bActionAnimInProcess)
		return;

	bool IsActorAlive = g_pGamePersistent->GetActorAliveStatus();

	if ((m_iActionTiming <= Device.dwTimeGlobal && !m_bItemTaked) && IsActorAlive)
	{
		m_iActionTiming = Device.dwTimeGlobal;

		bool vis_status = READ_IF_EXISTS(pSettings, r_bool, Object->cNameSect(), "visible_with_take_anim", true);

		g_player_hud->SetScriptItemVisible(vis_status);

		Object->H_SetParent(smart_cast<CObject*>(actor));
		Take(GameObject, false, true);

		m_bItemTaked = true;
	}

	if (m_bTakeItemActivated)
	{
		if ((m_iTakeAnimLength <= Device.dwTimeGlobal) || !IsActorAlive)
		{
			m_iTakeAnimLength = Device.dwTimeGlobal;
			m_iActionTiming = Device.dwTimeGlobal;
			m_action_anim_sound.stop();
			g_block_all_except_movement = false;
			g_actor_allow_ladder = true;
			actor->m_bActionAnimInProcess = false;
			m_bTakeItemActivated = false;
		}
	}
}

void CInventory::Take(CGameObject *pObj, bool bNotActivate, bool strict_placement)
{
	CInventoryItem *pIItem				= smart_cast<CInventoryItem*>(pObj);
	VERIFY								(pIItem);
	VERIFY								(pIItem->m_pInventory==NULL);
	VERIFY								(CanTakeItem(pIItem));
	
	pIItem->m_pInventory				= this;
	pIItem->SetDropManual				(FALSE);
	pIItem->AllowTrade					();
	//if net_Import for pObj arrived then the pObj will pushed to CrPr list (correction prediction)
	//usually net_Import arrived for objects that not has a parent object..
	//for unknown reason net_Import arrived for object that has a parent, so correction prediction schema will crash
	Level().RemoveObject_From_4CrPr		(pObj);

	if (Level().CurrentEntity()) {
		std::uint16_t actor_id = Level().CurrentEntity()->ID();
		if (GetOwner()->object_id() == actor_id &&
			this->m_pOwner->object_id() == actor_id) // actors inventory
		{
			CWeaponMagazined* pWeapon = smart_cast<CWeaponMagazined*>(pIItem);
			if (pWeapon && pWeapon->strapped_mode()) {
				pWeapon->strapped_mode(false);
				Ruck(pWeapon);
			}
		}
	}

	m_all.push_back						(pIItem);

	if(!strict_placement)
		pIItem->m_ItemCurrPlace.type	= eItemPlaceUndefined;

	bool result							= false;
	switch(pIItem->m_ItemCurrPlace.type)
	{
	case eItemPlaceBelt:
		result							= Belt(pIItem, strict_placement); 
		if(!result)
			pIItem->m_ItemCurrPlace.type	= eItemPlaceUndefined;
#ifdef DEBUG
		if(!result) 
			Msg("cant put in belt item %s", *pIItem->object().cName());
#endif

		break;
	case eItemPlaceRuck:
		result							 = Ruck(pIItem, strict_placement);
		if(!result)
			pIItem->m_ItemCurrPlace.type = eItemPlaceUndefined;
#ifdef DEBUG
		if(!result) 
			Msg("cant put in ruck item %s", *pIItem->object().cName());
#endif

		break;
	case eItemPlaceSlot:
		result							= Slot(pIItem->m_ItemCurrPlace.slot_id, pIItem, bNotActivate, strict_placement); 
		if(!result)
			pIItem->m_ItemCurrPlace.type = eItemPlaceUndefined;
#ifdef DEBUG
		if(!result) 
			Msg("cant slot in slot item %s", *pIItem->object().cName());
#endif
		break;
	}

	if(pIItem->CurrPlace()==eItemPlaceUndefined)
	{
		if( !pIItem->RuckDefault() )
		{
			if( CanPutInSlot(pIItem, pIItem->BaseSlot()) )
			{
				result						= Slot(pIItem->BaseSlot(), pIItem, bNotActivate,strict_placement); VERIFY(result);
			}else
				if (CanPutInBelt(pIItem))
				{
					result					= Belt(pIItem,strict_placement); VERIFY(result);
				}else
				{
					result					= Ruck(pIItem,strict_placement); VERIFY(result);
				}
		}else
		{
			result						= Ruck(pIItem,strict_placement); VERIFY(result);
		}
	}
	
	m_pOwner->OnItemTake				(pIItem);

	CalcTotalWeight						();
	InvalidateState						();

	pIItem->object().processing_deactivate();
	VERIFY								(pIItem->CurrPlace() != eItemPlaceUndefined);


	if( CurrentGameUI() )
	{
		CObject* pActor_owner = smart_cast<CObject*>(m_pOwner);

		if (Level().CurrentViewEntity() == pActor_owner)
		{
			CurrentGameUI()->OnInventoryAction(pIItem, GE_OWNERSHIP_TAKE);

			if (pIItem->CurrPlace() == eItemPlaceRuck)
				Actor()->ChangeInventoryFullness(pIItem->GetOccupiedInvSpace());
		}
		else if(CurrentGameUI()->ActorMenu().GetMenuMode()==mmDeadBodySearch)
		{
			if (m_pOwner == CurrentGameUI()->ActorMenu().GetPartner())
			{
				CurrentGameUI()->OnInventoryAction(pIItem, GE_OWNERSHIP_TAKE);

				if (pIItem->CurrPlace() == eItemPlaceRuck)
					Actor()->ChangeInventoryFullness(pIItem->GetOccupiedInvSpace());
			}
		}
	};
}

bool CInventory::DropItem(CGameObject *pObj, bool just_before_destroy, bool dont_create_shell) 
{
	CInventoryItem *pIItem				= smart_cast<CInventoryItem*>(pObj);
	VERIFY								(pIItem);
	VERIFY								(pIItem->m_pInventory);
	VERIFY								(pIItem->m_pInventory==this);
	VERIFY								(pIItem->m_ItemCurrPlace.type!=eItemPlaceUndefined);
	
	pIItem->object().processing_activate(); 
	
	switch(pIItem->CurrPlace())
	{
	case eItemPlaceBelt:{
			VERIFY(InBelt(pIItem));
			TIItemContainer::iterator temp_iter = std::find(m_belt.begin(), m_belt.end(), pIItem);
			if (temp_iter != m_belt.end())
			{
				m_belt.erase(temp_iter);
			} else
			{
				Msg("! ERROR: CInventory::Drop item not found in belt...");
			}
			pIItem->object().processing_deactivate();
		}break;
	case eItemPlaceRuck:{
			VERIFY(InRuck(pIItem));
			TIItemContainer::iterator temp_iter = std::find(m_ruck.begin(), m_ruck.end(), pIItem);
			if (temp_iter != m_ruck.end())
			{
				m_ruck.erase(temp_iter);
			} else
			{
				Msg("! ERROR: CInventory::Drop item not found in ruck...");
			}
		}break;
	case eItemPlaceSlot:{
			VERIFY			(InSlot(pIItem));
			if(m_iActiveSlot == pIItem->CurrSlot())
			{
				CActor* pActor	= smart_cast<CActor*>(m_pOwner);
				if (!pActor || pActor->g_Alive())
				{
					if (just_before_destroy)
					{
#ifdef DEBUG
						Msg("---DropItem activating slot [-1], forced, Frame[%d]", Device.dwFrame);
#endif // #ifdef DEBUG
						Activate		(NO_ACTIVE_SLOT, true);
					} else 
					{
#ifdef DEBUG
						Msg("---DropItem activating slot [-1], Frame[%d]", Device.dwFrame);
#endif // #ifdef DEBUG
						Activate		(NO_ACTIVE_SLOT);
					}
				}
			}
			m_slots[pIItem->CurrSlot()].m_pIItem = NULL;							
			pIItem->object().processing_deactivate();
		}break;
	default:
		NODEFAULT;
	};
	TIItemContainer::iterator it = std::find(m_all.begin(), m_all.end(), pIItem);
	if(it!=m_all.end())
		m_all.erase(std::find(m_all.begin(), m_all.end(), pIItem));
	else
		Msg("! CInventory::Drop item not found in inventory!!!");

	pIItem->m_pInventory = NULL;


	m_pOwner->OnItemDrop	(smart_cast<CInventoryItem*>(pObj), just_before_destroy);

	CalcTotalWeight					();
	InvalidateState					();
	m_drop_last_frame				= true;

	if( CurrentGameUI() )
	{
		CObject* pActor_owner = smart_cast<CObject*>(m_pOwner);

		if (Level().CurrentViewEntity() == pActor_owner)
		{
			CurrentGameUI()->OnInventoryAction(pIItem, GE_OWNERSHIP_REJECT);

			if (pIItem->CurrPlace() == eItemPlaceRuck)
				Actor()->ChangeInventoryFullness(-pIItem->GetOccupiedInvSpace());
		}
	};
	pObj->H_SetParent(0, dont_create_shell);
	return							true;
}

//положить вещь в слот
bool CInventory::Slot(u16 slot_id, PIItem pIItem, bool bNotActivate, bool strict_placement) 
{
	VERIFY(pIItem);
	
	if(ItemFromSlot(slot_id) == pIItem)
		return false;

	if (ItemFromSlot(slot_id) && pIItem->CurrPlace() == eItemPlaceSlot && pIItem->CurrSlot() == slot_id)
		return false;

//.	Msg("To Slot %s[%d]", *pIItem->object().cName(), pIItem->object().ID());

	if(!strict_placement && !CanPutInSlot(pIItem,slot_id)) 
	{
#ifdef _DEBUG
		Msg("there is item %s[%d,%x] in slot %d[%d,%x]", 
				ItemFromSlot(pIItem->CurrSlot())->object().cName().c_str(), 
				ItemFromSlot(pIItem->CurrSlot())->object().ID(), 
				ItemFromSlot(pIItem->CurrSlot()), 
				pIItem->CurrSlot(), 
				pIItem->object().ID(),
				pIItem);
#endif
//.		if(m_slots[pIItem->GetSlot()].m_pIItem == pIItem && !bNotActivate )
//.			Activate(pIItem->GetSlot());

		return false;
	}

	m_slots[slot_id].m_pIItem = pIItem;
	
	//удалить из рюкзака или пояса
	TIItemContainer::iterator it_ruck = std::find(m_ruck.begin(), m_ruck.end(), pIItem);
	TIItemContainer::iterator it_belt = std::find(m_belt.begin(), m_belt.end(), pIItem);
	if (!IsGameTypeSingle())
	{
		if (it_ruck != m_ruck.end())
		{
			m_ruck.erase(it_ruck);
			R_ASSERT(it_belt == m_belt.end());
		} else if(it_belt != m_belt.end())
		{
			m_belt.erase(it_belt);
			R_ASSERT(it_ruck == m_ruck.end());
		} else
		{
			u16 real_parent = pIItem->object().H_Parent() ? pIItem->object().H_Parent()->ID() : u16(-1);
			R_ASSERT2(GetOwner()->object_id() == real_parent,
				make_string("! ERROR: CL: actor [%d] doesn't contain [%d], real parent is [%d]", 
					GetOwner()->object_id(), pIItem->object_id(), real_parent).c_str()
			);
		}
#ifdef MP_LOGGING
		Msg("--- Actor [%d] places to slot item [%d]", GetOwner()->object_id(), pIItem->object_id());
#endif //#ifdef MP_LOGGING
	} else
	{
		if (it_ruck != m_ruck.end())
			m_ruck.erase(it_ruck);
		if (it_belt != m_belt.end())
			m_belt.erase(it_belt);
	}

	bool in_slot = InSlot(pIItem);
	if(in_slot && (pIItem->CurrSlot()!=slot_id)) 
	{
		if(GetActiveSlot() == pIItem->CurrSlot()) 
			Activate(NO_ACTIVE_SLOT);

		m_slots[pIItem->CurrSlot()].m_pIItem = NULL;
	}

	if (((m_iActiveSlot==slot_id) ||(m_iActiveSlot==NO_ACTIVE_SLOT) && m_iNextActiveSlot==NO_ACTIVE_SLOT) && (!bNotActivate))
	{
#ifdef DEBUG
		Msg("---To Slot: activating slot [%d], Frame[%d]", slot_id, Device.dwFrame);
#endif // #ifdef DEBUG
		Activate				(slot_id);
	}
	SInvItemPlace prev_place		= pIItem->m_ItemCurrPlace;
	m_pOwner->OnItemSlot			(pIItem, pIItem->m_ItemCurrPlace);
	pIItem->m_ItemCurrPlace.type	= eItemPlaceSlot;
	pIItem->m_ItemCurrPlace.slot_id = slot_id;
	pIItem->OnMoveToSlot			(prev_place);
	
	if (prev_place.type == eItemPlaceRuck)
		Actor()->ChangeInventoryFullness(-pIItem->GetOccupiedInvSpace());

	pIItem->object().processing_activate();

	return						true;
}

bool CInventory::Belt(PIItem pIItem, bool strict_placement) 
{
	if(!strict_placement && !CanPutInBelt(pIItem))	return false;
	
	//вещь была в слоте
	bool in_slot = InSlot(pIItem);
	if(in_slot) 
	{
		if(GetActiveSlot() == pIItem->CurrSlot()) 
			Activate(NO_ACTIVE_SLOT);

		m_slots[pIItem->CurrSlot()].m_pIItem = NULL;
	}
	
	m_belt.insert(m_belt.end(), pIItem); 

	if(!in_slot)
	{
		TIItemContainer::iterator it = std::find(m_ruck.begin(), m_ruck.end(), pIItem); 
		if(m_ruck.end() != it) 
			m_ruck.erase(it);
	}

	CalcTotalWeight					();
	InvalidateState					();

	SInvItemPlace prev_place		= pIItem->m_ItemCurrPlace;
	pIItem->m_ItemCurrPlace.type	= eItemPlaceBelt;
	m_pOwner->OnItemBelt			(pIItem, prev_place);
	pIItem->OnMoveToBelt			(prev_place);

	if (prev_place.type == eItemPlaceRuck)
		Actor()->ChangeInventoryFullness(-pIItem->GetOccupiedInvSpace());

	if(in_slot)
		pIItem->object().processing_deactivate();

	pIItem->object().processing_activate();

	return true;
}

bool CInventory::Ruck(PIItem pIItem, bool strict_placement) 
{
	if(!strict_placement && !CanPutInRuck(pIItem)) return false;

	if (!IsGameTypeSingle())
	{
		u16 real_parent = pIItem->object().H_Parent() ? pIItem->object().H_Parent()->ID() : u16(-1);
		if (GetOwner()->object_id() != real_parent)
		{
			Msg("! WARNING: CL: actor [%d] tries to place to ruck not own item [%d], that has parent [%d]",
				GetOwner()->object_id(), pIItem->object_id(), real_parent);
			return false;
		}
	}
	
	bool in_slot = InSlot(pIItem);
	//вещь была в слоте
	if(in_slot) 
	{
		if(GetActiveSlot() == pIItem->CurrSlot()) 
			Activate(NO_ACTIVE_SLOT);

		m_slots[pIItem->CurrSlot()].m_pIItem = NULL;
	}else
	{
		//вещь была на поясе или вообще только поднята с земли
		TIItemContainer::iterator it = std::find(m_belt.begin(), m_belt.end(), pIItem); 
		if(m_belt.end() != it) 
			m_belt.erase(it);

		if (!IsGameTypeSingle())
		{
			u16 item_parent_id = pIItem->object().H_Parent() ? pIItem->object().H_Parent()->ID() : u16(-1) ;
			u16 inventory_owner_id = GetOwner()->object_id();
			R_ASSERT2(item_parent_id == inventory_owner_id,
				make_string("! ERROR: CL: Actor[%d] tries to place to ruck not own item [%d], real item owner is [%d]",
				inventory_owner_id, pIItem->object_id(), item_parent_id).c_str()
			);
#ifdef MP_LOGGING
			Msg("--- Actor [%d] place to ruck item [%d]", inventory_owner_id, pIItem->object_id());
#endif
		}
	}
	
	m_ruck.insert									(m_ruck.end(), pIItem); 
	
	CalcTotalWeight									();
	InvalidateState									();

	m_pOwner->OnItemRuck							(pIItem, pIItem->m_ItemCurrPlace);
	SInvItemPlace prev_place						= pIItem->m_ItemCurrPlace;
	pIItem->m_ItemCurrPlace.type					= eItemPlaceRuck;
	pIItem->OnMoveToRuck							(prev_place);

	if (prev_place.type == eItemPlaceSlot || prev_place.type == eItemPlaceBelt)
		Actor()->ChangeInventoryFullness(-pIItem->GetOccupiedInvSpace());

	if(in_slot)
		pIItem->object().processing_deactivate();

	return true;
}
/*
void CInventory::Activate_deffered	(u32 slot, u32 _frame)
{
	 m_iLoadActiveSlot			= slot;
	 m_iLoadActiveSlotFrame		= _frame;
}*/


void CInventory::Activate(u16 slot, bool bForce) 
{	
	if(!OnServer())
	{
		return;
	}

	PIItem tmp_item = NULL;
	if (slot != NO_ACTIVE_SLOT)
		tmp_item = ItemFromSlot(slot);

	if (tmp_item && IsSlotBlocked(tmp_item) && (!bForce))
	{
		//to restore after unblocking ...
		SetPrevActiveSlot(slot);
		return;
	}

	if (GetActiveSlot()==slot || (GetNextActiveSlot()==slot && !bForce))
	{
//		if (m_iNextActiveSlot != slot) {
//			LPCSTR const name = smart_cast<CGameObject const*>(m_pOwner)->cName().c_str();
//			if ( !xr_strcmp("jup_b43_stalker_assistant_pri6695", name) )
//				LogStackTrace	("");
//			Msg				( "[%6d][%s] CInventory::Activate changing next active slot to %d", Device.dwTimeGlobal, name, slot );
//		}
		m_iNextActiveSlot=slot;
#ifdef DEBUG
//		Msg("--- There's no need to activate slot [%d], next active slot is [%d]", slot, m_iNextActiveSlot);
#endif
		return;
	}

	R_ASSERT2(slot<=LastSlot(), "wrong slot number");

	if (slot != NO_ACTIVE_SLOT && !m_slots[slot].CanBeActivated()) 
		return;

#ifdef DEBUG
//	Msg("--- Activating slot [%d], inventory owner: [%s], Frame[%d]", slot, m_pOwner->Name(), Device.dwFrame);
#endif // #ifdef DEBUG
	
	//активный слот не выбран
	if (GetActiveSlot() == NO_ACTIVE_SLOT)
	{
		if (tmp_item)
		{
//			if ( m_iNextActiveSlot != slot) {
//				LPCSTR const name = smart_cast<CGameObject const*>(m_pOwner)->cName().c_str();
//				if ( !xr_strcmp("jup_b43_stalker_assistant_pri6695", name) )
//					LogStackTrace	("");
//				Msg				( "[%6d][%s] CInventory::Activate changing next active slot2 to %d", Device.dwTimeGlobal, name, slot );
//			}
			m_iNextActiveSlot		= slot;
		}
		else 
		{
			if(slot==GRENADE_SLOT)//fake for grenade
			{
				PIItem gr = SameSlot(GRENADE_SLOT, NULL, true);
				if(gr)
					Slot(GRENADE_SLOT,gr);
			}
		}
	}
	//активный слот задействован
	else if (slot==NO_ACTIVE_SLOT || tmp_item)
	{
		PIItem active_item = ActiveItem();
		if(active_item && !bForce)
		{
			CHudItem* tempItem = active_item->cast_hud_item();
			R_ASSERT2(tempItem, active_item->object().cNameSect().c_str());
			
			tempItem->SendDeactivateItem();
#ifdef DEBUG
//			Msg("--- Inventory owner [%s]: send deactivate item [%s]", m_pOwner->Name(), active_item->NameItem());
#endif // #ifdef DEBUG
		} else //in case where weapon is going to destroy
		{
			if (tmp_item)
				tmp_item->ActivateItem();
			
//!			if ( m_iActiveSlot != slot ) {
//!				LPCSTR const name = smart_cast<CGameObject const*>(m_pOwner)->cName().c_str();
//				if ( !xr_strcmp("jup_b43_stalker_assistant_pri6695", name) )
//					LogStackTrace	("");
//!				Msg				("[%6d][%s] CInventory::Activate changing active slot from %d to %d", Device.dwTimeGlobal, name, m_iActiveSlot, slot );
//!			}

			m_iActiveSlot		= slot;
		}
//		if ( m_iNextActiveSlot != slot ) {
//			LPCSTR const name = smart_cast<CGameObject const*>(m_pOwner)->cName().c_str();
//			if ( !xr_strcmp("jup_b43_stalker_assistant_pri6695", name) && !slot )
//				LogStackTrace	("");
//			Msg				( "[%6d][%s] CInventory::Activate changing next active slot3 to %d", Device.dwTimeGlobal, name, slot );
//		}
		m_iNextActiveSlot		= slot;
	}
}


PIItem CInventory::ItemFromSlot(u16 slot) const
{
	VERIFY(NO_ACTIVE_SLOT != slot);
	return m_slots[slot].m_pIItem;
}

void CInventory::SendActionEvent(u16 cmd, u32 flags) 
{
	CActor *pActor = smart_cast<CActor*>(m_pOwner);
	if (!pActor) return;

	NET_Packet		P;
	pActor->u_EventGen		(P,GE_INV_ACTION, pActor->ID());
	P.w_u16					(cmd);
	P.w_u32					(flags);
	P.w_s32					(pActor->GetZoomRndSeed());
	P.w_s32					(pActor->GetShotRndSeed());
	pActor->u_EventSend		(P, net_flags(TRUE, TRUE, FALSE, TRUE));
};

bool CInventory::Action(u16 cmd, u32 flags) 
{
	CActor *pActor = smart_cast<CActor*>(m_pOwner);
	
	if (pActor)
	{
		switch(cmd)
		{
			case kWPN_FIRE:
			{
				pActor->SetShotRndSeed();
			}break;
			case kWPN_ZOOM : 
			{
				pActor->SetZoomRndSeed();
			}break;
		};
	};

	if (g_pGameLevel && OnClient() && pActor) 
	{
		switch(cmd)
		{
		case kUSE:		break;
		
		case kDROP:		
			{
				if ((flags & CMD_STOP) && !IsGameTypeSingle())
				{
					PIItem tmp_item = ActiveItem();
					if (tmp_item)
					{
						tmp_item->DenyTrade();
					}
				}
				SendActionEvent	(cmd, flags);
				return			true;
			}break;

		case kWPN_NEXT:
		case kWPN_RELOAD:
		case kWPN_FIRE:
		case kWPN_FUNC:
		case kWPN_FIREMODE_NEXT:
		case kWPN_FIREMODE_PREV:
		case kWPN_ZOOM	 : 
		case kTORCH:
		case kNIGHT_VISION:
			{
				SendActionEvent(cmd, flags);
			}break;
		}
	}


	if (	ActiveItem() && 
			ActiveItem()->Action(cmd, flags)) 
											return true;
	bool b_send_event = false;
	switch(cmd) 
	{
	case kWPN_1:
	case kWPN_2:
	case kWPN_3:
	case kWPN_4:
	case kWPN_5:
	case kWPN_6:
		{
			b_send_event = true;
			if (cmd == kWPN_6 && !IsGameTypeSingle()) return false;
			
			u16 slot = u16(cmd - kWPN_1 + 1);
			if ( flags & CMD_START )
			{
				ActiveWeapon( slot );
			}
		}break;
	case kWPN_7:
		{
			b_send_event = true;
			if (flags & CMD_START)
			{
				if (GetActiveSlot() == PISTOL_SLOT && ActiveItem())
				{
					Activate(NO_ACTIVE_SLOT);
				}
				else
				{
					Activate(PISTOL_SLOT);
				}
			}
		}break;
	case kARTEFACT:
		{
		    b_send_event = true;
			if(flags&CMD_START)
			{
                if(GetActiveSlot() == ARTEFACT_SLOT &&
					ActiveItem() /*&& IsGameTypeSingle()*/)
				{
					Activate(NO_ACTIVE_SLOT);
				}else {
					Activate(ARTEFACT_SLOT);
				}
			}
		}break;
	case kACTIVE_JOBS:
		{
			b_send_event = true;
			if (flags & CMD_START)
			{
				if (!psActorFlags.test(AF_3D_PDA)) return false;

				if (GetActiveSlot() == PDA_SLOT && ActiveItem())
				{
					Activate(NO_ACTIVE_SLOT);
				}
				else
				{
					Activate(PDA_SLOT);
				}
			}
		}break;
	case kINVENTORY:
		{
			b_send_event = true;
			if (flags & CMD_START)
			{
				if (!smart_cast<CCustomBackpack*>(Actor()->inventory().ItemFromSlot(BACKPACK_SLOT))) return false;

				if (GetActiveSlot() == BACKPACK_SLOT && ActiveItem())
				{
					Activate(NO_ACTIVE_SLOT);
				}
				else
				{
					Activate(BACKPACK_SLOT);
				}
			}
		}break;
	}

	if(b_send_event && g_pGameLevel && OnClient() && pActor)
			SendActionEvent(cmd, flags);

	return false;
}

void CInventory::ActiveWeapon( u16 slot )
{
	// weapon is in active slot
	if ( GetActiveSlot() == slot && ActiveItem() )
	{
		if ( IsGameTypeSingle() )
			Activate(NO_ACTIVE_SLOT);
		else
			ActivateNextItemInActiveSlot();

		return;
	}
	Activate(slot);
/*
	if ( IsGameTypeSingle() )
	{
		Activate(slot);
		return;
	}
	if ( GetActiveSlot() == slot )
	{
		return;
	}

	Activate(slot);
	if ( slot != NO_ACTIVE_SLOT && ItemFromSlot(slot) == NULL )
	{
		u16 prev_activ = GetActiveSlot();
		m_iActiveSlot  = slot;
		if ( !ActivateNextItemInActiveSlot() )
		{
			m_iActiveSlot = prev_activ;
		}
	}*/
}

void CInventory::Update() 
{
	if( OnServer() )
	{
		if(m_iActiveSlot!=m_iNextActiveSlot)
		{
			CObject* pActor_owner = smart_cast<CObject*>(m_pOwner);
			if (Level().CurrentViewEntity() == pActor_owner)
			{
				if(	(m_iNextActiveSlot!=NO_ACTIVE_SLOT) && 
					 ItemFromSlot(m_iNextActiveSlot)	&&
					 !g_player_hud->allow_activation(ItemFromSlot(m_iNextActiveSlot)->cast_hud_item())
				   )
				   return;
			}
			if( ActiveItem() )
			{
				CHudItem* hi = ActiveItem()->cast_hud_item();
				
				if(!hi->IsHidden())
				{
					if(hi->GetState()==CHUDState::eIdle && hi->GetNextState()==CHUDState::eIdle)
						hi->SendDeactivateItem();

					UpdateDropTasks	();
					return;
				}
			}
			
			if (GetNextActiveSlot() != NO_ACTIVE_SLOT)
			{
				PIItem tmp_next_active = ItemFromSlot(GetNextActiveSlot());
				if (tmp_next_active)
				{
					if (IsSlotBlocked(tmp_next_active))
					{
						Activate(m_iActiveSlot);
						return;
					} else
					{
						tmp_next_active->ActivateItem();
					}
				}
			}
			
//			if ( m_iActiveSlot != GetNextActiveSlot() ) {
//				LPCSTR const name = smart_cast<CGameObject const*>(m_pOwner)->cName().c_str();
//				if ( !xr_strcmp("jup_b43_stalker_assistant_pri6695", name) )
//					LogStackTrace	("");
//				Msg					("[%6d][%s] CInventory::Activate changing active slot from %d to next active slot %d", Device.dwTimeGlobal, name, m_iActiveSlot, GetNextActiveSlot() );
//			}
			m_iActiveSlot			= GetNextActiveSlot();
		}
		if((GetNextActiveSlot()!=NO_ACTIVE_SLOT) && ActiveItem() && ActiveItem()->cast_hud_item()->IsHidden())
				ActiveItem()->ActivateItem();
	}
	UpdateDropTasks	();
}

void CInventory::UpdateDropTasks()
{
	//проверить слоты
	for(u16 i=FirstSlot(); i<=LastSlot(); ++i)	
	{
		PIItem itm = ItemFromSlot(i);
		if(itm)
			UpdateDropItem		(itm);
	}

	for(i = 0; i < 2; ++i)	
	{
		TIItemContainer &list			= i?m_ruck:m_belt;
		TIItemContainer::iterator it	= list.begin();
		TIItemContainer::iterator it_e	= list.end();
	
		for( ;it!=it_e; ++it)
		{
			UpdateDropItem		(*it);
		}
	}

	if (m_drop_last_frame)
	{
		m_drop_last_frame			= false;
		m_pOwner->OnItemDropUpdate	();
	}
}

void CInventory::UpdateDropItem(PIItem pIItem)
{
	if( pIItem->GetDropManual() )
	{
		pIItem->SetDropManual(FALSE);
		pIItem->DenyTrade();

		if ( OnServer() ) 
		{
			NET_Packet					P;
			pIItem->object().u_EventGen	(P, GE_OWNERSHIP_REJECT, pIItem->object().H_Parent()->ID());
			P.w_u16						(u16(pIItem->object().ID()));
			pIItem->object().u_EventSend(P);
		}
	}// dropManual
}

//ищем на поясе гранату такоже типа
PIItem CInventory::Same(const PIItem pIItem, bool bSearchRuck) const
{
	const TIItemContainer &list = bSearchRuck ? m_ruck : m_belt;

	for(TIItemContainer::const_iterator it = list.begin(); list.end() != it; ++it) 
	{
		const PIItem l_pIItem = *it;
		
		if((l_pIItem != pIItem) && 
				!xr_strcmp(l_pIItem->object().cNameSect(), 
				pIItem->object().cNameSect())) 
			return l_pIItem;
	}
	return NULL;
}

//ищем на поясе вещь для слота 

PIItem CInventory::SameSlot(const u16 slot, PIItem pIItem, bool bSearchRuck) const
{
	if(slot == NO_ACTIVE_SLOT) 	return NULL;

	const TIItemContainer &list = bSearchRuck ? m_ruck : m_belt;
	
	for(TIItemContainer::const_iterator it = list.begin(); list.end() != it; ++it) 
	{
		PIItem _pIItem = *it;
		if(_pIItem != pIItem && _pIItem->BaseSlot() == slot) return _pIItem;
	}

	return NULL;
}

//найти в инвенторе вещь с указанным именем
PIItem CInventory::Get(LPCSTR name, bool bSearchRuck) const
{
	const TIItemContainer &list = bSearchRuck ? m_ruck : m_belt;
	
	for(TIItemContainer::const_iterator it = list.begin(); list.end() != it; ++it) 
	{
		PIItem pIItem = *it;
		if(!xr_strcmp(pIItem->object().cNameSect(), name) && 
								pIItem->Useful()) 
				return pIItem;
	}
	return NULL;
}

PIItem CInventory::Get(CLASS_ID cls_id, bool bSearchRuck) const
{
	const TIItemContainer &list = bSearchRuck ? m_ruck : m_belt;
	
	for(TIItemContainer::const_iterator it = list.begin(); list.end() != it; ++it) 
	{
		PIItem pIItem = *it;
		if(pIItem->object().CLS_ID == cls_id && 
								pIItem->Useful()) 
				return pIItem;
	}
	return NULL;
}

PIItem CInventory::Get(const u16 id, bool bSearchRuck) const
{
	const TIItemContainer &list = bSearchRuck ? m_ruck : m_belt;

	for(TIItemContainer::const_iterator it = list.begin(); list.end() != it; ++it) 
	{
		PIItem pIItem = *it;
		if(pIItem->object().ID() == id) 
			return pIItem;
	}
	return NULL;
}

//search both (ruck and belt)
PIItem CInventory::GetAny(LPCSTR name) const
{
	PIItem itm = Get(name, false);
	if(!itm)
		itm = Get(name, true);
	return itm;
}

PIItem CInventory::item(CLASS_ID cls_id) const
{
	const TIItemContainer &list = m_all;

	for(TIItemContainer::const_iterator it = list.begin(); list.end() != it; ++it) 
	{
		PIItem pIItem = *it;
		if(pIItem->object().CLS_ID == cls_id && 
			pIItem->Useful()) 
			return pIItem;
	}
	return NULL;
}

float CInventory::TotalWeight() const
{
	VERIFY(m_fTotalWeight>=0.f);
	return m_fTotalWeight;
}


float CInventory::CalcTotalWeight()
{
	float weight = 0;
	for(TIItemContainer::const_iterator it = m_all.begin(); m_all.end() != it; ++it) 
		weight += (*it)->Weight();

	m_fTotalWeight = weight;
	return m_fTotalWeight;
}


u32 CInventory::dwfGetSameItemCount(LPCSTR caSection, bool SearchAll)
{
	u32			l_dwCount = 0;
	TIItemContainer	&l_list = SearchAll ? m_all : m_ruck;
	for(TIItemContainer::iterator l_it = l_list.begin(); l_list.end() != l_it; ++l_it) 
	{
		PIItem	l_pIItem = *l_it;
		if (!xr_strcmp(l_pIItem->object().cNameSect(), caSection))
            ++l_dwCount;
	}
	
	return		(l_dwCount);
}
u32		CInventory::dwfGetGrenadeCount(LPCSTR caSection, bool SearchAll)
{
	u32			l_dwCount = 0;
	TIItemContainer	&l_list = SearchAll ? m_all : m_ruck;
	for(TIItemContainer::iterator l_it = l_list.begin(); l_list.end() != l_it; ++l_it) 
	{
		PIItem	l_pIItem = *l_it;
		if (l_pIItem->object().CLS_ID == CLSID_GRENADE_F1 || l_pIItem->object().CLS_ID == CLSID_GRENADE_RGD5)
			++l_dwCount;
	}

	return		(l_dwCount);
}

bool CInventory::bfCheckForObject(ALife::_OBJECT_ID tObjectID)
{
	TIItemContainer	&l_list = m_all;
	for(TIItemContainer::iterator l_it = l_list.begin(); l_list.end() != l_it; ++l_it) 
	{
		PIItem	l_pIItem = *l_it;
		if (l_pIItem->object().ID() == tObjectID)
			return(true);
	}
	return		(false);
}

CInventoryItem *CInventory::get_object_by_id(ALife::_OBJECT_ID tObjectID)
{
	TIItemContainer	&l_list = m_all;
	for(TIItemContainer::iterator l_it = l_list.begin(); l_list.end() != l_it; ++l_it) 
	{
		PIItem	l_pIItem = *l_it;
		if (l_pIItem->object().ID() == tObjectID)
			return	(l_pIItem);
	}
	return		(0);
}

//скушать предмет 
#include "game_object_space.h"
#include "script_callback_ex.h"
#include "script_game_object.h"

void CInventory::ChooseItmAnimOrNot(PIItem pIItem)
{
	CEatableItem* pItemToEat = smart_cast<CEatableItem*>(pIItem);
	if (!pItemToEat) return;

	bool HasAnim = pItemToEat->m_bHasAnimation;
	bool AnimSect = pItemToEat->anim_sect != nullptr;

	if (HasAnim && AnimSect)
		pItemToEat->HideWeapon();
	else
		Eat(pItemToEat);
}

bool CInventory::Eat(PIItem pIItem)
{
	//устанаовить съедобна ли вещь
	CEatableItem* pItemToEat = smart_cast<CEatableItem*>(pIItem);
	if ( !pItemToEat )			return false;

	CEntityAlive *entity_alive = smart_cast<CEntityAlive*>(m_pOwner);
	if ( !entity_alive )		return false;

	CInventoryOwner* IO	= smart_cast<CInventoryOwner*>(entity_alive);
	if ( !IO )					return false;
	
	CInventory* pInventory = pItemToEat->m_pInventory;
	if ( !pInventory || pInventory != this )	return false;
	if ( pInventory != IO->m_inventory )		return false;
	if ( pItemToEat->object().H_Parent()->ID() != entity_alive->ID() )		return false;
	
	if (!pItemToEat->UseBy(entity_alive))
		return false;

#ifdef MP_LOGGING
	Msg( "--- Actor [%d] use or eat [%d][%s]", entity_alive->ID(), pItemToEat->object().ID(), pItemToEat->object().cNameSect().c_str() );
#endif // MP_LOGGING

	if(IsGameTypeSingle() && Actor()->m_inventory == this)
		Actor()->callback(GameObject::eUseObject)((smart_cast<CGameObject*>(pIItem))->lua_game_object());

	if(pItemToEat->Empty())
	{
		pIItem->SetDropManual(TRUE);
		return		false;
	}
	return			true;
}

bool CInventory::ClientEat(PIItem pIItem)
{
	CEatableItem* pItemToEat = smart_cast<CEatableItem*>(pIItem);
	if ( !pItemToEat )			return false;

	CEntityAlive *entity_alive = smart_cast<CEntityAlive*>(m_pOwner);
	if ( !entity_alive )		return false;

	CInventoryOwner* IO	= smart_cast<CInventoryOwner*>(entity_alive);
	if ( !IO )					return false;
	
	CInventory* pInventory = pItemToEat->m_pInventory;
	if ( !pInventory || pInventory != this )	return false;
	if ( pInventory != IO->m_inventory )		return false;
	if ( pItemToEat->object().H_Parent()->ID() != entity_alive->ID() )		return false;
	
	NET_Packet						P;
	CGameObject::u_EventGen			(P, GEG_PLAYER_ITEM_EAT, pIItem->parent_id());
	P.w_u16							(pIItem->object().ID());
	CGameObject::u_EventSend		(P);
	return true;
}

bool CInventory::InSlot(const CInventoryItem* pIItem) const
{
	if(pIItem->CurrPlace() != eItemPlaceSlot)	return false;

	VERIFY(m_slots[pIItem->CurrSlot()].m_pIItem == pIItem);

	return true;
}

bool CInventory::InBelt(const CInventoryItem* pIItem) const
{
	if(Get(pIItem->object().ID(), false)) return true;
	return false;
}

bool CInventory::InRuck(const CInventoryItem* pIItem) const
{
	if(Get(pIItem->object().ID(), true)) return true;
	return false;
}


bool CInventory::CanPutInSlot(PIItem pIItem, u16 slot_id) const
{
	if(!m_bSlotsUseful) return false;

	if( !GetOwner()->CanPutInSlot(pIItem, slot_id ) ) return false;


	if(slot_id==HELMET_SLOT)
	{
		CCustomOutfit* pOutfit = m_pOwner->GetOutfit();
		if(pOutfit && !pOutfit->bIsHelmetAvaliable)
			return false;
	}

	if(slot_id!=NO_ACTIVE_SLOT && 
		NULL==ItemFromSlot(slot_id) )
		return true;
	
	return false;
}
//проверяет можем ли поместить вещь на пояс,
//при этом реально ничего не меняется
bool CInventory::CanPutInBelt(PIItem pIItem)
{
	if(InBelt(pIItem))					return false;
	if(!m_bBeltUseful)					return false;
	if(!pIItem || !pIItem->Belt())		return false;
	if(m_belt.size() >= BeltWidth())	return false;

	return FreeRoom_inBelt(m_belt, pIItem, BeltWidth(), 1);
}
//проверяет можем ли поместить вещь в рюкзак,
//при этом реально ничего не меняется
bool CInventory::CanPutInRuck(PIItem pIItem) const
{
	if(InRuck(pIItem)) return false;
	return true;
}

u32	CInventory::dwfGetObjectCount()
{
	return		(m_all.size());
}

CInventoryItem	*CInventory::tpfGetObjectByIndex(int iIndex)
{
	if ((iIndex >= 0) && (iIndex < (int)m_all.size())) {
		TIItemContainer	&l_list = m_all;
		int			i = 0;
		for(TIItemContainer::iterator l_it = l_list.begin(); l_list.end() != l_it; ++l_it, ++i) 
			if (i == iIndex)
                return	(*l_it);
	}
	else {
		ai().script_engine().script_log	(ScriptStorage::eLuaMessageTypeError,"invalid inventory index!");
		return	(0);
	}
	R_ASSERT	(false);
	return		(0);
}

CInventoryItem	*CInventory::GetItemFromInventory(LPCSTR caItemName)
{
	TIItemContainer	&l_list = m_all;

	u32 crc = crc32(caItemName, xr_strlen(caItemName));

	for(TIItemContainer::iterator l_it = l_list.begin(); l_list.end() != l_it; ++l_it)
		if ((*l_it)->object().cNameSect()._get()->dwCRC == crc){
			VERIFY(	0 == xr_strcmp( (*l_it)->object().cNameSect().c_str(), caItemName)  );
			return	(*l_it);
		}
	return	(0);
}


bool CInventory::CanTakeItem(CInventoryItem *inventory_item) const
{
	VERIFY			(inventory_item);
	VERIFY			(m_pOwner);

	if (inventory_item->object().getDestroy()) return false;

	if(!inventory_item->CanTake()) return false;


	for(TIItemContainer::const_iterator it = m_all.begin(); it != m_all.end(); it++)
		if((*it)->object().ID() == inventory_item->object().ID()) break;
	VERIFY3(it == m_all.end(), "item already exists in inventory",*inventory_item->object().cName());

	CActor* pActor = smart_cast<CActor*>(m_pOwner);
	//актер всегда может взять вещь
	if(!pActor && (TotalWeight() + inventory_item->Weight() > m_pOwner->MaxCarryWeight()))
		return	false;

	return	true;
}


u32  CInventory::BeltWidth() const
{
	CActor* pActor = smart_cast<CActor*>( m_pOwner );
	if ( pActor )
	{
		u32 result = pActor->GetBaseArtefactCount();

		CCustomOutfit* outfit = pActor->GetOutfit();
		if (outfit)
		{
			result += outfit->get_artefact_count();
			clamp(result, u32(0), u32(GameConstants::GetArtefactsCount()));
		}
		return result;
	}
	return 0; //m_iMaxBelt;
}

void  CInventory::AddAvailableItems(TIItemContainer& items_container, bool for_trade) const
{
	for(TIItemContainer::const_iterator it = m_ruck.begin(); m_ruck.end() != it; ++it) 
	{
		PIItem pIItem = *it;
		if(!for_trade || pIItem->CanTrade())
			items_container.push_back(pIItem);
	}

	if(m_bBeltUseful)
	{
		for(TIItemContainer::const_iterator it = m_belt.begin(); m_belt.end() != it; ++it) 
		{
			PIItem pIItem = *it;
			if(!for_trade || pIItem->CanTrade())
				items_container.push_back(pIItem);
		}
	}
	
	CAI_Stalker* pOwner = smart_cast<CAI_Stalker*>(m_pOwner);
	if (pOwner && !pOwner->g_Alive()) {
		std::uint16_t I = FirstSlot();
		std::uint16_t E = LastSlot();
		for (; I <= E; ++I) {
			PIItem item = ItemFromSlot(I);
			if (item && (item->BaseSlot() != BOLT_SLOT))
				items_container.push_back(item);
		}
	}
	else if (m_bSlotsUseful) {
		u16 I = FirstSlot();
		u16 E = LastSlot();
		for(;I<=E;++I)
		{
			PIItem item = ItemFromSlot(I);
			if(item && (!for_trade || item->CanTrade())  )
			{
				if (!SlotIsPersistent(I) || item->BaseSlot() == GRENADE_SLOT) {
					if (pOwner) {
						std::uint32_t slot = item->BaseSlot();

						if (slot != INV_SLOT_3)
							items_container.push_back(item);
					}
					else {
						items_container.push_back(item);
					}
				}
			}
		}
	}		
}

bool CInventory::isBeautifulForActiveSlot	(CInventoryItem *pIItem)
{
	if (!IsGameTypeSingle()) 
		return (true);

	u16 I = FirstSlot();
	u16 E = LastSlot();
	for(;I<=E;++I)
	{
		PIItem item = ItemFromSlot(I);
		if (item && item->IsNecessaryItem(pIItem))
			return		(true);
	}
	return				(false);
}

//.#include "WeaponHUD.h"
void CInventory::Items_SetCurrentEntityHud(bool current_entity)
{
	TIItemContainer::iterator it;
	for(it = m_all.begin(); m_all.end() != it; ++it) 
	{
		PIItem pIItem = *it;
		CWeapon* pWeapon = smart_cast<CWeapon*>(pIItem);
		if (pWeapon)
		{
			pWeapon->InitAddons();
			pWeapon->UpdateAddonsVisibility();
		}
	}
};

//call this only via Actor()->SetWeaponHideState()
void CInventory::SetSlotsBlocked(u16 mask, bool bBlock)
{
	R_ASSERT(OnServer() || Level().IsDemoPlayStarted());

	for(u16 i = FirstSlot(), ie = LastSlot(); i <= ie; ++i)
	{
		if(mask & (1<<i))
		{
			if (bBlock)
				BlockSlot(i);
			else
				UnblockSlot(i);
		}
	}
	
	if (bBlock)
	{
		TryDeactivateActiveSlot();	
	} else
	{
		TryActivatePrevSlot();
	}
}

void CInventory::TryActivatePrevSlot()
{
	u16 ActiveSlot		= GetActiveSlot();
	u16 PrevActiveSlot	= GetPrevActiveSlot();
	u16 NextActiveSlot	= GetNextActiveSlot();
	if ((
			(ActiveSlot == NO_ACTIVE_SLOT) ||
			(NextActiveSlot == NO_ACTIVE_SLOT)
		) &&
		(PrevActiveSlot != NO_ACTIVE_SLOT))
	{
		PIItem prev_active_item = ItemFromSlot(PrevActiveSlot);
		if (prev_active_item &&
			!IsSlotBlocked(prev_active_item) &&
			m_slots[PrevActiveSlot].CanBeActivated())
		{
#ifndef MASTER_GOLD
			Msg("Set slots blocked: activating prev slot [%d], Frame[%d]", PrevActiveSlot, Device.dwFrame);
#endif // #ifndef MASTER_GOLD
			Activate(PrevActiveSlot);
			SetPrevActiveSlot(NO_ACTIVE_SLOT);
		}
	}
}

void CInventory::TryDeactivateActiveSlot	()
{
	u16 ActiveSlot		= GetActiveSlot();
	u16 NextActiveSlot	= GetNextActiveSlot();

	if ((ActiveSlot == NO_ACTIVE_SLOT) && (NextActiveSlot == NO_ACTIVE_SLOT))
		return;
	
	PIItem		active_item = (ActiveSlot != NO_ACTIVE_SLOT) ? 
		ItemFromSlot(ActiveSlot) : NULL;
	PIItem		next_active_item = (NextActiveSlot != NO_ACTIVE_SLOT) ?
		ItemFromSlot(NextActiveSlot) : NULL;

	if (active_item &&
		(IsSlotBlocked(active_item) || !m_slots[ActiveSlot].CanBeActivated())
		)
	{
#ifndef MASTER_GOLD
		Msg("Set slots blocked: activating slot [-1], Frame[%d]", Device.dwFrame);
#endif // #ifndef MASTER_GOLD
		ItemFromSlot(ActiveSlot)->DiscardState();
		Activate			(NO_ACTIVE_SLOT);
		SetPrevActiveSlot	(ActiveSlot);
	} else if (next_active_item &&
		(IsSlotBlocked(next_active_item) || !m_slots[NextActiveSlot].CanBeActivated())
		)
	{
		Activate			(NO_ACTIVE_SLOT);
		SetPrevActiveSlot	(NextActiveSlot);
	}
}

void CInventory::BlockSlot(u16 slot_id)
{
	VERIFY(slot_id <= LAST_SLOT);
	
	++m_blocked_slots[slot_id];
	
	VERIFY2(m_blocked_slots[slot_id] < 5,
		make_string("blocked slot [%d] overflow").c_str());	
}

void CInventory::UnblockSlot(u16 slot_id)
{
	VERIFY(slot_id <= LAST_SLOT);
	VERIFY2(m_blocked_slots[slot_id] > 0,
		make_string("blocked slot [%d] underflow").c_str());	
	
	--m_blocked_slots[slot_id];	
}

bool CInventory::IsSlotBlocked(u16 slot_id) const
{
	VERIFY(slot_id <= LAST_SLOT);
	return m_blocked_slots[slot_id] > 0;
}

bool CInventory::IsSlotBlocked(PIItem const iitem) const
{
	VERIFY(iitem);
	return IsSlotBlocked(iitem->BaseSlot());
}
