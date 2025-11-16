////////////////////////////////////////////////////////////////////////////
//	Module 		: stalker_danger_in_direction_actions.cpp
//	Created 	: 31.05.2005
//  Modified 	: 31.05.2005
//	Author		: Dmitriy Iassenev
//	Description : Stalker danger in direction actions classes
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "stalker_danger_in_direction_actions.h"
#include "ai/stalker/ai_stalker.h"
#include "script_game_object.h"
#include "stalker_movement_manager.h"
#include "sight_manager.h"
#include "object_handler.h"
#include "movement_manager_space.h"
#include "detail_path_manager_space.h"
#include "stalker_decision_space.h"
#include "memory_manager.h"
#include "danger_manager.h"
#include "agent_manager.h"
#include "agent_member_manager.h"
#include "agent_location_manager.h"
#include "cover_point.h"
#include "danger_cover_location.h"
#include "cover_evaluators.h"
#include "ai_space.h"
#include "cover_manager.h"
#include "stalker_movement_restriction.h"

using namespace StalkerDecisionSpace;

extern float current_cover	(CAI_Stalker *object);

//////////////////////////////////////////////////////////////////////////
// CStalkerActionDangerInDirectionTakeCover
//////////////////////////////////////////////////////////////////////////

CStalkerActionDangerInDirectionTakeCover::CStalkerActionDangerInDirectionTakeCover	(CAI_Stalker *object, LPCSTR action_name) :
	inherited				(object,action_name)
{
}

void CStalkerActionDangerInDirectionTakeCover::initialize						()
{
	inherited::initialize	();

	object().get_movement().set_mental_state		(eMentalStateDanger);
	object().get_movement().set_body_state			(eBodyStateStand);
	object().get_movement().set_path_type			(MovementManager::ePathTypeLevelPath);
	object().get_movement().set_detail_path_type	(DetailPathManager::eDetailPathTypeSmooth);
	object().get_movement().set_movement_type		(::Random.randI(2) ? eMovementTypeRun : eMovementTypeWalk);
	u32											min_queue_size, max_queue_size, min_queue_interval, max_queue_interval;
	float										distance = object().get_memory().danger().selected()->position().distance_to(object().Position());
	select_queue_params							(distance,min_queue_size, max_queue_size, min_queue_interval, max_queue_interval);
	object().CObjectHandler::set_goal			(eObjectActionAimReady1,object().best_weapon(),min_queue_size, max_queue_size, min_queue_interval, max_queue_interval);
}

void CStalkerActionDangerInDirectionTakeCover::execute							()
{
	inherited::execute		();

	if (!object().get_memory().danger().selected())
		return;

	Fvector								position = object().get_memory().danger().selected()->position();

	object().get_sight().setup				(CSightAction(SightManager::eSightTypePosition,position,true));

	object().m_ce_best->setup			(position,10.f,170.f,10.f);
	const CCoverPoint					*point = ai().cover_manager().best_cover(object().Position(),10.f,*object().m_ce_best,CStalkerMovementRestrictor(m_object,true));
	if (!point) {
		object().m_ce_best->setup		(position,10.f,170.f,10.f);
		point							= ai().cover_manager().best_cover(object().Position(),30.f,*object().m_ce_best,CStalkerMovementRestrictor(m_object,true));
	}

	if (point) {
		object().get_movement().set_level_dest_vertex	(point->level_vertex_id());
		object().get_movement().set_desired_position	(&point->position());
//		if (object().get_movement().path_completed() && object().Position().distance_to(point->position()) < 1.f)
//			object().get_brain().affect_cover			(true);
//		else
//			object().get_brain().affect_cover			(false);
	}
	else {
		object().get_movement().set_nearest_accessible_position	();
//		object().get_brain().affect_cover				(true);
	}

	if (object().get_movement().path_completed())// && (object().get_memory().get_enemy().selected()->Position().distance_to_sqr(object().Position()) >= 10.f))
		m_storage->set_property			(eWorldPropertyInCover,true);
}

void CStalkerActionDangerInDirectionTakeCover::finalize							()
{
	inherited::finalize		();
}

//////////////////////////////////////////////////////////////////////////
// CStalkerActionDangerInDirectionLookOut
//////////////////////////////////////////////////////////////////////////

