#include "stdafx.h"
#include "UIInventoryWnd.h"
#include "../actor.h"
#include "../silencer.h"
#include "../scope.h"
#include "../grenadelauncher.h"
#include "../Artefact.h"
#include "../eatable_item.h"
#include "../BottleItem.h"
#include "../WeaponMagazined.h"
#include "../inventory.h"
#include "../game_base.h"
#include "../game_cl_base.h"
#include "../xr_level_controller.h"
#include "UICellItem.h"
#include "UIListBoxItem.h"
#include "../CustomOutfit.h"
#include "../Battery.h"

#include "../string_table.h"

void CUIInventoryWnd::EatItem(PIItem itm)
{
	SetCurrentItem							(NULL);
	if(!itm->Useful())						return;

	SendEvent_Item_Eat						(itm);

	PlaySnd									(eInvItemUse);
}

#include "../Medkit.h"
#include "../Antirad.h"
void CUIInventoryWnd::ActivatePropertiesBox()
{
	if (UIPropertiesBox->IsShown())
	{
		UIPropertiesBox->Hide();
	}

	PIItem item = CurrentIItem();
	if (!item)
	{
		return;
	}

	CUICellItem* cell_item = CurrentItem();
	UIPropertiesBox->RemoveAll();
	bool b_show = false;

	PropertiesBoxForSlots	(item, b_show);
	PropertiesBoxForWeapon	(cell_item, item, b_show);
	PropertiesBoxForAddon	(item, b_show);
	PropertiesBoxForUsing	(item, b_show);
	PropertiesBoxForDrop	(cell_item, item, b_show);

	if(b_show)
	{
		UIPropertiesBox->AutoUpdateSize	();
		UIPropertiesBox->BringAllToTop	();

		Fvector2						cursor_pos;
		Frect							vis_rect;
		GetAbsoluteRect					(vis_rect);
		cursor_pos						= GetUICursor().GetCursorPosition();
		cursor_pos.sub					(vis_rect.lt);
		UIPropertiesBox->Show			(vis_rect, cursor_pos);
		PlaySnd							(eInvProperties);
	}
}

