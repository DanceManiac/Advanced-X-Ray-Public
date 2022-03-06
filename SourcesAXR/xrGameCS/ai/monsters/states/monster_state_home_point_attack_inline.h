#pragma once

#include "state_move_to_point.h"
#include "state_look_point.h"
#include "state_custom_action.h"
#include "../../../cover_point.h"
#include "../monster_cover_manager.h"
#include "../monster_home.h"


#define TEMPLATE_SPECIALIZATION template <\
	typename _Object\
>

#define CStateMonsterAttackMoveToHomePointAbstract CStateMonsterAttackMoveToHomePoint<_Object>

//////////////////////////////////////////////////////////////////////////
// Construct Substates
//////////////////////////////////////////////////////////////////////////

TEMPLATE_SPECIALIZATION
CStateMonsterAttackMoveToHomePointAbstract::CStateMonsterAttackMoveToHomePoint(_Object *obj) 
                             : inherited(obj), m_ignore_enemies_while_hide(0)
{
	add_state	(eStateAttack_HomePoint_Hide,			xr_new<CStateMonsterMoveToPointEx<_Object> >	(obj));
	add_state	(eStateAttack_HomePoint_LookOpenPlace,	xr_new<CStateMonsterLookToPoint<_Object> >		(obj));
	add_state	(eStateAttack_HomePoint_Camp,			xr_new<CStateMonsterCustomAction<_Object> >		(obj));
}

TEMPLATE_SPECIALIZATION
CStateMonsterAttackMoveToHomePointAbstract::CStateMonsterAttackMoveToHomePoint(_Object *obj, bool ignore_enemies_while_hide) 
                             : inherited(obj), m_ignore_enemies_while_hide(ignore_enemies_while_hide)
{
	add_state	(eStateAttack_HomePoint_Hide,			xr_new<CStateMonsterMoveToPointEx<_Object> >	(obj));
	add_state	(eStateAttack_HomePoint_LookOpenPlace,	xr_new<CStateMonsterLookToPoint<_Object> >		(obj));
	add_state	(eStateAttack_HomePoint_Camp,			xr_new<CStateMonsterCustomAction<_Object> >		(obj));
}

//////////////////////////////////////////////////////////////////////////
// Initialize/Finalize
//////////////////////////////////////////////////////////////////////////

TEMPLATE_SPECIALIZATION
void CStateMonsterAttackMoveToHomePointAbstract::initialize()
{
	inherited::initialize	();
	m_target_node			= object->Home->get_place_in_cover();
	m_skip_camp				= false;

	if (m_target_node == u32(-1)) {
		m_target_node	= object->Home->get_place();
		m_skip_camp		= true;
	}

	CMonsterSquad *squad = monster_squad().get_squad(object);
	squad->lock_cover(m_target_node);
}

TEMPLATE_SPECIALIZATION
void CStateMonsterAttackMoveToHomePointAbstract::finalize()
{
	inherited::finalize();
	CMonsterSquad *squad = monster_squad().get_squad(object);
	squad->unlock_cover(m_target_node);
}

TEMPLATE_SPECIALIZATION
void CStateMonsterAttackMoveToHomePointAbstract::critical_finalize()
{
	inherited::critical_finalize();

	CMonsterSquad *squad = monster_squad().get_squad(object);
	squad->unlock_cover(m_target_node);
}

//////////////////////////////////////////////////////////////////////////
// Check Start Conditions / Completion
//////////////////////////////////////////////////////////////////////////

TEMPLATE_SPECIALIZATION
bool CStateMonsterAttackMoveToHomePointAbstract::check_start_conditions()
{
	if ( !object->Home->at_home() )
	{
		return true;
	}

	const CEntityAlive* enemy = object->EnemyMan.get_enemy();
	const Fvector enemy_pos = enemy->Position();
		
	if ( !object->Home->at_home(enemy_pos) )
	{
		return true;
	}

	return false;
}

