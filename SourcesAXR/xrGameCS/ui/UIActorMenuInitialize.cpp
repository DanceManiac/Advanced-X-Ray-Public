#include "stdafx.h"
#include "UIActorMenu.h"
#include "UIXmlInit.h"
#include "xrUIXmlParser.h"
#include "UICharacterInfo.h"
#include "UIDragDropListEx.h"
#include "UIDragDropReferenceList.h"
#include "UIActorStateInfo.h"
#include "UIItemInfo.h"
#include "UIFrameLineWnd.h"
#include "UIMessageBoxEx.h"
#include "UIPropertiesBox.h"
#include "UI3tButton.h"

#include "UIInventoryUpgradeWnd.h"
#include "UIInvUpgradeInfo.h"

#include "ai_space.h"
#include "alife_simulator.h"
#include "object_broker.h"
#include "UIWndCallback.h"
#include "UIHelper.h"
#include "ui_base.h"
#include "../string_table.h"
#include "AdvancedXrayGameConstants.h"
#include "../HUDManager.h"

CUIActorMenu::CUIActorMenu()
{
	m_currMenuMode					= mmUndefined;
	m_trade_partner_inventory_state = 0;
	Construct						();
}

CUIActorMenu::~CUIActorMenu()
{
	xr_delete			(m_message_box_yes_no);
	xr_delete			(m_message_box_ok);
	xr_delete			(m_UIPropertiesBox);
	xr_delete			(m_hint_wnd);
	xr_delete			(m_ItemInfo);

	if (m_bBeltSlotsOverInitialized)
		m_belt_list_over.clear();
	if (m_bArtefactSlotsHighlightInitialized)
		m_ArtefactSlotsHighlight.clear();

	ClearAllLists		();
}

