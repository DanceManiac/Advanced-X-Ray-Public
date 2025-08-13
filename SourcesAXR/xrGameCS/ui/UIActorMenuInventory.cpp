#include "stdafx.h"
#include "pch_script.h"
#include "UIActorMenu.h"
#include "../inventory.h"
#include "../inventoryOwner.h"
#include "UIInventoryUtilities.h"
#include "UIItemInfo.h"
#include "../Level.h"
#include "UICellItemFactory.h"
#include "UIDragDropListEx.h"
#include "UIDragDropReferenceList.h"
#include "UICellCustomItems.h"
#include "UIItemInfo.h"
#include "UIFrameLineWnd.h"
#include "UIPropertiesBox.h"
#include "UIListBoxItem.h"
#include "UIMainIngameWnd.h"
#include "UIGameCustom.h"
#include "ui\UIActorMenu.h"
#include "HUDManager.h"

#include "../silencer.h"
#include "../scope.h"
#include "../grenadelauncher.h"
#include "../LaserDesignator.h"
#include "../TacticalTorch.h"
#include "../Artefact.h"
#include "../eatable_item.h"
#include "../BottleItem.h"
#include "../WeaponMagazined.h"
#include "../Medkit.h"
#include "../Antirad.h"
#include "../CustomOutfit.h"
#include "../ActorHelmet.h"
#include "../player_hud.h"
#include "../CustomDetector.h"
#include "../AnomalyDetector.h"
#include "../Torch.h"
#include "../PDA.h"
#include "../Battery.h"
#include "../AntigasFilter.h"
#include "../RepairKit.h"
#include "../ArtefactContainer.h"
#include "../SleepingBag.h"
#include "UI3tButton.h"
#include "../UICursor.h"
#include "../MPPlayersBag.h"
#include "../HUDManager.h"

#include "../Actor.h"
#include "AdvancedXrayGameConstants.h"

#include "../ai_space.h"
#include "script_engine.h"
#include "script_game_object.h"
#include "script_export_space.h"
#include "script_storage.h"

using namespace luabind;

void move_item_from_to(u16 from_id, u16 to_id, u16 what_id);

void CUIActorMenu::InitInventoryMode()
{
	m_pInventoryBagList->Show			(true);
	m_pInventoryBeltList->Show			(true);
	m_pInventoryOutfitList->Show		(true);
	m_pInventoryDetectorList->Show		(true);
	m_pInventoryPistolList->Show		(true);
	m_pInventoryAutomaticList->Show		(true);
	m_pInventoryBoltList->Show			(true);

	if (m_pQuickSlot)
		m_pQuickSlot->Show				(true);
	
	if (m_pTrashList)
		m_pTrashList->Show				(true);

	if (m_sleep_button)
		m_sleep_button->Show			(true);

	m_RightDelimiter->Show				(false);
	if (m_clock_value)
		m_clock_value->Show				(true);

	m_pInventoryKnifeList->Show(true);
	m_pInventoryBinocularList->Show(true);
	m_pInventoryTorchList->Show(true);
	m_pInventoryBackpackList->Show(true);
	m_pInventoryHelmetList->Show(true);
	m_pInventorySecondHelmetList->Show(true);
	m_pInventoryDosimeterList->Show(true);
	m_pInventoryPantsList->Show(true);

	InitInventoryContents				(m_pInventoryBagList);

	VERIFY( HUD().GetUI() && HUD().GetUI()->UIMainIngameWnd );

	if (psHUD_Flags.test(HUD_MINIMAP_INVENTORY))
	{
		HUD().GetUI()->UIMainIngameWnd->ShowZoneMap(true);
	}
	else
	{
		HUD().GetUI()->UIMainIngameWnd->ShowZoneMap(false);
	}

	m_bNeedMoveAfsToBag = false;
}

void CUIActorMenu::DeInitInventoryMode()
{
	if (m_pTrashList)
		m_pTrashList->Show				(false);
	if (m_sleep_button)
		m_sleep_button->Show			(false);
	if (m_clock_value)
		m_clock_value->Show					(false);
}

void CUIActorMenu::SendEvent_ActivateSlot(u32 slot, u16 recipient)
{
	NET_Packet						P;
	CGameObject::u_EventGen			(P, GEG_PLAYER_ACTIVATE_SLOT, recipient);
	P.w_u32							(slot);
	CGameObject::u_EventSend		(P);
}

void CUIActorMenu::SendEvent_Item2Slot(PIItem pItem, u16 recipient)
{
	if(pItem->parent_id()!=recipient)
		move_item_from_to			(pItem->parent_id(), recipient, pItem->object_id());

	NET_Packet						P;
	CGameObject::u_EventGen			(P, GEG_PLAYER_ITEM2SLOT, pItem->object().H_Parent()->ID());
	P.w_u16							(pItem->object().ID());
	CGameObject::u_EventSend		(P);

	PlaySnd							(eItemToSlot);
	clear_highlight_lists			();
};

void CUIActorMenu::SendEvent_Item2Belt(PIItem pItem, u16 recipient)
{
	if(pItem->parent_id()!=recipient)
		move_item_from_to			(pItem->parent_id(), recipient, pItem->object_id());

	NET_Packet						P;
	CGameObject::u_EventGen			(P, GEG_PLAYER_ITEM2BELT, pItem->object().H_Parent()->ID());
	P.w_u16							(pItem->object().ID());
	CGameObject::u_EventSend		(P);

	PlaySnd							(eItemToBelt);
	clear_highlight_lists			();
};

void CUIActorMenu::SendEvent_Item2Ruck(PIItem pItem, u16 recipient)
{
	if(pItem->parent_id()!=recipient)
		move_item_from_to			(pItem->parent_id(), recipient, pItem->object_id());

	NET_Packet						P;
	CGameObject::u_EventGen			(P, GEG_PLAYER_ITEM2RUCK, pItem->object().H_Parent()->ID());
	P.w_u16							(pItem->object().ID());
	CGameObject::u_EventSend		(P);
	
	clear_highlight_lists			();
	PlaySnd							(eItemToRuck);
};

void CUIActorMenu::SendEvent_Item_Eat(PIItem pItem, u16 recipient)
{
	if(pItem->parent_id()!=recipient)
		move_item_from_to			(pItem->parent_id(), recipient, pItem->object_id());

	NET_Packet						P;
	CGameObject::u_EventGen			(P, GEG_PLAYER_ITEM_EAT, recipient);
	P.w_u16							(pItem->object().ID());
	CGameObject::u_EventSend		(P);
};

void CUIActorMenu::SendEvent_Item_Drop(PIItem pItem, u16 recipient)
{
	R_ASSERT(pItem->parent_id()==recipient);
	if (!IsGameTypeSingle())
		pItem->DenyTrade();
	//pItem->SetDropManual			(TRUE);
	NET_Packet					P;
	pItem->object().u_EventGen	(P,GE_OWNERSHIP_REJECT,pItem->parent_id());
	P.w_u16						(pItem->object().ID());
	pItem->object().u_EventSend	(P);
	PlaySnd						(eDropItem);
	clear_highlight_lists		();
}

void CUIActorMenu::DropAllCurrentItem()
{
	if ( CurrentIItem() && !CurrentIItem()->IsQuestItem() )
	{
		u32 const cnt = CurrentItem()->ChildsCount();
		for( u32 i = 0; i < cnt; ++i )
		{
			CUICellItem*	itm  = CurrentItem()->PopChild(NULL);
			PIItem			iitm = (PIItem)itm->m_pData;
			SendEvent_Item_Drop( iitm, m_pActorInvOwner->object_id() );
		}

		SendEvent_Item_Drop( CurrentIItem(), m_pActorInvOwner->object_id() );
	}
	SetCurrentItem								(NULL);
}

bool CUIActorMenu::DropAllItemsFromRuck( bool quest_force )
{
	if ( !IsShown() || !m_pInventoryBagList || m_currMenuMode != mmInventory )
	{
		return false;
	}

	u32 const ci_count = m_pInventoryBagList->ItemsCount();
	for ( u32 i = 0; i < ci_count; ++i )
	{
		CUICellItem* ci = m_pInventoryBagList->GetItemIdx( i );
		VERIFY( ci );
		PIItem item = (PIItem)ci->m_pData;
		VERIFY( item );

		if ( !quest_force && item->IsQuestItem() )
		{
			continue;
		}

		u32 const cnt = ci->ChildsCount();
		for( u32 j = 0; j < cnt; ++j )
		{
			CUICellItem*	child_ci   = ci->PopChild(NULL);
			PIItem			child_item = (PIItem)child_ci->m_pData;
			SendEvent_Item_Drop( child_item, m_pActorInvOwner->object_id() );
		}
		SendEvent_Item_Drop( item, m_pActorInvOwner->object_id() );
	}

	SetCurrentItem( NULL );
	return true;
}

bool FindItemInList(CUIDragDropListEx* lst, PIItem pItem, CUICellItem*& ci_res)
{
	u32 count = lst->ItemsCount();
	for (u32 i=0; i<count; ++i)
	{
		CUICellItem* ci				= lst->GetItemIdx(i);
		for(u32 j=0; j<ci->ChildsCount(); ++j)
		{
			CUIInventoryCellItem* ici = smart_cast<CUIInventoryCellItem*>(ci->Child(j));
			if(ici->object()==pItem)
			{
				ci_res = ici;
				//lst->RemoveItem(ci,false);
				return true;
			}
		}

		CUIInventoryCellItem* ici = smart_cast<CUIInventoryCellItem*>(ci);
		if(ici->object()==pItem)
		{
			ci_res = ci;
			//lst->RemoveItem(ci,false);
			return true;
		}
	}
	return false;
}

bool RemoveItemFromList(CUIDragDropListEx* lst, PIItem pItem)
{// fixme
	CUICellItem*	ci	= NULL;
	if(FindItemInList(lst, pItem, ci))
	{
		R_ASSERT		(ci);

		CUICellItem* dying_cell = lst->RemoveItem	(ci, false);
		xr_delete(dying_cell);

		return			true;
	}else
		return			false;
}

