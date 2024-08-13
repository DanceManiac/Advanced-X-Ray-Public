#include "stdafx.h"
#include "UITradeWnd.h"

#include "xrUIXmlParser.h"
#include "UIXmlInit.h"
#include "UIStatic.h"
#include "UIItemInfo.h"
#include "UIWindow.h"

#include "../Entity.h"
#include "../HUDManager.h"
#include "../WeaponAmmo.h"
#include "../Actor.h"
#include "../Trade.h"
#include "../UIGameSP.h"
#include "UIInventoryUtilities.h"
#include "../inventoryowner.h"
#include "../eatable_item.h"
#include "../inventory.h"
#include "../level.h"
#include "../string_table.h"
#include "character_info.h"
#include "UIMultiTextStatic.h"
#include "UI3tButton.h"
#include "UIItemInfo.h"

#include "UICharacterInfo.h"
#include "UIDragDropListEx.h"
#include "UICellItem.h"
#include "UICellItemFactory.h"

#include "AdvancedXrayGameConstants.h"
#include "../../xrEngine/x_ray.h"

#define				TRADE_XML			"trade.xml"
#define				TRADE_CHARACTER_XML	"trade_character.xml"
#define				TRADE_ITEM_XML		"trade_item.xml"

extern bool SSFX_UI_DoF_active;

CUITradeWnd::CUITradeWnd()
	:	m_bDealControlsVisible	(false),
		m_pTrade(NULL),
		m_pOthersTrade(NULL),
		bStarted(false)
{
	Init();
	Hide();
	SetCurrentItem			(NULL);
}

CUITradeWnd::~CUITradeWnd()
{
	UIOurBagList.ClearAll		(true);
	UIOurTradeList.ClearAll		(true);
	UIOthersBagList.ClearAll	(true);
	UIOthersTradeList.ClearAll	(true);
}

void CUITradeWnd::Init()
{
	CUIXml								uiXml;
	uiXml.Load							(CONFIG_PATH, UI_PATH, TRADE_XML);
	CUIXmlInit							xml_init;

	xml_init.InitWindow					(uiXml, "main", 0, this);

	//статические элементы интерфейса
	AttachChild							(&UIStaticTop);
	xml_init.InitStatic					(uiXml, "top_background", 0, &UIStaticTop);
	AttachChild							(&UIStaticBottom);
	xml_init.InitStatic					(uiXml, "bottom_background", 0, &UIStaticBottom);

	//иконки с изображение нас и партнера по торговле
	AttachChild							(&UIOurIcon);
	xml_init.InitStatic					(uiXml, "static_icon", 0, &UIOurIcon);
	AttachChild							(&UIOthersIcon);
	xml_init.InitStatic					(uiXml, "static_icon", 1, &UIOthersIcon);
	UIOurIcon.AttachChild				(&UICharacterInfoLeft);
	UICharacterInfoLeft.Init			(0,0, UIOurIcon.GetWidth(), UIOurIcon.GetHeight(), TRADE_CHARACTER_XML);
	UIOthersIcon.AttachChild			(&UICharacterInfoRight);
	UICharacterInfoRight.Init			(0,0, UIOthersIcon.GetWidth(), UIOthersIcon.GetHeight(), TRADE_CHARACTER_XML);


	//Списки торговли
	AttachChild							(&UIOurBagWnd);
	xml_init.InitStatic					(uiXml, "our_bag_static", 0, &UIOurBagWnd);
	AttachChild							(&UIOthersBagWnd);
	xml_init.InitStatic					(uiXml, "others_bag_static", 0, &UIOthersBagWnd);

	UIOurBagWnd.AttachChild				(&UIOurMoneyStatic);
	xml_init.InitStatic					(uiXml, "our_money_static", 0, &UIOurMoneyStatic);

	UIOthersBagWnd.AttachChild			(&UIOtherMoneyStatic);
	xml_init.InitStatic					(uiXml, "other_money_static", 0, &UIOtherMoneyStatic);

	AttachChild							(&UIOurTradeWnd);
	xml_init.InitStatic					(uiXml, "static", 0, &UIOurTradeWnd);
	AttachChild							(&UIOthersTradeWnd);
	xml_init.InitStatic					(uiXml, "static", 1, &UIOthersTradeWnd);

	UIOurTradeWnd.AttachChild			(&UIOurPriceCaption);
	xml_init.InitMultiTextStatic		(uiXml, "price_mt_static", 0, &UIOurPriceCaption);

	UIOthersTradeWnd.AttachChild		(&UIOthersPriceCaption);
	xml_init.InitMultiTextStatic		(uiXml, "price_mt_static", 0, &UIOthersPriceCaption);

	//Списки Drag&Drop
	UIOurBagWnd.AttachChild				(&UIOurBagList);	
	xml_init.InitDragDropListEx			(uiXml, "dragdrop_list", 0, &UIOurBagList);

	UIOthersBagWnd.AttachChild			(&UIOthersBagList);	
	xml_init.InitDragDropListEx			(uiXml, "dragdrop_list", 1, &UIOthersBagList);

	UIOurTradeWnd.AttachChild			(&UIOurTradeList);	
	xml_init.InitDragDropListEx			(uiXml, "dragdrop_list", 2, &UIOurTradeList);

	UIOthersTradeWnd.AttachChild		(&UIOthersTradeList);	
	xml_init.InitDragDropListEx			(uiXml, "dragdrop_list", 3, &UIOthersTradeList);

	AttachChild							(&UIDescWnd);
	xml_init.InitStatic					(uiXml, "desc_static", 0, &UIDescWnd);
	xml_init.InitAutoStatic				(uiXml, "auto_static_hack", &UIDescWnd);
	UIDescWnd.AttachChild				(&UIItemInfo);
	UIItemInfo.Init						(0,0, UIDescWnd.GetWidth(), UIDescWnd.GetHeight(), TRADE_ITEM_XML);

	xml_init.InitAutoStatic				(uiXml, "auto_static", this);

	AttachChild							(&UIPerformTradeButton);
	xml_init.Init3tButton				(uiXml, "button", 0, &UIPerformTradeButton);

	AttachChild							(&UIToTalkButton);
	xml_init.Init3tButton				(uiXml, "button", 1, &UIToTalkButton);

	UIDealMsg							= NULL;

	BindDragDropListEnents				(&UIOurBagList);
	BindDragDropListEnents				(&UIOthersBagList);
	BindDragDropListEnents				(&UIOurTradeList);
	BindDragDropListEnents				(&UIOthersTradeList);
	InitHighlights						(uiXml);
}

