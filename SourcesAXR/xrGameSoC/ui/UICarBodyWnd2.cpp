#include "StdAfx.h"
#include "UICarBodyWnd.h"

#include "UICellItem.h"
#include "UIDragDropListEx.h"
#include "UIListBoxItem.h"
#include "UIPropertiesBox.h"

#include "antirad.h"
#include "BottleItem.h"
#include "CustomOutfit.h"
#include "eatable_item.h"
#include "Inventory.h"
#include "InventoryBox.h"
#include "InventoryOwner.h"
#include "Level.h"
#include "medkit.h"
#include "Weapon.h"
#include "WeaponMagazined.h"
#include "WeaponMagazined.h"
#include "Car.h"
#include "../Battery.h"
#include "../AntigasFilter.h"
#include "../RepairKit.h"
#include "../Torch.h"
#include "../AnomalyDetector.h"
#include "../PDA.h"

#include "../string_table.h"

#include "UICursor.h"

void CUICarBodyWnd::ActivatePropertiesBox()
{
	if (m_pUIPropertiesBox->IsShown())
	{
		m_pUIPropertiesBox->Hide();
	}

	PIItem item = CurrentIItem();
	if (!item)
	{
		return;
	}

	CUICellItem* cell_item = CurrentItem();
	m_pUIPropertiesBox->RemoveAll();
	bool b_show = false;

	PropertiesBoxForWeapon(cell_item, item, b_show);
	PropertiesBoxForUsing(item, b_show);
	PropertiesBoxForDrop(cell_item, item, b_show);

	if (b_show)
	{
		m_pUIPropertiesBox->AutoUpdateSize();
		m_pUIPropertiesBox->BringAllToTop();

		Fvector2						cursor_pos;
		Frect							vis_rect;
		GetAbsoluteRect(vis_rect);
		cursor_pos = GetUICursor().GetCursorPosition();
		cursor_pos.sub(vis_rect.lt);
		m_pUIPropertiesBox->Show(vis_rect, cursor_pos);
	//	PlaySnd(eInvProperties);
	}
}