void CUIActorMenu::OnInventoryAction(PIItem pItem, u16 action_type)
{
	CUIDragDropListEx* all_lists[] =
	{
		m_pInventoryBeltList,
		m_pInventoryBoltList,
		m_pInventoryPistolList,
		m_pInventoryAutomaticList,
		m_pInventoryOutfitList,
		m_pInventoryDetectorList,
		m_pInventoryBagList,
		m_pTradeActorBagList,
		m_pTradeActorList,
		m_pInventoryKnifeList,
		m_pInventoryBinocularList,
		m_pInventoryTorchList,
		m_pInventoryBackpackList,
		m_pInventoryHelmetList,
		m_pInventorySecondHelmetList,
		m_pInventoryDosimeterList,
		m_pInventoryPantsList,
		NULL
	};

	switch (action_type)
	{
	case GE_TRADE_BUY:
	case GE_OWNERSHIP_TAKE:
		{
			bool b_already = false;

				CUIDragDropListEx* lst_to_add		= nullptr;
				EItemPlace pl						= pItem->m_eItemCurrPlace;
				if ( pItem->GetSlot() == GRENADE_SLOT )
				{
					pl = eItemPlaceRuck;
				}
#ifndef MASTER_GOLD
				Msg("item place [%d]", pl);
#endif // #ifndef MASTER_GOLD

				if(pl == eItemPlaceSlot)
					lst_to_add					= GetSlotList(pItem->GetSlot());
				else if(pl == eItemPlaceRuck)
					lst_to_add					= GetListByType(iActorBag);
				else if(pl == eItemPlaceBelt)
					lst_to_add					= GetListByType(iActorBelt);
		else /* if(pl.type==eItemPlaceRuck)*/
		{
			if (pItem->parent_id() == m_pActorInvOwner->object_id())
				lst_to_add = GetListByType(iActorBag);
			else
				lst_to_add = GetListByType(iDeadBodyBag);
		}

		for (auto& curr : all_lists)
		{
			if (!curr) // m_pInventoryHelmetList can be nullptr
				continue;
			CUICellItem* ci = nullptr;

			if (FindItemInList(curr, pItem, ci))
			{
				if (lst_to_add != curr)
				{
					RemoveItemFromList(curr, pItem);
				}
				else
				{
					b_already = true;
				}
				// break;
			}
		}
		CUICellItem* ci = nullptr;
		if (GetMenuMode() == mmDeadBodySearch && FindItemInList(m_pDeadBodyBagList, pItem, ci))
			break;

		if (!b_already)
		{
			if (lst_to_add)
			{
				CUICellItem* itm = create_cell_item(pItem);
				lst_to_add->SetItem(itm);
			}
		}
		if (m_pActorInvOwner && m_pQuickSlot)
			m_pQuickSlot->ReloadReferences(m_pActorInvOwner);
	}
	break;
	case GE_TRADE_SELL:
	case GE_OWNERSHIP_REJECT:
	{
		if (CUIDragDropListEx::m_drag_item)
		{
			CUIInventoryCellItem* ici = smart_cast<CUIInventoryCellItem*>(CUIDragDropListEx::m_drag_item->ParentItem());
			R_ASSERT(ici);
			if (ici->object() == pItem)
			{
				CUIDragDropListEx* _drag_owner = ici->OwnerList();
				_drag_owner->DestroyDragItem();
			}
		}

		for (auto& curr : all_lists)
		{
			if (!curr) // m_pInventoryHelmetList can be nullptr
				continue;

			if (RemoveItemFromList(curr, pItem))
			{
#ifdef DEBUG
				Msg("all ok. item [%d] removed from list", pItem->object_id());
#endif
				break;
			}
		}

		if (m_pActorInvOwner && m_pQuickSlot)
			m_pQuickSlot->ReloadReferences(m_pActorInvOwner);
	}
	break;
	}
	UpdateItemsPlace();
}
void CUIActorMenu::AttachAddon(PIItem item_to_upgrade)
{
	PlaySnd										(eAttachAddon);
	R_ASSERT									(item_to_upgrade);
	if (OnClient())
	{
		NET_Packet								P;
		CGameObject::u_EventGen					(P, GE_ADDON_ATTACH, item_to_upgrade->object().ID());
		P.w_u16									(CurrentIItem()->object().ID());
		CGameObject::u_EventSend				(P);
	};

	item_to_upgrade->Attach						(CurrentIItem(), true);

	SetCurrentItem								(NULL);
}

void CUIActorMenu::DetachAddon(LPCSTR addon_name, PIItem itm)
{
	PlaySnd										(eDetachAddon);
	if (OnClient())
	{
		NET_Packet								P;
		if(itm==NULL)
			CGameObject::u_EventGen				(P, GE_ADDON_DETACH, CurrentIItem()->object().ID());
		else
			CGameObject::u_EventGen				(P, GE_ADDON_DETACH, itm->object().ID());

		P.w_stringZ								(addon_name);
		CGameObject::u_EventSend				(P);
		return;
	}
	if(itm==NULL)
		CurrentIItem()->Detach					(addon_name, true);
	else
		itm->Detach								(addon_name, true);
}

void CUIActorMenu::InitCellForSlot( u32 slot_idx ) 
{
	VERIFY( KNIFE_SLOT <= slot_idx && slot_idx <= LAST_SLOT);
	PIItem item	= m_pActorInvOwner->inventory().m_slots[slot_idx].m_pIItem;
	if ( !item )
	{
		return;
	}

	CUIDragDropListEx* curr_list	= GetSlotList( slot_idx );
	if (!curr_list)
		return;

	CUICellItem* cell_item			= create_cell_item( item );
	curr_list->SetItem( cell_item );
	if ( m_currMenuMode == mmTrade && m_pPartnerInvOwner )
	{
		ColorizeItem( cell_item, !CanMoveToPartner( item ) );
	}
}

void CUIActorMenu::InitInventoryContents(CUIDragDropListEx* pBagList) 
{
	ClearAllLists				();
	m_pMouseCapturer			= NULL;
	m_UIPropertiesBox->Hide		();
	SetCurrentItem				(NULL);

	CUIDragDropListEx*			curr_list = NULL;
	//Slots
	InitCellForSlot				(PISTOL_SLOT);
	InitCellForSlot				(RIFLE_SLOT);
	InitCellForSlot				(OUTFIT_SLOT);
	InitCellForSlot				(DETECTOR_SLOT);
	InitCellForSlot(KNIFE_SLOT);
	InitCellForSlot(APPARATUS_SLOT);
	InitCellForSlot(TORCH_SLOT);
	InitCellForSlot(BACKPACK_SLOT);
	InitCellForSlot(HELMET_SLOT);
	InitCellForSlot(SECOND_HELMET_SLOT);
	InitCellForSlot(DOSIMETER_SLOT);
	InitCellForSlot(PANTS_SLOT);
	InitCellForSlot(PDA_SLOT);

	curr_list					= m_pInventoryBeltList;
	TIItemContainer::iterator itb = m_pActorInvOwner->inventory().m_belt.begin();
	TIItemContainer::iterator ite = m_pActorInvOwner->inventory().m_belt.end();
	for ( ; itb != ite; ++itb )
	{
		CUICellItem* itm		= create_cell_item(*itb);
		curr_list->SetItem		(itm);
		if ( m_currMenuMode == mmTrade && m_pPartnerInvOwner )
		{
			ColorizeItem( itm, !CanMoveToPartner( *itb ) );
		}
	}

	TIItemContainer				ruck_list;
	ruck_list					= m_pActorInvOwner->inventory().m_ruck;
	std::sort					( ruck_list.begin(), ruck_list.end(), InventoryUtilities::GreaterRoomInRuck );

	curr_list					= pBagList;

	itb = ruck_list.begin();
	ite = ruck_list.end();
	for ( ; itb != ite; ++itb )
	{
		CMPPlayersBag* bag = smart_cast<CMPPlayersBag*>( &(*itb)->object() );
		if ( bag )
		{
			continue;
		}
		CUICellItem* itm = create_cell_item( *itb );
		curr_list->SetItem(itm);
		if ( m_currMenuMode == mmTrade && m_pPartnerInvOwner )
		{
			ColorizeItem( itm, !CanMoveToPartner( *itb ) );
		}
	}
	if (m_pQuickSlot)
		m_pQuickSlot->ReloadReferences(m_pActorInvOwner);
}

bool CUIActorMenu::TryActiveSlot(CUICellItem* itm)
{
	PIItem	iitem	= (PIItem)itm->m_pData;
	u32 slot		= iitem->GetSlot();

	if ( slot == GRENADE_SLOT )
	{
		PIItem	prev_iitem = m_pActorInvOwner->inventory().m_slots[slot].m_pIItem;
		if ( prev_iitem && (prev_iitem->object().cNameSect() != iitem->object().cNameSect()) )
		{
			SendEvent_Item2Ruck( prev_iitem, m_pActorInvOwner->object_id() );
			SendEvent_Item2Slot( iitem, m_pActorInvOwner->object_id() );
		}
		SendEvent_ActivateSlot( slot, m_pActorInvOwner->object_id() );
		return true;
	}
	if ( slot == DETECTOR_SLOT )
	{

	}
	return false;
}