CStalkerActionDangerInDirectionLookOut::CStalkerActionDangerInDirectionLookOut	(CAI_Stalker *object, LPCSTR action_name) :
	inherited				(object,action_name)
{
	m_crouch_look_out_random.seed				(u32(CPU::QPC() & 0xffffffff));
}

void CStalkerActionDangerInDirectionLookOut::initialize							()
{
	inherited::initialize	();

	m_storage->set_property						(eWorldPropertyUseCrouchToLookOut,	!!m_crouch_look_out_random.random(2));

	object().get_movement().set_desired_direction	(0);
	object().get_movement().set_path_type			(MovementManager::ePathTypeLevelPath);
	object().get_movement().set_detail_path_type	(DetailPathManager::eDetailPathTypeSmooth);
	object().get_movement().set_mental_state		(eMentalStateDanger);

	object().get_movement().set_body_state			(m_storage->property(eWorldPropertyUseCrouchToLookOut) ? eBodyStateCrouch : eBodyStateStand);
	object().get_movement().set_movement_type		(eMovementTypeWalk);
	object().get_movement().set_nearest_accessible_position	();

	u32											min_queue_size, max_queue_size, min_queue_interval, max_queue_interval;
	float										distance = object().get_memory().danger().selected()->position().distance_to(object().Position());
	select_queue_params							(distance,min_queue_size, max_queue_size, min_queue_interval, max_queue_interval);
	object().CObjectHandler::set_goal			(eObjectActionAimReady1,object().best_weapon(),min_queue_size, max_queue_size, min_queue_interval, max_queue_interval);

	set_inertia_time							(1000);
//	object().get_brain().affect_cover				(true);
}

void CStalkerActionDangerInDirectionLookOut::execute							()
{
	inherited::execute		();

//	CMemoryInfo							mem_object = object().get_memory().memory(object().get_memory().danger().selected()->object());
//
//	if (!mem_object.m_object)
//		return;
	Fvector								position = object().get_memory().danger().selected()->position();

	object().get_sight().setup				(CSightAction(SightManager::eSightTypePosition,position,true));

	if (current_cover(m_object) >= 3.f) {
		object().get_movement().set_nearest_accessible_position	();
		m_storage->set_property			(eWorldPropertyLookedOut,true);
		return;
	}

//	Fvector								position = mem_object.m_object_params.m_position;
	object().m_ce_close->setup			(position,10.f,170.f,10.f);
	const CCoverPoint					*point = ai().cover_manager().best_cover(object().Position(),10.f,*object().m_ce_close,CStalkerMovementRestrictor(m_object,true,false));
	if (!point || (point->position().similar(object().Position()) && object().get_movement().path_completed())) {
		object().m_ce_close->setup		(position,10.f,170.f,10.f);
		point							= ai().cover_manager().best_cover(object().Position(),30.f,*object().m_ce_close,CStalkerMovementRestrictor(m_object,true,false));
	}

	if (point) {
		object().get_movement().set_level_dest_vertex	(point->level_vertex_id());
		object().get_movement().set_desired_position	(&point->position());
	}
	else
		object().get_movement().set_nearest_accessible_position	();

	if (point && point->position().similar(object().Position(),.5f) && object().get_movement().path_completed()) {
		m_storage->set_property			(eWorldPropertyLookedOut,true);
		object().get_movement().set_nearest_accessible_position	();
	}
}

void CStalkerActionDangerInDirectionLookOut::finalize							()
{
	inherited::finalize		();
}

//////////////////////////////////////////////////////////////////////////
// CStalkerActionDangerInDirectionHoldPosition
//////////////////////////////////////////////////////////////////////////

CStalkerActionDangerInDirectionHoldPosition::CStalkerActionDangerInDirectionHoldPosition	(CAI_Stalker *object, LPCSTR action_name) :
	inherited				(object,action_name)
{
}

