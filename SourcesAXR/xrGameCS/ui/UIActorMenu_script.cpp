////////////////////////////////////////////////////////////////////////////
//	Module 		: UIActorMenu_script.cpp
//	Created 	: 18.04.2008
//	Author		: Evgeniy Sokolov
//	Description : UI ActorMenu script implementation
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "UIActorMenu.h"
#include "UIPdaWnd.h"
#include "../actor.h"
#include "../inventory_item.h"
#include "../ai_space.h"
#include "../../XrServerEntitiesCS/script_engine.h"

#include "UITabControl.h"
#include "UIMainIngameWnd.h"
#include "UIActorMenu.h"
#include "UIZoneMap.h"
#include "UIMotionIcon.h"
#include "UIHudStatesWnd.h"
#include "UIGameCustom.h"
#include "InventoryBox.h"
#include "HUDManager.h"

using namespace luabind;

void CUIActorMenu::TryRepairItem(CUIWindow* w, void* d)
{
	PIItem item = get_upgrade_item();
	if ( !item )
	{
		return;
	}
	if ( item->GetCondition() > 0.99f )
	{
		return;
	}
	LPCSTR item_name = item->m_section_id.c_str();
	LPCSTR partner = m_pPartnerInvOwner->CharacterInfo().Profile().c_str();

	luabind::functor<bool> funct;
	R_ASSERT2(
		ai().script_engine().functor( "inventory_upgrades.can_repair_item", funct ),
		make_string( "Failed to get functor <inventory_upgrades.can_repair_item>, item = %s", item_name )
		);
	bool can_repair = funct( item_name, item->GetCondition(), partner );

	luabind::functor<LPCSTR> funct2;
	R_ASSERT2(
		ai().script_engine().functor( "inventory_upgrades.question_repair_item", funct2 ),
		make_string( "Failed to get functor <inventory_upgrades.question_repair_item>, item = %s", item_name )
		);
	LPCSTR question = funct2( item_name, item->GetCondition(), can_repair, partner );

	m_repair_mode = true;
	if ( can_repair )
	{
		CallMessageBoxYesNo( question );
	} 
	else
	{
		CallMessageBoxOK( question );
	}
}

void CUIActorMenu::RepairEffect_CurItem()
{
	PIItem item = CurrentIItem();
	if ( !item )
	{
		return;	
	}
	LPCSTR item_name = item->m_section_id.c_str();

	luabind::functor<void>	funct;
	R_ASSERT( ai().script_engine().functor( "inventory_upgrades.effect_repair_item", funct ) );
	funct( item_name, item->GetCondition() );

	item->SetCondition( 1.0f );
}

bool CUIActorMenu::CanUpgradeItem( PIItem item )
{
	VERIFY( item && m_pPartnerInvOwner );
	LPCSTR item_name = item->m_section_id.c_str();
	LPCSTR partner = m_pPartnerInvOwner->CharacterInfo().Profile().c_str();
		
	luabind::functor<bool> funct;
	R_ASSERT2(
		ai().script_engine().functor( "inventory_upgrades.can_upgrade_item", funct ),
		make_string( "Failed to get functor <inventory_upgrades.can_upgrade_item>, item = %s, mechanic = %s", item_name, partner )
		);

	return funct( item_name, partner );
}

void CUIActorMenu::CurModeToScript()
{
	int mode = (int)m_currMenuMode;
	luabind::functor<void>	funct;
	R_ASSERT( ai().script_engine().functor( "actor_menu.actor_menu_mode", funct ) );
	funct( mode );
}

template<class T>
class enum_dummy {};

void CUIActorMenu::script_register(lua_State* L)
{
		module(L)
		[
			class_<enum_dummy<EDDListType>>("EDDListType")
				.enum_("EDDListType")
				[
					value("iActorBag",				int(EDDListType::iActorBag)),
					value("iActorBelt",				int(EDDListType::iActorBelt)),
					value("iActorSlot",				int(EDDListType::iActorSlot)),
					value("iActorTrade",			int(EDDListType::iActorTrade)),
					value("iDeadBodyBag",			int(EDDListType::iDeadBodyBag)),
					value("iInvalid",				int(EDDListType::iInvalid)),
					value("iPartnerTrade",			int(EDDListType::iPartnerTrade)),
					value("iPartnerTradeBag",		int(EDDListType::iPartnerTradeBag))
				],

			class_<CUIActorMenu, CUIDialogWnd>("CUIActorMenu")
				.def(constructor<>())
				.def("get_drag_item",				&CUIActorMenu::GetCurrentItemAsGameObject)
				.def("refresh_current_cell_item",	&CUIActorMenu::RefreshCurrentItemCell)
				.def("IsShown",						&CUIActorMenu::IsShown)
				.def("ShowDialog",					&CUIActorMenu::ShowDialog)
				.def("HideDialog",					&CUIActorMenu::HideDialog),

 			class_< CUIMainIngameWnd, CUIWindow>("CUIMainIngameWnd")
				.def(constructor<>())
				//.def_readonly("UIStaticDiskIO",		&CUIMainIngameWnd::UIStaticDiskIO)
				//.def_readonly("UIStaticQuickHelp",	&CUIMainIngameWnd::UIStaticQuickHelp)
				//.def_readonly("UIMotionIcon",		&CUIMainIngameWnd::UIMotionIcon)
				.def_readonly("UIZoneMap",			&CUIMainIngameWnd::UIZoneMap)
				.def_readonly("m_ui_hud_states",	&CUIMainIngameWnd::m_ui_hud_states),
 			class_< CUIZoneMap >("CUIZoneMap")
				.def(constructor<>())
				.def_readonly("visible",			&CUIZoneMap::visible)
				.def("Background",					&CUIZoneMap::Background),
 			class_< CUIMotionIcon, CUIWindow>("CUIMotionIcon")
				.def(constructor<>()),
 			class_< CUIHudStatesWnd, CUIWindow>("CUIHudStatesWnd")
				.def(constructor<>())
				.def_readonly("m_back",				&CUIHudStatesWnd::m_back)
				.def_readonly("m_fire_mode",		&CUIHudStatesWnd::m_fire_mode)
				.def_readonly("m_ui_weapon_icon",	&CUIHudStatesWnd::m_ui_weapon_icon)
				.def_readonly("m_ui_health_bar",	&CUIHudStatesWnd::m_ui_health_bar)
				.def_readonly("m_ui_stamina_bar",	&CUIHudStatesWnd::m_ui_stamina_bar)
				.def_readonly("m_radia_damage",		&CUIHudStatesWnd::m_radia_damage)
		];

		module(L, "ActorMenu")
		[
			def("get_pda_menu",						+[]() { return &HUD().GetUI()->UIGame()->PdaMenu(); }),
			def("get_actor_menu",					+[]() { return &HUD().GetUI()->UIGame()->ActorMenu(); }),
			def("get_menu_mode",					+[]() { return HUD().GetUI()->UIGame()->ActorMenu().GetMenuMode(); }),
			def("get_maingame",						+[]() { return HUD().GetUI()->UIMainIngameWnd; })
		];
};

void CUIPdaWnd::script_register(lua_State* L)
{
		module(L)
		[
			class_<CUIPdaWnd, CUIDialogWnd>("CUIPdaWnd")
				.def(constructor<>())
				.def("IsShown",						&CUIPdaWnd::IsShown)
				.def("ShowDialog",					&CUIPdaWnd::ShowDialog)
				.def("HideDialog",					&CUIPdaWnd::HideDialog)
		];
};