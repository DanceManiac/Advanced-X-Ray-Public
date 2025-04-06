#include "pch_script.h"
#include "inventory.h"
#include "actor.h"
#include "trade.h"
#include "weapon.h"

#include "ui/UIInventoryUtilities.h"

#include "eatable_item.h"
#include "script_engine.h"
#include "xrmessages.h"
//#include "game_cl_base.h"
#include "xr_level_controller.h"
#include "level.h"
#include "ai_space.h"
#include "entitycondition.h"
#include "game_base_space.h"
#include "clsid_game.h"

#include "ai/stalker/ai_stalker.h"
#include "weaponmagazined.h"
#include "HudItem.h"
#include "PDA.h"
#include "player_hud.h"
#include "GamePersistent.h"
#include "CustomBackpack.h"
#include "CustomOutfit.h"
#include "ActorHelmet.h"
#include "HUDManager.h"
#include "UIGameSP.h"
#include "ui\UIInventoryWnd.h"

using namespace InventoryUtilities;

// what to block
u32	INV_STATE_BLOCK_ALL		= 0xffffffff;
u32	INV_STATE_LADDER		= INV_STATE_BLOCK_ALL;
u32	INV_STATE_CAR			= INV_STATE_BLOCK_ALL;
u32	INV_STATE_INV_WND		= INV_STATE_BLOCK_ALL;
u32	INV_STATE_BUY_MENU		= INV_STATE_BLOCK_ALL;
u32 INV_STATE_HIDE_WEAPON	= (1 << KNIFE_SLOT | 1 << PISTOL_SLOT | 1 << RIFLE_SLOT | 1 << GRENADE_SLOT);

CInventorySlot::CInventorySlot() 
{
	m_pIItem				= NULL;
	m_bVisible				= true;
	m_bPersistent			= false;
	m_blockCounter			= 0;
}

void CInventory::ReloadSlotsConfig()
{
	m_slots[BACKPACK_SLOT].m_bVisible = GameConstants::GetBackpackAnimsEnabled(); // Для опции анимированного рюкзака
}


CInventorySlot::~CInventorySlot() 
{
}

bool CInventorySlot::CanBeActivated() const 
{
	return (m_bVisible && !IsBlocked());
};

bool CInventorySlot::IsBlocked() const 
{
	return (m_blockCounter>0);
}

CInventory::CInventory() 
{
	inv_sect = "inventory";
	inv_settings = pSettings;
	
	if (pAdvancedSettings->section_exist("axr_inventory"))
	{
		inv_sect = "axr_inventory";
		inv_settings = pAdvancedSettings;
	}

	m_fTakeDist									= inv_settings->r_float	(inv_sect, "take_dist");
	m_fMaxWeight								= inv_settings->r_float	(inv_sect, "max_weight");
	m_iMaxBelt									= inv_settings->r_s32	(inv_sect, "max_belt");
	
	u32 sz										= inv_settings->r_s32(inv_sect, "slots");
	m_slots.resize								(sz);
	
	m_iActiveSlot								= NO_ACTIVE_SLOT;
	m_iNextActiveSlot							= NO_ACTIVE_SLOT;
	m_iPrevActiveSlot							= NO_ACTIVE_SLOT;
	m_iLoadActiveSlot							= NO_ACTIVE_SLOT;
	m_ActivationSlotReason						= eGeneral;
	m_pTarget									= nullptr;
	m_pOwner									= nullptr;

	string256 temp;
	for(u32 i=0; i<m_slots.size(); ++i ) 
	{
		sprintf_s(temp, "slot_persistent_%d", i+1);
		if (pSettings->line_exist(inv_sect, temp))
			m_slots[i].m_bPersistent = !!inv_settings->r_bool(inv_sect, temp);
	};

	m_slots[PDA_SLOT].m_bVisible				= true;
	m_slots[OUTFIT_SLOT].m_bVisible				= false;
	m_slots[DETECTOR_SLOT].m_bVisible			= false;
	m_slots[TORCH_SLOT].m_bVisible				= false;

	m_bSlotsUseful								= true;
	m_bBeltUseful								= false;

	m_fTotalWeight								= -1.f;
	m_dwModifyFrame								= 0;
	m_drop_last_frame							= false;
	m_iLoadActiveSlotFrame						= u32(-1);
}

CInventory::~CInventory() 
{
}

void CInventory::Clear()
{
	m_all.clear							();
	m_ruck.clear						();
	m_belt.clear						();
	
	for(u32 i=0; i<m_slots.size(); i++)
	{
		m_slots[i].m_pIItem				= NULL;
	}
	

	m_pOwner							= NULL;

	CalcTotalWeight						();
	InvalidateState						();
}

