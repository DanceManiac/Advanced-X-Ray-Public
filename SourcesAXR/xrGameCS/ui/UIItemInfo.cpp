//#include "stdafx.h"
#include "pch_script.h"

#include "uiiteminfo.h"
#include "uistatic.h"
#include "UIXmlInit.h"

#include "UIListWnd.h"
#include "UIProgressBar.h"
#include "UIScrollView.h"
#include "UIFrameWindow.h"

#include "ai_space.h"
#include "alife_simulator.h"
#include "../string_table.h"
#include "../Inventory_Item.h"
#include "UIInventoryUtilities.h"
#include "../PhysicsShellHolder.h"
#include "UIWpnParams.h"
#include "UIArtefactParams.h"
#include "UICellItem.h"
#include "UIInvUpgradeProperty.h"
#include "UIOutfitInfo.h"
#include "../Weapon.h"
#include "../CustomOutfit.h"
#include "UIInventoryItemParams.h"
#include "../AdvancedXrayGameConstants.h"
#include "../Torch.h"
#include "../CustomDetector.h"
#include "../AnomalyDetector.h"
#include "UIBoosterInfo.h"

extern const LPCSTR g_inventory_upgrade_xml;

#define INV_GRID_WIDTH2(HQ_ICONS) ((HQ_ICONS) ? (80.0f) : (40.0f))
#define INV_GRID_HEIGHT2(HQ_ICONS) ((HQ_ICONS) ? (80.0f) : (40.0f))

CUIItemInfo::CUIItemInfo()
{
	UIItemImageSize.set			(0.0f,0.0f);
	
	UICost						= NULL;
	UITradeTip					= NULL;
	UIWeight					= NULL;
	UIItemImage					= NULL;
	UIDesc						= NULL;
	UIConditionWnd				= NULL;
	UIWpnParams					= NULL;
	UIProperties				= NULL;
	UIOutfitInfo				= NULL;
	UIArtefactParams			= NULL;
	UIInventoryItem				= NULL;
	UIBoosterInfo				= NULL;
	UIName						= NULL;
	UIBackground				= NULL;
	m_pInvItem					= NULL;
	UIChargeConditionParams		= NULL;
	m_b_FitToHeight				= false;
	m_complex_desc				= false;
}

CUIItemInfo::~CUIItemInfo()
{
	xr_delete	(UIConditionWnd);
	xr_delete	(UIWpnParams);
	xr_delete	(UIArtefactParams);
	xr_delete	(UIProperties);
	xr_delete	(UIOutfitInfo);
	xr_delete	(UIInventoryItem);
	xr_delete	(UIChargeConditionParams);
	xr_delete	(UIBoosterInfo);
}