bool CUIActorMenu::ToSlot(CUICellItem* itm, bool force_place)
{
	CUIDragDropListEx*	old_owner			= itm->OwnerList();
	PIItem	iitem							= (PIItem)itm->m_pData;
	u32 _slot								= iitem->GetSlot();

	bool b_own_item							= (iitem->parent_id()==m_pActorInvOwner->object_id());

	CCustomOutfit* pOutfit = m_pActorInvOwner->GetOutfit();
	CHelmet* pHelmet1 = smart_cast<CHelmet*>(m_pActorInvOwner->inventory().ItemFromSlot(HELMET_SLOT));
	CHelmet* pHelmet2 = smart_cast<CHelmet*>(m_pActorInvOwner->inventory().ItemFromSlot(SECOND_HELMET_SLOT));

	if (_slot == HELMET_SLOT || _slot == SECOND_HELMET_SLOT)
	{
		if (pOutfit || pHelmet1 || pHelmet2)
		{
			CHelmet* pHelmet1 = smart_cast<CHelmet*>(m_pActorInvOwner->inventory().ItemFromSlot(HELMET_SLOT));
			CHelmet* pHelmet2 = smart_cast<CHelmet*>(m_pActorInvOwner->inventory().ItemFromSlot(SECOND_HELMET_SLOT));

			if (_slot == HELMET_SLOT)
			{
				if (pOutfit && !pOutfit->bIsHelmetAvaliable)
					return false;

				if (pHelmet2 && !pHelmet2->m_bSecondHelmetEnabled)
					return false;
			}
			else if (_slot == SECOND_HELMET_SLOT)
			{
				if (pOutfit && !pOutfit->bIsSecondHelmetAvaliable)
					return false;

				if (pHelmet1 && !pHelmet1->m_bSecondHelmetEnabled)
					return false;
			}
		}
	}

	if(m_pActorInvOwner->inventory().CanPutInSlot(iitem))
	{
		CUIDragDropListEx* new_owner		= GetSlotList(_slot);
		
		if ( _slot == GRENADE_SLOT || !new_owner )
		{
			return true; //fake, sorry (((
		}

		if (_slot == OUTFIT_SLOT)
		{
			CCustomOutfit* pOutfit = smart_cast<CCustomOutfit*>(iitem);
			if (pOutfit)
			{
				if (!pOutfit->bIsHelmetAvaliable)
				{
					CUIDragDropListEx* helmet_list = GetSlotList(HELMET_SLOT);
					if (helmet_list->ItemsCount() == 1)
					{
						CUICellItem* helmet_cell = helmet_list->GetItemIdx(0);
						ToBag(helmet_cell, false);
					}
				}

				if (!pOutfit->bIsSecondHelmetAvaliable)
				{
					CUIDragDropListEx* second_helmet_list = GetSlotList(SECOND_HELMET_SLOT);
					if (second_helmet_list->ItemsCount() == 1)
					{
						CUICellItem* second_helmet_cell = second_helmet_list->GetItemIdx(0);
						ToBag(second_helmet_cell, false);
					}
				}
			}
		}

		CHelmet* helmet = smart_cast<CHelmet*>(iitem);

		if (_slot == HELMET_SLOT)
		{
			if (helmet && !helmet->m_bSecondHelmetEnabled)
			{
				CUIDragDropListEx* second_helmet_list = GetSlotList(SECOND_HELMET_SLOT);
				if (second_helmet_list->ItemsCount() == 1)
				{
					CUICellItem* second_helmet_cell = second_helmet_list->GetItemIdx(0);
						ToBag(second_helmet_cell, false);
				}
			}
		}
		else if (_slot == SECOND_HELMET_SLOT)
		{
			if (helmet && !helmet->m_bSecondHelmetEnabled)
			{
				CUIDragDropListEx* helmet_list = GetSlotList(HELMET_SLOT);
				if (helmet_list->ItemsCount() == 1)
				{
					CUICellItem* helmet_cell = helmet_list->GetItemIdx(0);
					ToBag(helmet_cell, false);
				}
			}
		}
	
		bool result							= (!b_own_item) || m_pActorInvOwner->inventory().Slot(iitem);
		VERIFY								(result);

		CUICellItem* i						= old_owner->RemoveItem(itm, (old_owner==new_owner) );

		new_owner->SetItem					(i);
	
//.		if(!b_own_item)
		SendEvent_Item2Slot					(iitem, m_pActorInvOwner->object_id());

		SendEvent_ActivateSlot				(_slot, m_pActorInvOwner->object_id());
		
		//ColorizeItem						( itm, false );
		if ( _slot == OUTFIT_SLOT )
		{
			MoveArtefactsToBag();
		}
		return								true;
	}
	else
	{ // in case slot is busy
		if ( !force_place || _slot == NO_ACTIVE_SLOT )
			return false;
		if ( m_pActorInvOwner->inventory().m_slots[_slot].m_bPersistent && _slot != DETECTOR_SLOT  )
		{
			return false;
		}

		PIItem	_iitem						= m_pActorInvOwner->inventory().m_slots[_slot].m_pIItem;
		CUIDragDropListEx* slot_list		= GetSlotList(_slot);
		VERIFY								(slot_list->ItemsCount()==1);

		CUICellItem* slot_cell				= slot_list->GetItemIdx(0);
		VERIFY								(slot_cell && ((PIItem)slot_cell->m_pData)==_iitem);

		bool result							= ToBag(slot_cell, false);
		VERIFY								(result);

		result								= ToSlot(itm, false);
		if(b_own_item && result && _slot==DETECTOR_SLOT)
		{
			CCustomDetector* det			= smart_cast<CCustomDetector*>(iitem);
			det->ToggleDetector				(g_player_hud->attached_item(0)!=NULL);
		}

		return result;
	}
}


bool CUIActorMenu::ToBag(CUICellItem* itm, bool b_use_cursor_pos)
{
	PIItem	iitem						= (PIItem)itm->m_pData;

	bool b_own_item						= (iitem->parent_id()==m_pActorInvOwner->object_id());

	bool b_already						= m_pActorInvOwner->inventory().InRuck(iitem);

	CUIDragDropListEx*	old_owner		= itm->OwnerList();
	CUIDragDropListEx*	new_owner		= NULL;
	if(b_use_cursor_pos)
	{
			new_owner					= CUIDragDropListEx::m_drag_item->BackList();
			VERIFY						(GetListType(new_owner)==iActorBag);
	}else
			new_owner					= GetListByType(iActorBag);

	if(m_pActorInvOwner->inventory().CanPutInRuck(iitem) || (b_already && (new_owner!=old_owner)) )
	{
		bool result							= b_already || (!b_own_item || m_pActorInvOwner->inventory().Ruck(iitem) );
		VERIFY								(result);
		CUICellItem* i						= old_owner->RemoveItem(itm, (old_owner==new_owner) );
		if (!i)
			return false;

		if(b_use_cursor_pos)
			new_owner->SetItem				(i,old_owner->GetDragItemPosition());
		else
			new_owner->SetItem				(i);

		if(!b_already || !b_own_item)
			SendEvent_Item2Ruck					(iitem, m_pActorInvOwner->object_id());

		if ( m_currMenuMode == mmTrade && m_pPartnerInvOwner )
		{
			ColorizeItem( itm, !CanMoveToPartner( iitem ) );
		}
		return true;
	}
	return false;
}

bool CUIActorMenu::ToBelt(CUICellItem* itm, bool b_use_cursor_pos)
{
	PIItem	iitem						= (PIItem)itm->m_pData;
	bool b_own_item						= (iitem->parent_id()==m_pActorInvOwner->object_id());

	if(m_pActorInvOwner->inventory().CanPutInBelt(iitem))
	{
		CUIDragDropListEx*	old_owner		= itm->OwnerList();
		CUIDragDropListEx*	new_owner		= NULL;
		if(b_use_cursor_pos){
				new_owner					= CUIDragDropListEx::m_drag_item->BackList();
				VERIFY						(new_owner==m_pInventoryBeltList);
		}else
				new_owner					= m_pInventoryBeltList;

		bool result							= (!b_own_item) || m_pActorInvOwner->inventory().Belt(iitem);
		VERIFY								(result);
		CUICellItem* i						= old_owner->RemoveItem(itm, (old_owner==new_owner) );

		if(b_use_cursor_pos)
			new_owner->SetItem				(i,old_owner->GetDragItemPosition());
		else
			new_owner->SetItem				(i);

		if(!b_own_item)
			SendEvent_Item2Belt				(iitem, m_pActorInvOwner->object_id());

		//ColorizeItem						(itm, false);
		return								true;
	}
	return									false;
}
CUIDragDropListEx* CUIActorMenu::GetSlotList(u32 slot_idx)
{
	if ( slot_idx == NO_ACTIVE_SLOT /*|| m_pActorInvOwner->inventory().m_slots[slot_idx].m_bPersistent*/ )
	{
		return NULL;
	}
	switch ( slot_idx )
	{
		case PISTOL_SLOT:
			return m_pInventoryPistolList;
			break;

		case RIFLE_SLOT:
			return m_pInventoryAutomaticList;
			break;

		case OUTFIT_SLOT:
			return m_pInventoryOutfitList;
			break;

		case DETECTOR_SLOT:
			return m_pInventoryDetectorList;
			break;

		case GRENADE_SLOT://fake
		{
			if (m_currMenuMode == mmTrade)
			{
				return m_pTradeActorBagList;
			}
		}break;

		case KNIFE_SLOT:
		{
			return m_pInventoryKnifeList;
		}break;

		case APPARATUS_SLOT:
		{
			return m_pInventoryBinocularList;
		}break;

		case TORCH_SLOT:
		{
			return m_pInventoryTorchList;
		}break;

		case BACKPACK_SLOT:
		{
			return m_pInventoryBackpackList;
		}break;

		case DOSIMETER_SLOT:
		{
			return m_pInventoryDosimeterList;
		}break;

		case PANTS_SLOT:
		{
				return m_pInventoryPantsList;
		}break;

		case HELMET_SLOT:
		{
			return m_pInventoryHelmetList;
		}break;

		case SECOND_HELMET_SLOT:
		{
			return m_pInventorySecondHelmetList;
		}break;
	};
	return NULL;
}

bool CUIActorMenu::TryUseItem( CUICellItem* cell_itm )
{
	if ( !cell_itm )
	{
		return false;
	}
	PIItem item	= (PIItem)cell_itm->m_pData;

	CBottleItem*	pBottleItem		= smart_cast<CBottleItem*>	(item);
	CMedkit*		pMedkit			= smart_cast<CMedkit*>		(item);
	CAntirad*		pAntirad		= smart_cast<CAntirad*>		(item);
	CEatableItem*	pEatableItem	= smart_cast<CEatableItem*>	(item);
	CBattery*		pBattery		= smart_cast<CBattery*>		(item);
	CAntigasFilter* pFilter			= smart_cast<CAntigasFilter*>(item);
	CRepairKit*		pRepairKit		= smart_cast<CRepairKit*>	(item);
	CSleepingBag*	pSleepingBag	= smart_cast<CSleepingBag*>	(item);

	if (pSleepingBag)
	{
		pSleepingBag->StartSleep();

		PlaySnd(eItemUse);
		cell_itm->Update();
		SetCurrentItem(NULL);

		return true;
	}

	if ( !(pMedkit || pAntirad || pEatableItem || pBottleItem || pBattery || pFilter || pRepairKit) )
	{
		return false;
	}

	if ( !item->Useful() || (pFilter && !pFilter->UseAllowed()) || (pRepairKit && !pRepairKit->UseAllowed()))
	{
		return false;
	}

	u16 recipient = m_pActorInvOwner->object_id();
	if ( item->parent_id() != recipient )
	{
		//move_item_from_to	(itm->parent_id(), recipient, itm->object_id());
		cell_itm->OwnerList()->RemoveItem( cell_itm, false );
	}

	SendEvent_Item_Eat		( item, recipient );
	PlaySnd					( eItemUse );
	cell_itm->Update		();
	SetCurrentItem			( NULL );
	return true;
}