void CInventory::Take(CGameObject *pObj, bool bNotActivate, bool strict_placement)
{
	CInventoryItem *pIItem				= smart_cast<CInventoryItem*>(pObj);
	VERIFY								(pIItem);
	
	if (pIItem->m_pInventory)
	{
		Msg("! ERROR CInventory::Take but object has m_pInventory");
		Msg("! Inventory Owner is [%d]", GetOwner()->object_id());
		Msg("! Object Inventory Owner is [%d]", pIItem->m_pInventory->GetOwner()->object_id());

		CObject* p	= pObj->H_Parent();
		if (p)
			Msg("! object parent is [%s] [%d]", p->cName().c_str(), p->ID());
	}

	R_ASSERT							(CanTakeItem(pIItem));
	
	pIItem->m_pInventory				= this;
	pIItem->SetDropManual				(FALSE);

	u16 actor_id = Level().CurrentEntity()->ID();

	if (GetOwner()->object_id() == actor_id && this->m_pOwner->object_id() == actor_id)		//actors inventory
	{
		CWeaponMagazined* pWeapon = smart_cast<CWeaponMagazined*>(pIItem);
		if (pWeapon && pWeapon->strapped_mode())
		{
			pWeapon->strapped_mode(false);
			Ruck(pWeapon);
		}
	}

	m_all.push_back						(pIItem);

	if (!strict_placement)
		pIItem->m_eItemPlace			= eItemPlaceUndefined;

	bool result							= false;
	switch(pIItem->m_eItemPlace)
	{
	case eItemPlaceBelt:
		result							= Belt(pIItem); 
#ifdef DEBUG
		if (!result) 
			Msg("!![%s] cant put in belt item [%s], moving to ruck...", __FUNCTION__, pIItem->object().cName().c_str());
#endif

		break;
	case eItemPlaceRuck:
		result							= Ruck(pIItem);
#ifdef DEBUG
		if (!result) 
			Msg("!![%s] cant put in ruck item [%s], moving to ruck...", __FUNCTION__, pIItem->object().cName().c_str());
#endif

		break;
	case eItemPlaceSlot:
		result							= Slot(pIItem, bNotActivate); 
#ifdef DEBUG
		if (!result) 
			Msg("!![%s] cant put in slot item [%s], moving to ruck...", __FUNCTION__, pIItem->object().cName().c_str());
#endif

		break;
	default:
		if (CanPutInSlot(pIItem))
		{
			result						= Slot(pIItem, bNotActivate); VERIFY(result);
		} 
		else if ( !pIItem->RuckDefault() && CanPutInBelt(pIItem))
		{
			result						= Belt(pIItem); VERIFY(result);
		}
		else
		{
			result						= Ruck(pIItem); VERIFY(result);
		}
	}
	
	m_pOwner->OnItemTake				(pIItem);

	CalcTotalWeight						();
	InvalidateState						();

	pIItem->object().processing_deactivate();
	VERIFY								(pIItem->m_eItemPlace != eItemPlaceUndefined);

	CObject* pActor_owner = smart_cast<CObject*>(m_pOwner);

	if (Level().CurrentViewEntity() == pActor_owner)
	{
		if (pIItem->m_eItemPlace == eItemPlaceRuck)
			Actor()->ChangeInventoryFullness(pIItem->GetOccupiedInvSpace());
	}
}

bool CInventory::DropItem(CGameObject *pObj) 
{
	CInventoryItem *pIItem				= smart_cast<CInventoryItem*>(pObj);
	VERIFY								(pIItem);
	if (!pIItem)
		return false;

	if (pIItem->m_pInventory!=this)
	{
		Msg("ahtung !!! [%d]", Device.dwFrame);
		Msg("CInventory::DropItem pIItem->m_pInventory!=this");
		Msg("this = [%d]", GetOwner()->object_id());
		Msg("pIItem->m_pInventory = [%d]", pIItem->m_pInventory->GetOwner()->object_id());
	}

	R_ASSERT							(pIItem->m_pInventory);
	R_ASSERT							(pIItem->m_pInventory == this);
	VERIFY								(pIItem->m_eItemPlace != eItemPlaceUndefined);

	pIItem->object().processing_activate(); 
	pIItem->OnBeforeDrop();

	switch(pIItem->m_eItemPlace)
	{
	case eItemPlaceBelt:{
			R_ASSERT(InBelt(pIItem));
			m_belt.erase(std::find(m_belt.begin(), m_belt.end(), pIItem));
			pIItem->object().processing_deactivate();
		}break;
	case eItemPlaceRuck:{
			R_ASSERT(InRuck(pIItem));
			m_ruck.erase(std::find(m_ruck.begin(), m_ruck.end(), pIItem));
		}break;
	case eItemPlaceSlot:{
			R_ASSERT			(InSlot(pIItem));
			if (m_iActiveSlot == pIItem->GetSlot()) 
				Activate	(NO_ACTIVE_SLOT);

			m_slots[pIItem->GetSlot()].m_pIItem = NULL;	
			pIItem->OnDrop();
			pIItem->object().processing_deactivate();
		}break;
	default:
		NODEFAULT;
	};

	TIItemContainer::iterator	it = std::find(m_all.begin(), m_all.end(), pIItem);
	if ( it != m_all.end())
		m_all.erase				(it);
	else
		Msg						("! CInventory::Drop item not found in inventory!!!");

	pIItem->m_pInventory		= NULL;

	m_pOwner->OnItemDrop		(smart_cast<CInventoryItem*>(pObj));

	CalcTotalWeight				();
	InvalidateState				();
	m_drop_last_frame			= true;

	CObject* pActor_owner = smart_cast<CObject*>(m_pOwner);

	if (Level().CurrentViewEntity() == pActor_owner)
	{
		if (pIItem->m_eItemPlace == eItemPlaceRuck)
			Actor()->ChangeInventoryFullness(-pIItem->GetOccupiedInvSpace());
	}

	return							true;
}