void CUICarBodyWnd::ProcessPropertiesBoxClicked(CUIWindow* w, void* d)
{
	PIItem			item = CurrentIItem();
	CUICellItem* cell_item = CurrentItem();
	if (!m_pUIPropertiesBox->GetClickedItem() || !item || !cell_item || !cell_item->OwnerList())
	{
		return;
	}
	CWeapon* weapon = smart_cast<CWeapon*>(item);

	switch (m_pUIPropertiesBox->GetClickedItem()->GetTAG())
	{
	case INVENTORY_EAT_ACTION:
		{
			//	HUD().GetUI()->UIGame()->ActorMenu().SetCurrentConsumable(cell_item);
			EatItem(cell_item);
			SetCurrentItem(NULL);
			break;
		}
	case INVENTORY_DROP_ACTION:
		{
			void* d = m_pUIPropertiesBox->GetClickedItem()->GetData();
			bool b_all = (d == (void*)33);

			DropCurrentItem(b_all);
			break;
		}
	case INVENTORY_ALL_TO_BAG_ACTION:
		{
	#pragma todo("find out why it crashes")
			/*CUIDragDropListEx* old_owner = cell_item->OwnerList();
			CUIDragDropListEx* new_owner = (old_owner == m_pUIOthersBagList) ? m_pUIOurBagList : m_pUIOthersBagList;
			if (m_pOthersObject)
			{
				u32 cnt = CurrentItem()->ChildsCount();
				for (u32 i = 0; i < cnt; ++i)
				{
					CUICellItem* itm = CurrentItem()->PopChild();
					PIItem iitm = (PIItem)itm->m_pData;
					if (TransferItem(iitm, (old_owner == m_pUIOthersBagList) ? m_pOthersObject : m_pOurObject, (old_owner == m_pUIOurBagList) ? m_pOthersObject : m_pOurObject, (old_owner == m_pUIOurBagList)))
					{
						CUICellItem* ci = old_owner->RemoveItem(itm, true);
						new_owner->SetItem(ci);
					}
				}
			}
			else
			{
				u32 cnt = CurrentItem()->ChildsCount();
				bool bMoveDirection = (old_owner == m_pUIOthersBagList);

				for (u32 i = 0; i < cnt; ++i)
				{
					CUICellItem* itm = CurrentItem()->PopChild();
					PIItem iitm = (PIItem)itm->m_pData;
					u16 tmp_id = (smart_cast<CGameObject*>(m_pOurObject))->ID();
					move_item(bMoveDirection ? m_pInventoryBox->ID() : tmp_id, bMoveDirection ? tmp_id : m_pInventoryBox->ID(), iitm->object().ID());
				}

			}
			SetCurrentItem(NULL);*/
			break;
		}
	case INVENTORY_TO_BAG_ACTION:
		{
			CUIDragDropListEx* old_owner = cell_item->OwnerList();
			CUIDragDropListEx* new_owner = (old_owner == m_pUIOthersBagList) ? m_pUIOurBagList : m_pUIOthersBagList;

			if (m_pOthersObject)
			{
#pragma todo("ТЧ: Тут надо завозить deadbody_can_take_status()")
				//if (!m_pOthersObject->deadbody_can_take_status())
				//	return;

				PIItem quest_item = (PIItem)CurrentItem()->m_pData;
				if (quest_item->IsQuestItem())
					return;

				if (TransferItem(CurrentIItem(), (old_owner == m_pUIOthersBagList) ? m_pOthersObject : m_pOurObject, (old_owner == m_pUIOurBagList) ? m_pOthersObject : m_pOurObject, (old_owner == m_pUIOurBagList)))
				{
					CUICellItem* ci = old_owner->RemoveItem(CurrentItem(), false);
					new_owner->SetItem(ci);
				}
			}
			else
			{
				bool bMoveDirection = (old_owner == m_pUIOthersBagList);

				u16 tmp_id = (smart_cast<CGameObject*>(m_pOurObject))->ID();

				if (m_pInventoryBox)
					move_item(bMoveDirection ? m_pInventoryBox->ID() : tmp_id, bMoveDirection ? tmp_id : m_pInventoryBox->ID(), CurrentIItem()->object().ID());
				else
					move_item(bMoveDirection ? m_pCar->ID() : tmp_id, bMoveDirection ? tmp_id : m_pCar->ID(), CurrentIItem()->object().ID());
			}
			SetCurrentItem(NULL);
			break;
		}
	case INVENTORY_DETACH_SCOPE_ADDON:
		if (weapon)
		{
			DetachAddon(weapon->GetScopeName().c_str());
			for (u32 i = 0; i < cell_item->ChildsCount(); ++i)
			{
				CUICellItem* child_itm = cell_item->Child(i);
				PIItem			child_iitm = (PIItem)(child_itm->m_pData);
				CWeapon* wpn = smart_cast<CWeapon*>(child_iitm);
				if (child_iitm && wpn)
				{
					DetachAddon(wpn->GetScopeName().c_str());
				}
			}
		}
		break;
	case INVENTORY_DETACH_SILENCER_ADDON:
		if (weapon)
		{
			DetachAddon(weapon->GetSilencerName().c_str());
			for (u32 i = 0; i < cell_item->ChildsCount(); ++i)
			{
				CUICellItem* child_itm = cell_item->Child(i);
				PIItem			child_iitm = (PIItem)(child_itm->m_pData);
				CWeapon* wpn = smart_cast<CWeapon*>(child_iitm);
				if (child_iitm && wpn)
				{
					DetachAddon(wpn->GetSilencerName().c_str());
				}
			}
		}
		break;
	case INVENTORY_DETACH_GRENADE_LAUNCHER_ADDON:
		if (weapon)
		{
			DetachAddon(weapon->GetGrenadeLauncherName().c_str());
			for (u32 i = 0; i < cell_item->ChildsCount(); ++i)
			{
				CUICellItem* child_itm = cell_item->Child(i);
				PIItem			child_iitm = (PIItem)(child_itm->m_pData);
				CWeapon* wpn = smart_cast<CWeapon*>(child_iitm);
				if (child_iitm && wpn)
				{
					DetachAddon(wpn->GetGrenadeLauncherName().c_str());
				}
			}
		}
		break;
	case INVENTORY_RELOAD_MAGAZINE:
		if (weapon)
		{
			weapon->Action(kWPN_RELOAD, CMD_START);
		}
		break;
	case INVENTORY_UNLOAD_MAGAZINE:
		{
			CWeaponMagazined* weap_mag = smart_cast<CWeaponMagazined*>((CWeapon*)cell_item->m_pData);
			if (!weap_mag)
			{
				break;
			}
			weap_mag->UnloadMagazine();
			for (u32 i = 0; i < cell_item->ChildsCount(); ++i)
			{
				CUICellItem* child_itm = cell_item->Child(i);
				CWeaponMagazined* child_weap_mag = smart_cast<CWeaponMagazined*>((CWeapon*)child_itm->m_pData);
				if (child_weap_mag)
				{
					child_weap_mag->UnloadMagazine();
				}
			}
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
			EatItem(cell_item);
			break;
		}
	/*case BATTERY_CHARGE_DETECTOR:
		{
			CBattery* battery = smart_cast<CBattery*>(item);
			if (!battery)
				break;
			battery->m_iUseFor = 2;
			EatItem(cell_item);
			break;
		}*/
	case BATTERY_CHARGE_DOSIMETER:
		{
			CBattery* battery = smart_cast<CBattery*>(item);
			if (!battery)
				break;
			battery->m_iUseFor = 2;
			EatItem(cell_item);
			break;
		}
	case REPAIR_KIT_OUTFIT:
		{
			CRepairKit* repair_kit = smart_cast<CRepairKit*>(item);
			if (!repair_kit)
				break;
			repair_kit->m_iUseFor = 1;
			EatItem(cell_item);
			break;
		}
	case REPAIR_KIT_KNIFE:
		{
			CRepairKit* repair_kit = smart_cast<CRepairKit*>(item);
			if (!repair_kit)
				break;
			repair_kit->m_iUseFor = 2;
			EatItem(cell_item);
			break;
		}
	case REPAIR_KIT_WPN1:
		{
			CRepairKit* repair_kit = smart_cast<CRepairKit*>(item);
			if (!repair_kit)
				break;
			repair_kit->m_iUseFor = 3;
			EatItem(cell_item);
			break;
		}
	case REPAIR_KIT_WPN2:
		{
			CRepairKit* repair_kit = smart_cast<CRepairKit*>(item);
			if (!repair_kit)
				break;
			repair_kit->m_iUseFor = 4;
			EatItem(cell_item);
			break;
		}
	case FILTER_CHANGE_OUTFIT:
		{
			CAntigasFilter* filter = smart_cast<CAntigasFilter*>(item);
			if (!filter)
				break;
			EatItem(cell_item);
			break;
		}
	}//switch

	SetCurrentItem(NULL);
}//ProcessPropertiesBoxClicked