bool CUIActorMenu::ToQuickSlot(CUICellItem* itm)
{
	PIItem iitem = (PIItem)itm->m_pData;
	CEatableItemObject* eat_item = smart_cast<CEatableItemObject*>(iitem);
	if(!eat_item)
		return false;

	//Alundaio: Fix deep recursion if placing icon greater then col/row set in actor_menu.xml
	Ivector2 iWH = iitem->GetInvGridRect().rb;
	if (iWH.x > 1 || iWH.y > 1)
		return false;
	//Alundaio: END
		
    if (m_pQuickSlot)
    {
		u8 slot_idx = 0;

		if (!b_quick_vert)
			slot_idx = u8(m_pQuickSlot->PickCell(GetUICursor().GetCursorPosition()).x);
		else
			slot_idx = u8(m_pQuickSlot->PickCell(GetUICursor().GetCursorPosition()).y);

		if(slot_idx==255)
			return false;

		m_pQuickSlot->SetItem(create_cell_item(iitem), GetUICursor().GetCursorPosition());
		xr_strcpy(ACTOR_DEFS::g_quick_use_slots[slot_idx], iitem->m_section_id.c_str());
	}
	return true;
}


bool CUIActorMenu::OnItemDropped(PIItem itm, CUIDragDropListEx* new_owner, CUIDragDropListEx* old_owner)
{
	CUICellItem*	_citem	= (new_owner->ItemsCount()==1) ? new_owner->GetItemIdx(0) : NULL;
	PIItem _iitem	= _citem ? (PIItem)_citem->m_pData : NULL;

	if(!_iitem)						return	false;
	if(!_iitem->CanAttach(itm))		return	false;

	if(old_owner != m_pInventoryBagList)	return	false;

	AttachAddon						(_iitem);

	return							true;
}

void CUIActorMenu::TryHidePropertiesBox()
{
	if ( m_UIPropertiesBox->IsShown() )
	{
		m_UIPropertiesBox->Hide();
	}
}

void CUIActorMenu::ActivatePropertiesBox()
{
	TryHidePropertiesBox();
	if ( !(m_currMenuMode == mmInventory || m_currMenuMode == mmDeadBodySearch || m_currMenuMode == mmUpgrade) )
	{
		return;
	}

	PIItem item = CurrentIItem();
	if ( !item )
	{
		return;
	}

	CUICellItem* cell_item = CurrentItem();
	m_UIPropertiesBox->RemoveAll();
	bool b_show = false;

	if ( m_currMenuMode == mmInventory || m_currMenuMode == mmDeadBodySearch)
	{
		PropertiesBoxForSlots( item, b_show );
		PropertiesBoxForWeapon( cell_item, item, b_show );
		PropertiesBoxForAddon( item, b_show );
		PropertiesBoxForUsing( item, b_show );
		PropertiesBoxForPlaying(item, b_show);
		if ( m_currMenuMode == mmInventory )
			PropertiesBoxForDrop( cell_item, item, b_show );
	}
	else if ( m_currMenuMode == mmUpgrade )
	{
		PropertiesBoxForRepair( item, b_show );
	}

	if ( b_show )
	{
		m_UIPropertiesBox->AutoUpdateSize();
		m_UIPropertiesBox->BringAllToTop();

		Fvector2 cursor_pos_;
		Frect						vis_rect;
		GetAbsoluteRect				(vis_rect);
		cursor_pos_					= GetUICursor().GetCursorPosition();
		cursor_pos_.sub				(vis_rect.lt);
		m_UIPropertiesBox->Show		(vis_rect, cursor_pos_);
		PlaySnd						(eProperties);
	}
}

void CUIActorMenu::PropertiesBoxForSlots( PIItem item, bool& b_show )
{
	CCustomOutfit* pOutfit	= smart_cast<CCustomOutfit*>( item );
	CHelmet* pHelmet		= smart_cast<CHelmet*>		( item );
	CInventory*  inv		= &m_pActorInvOwner->inventory();

	// Флаг-признак для невлючения пункта контекстного меню: Dreess Outfit, если костюм уже надет
	bool bAlreadyDressed	= false;
	u32 const cur_slot		= item->GetSlot();

	if (!pOutfit && !pHelmet && cur_slot != NO_ACTIVE_SLOT
		&& !inv->m_slots[cur_slot].m_bPersistent && inv->CanPutInSlot(item) )
	{
		m_UIPropertiesBox->AddItem( "st_move_to_slot",  NULL, INVENTORY_TO_SLOT_ACTION );
		b_show = true;
	}
	if (item->Belt() && inv->CanPutInBelt( item ))
	{
		m_UIPropertiesBox->AddItem( "st_move_on_belt",  NULL, INVENTORY_TO_BELT_ACTION );
		b_show = true;
	}

	if (item->Ruck() && inv->CanPutInRuck( item )
		&& ( cur_slot == (u32)(-1) || !inv->m_slots[cur_slot].m_bPersistent ) )
	{
		if( !pOutfit )
		{
			if (!pHelmet)
				m_UIPropertiesBox->AddItem("st_move_to_bag", NULL, INVENTORY_TO_BAG_ACTION);
			else
				m_UIPropertiesBox->AddItem("st_undress_helmet", NULL, INVENTORY_TO_BAG_ACTION);
		}
		else
		{
			m_UIPropertiesBox->AddItem( "st_undress_outfit",  NULL, INVENTORY_TO_BAG_ACTION );
		}
		bAlreadyDressed = true;
		b_show			= true;
	}
	
	CCustomOutfit* outfit_in_slot = m_pActorInvOwner->GetOutfit();
	CHelmet* helmet_in_slot = smart_cast<CHelmet*>(m_pActorInvOwner->inventory().ItemFromSlot(HELMET_SLOT));
	CHelmet* second_helmet_in_slot = smart_cast<CHelmet*>(m_pActorInvOwner->inventory().ItemFromSlot(SECOND_HELMET_SLOT));
	
	if (pHelmet && !bAlreadyDressed && (!outfit_in_slot || outfit_in_slot->bIsHelmetAvaliable) && 
		(!helmet_in_slot || helmet_in_slot->m_bSecondHelmetEnabled) && 
		(!second_helmet_in_slot || second_helmet_in_slot->m_bSecondHelmetEnabled))
	{
		m_UIPropertiesBox->AddItem( "st_dress_helmet",  NULL, INVENTORY_TO_SLOT_ACTION );
		b_show			= true;
	}
}

