#include "pch_script.h"
#include "UIInventoryWnd.h"

#include "xrUIXmlParser.h"
#include "UIXmlInit.h"
#include "../string_table.h"

#include "../actor.h"
#include "../uigamesp.h"
#include "../hudmanager.h"

#include "../CustomOutfit.h"
#include "../CustomBackpack.h"

#include "../weapon.h"

#include "script_process.h"

#include "../eatable_item.h"
#include "../Battery.h"
#include "../AnomalyDetector.h"
#include "../Torch.h"
#include "../inventory.h"

#include "UIInventoryUtilities.h"
using namespace InventoryUtilities;


#include "../InfoPortion.h"
#include "../level.h"
#include "game_base_space.h"
#include "../entitycondition.h"

#include "../game_cl_base.h"
#include "UISleepWnd.h"
#include "../ActorCondition.h"
#include "UIDragDropListEx.h"
#include "UIOutfitSlot.h"
#include "UI3tButton.h"
#include "UIHelper.h"

#include "AdvancedXrayGameConstants.h"
#include "../../xrEngine/x_ray.h"

#define				INVENTORY_ITEM_XML		"inventory_item.xml"
#define				INVENTORY_XML			"inventory_new.xml"

CUIInventoryWnd*	g_pInvWnd = NULL;

extern bool SSFX_UI_DoF_active;

CUIInventoryWnd::CUIInventoryWnd()
{
	m_iCurrentActiveSlot				= NO_ACTIVE_SLOT;
	UIRank								= NULL;

	m_pUIBagList						= NULL;
	m_pUIBeltList						= NULL;
	m_pUIPistolList						= NULL;
	m_pUIAutomaticList					= NULL;
	m_pUIOutfitList						= NULL;

	// M.F.S. Team: New Slots
	m_pUIKnifeList						= NULL;
	m_pUITorchList						= NULL;
	m_pUIBinocularList					= NULL;
	m_pUIPdaList						= NULL;
	m_pUIDosimeterList					= NULL;
	m_pUIBackpackList					= NULL;
	m_pUIPantsList						= NULL;

	Init								();
	SetCurrentItem						(NULL);

	g_pInvWnd							= this;
	m_b_need_reinit						= false;
	Hide								();
}