void CUIInventoryWnd::ProcessPropertiesBoxClicked(CUIWindow* w, void* d)
{
	PIItem			item = CurrentIItem();
	CUICellItem* cell_item = CurrentItem();
	if (!UIPropertiesBox->GetClickedItem() || !item || !cell_item || !cell_item->OwnerList())
	{
		return;
	}
	CWeapon* weapon = smart_cast<CWeapon*>(item);

	switch (UIPropertiesBox->GetClickedItem()->GetTAG())
	{
	case INVENTORY_TO_SLOT_ACTION:
	{
		ToSlot(cell_item, true);
		SetCurrentItem(NULL);
	}break;
	case INVENTORY_TO_BELT_ACTION:
	{
		ToBelt(cell_item, false);
		SetCurrentItem(NULL);
	}break;
	case INVENTORY_TO_BAG_ACTION:
	{
		ToBag(cell_item, false);
		SetCurrentItem(NULL);
	}break;
	case INVENTORY_EAT_ACTION:
	{
	//	HUD().GetUI()->UIGame()->ActorMenu().SetCurrentConsumable(cell_item);
		TryUseItem(cell_item);
		SetCurrentItem(NULL);
		break;
	}
	case INVENTORY_DROP_ACTION:
	{
		void* d = UIPropertiesBox->GetClickedItem()->GetData();
		bool b_all = (d == (void*)33);

		DropCurrentItem(b_all);
		break;
	}
	case INVENTORY_ATTACH_ADDON:
	{
		PIItem item = CurrentIItem(); // temporary storing because of AttachAddon is setting curiitem to NULL
		AttachAddon((PIItem)(UIPropertiesBox->GetClickedItem()->GetData()));

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
					DetachAddon(wpn->GetScopeName().c_str(), child_iitm);
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
					DetachAddon(wpn->GetSilencerName().c_str(), child_iitm);
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
					DetachAddon(wpn->GetGrenadeLauncherName().c_str(), child_iitm);
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
	case BATTERY_CHARGE_TORCH:
		{
			CBattery* battery = smart_cast<CBattery*>(CurrentItem());
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
	/*case BATTERY_CHARGE_DOSIMETER:
		{
			CBattery* battery = smart_cast<CBattery*>(CurrentItem());
			if (!battery)
				break;
			battery->m_iUseFor = 2;
			TryUseItem((PIItem)(UIPropertiesBox.GetClickedItem()->GetData()));
			break;
		} */
	}//switch

	SetCurrentItem(NULL);
}//ProcessPropertiesBoxClicked


bool CUIInventoryWnd::TryUseItem(CUICellItem* cell_itm)
{
	if (!cell_itm)
	{
		return false;
	}
	PIItem				item				= (PIItem)cell_itm->m_pData;
	CBottleItem*		pBottleItem			= smart_cast<CBottleItem*>		(item);
	CMedkit*			pMedkit				= smart_cast<CMedkit*>			(item);
	CAntirad*			pAntirad			= smart_cast<CAntirad*>			(item);
	CEatableItem*		pEatableItem		= smart_cast<CEatableItem*>		(item);
	CBattery*			pBattery			= smart_cast<CBattery*>			(item);

	Msg("trying to eat [%s]", item->object().cNameSect().c_str());
	if (pMedkit || pAntirad || pEatableItem || pBottleItem || pBattery)
	{
		EatItem(item);
		return true;
	}
	return false;
}

bool CUIInventoryWnd::DropItem(PIItem itm, CUIDragDropListEx* lst)
{
	if (lst == m_pUIOutfitList)
	{
		return TryUseItem			(CurrentItem());
	}
	
	CUICellItem* _citem = lst->ItemsCount() ? lst->GetItemIdx(0) : NULL;
	PIItem _iitem = _citem ? (PIItem)_citem->m_pData : NULL;

	if (!_iitem)					return	false;
	if (!_iitem->CanAttach(itm))	return	false;
	AttachAddon						(_iitem);

	return							true;
}

void CUIInventoryWnd::PropertiesBoxForSlots(PIItem item, bool& b_show)
{
	CCustomOutfit* pOutfit	= smart_cast<CCustomOutfit*>( item );

	// Флаг-признак для невлючения пункта контекстного меню: Dreess Outfit, если костюм уже надет
	bool bAlreadyDressed	= false;
	u32 const cur_slot		= item->GetSlot();

	if (!pOutfit && cur_slot != NO_ACTIVE_SLOT
		&& !m_pInv->m_slots[cur_slot].m_bPersistent && m_pInv->CanPutInSlot(item) )
	{
		UIPropertiesBox->AddItem( "st_move_to_slot",  NULL, INVENTORY_TO_SLOT_ACTION );
		b_show = true;
	}
	if (item->Belt() && m_pInv->CanPutInBelt( item ))
	{
		UIPropertiesBox->AddItem( "st_move_on_belt",  NULL, INVENTORY_TO_BELT_ACTION );
		b_show = true;
	}

	if (item->Ruck() && m_pInv->CanPutInRuck( item )
		&& ( cur_slot == (u32)(-1) || !m_pInv->m_slots[cur_slot].m_bPersistent ) )
	{
		if( !pOutfit )
		{
			UIPropertiesBox->AddItem( "st_move_to_bag",  NULL, INVENTORY_TO_BAG_ACTION );
		}
		else
		{
			UIPropertiesBox->AddItem( "st_undress_outfit",  NULL, INVENTORY_TO_BAG_ACTION );
		}
		bAlreadyDressed = true;
		b_show			= true;
	}
	if ( pOutfit && !bAlreadyDressed )
	{
		UIPropertiesBox->AddItem( "st_dress_outfit",  NULL, INVENTORY_TO_SLOT_ACTION );
		b_show			= true;
	}
}

void CUIInventoryWnd::PropertiesBoxForWeapon(CUICellItem* cell_item, PIItem item, bool& b_show)
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
			UIPropertiesBox->AddItem("st_detach_gl", NULL, INVENTORY_DETACH_GRENADE_LAUNCHER_ADDON);
			b_show = true;
		}
	}
	if (pWeapon->ScopeAttachable())
	{
		if (pWeapon->IsScopeAttached())
		{
			UIPropertiesBox->AddItem("st_detach_scope", NULL, INVENTORY_DETACH_SCOPE_ADDON);
			b_show = true;
		}
	}
	if (pWeapon->SilencerAttachable())
	{
		if (pWeapon->IsSilencerAttached())
		{
			UIPropertiesBox->AddItem("st_detach_silencer", NULL, INVENTORY_DETACH_SILENCER_ADDON);
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
			UIPropertiesBox->AddItem("st_unload_magazine", NULL, INVENTORY_UNLOAD_MAGAZINE);
			b_show = true;
		}
	}
}

