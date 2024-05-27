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
			move_item(bMoveDirection ? m_pInventoryBox->ID() : tmp_id, bMoveDirection ? tmp_id : m_pInventoryBox->ID(), CurrentIItem()->object().ID());

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

	LPCSTR act_str = NULL;

	if (!item->Useful())
		return;

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
	if (act_str)
	{
		m_pUIPropertiesBox->AddItem(act_str, NULL, INVENTORY_EAT_ACTION);
		b_show = true;
	}
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
			CUICellItem*	itm				= CurrentItem()->PopChild();
			PIItem			iitm			= (PIItem)itm->m_pData;
			SendEvent_Item_Drop				(iitm);
		}

		SendEvent_Item_Drop					(CurrentIItem());
		SetCurrentItem						(NULL);
		//InventoryUtilities::UpdateWeight	(UIBagWnd, true);
		return;
	}
}
