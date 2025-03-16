#include "stdafx.h"
#include "UICellItem.h"
#include "UIHelper.h"
#include "UIInventoryWnd.h"

#include "../Artefact.h"
#include "../CustomBackpack.h"
#include "../AnomalyDetector.h"
#include "../CustomOutfit.h"
#include "../ActorHelmet.h"
#include "../GrenadeLauncher.h"
#include "../LaserDesignator.h"
#include "../PDA.h"
#include "../Scope.h"
#include "../Silencer.h"
#include "../TacticalTorch.h"
#include "../Torch.h"
#include "../Weapon.h"
#include "../WeaponBinoculars.h"
#include "../WeaponKnife.h"
#include "../WeaponMagazinedWGrenade.h"
#include "../WeaponPistol.h"
#include "../AdvancedXrayGameConstants.h"

void CUIInventoryWnd::InitHighlights(CUIXml& uiXml)
{
	m_InvSlot2Highlight = UIHelper::CreateStatic(uiXml, "inv_slot2_highlight", this);
	m_InvSlot2Highlight->Show(false);
	m_InvSlot3Highlight = UIHelper::CreateStatic(uiXml, "inv_slot3_highlight", this);
	m_InvSlot3Highlight->Show(false);
	m_OutfitSlotHighlight = UIHelper::CreateStatic(uiXml, "outfit_slot_highlight", this);
	m_OutfitSlotHighlight->Show(false);
	//m_QuickSlotsHighlight[0] = UIHelper::CreateStatic(uiXml, "quick_slot_highlight", this);
	//m_QuickSlotsHighlight[0]->Show(false);

	if (GameConstants::GetKnifeSlotEnabled())
	{
		if ((m_KnifeSlotHighlight = UIHelper::CreateStatic(uiXml, "knife_slot_highlight", this)))
			m_KnifeSlotHighlight->Show(false);
	}

	if (GameConstants::GetBinocularSlotEnabled())
	{
		if ((m_BinocularSlotHighlight = UIHelper::CreateStatic(uiXml, "binocular_slot_highlight", this)))
			m_BinocularSlotHighlight->Show(false);
	}

	if (GameConstants::GetTorchSlotEnabled())
	{
		if ((m_TorchSlotHighlight = UIHelper::CreateStatic(uiXml, "torch_slot_highlight", this)))
			m_TorchSlotHighlight->Show(false);
	}

	if (GameConstants::GetBackpackSlotEnabled())
	{
		if ((m_BackpackSlotHighlight = UIHelper::CreateStatic(uiXml, "backpack_slot_highlight", this)))
			m_BackpackSlotHighlight->Show(false);
	}

	if (GameConstants::GetDosimeterSlotEnabled())
	{
		if ((m_DetectorSlotHighlight = UIHelper::CreateStatic(uiXml, "detector_slot_highlight", this)))
			m_DetectorSlotHighlight->Show(false);
	}

	if (GameConstants::GetPantsSlotEnabled())
	{
		if ((m_PantsSlotHighlight = UIHelper::CreateStatic(uiXml, "pants_slot_highlight", this)))
			m_PantsSlotHighlight->Show(false);
	}

	if (GameConstants::GetPdaSlotEnabled())
	{
		if ((m_PdaSlotHighlight = UIHelper::CreateStatic(uiXml, "pda_slot_highlight", this)))
			m_PdaSlotHighlight->Show(false);
	}

	if (GameConstants::GetHelmetSlotEnabled())
	{
		if ((m_HelmetSlotHighlight = UIHelper::CreateStatic(uiXml, "helmet_slot_highlight", this)))
			m_HelmetSlotHighlight->Show(false);
	}

	if (GameConstants::GetSecondHelmetSlotEnabled())
	{
		if ((m_SecondHelmetSlotHighlight = UIHelper::CreateStatic(uiXml, "second_helmet_slot_highlight", this)))
			m_SecondHelmetSlotHighlight->Show(false);
	}

	Fvector2 pos;

	int cols = m_pUIBeltList->CellsCapacity().x;
	int rows = m_pUIBeltList->CellsCapacity().y;
	int counter = 1;
	float dx = uiXml.ReadAttribFlt("artefact_slot_highlight", 0, "dx", 24.0f);
	float dy = uiXml.ReadAttribFlt("artefact_slot_highlight", 0, "dy", 24.0f);
	for (u8 i = 0; i < rows; ++i)
	{
		for (u8 j = 0; j < cols; ++j)
		{
			m_ArtefactSlotsHighlight.push_back(UIHelper::CreateStatic(uiXml, "artefact_slot_highlight", this));

			if (i == 0 && j == 0)
			{
				pos = m_ArtefactSlotsHighlight[0]->GetWndPos();
				m_ArtefactSlotsHighlight[0]->Show(false);
				dx = uiXml.ReadAttribFlt("artefact_slot_highlight", 0, "dx", 24.0f);
				dy = uiXml.ReadAttribFlt("artefact_slot_highlight", 0, "dy", 24.0f);
			}
			else
			{
				if (j != 0)
					pos.x += dx;

				m_ArtefactSlotsHighlight[counter]->SetWndPos(pos);
				m_ArtefactSlotsHighlight[i]->Show(false);
				counter++;
			}
		}

		pos.x = m_ArtefactSlotsHighlight[0]->GetWndPos().x;
		pos.y += dy;
	}

	m_highlight_clear = true;
}