void CUIInventoryWnd::Init()
{
	CUIXml								uiXml;
	uiXml.Load							(CONFIG_PATH, UI_PATH, INVENTORY_XML);

	CUIXmlInit							xml_init;

	xml_init.InitWindow					(uiXml, "main", 0, this);

	AttachChild							(&UIBeltSlots);
	xml_init.InitStatic					(uiXml, "belt_slots", 0, &UIBeltSlots);

	AttachChild							(&UIBack);
	xml_init.InitStatic					(uiXml, "back", 0, &UIBack);

	AttachChild							(&UIStaticBottom);
	xml_init.InitStatic					(uiXml, "bottom_static", 0, &UIStaticBottom);

	AttachChild							(&UIBagWnd);
	xml_init.InitStatic					(uiXml, "bag_static", 0, &UIBagWnd);
	
	AttachChild							(&UIMoneyWnd);
	xml_init.InitStatic					(uiXml, "money_static", 0, &UIMoneyWnd);

	//Ёлементы автоматического добавлени€
	xml_init.InitAutoStatic				(uiXml, "auto_static", this);

	AttachChild							(&UIDescrWnd);
	xml_init.InitStatic					(uiXml, "descr_static", 0, &UIDescrWnd);

	UIDescrWnd.AttachChild				(&UIItemInfo);
	UIItemInfo.Init						(0, 0, UIDescrWnd.GetWidth(), UIDescrWnd.GetHeight(), INVENTORY_ITEM_XML);

	AttachChild							(&UIPersonalWnd);
	xml_init.InitFrameWindow			(uiXml, "character_frame_window", 0, &UIPersonalWnd);

	AttachChild							(&UIProgressBack);
	xml_init.InitStatic					(uiXml, "progress_background", 0, &UIProgressBack);

	if (GameID() != GAME_SINGLE){
		AttachChild						(&UIProgressBack_rank);
		xml_init.InitStatic				(uiXml, "progress_back_rank", 0, &UIProgressBack_rank);

		UIProgressBack_rank.AttachChild	(&UIProgressBarRank);
		xml_init.InitProgressBar		(uiXml, "progress_bar_rank", 0, &UIProgressBarRank);
		UIProgressBarRank.SetProgressPos(100);

	}

	UIProgressBack.AttachChild			(&UIProgressBarHealth);
	xml_init.InitProgressBar			(uiXml, "progress_bar_health", 0, &UIProgressBarHealth);
	
	UIProgressBack.AttachChild			(&UIProgressBarPsyHealth);
	xml_init.InitProgressBar			(uiXml, "progress_bar_psy", 0, &UIProgressBarPsyHealth);

	UIProgressBack.AttachChild			(&UIProgressBarRadiation);
	xml_init.InitProgressBar			(uiXml, "progress_bar_radiation", 0, &UIProgressBarRadiation);

	UIPersonalWnd.AttachChild			(&UIStaticPersonal);
	xml_init.InitStatic					(uiXml, "static_personal",0, &UIStaticPersonal);
//	UIStaticPersonal.Init				(1, UIPersonalWnd.GetHeight() - 175, 260, 260);

	AttachChild							(&UIActorStats);
	UIActorStats.InitFromXml			(uiXml);
//.	xml_init.InitStatic					(uiXml, "outfit_info_window",0, &UIActorStats);

	if (GameID() != GAME_SINGLE){
		UIRankFrame = xr_new<CUIStatic> (); UIRankFrame->SetAutoDelete(true);
		UIRank = xr_new<CUIStatic> (); UIRank->SetAutoDelete(true);

		CUIXmlInit::InitStatic(uiXml, "rank", 0, UIRankFrame);
		CUIXmlInit::InitStatic(uiXml, "rank:pic", 0, UIRank);
		AttachChild(UIRankFrame);
		UIRankFrame->AttachChild(UIRank);		
	}

	if (GameConstants::GetLimitedInventory())
	{
		m_ActorInvCapacityInfo			= UIHelper::CreateStatic(uiXml, "actor_inv_capacity_caption", this);
		m_ActorInvFullness				= UIHelper::CreateStatic(uiXml, "actor_inv_fullness", this);
		m_ActorInvCapacity				= UIHelper::CreateStatic(uiXml, "actor_inv_capacity", this);
		m_ActorInvCapacityInfo->AdjustWidthToText();
	}

	m_pUIBagList						= xr_new<CUIDragDropListEx>(); UIBagWnd.AttachChild(m_pUIBagList); m_pUIBagList->SetAutoDelete(true);
	xml_init.InitDragDropListEx			(uiXml, "dragdrop_bag", 0, m_pUIBagList);
	BindDragDropListEnents				(m_pUIBagList);

	m_pUIBeltList						= xr_new<CUIDragDropListEx>(); AttachChild(m_pUIBeltList); m_pUIBeltList->SetAutoDelete(true);
	xml_init.InitDragDropListEx			(uiXml, "dragdrop_belt", 0, m_pUIBeltList);
	BindDragDropListEnents				(m_pUIBeltList);

	m_pUIOutfitList						= xr_new<CUIOutfitDragDropList>(); AttachChild(m_pUIOutfitList); m_pUIOutfitList->SetAutoDelete(true);
	xml_init.InitDragDropListEx			(uiXml, "dragdrop_outfit", 0, m_pUIOutfitList);
	BindDragDropListEnents				(m_pUIOutfitList);

	m_pUIPistolList						= xr_new<CUIDragDropListEx>(); AttachChild(m_pUIPistolList); m_pUIPistolList->SetAutoDelete(true);
	xml_init.InitDragDropListEx			(uiXml, "dragdrop_pistol", 0, m_pUIPistolList);
	BindDragDropListEnents				(m_pUIPistolList);

	m_pUIAutomaticList						= xr_new<CUIDragDropListEx>(); AttachChild(m_pUIAutomaticList); m_pUIAutomaticList->SetAutoDelete(true);
	xml_init.InitDragDropListEx			(uiXml, "dragdrop_automatic", 0, m_pUIAutomaticList);
	BindDragDropListEnents				(m_pUIAutomaticList);

	if (GameConstants::GetKnifeSlotEnabled())
	{
		m_pUIKnifeList					= xr_new<CUIDragDropListEx>();
		AttachChild						(m_pUIKnifeList);
		m_pUIKnifeList->SetAutoDelete	(true);
		xml_init.InitDragDropListEx		(uiXml, "dragdrop_knife", 0, m_pUIKnifeList);
		BindDragDropListEnents			(m_pUIKnifeList);
	}

	if (GameConstants::GetTorchSlotEnabled())
	{
		m_pUITorchList					= xr_new<CUIDragDropListEx>();
		AttachChild						(m_pUITorchList);
		m_pUITorchList->SetAutoDelete	(true);
		xml_init.InitDragDropListEx		(uiXml, "dragdrop_torch", 0, m_pUITorchList);
		BindDragDropListEnents			(m_pUITorchList);
	}

	if (GameConstants::GetBinocularSlotEnabled())
	{
		m_pUIBinocularList				= xr_new<CUIDragDropListEx>();
		AttachChild						(m_pUIBinocularList);
		m_pUIBinocularList->SetAutoDelete(true);
		xml_init.InitDragDropListEx		(uiXml, "dragdrop_binocular", 0, m_pUIBinocularList);
		BindDragDropListEnents			(m_pUIBinocularList);
	}

	if (GameConstants::GetPdaSlotEnabled())
	{
		m_pUIPdaList					= xr_new<CUIDragDropListEx>();
		AttachChild						(m_pUIPdaList);
		m_pUIPdaList->SetAutoDelete(true);
		xml_init.InitDragDropListEx		(uiXml, "dragdrop_pda", 0, m_pUIPdaList);
		BindDragDropListEnents			(m_pUIPdaList);
	}

	if (GameConstants::GetDosimeterSlotEnabled())
	{
		m_pUIDosimeterList				= xr_new<CUIDragDropListEx>();
		AttachChild						(m_pUIDosimeterList);
		m_pUIDosimeterList->SetAutoDelete(true);
		xml_init.InitDragDropListEx		(uiXml, "dragdrop_dosimeter", 0, m_pUIDosimeterList);
		BindDragDropListEnents			(m_pUIDosimeterList);
	}

	if (GameConstants::GetBackpackSlotEnabled())
	{
		m_pUIBackpackList				= xr_new<CUIDragDropListEx>();
		AttachChild						(m_pUIBackpackList);
		m_pUIBackpackList->SetAutoDelete(true);
		xml_init.InitDragDropListEx		(uiXml, "dragdrop_backpack", 0, m_pUIBackpackList);
		BindDragDropListEnents			(m_pUIBackpackList);
	}

	if (GameConstants::GetPantsSlotEnabled())
	{
		m_pUIPantsList					= xr_new<CUIDragDropListEx>();
		AttachChild						(m_pUIPantsList);
		m_pUIPantsList->SetAutoDelete	(true);
		xml_init.InitDragDropListEx		(uiXml, "dragdrop_pants", 0, m_pUIPantsList);
		BindDragDropListEnents			(m_pUIPantsList);
	}

	//pop-up menu
	UIPropertiesBox							= xr_new <CUIPropertiesBox>();
	AttachChild								(UIPropertiesBox);
	UIPropertiesBox->SetAutoDelete			(true);
	UIPropertiesBox->Init					(0,0,300,300);
	UIPropertiesBox->Hide					();
	UIPropertiesBox->SetWindowName			("property_box");

	AttachChild							(&UIStaticTime);
	xml_init.InitStatic					(uiXml, "time_static", 0, &UIStaticTime);

	UIStaticTime.AttachChild			(&UIStaticTimeString);
	xml_init.InitStatic					(uiXml, "time_static_str", 0, &UIStaticTimeString);

	UIExitButton						= xr_new<CUI3tButton>();
	UIExitButton->SetAutoDelete			(true);
	AttachChild							(UIExitButton);
	xml_init.Init3tButton				(uiXml, "exit_button", 0, UIExitButton);
	UIExitButton->SetWindowName			("exit_button");

	InitHighlights						(uiXml);
	InitCallbacks						();

//Load sounds

	XML_NODE* stored_root				= uiXml.GetLocalRoot		();
	uiXml.SetLocalRoot					(uiXml.NavigateToNode		("action_sounds",0));
	::Sound->create						(sounds[eInvSndOpen],		uiXml.Read("snd_open",			0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eInvSndClose],		uiXml.Read("snd_close",			0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eInvItemToSlot],	uiXml.Read("snd_item_to_slot",	0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eInvItemToBelt],	uiXml.Read("snd_item_to_belt",	0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eInvItemToRuck],	uiXml.Read("snd_item_to_ruck",	0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eInvProperties],	uiXml.Read("snd_properties",	0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eInvDropItem],		uiXml.Read("snd_drop_item",		0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eInvAttachAddon],	uiXml.Read("snd_attach_addon",	0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eInvDetachAddon],	uiXml.Read("snd_detach_addon",	0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eInvItemUse],		uiXml.Read("snd_item_use",		0,	NULL),st_Effect,sg_SourceType);

	uiXml.SetLocalRoot					(stored_root);
}