void CUIActorMenu::PropertiesBoxForWeapon( CUICellItem* cell_item, PIItem item, bool& b_show )
{
	//отсоединение аддонов от вещи
	CWeapon*	pWeapon = smart_cast<CWeapon*>( item );
	if ( !pWeapon )
	{
		return;
	}

	if ( pWeapon->GrenadeLauncherAttachable())
	{
		if ( pWeapon->IsGrenadeLauncherAttached() )
		{
			m_UIPropertiesBox->AddItem( "st_detach_gl",  NULL, INVENTORY_DETACH_GRENADE_LAUNCHER_ADDON );
			b_show			= true;
		}
	}
	if ( pWeapon->ScopeAttachable())
	{
		if ( pWeapon->IsScopeAttached() )
		{
			m_UIPropertiesBox->AddItem( "st_detach_scope",  NULL, INVENTORY_DETACH_SCOPE_ADDON );
			b_show			= true;
		}
	}
	if ( pWeapon->SilencerAttachable())
	{
		if ( pWeapon->IsSilencerAttached() )
		{
			m_UIPropertiesBox->AddItem( "st_detach_silencer",  NULL, INVENTORY_DETACH_SILENCER_ADDON );
			b_show			= true;
		}
	}
	if (pWeapon->LaserAttachable())
	{
		if (pWeapon->IsLaserAttached())
		{
			m_UIPropertiesBox->AddItem("st_detach_laser", NULL, INVENTORY_DETACH_LASER_ADDON);
			b_show = true;
		}
	}
	if (pWeapon->TacticalTorchAttachable())
	{
		if (pWeapon->IsTacticalTorchAttached())
		{
			m_UIPropertiesBox->AddItem("st_detach_tactical_torch", NULL, INVENTORY_DETACH_TACTICAL_TORCH_ADDON);
			b_show = true;
		}
	}

	if ( smart_cast<CWeaponMagazined*>(pWeapon) && IsGameTypeSingle() )
	{
		bool b = ( pWeapon->GetAmmoElapsed() !=0 );
		if ( !b )
		{
			for ( u32 i = 0; i < cell_item->ChildsCount(); ++i )
			{
				CWeaponMagazined* weap_mag = smart_cast<CWeaponMagazined*>( (CWeapon*)cell_item->Child(i)->m_pData );
				if ( weap_mag && weap_mag->GetAmmoElapsed() )
				{
					b = true;
					break; // for
				}
			}
		}
		if ( b )
		{
			m_UIPropertiesBox->AddItem( "st_unload_magazine",  NULL, INVENTORY_UNLOAD_MAGAZINE );
			b_show = true;
		}
	}
}
#include "../string_table.h"
void CUIActorMenu::PropertiesBoxForAddon( PIItem item, bool& b_show )
{
	//присоединение аддонов к активному слоту (2 или 3)

	CScope*				pScope				= smart_cast<CScope*>			(item);
	CSilencer*			pSilencer			= smart_cast<CSilencer*>		(item);
	CGrenadeLauncher*	pGrenadeLauncher	= smart_cast<CGrenadeLauncher*>	(item);
	CLaserDesignator*	pLaser				= smart_cast<CLaserDesignator*>	(item);
	CTacticalTorch*		pTacticalTorch		= smart_cast<CTacticalTorch*>	(item);
	CInventory*			inv					= &m_pActorInvOwner->inventory();

	if ( pScope )
	{
		if ( inv->m_slots[PISTOL_SLOT].m_pIItem && inv->m_slots[PISTOL_SLOT].m_pIItem->CanAttach(pScope) )
		{
			PIItem tgt = inv->m_slots[PISTOL_SLOT].m_pIItem;
			m_UIPropertiesBox->AddItem( "st_attach_scope_to_pistol",  (void*)tgt, INVENTORY_ATTACH_ADDON );
			b_show			= true;
		}
		if ( inv->m_slots[RIFLE_SLOT].m_pIItem && inv->m_slots[RIFLE_SLOT].m_pIItem->CanAttach(pScope) )
		{
			PIItem tgt = inv->m_slots[RIFLE_SLOT].m_pIItem;
			m_UIPropertiesBox->AddItem( "st_attach_scope_to_rifle",  (void*)tgt, INVENTORY_ATTACH_ADDON );
			b_show			= true;
		}
		return;
	}
	
	if ( pSilencer )
	{
		if ( inv->m_slots[PISTOL_SLOT].m_pIItem && inv->m_slots[PISTOL_SLOT].m_pIItem->CanAttach(pSilencer) )
		{
			PIItem tgt = inv->m_slots[PISTOL_SLOT].m_pIItem;
			m_UIPropertiesBox->AddItem( "st_attach_silencer_to_pistol",  (void*)tgt, INVENTORY_ATTACH_ADDON );
			b_show			= true;
		}
		if ( inv->m_slots[RIFLE_SLOT].m_pIItem && inv->m_slots[RIFLE_SLOT].m_pIItem->CanAttach(pSilencer) )
		{
			PIItem tgt = inv->m_slots[RIFLE_SLOT].m_pIItem;
			m_UIPropertiesBox->AddItem( "st_attach_silencer_to_rifle",  (void*)tgt, INVENTORY_ATTACH_ADDON );
			b_show			= true;
		}
		return;
	}
	
	if ( pGrenadeLauncher )
	{
		if ( inv->m_slots[RIFLE_SLOT].m_pIItem && inv->m_slots[RIFLE_SLOT].m_pIItem->CanAttach(pGrenadeLauncher) )
		{
			PIItem tgt = inv->m_slots[RIFLE_SLOT].m_pIItem;
			m_UIPropertiesBox->AddItem( "st_attach_gl_to_rifle",  (void*)tgt, INVENTORY_ATTACH_ADDON );
			b_show			= true;
		}
	}

	if (pLaser)
	{
		if (inv->m_slots[PISTOL_SLOT].m_pIItem && inv->m_slots[PISTOL_SLOT].m_pIItem->CanAttach(pLaser))
		{
			PIItem tgt = inv->m_slots[PISTOL_SLOT].m_pIItem;
			m_UIPropertiesBox->AddItem("st_attach_laser_to_pistol", (void*)tgt, INVENTORY_ATTACH_ADDON);
			b_show = true;
		}
		if (inv->m_slots[RIFLE_SLOT].m_pIItem && inv->m_slots[RIFLE_SLOT].m_pIItem->CanAttach(pLaser))
		{
			PIItem tgt = inv->m_slots[RIFLE_SLOT].m_pIItem;
			m_UIPropertiesBox->AddItem("st_attach_laser_to_rifle", (void*)tgt, INVENTORY_ATTACH_ADDON);
			b_show = true;
		}
	}

	if (pTacticalTorch)
	{
		if (inv->m_slots[PISTOL_SLOT].m_pIItem && inv->m_slots[PISTOL_SLOT].m_pIItem->CanAttach(pTacticalTorch))
		{
			PIItem tgt = inv->m_slots[PISTOL_SLOT].m_pIItem;
			m_UIPropertiesBox->AddItem("st_attach_tactical_torch_to_pistol", (void*)tgt, INVENTORY_ATTACH_ADDON);
			b_show = true;
		}
		if (inv->m_slots[RIFLE_SLOT].m_pIItem && inv->m_slots[RIFLE_SLOT].m_pIItem->CanAttach(pTacticalTorch))
		{
			PIItem tgt = inv->m_slots[RIFLE_SLOT].m_pIItem;
			m_UIPropertiesBox->AddItem("st_attach_tactical_torch_to_rifle", (void*)tgt, INVENTORY_ATTACH_ADDON);
			b_show = true;
		}
	}
}