//положить вещь в слот
bool CInventory::Slot(PIItem pIItem, bool bNotActivate) 
{
	VERIFY(pIItem);
//	Msg("To Slot %s[%d]", *pIItem->object().cName(), pIItem->object().ID());
	
	if (!CanPutInSlot(pIItem)) 
	{
#if 0//def _DEBUG
		Msg("there is item %s[%d,%x] in slot %d[%d,%x]", 
				*m_slots[pIItem->GetSlot()].m_pIItem->object().cName(), 
				m_slots[pIItem->GetSlot()].m_pIItem->object().ID(), 
				m_slots[pIItem->GetSlot()].m_pIItem, 
				pIItem->GetSlot(), 
				pIItem->object().ID(),
				pIItem);
#endif
		if (m_slots[pIItem->GetSlot()].m_pIItem == pIItem && !bNotActivate )
			Activate(pIItem->GetSlot());

		return false;
	}


	m_slots[pIItem->GetSlot()].m_pIItem = pIItem;

	//удалить из рюкзака или пояса
	TIItemContainer::iterator it = std::find(m_ruck.begin(), m_ruck.end(), pIItem);
	if (m_ruck.end() != it) m_ruck.erase(it);
	it = std::find(m_belt.begin(), m_belt.end(), pIItem);
	if (m_belt.end() != it) m_belt.erase(it);



	if (( (m_iActiveSlot==pIItem->GetSlot())||(m_iActiveSlot==NO_ACTIVE_SLOT) && m_iNextActiveSlot==NO_ACTIVE_SLOT) && (!bNotActivate))
		Activate				(pIItem->GetSlot());

	
	m_pOwner->OnItemSlot		(pIItem, pIItem->m_eItemPlace);
	EItemPlace prev_place		= pIItem->m_eItemPlace;
	pIItem->m_eItemPlace		= eItemPlaceSlot;
	pIItem->OnMoveToSlot		(prev_place);
	
	if (prev_place == eItemPlaceRuck)
		Actor()->ChangeInventoryFullness(-pIItem->GetOccupiedInvSpace());
	
	pIItem->object().processing_activate();

	return						true;
}

bool CInventory::Belt(PIItem pIItem) 
{
	if (!CanPutInBelt(pIItem))	return false;
	
	//вещь была в слоте
	bool in_slot = InSlot(pIItem);
	if (in_slot) 
	{
		if (m_iActiveSlot == pIItem->GetSlot()) Activate(NO_ACTIVE_SLOT);
		m_slots[pIItem->GetSlot()].m_pIItem = NULL;
	}
	
	m_belt.insert(m_belt.end(), pIItem); 

	if (!in_slot)
	{
		TIItemContainer::iterator it = std::find(m_ruck.begin(), m_ruck.end(), pIItem); 
		if (m_ruck.end() != it) m_ruck.erase(it);
	}

	CalcTotalWeight();
	InvalidateState						();

	EItemPlace prev_place = pIItem->m_eItemPlace;
	pIItem->m_eItemPlace = eItemPlaceBelt;
	m_pOwner->OnItemBelt(pIItem, prev_place);
	pIItem->OnMoveToBelt();

	if (prev_place == eItemPlaceRuck)
		Actor()->ChangeInventoryFullness(-pIItem->GetOccupiedInvSpace());

	if (in_slot)
		pIItem->object().processing_deactivate();

	pIItem->object().processing_activate();

	return true;
}