void CUIInventoryWnd::InitCallbacks()
{
	Register(UIPropertiesBox);
	Register(UIExitButton);

	AddCallback(UIPropertiesBox->WindowName(), PROPERTY_CLICKED, CUIWndCallback::void_function(this, &CUIInventoryWnd::ProcessPropertiesBoxClicked));
	AddCallback(UIExitButton->WindowName(), BUTTON_CLICKED, CUIWndCallback::void_function(this, &CUIInventoryWnd::OnExitBtnClicked));
}

EDDListType CUIInventoryWnd::GetType(CUIDragDropListEx* l)
{
	if(l==m_pUIBagList)			return iwBag;
	if(l==m_pUIBeltList)		return iwBelt;

	if(l==m_pUIAutomaticList)	return iwSlot;
	if(l==m_pUIPistolList)		return iwSlot;
	if(l==m_pUIOutfitList)		return iwSlot;

	if (m_pUIKnifeList && l == m_pUIKnifeList)
		return iwSlot;

	if (m_pUITorchList && l == m_pUITorchList)
		return iwSlot;
	
	if (m_pUIBinocularList && l == m_pUIBinocularList)
		return iwSlot;

	if (m_pUIPdaList && l == m_pUIPdaList)
		return iwSlot;
	
	if (m_pUIDosimeterList && l == m_pUIDosimeterList)
		return iwSlot;
	
	if (m_pUIBackpackList && l == m_pUIBackpackList)
		return iwSlot;

	if (m_pUIPantsList && l == m_pUIPantsList)
		return iwSlot;

	NODEFAULT;
#ifdef DEBUG
	return iwSlot;
#endif // DEBUG
}