void CUIActorMenu::Construct()
{
	CUIXml								uiXml;

	if (psHUD_Flags.test(ALTERNATIV_INVENTORY))
	{
		uiXml.Load(CONFIG_PATH, UI_PATH, "actor_menu_alternativ.xml");
	}
	else
	{
		uiXml.Load(CONFIG_PATH, UI_PATH, "actor_menu.xml");
	}

	CUIXmlInit							xml_init;

	xml_init.InitWindow					(uiXml, "main", 0, this);
	m_hint_wnd = UIHelper::CreateHint	(uiXml, "hint_wnd");

	m_LeftBackground					= xr_new<CUIStatic>();
	m_LeftBackground->SetAutoDelete		(true);
	AttachChild							(m_LeftBackground);
	xml_init.InitStatic					(uiXml, "left_background", 0, m_LeftBackground);

	m_pUpgradeWnd						= xr_new<CUIInventoryUpgradeWnd>(); 
	AttachChild							(m_pUpgradeWnd);
	m_pUpgradeWnd->SetAutoDelete		(true);
	m_pUpgradeWnd->Init					();

	m_ActorCharacterInfo				= xr_new<CUICharacterInfo>();
	m_ActorCharacterInfo->SetAutoDelete	(true);
	AttachChild							(m_ActorCharacterInfo);
	m_ActorCharacterInfo->InitCharacterInfo(&uiXml, "actor_ch_info");

	m_PartnerCharacterInfo				= xr_new<CUICharacterInfo>();
	m_PartnerCharacterInfo->SetAutoDelete(true);
	AttachChild							(m_PartnerCharacterInfo);
	m_PartnerCharacterInfo->InitCharacterInfo( &uiXml, "partner_ch_info" );
	
	m_RightDelimiter					= UIHelper::CreateStatic(uiXml, "right_delimiter", this);
	m_ActorTradeCaption					= UIHelper::CreateStatic(uiXml, "right_delimiter:trade_caption", m_RightDelimiter);
	m_ActorTradePrice					= UIHelper::CreateStatic(uiXml, "right_delimiter:trade_price", m_RightDelimiter);
	m_ActorTradeWeightMax				= UIHelper::CreateStatic(uiXml, "right_delimiter:trade_weight_max", m_RightDelimiter);
	m_ActorTradeCaption->AdjustWidthToText();
	
	m_LeftDelimiter						= UIHelper::CreateStatic(uiXml, "left_delimiter", this);
	m_PartnerTradeCaption				= UIHelper::CreateStatic(uiXml, "left_delimiter:trade_caption", m_LeftDelimiter);
	m_PartnerTradePrice					= UIHelper::CreateStatic(uiXml, "left_delimiter:trade_price", m_LeftDelimiter);
	m_PartnerTradeWeightMax				= UIHelper::CreateStatic(uiXml, "left_delimiter:trade_weight_max", m_LeftDelimiter);
	m_PartnerTradeCaption->AdjustWidthToText();

	m_ActorBottomInfo					= UIHelper::CreateStatic(uiXml, "actor_weight_caption", this);
	m_ActorWeight						= UIHelper::CreateStatic(uiXml, "actor_weight", this);
	m_ActorWeightMax					= UIHelper::CreateStatic(uiXml, "actor_weight_max", this);
	m_ActorBottomInfo->AdjustWidthToText();

	m_PartnerBottomInfo					= UIHelper::CreateStatic(uiXml, "partner_weight_caption", this);
	m_PartnerWeight						= UIHelper::CreateStatic(uiXml, "partner_weight", this);
	m_PartnerBottomInfo->AdjustWidthToText();
	m_PartnerWeight_end_x 				= m_PartnerWeight->GetWndPos().x;

	if (GameConstants::GetLimitedInvBoxes())
	{
		m_PartnerInvCapacityInfo	= UIHelper::CreateStatic(uiXml, "partner_capacity_caption", this);
		m_PartnerInvFullness		= UIHelper::CreateStatic(uiXml, "partner_inv_fullness", this);
		m_PartnerInvCapacity		= UIHelper::CreateStatic(uiXml, "partner_inv_capacity", this);
		m_PartnerInvCapacityInfo->AdjustWidthToText();
	}

	if (GameConstants::GetLimitedInventory())
	{
		if (m_ActorInvCapacityInfo	= UIHelper::CreateStatic(uiXml, "actor_inv_capacity_caption", this, false))
			m_ActorInvCapacityInfo->AdjustWidthToText();
			
		m_ActorInvFullness		= UIHelper::CreateStatic(uiXml, "actor_inv_fullness", this, false);
		m_ActorInvCapacity		= UIHelper::CreateStatic(uiXml, "actor_inv_capacity", this, false);
	}

	m_PistolSlotHighlight = UIHelper::CreateStatic(uiXml, "pistol_slot_highlight", this);
	m_PistolSlotHighlight->Show(false);
	m_Riffle1Highlight = UIHelper::CreateStatic(uiXml, "riffle1_slot_highlight", this);
	m_Riffle1Highlight->Show(false);
	m_Riffle2Highlight = UIHelper::CreateStatic(uiXml, "riffle2_slot_highlight", this);
	m_Riffle2Highlight->Show(false);
	m_OutfitSlotHighlight = UIHelper::CreateStatic(uiXml, "outfit_slot_highlight", this);
	m_OutfitSlotHighlight->Show(false);
	m_DetectorSlotHighlight = UIHelper::CreateStatic(uiXml, "detector_slot_highlight", this);
	m_DetectorSlotHighlight->Show(false);
	m_QuickSlotsHighlight[0] = UIHelper::CreateStatic(uiXml, "quick_slot_highlight", this);
	m_QuickSlotsHighlight[0]->Show(false);
	m_KnifeSlotHighlight = UIHelper::CreateStatic(uiXml, "knife_slot_highlight", this);
	m_KnifeSlotHighlight->Show(false);
	m_BinocularSlotHighlight = UIHelper::CreateStatic(uiXml, "binoc_slot_highlight", this);
	m_BinocularSlotHighlight->Show(false);
	m_TorchSlotHighlight = UIHelper::CreateStatic(uiXml, "torch_slot_highlight", this);
	m_TorchSlotHighlight->Show(false);
	m_BackpackSlotHighlight = UIHelper::CreateStatic(uiXml, "backpack_slot_highlight", this);
	m_BackpackSlotHighlight->Show(false);
	m_DosimeterSlotHighlight = UIHelper::CreateStatic(uiXml, "dosimeter_slot_highlight", this);
	m_DosimeterSlotHighlight->Show(false);
	m_PantsSlotHighlight = UIHelper::CreateStatic(uiXml, "pants_slot_highlight", this);
	m_PantsSlotHighlight->Show(false);
	m_HelmetSlotHighlight = UIHelper::CreateStatic(uiXml, "helmet_slot_highlight", this);
	m_HelmetSlotHighlight->Show(false);
	m_SecondHelmetSlotHighlight = UIHelper::CreateStatic(uiXml, "second_helmet_slot_highlight", this);
	m_SecondHelmetSlotHighlight->Show(false);
	m_DeviceSlotHighlight = UIHelper::CreateStatic(uiXml, "device_slot_highlight", this);
	m_DeviceSlotHighlight->Show(false);

	m_pInventoryDeviceList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_device", this);
	m_pInventoryHelmetList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_helmet", this);
	m_pInventorySecondHelmetList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_second_helmet", this);
	m_pInventoryPantsList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_pants", this);
	m_pInventoryDosimeterList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_dosimeter", this);
	m_pInventoryKnifeList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_knife", this);
	m_pInventoryBinocularList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_bino", this);
	m_pInventoryTorchList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_torch", this);
	m_pInventoryBackpackList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_backpack", this);
	m_pInventoryBagList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_bag", this);
	m_pInventoryBeltList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_belt", this);
	m_pInventoryOutfitList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_outfit", this);
	m_pInventoryDetectorList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_detector", this);
	m_pInventoryPistolList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_pistol", this);
	m_pInventorySmgList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_smg", this);
	m_pInventoryBoltList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_bolt", this);
	m_pInventoryAutomaticList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_automatic", this);
	m_pTradeActorBagList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_actor_trade_bag", this);
	m_pTradeActorList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_actor_trade", this);
	m_pTradePartnerBagList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_partner_bag", this);
	m_pTradePartnerList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_partner_trade", this);
	m_pDeadBodyBagList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_deadbody_bag", this);

	if (m_pQuickSlot				= UIHelper::CreateDragDropReferenceList(uiXml, "dragdrop_quick_slots", this, false))
	{
		m_quick_vert_attrib			= uiXml.ReadAttrib		("dragdrop_quick_slots", 0, "horizontal", "");
		b_quick_vert				= (0==stricmp(m_quick_vert_attrib, "false") || 0==stricmp(m_quick_vert_attrib, "0"));
		m_pQuickSlot->Initialize	(!b_quick_vert);
	}

	Fvector2 pos{};
	float dx = 0.0f, dy = 0.0f;

	if (m_QuickSlotsHighlight[0])
	{
		pos		= m_QuickSlotsHighlight[0]->GetWndPos();
		dx		= uiXml.ReadAttribFlt("quick_slot_highlight", 0, "dx", 24.0f);
		dy		= uiXml.ReadAttribFlt("quick_slot_highlight", 0, "dy", 0.0f);
		for(u8 i=1;i<6;i++)
		{
			pos.x						+= dx;
			pos.y						+= dy;
			m_QuickSlotsHighlight[i]	= UIHelper::CreateStatic(uiXml, "quick_slot_highlight", this);
			m_QuickSlotsHighlight[i]	->SetWndPos(pos);
			m_QuickSlotsHighlight[i]	->Show(false);
		}
	}

	int cols = m_pInventoryBeltList->CellsCapacity().x;
	int rows = m_pInventoryBeltList->CellsCapacity().y;
	int counter = 1;

	if (uiXml.NavigateToNode("artefact_slot_highlight", 0))
	{
		m_bArtefactSlotsHighlightInitialized = true;
		dx = uiXml.ReadAttribFlt("artefact_slot_highlight", 0, "dx", 24.0f);
		dy = uiXml.ReadAttribFlt("artefact_slot_highlight", 0, "dy", 24.0f);
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
	}

	if (m_pTrashList				= UIHelper::CreateDragDropListEx(uiXml, "dragdrop_trash", this, false))
	{
		m_pTrashList->m_f_item_drop	= CUIDragDropListEx::DRAG_CELL_EVENT	(this,&CUIActorMenu::OnItemDrop);
		m_pTrashList->m_f_drag_event= CUIDragDropListEx::DRAG_ITEM_EVENT	(this,&CUIActorMenu::OnDragItemOnTrash);
	}

	if (uiXml.NavigateToNode("belt_list_over", 0))
	{
		m_bBeltSlotsOverInitialized = true;
		counter = 1;

		for (u8 i = 0; i < rows; ++i)
		{
			for (u8 j = 0; j < cols; ++j)
			{
				m_belt_list_over.push_back(UIHelper::CreateStatic(uiXml, "belt_list_over", this));

				if (i == 0 && j == 0)
				{
					pos = m_belt_list_over[0]->GetWndPos();
					dx = uiXml.ReadAttribFlt("belt_list_over", 0, "dx", 10.0f);
					dy = uiXml.ReadAttribFlt("belt_list_over", 0, "dy", 10.0f);
				}
				else
				{
					if (j != 0)
						pos.x += dx;

					m_belt_list_over[counter]->SetWndPos(pos);
					counter++;
				}
			}

			pos.x = m_belt_list_over[0]->GetWndPos().x;
			pos.y += dy;
		}
	}

	m_HelmetOver = UIHelper::CreateStatic(uiXml, "helmet_over", this);
	m_HelmetOver->Show(false);

	m_SecondHelmetOver = UIHelper::CreateStatic(uiXml, "second_helmet_over", this);
	m_SecondHelmetOver->Show(false);

	m_ActorMoney	= UIHelper::CreateStatic(uiXml, "actor_money_static", this);
	m_PartnerMoney	= UIHelper::CreateStatic(uiXml, "partner_money_static", this);
	m_QuickSlot1	= UIHelper::CreateStatic(uiXml, "quick_slot1_text", this, false);
	m_QuickSlot2	= UIHelper::CreateStatic(uiXml, "quick_slot2_text", this, false);
	m_QuickSlot3	= UIHelper::CreateStatic(uiXml, "quick_slot3_text", this, false);
	m_QuickSlot4	= UIHelper::CreateStatic(uiXml, "quick_slot4_text", this, false);
	m_QuickSlot5	= UIHelper::CreateStatic(uiXml, "quick_slot5_text", this, false);
	m_QuickSlot6	= UIHelper::CreateStatic(uiXml, "quick_slot6_text", this, false);


	m_KnifeSlot_progress = UIHelper::CreateProgressBar(uiXml, "progress_bar_slot_knife", this);
	m_PistolSlot_progress = UIHelper::CreateProgressBar(uiXml, "progress_bar_slot_pistol", this);
	m_SMGSlot_progress = UIHelper::CreateProgressBar(uiXml, "progress_bar_slot_mps", this);
	m_RifleSlot_progress = UIHelper::CreateProgressBar(uiXml, "progress_bar_slot_rifle", this);
	m_Outfit_progress = UIHelper::CreateProgressBar(uiXml, "progress_bar_slot_outfit", this);
	m_Helmet_progress = UIHelper::CreateProgressBar(uiXml, "progress_bar_slot_helmet", this);
	m_SecondHelmet_progress = UIHelper::CreateProgressBar(uiXml, "progress_bar_slot_second_helmet", this);
	m_Pants_progress = UIHelper::CreateProgressBar(uiXml, "progress_bar_slot_pants", this);
	m_TORCHSlot_progress = UIHelper::CreateProgressBar(uiXml, "progress_bar_slot_torch", this);
	m_DETECTORSlot_progress = UIHelper::CreateProgressBar(uiXml, "progress_bar_slot_detector", this);
	m_DOSIMETERSlot_progress = UIHelper::CreateProgressBar(uiXml, "progress_bar_slot_dosimeter", this);

	m_trade_button		= UIHelper::Create3tButtonEx(uiXml, "trade_button", this, false);
	m_takeall_button	= UIHelper::Create3tButtonEx(uiXml, "takeall_button", this);
	m_exit_button		= UIHelper::Create3tButtonEx(uiXml, "exit_button", this);
	m_sleep_button		= UIHelper::Create3tButtonEx(uiXml, "sleep_button", this, false);

	m_clock_value		= UIHelper::CreateStatic(uiXml, "clock_value", this, false);

/*
	m_pDeadBodyBagList					= xr_new<CUIDragDropListEx>(); 
	AttachChild							(m_pDeadBodyBagList);
	m_pDeadBodyBagList->SetAutoDelete	(true);
	xml_init.InitDragDropListEx			(uiXml, "dragdrop_deadbody_bag", 0, m_pDeadBodyBagList);
*/
	m_ActorStateInfo					= xr_new<ui_actor_state_wnd>();
	m_ActorStateInfo->init_from_xml		(uiXml, "actor_state_info");
	m_ActorStateInfo->SetAutoDelete		(true);
	AttachChild							(m_ActorStateInfo); 

	XML_NODE* stored_root				= uiXml.GetLocalRoot	();
	uiXml.SetLocalRoot					(uiXml.NavigateToNode	("action_sounds",0));
	::Sound->create						(sounds[eSndOpen],		uiXml.Read("snd_open",			0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eSndClose],		uiXml.Read("snd_close",			0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eItemToSlot],	uiXml.Read("snd_item_to_slot",	0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eItemToBelt],	uiXml.Read("snd_item_to_belt",	0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eItemToRuck],	uiXml.Read("snd_item_to_ruck",	0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eProperties],	uiXml.Read("snd_properties",	0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eDropItem],		uiXml.Read("snd_drop_item",		0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eAttachAddon],	uiXml.Read("snd_attach_addon",	0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eDetachAddon],	uiXml.Read("snd_detach_addon",	0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eItemUse],		uiXml.Read("snd_item_use",		0,	NULL),st_Effect,sg_SourceType);
	uiXml.SetLocalRoot					(stored_root);

	m_ItemInfo							= xr_new<CUIItemInfo>();
//-	m_ItemInfo->SetAutoDelete			(true);
//-	AttachChild							(m_ItemInfo);
	m_ItemInfo->InitItemInfo			("actor_menu_item.xml");

	m_upgrade_info						= NULL;
	if ( ai().get_alife() )
	{
		m_upgrade_info						= xr_new<UIInvUpgradeInfo>();
		m_upgrade_info->SetAutoDelete		(true);
		AttachChild							(m_upgrade_info);
		m_upgrade_info->init_from_xml		("actor_menu_item.xml");
	}

	m_message_box_yes_no				= xr_new<CUIMessageBoxEx>();	
	m_message_box_yes_no->InitMessageBox( "message_box_yes_no" );
	m_message_box_yes_no->SetAutoDelete	(true);
	m_message_box_yes_no->SetText		( "" );

	m_message_box_ok					= xr_new<CUIMessageBoxEx>();	
	m_message_box_ok->InitMessageBox	( "message_box_ok" );
	m_message_box_ok->SetAutoDelete		(true);
	m_message_box_ok->SetText			( "" );

	m_UIPropertiesBox					= xr_new<CUIPropertiesBox>();
	m_UIPropertiesBox->InitPropertiesBox(Fvector2().set(0,0),Fvector2().set(300,300));
	AttachChild							(m_UIPropertiesBox);
	m_UIPropertiesBox->Hide				();
	m_UIPropertiesBox->SetWindowName	( "property_box" );

	InitCallbacks						();

	BindDragDropListEvents(m_pInventoryBeltList);
	BindDragDropListEvents(m_pInventoryBoltList);
	BindDragDropListEvents(m_pInventoryPistolList);
	BindDragDropListEvents(m_pInventorySmgList);
	BindDragDropListEvents(m_pInventoryAutomaticList);
	BindDragDropListEvents(m_pInventoryOutfitList);
	BindDragDropListEvents(m_pInventoryDetectorList);
	BindDragDropListEvents(m_pInventoryBagList);
	BindDragDropListEvents(m_pTradeActorBagList);
	BindDragDropListEvents(m_pTradeActorList);
	BindDragDropListEvents(m_pTradePartnerBagList);
	BindDragDropListEvents(m_pTradePartnerList);
	BindDragDropListEvents(m_pDeadBodyBagList);
	BindDragDropListEvents(m_pQuickSlot);
	BindDragDropListEvents(m_pInventoryKnifeList);
	BindDragDropListEvents(m_pInventoryBinocularList);
	BindDragDropListEvents(m_pInventoryTorchList);
	BindDragDropListEvents(m_pInventoryBackpackList);
	BindDragDropListEvents(m_pInventoryDosimeterList);
	BindDragDropListEvents(m_pInventoryPantsList);
	BindDragDropListEvents(m_pInventoryHelmetList);
	BindDragDropListEvents(m_pInventorySecondHelmetList);
	BindDragDropListEvents(m_pInventoryDeviceList);


	m_allowed_drops[iTrashSlot].push_back(iActorBag);
	m_allowed_drops[iTrashSlot].push_back(iActorSlot);
	m_allowed_drops[iTrashSlot].push_back(iActorBelt);
	m_allowed_drops[iTrashSlot].push_back(iQuickSlot);

	m_allowed_drops[iActorSlot].push_back(iActorBag);
	m_allowed_drops[iActorSlot].push_back(iActorSlot);
	m_allowed_drops[iActorSlot].push_back(iActorTrade);
	m_allowed_drops[iActorSlot].push_back(iDeadBodyBag);

	m_allowed_drops[iActorBag].push_back(iActorSlot);
	m_allowed_drops[iActorBag].push_back(iActorBelt);
	m_allowed_drops[iActorBag].push_back(iActorTrade);
	m_allowed_drops[iActorBag].push_back(iDeadBodyBag);
	m_allowed_drops[iActorBag].push_back(iActorBag);
	m_allowed_drops[iActorBag].push_back(iQuickSlot);
	
	m_allowed_drops[iActorBelt].push_back(iActorBag);
	m_allowed_drops[iActorBelt].push_back(iActorTrade);
	m_allowed_drops[iActorBelt].push_back(iDeadBodyBag);
	m_allowed_drops[iActorBelt].push_back(iActorBelt);

	m_allowed_drops[iActorTrade].push_back(iActorSlot);
	m_allowed_drops[iActorTrade].push_back(iActorBag);
	m_allowed_drops[iActorTrade].push_back(iActorBelt);
	m_allowed_drops[iActorTrade].push_back(iActorTrade);
	m_allowed_drops[iActorTrade].push_back(iQuickSlot);

	m_allowed_drops[iPartnerTradeBag].push_back(iPartnerTrade);
	m_allowed_drops[iPartnerTradeBag].push_back(iPartnerTradeBag);
	m_allowed_drops[iPartnerTrade].push_back(iPartnerTradeBag);
	m_allowed_drops[iPartnerTrade].push_back(iPartnerTrade);

	m_allowed_drops[iDeadBodyBag].push_back(iActorSlot);
	m_allowed_drops[iDeadBodyBag].push_back(iActorBag);
	m_allowed_drops[iDeadBodyBag].push_back(iActorBelt);
	m_allowed_drops[iDeadBodyBag].push_back(iDeadBodyBag);

	m_allowed_drops[iQuickSlot].push_back(iActorBag);
	m_allowed_drops[iQuickSlot].push_back(iActorTrade);
	m_allowed_drops[iQuickSlot].push_back(iQuickSlot);

	m_upgrade_selected					= NULL;
	SetCurrentItem						(NULL);
	SetActor							(NULL);
	SetPartner							(NULL);
	SetInvBox							(NULL);

	m_actor_trade						= NULL;
	m_partner_trade						= NULL;
	m_repair_mode						= false;
	m_item_info_view					= false;
	m_highlight_clear					= true;

	DeInitInventoryMode					();
	DeInitTradeMode						();
	DeInitUpgradeMode					();
	DeInitDeadBodySearchMode			();
}

