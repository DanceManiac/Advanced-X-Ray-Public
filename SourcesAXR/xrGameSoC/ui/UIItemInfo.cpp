#include "stdafx.h"

#include "UIItemInfo.h"
#include "UIStatic.h"
#include "UIXmlInit.h"

#include "UIListWnd.h"
#include "UIProgressBar.h"
#include "UIScrollView.h"

#include "../string_table.h"
#include "../Inventory_Item.h"
#include "../PhysicsShellHolder.h"
#include "../CustomOutfit.h"
#include "../Weapon.h"
#include "../WeaponMagazinedWGrenade.h"
#include "../WeaponKnife.h"
#include "../WeaponBinoculars.h"

#include "UIInventoryUtilities.h"
#include "UIWpnParams.h"
#include "UIArtefactParams.h"
#include "UIOutfitInfo.h"
#include "UIBoosterInfo.h"
#include "UIInventoryItemParams.h"

#include "AdvancedXrayGameConstants.h"
#include "eatable_item.h"
#include "../Torch.h"
//#include "../CustomDetector.h"
#include "../AnomalyDetector.h"
#include "../ArtefactContainer.h"
#include "../AntigasFilter.h"
#include "../RepairKit.h"
#include "../CustomBackpack.h"

CUIItemInfo::CUIItemInfo()
{
	UIItemImageSize.set			(0.0f,0.0f);
	UICondProgresBar			= NULL;
	UICondition					= NULL;
	UICharge					= NULL;
	UICost						= NULL;
	UIWeight					= NULL;
	UIItemImage					= NULL;
	UIDesc						= NULL;
	UIWpnParams					= NULL;
	UIArtefactParams			= NULL;
	UIOutfitItem				= NULL;
	UIBoosterInfo				= NULL;
	UIInventoryItem				= NULL;
	UIName						= NULL;
	m_pInvItem					= NULL;
	m_b_force_drawing			= false;
}

CUIItemInfo::~CUIItemInfo()
{
	xr_delete					(UIWpnParams);
	xr_delete					(UIArtefactParams);
	xr_delete					(UIOutfitItem);
	xr_delete					(UIBoosterInfo);
	xr_delete					(UIInventoryItem);
}