void CUIInventoryWnd::PlaySnd(eInventorySndAction a)
{
	if (sounds[a]._handle())
        sounds[a].play					(NULL, sm_2D);
}

CUIInventoryWnd::~CUIInventoryWnd()
{
//.	ClearDragDrop(m_vDragDropItems);
	ClearAllLists						();
}

bool CUIInventoryWnd::OnMouseAction(float x, float y, EUIMessages mouse_action)
{
	if(m_b_need_reinit)
		return true;

	//вызов дополнительного меню по правой кнопке
	if (mouse_action == WINDOW_RBUTTON_DOWN)
	{
		if (UIPropertiesBox->IsShown())
		{
			UIPropertiesBox->Hide		();
			return						true;
		}
	}

	CUIWindow::OnMouseAction					(x, y, mouse_action);

	return true; // always returns true, because ::StopAnyMove() == true;
}

void CUIInventoryWnd::Draw()
{
	CUIWindow::Draw						();
}


void CUIInventoryWnd::Update()
{
	if(m_b_need_reinit)
		InitInventory					();


	CEntityAlive *pEntityAlive			= smart_cast<CEntityAlive*>(Level().CurrentEntity());

	if(pEntityAlive) 
	{
		float v = pEntityAlive->conditions().GetHealth()*100.0f;
		UIProgressBarHealth.SetProgressPos		(v);

		v = pEntityAlive->conditions().GetPsyHealth()*100.0f;
		UIProgressBarPsyHealth.SetProgressPos	(v);

		v = pEntityAlive->conditions().GetRadiation()*100.0f;
		UIProgressBarRadiation.SetProgressPos	(v);

		CInventoryOwner* pOurInvOwner	= smart_cast<CInventoryOwner*>(pEntityAlive);
		u32 _money						= 0;

		if (GameID() != GAME_SINGLE){
			game_PlayerState* ps = Game().GetPlayerByGameID(pEntityAlive->ID());
			if (ps){
				UIProgressBarRank.SetProgressPos(ps->experience_D*100);
				_money							= ps->money_for_round;
			}
		}else
		{
			_money							= pOurInvOwner->get_money();
		}
		// update money
		string64						sMoney;
		sprintf_s							(sMoney,"%d %s", _money, *CStringTable().translate("ui_st_currency"));
		UIMoneyWnd.SetText				(sMoney);

		// update outfit parameters
		CCustomOutfit* outfit			= smart_cast<CCustomOutfit*>(pOurInvOwner->inventory().m_slots[OUTFIT_SLOT].m_pIItem);		
		UIActorStats.Update				(outfit);		
	}

	UIStaticTimeString.SetText(*InventoryUtilities::GetGameTimeAsString(InventoryUtilities::etpTimeToMinutes));

	CUIWindow::Update					();
}