void CUIInventoryWnd::PropertiesBoxForUsing(PIItem item, bool& b_show)
{
	CMedkit* pMedkit = smart_cast<CMedkit*>		(item);
	CAntirad* pAntirad = smart_cast<CAntirad*>		(item);
	CEatableItem* pEatableItem = smart_cast<CEatableItem*>	(item);
	CBottleItem* pBottleItem = smart_cast<CBottleItem*>	(item);
	CArtefact* pArtefact = smart_cast<CArtefact*>	(item);
	CBattery* pBattery = smart_cast<CBattery*>(item);

	LPCSTR act_str = NULL;

	if (!item->Useful())
		return;

	CInventory* inv = &Actor()->inventory();
	PIItem	item_in_torch_slot = inv->ItemFromSlot(TORCH_SLOT);
	PIItem	item_in_detector_slot = inv->ItemFromSlot(DETECTOR_SLOT);
	//PIItem	item_in_anomaly_detector_slot = inv->ItemFromSlot(DOSIMETER_SLOT);

	if (pMedkit || pAntirad)
	{
		act_str = "st_use";
	}
	else if (pEatableItem)
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
	else if (pBattery)
	{
		if (item_in_torch_slot)
		{
			shared_str str = CStringTable().translate("st_charge_item");
			str.printf("%s %s", str.c_str(), item_in_torch_slot->m_name.c_str());
			UIPropertiesBox->AddItem(str.c_str(), (void*)item_in_torch_slot, BATTERY_CHARGE_TORCH);
			b_show = true;
		}

		if (item_in_detector_slot)
		{
			shared_str str = CStringTable().translate("st_charge_item");
			str.printf("%s %s", str.c_str(), item_in_detector_slot->m_name.c_str());
			UIPropertiesBox->AddItem(str.c_str(), (void*)item_in_detector_slot, BATTERY_CHARGE_DETECTOR);
			b_show = true;
		}

		/*if (item_in_anomaly_detector_slot) // Это потом пригодится ещё
		{
			shared_str str = CStringTable().translate("st_charge_item");
			str.printf("%s %s", str.c_str(), item_in_anomaly_detector_slot->m_name.c_str());
			UIPropertiesBox.AddItem(str.c_str(), (void*)item_in_anomaly_detector_slot, BATTERY_CHARGE_DOSIMETER);
			b_show = true;
		}  */
		return;
	}
	if (act_str)
	{
		UIPropertiesBox->AddItem(act_str, NULL, INVENTORY_EAT_ACTION);
		b_show = true;
	}
}

void CUIInventoryWnd::PropertiesBoxForDrop(CUICellItem* cell_item, PIItem item, bool& b_show)
{
	if (!item->IsQuestItem())
	{
		UIPropertiesBox->AddItem("st_drop", NULL, INVENTORY_DROP_ACTION);
		b_show = true;

		if (cell_item->ChildsCount())
		{
			UIPropertiesBox->AddItem("st_drop_all", (void*)33, INVENTORY_DROP_ACTION);
		}
	}
}

#include "../string_table.h"
void CUIInventoryWnd::PropertiesBoxForAddon(PIItem item, bool& b_show)
{
	//присоединение аддонов к активному слоту (2 или 3)

	CScope* pScope = smart_cast<CScope*>			(item);
	CSilencer* pSilencer = smart_cast<CSilencer*>		(item);
	CGrenadeLauncher* pGrenadeLauncher = smart_cast<CGrenadeLauncher*>	(item);

	if (pScope)
	{
		if (m_pInv->m_slots[PISTOL_SLOT].m_pIItem && m_pInv->m_slots[PISTOL_SLOT].m_pIItem->CanAttach(pScope))
		{
			PIItem tgt = m_pInv->m_slots[PISTOL_SLOT].m_pIItem;
			UIPropertiesBox->AddItem("st_attach_scope_to_pistol", (void*)tgt, INVENTORY_ATTACH_ADDON);
			b_show = true;
		}
		if (m_pInv->m_slots[RIFLE_SLOT].m_pIItem && m_pInv->m_slots[RIFLE_SLOT].m_pIItem->CanAttach(pScope))
		{
			PIItem tgt = m_pInv->m_slots[RIFLE_SLOT].m_pIItem;
			UIPropertiesBox->AddItem("st_attach_scope_to_rifle", (void*)tgt, INVENTORY_ATTACH_ADDON);
			b_show = true;
		}
		return;
	}

	if (pSilencer)
	{
		if (m_pInv->m_slots[PISTOL_SLOT].m_pIItem && m_pInv->m_slots[PISTOL_SLOT].m_pIItem->CanAttach(pSilencer))
		{
			PIItem tgt = m_pInv->m_slots[PISTOL_SLOT].m_pIItem;
			UIPropertiesBox->AddItem("st_attach_silencer_to_pistol", (void*)tgt, INVENTORY_ATTACH_ADDON);
			b_show = true;
		}
		if (m_pInv->m_slots[RIFLE_SLOT].m_pIItem && m_pInv->m_slots[RIFLE_SLOT].m_pIItem->CanAttach(pSilencer))
		{
			PIItem tgt = m_pInv->m_slots[RIFLE_SLOT].m_pIItem;
			UIPropertiesBox->AddItem("st_attach_silencer_to_rifle", (void*)tgt, INVENTORY_ATTACH_ADDON);
			b_show = true;
		}
		return;
	}

	if (pGrenadeLauncher)
	{
		if (m_pInv->m_slots[RIFLE_SLOT].m_pIItem && m_pInv->m_slots[RIFLE_SLOT].m_pIItem->CanAttach(pGrenadeLauncher))
		{
			PIItem tgt = m_pInv->m_slots[RIFLE_SLOT].m_pIItem;
			UIPropertiesBox->AddItem("st_attach_gl_to_rifle", (void*)tgt, INVENTORY_ATTACH_ADDON);
			b_show = true;
		}
	}
}