void CUITradeWnd::InitTrade(CInventoryOwner* pOur, CInventoryOwner* pOthers)
{
	VERIFY								(pOur);
	VERIFY								(pOthers);

	m_pInvOwner							= pOur;
	m_pOthersInvOwner					= pOthers;
	UIOthersPriceCaption.GetPhraseByIndex(0)->SetText(*CStringTable().translate("ui_st_opponent_items"));

	UICharacterInfoLeft.InitCharacter(m_pInvOwner);
	UICharacterInfoRight.InitCharacter(m_pOthersInvOwner);

	m_pInv								= &m_pInvOwner->inventory();
	m_pOthersInv						= pOur->GetTrade()->GetPartnerInventory();
		
	m_pTrade							= pOur->GetTrade();
	m_pOthersTrade						= pOur->GetTrade()->GetPartnerTrade();
    	
	EnableAll							();

	UpdateLists							(eBoth);
}  

void CUITradeWnd::SendMessage(CUIWindow *pWnd, s16 msg, void *pData)
{
	if(pWnd == &UIToTalkButton && msg == BUTTON_CLICKED)
	{
		SwitchToTalk();
	}
	else if(pWnd == &UIPerformTradeButton && msg == BUTTON_CLICKED)
	{
		PerformTrade();
	}

	CUIWindow::SendMessage(pWnd, msg, pData);
}

void CUITradeWnd::Draw()
{
	inherited::Draw				();
	if(UIDealMsg)		UIDealMsg->Draw();

}

extern void UpdateCameraDirection(CGameObject* pTo);