void CUIInventoryWnd::clear_highlight_lists()
{
	m_InvSlot2Highlight->Show(false);
	m_InvSlot3Highlight->Show(false);
	m_OutfitSlotHighlight->Show(false);

	if (GameConstants::GetKnifeSlotEnabled())
	{
		m_KnifeSlotHighlight->Show(false);
	}

	if (GameConstants::GetBinocularSlotEnabled())
	{
		m_BinocularSlotHighlight->Show(false);
	}

	if (GameConstants::GetTorchSlotEnabled())
	{
		if (m_TorchSlotHighlight)
			m_TorchSlotHighlight->Show(false);
	}

	if (GameConstants::GetBackpackSlotEnabled())
	{
		if (m_BackpackSlotHighlight)
			m_BackpackSlotHighlight->Show(false);
	}

	if (GameConstants::GetDosimeterSlotEnabled())
	{
		if (m_DetectorSlotHighlight)
			m_DetectorSlotHighlight->Show(false);
	}

	if (GameConstants::GetPantsSlotEnabled())
	{
		if (m_PantsSlotHighlight)
			m_PantsSlotHighlight->Show(false);
	}

	if (GameConstants::GetPdaSlotEnabled())
	{
		if (m_PdaSlotHighlight)
			m_PdaSlotHighlight->Show(false);
	}

	if (GameConstants::GetHelmetSlotEnabled())
	{
		if (m_HelmetSlotHighlight)
			m_HelmetSlotHighlight->Show(false);
	}

	if (GameConstants::GetSecondHelmetSlotEnabled())
	{
		if (m_SecondHelmetSlotHighlight)
			m_SecondHelmetSlotHighlight->Show(false);
	}

	//for (u8 i = 0; i < 4; i++)
	//	m_QuickSlotsHighlight[i]->Show(false);
	for (u8 i = 0; i < GameConstants::GetArtefactsCount(); i++)
		m_ArtefactSlotsHighlight[i]->Show(false);

	m_pUIBagList->clear_select_armament();
	m_pUIPistolList->clear_select_armament();
	m_pUIAutomaticList->clear_select_armament();
	m_pUIBeltList->clear_select_armament();

	m_highlight_clear = true;
}