void CStalkerActionDangerInDirectionHoldPosition::initialize					()
{
	inherited::initialize	();

	object().get_movement().set_desired_direction	(0);
	object().get_movement().set_path_type			(MovementManager::ePathTypeLevelPath);
	object().get_movement().set_detail_path_type	(DetailPathManager::eDetailPathTypeSmooth);
	object().get_movement().set_nearest_accessible_position		();
	object().get_movement().set_mental_state		(eMentalStateDanger);
	object().get_movement().set_body_state			(m_storage->property(eWorldPropertyUseCrouchToLookOut) ? eBodyStateCrouch : eBodyStateStand);
	object().get_movement().set_movement_type		(eMovementTypeStand);

	u32											min_queue_size, max_queue_size, min_queue_interval, max_queue_interval;
	float										distance = object().get_memory().danger().selected()->position().distance_to(object().Position());
	select_queue_params							(distance,min_queue_size, max_queue_size, min_queue_interval, max_queue_interval);
	object().CObjectHandler::set_goal			(eObjectActionAimReady1,object().best_weapon(),min_queue_size, max_queue_size, min_queue_interval, max_queue_interval);

	set_inertia_time							(5000 + ::Random32.random(5000));
//	object().get_brain().affect_cover				(true);
}

void CStalkerActionDangerInDirectionHoldPosition::execute						()
{
	inherited::execute		();

//	CMemoryInfo							mem_object = object().get_memory().memory(object().get_memory().danger().selected()->object());
//
//	if (!mem_object.m_object)
//		return;

	Fvector								position = object().get_memory().danger().selected()->position();

	if (current_cover(m_object) < 3.f)
		m_storage->set_property			(eWorldPropertyLookedOut,false);

	object().get_sight().setup				(CSightAction(SightManager::eSightTypePosition,position,true));
	
	if (completed() && object().agent_manager().get_member().can_detour()) {
		m_storage->set_property			(eWorldPropertyPositionHolded,true);
		m_storage->set_property			(eWorldPropertyInCover,false);
	}

	u32									min_queue_size, max_queue_size, min_queue_interval, max_queue_interval;
	float								distance = position.distance_to(object().Position());
	select_queue_params					(distance,min_queue_size, max_queue_size, min_queue_interval, max_queue_interval);
	object().CObjectHandler::set_goal	(eObjectActionAimReady1,object().best_weapon(),min_queue_size, max_queue_size, min_queue_interval, max_queue_interval);
}

void CStalkerActionDangerInDirectionHoldPosition::finalize						()
{
	inherited::finalize		();
}

//////////////////////////////////////////////////////////////////////////
// CStalkerActionDangerInDirectionDetour
//////////////////////////////////////////////////////////////////////////

CStalkerActionDangerInDirectionDetour::CStalkerActionDangerInDirectionDetour	(CAI_Stalker *object, LPCSTR action_name) :
	inherited				(object,action_name)
{
}

void CStalkerActionDangerInDirectionDetour::initialize							()
{
	inherited::initialize						();

	object().agent_manager().get_member().member	(&object()).detour	(true);
	object().get_movement().set_desired_direction	(0);
	object().get_movement().set_path_type			(MovementManager::ePathTypeLevelPath);
	object().get_movement().set_detail_path_type	(DetailPathManager::eDetailPathTypeSmooth);
	object().get_movement().set_body_state			(eBodyStateStand);
	object().get_movement().set_movement_type		(eMovementTypeWalk);
	object().get_movement().set_mental_state		(eMentalStateDanger);
	u32											min_queue_size, max_queue_size, min_queue_interval, max_queue_interval;
	float										distance = object().get_memory().danger().selected()->position().distance_to(object().Position());
	select_queue_params							(distance,min_queue_size, max_queue_size, min_queue_interval, max_queue_interval);
	object().CObjectHandler::set_goal			(eObjectActionAimReady1,object().best_weapon(),min_queue_size, max_queue_size, min_queue_interval, max_queue_interval);
	object().agent_manager().get_member().member(m_object).cover(0);
}