void CUIActorMenu::PropertiesBoxForUsing( PIItem item, bool& b_show )
{
	CMedkit*		pMedkit			= smart_cast<CMedkit*>		(item);
	CAntirad*		pAntirad		= smart_cast<CAntirad*>		(item);
	CEatableItem*	pEatableItem	= smart_cast<CEatableItem*>	(item);
	CBottleItem*	pBottleItem		= smart_cast<CBottleItem*>	(item);
	CBattery*		pBattery		= smart_cast<CBattery*>		(item);
	CAntigasFilter* pFilter			= smart_cast<CAntigasFilter*>(item);
	CRepairKit*		pRepairKit		= smart_cast<CRepairKit*>	(item);
	CArtefactContainer* pAfContainer= smart_cast<CArtefactContainer*>(item);
	CArtefact*		pArtefact		= smart_cast<CArtefact*>	(item);
	CSleepingBag*	pSleepingBag	= smart_cast<CSleepingBag*>	(item);

	CInventory*	inv = &m_pActorInvOwner->inventory();
	CTorch* item_in_torch_slot = smart_cast<CTorch*>(inv->ItemFromSlot(TORCH_SLOT));
	CCustomDetector* item_in_art_detector_slot = smart_cast<CCustomDetector*>(inv->ItemFromSlot(DETECTOR_SLOT));
	CDetectorAnomaly* item_in_anomaly_detector_slot = smart_cast<CDetectorAnomaly*>(inv->ItemFromSlot(DOSIMETER_SLOT));
	CCustomOutfit* item_in_outfit_slot = smart_cast<CCustomOutfit*>(inv->ItemFromSlot(OUTFIT_SLOT));
	CHelmet* item_in_helmet_slot = smart_cast<CHelmet*>(inv->ItemFromSlot(HELMET_SLOT));
	CHelmet* item_in_helmet2_slot = smart_cast<CHelmet*>(inv->ItemFromSlot(SECOND_HELMET_SLOT));
	CWeapon* item_in_knife_slot = smart_cast<CWeapon*>(inv->ItemFromSlot(KNIFE_SLOT));
	CWeapon* item_in_wpn1_slot = smart_cast<CWeapon*>(inv->ItemFromSlot(PISTOL_SLOT));
	CWeapon* item_in_wpn2_slot = smart_cast<CWeapon*>(inv->ItemFromSlot(RIFLE_SLOT));

	bool outfit_use_filter = false;
	bool helmet_use_filter = false;
	bool helmet2_use_filter = false;

	bool can_repair_outfit = false;
	bool can_repair_helmet = false;
	bool can_repair_helmet2 = false;
	bool can_repair_knife = false;
	bool can_repair_wpn1 = false;
	bool can_repair_wpn2 = false;

	if (item_in_outfit_slot && pFilter)
		outfit_use_filter = item_in_outfit_slot->m_bUseFilter && item_in_outfit_slot->m_fFilterCondition <= 0.99f && item_in_outfit_slot->IsNecessaryItem(pFilter->cNameSect().c_str(), item_in_outfit_slot->m_SuitableFilters);
	if (item_in_helmet_slot && pFilter)
		helmet_use_filter = item_in_helmet_slot->m_bUseFilter && item_in_helmet_slot->m_fFilterCondition <= 0.99f && item_in_helmet_slot->IsNecessaryItem(pFilter->cNameSect().c_str(), item_in_helmet_slot->m_SuitableFilters);
	if (item_in_helmet2_slot && pFilter)
		helmet2_use_filter = item_in_helmet2_slot->m_bUseFilter && item_in_helmet2_slot->m_fFilterCondition <= 0.99f && item_in_helmet2_slot->IsNecessaryItem(pFilter->cNameSect().c_str(), item_in_helmet2_slot->m_SuitableFilters);

	if (item_in_outfit_slot && pRepairKit)
		can_repair_outfit = item_in_outfit_slot->GetCondition() < 0.9f && item_in_outfit_slot->GetCondition() >= 0.4f && item_in_outfit_slot->IsNecessaryItem(pRepairKit->cNameSect().c_str(), item_in_outfit_slot->m_SuitableRepairKits);
	if (item_in_helmet_slot && pRepairKit)
		can_repair_helmet = item_in_helmet_slot->GetCondition() < 0.9f && item_in_helmet_slot->GetCondition() >= 0.4f && item_in_helmet_slot->IsNecessaryItem(pRepairKit->cNameSect().c_str(), item_in_helmet_slot->m_SuitableRepairKits);
	if (item_in_helmet2_slot && pRepairKit)
		can_repair_helmet2 = item_in_helmet2_slot->GetCondition() < 0.9f && item_in_helmet2_slot->GetCondition() >= 0.4f && item_in_helmet2_slot->IsNecessaryItem(pRepairKit->cNameSect().c_str(), item_in_helmet2_slot->m_SuitableRepairKits);
	if (item_in_knife_slot && pRepairKit)
		can_repair_knife = item_in_knife_slot->GetCondition() < 0.9f && item_in_knife_slot->GetCondition() >= 0.4f && item_in_knife_slot->IsNecessaryItem(pRepairKit->cNameSect().c_str(), item_in_knife_slot->m_SuitableRepairKits);
	if (item_in_wpn1_slot && pRepairKit)
		can_repair_wpn1 = item_in_wpn1_slot->GetCondition() < 0.9f && item_in_wpn1_slot->GetCondition() >= 0.4f && item_in_wpn1_slot->IsNecessaryItem(pRepairKit->cNameSect().c_str(), item_in_wpn1_slot->m_SuitableRepairKits);
	if (item_in_wpn2_slot && pRepairKit)
		can_repair_wpn2 = item_in_wpn2_slot->GetCondition() < 0.9f && item_in_wpn2_slot->GetCondition() >= 0.4f && item_in_wpn2_slot->IsNecessaryItem(pRepairKit->cNameSect().c_str(), item_in_wpn2_slot->m_SuitableRepairKits);

	LPCSTR act_str = NULL;
	CGameObject* GO = smart_cast<CGameObject*>(item);

	if (!item->Useful() || (pFilter && !pFilter->UseAllowed()))
		return;

	LPCSTR use_text = item->GetPropertyBoxUseText().c_str();

	if ( pMedkit || pAntirad )
	{
		act_str = use_text ? use_text : "st_use";
	}
	else if ( pBottleItem )
	{
		act_str = "st_drink";
	}
	else if (pBattery)
	{
		if (item_in_torch_slot && item_in_torch_slot->IsNecessaryItem(pBattery->cNameSect().c_str(), item_in_torch_slot->m_SuitableBatteries) && item_in_torch_slot->GetChargeLevel() <= 0.99f)
		{
			shared_str str = CStringTable().translate(use_text ? use_text : "st_charge_item");
			str.printf("%s %s", str.c_str(), item_in_torch_slot->m_name.c_str());
			m_UIPropertiesBox->AddItem(str.c_str(), (void*)item_in_torch_slot, BATTERY_CHARGE_TORCH);
			b_show = true;
		}

		if (item_in_art_detector_slot && item_in_art_detector_slot->IsNecessaryItem(pBattery->cNameSect().c_str(), item_in_art_detector_slot->m_SuitableBatteries) && item_in_art_detector_slot->GetChargeLevel() <= 0.99f)
		{
			shared_str str = CStringTable().translate(use_text ? use_text : "st_charge_item");
			str.printf("%s %s", str.c_str(), item_in_art_detector_slot->m_name.c_str());
			m_UIPropertiesBox->AddItem(str.c_str(), (void*)item_in_art_detector_slot, BATTERY_CHARGE_DETECTOR);
			b_show = true;
		}

		if (item_in_anomaly_detector_slot && item_in_anomaly_detector_slot->IsNecessaryItem(pBattery->cNameSect().c_str(), item_in_anomaly_detector_slot->m_SuitableBatteries) && item_in_anomaly_detector_slot->GetChargeLevel() <= 0.99f)
		{
			shared_str str = CStringTable().translate(use_text ? use_text : "st_charge_item");
			str.printf("%s %s", str.c_str(), item_in_anomaly_detector_slot->m_name.c_str());
			m_UIPropertiesBox->AddItem(str.c_str(), (void*)item_in_anomaly_detector_slot, BATTERY_CHARGE_DOSIMETER);
			b_show = true;
		}
		return;
	}
	else if (pFilter)
	{
		if (item_in_outfit_slot && outfit_use_filter)
		{
			shared_str str = CStringTable().translate(use_text ? use_text : "st_change_filter");
			str.printf("%s %s", str.c_str(), item_in_outfit_slot->m_name.c_str());
			m_UIPropertiesBox->AddItem(str.c_str(), (void*)item_in_outfit_slot, FILTER_CHANGE_OUTFIT);
			b_show = true;
		}

		if (item_in_helmet_slot && helmet_use_filter)
		{
			shared_str str = CStringTable().translate(use_text ? use_text : "st_change_filter");
			str.printf("%s %s", str.c_str(), item_in_helmet_slot->m_name.c_str());
			m_UIPropertiesBox->AddItem(str.c_str(), (void*)item_in_helmet_slot, FILTER_CHANGE_HELMET);
			b_show = true;
		}

		if (item_in_helmet2_slot && helmet2_use_filter)
		{
			shared_str str = CStringTable().translate(use_text ? use_text : "st_change_filter");
			str.printf("%s %s", str.c_str(), item_in_helmet2_slot->m_name.c_str());
			m_UIPropertiesBox->AddItem(str.c_str(), (void*)item_in_helmet2_slot, FILTER_CHANGE_HELMET);
			b_show = true;
		}

		return;
	}
	else if (pRepairKit)
	{
		if (item_in_outfit_slot && can_repair_outfit)
		{
			shared_str str = CStringTable().translate(use_text ? use_text : "st_repair");
			str.printf("%s %s", str.c_str(), item_in_outfit_slot->m_name.c_str());

			shared_str repair_hint{};

			if (!item_in_outfit_slot->m_ItemsForRepair.empty())
			{
				repair_hint = CStringTable().translate("st_materials_for_repair");

				for (int i = 0; i < item_in_outfit_slot->m_ItemsForRepair.size(); i++)
				{
					repair_hint.printf("%s\\n%s: x%s;\\n", repair_hint.c_str(), CStringTable().translate(item_in_outfit_slot->m_ItemsForRepairNames[i]).c_str(), std::to_string(item_in_outfit_slot->m_ItemsForRepair[i].second));
				}
			}

			m_UIPropertiesBox->AddItem(str.c_str(), (void*)item_in_outfit_slot, REPAIR_KIT_OUTFIT, repair_hint.c_str());
			b_show = true;
		}

		if (item_in_helmet_slot && can_repair_helmet)
		{
			shared_str str = CStringTable().translate(use_text ? use_text : "st_repair");
			str.printf("%s %s", str.c_str(), item_in_helmet_slot->m_name.c_str());

			shared_str repair_hint{};

			if (!item_in_helmet_slot->m_ItemsForRepair.empty())
			{
				repair_hint = CStringTable().translate("st_materials_for_repair");

				for (int i = 0; i < item_in_helmet_slot->m_ItemsForRepair.size(); i++)
				{
					repair_hint.printf("%s\\n%s: x%s;\\n", repair_hint.c_str(), CStringTable().translate(item_in_helmet_slot->m_ItemsForRepairNames[i]).c_str(), std::to_string(item_in_helmet_slot->m_ItemsForRepair[i].second));
				}
			}

			m_UIPropertiesBox->AddItem(str.c_str(), (void*)item_in_helmet_slot, REPAIR_KIT_HELMET, repair_hint.c_str());
			b_show = true;
		}

		if (item_in_helmet2_slot && can_repair_helmet2)
		{
			shared_str str = CStringTable().translate(use_text ? use_text : "st_repair");
			str.printf("%s %s", str.c_str(), item_in_helmet2_slot->m_name.c_str());

			shared_str repair_hint{};

			if (!item_in_helmet2_slot->m_ItemsForRepair.empty())
			{
				repair_hint = CStringTable().translate("st_materials_for_repair");

				for (int i = 0; i < item_in_helmet2_slot->m_ItemsForRepair.size(); i++)
				{
					repair_hint.printf("%s\\n%s: x%s;\\n", repair_hint.c_str(), CStringTable().translate(item_in_helmet2_slot->m_ItemsForRepairNames[i]).c_str(), std::to_string(item_in_helmet2_slot->m_ItemsForRepair[i].second));
				}
			}

			m_UIPropertiesBox->AddItem(str.c_str(), (void*)item_in_helmet2_slot, REPAIR_KIT_SECOND_HELMET, repair_hint.c_str());
			b_show = true;
		}

		if (item_in_knife_slot && can_repair_knife)
		{
			shared_str str = CStringTable().translate(use_text ? use_text : "st_repair");
			str.printf("%s %s", str.c_str(), item_in_knife_slot->m_name.c_str());
			
			shared_str repair_hint{};

			if (!item_in_knife_slot->m_ItemsForRepair.empty())
			{
				repair_hint = CStringTable().translate("st_materials_for_repair");

				for (int i = 0; i < item_in_knife_slot->m_ItemsForRepair.size(); i++)
				{
					repair_hint.printf("%s\\n%s: x%s;\\n", repair_hint.c_str(), CStringTable().translate(item_in_knife_slot->m_ItemsForRepairNames[i]).c_str(), std::to_string(item_in_knife_slot->m_ItemsForRepair[i].second));
				}
			}

			m_UIPropertiesBox->AddItem(str.c_str(), (void*)item_in_knife_slot, REPAIR_KIT_KNIFE, repair_hint.c_str());
			b_show = true;
		}

		if (item_in_wpn1_slot && can_repair_wpn1)
		{
			shared_str str = CStringTable().translate(use_text ? use_text : "st_repair");
			str.printf("%s %s", str.c_str(), item_in_wpn1_slot->m_name.c_str());
			
			shared_str repair_hint{};

			if (!item_in_wpn1_slot->m_ItemsForRepair.empty())
			{
				repair_hint = CStringTable().translate("st_materials_for_repair");

				for (int i = 0; i < item_in_wpn1_slot->m_ItemsForRepair.size(); i++)
				{
					repair_hint.printf("%s\\n%s: x%s;\\n", repair_hint.c_str(), CStringTable().translate(item_in_wpn1_slot->m_ItemsForRepairNames[i]).c_str(), std::to_string(item_in_wpn1_slot->m_ItemsForRepair[i].second));
				}
			}

			m_UIPropertiesBox->AddItem(str.c_str(), (void*)item_in_wpn1_slot, REPAIR_KIT_WPN1, repair_hint.c_str());

			b_show = true;
		}

		if (item_in_wpn2_slot && can_repair_wpn2)
		{
			shared_str str = CStringTable().translate(use_text ? use_text : "st_repair");
			str.printf("%s %s", str.c_str(), item_in_wpn2_slot->m_name.c_str());
			
			shared_str repair_hint{};

			if (!item_in_wpn2_slot->m_ItemsForRepair.empty())
			{
				repair_hint = CStringTable().translate("st_materials_for_repair");

				for (int i = 0; i < item_in_wpn2_slot->m_ItemsForRepair.size(); i++)
				{
					repair_hint.printf("%s\\n%s: x%s;\\n", repair_hint.c_str(), CStringTable().translate(item_in_wpn2_slot->m_ItemsForRepairNames[i]).c_str(), std::to_string(item_in_wpn2_slot->m_ItemsForRepair[i].second));
				}
			}

			m_UIPropertiesBox->AddItem(str.c_str(), (void*)item_in_wpn2_slot, REPAIR_KIT_WPN2, repair_hint.c_str());
			b_show = true;
		}
		return;
	}
	else if (pArtefact)
	{
		TIItemContainer::iterator it = inv->m_ruck.begin();
		TIItemContainer::iterator ite = inv->m_ruck.end();

		for (it; ite != it; ++it)
		{
			CArtefactContainer* container = smart_cast<CArtefactContainer*>(*it);

			if (container && !container->IsFull())
			{
				shared_str str = CStringTable().translate(use_text ? use_text : "st_put_to");
				str.printf("%s %s", str.c_str(), container->m_name.c_str());
				m_UIPropertiesBox->AddItem(str.c_str(), (void*)container, ARTEFACT_TO_CONTAINER);
				b_show = true;
			}
		}
	}
	else if (pAfContainer && pAfContainer->GetArtefactsInside().size())
	{
		for (auto af_in_container : pAfContainer->GetArtefactsInside())
		{
			CArtefact* af_in_container_casted = smart_cast<CArtefact*>(af_in_container);

			if (af_in_container_casted)
			{
				shared_str str = CStringTable().translate(use_text ? use_text : "st_take_from");
				str.printf("%s %s", str.c_str(), af_in_container_casted->m_name.c_str());
				m_UIPropertiesBox->AddItem(str.c_str(), (void*)af_in_container_casted, ARTEFACT_FROM_CONTAINER);
				b_show = true;
			}
		}
	}
	else if (pSleepingBag)
	{
		m_UIPropertiesBox->AddItem(use_text ? use_text : "st_use", NULL, INVENTORY_SLEEP_ACTION);
		b_show = true;
	}
	else if ( pEatableItem )
	{
		if ( pBottleItem )
		{
			act_str = use_text ? use_text : "st_drink";
		}
		else
		{
			act_str = use_text ? use_text : "st_eat";
		}
	}
	if ( act_str )
	{
		m_UIPropertiesBox->AddItem( act_str,  NULL, INVENTORY_EAT_ACTION );
		b_show			= true;
	}
	
	auto CustomEatAction = [&](const char* use_action_name, enum EUIMessages message)
		{
			LPCSTR functor_name = READ_IF_EXISTS(pSettings, r_string, GO->cNameSect(), use_action_name, 0);
			if (functor_name)
			{
				luabind::functor<LPCSTR> funct1;
				if (ai().script_engine().functor(functor_name, funct1))
				{
					act_str = funct1(GO->lua_game_object());
					if (act_str)
					{
						m_UIPropertiesBox->AddItem(act_str, NULL, message);
						b_show = true;
					}
				}
			}
		};

	//Custom Use actions
	CustomEatAction("use1_functor", INVENTORY_EAT2_ACTION);
	CustomEatAction("use2_functor", INVENTORY_EAT3_ACTION);
	CustomEatAction("use3_functor", INVENTORY_EAT4_ACTION);
	CustomEatAction("use4_functor", INVENTORY_EAT5_ACTION);
	CustomEatAction("use5_functor", INVENTORY_EAT6_ACTION);
	CustomEatAction("use6_functor", INVENTORY_EAT7_ACTION);
	CustomEatAction("use7_functor", INVENTORY_EAT8_ACTION);
	CustomEatAction("use8_functor", INVENTORY_EAT9_ACTION);
	CustomEatAction("use9_functor", INVENTORY_EAT10_ACTION);
	CustomEatAction("use10_functor", INVENTORY_EAT11_ACTION);
}

