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

#include "UIInventoryUtilities.h"
#include "UIWpnParams.h"
#include "UIArtefactParams.h"
#include "UIOutfitInfo.h"

#include "AdvancedXrayGameConstants.h"

CUIItemInfo::CUIItemInfo()
{
	UIItemImageSize.set			(0.0f,0.0f);
	UICondProgresBar			= NULL;
	UICondition					= NULL;
	UICost						= NULL;
	UIWeight					= NULL;
	UIItemImage					= NULL;
	UIDesc						= NULL;
	UIWpnParams					= NULL;
	UIArtefactParams			= NULL;
	UIOutfitItem				= NULL;
	UIName						= NULL;
	m_pInvItem					= NULL;
	m_b_force_drawing			= false;
}

CUIItemInfo::~CUIItemInfo()
{
	xr_delete					(UIWpnParams);
	xr_delete					(UIArtefactParams);
	xr_delete					(UIOutfitItem);
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
		if (uiXml.NavigateToNode("outfit_info", 0))
		{
			UIOutfitItem = xr_new<CUIOutfitItem>();
			UIOutfitItem->InitFromXml(uiXml);
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
		sprintf_s			(str, "%d RU", pInvItem->Cost());		// will be owerwritten in multiplayer
		UICost->SetText		(str);
	}

	if (UICondProgresBar)
	{
		float cond							= pInvItem->GetConditionToShow();
		UICondProgresBar->Show				(true);
		UICondProgresBar->SetProgressPos	( cond*100.0f+1.0f-EPS );
	}

	if (UIDesc)
	{
		UIDesc->Clear						();
		VERIFY								(0==UIDesc->GetSize());
		TryAddWpnInfo						(pInvItem->object().cNameSect());
		TryAddArtefactInfo					(*pInvItem);
		TryAddOutfitInfo					(*pInvItem, NULL);
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

		UIItemImage->GetUIStaticItem().SetOriginalRect(	float(iXPos*INV_GRID_WIDTH), float(iYPos*INV_GRID_HEIGHT),
														float(iGridWidth*INV_GRID_WIDTH),	float(iGridHeight*INV_GRID_HEIGHT));
		UIItemImage->TextureOn				();
		UIItemImage->ClipperOn				();
		UIItemImage->SetStretchTexture		(true);
		Frect v_r							= {	0.0f, 
												0.0f, 
												float(iGridWidth*INV_GRID_WIDTH),	
												float(iGridHeight*INV_GRID_HEIGHT)};
		if(UI().is_widescreen())
			v_r.x2 /= 1.328f;

		UIItemImage->GetUIStaticItem().SetRect	(v_r);
		UIItemImage->SetWidth					(_min(v_r.width(),	UIItemImageSize.x));
		UIItemImage->SetHeight					(_min(v_r.height(),	UIItemImageSize.y));
	}
}

void CUIItemInfo::TryAddWpnInfo(const shared_str& wpn_section)
{
	if (UIWpnParams->Check(wpn_section) && GameConstants::GetShowWpnInfo())
	{
		UIWpnParams->SetInfo(wpn_section);
		UIDesc->AddWindow(UIWpnParams,false);
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

void CUIItemInfo::Draw()
{
	if (m_pInvItem || m_b_force_drawing)
		inherited::Draw();
}
