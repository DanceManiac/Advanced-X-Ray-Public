////////////////////////////////////////////////////////////////////////////
//	Module 		: game_sv_base_script.cpp
//	Created 	: 28.06.2004
//  Modified 	: 28.06.2004
//	Author		: Dmitriy Iassenev
//	Description : Base server game script export
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "game_sv_base.h"
#include "xrMessages.h"
#include "ui/UIInventoryUtilities.h"
#include "xr_time.h"
#include "../../xrCore/net_utils.h"
#include "UI/UIGameTutorial.h"
#include "string_table.h"
#include "object_broker.h"
#include "Actor.h"

using namespace luabind;

bool g_block_all_except_movement = false;
extern bool g_block_actor_movement;
extern bool g_actor_allow_ladder;
bool g_saves_locked = false;

CUISequencer* g_tutorial = NULL;
CUISequencer* g_tutorial2 = NULL;

void start_tutorial(LPCSTR name)
{
	// Skip any game tutorial on load screen
	if (load_screen_renderer.IsActive())
		return;

	if(g_tutorial){
		VERIFY				(!g_tutorial2);
		g_tutorial2			= g_tutorial;
	};
		
	g_tutorial							= xr_new<CUISequencer>();
	g_tutorial->Start					(name);
	if(g_tutorial2)
		g_tutorial->m_pStoredInputReceiver = g_tutorial2->m_pStoredInputReceiver;

}

void reload_language()
{
	CStringTable().ReloadLanguage();
}

LPCSTR translate_string(LPCSTR str)
{
	return *CStringTable().translate(str);
}

bool has_active_tutotial()
{
	return (g_tutorial!=NULL);
}

void block_all_except_movement(bool b)
{
	g_block_all_except_movement = b;
}

bool only_movement_allowed()
{
	return g_block_all_except_movement;
}

void block_actor_movement(bool b)
{
	g_block_actor_movement = b;
}

bool is_actor_movement_blocked()
{
	return g_block_actor_movement;
}

void set_actor_allow_ladder(bool b)
{
	g_actor_allow_ladder = b;
}

bool actor_allow_ladder()
{
	return g_actor_allow_ladder;
}

float get_devices_psy_factor()
{
	if (Actor())
		return Actor()->GetDevicesPsyFactor();

	Msg("![get_devices_psy_factor]: Actor not found!");
	return 0;
}

void set_devices_psy_factor(float psy_factor)
{
	clamp(psy_factor, 0.0f, 1.0f);

	if (Actor())
	{
		Actor()->SetDevicesPsyFactor(psy_factor);
		return;
	}

	Msg("![set_devices_psy_factor]: Actor not found!");
}

void set_game_saves_lock(bool b)
{
	g_saves_locked = b;
}

bool get_saves_lock_status()
{
	return g_saves_locked;
}