void CUIItemInfo::InitItemInfo(LPCSTR xml_name)
{
	CUIXml						uiXml;
	uiXml.Load					(CONFIG_PATH, UI_PATH, xml_name);
	CUIXmlInit					xml_init;

	if(uiXml.NavigateToNode("main_frame",0))
	{
		Frect wnd_rect;
		wnd_rect.x1		= uiXml.ReadAttribFlt("main_frame", 0, "x", 0);
		wnd_rect.y1		= uiXml.ReadAttribFlt("main_frame", 0, "y", 0);

		wnd_rect.x2		= uiXml.ReadAttribFlt("main_frame", 0, "width", 0);
		wnd_rect.y2		= uiXml.ReadAttribFlt("main_frame", 0, "height", 0);
		wnd_rect.x2		+= wnd_rect.x1;
		wnd_rect.y2		+= wnd_rect.y1;
		inherited::SetWndRect(wnd_rect);
		
		delay			= uiXml.ReadAttribInt("main_frame", 0, "delay", 500);
	}
	if(uiXml.NavigateToNode("background_frame",0))
	{
		UIBackground				= xr_new<CUIFrameWindow>();
		UIBackground->SetAutoDelete	(true);
		AttachChild					(UIBackground);
		xml_init.InitFrameWindow	(uiXml, "background_frame", 0,	UIBackground);
	}
	m_complex_desc = false;
	if(uiXml.NavigateToNode("static_name",0))
	{
		UIName						= xr_new<CUIStatic>();	 
		AttachChild					(UIName);		
		UIName->SetAutoDelete		(true);
		xml_init.InitStatic			(uiXml, "static_name", 0,	UIName);
		m_complex_desc				= ( uiXml.ReadAttribInt("static_name", 0, "complex_desc", 0) == 1 );
	}
	if(uiXml.NavigateToNode("static_weight",0))
	{
		UIWeight				= xr_new<CUIStatic>();	 
		AttachChild				(UIWeight);		
		UIWeight->SetAutoDelete(true);
		xml_init.InitStatic		(uiXml, "static_weight", 0,			UIWeight);
	}

	if(uiXml.NavigateToNode("static_cost",0))
	{
		UICost					= xr_new<CUIStatic>();	 
		AttachChild				(UICost);
		UICost->SetAutoDelete	(true);
		xml_init.InitStatic		(uiXml, "static_cost", 0,			UICost);
	}

	if(uiXml.NavigateToNode("static_no_trade",0))
	{
		UITradeTip					= xr_new<CUIStatic>();
		AttachChild					(UITradeTip);
		UITradeTip->SetAutoDelete	(true);
		xml_init.InitStatic			(uiXml, "static_no_trade", 0,		UITradeTip);
	}

	if(uiXml.NavigateToNode("descr_list",0))
	{
		UIConditionWnd					= xr_new<CUIConditionParams>();
		UIConditionWnd->InitFromXml		(uiXml);
		UIChargeConditionParams			= xr_new<CUIItemConditionParams>();
		UIChargeConditionParams->InitFromXml(uiXml);
		UIWpnParams						= xr_new<CUIWpnParams>();
		UIWpnParams->InitFromXml		(uiXml);
		UIArtefactParams				= xr_new<CUIArtefactParams>();
		UIArtefactParams->InitFromXml	(uiXml);
		UIBoosterInfo					= xr_new<CUIBoosterInfo>();
		UIBoosterInfo->InitFromXml		(uiXml);

		if ( ai().get_alife() ) // (-designer)
		{
			UIProperties					= xr_new<UIInvUpgPropertiesWnd>();
			UIProperties->init_from_xml		("actor_menu_item.xml");
		}

		UIDesc							= xr_new<CUIScrollView>(); 
		AttachChild						(UIDesc);		
		UIDesc->SetAutoDelete			(true);
		m_desc_info.bShowDescrText		= !!uiXml.ReadAttribInt("descr_list",0,"only_text_info", 1);
		m_b_FitToHeight					= !!uiXml.ReadAttribInt("descr_list",0,"fit_to_height", 0);
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
	
	if ( uiXml.NavigateToNode( "outfit_info", 0 ) )
	{
		UIOutfitInfo			= xr_new<CUIOutfitInfo>();
		UIOutfitInfo->InitFromXml( uiXml );
	}

	if (uiXml.NavigateToNode("inventory_items_info", 0))
	{
		UIInventoryItem = xr_new<CUIInventoryItem>();
		UIInventoryItem->InitFromXml(uiXml);
	}

	xml_init.InitAutoStaticGroup	(uiXml, "auto", 0, this);
}

void CUIItemInfo::InitItemInfo(Fvector2 pos, Fvector2 size, LPCSTR xml_name)
{
	inherited::SetWndPos	(pos);
	inherited::SetWndSize	(size);
    InitItemInfo			(xml_name);
}

bool	IsGameTypeSingle();

void CUIItemInfo::InitItem(CUICellItem* pCellItem, CInventoryItem* pCompareItem, u32 item_price, LPCSTR trade_tip)
{
	if(!pCellItem)
	{
		m_pInvItem			= NULL;
		Enable				(false);
		return;
	}

	PIItem pInvItem			= (PIItem)pCellItem->m_pData;
	m_pInvItem				= pInvItem;
	Enable					(NULL != m_pInvItem);
	if(!m_pInvItem)			return;

	Fvector2				pos;	pos.set( 0.0f, 0.0f );
	string256				str;
	if ( UIName )
	{
		UIName->SetText		(pInvItem->NameItem());
		UIName->AdjustHeightToText();
		pos.y = UIName->GetWndPos().y + UIName->GetHeight() + 4.0f;
	}
	if ( UIWeight )
	{
		LPCSTR  kg_str = CStringTable().translate( "st_kg" ).c_str();
		float	weight = pInvItem->Weight();
		
		if ( !weight )
		{
			if ( CWeaponAmmo* ammo = dynamic_cast<CWeaponAmmo*>(pInvItem) )
			{
				// its helper item, m_boxCur is zero, so recalculate via CInventoryItem::Weight()
				weight = pInvItem->CInventoryItem::Weight();
				for( u32 j = 0; j < pCellItem->ChildsCount(); ++j )
				{
					PIItem jitem	= (PIItem)pCellItem->Child(j)->m_pData;
					weight			+= jitem->CInventoryItem::Weight();
				}
			}
		}

		xr_sprintf			(str, "%3.2f %s", weight, kg_str );
		UIWeight->SetText	(str);
		
		pos.x = UIWeight->GetWndPos().x;
		if ( m_complex_desc )
		{
			UIWeight->SetWndPos	(pos);
		}
	}
	if ( UICost && IsGameTypeSingle() )
	{
		if (item_price != u32(-1))
		{
			xr_sprintf(str, "%d RU", item_price);// will be owerwritten in multiplayer
			UICost->SetText(str);
			UICost->Show(true);
		}
		else if (item_price == u32(-1))
		{
			xr_sprintf(str, "%d RU", pInvItem->Cost());// will be owerwritten in multiplayer
			UICost->SetText(str);
			UICost->Show(true);
		}
	}
	else
		UICost->Show(false);
	
//	CActor* actor = smart_cast<CActor*>( Level().CurrentViewEntity() );
//	if ( g_pGameLevel && Level().game && actor )
//	{
//		game_cl_Deathmatch* gs_mp = smart_cast<game_cl_Deathmatch*>( Game() );
//		IBuyWnd* buy_menu = gs_mp->pCurBuyMenu->GetItemPrice();
//		GetItemPrice();
//	}
	
	if ( UITradeTip && IsGameTypeSingle())
	{
		pos.y = UITradeTip->GetWndPos().y;
		if ( UIWeight && m_complex_desc )
		{
			pos.y = UIWeight->GetWndPos().y + UIWeight->GetHeight() + 4.0f;
		}

		if(trade_tip==NULL)
			UITradeTip->Show(false);
		else
		{
			UITradeTip->SetText(CStringTable().translate(trade_tip).c_str());
			UITradeTip->AdjustHeightToText();
			UITradeTip->SetWndPos(pos);
			UITradeTip->Show(true);
		}
	}
	
	if ( UIDesc )
	{
		pos.y = UIDesc->GetWndPos().y;
		if ( UIWeight )
			pos.y = UIWeight->GetWndPos().y + UIWeight->GetHeight() + 4.0f;

		if(UITradeTip && trade_tip!=NULL)
			pos.y = UITradeTip->GetWndPos().y + UITradeTip->GetHeight() + 4.0f;

		pos.x					= UIDesc->GetWndPos().x;
		UIDesc->SetWndPos		(pos);
		UIDesc->Clear			();
		VERIFY					(0==UIDesc->GetSize());
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
		TryAddConditionInfo					(*pInvItem, pCompareItem);
		TryAddWpnInfo						(*pInvItem, pCompareItem);
		TryAddArtefactInfo					(*pInvItem);
		TryAddOutfitInfo					(*pInvItem, pCompareItem);
		TryAddUpgradeInfo					(*pInvItem);
		TryAddItemInfo						(*pInvItem);
		TryAddBoosterInfo					(*pInvItem);

		if(m_b_FitToHeight)
		{
			UIDesc->SetWndSize				(Fvector2().set(UIDesc->GetWndSize().x, UIDesc->GetPadSize().y) );
			Fvector2 new_size;
			new_size.x						= GetWndSize().x;
			new_size.y						= UIDesc->GetWndPos().y+UIDesc->GetWndSize().y+20.0f;
			new_size.x						= _max(105.0f, new_size.x);
			new_size.y						= _max(105.0f, new_size.y);
			SetWndSize						(new_size);
			if(UIBackground)
				UIBackground->InitFrameWindow(UIBackground->GetWndPos(), new_size);
		}

		UIDesc->ScrollToBegin				();
	}
	if(UIItemImage)
	{
		// Загружаем картинку
		UIItemImage->SetShader				(InventoryUtilities::GetEquipmentIconsShader());

		Irect item_grid_rect				= pInvItem->GetInvGridRect();
		UIItemImage->GetUIStaticItem().SetOriginalRect(	float(item_grid_rect.x1*INV_GRID_WIDTH(GameConstants::GetUseHQ_Icons())), float(item_grid_rect.y1*INV_GRID_HEIGHT(GameConstants::GetUseHQ_Icons())),
														float(item_grid_rect.x2*INV_GRID_WIDTH(GameConstants::GetUseHQ_Icons())),	float(item_grid_rect.y2*INV_GRID_HEIGHT(GameConstants::GetUseHQ_Icons())));
		UIItemImage->TextureOn				();
		UIItemImage->ClipperOn				();
		UIItemImage->SetStretchTexture		(true);
		Frect v_r{};
			
		if (GameConstants::GetUseHQ_Icons())
		{
			v_r = { 0.0f,
				0.0f,
				float(item_grid_rect.x2 * INV_GRID_WIDTH2(GameConstants::GetUseHQ_Icons()) / 2),
				float(item_grid_rect.y2 * INV_GRID_HEIGHT2(GameConstants::GetUseHQ_Icons()) / 2) };
		}
		else
		{
				v_r = { 0.0f,
				0.0f,
				float(item_grid_rect.x2 * INV_GRID_WIDTH2(GameConstants::GetUseHQ_Icons())),
				float(item_grid_rect.y2 * INV_GRID_HEIGHT2(GameConstants::GetUseHQ_Icons())) };
		}
		if(UI()->is_16_9_mode())
			v_r.x2 /= 1.2f;

		UIItemImage->GetUIStaticItem().SetRect	(v_r);
//		UIItemImage->SetWidth					(_min(v_r.width(),	UIItemImageSize.x));
//		UIItemImage->SetHeight					(_min(v_r.height(),	UIItemImageSize.y));
		UIItemImage->SetWidth					( v_r.width()  );
		UIItemImage->SetHeight					( v_r.height() );
	}
}

void CUIItemInfo::InitItemUpgrade(CInventoryItem* pInvItem)
{
	m_pInvItem				= pInvItem;
	Enable					(NULL != m_pInvItem);
	if(!m_pInvItem)			return;

	Fvector2				pos;	pos.set( 0.0f, 0.0f );
	string256				str;
	if ( UIName )
	{
		UIName->SetText		(pInvItem->NameItem());
		UIName->AdjustHeightToText();
	}
	if ( UIWeight )
	{
		LPCSTR  kg_str = CStringTable().translate( "st_kg" ).c_str();
		float	weight = pInvItem->Weight();

		xr_sprintf			(str, "%3.2f %s", weight, kg_str );
		UIWeight->SetText	(str);
	}
	if ( UICost && IsGameTypeSingle() )
	{
		xr_sprintf(str, "%d RU", pInvItem->Cost());// will be owerwritten in multiplayer
		UICost->SetText(str);
		UICost->Show(true);
	
	}
	
	if ( UIDesc )
	{

		pos.x					= UIDesc->GetWndPos().x;
		UIDesc->SetWndPos		(pos);
		UIDesc->Clear			();
		VERIFY					(0==UIDesc->GetSize());
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
		TryAddConditionInfo					(*pInvItem, NULL);
		TryAddWpnInfo						(*pInvItem, NULL);
		TryAddArtefactInfo					(*pInvItem);
		TryAddOutfitInfo					(*pInvItem, NULL);
		TryAddUpgradeInfo					(*pInvItem);
		TryAddBoosterInfo					(*pInvItem);

		if(m_b_FitToHeight)
		{
			UIDesc->SetWndSize				(Fvector2().set(UIDesc->GetWndSize().x, UIDesc->GetPadSize().y) );
			Fvector2 new_size;
			new_size.x						= GetWndSize().x;
			new_size.y						= UIDesc->GetWndPos().y+UIDesc->GetWndSize().y+20.0f;
			new_size.x						= _max(105.0f, new_size.x);
			new_size.y						= _max(105.0f, new_size.y);
			SetWndSize						(new_size);
			if(UIBackground)
				UIBackground->InitFrameWindow(UIBackground->GetWndPos(), new_size);
		}

		UIDesc->ScrollToBegin				();
	}
	if(UIItemImage)
	{
		// Загружаем картинку
		UIItemImage->SetShader				(InventoryUtilities::GetEquipmentIconsShader());

		Irect item_grid_rect				= pInvItem->GetInvGridRect();
		UIItemImage->GetUIStaticItem().SetOriginalRect(	float(item_grid_rect.x1*INV_GRID_WIDTH(GameConstants::GetUseHQ_Icons())), float(item_grid_rect.y1*INV_GRID_HEIGHT(GameConstants::GetUseHQ_Icons())),
														float(item_grid_rect.x2*INV_GRID_WIDTH(GameConstants::GetUseHQ_Icons())),	float(item_grid_rect.y2*INV_GRID_HEIGHT(GameConstants::GetUseHQ_Icons())));
		UIItemImage->TextureOn				();
		UIItemImage->ClipperOn				();
		UIItemImage->SetStretchTexture		(true);
		Frect v_r{};
			
		if (GameConstants::GetUseHQ_Icons())
		{
			v_r = { 0.0f,
				0.0f,
				float(item_grid_rect.x2 * INV_GRID_WIDTH2(GameConstants::GetUseHQ_Icons()) / 2),
				float(item_grid_rect.y2 * INV_GRID_HEIGHT2(GameConstants::GetUseHQ_Icons()) / 2) };
		}
		else
		{
				v_r = { 0.0f,
				0.0f,
				float(item_grid_rect.x2 * INV_GRID_WIDTH2(GameConstants::GetUseHQ_Icons())),
				float(item_grid_rect.y2 * INV_GRID_HEIGHT2(GameConstants::GetUseHQ_Icons())) };
		}
		if(UI()->is_16_9_mode())
			v_r.x2 /= 1.2f;

		UIItemImage->GetUIStaticItem().SetRect	(v_r);
//		UIItemImage->SetWidth					(_min(v_r.width(),	UIItemImageSize.x));
//		UIItemImage->SetHeight					(_min(v_r.height(),	UIItemImageSize.y));
		UIItemImage->SetWidth					( v_r.width()  );
		UIItemImage->SetHeight					( v_r.height() );
	}
}

void CUIItemInfo::TryAddConditionInfo( CInventoryItem& pInvItem, CInventoryItem* pCompareItem )
{
	CTorch*			torch = smart_cast<CTorch*>(&pInvItem);
	CCustomDetector* artefact_detector = smart_cast<CCustomDetector*>(&pInvItem);
	CDetectorAnomaly* anomaly_detector = smart_cast<CDetectorAnomaly*>(&pInvItem);

	bool ShowCharge = GameConstants::GetTorchHasBattery() || GameConstants::GetArtDetectorUseBattery() || GameConstants::GetAnoDetectorUseBattery();

	if ( pInvItem.IsUsingCondition() )
	{
		UIConditionWnd->SetInfo( pCompareItem, pInvItem );
		UIDesc->AddWindow( UIConditionWnd, false );
	}

	if ((torch || artefact_detector || anomaly_detector) && ShowCharge)
	{
		UIChargeConditionParams->SetInfo(pCompareItem, pInvItem);
		UIDesc->AddWindow(UIChargeConditionParams, false);
	}
}

void CUIItemInfo::TryAddWpnInfo( CInventoryItem& pInvItem, CInventoryItem* pCompareItem )
{
	if (UIWpnParams->Check( pInvItem) && GameConstants::GetShowWpnInfo())
	{
		UIWpnParams->SetInfo( pCompareItem, pInvItem );
		UIDesc->AddWindow( UIWpnParams, false );
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

void CUIItemInfo::TryAddOutfitInfo( CInventoryItem& pInvItem, CInventoryItem* pCompareItem )
{
	CCustomOutfit* outfit = smart_cast<CCustomOutfit*>(&pInvItem);
	if ( outfit && UIOutfitInfo )
	{
		CCustomOutfit* comp_outfit = smart_cast<CCustomOutfit*>(pCompareItem);
		UIOutfitInfo->UpdateInfo( outfit, comp_outfit );
		UIDesc->AddWindow( UIOutfitInfo, false );
	}
}

void CUIItemInfo::TryAddUpgradeInfo( CInventoryItem& pInvItem )
{
	if ( pInvItem.upgardes().size() && UIProperties )
	{
		UIProperties->set_item_info( pInvItem );
		UIDesc->AddWindow( UIProperties, false );
	}
}

void CUIItemInfo::TryAddItemInfo(CInventoryItem& pInvItem)
{
	CInventoryItemObject* item = smart_cast<CInventoryItemObject*>(&pInvItem);
	CHudItemObject* hud_item = smart_cast<CHudItemObject*>(&pInvItem);

	if ((item || hud_item) && UIInventoryItem)
	{
		UIInventoryItem->SetInfo(pInvItem.object().cNameSect());
		UIDesc->AddWindow(UIInventoryItem, false);
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

void CUIItemInfo::Draw()
{
	if(m_pInvItem)
		inherited::Draw();
}