bool CInventory::Ruck(PIItem pIItem) 
{
	if (!CanPutInRuck(pIItem)) return false;
	
	bool in_slot = InSlot(pIItem);
	//вещь была в слоте
	if (in_slot) 
	{
		if (m_iActiveSlot == pIItem->GetSlot()) Activate(NO_ACTIVE_SLOT);
		m_slots[pIItem->GetSlot()].m_pIItem = NULL;
	}
	else
	{
		//вещь была на поясе или вообще только поднята с земли
		TIItemContainer::iterator it = std::find(m_belt.begin(), m_belt.end(), pIItem); 
		if (m_belt.end() != it) m_belt.erase(it);
	}
	
	m_ruck.insert									(m_ruck.end(), pIItem); 
	
	CalcTotalWeight									();
	InvalidateState									();

	m_pOwner->OnItemRuck							(pIItem, pIItem->m_eItemPlace);
	EItemPlace prev_place							= pIItem->m_eItemPlace;
	pIItem->m_eItemPlace							= eItemPlaceRuck;
	pIItem->OnMoveToRuck							(prev_place);

	if (prev_place == eItemPlaceSlot || prev_place == eItemPlaceBelt)
		Actor()->ChangeInventoryFullness(pIItem->GetOccupiedInvSpace());

	if (in_slot)
		pIItem->object().processing_deactivate();

	return true;
}

void CInventory::Activate_deffered	(u32 slot, u32 _frame)
{
	 m_iLoadActiveSlot			= slot;
	 m_iLoadActiveSlotFrame		= _frame;
}

void  CInventory::ActivateNextItemInActiveSlot()
{
	if (m_iActiveSlot==NO_ACTIVE_SLOT)	return;
	
	PIItem current_item		= m_slots[m_iActiveSlot].m_pIItem;
	PIItem new_item			= NULL;

	bool b = (current_item==NULL);
	
	TIItemContainer::const_iterator it		= m_all.begin();
	TIItemContainer::const_iterator it_e	= m_all.end();

	for(; it!=it_e; ++it) 
	{
		PIItem _pIItem		= *it;
		if (_pIItem==current_item)
		{
			b = true;
			continue;
		}
		if (_pIItem->GetSlot()==m_iActiveSlot)
			new_item = _pIItem;

		if (b && new_item)
			break;
	}

	if (new_item==NULL)
		return; //only 1 item for this slot

	bool res = Ruck						(current_item);
	R_ASSERT							(res);
	NET_Packet							P;
	current_item->object().u_EventGen	(P, GEG_PLAYER_ITEM2RUCK, current_item->object().H_Parent()->ID());
	P.w_u16								(current_item->object().ID());
	current_item->object().u_EventSend	(P);

	res = Slot							(new_item);
	R_ASSERT							(res);
	new_item->object().u_EventGen		(P, GEG_PLAYER_ITEM2SLOT, new_item->object().H_Parent()->ID());
	P.w_u16								(new_item->object().ID());
	new_item->object().u_EventSend		(P);
	
	//activate
	new_item->object().u_EventGen		(P, GEG_PLAYER_ACTIVATE_SLOT, new_item->object().H_Parent()->ID());
	P.w_u32								(new_item->GetSlot());
	new_item->object().u_EventSend		(P);


	Msg("CHANGE");
}

bool CInventory::Activate(u32 slot, EActivationReason reason, bool bForce) 
{	
	if (	m_ActivationSlotReason==eKeyAction	&& reason==eImportUpdate )
		return false;

	bool res = false;

	if (Device.dwFrame == m_iLoadActiveSlotFrame) 
	{
		 if ( (m_iLoadActiveSlot == slot) && m_slots[slot].m_pIItem )
			m_iLoadActiveSlotFrame = u32(-1);
		 else
			{
			 res = false;
			 goto _finish;
			}

	}

	if ( (slot!=NO_ACTIVE_SLOT && slot != BACKPACK_SLOT && slot != PDA_SLOT && m_slots[slot].IsBlocked()) && !bForce)
	{
		res = false;
		goto _finish;
	}

	R_ASSERT2(slot == NO_ACTIVE_SLOT || slot<m_slots.size(), "wrong slot number");

	if (slot != NO_ACTIVE_SLOT && !m_slots[slot].m_bVisible) 
	{
		res = false;
		goto _finish;
	}
	
	CUIGameSP* pGameSP = smart_cast<CUIGameSP*>(HUD().GetUI()->UIGame());
	if (m_iActiveSlot == BACKPACK_SLOT && slot != NO_ACTIVE_SLOT && pGameSP->InventoryMenu->IsShown())
		return false;

#pragma todo("DANCE MANIAC: Hack for last thrown grenade block slots and crash game =/")
	if (m_iActiveSlot == GRENADE_SLOT && !m_slots[m_iActiveSlot].m_pIItem)
	{
		m_iActiveSlot = NO_ACTIVE_SLOT;
		Activate(NO_ACTIVE_SLOT);
	}

	if (m_iActiveSlot == slot && m_iActiveSlot != NO_ACTIVE_SLOT && m_slots[m_iActiveSlot].m_pIItem)
	{
		m_slots[m_iActiveSlot].m_pIItem->Activate();
	}

	if (	m_iActiveSlot == slot || 
		(m_iNextActiveSlot == slot &&
		 m_iActiveSlot != NO_ACTIVE_SLOT &&
		m_slots[m_iActiveSlot].m_pIItem->cast_hud_item() &&
		m_slots[m_iActiveSlot].m_pIItem->cast_hud_item()->IsHiding()
		 )
	   )
	{
		res = false;
		goto _finish;
	}

	//активный слот не выбран
	if (m_iActiveSlot == NO_ACTIVE_SLOT)
	{
		if (m_slots[slot].m_pIItem)
		{
			m_iNextActiveSlot		= slot;
			m_ActivationSlotReason	= reason;
			res = true;
			goto _finish;
		}
		else 
		{
			if (slot==GRENADE_SLOT)//fake for grenade
			{
				PIItem gr = SameSlot(GRENADE_SLOT, NULL, true);
				if (gr)
				{
					Slot(gr);
					goto _finish;
				}else
				{
					res = false;
					goto _finish;
				}

			}else
			{
				res = false;
				goto _finish;
			}
		}
	}
	//активный слот задействован
	else if (slot == NO_ACTIVE_SLOT || m_slots[slot].m_pIItem)
	{
		if (m_slots[m_iActiveSlot].m_pIItem)
			m_slots[m_iActiveSlot].m_pIItem->Deactivate();

		m_iNextActiveSlot		= slot;
		m_ActivationSlotReason	= reason;
	
		res = true;
		goto _finish;
	}

	_finish:

	if (res)
		m_ActivationSlotReason	= reason;
	return res;
}