#pragma optimize("s",on)
void game_sv_GameState::script_register(lua_State *L)
{

	module(L,"game")
	[
	class_< xrTime >("CTime")
		.enum_("date_format")
		[
			value("DateToDay",		int(InventoryUtilities::edpDateToDay)),
			value("DateToMonth",	int(InventoryUtilities::edpDateToMonth)),
			value("DateToYear",		int(InventoryUtilities::edpDateToYear))
		]
		.enum_("time_format")
		[
			value("TimeToHours",	int(InventoryUtilities::etpTimeToHours)),
			value("TimeToMinutes",	int(InventoryUtilities::etpTimeToMinutes)),
			value("TimeToSeconds",	int(InventoryUtilities::etpTimeToSeconds)),
			value("TimeToMilisecs",	int(InventoryUtilities::etpTimeToMilisecs))
		]
		.def(						constructor<>()				)
		.def(						constructor<const xrTime&>())
		.def(const_self <			xrTime()					)
		.def(const_self <=			xrTime()					)
		.def(const_self >			xrTime()					)
		.def(const_self >=			xrTime()					)
		.def(const_self ==			xrTime()					)
		.def(self +					xrTime()					)
		.def(self -					xrTime()					)

		.def("diffSec"				,&xrTime::diffSec_script)
		.def("add"					,&xrTime::add_script)
		.def("sub"					,&xrTime::sub_script)

		.def("setHMS"				,&xrTime::setHMS)
		.def("setHMSms"				,&xrTime::setHMSms)
		.def("set"					,&xrTime::set)
		.def("get"					,&xrTime::get, out_value<2>() + out_value<3>() + out_value<4>() + out_value<5>() + out_value<6>() + out_value<7>() + out_value<8>())
		.def("dateToString"			,&xrTime::dateToString)
		.def("timeToString"			,&xrTime::timeToString),
		// declarations
		def("time",					get_time),
		def("get_game_time",		get_time_struct),
//		def("get_surge_time",	Game::get_surge_time),
//		def("get_object_by_name",Game::get_object_by_name),
	
	class_< game_sv_GameState, game_GameState >("game_sv_GameState")

	.def("get_eid",				&game_sv_GameState::get_eid)
	.def("get_id",				&game_sv_GameState::get_id)
	/*.def("get_it",				&game_sv_GameState::get_it)
	.def("get_it_2_id",			&game_sv_GameState::get_it_2_id)
	.def("get_name_it",			&game_sv_GameState::get_name_it) */
	.def("get_name_id",			&game_sv_GameState::get_name_id)
	.def("get_player_name_id",	&game_sv_GameState::get_player_name_id)
	
	.def("get_players_count",	&game_sv_GameState::get_players_count)
	.def("get_id_2_eid",		&game_sv_GameState::get_id_2_eid)

	.def("get_option_i",		&game_sv_GameState::get_option_i)
	.def("get_option_s",		&game_sv_GameState::get_option_s)
	.def("u_EventSend",			&game_sv_GameState::u_EventSend)

	.def("GenerateGameMessage",	&game_sv_GameState::GenerateGameMessage)
	.def("getRP",				&game_sv_GameState::getRP)
	.def("getRPcount",			&game_sv_GameState::getRPcount),

	def("start_tutorial",		&start_tutorial),
	def("has_active_tutorial",	&has_active_tutotial),
	def("reload_language",		&reload_language),
	def("translate_string",		&translate_string),

	def("only_allow_movekeys",	block_all_except_movement),
	def("only_movekeys_allowed",only_movement_allowed),
	def("block_actor_movement", block_actor_movement),
	def("is_actor_movement_blocked", is_actor_movement_blocked),
	def("set_actor_allow_ladder",set_actor_allow_ladder),
	def("actor_ladder_allowed", actor_allow_ladder),
	def("active_tutorial_name", +[]() { return g_tutorial->GetTutorName(); }),
	def("log_stack_trace",		&xrDebug::LogStackTrace),
	def("get_devices_psy_factor", &get_devices_psy_factor),
	def("set_devices_psy_factor", &set_devices_psy_factor),
	def("set_lock_saves",		set_game_saves_lock),
	def("get_saves_lock_status", get_saves_lock_status)

	];
	
	module(L)
	[
	class_<enum_exporter<EGamePlayerFlags> >("game_player_flags")
		.enum_("flags")
		[
			value("GAME_PLAYER_FLAG_LOCAL",						int(GAME_PLAYER_FLAG_LOCAL)),
			value("GAME_PLAYER_FLAG_READY",						int(GAME_PLAYER_FLAG_READY)),
			value("GAME_PLAYER_FLAG_VERY_VERY_DEAD",			int(GAME_PLAYER_FLAG_VERY_VERY_DEAD)),
			value("GAME_PLAYER_FLAG_SPECTATOR",					int(GAME_PLAYER_FLAG_SPECTATOR)),
			value("GAME_PLAYER_FLAG_SCRIPT_BEGINS_FROM",		int(GAME_PLAYER_FLAG_SCRIPT_BEGINS_FROM))
		],

	class_<enum_exporter<EGamePhases> >("game_phases")
		.enum_("phases")
		[
			value("GAME_PHASE_NONE",							int(GAME_PHASE_NONE)),
			value("GAME_PHASE_INPROGRESS",						int(GAME_PHASE_INPROGRESS)),
			value("GAME_PHASE_PENDING",							int(GAME_PHASE_PENDING)),
			value("GAME_PHASE_TEAM1_SCORES",					int(GAME_PHASE_TEAM1_SCORES)),
			value("GAME_PHASE_TEAM2_SCORES",					int(GAME_PHASE_TEAM2_SCORES)),
			value("GAME_PHASE_TEAMS_IN_A_DRAW",					int(GAME_PHASE_TEAMS_IN_A_DRAW)),
			value("GAME_PHASE_SCRIPT_BEGINS_FROM",				int(GAME_PHASE_SCRIPT_BEGINS_FROM))
		],

	class_<enum_exporter<EGameMessages> >("game_messages")
		.enum_("messages")
		[
			value("GAME_EVENT_PLAYER_READY",					int(GAME_EVENT_PLAYER_READY)),
			value("GAME_EVENT_PLAYER_CHANGE_TEAM",				int(GAME_EVENT_PLAYER_GAME_MENU)),
			value("GAME_EVENT_PLAYER_KILL",						int(GAME_EVENT_PLAYER_KILL)),
			value("GAME_EVENT_PLAYER_BUY_FINISHED",				int(GAME_EVENT_PLAYER_BUY_FINISHED)),
			value("GAME_EVENT_PLAYER_CHANGE_SKIN",				int(GAME_EVENT_PLAYER_GAME_MENU)),
			value("GAME_EVENT_PLAYER_CONNECTED",				int(GAME_EVENT_PLAYER_CONNECTED)),
			value("GAME_EVENT_PLAYER_DISCONNECTED",				int(GAME_EVENT_PLAYER_DISCONNECTED)),
			value("GAME_EVENT_PLAYER_KILLED",					int(GAME_EVENT_PLAYER_KILLED)),
			value("GAME_EVENT_PLAYER_JOIN_TEAM",				int(GAME_EVENT_PLAYER_JOIN_TEAM)),
			value("GAME_EVENT_ROUND_STARTED",					int(GAME_EVENT_ROUND_STARTED)),
			value("GAME_EVENT_ROUND_END",						int(GAME_EVENT_ROUND_END)),
			value("GAME_EVENT_ARTEFACT_SPAWNED",				int(GAME_EVENT_ARTEFACT_SPAWNED)),
			value("GAME_EVENT_ARTEFACT_DESTROYED",				int(GAME_EVENT_ARTEFACT_DESTROYED)),
			value("GAME_EVENT_ARTEFACT_TAKEN",					int(GAME_EVENT_ARTEFACT_TAKEN)),
			value("GAME_EVENT_ARTEFACT_DROPPED",				int(GAME_EVENT_ARTEFACT_DROPPED)),
			value("GAME_EVENT_ARTEFACT_ONBASE",					int(GAME_EVENT_ARTEFACT_ONBASE)),
			value("GAME_EVENT_PLAYER_ENTER_TEAM_BASE",			int(GAME_EVENT_PLAYER_ENTER_TEAM_BASE)),
			value("GAME_EVENT_PLAYER_LEAVE_TEAM_BASE",			int(GAME_EVENT_PLAYER_LEAVE_TEAM_BASE)),
			value("GAME_EVENT_BUY_MENU_CLOSED",					int(GAME_EVENT_BUY_MENU_CLOSED)),
			value("GAME_EVENT_TEAM_MENU_CLOSED",				int(GAME_EVENT_TEAM_MENU_CLOSED)),
			value("GAME_EVENT_SKIN_MENU_CLOSED",				int(GAME_EVENT_SKIN_MENU_CLOSED)),
			value("GAME_EVENT_SCRIPT_BEGINS_FROM",				int(GAME_EVENT_SCRIPT_BEGINS_FROM))
		]

	
	];
}