TEMPLATE_SPECIALIZATION
bool CStateMonsterAttackMoveToHomePointAbstract::check_completion()
{
	if ( !m_ignore_enemies_while_hide )
	{
		if (object->HitMemory.get_last_hit_time() > time_state_started) return true;
		if (object->EnemyMan.see_enemy_now() && object->Home->at_home(object->EnemyMan.get_enemy()->Position())) return true;
	}
	else
	{
		if (object->EnemyMan.see_enemy_now() && object->Home->at_min_home(object->EnemyMan.get_enemy()->Position())) return true;
	}
	
	if (object->ai_location().level_vertex_id() == m_target_node && !object->control().path_builder().is_moving_on_path()) return true;
	if (m_skip_camp && (prev_substate != u32(-1)) && (prev_substate != eStateAttack_HomePoint_Hide) ) return true;
	
	return false;
}

//////////////////////////////////////////////////////////////////////////
// Select Substate
//////////////////////////////////////////////////////////////////////////

TEMPLATE_SPECIALIZATION
void CStateMonsterAttackMoveToHomePointAbstract::reselect_state()
{
	if (prev_substate == u32(-1)) {
		select_state(eStateAttack_HomePoint_Hide);
		return;
	} 
	
	if (prev_substate == eStateAttack_HomePoint_Hide) {
		select_state(eStateAttack_HomePoint_LookOpenPlace);
		return;
	}

	select_state(eStateAttack_HomePoint_Camp);
}

//////////////////////////////////////////////////////////////////////////
// Setup Substates
//////////////////////////////////////////////////////////////////////////

TEMPLATE_SPECIALIZATION
void CStateMonsterAttackMoveToHomePointAbstract::setup_substates()
{
	state_ptr state = get_state_current();

	if (current_substate == eStateAttack_HomePoint_Hide) {
		SStateDataMoveToPointEx data;

		// in this mode we run to max distance 
		if ( m_ignore_enemies_while_hide )
		{
 			const CEntityAlive* enemy = object->EnemyMan.get_enemy();
 			Fvector enemy2home = object->Home->get_home_point();
 			enemy2home.sub( enemy->Position() );
 			enemy2home.normalize_safe();
 			m_target_node			= object->Home->get_place_in_max_home_to_direction(enemy2home);
		}

		data.vertex				= m_target_node;
		data.point				= ai().level_graph().vertex_position(data.vertex);
		data.action.action		= ACT_RUN;
		data.action.time_out	= 0;		// do not use time out
		data.completion_dist	= 0.f;		// get exactly to the point
		data.time_to_rebuild	= 0;		// do not rebuild
		data.accelerated		= true;
		data.braking			= false;
		data.accel_type 		= eAT_Aggressive;
		data.action.sound_type	= MonsterSound::eMonsterSoundAggressive;
		data.action.sound_delay = object->db().m_dwAttackSndDelay;

		state->fill_data_with(&data, sizeof(SStateDataMoveToPointEx));
		return;
	}

	if (current_substate == eStateAttack_HomePoint_LookOpenPlace) {

		SStateDataLookToPoint	data;

		Fvector dir;
		object->CoverMan->less_cover_direction(dir);

		data.point.mad			(object->Position(),dir,10.f);
		data.action.action		= ACT_STAND_IDLE;
		data.action.time_out	= 2000;		
		data.action.sound_type	= MonsterSound::eMonsterSoundAggressive;
		data.action.sound_delay = object->db().m_dwIdleSndDelay;
		data.face_delay			= 0;

		state->fill_data_with(&data, sizeof(SStateDataLookToPoint));
		return;
	}

	if (current_substate == eStateAttack_HomePoint_Camp) {
		SStateDataAction data;

		data.action		= ACT_LOOK_AROUND;
		data.time_out	= 0;			// do not use time out
		data.sound_type	= MonsterSound::eMonsterSoundAggressive;
		data.sound_delay = object->db().m_dwIdleSndDelay;

		state->fill_data_with(&data, sizeof(SStateDataAction));

		return;
	}
}

#undef TEMPLATE_SPECIALIZATION
#undef CStateMonsterAttackMoveToHomePointAbstract
