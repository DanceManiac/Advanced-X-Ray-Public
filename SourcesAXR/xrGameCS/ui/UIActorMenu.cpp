#include "stdafx.h"
#include "UIActorMenu.h"
#include "UIActorStateInfo.h"
#include "../actor.h"
#include "../uigamesp.h"
#include "../hudmanager.h"
#include "../inventory.h"
#include "../inventory_item.h"
#include "../InventoryBox.h"
#include "../trade.h"
#include "../trade_parameters.h"
#include "object_broker.h"
#include "../ai/monsters/BaseMonster/base_monster.h"
#include "UIInventoryUtilities.h"
#include "game_cl_base.h"

#include "UICursor.h"
#include "UICellItem.h"
#include "UICharacterInfo.h"
#include "UIItemInfo.h"
#include "UIDragDropListEx.h"
#include "UIInventoryUpgradeWnd.h"
#include "UI3tButton.h"
#include "UIBtnHint.h"
#include "UIMessageBoxEx.h"
#include "UIPropertiesBox.h"
#include "UIMainIngameWnd.h"
#include "../xrEngine/x_ray.h"

bool SSFX_UI_DoF_active = false;

void CUIActorMenu::SetActor(CInventoryOwner* io)
{
	R_ASSERT			(!IsShown());
	m_last_time			= Device.dwTimeGlobal;
	m_pActorInvOwner	= io;
	
	if ( IsGameTypeSingle() )
	{
		if ( io )
			m_ActorCharacterInfo->InitCharacter(m_pActorInvOwner);
		else
			m_ActorCharacterInfo->ClearInfo();
	}
	else
	{
		UpdateActorMP();
	}
}

void CUIActorMenu::SetPartner(CInventoryOwner* io)
{
	R_ASSERT			(!IsShown());
	m_pPartnerInvOwner	= io;
	if ( m_pPartnerInvOwner )
	{
		CBaseMonster* monster = smart_cast<CBaseMonster*>( m_pPartnerInvOwner );
		if ( monster || m_pPartnerInvOwner->use_simplified_visual() ) 
		{
			m_PartnerCharacterInfo->ClearInfo();
			if ( monster )
			{
				shared_str monster_tex_name = pSettings->r_string( monster->cNameSect(), "icon" );
				m_PartnerCharacterInfo->UIIcon().InitTexture( monster_tex_name.c_str() );
				m_PartnerCharacterInfo->UIIcon().SetStretchTexture( true );
			}
		}
		else 
		{
			m_PartnerCharacterInfo->InitCharacter(m_pPartnerInvOwner);
		}
		SetInvBox( NULL );
	}
	else
	{
		m_PartnerCharacterInfo->ClearInfo();
	}
}

void CUIActorMenu::SetInvBox(CInventoryBox* box)
{
	R_ASSERT			(!IsShown());
	m_pInvBox = box;
	if ( box )
	{
		m_pInvBox->m_in_use = true;
		SetPartner( NULL );
	}
}

void CUIActorMenu::SetCarTrunk(CCar* pCar)
{
	R_ASSERT(!IsShown());
	m_pCar = pCar;
	if (m_pCar)
	{
		SetPartner(NULL);
	}
}