void CStalkerActionDangerInDirectionDetour::execute								()
{
	inherited::execute		();

	if (!object().get_memory().danger().selected()->object())
		return;

	CMemoryInfo							mem_object = object().get_memory().memory(object().get_memory().danger().selected()->object());

	if (!mem_object.m_object)
		return;

	Fvector								position = object().get_memory().danger().selected()->position();

	if (object().get_movement().path_completed()) {
		object().m_ce_angle->setup			(position,10.f,object().ffGetRange(),mem_object.m_object_params.m_level_vertex_id);
		const CCoverPoint					*point = ai().cover_manager().best_cover(object().Position(),10.f,*object().m_ce_angle,CStalkerMovementRestrictor(m_object,true));
		if (!point) {
			object().m_ce_angle->setup		(position,10.f,object().ffGetRange(),mem_object.m_object_params.m_level_vertex_id);
			point							= ai().cover_manager().best_cover(object().Position(),30.f,*object().m_ce_angle,CStalkerMovementRestrictor(m_object,true));
		}

		if (point) {
			object().get_movement().set_level_dest_vertex	(point->level_vertex_id());
			object().get_movement().set_desired_position	(&point->position());
		}
		else
			object().get_movement().set_nearest_accessible_position	();

		if (object().get_movement().path_completed())
			m_storage->set_property			(eWorldPropertyEnemyDetoured,true);
	}

	object().get_sight().setup					(CSightAction(SightManager::eSightTypePosition,mem_object.m_object_params.m_position,true));
}

void CStalkerActionDangerInDirectionDetour::finalize							()
{
	inherited::finalize		();
}

//////////////////////////////////////////////////////////////////////////
// CStalkerActionDangerInDirectionSearch
//////////////////////////////////////////////////////////////////////////

CStalkerActionDangerInDirectionSearch::CStalkerActionDangerInDirectionSearch	(CAI_Stalker *object, LPCSTR action_name) :
	inherited				(object,action_name)
{
}

void CStalkerActionDangerInDirectionSearch::initialize						()
{
	inherited::initialize	();

	object().get_movement().set_desired_direction		(0);
	object().get_movement().set_path_type				(MovementManager::ePathTypeLevelPath);
	object().get_movement().set_detail_path_type		(DetailPathManager::eDetailPathTypeSmooth);
	object().get_movement().set_body_state				(eBodyStateStand);
	object().get_movement().set_movement_type			(eMovementTypeWalk);
	object().get_movement().set_mental_state			(eMentalStateDanger);

	u32												min_queue_size, max_queue_size, min_queue_interval, max_queue_interval;
	float											distance = object().get_memory().danger().selected()->position().distance_to(object().Position());
	select_queue_params								(distance,min_queue_size, max_queue_size, min_queue_interval, max_queue_interval);
	object().CObjectHandler::set_goal				(eObjectActionAimReady1,object().best_weapon(),min_queue_size, max_queue_size, min_queue_interval, max_queue_interval);

	object().agent_manager().get_member().member(m_object).cover(0);
}

void CStalkerActionDangerInDirectionSearch::execute							()
{
	inherited::execute		();

	if (!object().get_memory().danger().selected()->object())
		return;

	CMemoryInfo							mem_object = object().get_memory().memory(object().get_memory().danger().selected()->object());

	if (!mem_object.m_object)
		return;

	Fvector								position = object().get_memory().danger().selected()->position();

	if (object().get_movement().path_completed()) {
		object().m_ce_ambush->setup		(position,mem_object.m_self_params.m_position,10.f);
		const CCoverPoint				*point = ai().cover_manager().best_cover(position,10.f,*object().m_ce_ambush,CStalkerMovementRestrictor(m_object,true));
		if (!point) {
			object().m_ce_ambush->setup	(position,mem_object.m_self_params.m_position,10.f);
			point						= ai().cover_manager().best_cover(position,30.f,*object().m_ce_ambush,CStalkerMovementRestrictor(m_object,true));
		}

		if (point) {
			object().get_movement().set_level_dest_vertex	(point->level_vertex_id());
			object().get_movement().set_desired_position	(&point->position());
		}
		else
			object().get_movement().set_nearest_accessible_position	();

		if (object().get_movement().path_completed() && completed()) {
			if (object().get_memory().danger().selected()->object())
				object().get_memory().enable(object().get_memory().danger().selected()->object(),false);
			else
				object().get_memory().danger().time_line(Device.dwTimeGlobal);
		}
	}

	object().get_sight().setup	(CSightAction(SightManager::eSightTypePosition,mem_object.m_object_params.m_position,true));
}

void CStalkerActionDangerInDirectionSearch::finalize							()
{
	inherited::finalize		();
}
