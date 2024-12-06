////////////////////////////////////////////////////////////////////////////
//	Module 		: level_script.cpp
//	Created 	: 28.06.2004
//  Modified 	: 28.06.2004
//	Author		: Dmitriy Iassenev
//	Description : Level script export
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "level.h"
#include "actor.h"
#include "script_game_object.h"
#include "patrol_path_storage.h"
#include "xrServer.h"
#include "client_spawn_manager.h"
#include "../xrEngine/igame_persistent.h"
#include "game_cl_base.h"
#include "ui/UIDialogWnd.h"
#include "date_time.h"
#include "ai_space.h"
#include "level_graph.h"
#include "PHCommander.h"
#include "PHScriptCall.h"
#include "HUDManager.h"
#include "script_engine.h"
#include "game_cl_single.h"

#include "map_manager.h"
#include "map_location.h"
#include "phworld.h"

#include "alife_simulator.h"
#include "alife_time_manager.h"
#include "game_sv_single.h"
#include "string_table.h"
#include "../xrEngine/Rain.h"

#include "CustomTimer.h"

using namespace luabind;

LPCSTR command_line	()
{
	return		(Core.Params);
}

#ifdef DEBUG
void check_object(CScriptGameObject *object)
{
	try {
		Msg	("check_object %s",object->Name());
	}
	catch(...) {
		object = object;
	}
}

CScriptGameObject *tpfGetActor()
{
	static bool first_time = true;
	if (first_time)
		ai().script_engine().script_log(eLuaMessageTypeError,"Do not use level.actor function!");
	first_time = false;
	
	CActor *l_tpActor = smart_cast<CActor*>(Level().CurrentEntity());
	if (l_tpActor)
		return	(smart_cast<CGameObject*>(l_tpActor)->lua_game_object());
	else
		return	(0);
}

CScriptGameObject *get_object_by_name(LPCSTR caObjectName)
{
	static bool first_time = true;
	if (first_time)
		ai().script_engine().script_log(eLuaMessageTypeError,"Do not use level.object function!");
	first_time = false;
	
	CGameObject		*l_tpGameObject	= smart_cast<CGameObject*>(Level().Objects.FindObjectByName(caObjectName));
	if (l_tpGameObject)
		return		(l_tpGameObject->lua_game_object());
	else
		return		(0);
}
#endif

CScriptGameObject *get_object_by_id(u16 id)
{
	CGameObject* pGameObject = smart_cast<CGameObject*>(Level().Objects.net_Find(id));
	if(!pGameObject)
		return NULL;

	return pGameObject->lua_game_object();
}

LPCSTR get_weather	()
{
	return			(*g_pGamePersistent->Environment().GetWeather());
}

void set_weather	(LPCSTR weather_name, bool forced)
{
	return			(g_pGamePersistent->Environment().SetWeather(weather_name,forced));
}

bool set_weather_fx	(LPCSTR weather_name)
{
	return			(g_pGamePersistent->Environment().SetWeatherFX(weather_name));
}

bool start_weather_fx_from_time(LPCSTR weather_name, float time)
{
	return			(g_pGamePersistent->Environment().StartWeatherFXFromTime(weather_name, time));
}

bool is_wfx_playing	()
{
	return			(g_pGamePersistent->Environment().IsWFXPlaying());
}

float get_wfx_time()
{
	return			(g_pGamePersistent->Environment().wfx_time);
}

void stop_weather_fx()
{
	g_pGamePersistent->Environment().StopWFX();
}

void change_game_time(u32 days, u32 hours, u32 mins)
{
	game_sv_Single* tpGame = smart_cast<game_sv_Single*>(Level().Server->game);
	if (tpGame && ai().get_alife())
	{
		u32 value = days * 86400 + hours * 3600 + mins * 60;
		float fValue = static_cast<float> (value);
		value *= 1000;//msec		
		g_pGamePersistent->Environment().ChangeGameTime(fValue);
		tpGame->alife().time_manager().change_game_time(value);
	}
}

void set_time_factor(float time_factor)
{
	if (!OnServer())
		return;

	Level().Server->game->SetGameTimeFactor(time_factor);
}

float get_time_factor()
{
	return			(Level().GetGameTimeFactor());
}

void set_game_difficulty(ESingleGameDifficulty dif)
{
	g_SingleGameDifficulty		= dif;
	game_cl_Single* game		= smart_cast<game_cl_Single*>(Level().game); VERIFY(game);
	game->OnDifficultyChanged	();
}
ESingleGameDifficulty get_game_difficulty()
{
	return g_SingleGameDifficulty;
}

u32 get_time_days()
{
	u32 year = 0, month = 0, day = 0, hours = 0, mins = 0, secs = 0, milisecs = 0;
	split_time((g_pGameLevel && Level().game) ? Level().GetGameTime() : ai().alife().time_manager().game_time(), year, month, day, hours, mins, secs, milisecs);
	return			day;
}