void CUIInventoryWnd::highlight_item_slot(CUICellItem* cell_item)
{
	PIItem item = (PIItem)cell_item->m_pData;
	if (!item)
		return;

	if (CUIDragDropListEx::m_drag_item)
		return;

	CWeapon* weapon = smart_cast<CWeapon*>(item);
	CCustomOutfit* outfit = smart_cast<CCustomOutfit*>(item);
	CDetectorAnomaly* anomaly_detector = smart_cast<CDetectorAnomaly*>(item);
	CArtefact* artefact = smart_cast<CArtefact*>(item);
	CWeaponKnife* knife = smart_cast<CWeaponKnife*>(item);
	CWeaponBinoculars* binoculars = smart_cast<CWeaponBinoculars*>(item);
	CTorch* torch = smart_cast<CTorch*>(item);
	CCustomBackpack* backpack = smart_cast<CCustomBackpack*>(item);
	CHelmet* helmet = smart_cast<CHelmet*>(item);
	CPda* pda = smart_cast<CPda*>(item);
	CWeaponPistol* pistol = smart_cast<CWeaponPistol*>(item);

	if (weapon && (!knife && !binoculars))
	{
		if (pistol)
		{
			m_InvSlot2Highlight->Show(true);
		}
		else
		{
			m_InvSlot3Highlight->Show(true);
		}

		return;
	}

	if (outfit)
	{
		m_OutfitSlotHighlight->Show(true);

		if (GameConstants::GetPantsSlotEnabled())
		{
			if (m_PantsSlotHighlight)
				m_PantsSlotHighlight->Show(true);
		}
		return;
	}

	if (anomaly_detector)
	{
		m_DetectorSlotHighlight->Show(true);
		return;
	}

#pragma todo ("temporary without quickslots")
	/*if (eatable)
	{
		if (cell_item->OwnerList() && GetListType(cell_item->OwnerList()) == iQuickSlot)
			return;

		for (u8 i = 0; i < 4; i++)
			m_QuickSlotsHighlight[i]->Show(true);
		return;
	}*/

	if (artefact)
	{
		if (cell_item->OwnerList() && GetType(cell_item->OwnerList()) == iwBelt)
			return;

		Ivector2 cap = m_pUIBeltList->CellsCapacity();
		for (u8 i = 0; i < (cap.x * cap.y); i++)
			m_ArtefactSlotsHighlight[i]->Show(true);
		return;
	}

	if (GameConstants::GetKnifeSlotEnabled())
	{
		if (knife)
		{
			if (m_KnifeSlotHighlight)
			{
				m_KnifeSlotHighlight->Show(true);
			}
			return;
		}
	}

	if (GameConstants::GetBinocularSlotEnabled())
	{
		if (binoculars)
		{
			if (m_BinocularSlotHighlight)
			{
				m_BinocularSlotHighlight->Show(true);
			}
			return;
		}
	}

	if (GameConstants::GetTorchSlotEnabled())
	{
		if (torch)
		{
			if (m_TorchSlotHighlight)
			{
				m_TorchSlotHighlight->Show(true);
			}
			return;
		}
	}

	if (GameConstants::GetBackpackSlotEnabled())
	{
		if (backpack)
		{
			if (m_BackpackSlotHighlight)
			{
				m_BackpackSlotHighlight->Show(true);
			}
			return;
		}
	}

	if (GameConstants::GetDosimeterSlotEnabled())
	{
		if (anomaly_detector)
		{
			m_DetectorSlotHighlight->Show(true);
			return;
		}
	}

	if (GameConstants::GetPdaSlotEnabled())
	{
		if (pda)
		{
			m_PdaSlotHighlight->Show(true);
			return;
		}
	}

	if (GameConstants::GetHelmetSlotEnabled())
	{
		if (helmet)
		{
			if (m_HelmetSlotHighlight)
			{
				m_HelmetSlotHighlight->Show(true);
			}
			return;
		}
	}

	if (GameConstants::GetSecondHelmetSlotEnabled())
	{
		if (helmet)
		{
			if (m_SecondHelmetSlotHighlight)
			{
				m_SecondHelmetSlotHighlight->Show(true);
			}
			return;
		}
	}
}

void CUIInventoryWnd::set_highlight_item(CUICellItem* cell_item)
{
	PIItem item = (PIItem)cell_item->m_pData;
	if (!item)
	{
		return;
	}
	highlight_item_slot(cell_item);
	highlight_armament(item, m_pUIBagList);
	highlight_armament(item, m_pUIPistolList);
	highlight_armament(item, m_pUIAutomaticList);
	highlight_armament(item, m_pUIBeltList);
	/*case mmTrade:
	{
		highlight_armament(item, m_pTradeActorBagList);
		highlight_armament(item, m_pTradeActorList);
		highlight_armament(item, m_pTradePartnerBagList);
		highlight_armament(item, m_pTradePartnerList);
		break;
	}
	case mmDeadBodySearch:
	{
		highlight_armament(item, m_pInventoryBagList);
		highlight_armament(item, m_pDeadBodyBagList);
		break;
	}*/
	m_highlight_clear = false;
}

