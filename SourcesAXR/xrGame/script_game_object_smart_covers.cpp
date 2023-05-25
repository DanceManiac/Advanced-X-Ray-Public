////////////////////////////////////////////////////////////////////////////
//	Module 		: script_game_object_smart_covers.cpp
//	Created 	: 14.02.2008
//  Modified 	: 14.02.2008
//	Author		: Dmitriy Iassenev
//	Description : script game object class smart covers stuff
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "script_game_object.h"
#include "script_game_object_impl.h"
#include "ai/stalker/ai_stalker.h"
#include "stalker_movement_manager_smart_cover.h"
#include "script_callback_ex.h"
#include "smart_cover.h"

bool CScriptGameObject::use_smart_covers_only		() const
{
	RMakeObj(CAI_Stalker,stalker,false);
	return								(stalker->use_smart_covers_only());
}

void CScriptGameObject::use_smart_covers_only		(bool value)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->use_smart_covers_only		(value);
}

void CScriptGameObject::set_smart_cover_target_selector	()
{
	MakeObj(CAI_Stalker,stalker);
	stalker->movement().target_selector	(CScriptCallbackEx<void>());
}

void CScriptGameObject::set_smart_cover_target_selector	(luabind::functor<void> functor)
{
	MakeObj(CAI_Stalker,stalker);
	CScriptCallbackEx<void>				callback;
	callback.set						(functor);
	stalker->movement().target_selector	(callback);
}

void CScriptGameObject::set_smart_cover_target_selector	(luabind::functor<void> functor, luabind::object object)
{
	MakeObj2(CAI_Stalker,stalker,&this->object());
	CScriptCallbackEx<void>				callback;
	callback.set						(functor, object);
	stalker->movement().target_selector	(callback);
}

void CScriptGameObject::set_smart_cover_target_idle		()
{
	MakeObj(CAI_Stalker,stalker);
	if (!stalker->g_Alive()) {
		Msg("! CAI_Stalker: do not call smart_cover_setup_idle_target when stalker is dead");
		return;
	}

	stalker->movement().target_idle		();
}

void CScriptGameObject::set_smart_cover_target_lookout	()
{
	MakeObj(CAI_Stalker,stalker);
	if (!stalker->g_Alive()) {
		Msg("! CAI_Stalker: do not call smart_cover_setup_lookout_target when stalker is dead");
		return;
	}

	stalker->movement().target_lookout	();
}

void CScriptGameObject::set_smart_cover_target_fire		()
{
	MakeObj(CAI_Stalker,stalker);
	if (!stalker->g_Alive()) {
		Msg("! CAI_Stalker: do not call smart_cover_setup_fire_target when stalker is dead");
		return;
	}

	stalker->movement().target_fire		();
}

void CScriptGameObject::set_smart_cover_target_fire_no_lookout()
{
	MakeObj(CAI_Stalker,stalker);
	if (!stalker->g_Alive()) {
		Msg("! CAI_Stalker: do not call set_smart_cover_target_fire_no_lookout when stalker is dead");
		return;
	}

	stalker->movement().target_fire_no_lookout	();
}

void CScriptGameObject::set_smart_cover_target_default		(bool value)
{
	MakeObj(CAI_Stalker,stalker);
	if (!stalker->g_Alive()) {
		Msg("! CAI_Stalker: do not call set_smart_cover_target_default when stalker is dead");
		return;
	}

	stalker->movement().target_default	(value);
}

bool CScriptGameObject::in_smart_cover						() const
{
	RMakeObj(CAI_Stalker,stalker,true);
	return								(stalker->movement().in_smart_cover());
}

void CScriptGameObject::set_dest_smart_cover						(LPCSTR cover_id)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->movement().target_params().cover_id(cover_id);
}

void CScriptGameObject::set_dest_smart_cover						()
{
	MakeObj(CAI_Stalker,stalker);
	stalker->movement().target_params().cover_id("");
}