void CUIActorMenu::BindDragDropListEvents(CUIDragDropListEx* lst)
{
	lst->m_f_item_drop				= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUIActorMenu::OnItemDrop);
	lst->m_f_item_start_drag		= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUIActorMenu::OnItemStartDrag);
	lst->m_f_item_db_click			= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUIActorMenu::OnItemDbClick);
	lst->m_f_item_selected			= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUIActorMenu::OnItemSelected);
	lst->m_f_item_rbutton_click		= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUIActorMenu::OnItemRButtonClick);
	lst->m_f_item_mbutton_click		= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUIActorMenu::OnItemMButtonClick);
	lst->m_f_item_focus_received	= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUIActorMenu::OnItemFocusReceive);
	lst->m_f_item_focus_lost		= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUIActorMenu::OnItemFocusLost);
	lst->m_f_item_focused_update	= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUIActorMenu::OnItemFocusedUpdate);
}

void CUIActorMenu::InitCallbacks()
{
	if (m_trade_button)
		Register						(m_trade_button );
	Register						(m_takeall_button);
	Register						(m_exit_button);
	Register						(m_UIPropertiesBox);
	VERIFY							(m_pUpgradeWnd);
	Register						(m_pUpgradeWnd->m_btn_repair);
	if (m_sleep_button)
		Register(m_sleep_button);

	if (m_trade_button)
		AddCallback( m_trade_button->WindowName(),    BUTTON_CLICKED,   CUIWndCallback::void_function( this, &CUIActorMenu::OnBtnPerformTrade ) );
	AddCallback( m_takeall_button->WindowName(),  BUTTON_CLICKED,   CUIWndCallback::void_function( this, &CUIActorMenu::TakeAllFromPartner ) );
	AddCallback( m_exit_button->WindowName(),     BUTTON_CLICKED,   CUIWndCallback::void_function( this, &CUIActorMenu::OnBtnExitClicked ) );
	AddCallback( m_UIPropertiesBox->WindowName(), PROPERTY_CLICKED, CUIWndCallback::void_function( this, &CUIActorMenu::ProcessPropertiesBoxClicked ) );
	AddCallback( m_pUpgradeWnd->m_btn_repair->WindowName(), BUTTON_CLICKED,   CUIWndCallback::void_function( this, &CUIActorMenu::TryRepairItem ) );
	if (m_sleep_button)
		AddCallback(m_sleep_button->WindowName(),    BUTTON_CLICKED,   CUIWndCallback::void_function(this, &CUIActorMenu::OnBtnSleepClicked));
}