void CUIActorMenu::SetMenuMode(EMenuMode mode)
{
	SetCurrentItem( NULL );
	m_hint_wnd->set_text( NULL );
	
	CActor* actor = smart_cast<CActor*>( m_pActorInvOwner );
	if ( actor )	{	actor->PickupModeOff();	}
	
	if ( mode != m_currMenuMode )
	{
		switch(m_currMenuMode)
		{
		case mmUndefined:
			break;
		case mmInventory:
			DeInitInventoryMode();
			break;
		case mmTrade:
			DeInitTradeMode();
			break;
		case mmUpgrade:
			DeInitUpgradeMode();
			break;
		case mmDeadBodySearch:
			DeInitDeadBodySearchMode();
			break;
		default:
			R_ASSERT(0);
			break;
		}

		HUD().GetUI()->UIMainIngameWnd->ShowZoneMap(false);

		m_currMenuMode = mode;
		switch(mode)
		{
		case mmUndefined:
#ifdef DEBUG
			Msg("* now is Undefined mode");
#endif // #ifdef DEBUG
			ResetMode();
			break;
		case mmInventory:
			InitInventoryMode();
#ifdef DEBUG
			Msg("* now is Inventory mode");
#endif // #ifdef DEBUG
			break;
		case mmTrade:
			InitTradeMode();
#ifdef DEBUG
			Msg("* now is Trade mode");
#endif // #ifdef DEBUG
			break;
		case mmUpgrade:
			InitUpgradeMode();
#ifdef DEBUG
			Msg("* now is Upgrade mode");
#endif // #ifdef DEBUG
			break;
		case mmDeadBodySearch:
			InitDeadBodySearchMode();
#ifdef DEBUG
			Msg("* now is DeadBodySearch mode");
#endif // #ifdef DEBUG
			break;
		default:
			R_ASSERT(0);
			break;
		}
		CurModeToScript();
	}//if

	if ( m_pActorInvOwner )
	{
		UpdateOutfit();
		UpdateActor();
	}
	UpdateButtonsLayout();

	if (m_currMenuMode >= 1)
	{
		ps_ssfx_wpn_dof_1 = GameConstants::GetSSFX_FocusDoF();
		ps_ssfx_wpn_dof_2 = GameConstants::GetSSFX_FocusDoF().z;
		SSFX_UI_DoF_active = true;
	}
}

void CUIActorMenu::PlaySnd(eActorMenuSndAction a)
{
	if (sounds[a]._handle())
        sounds[a].play					(NULL, sm_2D);
}

void CUIActorMenu::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
	CUIWndCallback::OnEvent		(pWnd, msg, pData);
}

void CUIActorMenu::Show()
{
	CActor* pActor = smart_cast<CActor*>(Level().CurrentEntity());
	CCustomDetector* pDet = smart_cast<CCustomDetector*>(Actor()->inventory().ItemFromSlot(DETECTOR_SLOT));

	SetMenuMode							(m_currMenuMode);
	inherited::Show						();
	PlaySnd								(eSndOpen);
	m_ActorStateInfo->Show				(true);
	m_ActorStateInfo->UpdateActorInfo	(m_pActorInvOwner);

	if (pActor && GameConstants::GetHideWeaponInInventory())
	{
		Actor()->SetWeaponHideState(INV_STATE_BLOCK_ALL, true);

		if (pDet)
			pDet->HideDetector(true);

		Actor()->block_action(kDETECTOR);
	}
}

void CUIActorMenu::Hide()
{
	CActor* pActor = smart_cast<CActor*>(Level().CurrentEntity());

	inherited::Hide						();
	PlaySnd								(eSndClose);
	SetMenuMode							(mmUndefined);
	m_ActorStateInfo->Show				(false);

	if (pActor && GameConstants::GetHideWeaponInInventory())
	{
		Actor()->SetWeaponHideState(INV_STATE_BLOCK_ALL, false);
		Actor()->unblock_action(kDETECTOR);
	}
}

void CUIActorMenu::Draw()
{
	inherited::Draw();
	HUD().GetUI()->UIMainIngameWnd->DrawZoneMap();
	m_ItemInfo->Draw();
	m_hint_wnd->Draw();
}