void CUIInventoryWnd::Show() 
{ 
	InitInventory			();
	inherited::Show			();

	CActor* pActor = smart_cast<CActor*>(Level().CurrentEntity());

	if (!IsGameTypeSingle())
	{
		if(!pActor) return;

		pActor->SetWeaponHideState(INV_STATE_INV_WND, true);

		//rank icon		
		int team = Game().local_player->team;
		int rank = Game().local_player->rank;
		string256 _path;		
		if (GameID() != GAME_DEATHMATCH){
			if (1==team)
		        sprintf_s(_path, "ui_hud_status_green_0%d", rank+1);
			else
				sprintf_s(_path, "ui_hud_status_blue_0%d", rank+1);
		}
		else
		{
			sprintf_s(_path, "ui_hud_status_green_0%d", rank+1);
		}
		UIRank->InitTexture(_path);
	}

	SendInfoToActor						("ui_inventory");

	Update								();
	PlaySnd								(eInvSndOpen);

	if (pActor && GameConstants::GetHideWeaponInInventory())
	{
		pActor->SetWeaponHideState(INV_STATE_BLOCK_ALL, true);
	}

	if (!SSFX_UI_DoF_active)
	{
		ps_ssfx_wpn_dof_1 = GameConstants::GetSSFX_FocusDoF();
		ps_ssfx_wpn_dof_2 = GameConstants::GetSSFX_FocusDoF().z;
		SSFX_UI_DoF_active = true;
	}
}

void CUIInventoryWnd::Hide()
{
	PlaySnd								(eInvSndClose);
	inherited::Hide						();

	SendInfoToActor						("ui_inventory_hide");
	ClearAllLists						();

	//достать вещь в активный слот
	CActor *pActor = smart_cast<CActor*>(Level().CurrentEntity());
	if (pActor && m_iCurrentActiveSlot != NO_ACTIVE_SLOT && 
		pActor->inventory().m_slots[m_iCurrentActiveSlot].m_pIItem)
	{
		pActor->inventory().Activate(m_iCurrentActiveSlot);
		m_iCurrentActiveSlot = NO_ACTIVE_SLOT;
	}

	if (!IsGameTypeSingle())
	{
		if (!pActor)
			return;

		pActor->SetWeaponHideState(INV_STATE_INV_WND, false);
	}

	if (pActor && GameConstants::GetHideWeaponInInventory())
	{
		pActor->SetWeaponHideState(INV_STATE_BLOCK_ALL, false);
	}

	if (SSFX_UI_DoF_active)
	{
		ps_ssfx_wpn_dof_1 = GameConstants::GetSSFX_DefaultDoF();
		ps_ssfx_wpn_dof_2 = GameConstants::GetSSFX_DefaultDoF().z;
		SSFX_UI_DoF_active = false;
	}
	clear_highlight_lists();
}