void CUITradeWnd::Update()
{
	EListType et					= eNone;

	if(m_pInv->ModifyFrame()==Device.dwFrame && m_pOthersInv->ModifyFrame()==Device.dwFrame){
		et = eBoth;
	}else if(m_pInv->ModifyFrame()==Device.dwFrame){
		et = e1st;
	}else if(m_pOthersInv->ModifyFrame()==Device.dwFrame){
		et = e2nd;
	}
	if(et!=eNone)
		UpdateLists					(et);

	inherited::Update				();
	UpdateCameraDirection			(smart_cast<CGameObject*>(m_pOthersInvOwner));

	if(UIDealMsg){
		UIDealMsg->Update();
		if( !UIDealMsg->IsActual()){
			HUD().GetUI()->UIGame()->RemoveCustomStatic("not_enough_money_mine");
			HUD().GetUI()->UIGame()->RemoveCustomStatic("not_enough_money_other");
			UIDealMsg			= NULL;
		}
	}
}

#include "UIInventoryUtilities.h"
void CUITradeWnd::Show()
{
	InventoryUtilities::SendInfoToActor("ui_trade");
	inherited::Show					(true);
	inherited::Enable				(true);

	SetCurrentItem					(NULL);
	ResetAll						();
	UIDealMsg				= NULL;

	if (smart_cast<CActor*>(Level().CurrentEntity()) && GameConstants::GetHideWeaponInInventory())
	{
		Actor()->SetWeaponHideState(INV_STATE_BLOCK_ALL, true);
	}

	if (!SSFX_UI_DoF_active)
	{
		ps_ssfx_wpn_dof_1 = GameConstants::GetSSFX_FocusDoF();
		ps_ssfx_wpn_dof_2 = GameConstants::GetSSFX_FocusDoF().z;
		SSFX_UI_DoF_active = true;
	}
}

void CUITradeWnd::Hide()
{
	InventoryUtilities::SendInfoToActor("ui_trade_hide");
	inherited::Show					(false);
	inherited::Enable				(false);
	if(bStarted)
		StopTrade					();
	
	UIDealMsg				= NULL;

	if(HUD().GetUI()->UIGame()){
		HUD().GetUI()->UIGame()->RemoveCustomStatic("not_enough_money_mine");
		HUD().GetUI()->UIGame()->RemoveCustomStatic("not_enough_money_other");
	}

	UIOurBagList.ClearAll		(true);
	UIOurTradeList.ClearAll	(true);
	UIOthersBagList.ClearAll	(true);
	UIOthersTradeList.ClearAll(true);

	if (smart_cast<CActor*>(Level().CurrentEntity()) && GameConstants::GetHideWeaponInInventory())
	{
		Actor()->SetWeaponHideState(INV_STATE_BLOCK_ALL, false);
	}

	if (SSFX_UI_DoF_active)
	{
		ps_ssfx_wpn_dof_1 = GameConstants::GetSSFX_DefaultDoF();
		ps_ssfx_wpn_dof_2 = GameConstants::GetSSFX_DefaultDoF().z;
		SSFX_UI_DoF_active = false;
	}
}

void CUITradeWnd::StartTrade()
{
	if (m_pTrade)					m_pTrade->TradeCB(true);
	if (m_pOthersTrade)				m_pOthersTrade->TradeCB(true);
	bStarted						= true;
}

void CUITradeWnd::StopTrade()
{
	if (m_pTrade)					m_pTrade->TradeCB(false);
	if (m_pOthersTrade)				m_pOthersTrade->TradeCB(false);
	bStarted						= false;
}

#include "../trade_parameters.h"
bool CUITradeWnd::CanMoveToOther(PIItem pItem)
{
	if (pItem->GetCondition() < m_pOthersInvOwner->trade_parameters().buy_item_condition_factor)
		return false;

	float r1				= CalcItemsWeight(&UIOurTradeList);	// our
	float r2				= CalcItemsWeight(&UIOthersTradeList);	// other

	float itmWeight			= pItem->Weight();
	float otherInvWeight	= m_pOthersInv->CalcTotalWeight();
	float otherMaxWeight	= m_pOthersInv->GetMaxWeight();

	if (!m_pOthersInvOwner->trade_parameters().enabled(
			CTradeParameters::action_buy(0),
			pItem->object().cNameSect()
		))
		return				(false);

	if(otherInvWeight-r2+r1+itmWeight > otherMaxWeight)
		return				false;

	return true;
}