void CUIActorMenu::Update()
{	
	{ // all mode
		m_last_time = Device.dwTimeGlobal;
		m_ActorStateInfo->UpdateActorInfo( m_pActorInvOwner );
	}

	switch ( m_currMenuMode )
	{
	case mmUndefined:
		break;
	case mmInventory:
		{
			m_clock_value->SetText( InventoryUtilities::GetGameTimeAsString( InventoryUtilities::etpTimeToMinutes ).c_str() );
			HUD().GetUI()->UIMainIngameWnd->UpdateZoneMap();
			break;
		}
	case mmTrade:
		{
			if(m_pPartnerInvOwner->inventory().ModifyFrame() != m_trade_partner_inventory_state)
				InitPartnerInventoryContents	();
			CheckDistance					();
			break;
		}
	case mmUpgrade:
		{
			UpdateUpgradeItem();
			CheckDistance();
			break;
		}
	case mmDeadBodySearch:
		{
			CheckDistance();
			break;
		}
	default: R_ASSERT(0); break;
	}
	
	inherited::Update();
	m_ItemInfo->Update();
	m_hint_wnd->Update();
}
bool CUIActorMenu::StopAnyMove()  // true = ????? ?? ???? ??? ???????? ????
{
	switch ( m_currMenuMode )
	{
	case mmInventory:
		return false;
	case mmUndefined:
	case mmTrade:
	case mmUpgrade:
	case mmDeadBodySearch:
		return true;
	}
	return true;
}

void CUIActorMenu::CheckDistance()
{
	CGameObject* pActorGO	= smart_cast<CGameObject*>(m_pActorInvOwner);
	CGameObject* pPartnerGO	= smart_cast<CGameObject*>(m_pPartnerInvOwner);
	CGameObject* pBoxGO		= smart_cast<CGameObject*>(m_pInvBox);
	VERIFY( pActorGO && (pPartnerGO || pBoxGO || m_pCar) );

	if ( pPartnerGO )
	{
		if ( ( pActorGO->Position().distance_to( pPartnerGO->Position() ) > 3.0f ) &&
			!m_pPartnerInvOwner->NeedOsoznanieMode() )
		{
			g_btnHint->Discard();
			GetHolder()->StartStopMenu( this, true ); // hide actor menu
		}
	}
	else if (m_pCar && Actor()->Holder())
	{
		//nop
	}
	else //pBoxGO
	{
		VERIFY( pBoxGO );
		if ( pActorGO->Position().distance_to( pBoxGO->Position() ) > 3.0f )
		{
			g_btnHint->Discard();
			GetHolder()->StartStopMenu( this, true ); // hide actor menu
		}
	}
}

EDDListType CUIActorMenu::GetListType(CUIDragDropListEx* l)
{
	if(l==m_pInventoryBagList)			return iActorBag;
	if(l==m_pInventoryBeltList)			return iActorBelt;

	if(l==m_pInventoryAutomaticList)	return iActorSlot;
	if(l==m_pInventoryPistolList)		return iActorSlot;
	if(l==m_pInventoryOutfitList)		return iActorSlot;
	if(l==m_pInventoryDetectorList)		return iActorSlot;
	

	if(l==m_pTradeActorBagList)			return iActorBag;
	if(l==m_pTradeActorList)			return iActorTrade;
	if(l==m_pTradePartnerBagList)		return iPartnerTradeBag;
	if(l==m_pTradePartnerList)			return iPartnerTrade;
	if(l==m_pDeadBodyBagList)			return iDeadBodyBag;
	if(l==m_pTrashList)					return iTrashSlot;

	if (GameConstants::GetKnifeSlotEnabled())
	{
		if (l == m_pInventoryKnifeList)	return iActorSlot;
	}

	if (GameConstants::GetBinocularSlotEnabled())
	{
		if (l == m_pInventoryBinocularList) return iActorSlot;;
	}

	if (GameConstants::GetTorchSlotEnabled())
	{
		if (l == m_pInventoryTorchList) return iActorSlot;
	}

	if (GameConstants::GetBackpackSlotEnabled())
	{
		if (l == m_pInventoryBackpackList) return iActorSlot;
	}

	if (GameConstants::GetDosimeterSlotEnabled())
	{
		if (l == m_pInventoryDosimeterList)
			return iActorSlot;
	}

	if (GameConstants::GetPantsSlotEnabled())
	{
		if (l == m_pInventoryPantsList)
			return iActorSlot;
	}

	if (GameConstants::GetPdaSlotEnabled())
	{
		if (l == m_pInventoryPdaList)
			return iActorSlot;
	}

	R_ASSERT(0);
	
	return iInvalid;
}