/*bool CUICarBodyWnd::TryUseItem(CUICellItem* cell_itm)
{
	if (!cell_itm)
	{
		return false;
	}
	PIItem				item = (PIItem)cell_itm->m_pData;
	CBottleItem* pBottleItem = smart_cast<CBottleItem*>		(item);
	CMedkit* pMedkit = smart_cast<CMedkit*>			(item);
	CAntirad* pAntirad = smart_cast<CAntirad*>			(item);
	CEatableItem* pEatableItem = smart_cast<CEatableItem*>		(item);

	Msg("trying to eat [%s]", item->object().cNameSect().c_str());
	if (pMedkit || pAntirad || pEatableItem || pBottleItem)
	{
		EatItem(item);
		return true;
	}
	return false;
}*/

/*bool CUICarBodyWnd::DropItem(PIItem itm, CUIDragDropListEx* lst)
{
	CUICellItem* _citem = lst->ItemsCount() ? lst->GetItemIdx(0) : NULL;
	PIItem _iitem = _citem ? (PIItem)_citem->m_pData : NULL;

	if (!_iitem)					return	false;
	if (!_iitem->CanAttach(itm))	return	false;
	AttachAddon(_iitem);

	return							true;
}*/

void CUICarBodyWnd::PropertiesBoxForWeapon(CUICellItem* cell_item, PIItem item, bool& b_show)
{
	//отсоединение аддонов от вещи
	CWeapon* pWeapon = smart_cast<CWeapon*>(item);
	if (!pWeapon)
	{
		return;
	}

	if (pWeapon->GrenadeLauncherAttachable())
	{
		if (pWeapon->IsGrenadeLauncherAttached())
		{
			m_pUIPropertiesBox->AddItem("st_detach_gl", NULL, INVENTORY_DETACH_GRENADE_LAUNCHER_ADDON);
			b_show = true;
		}
	}
	if (pWeapon->ScopeAttachable())
	{
		if (pWeapon->IsScopeAttached())
		{
			m_pUIPropertiesBox->AddItem("st_detach_scope", NULL, INVENTORY_DETACH_SCOPE_ADDON);
			b_show = true;
		}
	}
	if (pWeapon->SilencerAttachable())
	{
		if (pWeapon->IsSilencerAttached())
		{
			m_pUIPropertiesBox->AddItem("st_detach_silencer", NULL, INVENTORY_DETACH_SILENCER_ADDON);
			b_show = true;
		}
	}

	if (smart_cast<CWeaponMagazined*>(pWeapon) && IsGameTypeSingle())
	{
		bool b = (pWeapon->GetAmmoElapsed() != 0);
		if (!b)
		{
			for (u32 i = 0; i < cell_item->ChildsCount(); ++i)
			{
				CWeaponMagazined* weap_mag = smart_cast<CWeaponMagazined*>((CWeapon*)cell_item->Child(i)->m_pData);
				if (weap_mag && weap_mag->GetAmmoElapsed())
				{
					b = true;
					break; // for
				}
			}
		}
		if (b)
		{
			m_pUIPropertiesBox->AddItem("st_unload_magazine", NULL, INVENTORY_UNLOAD_MAGAZINE);
			b_show = true;
		}
	}
}