void move_item(CUICellItem* itm, CUIDragDropListEx* from, CUIDragDropListEx* to)
{
	CUICellItem* _itm		= from->RemoveItem	(itm, false);
	to->SetItem				(_itm);
}

bool CUITradeWnd::ToOurTrade()
{
	if (!CanMoveToOther(CurrentIItem()))	return false;

	move_item				(CurrentItem(), &UIOurBagList, &UIOurTradeList);
	UpdatePrices			();
	return					true;
}

bool CUITradeWnd::ToOthersTrade()
{
	move_item				(CurrentItem(), &UIOthersBagList, &UIOthersTradeList);
	UpdatePrices			();

	return					true;
}

bool CUITradeWnd::ToOurBag()
{
	move_item				(CurrentItem(), &UIOurTradeList, &UIOurBagList);
	UpdatePrices			();
	
	return					true;
}

bool CUITradeWnd::ToOthersBag()
{
	move_item				(CurrentItem(), &UIOthersTradeList, &UIOthersBagList);
	UpdatePrices			();

	return					true;
}

float CUITradeWnd::CalcItemsWeight(CUIDragDropListEx* pList)
{
	float res = 0.0f;

	for(u32 i=0; i<pList->ItemsCount(); ++i)
	{
		CUICellItem* itm	= pList->GetItemIdx	(i);
		PIItem	iitem		= (PIItem)itm->m_pData;
		res					+= iitem->Weight();
		for(u32 j=0; j<itm->ChildsCount(); ++j){
			PIItem	jitem		= (PIItem)itm->Child(j)->m_pData;
			res					+= jitem->Weight();
		}
	}
	return res;
}

u32 CUITradeWnd::CalcItemsPrice(CUIDragDropListEx* pList, CTrade* pTrade, bool bBuying)
{
	u32 iPrice				= 0;
	
	for(u32 i=0; i<pList->ItemsCount(); ++i)
	{
		CUICellItem* itm	= pList->GetItemIdx(i);
		PIItem iitem		= (PIItem)itm->m_pData;
		iPrice				+= pTrade->GetItemPrice(iitem, bBuying);

		for(u32 j=0; j<itm->ChildsCount(); ++j){
			PIItem jitem	= (PIItem)itm->Child(j)->m_pData;
			iPrice			+= pTrade->GetItemPrice(jitem, bBuying);
		}

	}

	return					iPrice;
}

void CUITradeWnd::PerformTrade()
{

	if (UIOurTradeList.ItemsCount()==0 && UIOthersTradeList.ItemsCount()==0) 
		return;

	int our_money			= (int)m_pInvOwner->get_money();
	int others_money		= (int)m_pOthersInvOwner->get_money();

	int delta_price			= int(m_iOurTradePrice-m_iOthersTradePrice);

	our_money				+= delta_price;
	others_money			-= delta_price;

	if(our_money>=0 && others_money>=0 && (m_iOurTradePrice>=0 || m_iOthersTradePrice>0))
	{
		m_pOthersTrade->OnPerformTrade(m_iOthersTradePrice, m_iOurTradePrice);
		
		TransferItems		(&UIOurTradeList,		&UIOthersBagList, m_pOthersTrade,	true);
		TransferItems		(&UIOthersTradeList,	&UIOurBagList,	m_pOthersTrade,	false);
	}else
	{
		if(others_money<0)
			UIDealMsg		= HUD().GetUI()->UIGame()->AddCustomStatic("not_enough_money_other", true);
		else
			UIDealMsg		= HUD().GetUI()->UIGame()->AddCustomStatic("not_enough_money_mine", true);


		UIDealMsg->m_endTime	= Device.fTimeGlobal+2.0f;// sec
	}
	SetCurrentItem			(NULL);
}

void CUITradeWnd::DisableAll()
{
	UIOurBagWnd.Enable			(false);
	UIOthersBagWnd.Enable			(false);
	UIOurTradeWnd.Enable			(false);
	UIOthersTradeWnd.Enable		(false);
}

void CUITradeWnd::EnableAll()
{
	UIOurBagWnd.Enable			(true);
	UIOthersBagWnd.Enable			(true);
	UIOurTradeWnd.Enable			(true);
	UIOthersTradeWnd.Enable		(true);
}