CUIDragDropListEx* CUIActorMenu::GetListByType(EDDListType t)
{
	switch(t)
	{
		case iActorBag:
			{
				if(m_currMenuMode==mmTrade)
					return m_pTradeActorBagList;
				else
					return m_pInventoryBagList;
			}break;
		default:
			{
				R_ASSERT("invalid call");
			}break;
	}
	return NULL;
}

CUICellItem* CUIActorMenu::CurrentItem()
{
	return m_pCurrentCellItem;
}

PIItem CUIActorMenu::CurrentIItem()
{
	return	(m_pCurrentCellItem)? (PIItem)m_pCurrentCellItem->m_pData : NULL;
}

void CUIActorMenu::SetCurrentItem(CUICellItem* itm)
{
	m_repair_mode = false;
	m_pCurrentCellItem = itm;
	if ( !itm )
	{
		InfoCurItem( NULL );
	}
	TryHidePropertiesBox();

	if ( m_currMenuMode == mmUpgrade )
	{
		SetupUpgradeItem();
	}
}

// ================================================================

void CUIActorMenu::InfoCurItem( CUICellItem* cell_item )
{
	if ( !cell_item )
	{
		m_ItemInfo->InitItem( NULL );
		return;
	}
	PIItem current_item = (PIItem)cell_item->m_pData;

	PIItem compare_item = NULL;
	u32    compare_slot = current_item->GetSlot();
	if ( compare_slot != NO_ACTIVE_SLOT )
	{
		compare_item = m_pActorInvOwner->inventory().ItemFromSlot(compare_slot);
	}

	if(m_currMenuMode ==mmTrade)
	{
		CInventoryOwner* item_owner = smart_cast<CInventoryOwner*>(current_item->m_pInventory->GetOwner());
		u32 item_price = u32(-1);
		if(item_owner && item_owner==m_pActorInvOwner)
			item_price = m_partner_trade->GetItemPrice(current_item, true);
		else
			item_price = m_partner_trade->GetItemPrice(current_item, false);

		//if(item_price>500)
		//	item_price = iFloor(item_price/10+0.5f)*10;

		CWeaponAmmo* ammo = smart_cast<CWeaponAmmo*>(current_item);
		if(ammo)
		{
			for( u32 j = 0; j < cell_item->ChildsCount(); ++j )
			{
				u32 tmp_price	= 0;
				PIItem jitem	= (PIItem)cell_item->Child(j)->m_pData;
				CInventoryOwner* ammo_owner = smart_cast<CInventoryOwner*>(jitem->m_pInventory->GetOwner());
				if(ammo_owner && ammo_owner==m_pActorInvOwner)
					tmp_price = m_partner_trade->GetItemPrice(jitem, true);
				else
					tmp_price = m_partner_trade->GetItemPrice(jitem, false);

				//if(tmp_price>500)
				//	tmp_price = iFloor(tmp_price/10+0.5f)*10;

				item_price		+= tmp_price;
			}
		}

		if(	!current_item->CanTrade() || 
			(!m_pPartnerInvOwner->trade_parameters().enabled(CTradeParameters::action_buy(0), 
															current_item->object().cNameSect()) &&
			item_owner && item_owner==m_pActorInvOwner)
		)
			m_ItemInfo->InitItem	( cell_item, compare_item, u32(-1), "st_no_trade_tip_1" );
		else if(current_item->GetCondition()<m_pPartnerInvOwner->trade_parameters().buy_item_condition_factor)
			m_ItemInfo->InitItem	( cell_item, compare_item, u32(-1), "st_no_trade_tip_2" );
		else
			m_ItemInfo->InitItem	( cell_item, compare_item, item_price );
	}
	else
		m_ItemInfo->InitItem	( cell_item, compare_item, u32(-1));

	float dx_pos = GetWndRect().left;
	fit_in_rect(m_ItemInfo, Frect().set( 0.0f, 0.0f, UI_BASE_WIDTH - dx_pos, UI_BASE_HEIGHT ), 10.0f, dx_pos );
}