void CUIInventoryWnd::AttachAddon(PIItem item_to_upgrade)
{
	PlaySnd										(eInvAttachAddon);
	R_ASSERT									(item_to_upgrade);
	if (OnClient())
	{
		NET_Packet								P;
		item_to_upgrade->object().u_EventGen	(P, GE_ADDON_ATTACH, item_to_upgrade->object().ID());
		P.w_u32									(CurrentIItem()->object().ID());
		item_to_upgrade->object().u_EventSend	(P);
	};

	item_to_upgrade->Attach						(CurrentIItem(), true);


	//спр€тать вещь из активного слота в инвентарь на врем€ вызова менюшки
	CActor *pActor								= smart_cast<CActor*>(Level().CurrentEntity());
	if(pActor && item_to_upgrade == pActor->inventory().ActiveItem())
	{
			m_iCurrentActiveSlot				= pActor->inventory().GetActiveSlot();
			pActor->inventory().Activate		(NO_ACTIVE_SLOT);
	}
	SetCurrentItem								(NULL);
}

void CUIInventoryWnd::DetachAddon(LPCSTR addon_name, PIItem itm)
{
	PlaySnd										(eInvDetachAddon);
	if (OnClient())
	{
		NET_Packet								P;
		if (itm == NULL)
			CGameObject::u_EventGen(P, GE_ADDON_DETACH, CurrentIItem()->object().ID());
		else
			CGameObject::u_EventGen(P, GE_ADDON_DETACH, itm->object().ID());

		P.w_stringZ(addon_name);
		CGameObject::u_EventSend(P);
		return;
	}
	if (itm == NULL)
		CurrentIItem()->Detach(addon_name, true);
	else
		itm->Detach(addon_name, true);

	//спр€тать вещь из активного слота в инвентарь на врем€ вызова менюшки
	CActor *pActor								= smart_cast<CActor*>(Level().CurrentEntity());
	if(pActor && CurrentIItem() == pActor->inventory().ActiveItem())
	{
			m_iCurrentActiveSlot				= pActor->inventory().GetActiveSlot();
			pActor->inventory().Activate		(NO_ACTIVE_SLOT);
	}
}


void	CUIInventoryWnd::SendEvent_ActivateSlot	(PIItem	pItem)
{
	NET_Packet						P;
	pItem->object().u_EventGen		(P, GEG_PLAYER_ACTIVATE_SLOT, pItem->object().H_Parent()->ID());
	P.w_u32							(pItem->GetSlot());
	pItem->object().u_EventSend		(P);
}

void	CUIInventoryWnd::SendEvent_Item2Slot			(PIItem	pItem)
{
	NET_Packet						P;
	pItem->object().u_EventGen		(P, GEG_PLAYER_ITEM2SLOT, pItem->object().H_Parent()->ID());
	P.w_u16							(pItem->object().ID());
	pItem->object().u_EventSend		(P);
	g_pInvWnd->PlaySnd				(eInvItemToSlot);
};

void	CUIInventoryWnd::SendEvent_Item2Belt			(PIItem	pItem)
{
	NET_Packet						P;
	pItem->object().u_EventGen		(P, GEG_PLAYER_ITEM2BELT, pItem->object().H_Parent()->ID());
	P.w_u16							(pItem->object().ID());
	pItem->object().u_EventSend		(P);
	g_pInvWnd->PlaySnd				(eInvItemToBelt);
};

void	CUIInventoryWnd::SendEvent_Item2Ruck			(PIItem	pItem)
{
	NET_Packet						P;
	pItem->object().u_EventGen		(P, GEG_PLAYER_ITEM2RUCK, pItem->object().H_Parent()->ID());
	P.w_u16							(pItem->object().ID());
	pItem->object().u_EventSend		(P);

	g_pInvWnd->PlaySnd				(eInvItemToRuck);
};

void	CUIInventoryWnd::SendEvent_Item_Drop(PIItem	pItem)
{
	pItem->SetDropManual			(TRUE);

	if( OnClient() )
	{
		NET_Packet					P;
		pItem->object().u_EventGen	(P, GE_OWNERSHIP_REJECT, pItem->object().H_Parent()->ID());
		P.w_u16						(pItem->object().ID());
		pItem->object().u_EventSend(P);
	}
	g_pInvWnd->PlaySnd				(eInvDropItem);
};