u32 get_time_hours()
{
	u32 year = 0, month = 0, day = 0, hours = 0, mins = 0, secs = 0, milisecs = 0;
	split_time((g_pGameLevel && Level().game) ? Level().GetGameTime() : ai().alife().time_manager().game_time(), year, month, day, hours, mins, secs, milisecs);
	return			hours;
}

u32 get_time_minutes()
{
	u32 year = 0, month = 0, day = 0, hours = 0, mins = 0, secs = 0, milisecs = 0;
	split_time((g_pGameLevel && Level().game) ? Level().GetGameTime() : ai().alife().time_manager().game_time(), year, month, day, hours, mins, secs, milisecs);
	return			mins;
}

float cover_in_direction(u32 level_vertex_id, const Fvector &direction)
{
	float			y,p;
	direction.getHP	(y,p);
	return			(ai().level_graph().cover_in_direction(y,level_vertex_id));
}

float rain_factor()
{
	return			(g_pGamePersistent->Environment().CurrentEnv->rain_density);
}

float rain_wetness()
{
	return (g_pGamePersistent->Environment().wetness_accum);
}

float rain_hemi()
{
	CEffect_Rain* rain = g_pGamePersistent->pEnvironment->eff_Rain;

	if (rain)
	{
		return rain->GetRainHemi();
	}
	else
	{
		CObject* E = g_pGameLevel->CurrentViewEntity();
		if (E && E->renderable_ROS())
		{
			float* hemi_cube = E->renderable_ROS()->get_luminocity_hemi_cube();
			float hemi_val = _max(hemi_cube[0], hemi_cube[1]);
			hemi_val = _max(hemi_val, hemi_cube[2]);
			hemi_val = _max(hemi_val, hemi_cube[3]);
			hemi_val = _max(hemi_val, hemi_cube[5]);

			return hemi_val;
		}

		return 0.f;
	}
}

float air_temperature()
{
	return (g_pGamePersistent->Environment().CurrentEnv->m_fAirTemperature);
}

u32	vertex_in_direction(u32 level_vertex_id, Fvector direction, float max_distance)
{
	direction.normalize_safe();
	direction.mul	(max_distance);
	Fvector			start_position = ai().level_graph().vertex_position(level_vertex_id);
	Fvector			finish_position = Fvector(start_position).add(direction);
	u32				result = u32(-1);
	ai().level_graph().farthest_vertex_in_direction(level_vertex_id,start_position,finish_position,result,0);
	return			(ai().level_graph().valid_vertex_id(result) ? result : level_vertex_id);
}

Fvector vertex_position(u32 level_vertex_id)
{
	return			(ai().level_graph().vertex_position(level_vertex_id));
}

void map_add_object_spot(u16 id, LPCSTR spot_type, LPCSTR text)
{
	CMapLocation* ml = Level().MapManager().AddMapLocation(spot_type,id);
	if( xr_strlen(text) )
			ml->SetHint(text);
}

void map_add_object_spot_ser(u16 id, LPCSTR spot_type, LPCSTR text)
{
	CMapLocation* ml = Level().MapManager().AddMapLocation(spot_type,id);
	if( xr_strlen(text) )
			ml->SetHint(text);

	ml->SetSerializable(true);
}

void map_change_spot_hint(u16 id, LPCSTR spot_type, LPCSTR text)
{
	CMapLocation* ml	= Level().MapManager().GetMapLocation(spot_type, id);
	if(!ml)				return;
	ml->SetHint			(text);
}

void map_remove_object_spot(u16 id, LPCSTR spot_type)
{
	Level().MapManager().RemoveMapLocation(spot_type, id);
}

u16 map_has_object_spot(u16 id, LPCSTR spot_type)
{
	return Level().MapManager().HasMapLocation(spot_type, id);
}

bool patrol_path_exists(LPCSTR patrol_path)
{
	return		(!!ai().patrol_paths().path(patrol_path,true));
}

LPCSTR get_name()
{
	return		(*Level().name());
}

void prefetch_sound	(LPCSTR name)
{
	Level().PrefetchSound(name);
}


CClientSpawnManager	&get_client_spawn_manager()
{
	return		(Level().client_spawn_manager());
}

void start_stop_menu(CUIDialogWnd* pDialog, bool bDoHideIndicators)
{
	HUD().GetUI()->StartStopMenu(pDialog,bDoHideIndicators);
}


void add_dialog_to_render(CUIDialogWnd* pDialog)
{
	HUD().GetUI()->AddDialogToRender(pDialog);
}

void remove_dialog_to_render(CUIDialogWnd* pDialog)
{
	HUD().GetUI()->RemoveDialogToRender(pDialog);
}