void CUIActorMenu::PropertiesBoxForPlaying(PIItem item, bool& b_show)
{
	CPda* pPda = smart_cast<CPda*>(item);
	if(!pPda || !pPda->CanPlayScriptFunction())
		return;

	LPCSTR act_str = "st_play";
	m_UIPropertiesBox->AddItem(act_str,  NULL, INVENTORY_PLAY_ACTION);
	b_show = true;
}

void CUIActorMenu::PropertiesBoxForDrop( CUICellItem* cell_item, PIItem item, bool& b_show )
{
	if ( !item->IsQuestItem() )
	{
		m_UIPropertiesBox->AddItem( "st_drop", NULL, INVENTORY_DROP_ACTION );
		b_show = true;

		if ( cell_item->ChildsCount() )
		{
			m_UIPropertiesBox->AddItem( "st_drop_all", (void*)33, INVENTORY_DROP_ACTION );
		}
	}
}

void CUIActorMenu::PropertiesBoxForRepair( PIItem item, bool& b_show )
{
	CCustomOutfit* pOutfit = smart_cast<CCustomOutfit*>( item );
	CWeapon*       pWeapon = smart_cast<CWeapon*>( item );
	CHelmet*       pHelmet = smart_cast<CHelmet*>( item );

	if ( (pOutfit || pWeapon || pHelmet) && item->GetCondition() < 0.99f )
	{
		m_UIPropertiesBox->AddItem( "ui_inv_repair", NULL, INVENTORY_REPAIR );
		b_show = true;
	}
}