void CUICarBodyWnd::PropertiesBoxForUsing(PIItem item, bool& b_show)
{
	CMedkit* pMedkit			= smart_cast<CMedkit*>		(item);
	CAntirad* pAntirad			= smart_cast<CAntirad*>		(item);
	CEatableItem* pEatableItem	= smart_cast<CEatableItem*>	(item);
	CBottleItem* pBottleItem	= smart_cast<CBottleItem*>	(item);
	CBattery* pBattery			= smart_cast<CBattery*>		(item);
	CAntigasFilter* pFilter		= smart_cast<CAntigasFilter*>(item);
	CRepairKit* pRepairKit		= smart_cast<CRepairKit*>	(item);

	LPCSTR act_str = NULL;

	if (!item->Useful() || (pFilter && !pFilter->UseAllowed()) || (pRepairKit && !pRepairKit->UseAllowed()))
		return;

	CTorch* item_in_torch_slot = smart_cast<CTorch*>(m_pOurObject->inventory().ItemFromSlot(TORCH_SLOT));
	//CCustomDetector* item_in_detector_slot = smart_cast<CCustomDetector*>(m_pOurObject->inventory().ItemFromSlot(DETECTOR_SLOT));
	CCustomOutfit* item_in_outfit_slot = smart_cast<CCustomOutfit*>(m_pOurObject->inventory().ItemFromSlot(OUTFIT_SLOT));
	CDetectorAnomaly* item_in_anomaly_detector_slot = smart_cast<CDetectorAnomaly*>(m_pOurObject->inventory().ItemFromSlot(DETECTOR_SLOT));
	CWeapon* item_in_knife_slot = smart_cast<CWeapon*>(m_pOurObject->inventory().ItemFromSlot(KNIFE_SLOT));
	CWeapon* item_in_wpn1_slot = smart_cast<CWeapon*>(m_pOurObject->inventory().ItemFromSlot(PISTOL_SLOT));
	CWeapon* item_in_wpn2_slot = smart_cast<CWeapon*>(m_pOurObject->inventory().ItemFromSlot(RIFLE_SLOT));

	bool can_repair_outfit = false;
	bool can_repair_knife = false;
	bool can_repair_wpn1 = false;
	bool can_repair_wpn2 = false;

	bool outfit_use_filter = false;

	if (item_in_outfit_slot && pFilter)
		outfit_use_filter = item_in_outfit_slot->m_bUseFilter && item_in_outfit_slot->m_fFilterCondition <= 0.99f && item_in_outfit_slot->IsNecessaryItem(pFilter->cNameSect().c_str(), item_in_outfit_slot->m_SuitableFilters);

	if (item_in_outfit_slot && pRepairKit)
		can_repair_outfit = item_in_outfit_slot->GetCondition() < 0.9f && item_in_outfit_slot->GetCondition() >= 0.4f && item_in_outfit_slot->IsNecessaryItem(pRepairKit->cNameSect().c_str(), item_in_outfit_slot->m_SuitableRepairKits);
	if (item_in_knife_slot && pRepairKit)
		can_repair_knife = item_in_knife_slot->GetCondition() < 0.9f && item_in_knife_slot->GetCondition() >= 0.4f && item_in_knife_slot->IsNecessaryItem(pRepairKit->cNameSect().c_str(), item_in_knife_slot->m_SuitableRepairKits);
	if (item_in_wpn1_slot && pRepairKit)
		can_repair_wpn1 = item_in_wpn1_slot->GetCondition() < 0.9f && item_in_wpn1_slot->GetCondition() >= 0.4f && item_in_wpn1_slot->IsNecessaryItem(pRepairKit->cNameSect().c_str(), item_in_wpn1_slot->m_SuitableRepairKits);
	if (item_in_wpn2_slot && pRepairKit)
		can_repair_wpn2 = item_in_wpn2_slot->GetCondition() < 0.9f && item_in_wpn2_slot->GetCondition() >= 0.4f && item_in_wpn2_slot->IsNecessaryItem(pRepairKit->cNameSect().c_str(), item_in_wpn2_slot->m_SuitableRepairKits);

	if (pBattery)
	{
		if (item_in_torch_slot && item_in_torch_slot->IsNecessaryItem(pBattery->cNameSect().c_str(), item_in_torch_slot->m_SuitableBatteries) && item_in_torch_slot->GetChargeLevel() <= 0.99f)
		{
			shared_str str = CStringTable().translate("st_charge_item");
			str.printf("%s %s", str.c_str(), item_in_torch_slot->m_name.c_str());
			m_pUIPropertiesBox->AddItem(str.c_str(), (void*)item_in_torch_slot, BATTERY_CHARGE_TORCH);
			b_show = true;
		}

		/*if (item_in_detector_slot && item_in_detector_slot->IsNecessaryItem(pBattery->cNameSect().c_str(), item_in_detector_slot->m_SuitableBatteries) && item_in_detector_slot->GetChargeLevel() <= 0.99f)
		{
			shared_str str = CStringTable().translate("st_charge_item");
			str.printf("%s %s", str.c_str(), item_in_detector_slot->m_name.c_str());
			m_pUIPropertiesBox->AddItem(str.c_str(), (void*)item_in_detector_slot, BATTERY_CHARGE_DETECTOR);
			b_show = true;
		} */

		if (item_in_anomaly_detector_slot && item_in_anomaly_detector_slot->IsNecessaryItem(pBattery->cNameSect().c_str(), item_in_anomaly_detector_slot->m_SuitableBatteries) && item_in_anomaly_detector_slot->GetChargeLevel() <= 0.99f)
		{
			shared_str str = CStringTable().translate("st_charge_item");
			str.printf("%s %s", str.c_str(), item_in_anomaly_detector_slot->m_name.c_str());
			m_pUIPropertiesBox->AddItem(str.c_str(), (void*)item_in_anomaly_detector_slot, BATTERY_CHARGE_DOSIMETER);
			b_show = true;
		}

		return;
	}
	else if (pRepairKit)
	{
		if (item_in_outfit_slot && can_repair_outfit)
		{
			shared_str str = CStringTable().translate("st_repair");
			str.printf("%s %s", str.c_str(), item_in_outfit_slot->m_name.c_str());
			m_pUIPropertiesBox->AddItem(str.c_str(), (void*)item_in_outfit_slot, REPAIR_KIT_OUTFIT);
			b_show = true;
		}

		if (item_in_knife_slot && can_repair_knife)
		{
			shared_str str = CStringTable().translate("st_repair");
			str.printf("%s %s", str.c_str(), item_in_knife_slot->m_name.c_str());
			m_pUIPropertiesBox->AddItem(str.c_str(), (void*)item_in_knife_slot, REPAIR_KIT_KNIFE);
			b_show = true;
		}

		if (item_in_wpn1_slot && can_repair_wpn1)
		{
			shared_str str = CStringTable().translate("st_repair");
			str.printf("%s %s", str.c_str(), item_in_wpn1_slot->m_name.c_str());
			m_pUIPropertiesBox->AddItem(str.c_str(), (void*)item_in_wpn1_slot, REPAIR_KIT_WPN1);
			b_show = true;
		}

		if (item_in_wpn2_slot && can_repair_wpn2)
		{
			shared_str str = CStringTable().translate("st_repair");
			str.printf("%s %s", str.c_str(), item_in_wpn2_slot->m_name.c_str());
			m_pUIPropertiesBox->AddItem(str.c_str(), (void*)item_in_wpn2_slot, REPAIR_KIT_WPN2);
			b_show = true;
		}
		return;
	}
	else if (pFilter)
	{
		if (item_in_outfit_slot && outfit_use_filter)
		{
			shared_str str = CStringTable().translate("st_change_filter");
			str.printf("%s %s", str.c_str(), item_in_outfit_slot->m_name.c_str());
			m_pUIPropertiesBox->AddItem(str.c_str(), (void*)item_in_outfit_slot, FILTER_CHANGE_OUTFIT);
			b_show = true;
		}
		return;
	}
	if (pMedkit || pAntirad)
	{
		act_str = "st_use";
	}
	else if (pEatableItem && !pBattery)
	{
		if (pBottleItem)
		{
			act_str = "st_drink";
		}
		else
		{
			act_str = "st_eat";
		}
	}
	if (act_str)
	{
		m_pUIPropertiesBox->AddItem(act_str, NULL, INVENTORY_EAT_ACTION);
		b_show = true;
	}
}