CUIDialogWnd* main_input_receiver()
{
	return HUD().GetUI()->MainInputReceiver();
}
#include "UIGameCustom.h"
void hide_indicators()
{
	if(HUD().GetUI())
	{
	HUD().GetUI()->UIGame()->HideShownDialogs();
		HUD().GetUI()->ShowGameIndicators(false);
		HUD().GetUI()->ShowCrosshair(false);
	}
}

void hide_indicators_safe()
{
	if(HUD().GetUI())
	{
		HUD().GetUI()->ShowGameIndicators(false);
		HUD().GetUI()->ShowCrosshair(false);

		//HUD().GetUI()->OnExternalHideIndicators();
	}
}

void show_indicators()
{
	if(HUD().GetUI())
	{
		HUD().GetUI()->ShowGameIndicators(true);
		HUD().GetUI()->ShowCrosshair(true);
	}
}


bool is_level_present()
{
	return (!!g_pGameLevel);
}

void add_call(const luabind::functor<bool> &condition,const luabind::functor<void> &action)
{
	luabind::functor<bool>		_condition = condition;
	luabind::functor<void>		_action = action;
	CPHScriptCondition	* c=xr_new<CPHScriptCondition>(_condition);
	CPHScriptAction		* a=xr_new<CPHScriptAction>(_action);
	Level().ph_commander_scripts().add_call(c,a);
}

void remove_call(const luabind::functor<bool> &condition,const luabind::functor<void> &action)
{
	CPHScriptCondition	c(condition);
	CPHScriptAction		a(action);
	Level().ph_commander_scripts().remove_call(&c,&a);
}

void add_call(const luabind::object &lua_object, LPCSTR condition,LPCSTR action)
{
//	try{	
//		CPHScriptObjectCondition	*c=xr_new<CPHScriptObjectCondition>(lua_object,condition);
//		CPHScriptObjectAction		*a=xr_new<CPHScriptObjectAction>(lua_object,action);
		luabind::functor<bool>		_condition = object_cast<luabind::functor<bool> >(lua_object[condition]);
		luabind::functor<void>		_action = object_cast<luabind::functor<void> >(lua_object[action]);
		CPHScriptObjectConditionN	*c=xr_new<CPHScriptObjectConditionN>(lua_object,_condition);
		CPHScriptObjectActionN		*a=xr_new<CPHScriptObjectActionN>(lua_object,_action);
		Level().ph_commander_scripts().add_call_unique(c,c,a,a);
//	}
//	catch(...)
//	{
//		Msg("add_call excepted!!");
//	}
}

void remove_call(const luabind::object &lua_object, LPCSTR condition,LPCSTR action)
{
	CPHScriptObjectCondition	c(lua_object,condition);
	CPHScriptObjectAction		a(lua_object,action);
	Level().ph_commander_scripts().remove_call(&c,&a);
}

void add_call(const luabind::object &lua_object, const luabind::functor<bool> &condition,const luabind::functor<void> &action)
{

	CPHScriptObjectConditionN	*c=xr_new<CPHScriptObjectConditionN>(lua_object,condition);
	CPHScriptObjectActionN		*a=xr_new<CPHScriptObjectActionN>(lua_object,action);
	Level().ph_commander_scripts().add_call(c,a);
}

void remove_call(const luabind::object &lua_object, const luabind::functor<bool> &condition,const luabind::functor<void> &action)
{
	CPHScriptObjectConditionN	c(lua_object,condition);
	CPHScriptObjectActionN		a(lua_object,action);
	Level().ph_commander_scripts().remove_call(&c,&a);
}

void remove_calls_for_object(const luabind::object &lua_object)
{
	CPHSriptReqObjComparer c(lua_object);
	Level().ph_commander_scripts().remove_calls(&c);
}

CPHWorld* physics_world()
{
	return	ph_world;
}
CEnvironment *environment()
{
	return		(g_pGamePersistent->pEnvironment);
}

CEnvDescriptor *current_environment(CEnvironment *self)
{
	return self->CurrentEnv;
}
extern bool g_bDisableAllInput;
void disable_input()
{
	g_bDisableAllInput = true;
}
void enable_input()
{
	g_bDisableAllInput = false;
}

void spawn_phantom(const Fvector &position)
{
	Level().spawn_item("m_phantom", position, u32(-1), u16(-1), false);
}

Fbox get_bounding_volume()
{
	return Level().ObjectSpace.GetBoundingVolume();
}

void iterate_sounds					(LPCSTR prefix, u32 max_count, const CScriptCallbackEx<void> &callback)
{
	for (int j=0, N = _GetItemCount(prefix); j<N; ++j) {
		string_path					fn, s;
		_GetItem					(prefix,j,s);
		if (FS.exist(fn,"$game_sounds$",s,".ogg"))
			callback				(prefix);

		for (u32 i=0; i<max_count; ++i)
		{
			string_path					name;
			sprintf_s					(name,"%s%d",s,i);
			if (FS.exist(fn,"$game_sounds$",name,".ogg"))
				callback			(name);
		}
	}
}