void CUIActorMenu::UpdateButtonsLayout()
{
	Fvector2 btn_exit_pos;
	if((m_trade_button && m_trade_button->IsShown()) || m_takeall_button->IsShown())
	{
		btn_exit_pos	= m_trade_button->GetWndPos();
		btn_exit_pos.x	+=m_trade_button->GetWndSize().x;
	}else
	{
		btn_exit_pos	= m_trade_button->GetWndPos();
		btn_exit_pos.x	+=m_trade_button->GetWndSize().x/2.0f;
	}
	
	m_exit_button->SetWndPos(btn_exit_pos);

	UpdateConditionProgressBars();

	string32 tmp;
	LPCSTR str;

	if (m_QuickSlot1)
	{
		
		str = CStringTable().translate("quick_use_str_1").c_str();
		strncpy_s(tmp, sizeof(tmp), str, 3);
		if(tmp[2]==',')
			tmp[1] = '\0';
		m_QuickSlot1->SetTextST(tmp);
	}
	
	if (m_QuickSlot2)
	{
		str = CStringTable().translate("quick_use_str_2").c_str();
		strncpy_s(tmp, sizeof(tmp), str, 3);
		if(tmp[2]==',')
			tmp[1] = '\0';
		m_QuickSlot2->SetTextST(tmp);
	}
	
	if (m_QuickSlot3)
	{
		str = CStringTable().translate("quick_use_str_3").c_str();
		strncpy_s(tmp, sizeof(tmp), str, 3);
		if(tmp[2]==',')
			tmp[1] = '\0';
		m_QuickSlot3->SetTextST(tmp);
	}
	
	if (m_QuickSlot4)
	{
		str = CStringTable().translate("quick_use_str_4").c_str();
		strncpy_s(tmp, sizeof(tmp), str, 3);
		if(tmp[2]==',')
			tmp[1] = '\0';
		m_QuickSlot4->SetTextST(tmp);
	}


	if (m_QuickSlot5)
	{
		str = CStringTable().translate("quick_use_str_5").c_str();
		strncpy_s(tmp, sizeof(tmp), str, 3);
		if (tmp[2] == ',')
			tmp[1] = '\0';
		m_QuickSlot5->SetTextST(tmp);
	}

	if (m_QuickSlot6)
	{
		str = CStringTable().translate("quick_use_str_6").c_str();
		strncpy_s(tmp, sizeof(tmp), str, 3);
		if (tmp[2] == ',')
			tmp[1] = '\0';
		m_QuickSlot6->SetTextST(tmp);
	}

}