void CUICarBodyWnd::PropertiesBoxForPlaying(PIItem item, bool& b_show)
{
	CPda* pPda = smart_cast<CPda*>(item);
	if (!pPda || !pPda->CanPlayScriptFunction())
		return;

	LPCSTR act_str = "st_play";
	m_pUIPropertiesBox->AddItem(act_str, NULL, INVENTORY_PLAY_ACTION);
	b_show = true;
}

void CUICarBodyWnd::PropertiesBoxForDrop(CUICellItem* cell_item, PIItem item, bool& b_show)
{
	if (!item->IsQuestItem())
	{
		m_pUIPropertiesBox->AddItem("st_drop", NULL, INVENTORY_DROP_ACTION);
		b_show = true;

		if (cell_item->ChildsCount())
		{
			m_pUIPropertiesBox->AddItem("st_drop_all", (void*)33, INVENTORY_DROP_ACTION);
		}
	}
	if (item->object().H_Parent()->ID() != m_pOurObject->object_id())
	{
		m_pUIPropertiesBox->AddItem("st_move_to_bag", NULL, INVENTORY_TO_BAG_ACTION);
		//if (cell_item->ChildsCount())
		//{
		//	m_pUIPropertiesBox->AddItem("st_move_all_to_bag", NULL, INVENTORY_ALL_TO_BAG_ACTION); // (void*)33 does not work here, trying to use another message
		//}
		b_show = true;
	}
	else
	{
		if (m_pOthersObject)
		{
			m_pUIPropertiesBox->AddItem("st_move_to_partners_bag", NULL, INVENTORY_TO_BAG_ACTION);
			//if (cell_item->ChildsCount())
			//{
			//	m_pUIPropertiesBox->AddItem("st_move_all_to_partners_bag", NULL, INVENTORY_ALL_TO_BAG_ACTION);
			//}
		}
		else if (m_pInventoryBox)
		{
			m_pUIPropertiesBox->AddItem("st_move_to_box", NULL, INVENTORY_TO_BAG_ACTION);
			//if (cell_item->ChildsCount())
			//{
			//	m_pUIPropertiesBox->AddItem("st_move_all_to_box", NULL, INVENTORY_ALL_TO_BAG_ACTION);
			//}
		}
		b_show = true;
	}
}

