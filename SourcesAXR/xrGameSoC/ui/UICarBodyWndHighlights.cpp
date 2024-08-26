#include "stdafx.h"
#include "UICellItem.h"
#include "UIHelper.h"
#include "UICarBodyWnd.h"
#include "UIDragDropListEx.h"

#include "../Artefact.h"
#include "../CustomBackpack.h"
#include "../AnomalyDetector.h"
#include "../CustomOutfit.h"
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

void CUICarBodyWnd::InitHighlights(CUIXml& uiXml)
{
	m_highlight_clear = true;
}

void CUICarBodyWnd::clear_highlight_lists()
{
	m_pUIOurBagList->clear_select_armament();
	m_pUIOthersBagList->clear_select_armament();
	m_highlight_clear = true;
}

void CUICarBodyWnd::set_highlight_item(CUICellItem* cell_item)
{
	PIItem item = (PIItem)cell_item->m_pData;
	if (!item)
	{
		return;
	}

	highlight_armament(item, m_pUIOurBagList);
	highlight_armament(item, m_pUIOthersBagList);
	m_highlight_clear = false;
}

void CUICarBodyWnd::highlight_armament(PIItem item, CUIDragDropListEx* ddlist)
{
	ddlist->clear_select_armament();
	highlight_ammo_for_weapon(item, ddlist);
	highlight_weapons_for_ammo(item, ddlist);
	highlight_weapons_for_addon(item, ddlist);
}

void CUICarBodyWnd::highlight_ammo_for_weapon(PIItem weapon_item, CUIDragDropListEx* ddlist)
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

void CUICarBodyWnd::highlight_weapons_for_ammo(PIItem ammo_item, CUIDragDropListEx* ddlist)
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

bool CUICarBodyWnd::highlight_addons_for_weapon(PIItem weapon_item, CUICellItem* ci)
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

void CUICarBodyWnd::highlight_weapons_for_addon(PIItem addon_item, CUIDragDropListEx* ddlist)
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

bool CUICarBodyWnd::OnItemFocusReceive(CUICellItem* itm)
{
	itm->m_selected = true;
	set_highlight_item(itm);
	return true;
}

bool CUICarBodyWnd::OnItemFocusLost(CUICellItem* itm)
{
	if (itm)
	{
		itm->m_selected = false;
	}
	clear_highlight_lists();

	return true;
}

bool CUICarBodyWnd::OnItemFocusedUpdate(CUICellItem* itm)
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