void	CUIInventoryWnd::SendEvent_Item_Eat			(PIItem	pItem)
{
	R_ASSERT						(pItem->m_pCurrentInventory==m_pInv);
	NET_Packet						P;
	pItem->object().u_EventGen		(P, GEG_PLAYER_ITEM_EAT, pItem->object().H_Parent()->ID());
	P.w_u16							(pItem->object().ID());
	pItem->object().u_EventSend		(P);
};


void CUIInventoryWnd::BindDragDropListEnents(CUIDragDropListEx* lst)
{
	lst->m_f_item_drop				= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUIInventoryWnd::OnItemDrop);
	lst->m_f_item_start_drag		= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUIInventoryWnd::OnItemStartDrag);
	lst->m_f_item_db_click			= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUIInventoryWnd::OnItemDbClick);
	lst->m_f_item_selected			= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUIInventoryWnd::OnItemSelected);
	lst->m_f_item_rbutton_click		= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUIInventoryWnd::OnItemRButtonClick);
	lst->m_f_item_focus_received	= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUIInventoryWnd::OnItemFocusReceive);
	lst->m_f_item_focus_lost		= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUIInventoryWnd::OnItemFocusLost);
	lst->m_f_item_focused_update	= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUIInventoryWnd::OnItemFocusedUpdate);
}


#include "../xr_level_controller.h"
#include <dinput.h>

bool CUIInventoryWnd::OnKeyboardAction(int dik, EUIMessages keyboard_action)
{
	if(m_b_need_reinit)
		return true;

	if (UIPropertiesBox->GetVisible())
		UIPropertiesBox->OnKeyboardAction(dik, keyboard_action);

	if (is_binded(kDROP, dik))
	{
		if (WINDOW_KEY_PRESSED==keyboard_action)
			DropCurrentItem(false);
		return true;
	}

	if (is_binded(kUSE, dik) || is_binded(kINVENTORY, dik) || is_binded(kQUIT, dik))
	{
		if (WINDOW_KEY_PRESSED == keyboard_action)
		{
			CCustomBackpack* backpack = smart_cast<CCustomBackpack*>(Actor()->inventory().ItemFromSlot(BACKPACK_SLOT));
			if (GameConstants::GetBackpackAnimsEnabled() && backpack)
			{
				if (Actor()->inventory().GetActiveSlot() == BACKPACK_SLOT && Actor()->inventory().ActiveItem())
				{
					Actor()->inventory().Activate(NO_ACTIVE_SLOT);
				}

				GetHolder()->StartStopMenu(this, true);
			}
			else
			{
				GetHolder()->StartStopMenu(this, true);
			}
		}
		return true;
	}

	if (WINDOW_KEY_PRESSED == keyboard_action)
	{
#ifdef DEBUG
		CTorch* flashlight = smart_cast<CTorch*>(CurrentIItem());
		CDetectorAnomaly* anomaly_detector = smart_cast<CDetectorAnomaly*>(CurrentIItem());
		CBattery* battery = smart_cast<CBattery*>(CurrentIItem());
		if(DIK_NUMPAD7 == dik && CurrentIItem())
		{
			if (flashlight && GameConstants::GetTorchHasBattery() || anomaly_detector && GameConstants::GetAnoDetectorUseBattery() || battery)
			{
				CurrentIItem()->ChangeChargeLevel(-0.05f);
			}
			else
				CurrentIItem()->ChangeCondition(-0.05f);
			UIItemInfo.InitItem(CurrentIItem());
		}
		else if(DIK_NUMPAD8 == dik && CurrentIItem())
		{
			if (flashlight && GameConstants::GetTorchHasBattery() || anomaly_detector && GameConstants::GetAnoDetectorUseBattery() || battery)
			{
				CurrentIItem()->ChangeChargeLevel(0.05f);
			}
			else
				CurrentIItem()->ChangeCondition(0.05f);
			UIItemInfo.InitItem(CurrentIItem());
		}
#endif
	}
	if( inherited::OnKeyboardAction(dik,keyboard_action) )return true;

	return false;
}