void SendEvent_Item_Drop(PIItem	pItem)
{
	pItem->SetDropManual(TRUE);

	if (OnClient())
	{
		NET_Packet					P;
		pItem->object().u_EventGen(P, GE_OWNERSHIP_REJECT, pItem->object().H_Parent()->ID());
		P.w_u16(pItem->object().ID());
		pItem->object().u_EventSend(P);
	}
};

void CUICarBodyWnd::DropCurrentItem(bool b_all)
{
	if(!b_all && CurrentIItem() && !CurrentIItem()->IsQuestItem())
	{
		SendEvent_Item_Drop		(CurrentIItem());
		SetCurrentItem			(NULL);
		//InventoryUtilities::UpdateWeight			(UIBagWnd, true);
		return;
	}

	if(b_all && CurrentIItem() && !CurrentIItem()->IsQuestItem())
	{
		u32 cnt = CurrentItem()->ChildsCount();

		for(u32 i=0; i<cnt; ++i){
			CUICellItem*	itm				= CurrentItem()->PopChild(NULL);
			PIItem			iitm			= (PIItem)itm->m_pData;
			SendEvent_Item_Drop				(iitm);
		}

		SendEvent_Item_Drop					(CurrentIItem());
		SetCurrentItem						(NULL);
		//InventoryUtilities::UpdateWeight	(UIBagWnd, true);
		return;
	}
}