PIItem CInventory::ItemFromSlot(u32 slot) const
{
	if (slot == NO_ACTIVE_SLOT)
		return (0);

	//VERIFY(NO_ACTIVE_SLOT != slot);
	return m_slots[slot].m_pIItem;
}

void CInventory::SendActionEvent(s32 cmd, u32 flags) 
{
	CActor *pActor = smart_cast<CActor*>(m_pOwner);
	if (!pActor) return;

	NET_Packet		P;
	pActor->u_EventGen		(P,GE_INV_ACTION, pActor->ID());
	P.w_s32					(cmd);
	P.w_u32					(flags);
	P.w_s32					(pActor->GetZoomRndSeed());
	P.w_s32					(pActor->GetShotRndSeed());
	pActor->u_EventSend		(P, net_flags(TRUE, TRUE, FALSE, TRUE));
};

bool CInventory::Action(s32 cmd, u32 flags) 
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

	if (g_pGameLevel && OnClient() && pActor) {
		switch(cmd)
		{
		case kUSE:
			{
			}break;
		
		case kDROP:		
		
			{
				SendActionEvent(cmd, flags);
				return true;
			}break;

		case kWPN_NEXT:
		case kWPN_RELOAD:
		case kWPN_FIRE:
		case kWPN_FUNC:
		case kWPN_FIREMODE_NEXT:
		case kWPN_FIREMODE_PREV:
		case kWPN_ZOOM : 
		case kTORCH:
		case kNIGHT_VISION:
			{
				SendActionEvent(cmd, flags);
			}break;
		}
	}


	if (m_iActiveSlot < m_slots.size() && 
			m_slots[m_iActiveSlot].m_pIItem && 
			m_slots[m_iActiveSlot].m_pIItem->Action(cmd, flags)) 
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
		   if (cmd == kWPN_6 && !IsGameTypeSingle()) return false;

			if (flags&CMD_START)
			{
				if ((int)m_iActiveSlot == cmd - kWPN_1 &&
					m_slots[m_iActiveSlot].m_pIItem )
				{
					if (IsGameTypeSingle())
						b_send_event = Activate(NO_ACTIVE_SLOT);
					else
					{
						ActivateNextItemInActiveSlot();
					}
				}else{ 					
					if ((int)m_iActiveSlot == cmd - kWPN_1 && !IsGameTypeSingle())
						break;

					b_send_event = Activate(cmd - kWPN_1, eKeyAction);
				}
			}
		}break;
	case kARTEFACT:
		{
			if (flags&CMD_START)
			{
				if ((int)m_iActiveSlot == ARTEFACT_SLOT &&
					m_slots[m_iActiveSlot].m_pIItem && IsGameTypeSingle())
				{
					b_send_event = Activate(NO_ACTIVE_SLOT);
				}else {
					b_send_event = Activate(ARTEFACT_SLOT);
				}
			}
		}break;
	case kACTIVE_JOBS:
	case kMAP:
	case kCONTACTS:
		{
			b_send_event = true;
			if (flags & CMD_STOP)
			{
				auto Pda = m_pOwner->GetPDA();
				if (!Pda || !Pda->Is3DPDA() || !psActorFlags.test(AF_3D_PDA))
					break;


				if (GetActiveSlot() == PDA_SLOT && ActiveItem())
					Activate(NO_ACTIVE_SLOT);
				else
					Activate(PDA_SLOT);
			}
		}break;
	case kINVENTORY:
		{
			b_send_event = true;
			if (flags & CMD_STOP)
			{
				if (!GameConstants::GetBackpackAnimsEnabled() || !smart_cast<CCustomBackpack*>(ItemFromSlot(BACKPACK_SLOT))) return false;

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

	if (b_send_event && g_pGameLevel && OnClient() && pActor)
			SendActionEvent(cmd, flags);

	return false;
}


void CInventory::Update() 
{
	bool bActiveSlotVisible;
#pragma todo("DANCE MANIAC: Add [m_slots[m_iActiveSlot].m_pIItem &&] for fix crash.")
	if (m_iActiveSlot == NO_ACTIVE_SLOT || 
		m_slots[m_iActiveSlot].m_pIItem && !m_slots[m_iActiveSlot].m_pIItem->cast_hud_item() ||
		m_slots[m_iActiveSlot].m_pIItem && m_slots[m_iActiveSlot].m_pIItem->cast_hud_item()->IsHidden())
	{ 
		bActiveSlotVisible = false;
	}
	else 
	{
		bActiveSlotVisible = true;
	}

	if (m_iNextActiveSlot != m_iActiveSlot && !bActiveSlotVisible)
	{
		if (m_iNextActiveSlot != NO_ACTIVE_SLOT &&
			m_slots[m_iNextActiveSlot].m_pIItem)
			m_slots[m_iNextActiveSlot].m_pIItem->Activate();

		m_iActiveSlot = m_iNextActiveSlot;
	}
	UpdateDropTasks	();
}

void CInventory::UpdateDropTasks()
{
	for(u32 i=0; i<m_slots.size(); ++i)	
	{
		if (m_slots[i].m_pIItem)
			UpdateDropItem		(m_slots[i].m_pIItem);
	}

	for(u32 i = 0; i < 2; ++i)
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
	if (pIItem && pIItem->GetDropManual())
	{
		pIItem->SetDropManual(FALSE);
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
		
		if ((l_pIItem != pIItem) && 
				!xr_strcmp(l_pIItem->object().cNameSect(), 
				pIItem->object().cNameSect())) 
			return l_pIItem;
	}
	return NULL;
}

//ищем на поясе вещь для слота 

PIItem CInventory::SameSlot(const u32 slot, PIItem pIItem, bool bSearchRuck) const
{
	if (slot == NO_ACTIVE_SLOT) 	return NULL;

	const TIItemContainer &list = bSearchRuck ? m_ruck : m_belt;
	
	for(TIItemContainer::const_iterator it = list.begin(); list.end() != it; ++it) 
	{
		PIItem _pIItem = *it;
		if (_pIItem != pIItem && _pIItem->GetSlot() == slot) return _pIItem;
	}

	return NULL;
}

//найти в инвенторе вещь с указанным именем
PIItem CInventory::Get(const char *name, bool bSearchRuck) const
{
	const TIItemContainer &list = bSearchRuck ? m_ruck : m_belt;
	
	for(TIItemContainer::const_iterator it = list.begin(); list.end() != it; ++it) 
	{
		PIItem pIItem = *it;
		if (pIItem && !xr_strcmp(pIItem->object().cNameSect(), name) && 
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
		if (pIItem && pIItem->object().CLS_ID == cls_id && 
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
		if (pIItem && pIItem->object().ID() == id) 
			return pIItem;
	}
	return NULL;
}

//search both (ruck and belt)
PIItem CInventory::GetAny(const char *name) const
{
	PIItem itm = Get(name, false);
	if (!itm)
		itm = Get(name, true);
	return itm;
}

PIItem CInventory::item(CLASS_ID cls_id) const
{
	const TIItemContainer &list = m_all;

	for(TIItemContainer::const_iterator it = list.begin(); list.end() != it; ++it) 
	{
		PIItem pIItem = *it;
		if (pIItem && pIItem->object().CLS_ID == cls_id && 
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
		if (l_pIItem && !xr_strcmp(l_pIItem->object().cNameSect(), caSection))
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
		if (l_pIItem && l_pIItem->object().CLS_ID == CLSID_GRENADE_F1 || l_pIItem->object().CLS_ID == CLSID_GRENADE_RGD5)
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
		if (l_pIItem && l_pIItem->object().ID() == tObjectID)
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
		if (l_pIItem && l_pIItem->object().ID() == tObjectID)
			return	(l_pIItem);
	}
	return		(0);
}

//скушать предмет 
#include "game_object_space.h"
#include "script_callback_ex.h"
#include "script_game_object.h"
#include "Battery.h"
#include "RepairKit.h"
#include "AntigasFilter.h"

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
	R_ASSERT(pIItem->m_pInventory == this);
	//устанаовить съедобна ли вещь
	CEatableItem* pItemToEat = smart_cast<CEatableItem*>(pIItem);
	CBattery* pBattery = smart_cast<CBattery*>(pIItem);
	CRepairKit* pRepairKit = smart_cast<CRepairKit*>(pIItem);
	CAntigasFilter* pFilter = smart_cast<CAntigasFilter*>(pIItem);
	R_ASSERT				(pItemToEat);

	CEntityAlive *entity_alive = smart_cast<CEntityAlive*>(m_pOwner);
	R_ASSERT				(entity_alive);
	
	if (xr_strcmp(pItemToEat->m_use_functor_str, ""))
	{
		luabind::functor<void> m_functor;
		if (ai().script_engine().functor(pItemToEat->m_use_functor_str.c_str(), m_functor))
		{
			m_functor();

#ifdef DEBUG
			Msg("[CInventory::Eat]: Lua function [%s] called from item [%s] by use_functor.", pItemToEat->m_use_functor_str.c_str(), pItemToEat->m_section_id.c_str());
#endif
		}
#ifdef DEBUG
		else
		{
			Msg("[CInventory::Eat]: ERROR: Lua function [%s] called from item [%s] by use_functor not found!", pItemToEat->m_use_functor_str.c_str(), pItemToEat->m_section_id.c_str());
		}
#endif
	}

#pragma todo("Find out why it works only with these hacks")
	if (!pBattery && !pRepairKit && !pFilter && !pItemToEat->m_bUnlimited) // что это за говно вообще было???
	{
		pItemToEat->UseBy(entity_alive);
	}
	else if (pBattery)
	{
		pBattery->UseBy(entity_alive);
	}
	else if (pFilter)
	{
		pFilter->UseBy(entity_alive);
	}
	else if (pRepairKit)
	{
		pRepairKit->UseBy(entity_alive);
	}


	if (IsGameTypeSingle() && Actor()->m_inventory == this)
		Actor()->callback(GameObject::eUseObject)((smart_cast<CGameObject*>(pIItem))->lua_game_object());

	if (((pBattery && pBattery->Empty()) || (pRepairKit && pRepairKit->Empty()) || (pFilter && pFilter->Empty())) && entity_alive->Local())
	{
		NET_Packet					P;
		CGameObject::u_EventGen		(P, GE_OWNERSHIP_REJECT, entity_alive->ID());
		P.w_u16						(pIItem->object().ID());
		CGameObject::u_EventSend	(P);

		CGameObject::u_EventGen		(P, GE_DESTROY, pIItem->object().ID());
		CGameObject::u_EventSend	(P);

		return		false;
	}
	else if (!pItemToEat->m_bUnlimited && pItemToEat->Empty() && entity_alive->Local())
	{
		NET_Packet					P;
		CGameObject::u_EventGen		(P, GE_OWNERSHIP_REJECT, entity_alive->ID());
		P.w_u16						(pIItem->object().ID());
		CGameObject::u_EventSend	(P);

		CGameObject::u_EventGen		(P, GE_DESTROY, pIItem->object().ID());
		CGameObject::u_EventSend	(P);

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

bool CInventory::InSlot(PIItem pIItem) const
{
	if (pIItem->GetSlot() < m_slots.size() && 
		m_slots[pIItem->GetSlot()].m_pIItem == pIItem)
		return true;
	return false;
}
bool CInventory::InBelt(PIItem pIItem) const
{
	if (Get(pIItem->object().ID(), false)) return true;
	return false;
}
bool CInventory::InRuck(PIItem pIItem) const
{
	if (Get(pIItem->object().ID(), true)) return true;
	return false;
}


bool CInventory::CanPutInSlot(PIItem pIItem) const
{
	if (!m_bSlotsUseful) return false;

	if ( !GetOwner()->CanPutInSlot(pIItem, pIItem->GetSlot() ) ) return false;

	CCustomOutfit* pOutfit = m_pOwner->GetOutfit();
	CHelmet* pHelmet1 = smart_cast<CHelmet*>(m_pOwner->inventory().ItemFromSlot(HELMET_SLOT));
	CHelmet* pHelmet2 = smart_cast<CHelmet*>(m_pOwner->inventory().ItemFromSlot(SECOND_HELMET_SLOT));

	if (pOutfit || pHelmet1 || pHelmet2)
	{
		if (pIItem->GetSlot() == HELMET_SLOT)
		{
			if ((pOutfit && !pOutfit->bIsHelmetAvaliable) || (pHelmet2 && !pHelmet2->m_bSecondHelmetEnabled))
				return false;
		}

		if (pIItem->GetSlot() == SECOND_HELMET_SLOT)
		{
			if ((pOutfit && !pOutfit->bIsSecondHelmetAvaliable) || (pHelmet1 && !pHelmet1->m_bSecondHelmetEnabled))
				return false;
		}
	}

	if (pIItem->GetSlot() < m_slots.size() && 
		m_slots[pIItem->GetSlot()].m_pIItem == NULL )
		return true;
	
	return false;
}
//проверяет можем ли поместить вещь на пояс,
//при этом реально ничего не меняется
bool CInventory::CanPutInBelt(PIItem pIItem)
{
	if (InBelt(pIItem))					return false;
	if (!m_bBeltUseful)					return false;
	if (!pIItem || !pIItem->Belt())		return false;
	if (m_belt.size() == BeltWidth())	return false;

	return FreeRoom_inBelt(m_belt, pIItem, BeltWidth(), 1);
}
//проверяет можем ли поместить вещь в рюкзак,
//при этом реально ничего не меняется
bool CInventory::CanPutInRuck(PIItem pIItem) const
{
	if (InRuck(pIItem)) return false;
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
	if (inventory_item->object().getDestroy()) return false;

	if (!inventory_item->CanTake()) return false;

	TIItemContainer::const_iterator it = m_all.begin();

	for (; it != m_all.end(); it++)
		if ((*it)->object().ID() == inventory_item->object().ID()) break;

	VERIFY3(it == m_all.end(), "item already exists in inventory",*inventory_item->object().cName());

	CActor* pActor = smart_cast<CActor*>(m_pOwner);
	//актер всегда может взять вещь
	if (!pActor && (TotalWeight() + inventory_item->Weight() > m_pOwner->MaxCarryWeight()))
		return	false;

	return	true;
}


u32  CInventory::BeltWidth() const
{
	return m_iMaxBelt;
}

void  CInventory::AddAvailableItems(TIItemContainer& items_container, bool for_trade) const
{
	for(TIItemContainer::const_iterator it = m_ruck.begin(); m_ruck.end() != it; ++it) 
	{
		PIItem pIItem = *it;
		if (!for_trade || pIItem->CanTrade())
			items_container.push_back(pIItem);
	}

	if (m_bBeltUseful)
	{
		for(TIItemContainer::const_iterator it = m_belt.begin(); m_belt.end() != it; ++it) 
		{
			PIItem pIItem = *it;
			if (!for_trade || pIItem->CanTrade())
				items_container.push_back(pIItem);
		}
	}
	
	CAI_Stalker* pOwner = smart_cast<CAI_Stalker*>(m_pOwner);
	if (pOwner && !pOwner->g_Alive())
	{
		TISlotArr::const_iterator slot_it			= m_slots.begin();
		TISlotArr::const_iterator slot_it_e			= m_slots.end();
		for(;slot_it!=slot_it_e;++slot_it)
		{
			const CInventorySlot& S = *slot_it;
			if (S.m_pIItem && (S.m_pIItem->GetSlot() == RIFLE_SLOT || S.m_pIItem->GetSlot() == GRENADE_SLOT))
				items_container.push_back(S.m_pIItem);
		}

	}
	else if (m_bSlotsUseful) {
		TISlotArr::const_iterator slot_it = m_slots.begin();
		TISlotArr::const_iterator slot_it_e = m_slots.end();
		for (; slot_it != slot_it_e; ++slot_it)
		{
			const CInventorySlot& S = *slot_it;
			if (S.m_pIItem && (!for_trade || S.m_pIItem->CanTrade()))
			{
				if (!S.m_bPersistent || S.m_pIItem->GetSlot()==GRENADE_SLOT )
					items_container.push_back(S.m_pIItem);
			}
		}
	}		
}

bool CInventory::isBeautifulForActiveSlot	(CInventoryItem *pIItem)
{
	if (!IsGameTypeSingle()) return (true);
	TISlotArr::iterator it =  m_slots.begin();
	for( ; it!=m_slots.end(); ++it) {
		if ((*it).m_pIItem && (*it).m_pIItem->IsNecessaryItem(pIItem))
			return		(true);
	}
	return				(false);
}

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
void CInventory::SetSlotsBlocked(u32 mask, bool bBlock)
{
	bool bChanged = false;
	for(u32 i = 0; i < m_slots.size(); ++i)
	{
		if (mask & ((u32)1<<i))
		{
			bool bCanBeActivated = m_slots[i].CanBeActivated();

			if (bBlock)
			{
				++m_slots[i].m_blockCounter;
				VERIFY2(m_slots[i].m_blockCounter< 5,"block slots overflow");
			}
			else
			{
				--m_slots[i].m_blockCounter;
				VERIFY2(m_slots[i].m_blockCounter>-5,"block slots underflow");
			}

			if (bCanBeActivated != m_slots[i].CanBeActivated())
				bChanged = true;
		}
	}
	if (bChanged)
	{
		u32 ActiveSlot		= GetActiveSlot();
		u32 PrevActiveSlot	= GetPrevActiveSlot();
		if (ActiveSlot==NO_ACTIVE_SLOT)
		{//try to restore hidden weapon
			if (PrevActiveSlot!=NO_ACTIVE_SLOT && m_slots[PrevActiveSlot].CanBeActivated()) 
				if (Activate(PrevActiveSlot))
					SetPrevActiveSlot(NO_ACTIVE_SLOT);
		}
		else
		{//try to hide active weapon
			if (!m_slots[ActiveSlot].CanBeActivated() )
				if (Activate(NO_ACTIVE_SLOT))
					SetPrevActiveSlot(ActiveSlot);
		}
	}
}