void iterate_sounds1				(LPCSTR prefix, u32 max_count, luabind::functor<void> functor)
{
	CScriptCallbackEx<void>		temp;
	temp.set					(functor);
	iterate_sounds				(prefix,max_count,temp);
}

void iterate_sounds2				(LPCSTR prefix, u32 max_count, luabind::object object, luabind::functor<void> functor)
{
	CScriptCallbackEx<void>		temp;
	temp.set					(functor,object);
	iterate_sounds				(prefix,max_count,temp);
}

#include "actoreffector.h"
float add_cam_effector(LPCSTR fn, int id, bool cyclic, LPCSTR cb_func)
{
	CAnimatorCamEffectorScriptCB* e		= xr_new<CAnimatorCamEffectorScriptCB>(cb_func);
	e->SetType					((ECamEffectorType)id);
	e->SetCyclic				(cyclic);
	e->Start					(fn);
	Actor()->Cameras().AddCamEffector(e);
	return						e->GetAnimatorLength();
}

float add_cam_effector2(LPCSTR fn, int id, bool cyclic, LPCSTR cb_func)
{
	CAnimatorCamEffectorScriptCB* e		= xr_new<CAnimatorCamEffectorScriptCB>(cb_func);
	e->m_bAbsolutePositioning	= true;
	e->SetType					((ECamEffectorType)id);
	e->SetCyclic				(cyclic);
	e->Start					(fn);
	Actor()->Cameras().AddCamEffector(e);
	return						e->GetAnimatorLength();
}

void remove_cam_effector(int id)
{
	Actor()->Cameras().RemoveCamEffector((ECamEffectorType)id );
}
		
float get_snd_volume()
{
	return psSoundVFactor;
}

void set_snd_volume(float v)
{
	psSoundVFactor = v;
	clamp(psSoundVFactor,0.0f,1.0f);
}
#include "actor_statistic_mgr.h"
void add_actor_points(LPCSTR sect, LPCSTR detail_key, int cnt, int pts)
{
	return Actor()->StatisticMgr().AddPoints(sect, detail_key, cnt, pts);
}

void add_actor_points_str(LPCSTR sect, LPCSTR detail_key, LPCSTR str_value)
{
	return Actor()->StatisticMgr().AddPoints(sect, detail_key, str_value);
}

int get_actor_points(LPCSTR sect)
{
	return Actor()->StatisticMgr().GetSectionPoints(sect);
}
extern int get_actor_ranking();
extern void add_human_to_top_list		(u16 id);
extern void remove_human_from_top_list	(u16 id);



#include "ActorEffector.h"
void add_complex_effector(LPCSTR section, int id)
{
	AddEffector(Actor(),id, section);
}

void remove_complex_effector(int id)
{
	RemoveEffector(Actor(),id);
}

#include "postprocessanimator.h"
void add_pp_effector(LPCSTR fn, int id, bool cyclic)
{
	CPostprocessAnimator* pp		= xr_new<CPostprocessAnimator>(id, cyclic);
	pp->Load						(fn);
	Actor()->Cameras().AddPPEffector	(pp);
}

void remove_pp_effector(int id)
{
	CPostprocessAnimator*	pp	= smart_cast<CPostprocessAnimator*>(Actor()->Cameras().GetPPEffector((EEffectorPPType)id));

	if(pp) pp->Stop(1.0f);

}

void set_pp_effector_factor(int id, float f, float f_sp)
{
	CPostprocessAnimator*	pp	= smart_cast<CPostprocessAnimator*>(Actor()->Cameras().GetPPEffector((EEffectorPPType)id));

	if(pp) pp->SetDesiredFactor(f,f_sp);
}

void set_pp_effector_factor2(int id, float f)
{
	CPostprocessAnimator*	pp	= smart_cast<CPostprocessAnimator*>(Actor()->Cameras().GetPPEffector((EEffectorPPType)id));

	if(pp) pp->SetCurrentFactor(f);
}

#include "relation_registry.h"

 int g_community_goodwill(LPCSTR _community, int _entity_id)
 {
	 CHARACTER_COMMUNITY c;
	 c.set					(_community);

 	return RELATION_REGISTRY().GetCommunityGoodwill(c.index(), u16(_entity_id));
 }

void g_set_community_goodwill(LPCSTR _community, int _entity_id, int val)
{
	 CHARACTER_COMMUNITY	c;
	 c.set					(_community);
	RELATION_REGISTRY().SetCommunityGoodwill(c.index(), u16(_entity_id), val);
}