CCoverPoint const* CScriptGameObject::get_dest_smart_cover			()
{
	RMakeObj(CAI_Stalker,stalker,NULL);
	return								(stalker->movement().target_params().cover());
}
LPCSTR CScriptGameObject::get_dest_smart_cover_name		()
{
	RMakeObj(CAI_Stalker,stalker,NULL);
	return								(stalker->movement().target_params().cover_id().c_str());
}

void CScriptGameObject::set_dest_loophole							(LPCSTR loophole_id)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->movement().target_params().cover_loophole_id(loophole_id);
}

void CScriptGameObject::set_dest_loophole							()
{
	MakeObj(CAI_Stalker,stalker);
	stalker->movement().target_params().cover_loophole_id("");
}

void CScriptGameObject::set_smart_cover_target						(Fvector value)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->movement().target_params().cover_fire_position		(&value);
}

void CScriptGameObject::set_smart_cover_target						()
{
	MakeObj(CAI_Stalker,stalker);
	stalker->movement().target_params().cover_fire_position		(0);
}

void CScriptGameObject::set_smart_cover_target						(CScriptGameObject *enemy_object)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->movement().target_params().cover_fire_object		(&enemy_object->object());
}

bool CScriptGameObject::in_loophole_fov					(LPCSTR cover_id, LPCSTR loophole_id, Fvector object_position) const
{
	RMakeObj(CAI_Stalker,stalker,false);
	return								(stalker->movement().in_fov(cover_id, loophole_id, object_position));
}

bool CScriptGameObject::in_current_loophole_fov			(Fvector object_position) const
{
	RMakeObj(CAI_Stalker,stalker,false);
	return								(stalker->movement().in_current_loophole_fov (object_position));
}

bool CScriptGameObject::in_loophole_range				(LPCSTR cover_id, LPCSTR loophole_id, Fvector object_position) const
{
	RMakeObj(CAI_Stalker,stalker,false);
	return								(stalker->movement().in_range (cover_id, loophole_id, object_position));
}

bool CScriptGameObject::in_current_loophole_range				(Fvector object_position) const
{
	RMakeObj(CAI_Stalker,stalker,false);
	return								(stalker->movement().in_current_loophole_range (object_position));
}

float const CScriptGameObject::idle_min_time					() const
{
	RMakeObj(CAI_Stalker,stalker,flt_max);
	return								(stalker->movement().idle_min_time());
}

void CScriptGameObject::idle_min_time							(float value)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->movement().idle_min_time	(value);
}

float const CScriptGameObject::idle_max_time					() const
{
	RMakeObj(CAI_Stalker,stalker,flt_max);
	return								(stalker->movement().idle_max_time());
}

void CScriptGameObject::idle_max_time							(float value)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->movement().idle_max_time	(value);
}

float const CScriptGameObject::lookout_min_time					() const
{
	RMakeObj(CAI_Stalker,stalker,flt_max);
	return								(stalker->movement().lookout_min_time());
}

void CScriptGameObject::lookout_min_time						(float value)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->movement().lookout_min_time(value);
}

float const CScriptGameObject::lookout_max_time					() const
{
	RMakeObj(CAI_Stalker,stalker,flt_max);
	return								(stalker->movement().lookout_max_time());
}

void CScriptGameObject::lookout_max_time						(float value)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->movement().lookout_max_time(value);
}

float CScriptGameObject::apply_loophole_direction_distance() const
{
	RMakeObj(CAI_Stalker,stalker,flt_max);
	return								(stalker->movement().apply_loophole_direction_distance());
}

void CScriptGameObject::apply_loophole_direction_distance		(float value)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->movement().apply_loophole_direction_distance	(value);
}

bool CScriptGameObject::movement_target_reached					()
{
	RMakeObj(CAI_Stalker,stalker,false);
	return								(stalker->movement().current_params().equal_to_target(stalker->movement().target_params()));
}