void CUIItemInfo::Init(LPCSTR xml_name){

	CUIXml						uiXml;
	uiXml.Load					(CONFIG_PATH, UI_PATH, xml_name);

	CUIXmlInit					xml_init;

	if (uiXml.NavigateToNode("main_frame", 0))
	{
		Frect wnd_rect{ 0.f, 0.f };
		wnd_rect.x1		= uiXml.ReadAttribFlt("main_frame", 0, "x", 0);
		wnd_rect.y1		= uiXml.ReadAttribFlt("main_frame", 0, "y", 0);

		wnd_rect.x2		= uiXml.ReadAttribFlt("main_frame", 0, "width", 0);
		wnd_rect.y2		= uiXml.ReadAttribFlt("main_frame", 0, "height", 0);
		
		inherited::Init(wnd_rect.x1, wnd_rect.y1, wnd_rect.x2, wnd_rect.y2);
	}

	if (uiXml.NavigateToNode("static_name", 0))
	{
		UIName						= xr_new<CUIStatic>();	 
		AttachChild					(UIName);		
		UIName->SetAutoDelete		(true);
		xml_init.InitStatic			(uiXml, "static_name", 0,	UIName);
	}
	if(uiXml.NavigateToNode("static_weight",0))
	{
		UIWeight				= xr_new<CUIStatic>();	 
		AttachChild				(UIWeight);		
		UIWeight->SetAutoDelete(true);
		xml_init.InitStatic		(uiXml, "static_weight", 0,			UIWeight);
	}

	if (uiXml.NavigateToNode("static_cost", 0))
	{
		UICost					= xr_new<CUIStatic>();	 
		AttachChild				(UICost);
		UICost->SetAutoDelete	(true);
		xml_init.InitStatic		(uiXml, "static_cost", 0,			UICost);
	}

	if (uiXml.NavigateToNode("static_condition", 0))
	{
		UICondition					= xr_new<CUIStatic>();	 
		AttachChild					(UICondition);
		UICondition->SetAutoDelete	(true);
		xml_init.InitStatic			(uiXml, "static_condition", 0,		UICondition);
		UICondition->Show			(false);
	}

	if (uiXml.NavigateToNode("static_charge", 0))
	{
		UICharge					= xr_new<CUIStatic>();	 
		AttachChild					(UICharge);
		UICharge->SetAutoDelete		(true);
		xml_init.InitStatic			(uiXml, "static_charge", 0, UICharge);
		UICharge->Show				(false);
	}

	if (uiXml.NavigateToNode("condition_progress", 0))
	{
		UICondProgresBar			= xr_new<CUIProgressBar>(); AttachChild(UICondProgresBar);UICondProgresBar->SetAutoDelete(true);
		xml_init.InitProgressBar	(uiXml, "condition_progress", 0, UICondProgresBar);
	}

	if (uiXml.NavigateToNode("descr_list", 0))
	{
		UIWpnParams						= xr_new<CUIWpnParams>();
		UIArtefactParams				= xr_new<CUIArtefactParams>();
		UIWpnParams->InitFromXml		(uiXml);
		UIArtefactParams->InitFromXml	(uiXml);

		UIBoosterInfo					= xr_new<CUIBoosterInfo>();
		UIBoosterInfo->InitFromXml		(uiXml);

		if (uiXml.NavigateToNode("outfit_info", 0))
		{
			UIOutfitItem = xr_new<CUIOutfitItem>();
			UIOutfitItem->InitFromXml(uiXml);
		}

		if (uiXml.NavigateToNode("inventory_items_info", 0))
		{
			UIInventoryItem = xr_new<CUIInventoryItem>();
			UIInventoryItem->InitFromXml(uiXml);
		}

		UIDesc							= xr_new<CUIScrollView>(); 
		AttachChild						(UIDesc);
		UIDesc->SetAutoDelete			(true);
		m_desc_info.bShowDescrText		= !!uiXml.ReadAttribInt("descr_list",0,"only_text_info", 1);
		xml_init.InitScrollView			(uiXml, "descr_list", 0, UIDesc);
		xml_init.InitFont				(uiXml, "descr_list:font", 0, m_desc_info.uDescClr, m_desc_info.pDescFont);
	}	

	if (uiXml.NavigateToNode("image_static", 0))
	{	
		UIItemImage					= xr_new<CUIStatic>();	 
		AttachChild					(UIItemImage);	
		UIItemImage->SetAutoDelete	(true);
		xml_init.InitStatic			(uiXml, "image_static", 0, UIItemImage);
		UIItemImage->TextureOn		();

		UIItemImage->TextureOff			();
		UIItemImage->ClipperOn			();
		UIItemImageSize.set				(UIItemImage->GetWidth(),UIItemImage->GetHeight());
	}

	xml_init.InitAutoStaticGroup	(uiXml, "auto", 0, this);
}

void CUIItemInfo::Init(float x, float y, float width, float height, LPCSTR xml_name)
{
	inherited::Init	(x, y, width, height);
    Init			(xml_name);
}

bool				IsGameTypeSingle();