void CUITradeWnd::UpdatePrices()
{
	m_iOurTradePrice	= CalcItemsPrice	(&UIOurTradeList,		m_pOthersTrade, true);
	m_iOthersTradePrice = CalcItemsPrice	(&UIOthersTradeList,	m_pOthersTrade, false);


	string256				buf;
	sprintf_s					(buf, "%d RU", m_iOurTradePrice);
	UIOurPriceCaption.GetPhraseByIndex(2)->str = buf;
	sprintf_s					(buf, "%d RU", m_iOthersTradePrice);
	UIOthersPriceCaption.GetPhraseByIndex(2)->str = buf;

	sprintf_s					(buf, "%d RU", m_pInvOwner->get_money());
	UIOurMoneyStatic.SetText(buf);

	if(!m_pOthersInvOwner->InfinitiveMoney()){
		sprintf_s					(buf, "%d RU", m_pOthersInvOwner->get_money());
		UIOtherMoneyStatic.SetText(buf);
	}else
	{
		UIOtherMoneyStatic.SetText("---");
	}
}

void CUITradeWnd::TransferItems(CUIDragDropListEx* pSellList,
								CUIDragDropListEx* pBuyList,
								CTrade* pTrade,
								bool bBuying)
{
	while(pSellList->ItemsCount())
	{
		CUICellItem* itm	=	pSellList->RemoveItem(pSellList->GetItemIdx(0),false);
		pTrade->TransferItem	((PIItem)itm->m_pData, bBuying);
		pBuyList->SetItem		(itm);
	}

	pTrade->pThis.inv_owner->set_money ( pTrade->pThis.inv_owner->get_money(), true );
	pTrade->pPartner.inv_owner->set_money( pTrade->pPartner.inv_owner->get_money(), true );
}

void CUITradeWnd::UpdateLists(EListType mode)
{
	if(mode==eBoth||mode==e1st){
		UIOurBagList.ClearAll(true);
		UIOurTradeList.ClearAll(true);
	}

	if(mode==eBoth||mode==e2nd){
		UIOthersBagList.ClearAll(true);
		UIOthersTradeList.ClearAll(true);
	}

	UpdatePrices						();


	if(mode==eBoth||mode==e1st){
		ruck_list.clear					();
   		m_pInv->AddAvailableItems		(ruck_list, true);
		std::sort						(ruck_list.begin(),ruck_list.end(),InventoryUtilities::GreaterRoomInRuck);
		FillList						(ruck_list, UIOurBagList, true);
	}

	if(mode==eBoth||mode==e2nd){
		ruck_list.clear					();
		m_pOthersInv->AddAvailableItems	(ruck_list, true);
		std::sort						(ruck_list.begin(),ruck_list.end(),InventoryUtilities::GreaterRoomInRuck);
		FillList						(ruck_list, UIOthersBagList, true);
	}
}

void CUITradeWnd::FillList	(TIItemContainer& cont, CUIDragDropListEx& dragDropList, bool do_colorize)
{
	TIItemContainer::iterator it	= cont.begin();
	TIItemContainer::iterator it_e	= cont.end();

	for(; it != it_e; ++it) 
	{
		CUICellItem* itm			= create_cell_item	(*it);
		if(do_colorize)				ColorizeItem		(itm, CanMoveToOther(*it));
		dragDropList.SetItem		(itm);
	}

}

bool CUITradeWnd::OnItemStartDrag(CUICellItem* itm)
{
	return false; //default behaviour
}

bool CUITradeWnd::OnItemSelected(CUICellItem* itm)
{
	SetCurrentItem		(itm);
	return				false;
}

bool CUITradeWnd::OnItemRButtonClick(CUICellItem* itm)
{
	SetCurrentItem				(itm);
	return						false;
}


bool CUITradeWnd::OnItemDrop(CUICellItem* itm)
{
	CUIDragDropListEx*	old_owner		= itm->OwnerList();
	CUIDragDropListEx*	new_owner		= CUIDragDropListEx::m_drag_item->BackList();
	if(old_owner==new_owner || !old_owner || !new_owner)
					return false;

	if(old_owner==&UIOurBagList && new_owner==&UIOurTradeList)
		ToOurTrade				();
	else if(old_owner==&UIOurTradeList && new_owner==&UIOurBagList)
		ToOurBag				();
	else if(old_owner==&UIOthersBagList && new_owner==&UIOthersTradeList)
		ToOthersTrade			();
	else if(old_owner==&UIOthersTradeList && new_owner==&UIOthersBagList)
		ToOthersBag				();

	return true;
}