void g_change_community_goodwill(LPCSTR _community, int _entity_id, int val)
{
	 CHARACTER_COMMUNITY	c;
	 c.set					(_community);
	RELATION_REGISTRY().ChangeCommunityGoodwill(c.index(), u16(_entity_id), val);
}

//Alundaio: namespace level exports extension
//ability to get the target game_object at crosshair
CScriptGameObject* g_get_target_obj()
{
	collide::rq_result& RQ = HUD().GetCurrentRayQuery();
	if (RQ.O)
	{
		CGameObject* game_object = smart_cast<CGameObject*>(RQ.O);
		if (game_object)
			return game_object->lua_game_object();
	}
	return nullptr;
}

float g_get_target_dist()
{
	collide::rq_result& RQ = HUD().GetCurrentRayQuery();
	return RQ.range;
}

//Alundaio: END

void send_event_key_press(int dik)
{
	Level().IR_OnKeyboardPress(dik);
}

void send_event_key_release(int dik)
{
	Level().IR_OnKeyboardRelease(dik);
}

void send_event_key_hold(int dik)
{
	Level().IR_OnKeyboardHold(dik);
}

void send_event_mouse_wheel(int vol)
{
	Level().IR_OnMouseWheel(vol);
}

void block_player_action(EGameActions dik)
{
	Actor()->block_action(dik);
}

void unblock_player_action(EGameActions dik)
{
	Actor()->unblock_action(dik);
}

void set_global_time_factor(float tf)
{
	if (!OnServer())
		return;

	Device.time_factor(tf);
	psSpeedOfSound = tf;
}

float get_global_time_factor()
{
	return			(Device.time_factor());
}

void set_skills_points(int num)
{
	Actor()->ActorSkills->set_skills_points(num);
}

void inc_skills_points(int num)
{
	Actor()->ActorSkills->inc_skills_points(num);
}

void dec_skills_points(int num)
{
	Actor()->ActorSkills->dec_skills_points(num);
}

int get_skills_points()
{
	return Actor()->ActorSkills->get_skills_points();
}

void set_survival_skill(int num)
{
	Actor()->ActorSkills->set_survival_skill(num);
}

void inc_survival_skill(int num)
{
	Actor()->ActorSkills->inc_survival_skill(num);
}

void dec_survival_skill(int num)
{
	Actor()->ActorSkills->dec_survival_skill(num);
}

int get_survival_skill()
{
	return Actor()->ActorSkills->get_survival_skill();
}

void set_power_skill(int num)
{
	Actor()->ActorSkills->set_power_skill(num);
}

void inc_power_skill(int num)
{
	Actor()->ActorSkills->inc_power_skill(num);
}

void dec_power_skill(int num)
{
	Actor()->ActorSkills->dec_power_skill(num);
}

int get_power_skill()
{
	return Actor()->ActorSkills->get_power_skill();
}

void set_repair_skill(int num)
{
	Actor()->ActorSkills->set_repair_skill(num);
}

void inc_repair_skill(int num)
{
	Actor()->ActorSkills->inc_repair_skill(num);
}

void dec_repair_skill(int num)
{
	Actor()->ActorSkills->dec_repair_skill(num);
}

int get_repair_skill()
{
	return Actor()->ActorSkills->get_repair_skill();
}

void set_endurance_skill(int num)
{
	Actor()->ActorSkills->set_endurance_skill(num);
}

void inc_endurance_skill(int num)
{
	Actor()->ActorSkills->inc_endurance_skill(num);
}

void dec_endurance_skill(int num)
{
	Actor()->ActorSkills->dec_endurance_skill(num);
}

int get_endurance_skill()
{
	return Actor()->ActorSkills->get_endurance_skill();
}

void buy_skill(int num)
{
	return Actor()->ActorSkills->BuySkill(num);
}

u32 g_get_target_element()
{
	collide::rq_result& RQ = HUD().GetCurrentRayQuery();
	if (RQ.element)
		return RQ.element;

	return 0;
}

//can spawn entities like bolts, phantoms, ammo, etc. which normally crash when using alife():create()
void spawn_section(pcstr sSection, Fvector3 vPosition, u32 LevelVertexID, u16 ParentID, bool bReturnItem = false)
{
	Level().spawn_item(sSection, vPosition, LevelVertexID, ParentID, bReturnItem);
}

u8 get_active_cam()
{
	CActor* actor = smart_cast<CActor*>(Level().CurrentViewEntity());
	if (actor)
		return (u8)actor->active_cam();

	return 255;
}

void patrol_path_add(LPCSTR patrol_path, CPatrolPath* path)
{
	ai().patrol_paths_raw().add_path(shared_str(patrol_path), path);
}

void patrol_path_remove(LPCSTR patrol_path)
{
	ai().patrol_paths_raw().remove_path(shared_str(patrol_path));
}

