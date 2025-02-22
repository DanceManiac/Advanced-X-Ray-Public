#include "pch_script.h"

#include "ai_space.h"
#include "script_engine.h"
#include "script_ui_registrator.h"
#include "UI\UIMultiTextStatic.h"
#include "MainMenu.h"

using namespace luabind;

CMainMenu*	MainMenu();

void create_global_achievement(LPCSTR name, LPCSTR description, LPCSTR icon_name)
{
	if (!MainMenu()->GetGlobalAchievementsManager())
	{
		ai().script_engine().script_log(ScriptStorage::eLuaMessageTypeError, "GLOBAL ACHIEVEMENTS : CGlobalAchievementsManager is NULL!");
		return;
	}

	MainMenu()->GetGlobalAchievementsManager()->Create(name, description, icon_name);
}

void get_global_achievement(LPCSTR name)
{
	if (!MainMenu()->GetGlobalAchievementsManager())
	{
		ai().script_engine().script_log(ScriptStorage::eLuaMessageTypeError, "GLOBAL ACHIEVEMENTS : CGlobalAchievementsManager is NULL!");
		return;
	}

	MainMenu()->GetGlobalAchievementsManager()->GetAchievement(name);
}

int get_all_achievements(lua_State* L)
{
	CGlobalAchievementsManager* manager = MainMenu()->GetGlobalAchievementsManager();
	if (!manager)
	{
		ai().script_engine().script_log(ScriptStorage::eLuaMessageTypeError, "GLOBAL ACHIEVEMENTS : CGlobalAchievementsManager is NULL!");
		lua_pushnil(L);
		return 1;
	}

	xr_vector<CGlobalAchievement*> achievements = manager->GetAllAchievements();

	lua_createtable(L, achievements.size(), 0);

	for (size_t i = 0; i < achievements.size(); ++i)
	{
		lua_createtable(L, 0, 3);

		// Имя
		lua_pushstring(L, achievements[i]->getName().c_str());
		lua_setfield(L, -2, "name");

		// Описание
		lua_pushstring(L, achievements[i]->getDescription().c_str());
		lua_setfield(L, -2, "description");

		// Название иконки
		lua_pushstring(L, achievements[i]->getIconName().c_str());
		lua_setfield(L, -2, "icon");

		lua_rawseti(L, -2, i + 1);
	}

	return 1;
}

void delete_global_achievement(LPCSTR name)
{
	if (!MainMenu()->GetGlobalAchievementsManager())
	{
		ai().script_engine().script_log(ScriptStorage::eLuaMessageTypeError, "GLOBAL ACHIEVEMENTS : CGlobalAchievementsManager is NULL!");
		return;
	}

	MainMenu()->GetGlobalAchievementsManager()->Delete(name);
}

#pragma optimize("s",on)
void UIRegistrator::script_register(lua_State *L)
{
	module(L)
	[

		class_<CGameFont>("CGameFont")
			.enum_("EAligment")
			[
				value("alLeft",						int(CGameFont::alLeft)),
				value("alRight",					int(CGameFont::alRight)),
				value("alCenter",					int(CGameFont::alCenter))
			],

		class_<CUICaption>("CUICaption")
			.def("addCustomMessage",	&CUICaption::addCustomMessage)
			.def("setCaption",			&CUICaption::setCaption),

		class_<Patch_Dawnload_Progress>("Patch_Dawnload_Progress")
			.def("GetInProgress",	&Patch_Dawnload_Progress::GetInProgress)
			.def("GetStatus",		&Patch_Dawnload_Progress::GetStatus)
			.def("GetFlieName",		&Patch_Dawnload_Progress::GetFlieName)
			.def("GetProgress",		&Patch_Dawnload_Progress::GetProgress),

		class_<CMainMenu>("CMainMenu")
			.def("GetPatchProgress",		&CMainMenu::GetPatchProgress)
			.def("CancelDownload",			&CMainMenu::CancelDownload)
			.def("ValidateCDKey",			&CMainMenu::ValidateCDKey)
			.def("GetGSVer",				&CMainMenu::GetGSVer)
			.def("GetAxrPlatform",			&CMainMenu::GetAxrPlatform)
	],
	module(L,"main_menu")
	[
		def("get_main_menu",				&MainMenu)
	];

	module(L, "global_achievements")
	[
		def("get",							get_global_achievement),
		def("create",						create_global_achievement),
		def("delete",						delete_global_achievement),
		def("get_all_lua_table",			get_all_achievements, raw<1>())
	];
}