void CUIInventoryWnd::highlight_armament(PIItem item, CUIDragDropListEx* ddlist)
{
	ddlist->clear_select_armament();
	highlight_ammo_for_weapon(item, ddlist);
	highlight_weapons_for_ammo(item, ddlist);
	highlight_weapons_for_addon(item, ddlist);
}

void CUIInventoryWnd::highlight_ammo_for_weapon(PIItem weapon_item, CUIDragDropListEx* ddlist)
{
	VERIFY(weapon_item);
	VERIFY(ddlist);
	static xr_vector<shared_str>	ammo_types;
	ammo_types.clear_not_free();

	CWeapon* weapon = smart_cast<CWeapon*>(weapon_item);
	CWeaponKnife* knife = smart_cast<CWeaponKnife*>(weapon_item);
	CWeaponBinoculars* binoculars = smart_cast<CWeaponBinoculars*>(weapon_item);

	if (!weapon || knife || binoculars)
	{
		return;
	}
	ammo_types.assign(weapon->m_ammoTypes.begin(), weapon->m_ammoTypes.end());

	CWeaponMagazinedWGrenade* wg = smart_cast<CWeaponMagazinedWGrenade*>(weapon_item);
	if (wg)
	{
		if (wg->IsGrenadeLauncherAttached() && wg->m_ammoTypes2.size())
		{
			ammo_types.insert(ammo_types.end(), wg->m_ammoTypes2.begin(), wg->m_ammoTypes2.end());
		}
	}

	if (ammo_types.size() == 0)
	{
		return;
	}
	xr_vector<shared_str>::iterator ite = ammo_types.end();

	u32 const cnt = ddlist->ItemsCount();
	for (u32 i = 0; i < cnt; ++i)
	{
		CUICellItem* ci = ddlist->GetItemIdx(i);
		PIItem item = (PIItem)ci->m_pData;
		if (!item)
		{
			continue;
		}
		CWeaponAmmo* ammo = smart_cast<CWeaponAmmo*>(item);
		if (!ammo)
		{
			highlight_addons_for_weapon(weapon_item, ci);
			continue; // for i
		}
		shared_str const& ammo_name = item->object().cNameSect();

		xr_vector<shared_str>::iterator itb = ammo_types.begin();
		for (; itb != ite; ++itb)
		{
			if (ammo_name._get() == (*itb)._get())
			{
				ci->m_select_armament = true;
				break; // itb
			}
		}
	}//for i

}

void CUIInventoryWnd::highlight_weapons_for_ammo(PIItem ammo_item, CUIDragDropListEx* ddlist)
{
	VERIFY(ammo_item);
	VERIFY(ddlist);

	CWeaponAmmo* ammo = smart_cast<CWeaponAmmo*>(ammo_item);

	if (!ammo)
	{
		return;
	}

	shared_str const& ammo_name = ammo_item->object().cNameSect();

	u32 const cnt = ddlist->ItemsCount();
	for (u32 i = 0; i < cnt; ++i)
	{
		CUICellItem* ci = ddlist->GetItemIdx(i);
		PIItem item = (PIItem)ci->m_pData;
		if (!item)
		{
			continue;
		}

		CWeapon* weapon = smart_cast<CWeapon*>(item);
		CWeaponKnife* knife = smart_cast<CWeaponKnife*>(item);
		CWeaponBinoculars* binoculars = smart_cast<CWeaponBinoculars*>(item);

		if (!weapon || knife || binoculars)
		{
			continue;
		}

		xr_vector<shared_str>::iterator itb = weapon->m_ammoTypes.begin();
		xr_vector<shared_str>::iterator ite = weapon->m_ammoTypes.end();
		for (; itb != ite; ++itb)
		{
			if (ammo_name._get() == (*itb)._get())
			{
				ci->m_select_armament = true;
				break; // for itb
			}
		}

		CWeaponMagazinedWGrenade* wg = smart_cast<CWeaponMagazinedWGrenade*>(item);
		if (!wg || !wg->IsGrenadeLauncherAttached() || !wg->m_ammoTypes2.size())
		{
			continue; // for i
		}
		itb = wg->m_ammoTypes2.begin();
		ite = wg->m_ammoTypes2.end();
		for (; itb != ite; ++itb)
		{
			if (ammo_name._get() == (*itb)._get())
			{
				ci->m_select_armament = true;
				break; // for itb
			}
		}
	}//for i

}