void CUIItemInfo::InitItem(CInventoryItem* pInvItem)
{
	m_pInvItem				= pInvItem;
	if (!m_pInvItem)
		return;

	string256				str;
	if (UIName)
	{
		UIName->SetText		(pInvItem->Name());
	}
	if (UIWeight)
	{
		sprintf_s			(str, "%3.2f kg", pInvItem->Weight());
		UIWeight->SetText	(str);
	}
	if (UICost && IsGameTypeSingle())
	{
		sprintf_s			(str, "%d %s", pInvItem->Cost(), *CStringTable().translate("ui_st_currency"));		// will be owerwritten in multiplayer
		UICost->SetText		(str);
	}

	if (UICondProgresBar)
	{
		float cond							= 0.f;

		CTorch* flashlight					= smart_cast<CTorch*>(pInvItem);
		CDetectorAnomaly* anomaly_detector	= smart_cast<CDetectorAnomaly*>(pInvItem);
		CBattery* battery					= smart_cast<CBattery*>(pInvItem);
		CAntigasFilter* filter				= smart_cast<CAntigasFilter*>(pInvItem);
		
		if (UICharge && (GameConstants::GetTorchHasBattery() && flashlight || GameConstants::GetAnoDetectorUseBattery() && anomaly_detector || battery))
		{
			if (flashlight && GameConstants::GetTorchHasBattery())
				cond = flashlight->GetChargeToShow();
			if (anomaly_detector && GameConstants::GetAnoDetectorUseBattery())
				cond = anomaly_detector->GetChargeToShow();
			if (battery)
				cond = battery->GetCurrentChargeLevel();
			UICharge->Show(true);
			if (UICondition)
				UICondition->Show(false);
			UICondProgresBar->Show(true);
		}
		else if (pInvItem->IsUsingCondition())
		{
			cond							= pInvItem->GetConditionToShow();

			if (filter)
				cond = filter->GetFilterCondition();
			if (UICondition)
				UICondition->Show(true);
			if (UICharge)
				UICharge->Show(false);
			UICondProgresBar->Show(true);
		}
		else
		{
			if (UICharge)
				UICharge->Show(false);
			if (UICondition)
				UICondition->Show(false);
			UICondProgresBar->Show(false);
		}
		if (UICondProgresBar->IsShown())
			UICondProgresBar->SetProgressPos(cond * 100.0f + 1.0f - EPS);
	}

	if (UIDesc)
	{
		UIDesc->Clear						();
		VERIFY								(0==UIDesc->GetSize());

		TryAddWpnInfo						(*pInvItem);
		TryAddArtefactInfo					(*pInvItem);
		TryAddOutfitInfo					(*pInvItem, NULL);
		TryAddBoosterInfo					(*pInvItem);
		TryAddItemInfo						(*pInvItem);

		if(m_desc_info.bShowDescrText)
		{
			CUIStatic* pItem					= xr_new<CUIStatic>();
			pItem->SetTextColor					(m_desc_info.uDescClr);
			pItem->SetFont						(m_desc_info.pDescFont);
			pItem->SetWidth						(UIDesc->GetDesiredChildWidth());
			pItem->SetTextComplexMode			(true);
			pItem->SetText						(*pInvItem->ItemDescription());
			pItem->AdjustHeightToText			();
			UIDesc->AddWindow					(pItem, true);
			TryAddWpnAmmoInfo					(*pInvItem);
		}
		UIDesc->ScrollToBegin				();
	}
	if (UIItemImage)
	{
		// Загружаем картинку
		UIItemImage->SetShader				(InventoryUtilities::GetEquipmentIconsShader());

		int iGridWidth						= pInvItem->GetGridWidth();
		int iGridHeight						= pInvItem->GetGridHeight();
		int iXPos							= pInvItem->GetXPos();
		int iYPos							= pInvItem->GetYPos();

		UIItemImage->GetUIStaticItem().SetOriginalRect(	float(iXPos * UI().inv_grid_kx()), float(iYPos * UI().inv_grid_kx()),
														float(iGridWidth * UI().inv_grid_kx()),	float(iGridHeight * UI().inv_grid_kx()));

		UIItemImage->TextureOn				();
		UIItemImage->ClipperOn				();
		UIItemImage->SetStretchTexture		(true);
		
		Frect v_r{};

		v_r = { 0.0f,
												0.0f,
												float(iGridWidth * UI().inv_grid_40()),
												float(iGridHeight * UI().inv_grid_40()) };

		if(UI().is_widescreen())
			v_r.x2 /= 1.328f;

		UIItemImage->GetUIStaticItem().SetRect	(v_r);
		UIItemImage->SetWidth					(_min(v_r.width(),	UIItemImageSize.x));
		UIItemImage->SetHeight					(_min(v_r.height(),	UIItemImageSize.y));
	}
}