void set_active_cam(u8 mode)
{
	CActor* actor = smart_cast<CActor*>(Level().CurrentViewEntity());
	if (actor && mode <= eacMaxCam)
		actor->cam_Set((EActorCameras)mode);
}

//ability to update level netpacket
void g_send(NET_Packet& P, bool bReliable = false, bool bSequential = true, bool bHighPriority = false, bool bSendImmediately = false)
{
	Level().Send(P);
}

void create_custom_timer(LPCSTR name, int start_value, ETimerMode mode = eTimerModeMilliseconds)
{
	if (!Actor()->TimerManager)
	{
		ai().script_engine().script_log(ScriptStorage::eLuaMessageTypeError, "CUSTOM TIMER : TimerManager is NULL!");
		return;
	}

	Actor()->TimerManager->CreateTimer(name, start_value, mode);
}

void start_custom_timer(LPCSTR name)
{
	if (!Actor()->TimerManager)
	{
		ai().script_engine().script_log(ScriptStorage::eLuaMessageTypeError, "CUSTOM TIMER : TimerManager is NULL!");
		return;
	}

	Actor()->TimerManager->StartTimer(name);
}

void stop_custom_timer(LPCSTR name)
{
	if (!Actor()->TimerManager)
	{
		ai().script_engine().script_log(ScriptStorage::eLuaMessageTypeError, "CUSTOM TIMER : TimerManager is NULL!");
		return;
	}

	Actor()->TimerManager->StopTimer(name);
}

void reset_custom_timer(LPCSTR name)
{
	if (!Actor()->TimerManager)
	{
		ai().script_engine().script_log(ScriptStorage::eLuaMessageTypeError, "CUSTOM TIMER : TimerManager is NULL!");
		return;
	}

	Actor()->TimerManager->ResetTimer(name);
}

void delete_custom_timer(LPCSTR name)
{
	if (!Actor()->TimerManager)
	{
		ai().script_engine().script_log(ScriptStorage::eLuaMessageTypeError, "CUSTOM TIMER : TimerManager is NULL!");
		return;
	}

	Actor()->TimerManager->DeleteTimer(name);
}

u64 get_custom_timer(LPCSTR name)
{
	if (!Actor()->TimerManager)
	{
		ai().script_engine().script_log(ScriptStorage::eLuaMessageTypeError, "CUSTOM TIMER : TimerManager is NULL!");
		return 0;
	}

	return Actor()->TimerManager->GetTimerValue(name);
}

u32 nearest_vertex_id(const Fvector& vec)
{
	return ai().level_graph().vertex_id(vec);
}

bool valid_vertex_id(u32 level_vertex_id)
{
	return ai().level_graph().valid_vertex_id(level_vertex_id);
}

u32 vertex_count()
{
	return ai().level_graph().header().vertex_count();
}

LPCSTR get_moon_phase()
{
	return Level().GetMoonPhase().c_str();
}

LPCSTR get_past_wdesc()
{
	return			(g_pGamePersistent->Environment().Current[0] ? g_pGamePersistent->Environment().Current[0]->m_identifier.c_str() : "null");
}

LPCSTR get_next_wdesc()
{
	return			(g_pGamePersistent->Environment().Current[1] ? g_pGamePersistent->Environment().Current[1]->m_identifier.c_str() : "null");
}

float get_past_wdesc_execution_time()
{
	return			(g_pGamePersistent->Environment().Current[0] ? g_pGamePersistent->Environment().Current[0]->exec_time : -1.f);
}

float get_next_wdesc_execution_time()
{
	return			(g_pGamePersistent->Environment().Current[1] ? g_pGamePersistent->Environment().Current[1]->exec_time : -1.f);
}

float get_weather_game_time()
{
	return			(&g_pGamePersistent->Environment() ? g_pGamePersistent->Environment().GetGameTime() : -1.f);
}

void set_past_wdesc(LPCSTR WeatherSection)
{
	if (&g_pGamePersistent->Environment())
	{
		g_pGamePersistent->Environment().SetEnvDesc(WeatherSection, g_pGamePersistent->Environment().Current[0]);
	}
}

void set_next_wdesc(LPCSTR WeatherSection)
{
	if (&g_pGamePersistent->Environment())
	{
		g_pGamePersistent->Environment().SetEnvDesc(WeatherSection, g_pGamePersistent->Environment().Current[1]);
	}
}

bool is_developer()
{
	return bDeveloperMode;
}

float get_air_temperature_f()
{
	if (!g_pGamePersistent || !g_pGamePersistent->Environment().CurrentEnv)
	{
		Msg("![level_script::get_air_temperature_f]: g_pGamePersistent or CurrentEnv is nullptr!");
		return 0.0f;
	}

	return g_pGamePersistent->Environment().CurrentEnv->m_fAirTemperature;
}