void CUIActorMenu::ProcessPropertiesBoxClicked( CUIWindow* w, void* d )
{
	PIItem			item		= CurrentIItem();
	CUICellItem*	cell_item	= CurrentItem();
	if ( !m_UIPropertiesBox->GetClickedItem() || !item || !cell_item || !cell_item->OwnerList() )
	{
		return;
	}
	CWeapon* weapon = smart_cast<CWeapon*>( item );

	auto CustomEatAction = [&](const char* use_action_name)
		{
			CGameObject* GO = smart_cast<CGameObject*>(item);
			LPCSTR functor_name = READ_IF_EXISTS(pSettings, r_string, GO->cNameSect(), use_action_name, 0);
			if (functor_name)
			{
				luabind::functor<bool> funct1;
				if (ai().script_engine().functor(functor_name, funct1))
				{
					if (funct1(GO->lua_game_object()))
					{
						HUD().GetUI()->UIGame()->ActorMenu().SetCurrentConsumable(cell_item);
						TryUseItem(cell_item);
					}
				}
			}
		};

	switch ( m_UIPropertiesBox->GetClickedItem()->GetTAG() )
	{
	case INVENTORY_TO_SLOT_ACTION:	ToSlot( cell_item, true  );		break;
	case INVENTORY_TO_BELT_ACTION:	ToBelt( cell_item, false );		break;
	case INVENTORY_TO_BAG_ACTION:	ToBag ( cell_item, false );		break;
	case INVENTORY_EAT_ACTION:
		{
			HUD().GetUI()->UIGame()->ActorMenu().SetCurrentConsumable(cell_item);
			TryUseItem(cell_item);
			break;
		}
	case INVENTORY_EAT2_ACTION:
		{ CustomEatAction("use1_action_functor"); break; }
	case INVENTORY_EAT3_ACTION:
		{ CustomEatAction("use2_action_functor"); break; }
	case INVENTORY_EAT4_ACTION:
		{ CustomEatAction("use3_action_functor"); break; }
	case INVENTORY_EAT5_ACTION:
		{ CustomEatAction("use4_action_functor"); break; }
	case INVENTORY_EAT6_ACTION:
		{ CustomEatAction("use5_action_functor"); break; }
	case INVENTORY_EAT7_ACTION:
		{ CustomEatAction("use6_action_functor"); break; }
	case INVENTORY_EAT8_ACTION:
		{ CustomEatAction("use7_action_functor"); break; }
	case INVENTORY_EAT9_ACTION:
		{ CustomEatAction("use8_action_functor"); break; }
	case INVENTORY_EAT10_ACTION:
		{ CustomEatAction("use9_action_functor"); break; }
	case INVENTORY_EAT11_ACTION:
		{ CustomEatAction("use10_action_functor"); break; }
	case INVENTORY_DROP_ACTION:
		{
			void* dd = m_UIPropertiesBox->GetClickedItem()->GetData();
			if ( dd == (void*)33 )
			{
				DropAllCurrentItem();
			}
			else
			{
				SendEvent_Item_Drop( item, m_pActorInvOwner->object_id() );
			}
			break;
		}
	case INVENTORY_ATTACH_ADDON:
		{
			PIItem item = CurrentIItem(); // temporary storing because of AttachAddon is setting curiitem to NULL
			AttachAddon((PIItem)(m_UIPropertiesBox->GetClickedItem()->GetData()));
			if(m_currMenuMode==mmDeadBodySearch)
				RemoveItemFromList(m_pDeadBodyBagList, item);
			
			break;
		}
	case INVENTORY_DETACH_SCOPE_ADDON:
		if ( weapon )
		{
			DetachAddon( weapon->GetScopeName().c_str() );
			for ( u32 i = 0; i < cell_item->ChildsCount(); ++i )
			{
				CUICellItem*	child_itm	= cell_item->Child(i);
				PIItem			child_iitm	= (PIItem)(child_itm->m_pData);
				CWeapon* wpn = smart_cast<CWeapon*>( child_iitm );
				if ( child_iitm && wpn )
				{
					DetachAddon(wpn->GetScopeName().c_str(), child_iitm);
				}
			}
		}
		break;
	case INVENTORY_DETACH_SILENCER_ADDON:
		if ( weapon )
		{
			DetachAddon( weapon->GetSilencerName().c_str() );
			for ( u32 i = 0; i < cell_item->ChildsCount(); ++i )
			{
				CUICellItem*	child_itm	= cell_item->Child(i);
				PIItem			child_iitm	= (PIItem)(child_itm->m_pData);
				CWeapon* wpn = smart_cast<CWeapon*>( child_iitm );
				if ( child_iitm && wpn )
				{
					DetachAddon(wpn->GetSilencerName().c_str(), child_iitm);
				}
			}
		}
		break;
	case INVENTORY_DETACH_GRENADE_LAUNCHER_ADDON:
		if ( weapon )
		{
			DetachAddon( weapon->GetGrenadeLauncherName().c_str() );
			for ( u32 i = 0; i < cell_item->ChildsCount(); ++i )
			{
				CUICellItem*	child_itm	= cell_item->Child(i);
				PIItem			child_iitm	= (PIItem)(child_itm->m_pData);
				CWeapon* wpn = smart_cast<CWeapon*>( child_iitm );
				if ( child_iitm && wpn )
				{
					DetachAddon(wpn->GetGrenadeLauncherName().c_str(), child_iitm);
				}
			}
		}
		break;
	case INVENTORY_DETACH_LASER_ADDON:
		if (weapon)
		{
			DetachAddon(weapon->GetLaserName().c_str());
		}
		break;
	case INVENTORY_DETACH_TACTICAL_TORCH_ADDON:
		if (weapon)
		{
			DetachAddon(weapon->GetTacticalTorchName().c_str());
		}
		break;
	case INVENTORY_RELOAD_MAGAZINE:
		if ( weapon )
		{
			weapon->Action( kWPN_RELOAD, CMD_START );
		}
		break;
	case INVENTORY_UNLOAD_MAGAZINE:
		{
			CWeaponMagazined* weap_mag = smart_cast<CWeaponMagazined*>( (CWeapon*)cell_item->m_pData );
			if ( !weap_mag )
			{
				break;
			}
			weap_mag->UnloadMagazine();
			for ( u32 i = 0; i < cell_item->ChildsCount(); ++i )
			{
				CUICellItem*		child_itm		= cell_item->Child(i);
				CWeaponMagazined*	child_weap_mag	= smart_cast<CWeaponMagazined*>( (CWeapon*)child_itm->m_pData );
				if ( child_weap_mag )
				{
					child_weap_mag->UnloadMagazine();
				}
			}
			break;
		}
	case INVENTORY_REPAIR:
		{
			TryRepairItem(this,0);
			return;
			break;
		}
	case INVENTORY_PLAY_ACTION:
		{
			CPda* pPda = smart_cast<CPda*>(item);
			if(!pPda)
				break;
			pPda->PlayScriptFunction();
			break;
		}
	case BATTERY_CHARGE_TORCH:
		{
			CBattery* battery = smart_cast<CBattery*>(item);
			if (!battery)
				break;
			battery->m_iUseFor = 1;
			TryUseItem(cell_item);
			break;
		}
	case BATTERY_CHARGE_DETECTOR:
		{
			CBattery* battery = smart_cast<CBattery*>(item);
			if (!battery)
				break;
			battery->m_iUseFor = 2;
			TryUseItem(cell_item);
			break;
		}
	case BATTERY_CHARGE_DOSIMETER:
		{
			CBattery* battery = smart_cast<CBattery*>(item);
			if (!battery)
				break;
			battery->m_iUseFor = 3;
			TryUseItem(cell_item);
			break;
		}
	case FILTER_CHANGE_OUTFIT:
		{
			CAntigasFilter* filter = smart_cast<CAntigasFilter*>(item);
			if (!filter)
				break;
			filter->m_iUseFor = 1;
			TryUseItem(cell_item);
			break;
		}
	case FILTER_CHANGE_HELMET:
		{
			CAntigasFilter* filter = smart_cast<CAntigasFilter*>(item);
			if (!filter)
				break;
			filter->m_iUseFor = 2;
			TryUseItem(cell_item);
			break;
		}
	case FILTER_CHANGE_SECOND_HELMET:
		{
			CAntigasFilter* filter = smart_cast<CAntigasFilter*>(item);
			if (!filter)
				break;
			filter->m_iUseFor = 3;
			TryUseItem(cell_item);
			break;
		}
	case REPAIR_KIT_OUTFIT:
		{
			CRepairKit* repair_kit = smart_cast<CRepairKit*>(item);
			if (!repair_kit)
				break;
			repair_kit->m_iUseFor = 1;
			TryUseItem(cell_item);
			break;
		}
	case REPAIR_KIT_HELMET:
		{
		CRepairKit* repair_kit = smart_cast<CRepairKit*>(item);
			if (!repair_kit)
				break;
			repair_kit->m_iUseFor = 2;
			TryUseItem(cell_item);
			break;
		}
	case REPAIR_KIT_SECOND_HELMET:
		{
			CRepairKit* repair_kit = smart_cast<CRepairKit*>(item);
			if (!repair_kit)
				break;
			repair_kit->m_iUseFor = 3;
			TryUseItem(cell_item);
			break;
		}
	case REPAIR_KIT_KNIFE:
		{
			CRepairKit* repair_kit = smart_cast<CRepairKit*>(item);
			if (!repair_kit)
				break;
			repair_kit->m_iUseFor = 4;
			TryUseItem(cell_item);
			break;
		}
	case REPAIR_KIT_WPN1:
		{
			CRepairKit* repair_kit = smart_cast<CRepairKit*>(item);
			if (!repair_kit)
				break;
			repair_kit->m_iUseFor = 5;
			TryUseItem(cell_item);
			break;
		}
	case REPAIR_KIT_WPN2:
		{
			CRepairKit* repair_kit = smart_cast<CRepairKit*>(item);
			if (!repair_kit)
				break;
			repair_kit->m_iUseFor = 6;
			TryUseItem(cell_item);
			break;
		}
	case ARTEFACT_TO_CONTAINER:
		{
			CArtefact* artefact = smart_cast<CArtefact*>(item);
			CArtefactContainer* af_container = smart_cast<CArtefactContainer*>((PIItem)m_UIPropertiesBox->GetClickedItem()->GetData());

			if (!artefact || !af_container)
				break;

			af_container->PutArtefactToContainer(*artefact);

			if (m_currMenuMode == mmDeadBodySearch)
				RemoveItemFromList(m_pDeadBodyBagList, item);

			artefact->DestroyObject();

			break;
		}
	case ARTEFACT_FROM_CONTAINER:
		{
			CArtefactContainer* af_container = smart_cast<CArtefactContainer*>(item);
			CArtefact* artefact = smart_cast<CArtefact*>((PIItem)m_UIPropertiesBox->GetClickedItem()->GetData());

			if (!af_container)
				break;

			af_container->TakeArtefactFromContainer(artefact);

			m_ItemInfo->ResetInventoryItem();

			break;
		}
	case INVENTORY_SLEEP_ACTION:
		{
			CSleepingBag* sleeping_bag = smart_cast<CSleepingBag*>(item);

			if (!sleeping_bag)
				break;

			sleeping_bag->StartSleep();

			break;
		}
	}//switch

	SetCurrentItem( NULL );
	UpdateItemsPlace();
}//ProcessPropertiesBoxClicked

void CUIActorMenu::UpdateOutfit()
{
	if (m_bBeltSlotsOverInitialized)
	{
		for (u8 i = 0; i < GameConstants::GetArtefactsCount(); ++i)
		{
			m_belt_list_over[i]->SetVisible(true);
		}
	}

	u32 af_count = m_pActorInvOwner->inventory().BeltWidth();
	VERIFY( 0 <= af_count && af_count <= GameConstants::GetArtefactsCount());

	VERIFY( m_pInventoryBeltList );
	PIItem         ii_outfit = m_pActorInvOwner->inventory().m_slots[OUTFIT_SLOT].m_pIItem;
	CCustomOutfit* outfit    = smart_cast<CCustomOutfit*>( ii_outfit );

	if (outfit && !outfit->bIsHelmetAvaliable)
		m_HelmetOver->Show(true);
	else
		m_HelmetOver->Show(false);

	if (outfit && !outfit->bIsSecondHelmetAvaliable)
		m_SecondHelmetOver->Show(true);
	else
		m_SecondHelmetOver->Show(false);

	CHelmet* pHelmet1 = smart_cast<CHelmet*>(m_pActorInvOwner->inventory().ItemFromSlot(HELMET_SLOT));
	CHelmet* pHelmet2 = smart_cast<CHelmet*>(m_pActorInvOwner->inventory().ItemFromSlot(SECOND_HELMET_SLOT));

	if (pHelmet1 && !pHelmet1->m_bSecondHelmetEnabled)
	{
		if (!pHelmet2)
			m_SecondHelmetOver->Show(true);
	}
	else if (pHelmet2 && !pHelmet2->m_bSecondHelmetEnabled)
	{
		if (!pHelmet1)
			m_HelmetOver->Show(true);
	}


	if (outfit && !m_bNeedMoveAfsToBag)
		m_bNeedMoveAfsToBag = true;

	if ( !ii_outfit || !outfit )
	{
		if (m_bNeedMoveAfsToBag)
		{
			MoveArtefactsToBag();
			m_bNeedMoveAfsToBag = false;
		}

		if (!af_count)
			return;
	}

	Ivector2 afc;
	afc.x = m_pInventoryBeltList->CellsCapacity().x;
	afc.y = m_pInventoryBeltList->CellsCapacity().y;

	m_pInventoryBeltList->SetCellsCapacity( afc );

	if (m_bBeltSlotsOverInitialized)
	{
		for (u8 i = 0; i < af_count; ++i)
		{
			m_belt_list_over[i]->SetVisible(false);
		}
	}
}

void CUIActorMenu::MoveArtefactsToBag()
{
	while ( m_pInventoryBeltList->ItemsCount() )
	{
		CUICellItem* ci = m_pInventoryBeltList->GetItemIdx(0);
		VERIFY( ci && ci->m_pData );
		ToBag( ci, false );
	}//for i
	m_pInventoryBeltList->ClearAll( true );
}

void CUIActorMenu::RefreshCurrentItemCell()
{
	CUICellItem* ci = CurrentItem();
	if (!ci)
		return;

	if (ci->ChildsCount() > 0)
	{
		CUIDragDropListEx* invlist = GetListByType(iActorBag);

		if (invlist->IsOwner(ci))
		{
			CUICellItem* parent = invlist->RemoveItem(ci, true);

			while (parent->ChildsCount())
			{
				CUICellItem* child = parent->PopChild(nullptr);
				invlist->SetItem(child);
			}

			invlist->SetItem(parent, GetUICursor().GetCursorPosition());
		}
	}
}

void CUIActorMenu::RefreshConsumableCells()
{
	CUICellItem* ci = GetCurrentConsumable();
	if (ci)
	{
		CEatableItem* eitm = smart_cast<CEatableItem*>((CEatableItem*)ci->m_pData);
		if (eitm)
		{
			//Fvector2 cp = GetUICursor().GetCursorPosition(); // XXX: This is unused
			CUIDragDropListEx* invlist = GetListByType(iActorBag);

			CUICellItem* parent = invlist->RemoveItem(ci, true);
			const u32 c = parent->ChildsCount();
			if (c > 0)
			{
				while (parent->ChildsCount())
				{
					CUICellItem* child = parent->PopChild(nullptr);
					invlist->SetItem(child);
				}

				invlist->SetItem(parent);
			}
			else
				invlist->SetItem(parent);
		}
		SetCurrentConsumable(nullptr);
	}
}