void CUIItemInfo::TryAddWpnAmmoInfo(CInventoryItem& pInvItem)
{
	if (!GameConstants::GetAutoAmmoInfo())
		return;
	CWeapon* pWeapon = smart_cast<CWeapon*>(&pInvItem);
	CWeaponMagazinedWGrenade* pWeaponMagGren = smart_cast<CWeaponMagazinedWGrenade*>(&pInvItem);
	CWeaponKnife* pKnife = smart_cast<CWeaponKnife*>(&pInvItem);
	CWeaponBinoculars* pBinoc = smart_cast<CWeaponBinoculars*>(&pInvItem);
	if (!pWeapon || pKnife || pBinoc)
		return;
	if (UIWpnParams && !UIWpnParams->Check(pInvItem))
		return;
	if (!!READ_IF_EXISTS(pSettings, r_bool, pWeapon->cNameSect().c_str(), "show_ammo", true) == false)
		return;
	bool use_desc = GameConstants::GetAutoAmmoInfoDesc();
	LPCSTR name_or_desc = "inv_name_short";
	if (use_desc)
		name_or_desc = "inv_name_ammo_descr";
	CUIStatic* m_text = nullptr;
	m_text = xr_new<CUIStatic>();
	m_text->Init(0.f, 0.f, UIDesc->GetWidth(), 0.f);
	m_text->SetTextColor(m_desc_info.uDescClr);
	m_text->SetFont(m_desc_info.pDescFont);
	string512 ammo_list = "";
	string512 grenades_list = "";
	for (u32 i = 0; i < pWeapon->m_ammoTypes.size(); i++)
	{
		if (!!pSettings->line_exist(pWeapon->m_ammoTypes[i].c_str(), name_or_desc) == false)
		{
			Msg("Can't find [%s] in ammo section [%s], reading [inv_name_short]", pWeapon->m_ammoTypes[i].c_str(), name_or_desc);
			name_or_desc = "inv_name_short";
		}
		if (i != 0)
		{
			if (i == pWeapon->m_ammoTypes.size())
				xr_sprintf(ammo_list, "%s%s%s", ammo_list, *CStringTable().translate("st_bullet_symbol"), CStringTable().translate(pSettings->r_string(pWeapon->m_ammoTypes[i].c_str(), name_or_desc)).c_str());
			else
				xr_sprintf(ammo_list, "%s%s %s %s", ammo_list, *CStringTable().translate("st_line_break"), *CStringTable().translate("st_bullet_symbol"), CStringTable().translate(pSettings->r_string(pWeapon->m_ammoTypes[i].c_str(), name_or_desc)).c_str());
		}
		else
		{
			xr_sprintf(ammo_list, "%s %s", *CStringTable().translate("st_bullet_symbol"), CStringTable().translate(pSettings->r_string(pWeapon->m_ammoTypes.front().c_str(), name_or_desc)).c_str());
		}
	}
	bool need_add_grenades = pWeaponMagGren && (pWeaponMagGren->GrenadeLauncherAttachable() || pWeaponMagGren->IsGrenadeLauncherAttached());
	if (need_add_grenades)
	{
		name_or_desc = "inv_name_short";
		for (u32 i1 = 0; i1 < pWeaponMagGren->m_ammoTypes2.size(); i1++)
		{
			if (i1 != 0)
			{
				if (i1 == pWeaponMagGren->m_ammoTypes2.size())
					xr_sprintf(grenades_list, "%s%s%s", grenades_list, *CStringTable().translate("st_bullet_symbol"), CStringTable().translate(pSettings->r_string(pWeaponMagGren->m_ammoTypes2[i1].c_str(), name_or_desc)).c_str());
				else
					xr_sprintf(grenades_list, "%s%s %s %s", grenades_list, *CStringTable().translate("st_line_break"), *CStringTable().translate("st_bullet_symbol"), CStringTable().translate(pSettings->r_string(pWeaponMagGren->m_ammoTypes2[i1].c_str(), name_or_desc)).c_str());
			}
			else
			{
				xr_sprintf(grenades_list, "%s %s", *CStringTable().translate("st_bullet_symbol"), CStringTable().translate(pSettings->r_string(pWeaponMagGren->m_ammoTypes2.front().c_str(), name_or_desc)).c_str());
			}
		}
	}
	string4096 out_text = "";
	if (need_add_grenades)
	{
		xr_sprintf(out_text, " %s %s %s %s", *CStringTable().translate("st_ammos"), ammo_list, *CStringTable().translate("st_for_gl"), grenades_list);
	}
	else
	{
		xr_sprintf(out_text, " %s %s", *CStringTable().translate("st_ammos"), ammo_list);
	}
	m_text->SetText(out_text);
	m_text->SetTextComplexMode(true);
	m_text->AdjustHeightToText();
	UIDesc->AddWindow(m_text, false);
}