luabind::internal_string get_air_temperature_fs()
{
	if (!g_pGamePersistent || !g_pGamePersistent->Environment().CurrentEnv)
	{
		Msg("![level_script::get_air_temperature_fs]: g_pGamePersistent or CurrentEnv is nullptr!");
		return "ERROR";
	}

	float cur_temperature = g_pGamePersistent->Environment().CurrentEnv->m_fAirTemperature;
	string16 temper = "";

	if (cur_temperature < 0)
		xr_sprintf(temper, "%.1f %s", cur_temperature, *CStringTable().translate("st_degree"));
	else
		xr_sprintf(temper, "+%.1f %s", cur_temperature, *CStringTable().translate("st_degree"));

	return temper;
}

#pragma optimize("s",on)
void CLevel::script_register(lua_State *L)
{
	class_<CEnvDescriptor>("CEnvDescriptor")
		.def_readonly("fog_density",			&CEnvDescriptor::fog_density)
		.def_readonly("far_plane",				&CEnvDescriptor::far_plane),

	class_<CEnvironment>("CEnvironment")
		.def("current",							current_environment);

	module(L,"level")
	[
		//Alundaio: Extend level namespace exports
		def("send",								&g_send), //allow the ability to send netpacket to level
		def("get_target_element",				&g_get_target_element), //Can get bone cursor is targeting
		def("spawn_item",						&spawn_section),
		def("get_active_cam",					&get_active_cam),
		def("set_active_cam",					&set_active_cam),
		def("get_start_time",					+[]() { return xrTime(Level().GetStartGameTime()); }),
		def("valid_vertex",						+[](u32 level_vertex_id) { return ai().level_graph().valid_vertex_id(level_vertex_id); }),
		//Alundaio: END
		// obsolete\deprecated
		def("get_target_obj",					g_get_target_obj), //intentionally named to what is in xray extensions
		def("get_target_dist",					g_get_target_dist),
		def("object_by_id",						get_object_by_id),
#ifdef DEBUG
		def("debug_object",						get_object_by_name),
		def("debug_actor",						tpfGetActor),
		def("check_object",						check_object),
#endif
		
		def("send_event_key_press",				&send_event_key_press),
		def("send_event_key_release",			&send_event_key_release),
		def("send_event_key_hold",				&send_event_key_hold),
		def("send_event_mouse_wheel",			&send_event_mouse_wheel),
		def("block_player_action",				&block_player_action),
		def("unblock_player_action",			&unblock_player_action),
		def("set_global_time_factor",			&set_global_time_factor),
		def("get_global_time_factor",			&get_global_time_factor),

		def("get_weather",						get_weather),
		def("set_weather",						set_weather),
		def("set_past_weather",					set_past_wdesc),
		def("set_next_weather",					set_next_wdesc),
		def("set_weather_fx",					set_weather_fx),
		def("get_weather_game_time",			get_weather_game_time),
		def("get_past_wdesc_execution_time",	get_past_wdesc_execution_time),
		def("get_next_wdesc_execution_time",	get_next_wdesc_execution_time),
		def("get_past_weather",					get_past_wdesc),
		def("get_next_weather",					get_next_wdesc),
		def("set_weather_fx",					set_weather_fx),
		def("start_weather_fx_from_time",		start_weather_fx_from_time),
		def("is_wfx_playing",					is_wfx_playing),
		def("get_wfx_time",						get_wfx_time),
		def("stop_weather_fx",					stop_weather_fx),
		def("get_moon_phase",					get_moon_phase),
		def("get_air_temperature_f",			get_air_temperature_f),
		def("get_air_temperature_fs",			get_air_temperature_fs),
		def("is_developer",						is_developer),
		def("environment",						environment),
		
		def("set_time_factor",					set_time_factor),
		def("get_time_factor",					get_time_factor),

		def("set_game_difficulty",				set_game_difficulty),
		def("get_game_difficulty",				get_game_difficulty),
		
		def("get_time_days",					get_time_days),
		def("get_time_hours",					get_time_hours),
		def("get_time_minutes",					get_time_minutes),
		def("change_game_time",					change_game_time),

		def("cover_in_direction",				cover_in_direction),
		def("vertex_in_direction",				vertex_in_direction),
		def("rain_factor",						rain_factor),
		def("patrol_path_exists",				patrol_path_exists),
		def("vertex_position",					vertex_position),
		def("name",								get_name),
		def("prefetch_sound",					prefetch_sound),
		def("patrol_path_add",					&patrol_path_add ),
		def("patrol_path_remove",				&patrol_path_remove ),

		def("client_spawn_manager",				get_client_spawn_manager),

		def("rain_wetness",						rain_wetness),
		def("rain_hemi",						rain_hemi),

		def("air_temperature",					air_temperature),

		def("map_add_object_spot_ser",			map_add_object_spot_ser),
		def("map_add_object_spot",				map_add_object_spot),
		def("map_remove_object_spot",			map_remove_object_spot),
		def("map_has_object_spot",				map_has_object_spot),
		def("map_change_spot_hint",				map_change_spot_hint),

		def("start_stop_menu",					start_stop_menu),
		def("add_dialog_to_render",				add_dialog_to_render),
		def("remove_dialog_to_render",			remove_dialog_to_render),
		def("main_input_receiver",				main_input_receiver),
		def("hide_indicators",					hide_indicators),
		def("show_indicators",					show_indicators),
		def("add_call",							((void (*) (const luabind::functor<bool> &,const luabind::functor<void> &)) &add_call)),
		def("add_call",							((void (*) (const luabind::object &,const luabind::functor<bool> &,const luabind::functor<void> &)) &add_call)),
		def("add_call",							((void (*) (const luabind::object &, LPCSTR, LPCSTR)) &add_call)),
		def("remove_call",						((void (*) (const luabind::functor<bool> &,const luabind::functor<void> &)) &remove_call)),
		def("remove_call",						((void (*) (const luabind::object &,const luabind::functor<bool> &,const luabind::functor<void> &)) &remove_call)),
		def("remove_call",						((void (*) (const luabind::object &, LPCSTR, LPCSTR)) &remove_call)),
		def("remove_calls_for_object",			remove_calls_for_object),
		def("present",							is_level_present),
		def("disable_input",					disable_input),
		def("enable_input",						enable_input),
		def("spawn_phantom",					spawn_phantom),

		def("get_bounding_volume",				get_bounding_volume),

		def("iterate_sounds",					&iterate_sounds1),
		def("iterate_sounds",					&iterate_sounds2),
		def("physics_world",					&physics_world),
		def("get_snd_volume",					&get_snd_volume),
		def("set_snd_volume",					&set_snd_volume),
		def("add_cam_effector",					&add_cam_effector),
		def("add_cam_effector2",				&add_cam_effector2),
		def("remove_cam_effector",				&remove_cam_effector),
		def("add_pp_effector",					&add_pp_effector),
		def("set_pp_effector_factor",			&set_pp_effector_factor),
		def("set_pp_effector_factor",			&set_pp_effector_factor2),
		def("remove_pp_effector",				&remove_pp_effector),

		def("add_complex_effector",				&add_complex_effector),
		def("remove_complex_effector",			&remove_complex_effector),

		def("nearest_vertex_id",				&nearest_vertex_id),
		def("valid_vertex_id",					valid_vertex_id),
		def("vertex_count",						vertex_count),
		
		def("game_id",							&GameID),
		def("create_custom_timer",				&create_custom_timer),
		def("start_custom_timer",				&start_custom_timer),
		def("stop_custom_timer",				&stop_custom_timer),
		def("reset_custom_timer",				&reset_custom_timer),
		def("delete_custom_timer",				&delete_custom_timer),
		def("get_custom_timer",					&get_custom_timer)
	],
	
	module(L,"actor_stats")
	[
		def("add_points",						&add_actor_points),
		def("add_points_str",					&add_actor_points_str),
		def("get_points",						&get_actor_points),
		def("add_to_ranking",					&add_human_to_top_list),
		def("remove_from_ranking",				&remove_human_from_top_list),
		def("get_actor_ranking",				&get_actor_ranking),
		def("set_skills_points",				&set_skills_points),
		def("inc_skills_points",				&inc_skills_points),
		def("dec_skills_points",				&dec_skills_points),
		def("get_skills_points",				&get_skills_points),
		def("set_survival_skill",				&set_survival_skill),
		def("get_survival_skill",				&get_survival_skill),
		def("inc_survival_skill",				&inc_survival_skill),
		def("dec_survival_skill",				&dec_survival_skill),
		def("set_power_skill",					&set_power_skill),
		def("get_power_skill",					&get_power_skill),
		def("inc_power_skill",					&inc_power_skill),
		def("dec_power_skill",					&dec_power_skill),
		def("set_repair_skill",					&set_repair_skill),
		def("get_repair_skill",					&get_repair_skill),
		def("inc_repair_skill",					&inc_repair_skill),
		def("dec_repair_skill",					&dec_repair_skill),
		def("set_endurance_skill",				&set_endurance_skill),
		def("get_endurance_skill",				&get_endurance_skill),
		def("inc_endurance_skill",				&inc_endurance_skill),
		def("dec_endurance_skill",				&dec_endurance_skill),
		def("buy_skill",						&buy_skill)
	];

	module(L)
	[
		def("command_line",						&command_line),
		def("IsGameTypeSingle",					&IsGameTypeSingle)
	];

	module(L,"relation_registry")
	[
		def("community_goodwill",				&g_community_goodwill),
		def("set_community_goodwill",			&g_set_community_goodwill),
		def("change_community_goodwill",		&g_change_community_goodwill)
	];
}