bool CUITradeWnd::OnItemDbClick(CUICellItem* itm)
{
	SetCurrentItem						(itm);
	CUIDragDropListEx*	old_owner		= itm->OwnerList();
	
	if(old_owner == &UIOurBagList)
		ToOurTrade				();
	else if(old_owner == &UIOurTradeList)
		ToOurBag				();
	else if(old_owner == &UIOthersBagList)
		ToOthersTrade			();
	else if(old_owner == &UIOthersTradeList)
		ToOthersBag				();
	else
		R_ASSERT2(false, "wrong parent for cell item");

	return true;
}


CUICellItem* CUITradeWnd::CurrentItem()
{
	return m_pCurrentCellItem;
}

PIItem CUITradeWnd::CurrentIItem()
{
	return	(m_pCurrentCellItem)?(PIItem)m_pCurrentCellItem->m_pData : NULL;
}

void CUITradeWnd::SetCurrentItem(CUICellItem* itm)
{
	if(m_pCurrentCellItem == itm) return;
	m_pCurrentCellItem				= itm;
	UIItemInfo.InitItem	(CurrentIItem());
	
	if(!m_pCurrentCellItem)		return;

	CUIDragDropListEx* owner	= itm->OwnerList();
	bool bBuying				= (owner==&UIOurBagList) || (owner==&UIOurTradeList);

	if(itm && UIItemInfo.UICost){

		string256			str;

		sprintf_s				(str, "%d RU", m_pOthersTrade->GetItemPrice(CurrentIItem(), bBuying) );
		UIItemInfo.UICost->SetText (str);
	}
}

void CUITradeWnd::SwitchToTalk()
{
	GetMessageTarget()->SendMessage		(this, TRADE_WND_CLOSED);
}

void CUITradeWnd::BindDragDropListEnents(CUIDragDropListEx* lst)
{
	lst->m_f_item_drop				= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUITradeWnd::OnItemDrop);
	lst->m_f_item_start_drag		= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUITradeWnd::OnItemStartDrag);
	lst->m_f_item_db_click			= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUITradeWnd::OnItemDbClick);
	lst->m_f_item_selected			= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUITradeWnd::OnItemSelected);
	lst->m_f_item_rbutton_click		= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUITradeWnd::OnItemRButtonClick);
	lst->m_f_item_focus_received	= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUITradeWnd::OnItemFocusReceive);
	lst->m_f_item_focus_lost		= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUITradeWnd::OnItemFocusLost);
	lst->m_f_item_focused_update	= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUITradeWnd::OnItemFocusedUpdate);
}

void CUITradeWnd::ColorizeItem(CUICellItem* itm, bool b)
{
	CInventoryItem*	jitem = (CInventoryItem*)itm->m_pData;
	PIItem piitem = (PIItem)itm->m_pData;
	if ((jitem->m_eItemPlace == eItemPlaceBelt || jitem->m_eItemPlace == eItemPlaceSlot) && b && piitem->m_pCurrentInventory->GetOwner() == m_pInvOwner)
	{
		itm->SetColor(color_rgba(180, 255, 180, 255));
	}
	else if ((jitem->m_eItemPlace == eItemPlaceBelt || jitem->m_eItemPlace == eItemPlaceSlot) && !b)
	{
		itm->SetColor(color_rgba(225, 155, 130, 255));
	}
	else if (jitem->m_eItemPlace == eItemPlaceRuck && !b)
	{
		itm->SetColor(color_rgba(255, 100, 100, 255));
	}
}

bool CUITradeWnd::OnKeyboardAction(int dik, EUIMessages keyboard_action)
{
	if (inherited::OnKeyboardAction(dik, keyboard_action))return true;

	if (keyboard_action == WINDOW_KEY_PRESSED)
	{
		if (is_binded(kUSE, dik) || is_binded(kQUIT, dik))
		{
			SwitchToTalk();
			return true;
		}
		else if (is_binded(kSPRINT_TOGGLE, dik))
		{
			PerformTrade();
			return true;
		}
	}
	return false;
}