void CUIItemInfo::TryAddWpnInfo(CInventoryItem& pInvItem)
{
	if (UIWpnParams->Check(pInvItem) && GameConstants::GetShowWpnInfo())
	{
		UIWpnParams->SetInfo(pInvItem);
		UIDesc->AddWindow(UIWpnParams, false);
	}
}

void CUIItemInfo::TryAddArtefactInfo(CInventoryItem& pInvItem)
{
	if (UIArtefactParams->Check(pInvItem.object().cNameSect()) && UIArtefactParams->CheckDescrInfoPortions(pInvItem.object().cNameSect()))
	{
		UIArtefactParams->SetInfo(pInvItem);
		UIDesc->AddWindow( UIArtefactParams, false );
	}
}

void CUIItemInfo::TryAddOutfitInfo(CInventoryItem& pInvItem, CInventoryItem* pCompareItem)
{
	CCustomOutfit* outfit = smart_cast<CCustomOutfit*>(&pInvItem);

	if (outfit && UIOutfitItem)
	{
		CCustomOutfit* comp_outfit = smart_cast<CCustomOutfit*>(pCompareItem);
		UIOutfitItem->SetInfo(outfit, comp_outfit);
		UIDesc->AddWindow(UIOutfitItem, false);
	}
}

void CUIItemInfo::TryAddBoosterInfo(CInventoryItem& pInvItem)
{
	CEatableItem* food = smart_cast<CEatableItem*>(&pInvItem);
	if (food && UIBoosterInfo)
	{
		UIBoosterInfo->SetInfo(pInvItem);
		UIDesc->AddWindow(UIBoosterInfo, false);
	}
}

void CUIItemInfo::ResetInventoryItem()
{
	if (UIInventoryItem)
		UIInventoryItem->SetHeight(0);
}

void CUIItemInfo::TryAddItemInfo(CInventoryItem& pInvItem)
{
	CTorch* pTorch = smart_cast<CTorch*>(&pInvItem);
	//CCustomDetector* pArtefact_detector = smart_cast<CCustomDetector*>(&pInvItem);
	CDetectorAnomaly* pAnomaly_detector = smart_cast<CDetectorAnomaly*>(&pInvItem);
	CArtefactContainer* pAf_container = smart_cast<CArtefactContainer*>(&pInvItem);
	CCustomBackpack* pBackpack = smart_cast<CCustomBackpack*>(&pInvItem);
	CBattery* pBattery = smart_cast<CBattery*>(&pInvItem);
	CAntigasFilter* pFilter = smart_cast<CAntigasFilter*>(&pInvItem);
	CRepairKit* pKit = smart_cast<CRepairKit*>(&pInvItem);

	bool ShowChargeTorch = GameConstants::GetTorchHasBattery();
	//bool ShowChargeDetector = GameConstants::GetArtDetectorUseBattery();
	bool ShowChargeAnoDetector = GameConstants::GetAnoDetectorUseBattery();

	if ((pTorch && ShowChargeTorch || /*pArtefact_detector && ShowChargeDetector ||*/ pAnomaly_detector && ShowChargeAnoDetector || pAf_container || pBackpack || pBattery || pFilter || pKit) && UIInventoryItem)
	{
		UIInventoryItem->SetInfo(pInvItem);
		UIDesc->AddWindow(UIInventoryItem, false);
	}
}

void CUIItemInfo::Draw()
{
	if (m_pInvItem || m_b_force_drawing)
		inherited::Draw();
}