bool CUIInventoryWnd::highlight_addons_for_weapon(PIItem weapon_item, CUICellItem* ci)
{
	PIItem item = (PIItem)ci->m_pData;
	if (!item)
	{
		return false;
	}

	CScope* pScope = smart_cast<CScope*>(item);
	if (pScope && weapon_item->CanAttach(pScope))
	{
		ci->m_select_armament = true;
		return true;
	}

	CSilencer* pSilencer = smart_cast<CSilencer*>(item);
	if (pSilencer && weapon_item->CanAttach(pSilencer))
	{
		ci->m_select_armament = true;
		return true;
	}

	CGrenadeLauncher* pGrenadeLauncher = smart_cast<CGrenadeLauncher*>(item);
	if (pGrenadeLauncher && weapon_item->CanAttach(pGrenadeLauncher))
	{
		ci->m_select_armament = true;
		return true;
	}

	CLaserDesignator* pLaser = smart_cast<CLaserDesignator*>(item);
	if (pLaser && weapon_item->CanAttach(pLaser))
	{
		ci->m_select_armament = true;
		return true;
	}

	CTacticalTorch* pTacticalTorch = smart_cast<CTacticalTorch*>(item);
	if (pTacticalTorch && weapon_item->CanAttach(pTacticalTorch))
	{
		ci->m_select_armament = true;
		return true;
	}

	return false;
}

void CUIInventoryWnd::highlight_weapons_for_addon(PIItem addon_item, CUIDragDropListEx* ddlist)
{
	VERIFY(addon_item);
	VERIFY(ddlist);

	CScope* pScope						= smart_cast<CScope*>			(addon_item);
	CSilencer* pSilencer				= smart_cast<CSilencer*>		(addon_item);
	CGrenadeLauncher* pGrenadeLauncher	= smart_cast<CGrenadeLauncher*>	(addon_item);
	CLaserDesignator* pLaser			= smart_cast<CLaserDesignator*>	(addon_item);
	CTacticalTorch* pTacticalTorch		= smart_cast<CTacticalTorch*>	(addon_item);

	if (!pScope && !pSilencer && !pGrenadeLauncher && !pLaser && !pTacticalTorch)
	{
		return;
	}

	u32 const cnt = ddlist->ItemsCount();
	for (u32 i = 0; i < cnt; ++i)
	{
		CUICellItem* ci = ddlist->GetItemIdx(i);
		PIItem item = (PIItem)ci->m_pData;
		if (!item)
		{
			continue;
		}
		CWeapon* weapon = smart_cast<CWeapon*>(item);
		if (!weapon)
		{
			continue;
		}

		if (pScope && weapon->CanAttach(pScope))
		{
			ci->m_select_armament = true;
			continue;
		}
		if (pSilencer && weapon->CanAttach(pSilencer))
		{
			ci->m_select_armament = true;
			continue;
		}
		if (pGrenadeLauncher && weapon->CanAttach(pGrenadeLauncher))
		{
			ci->m_select_armament = true;
			continue;
		}
		if (pLaser && weapon->CanAttach(pLaser))
		{
			ci->m_select_armament = true;
			continue;
		}
		if (pTacticalTorch && weapon->CanAttach(pTacticalTorch))
		{
			ci->m_select_armament = true;
			continue;
		}

	}//for i
}

bool CUIInventoryWnd::OnItemFocusReceive(CUICellItem* itm)
{
	itm->m_selected = true;
	set_highlight_item(itm);
	return true;
}

bool CUIInventoryWnd::OnItemFocusLost(CUICellItem* itm)
{
	if (itm)
	{
		itm->m_selected = false;
	}
	clear_highlight_lists();

	return true;
}

bool CUIInventoryWnd::OnItemFocusedUpdate(CUICellItem* itm)
{
	if (itm)
	{
		itm->m_selected = true;
		if (m_highlight_clear)
		{
			set_highlight_item(itm);
		}
	}

	return true;
}