void CUIActorMenu::UpdateItemsPlace()
{
	switch ( m_currMenuMode )
	{
	case mmUndefined:
		break;
	case mmInventory:
		
		break;
	case mmTrade:
		UpdatePrices();
		break;
	case mmUpgrade:
		SetupUpgradeItem();
		break;
	case mmDeadBodySearch:
		UpdateDeadBodyBag();
		break;
	default:
		R_ASSERT(0);
		break;
	}

	if ( m_pActorInvOwner )
	{
		UpdateOutfit();
		UpdateActor();
	}
}

void CUIActorMenu::ClearAllLists()
{
	m_pInventoryBagList->ClearAll				(true);
	
	m_pInventoryBeltList->ClearAll				(true);
	m_pInventoryOutfitList->ClearAll			(true);
	m_pInventoryDetectorList->ClearAll			(true);
	m_pInventoryPistolList->ClearAll			(true);
	m_pInventoryAutomaticList->ClearAll			(true);

	m_pTradeActorBagList->ClearAll				(true);
	m_pTradeActorList->ClearAll					(true);
	m_pTradePartnerBagList->ClearAll			(true);
	m_pTradePartnerList->ClearAll				(true);
	m_pDeadBodyBagList->ClearAll				(true);

	if (GameConstants::GetKnifeSlotEnabled())
	{
		m_pInventoryKnifeList->ClearAll(true);
	}

	if (GameConstants::GetBinocularSlotEnabled())
	{
		m_pInventoryBinocularList->ClearAll(true);
	}

	if (GameConstants::GetTorchSlotEnabled())
	{
		m_pInventoryTorchList->ClearAll(true);
	}

	if (GameConstants::GetBackpackSlotEnabled())
	{
		m_pInventoryBackpackList->ClearAll(true);
	}

	if (GameConstants::GetDosimeterSlotEnabled())
	{
		m_pInventoryDosimeterList->ClearAll(true);
	}

	if (GameConstants::GetPantsSlotEnabled())
	{
		m_pInventoryPantsList->ClearAll(true);
	}

	if (GameConstants::GetPdaSlotEnabled())
	{
		m_pInventoryPdaList->ClearAll(true);
	}
}

void CUIActorMenu::CallMessageBoxYesNo( LPCSTR text )
{
	m_message_box_yes_no->SetText( text );
	m_message_box_yes_no->func_on_ok = CUIWndCallback::void_function( this, &CUIActorMenu::OnMesBoxYes );
	HUD().GetUI()->StartStopMenu( m_message_box_yes_no, false );
}

void CUIActorMenu::CallMessageBoxOK( LPCSTR text )
{
	m_message_box_ok->SetText( text );
	HUD().GetUI()->StartStopMenu( m_message_box_ok, false );
}

void CUIActorMenu::ResetMode()
{
	ClearAllLists				();
	m_pMouseCapturer			= NULL;
	m_UIPropertiesBox->Hide		();
	SetCurrentItem				(NULL);

	if (SSFX_UI_DoF_active)
	{
		ps_ssfx_wpn_dof_1 = GameConstants::GetSSFX_DefaultDoF();
		ps_ssfx_wpn_dof_2 = GameConstants::GetSSFX_DefaultDoF().z;
		SSFX_UI_DoF_active = false;
	}
}

void CUIActorMenu::UpdateActorMP()
{
	if ( !Level().game || !Game().local_player || !m_pActorInvOwner || IsGameTypeSingle() )
	{
		m_ActorCharacterInfo->ClearInfo();
		m_ActorMoney->SetText( "" );
		return;
	}

	int money = Game().local_player->money_for_round;

	string64 buf;
	sprintf_s( buf, "%d RU", money );
	m_ActorMoney->SetText( buf );

	m_ActorCharacterInfo->InitCharacterMP( Game().local_player->name, "ui_npc_u_nebo_1" );

}

CScriptGameObject* CUIActorMenu::GetCurrentItemAsGameObject()
{
	CGameObject* GO = smart_cast<CGameObject*>(CurrentIItem());
	if (GO)
		return GO->lua_game_object();

	return nullptr;
}