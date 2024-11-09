////////////////////////////////////////////////////////////////////////////
//	Module 		: agent_manager.cpp
//	Created 	: 24.05.2004
//  Modified 	: 24.05.2004
//	Author		: Dmitriy Iassenev
//	Description : Agent manager
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "agent_manager.h"
#include "agent_corpse_manager.h"
#include "agent_enemy_manager.h"
#include "agent_explosive_manager.h"
#include "agent_location_manager.h"
#include "agent_member_manager.h"
#include "agent_memory_manager.h"
#include "agent_manager_planner.h"
#include "profiler.h"

CAgentManager::CAgentManager			()
{
	init_scheduler				();
	init_components				();
}

CAgentManager::~CAgentManager			()
{
	VERIFY						(get_member().members().empty());
#ifdef USE_SCHEDULER_IN_AGENT_MANAGER
	remove_scheduler			();
#endif // USE_SCHEDULER_IN_AGENT_MANAGER
	remove_components			();
}

void CAgentManager::init_scheduler		()
{
#ifdef USE_SCHEDULER_IN_AGENT_MANAGER
	shedule.t_min				= 1000;
	shedule.t_max				= 1000;
	shedule_register			();
#else // USE_SCHEDULER_IN_AGENT_MANAGER
	m_last_update_time			= 0;
	m_update_rate				= 1000;
#endif // USE_SCHEDULER_IN_AGENT_MANAGER
}

void CAgentManager::init_components		()
{
	m_corpse					= xr_new<CAgentCorpseManager>	(this);
	m_enemy						= xr_new<CAgentEnemyManager>	(this);
	m_explosive					= xr_new<CAgentExplosiveManager>(this);
	m_location					= xr_new<CAgentLocationManager>	(this);
	m_member					= xr_new<CAgentMemberManager>	(this);
	m_memory					= xr_new<CAgentMemoryManager>	(this);
	m_brain						= xr_new<CAgentManagerPlanner>	();
	get_brain().setup				(this);
}

#ifdef USE_SCHEDULER_IN_AGENT_MANAGER
void CAgentManager::remove_scheduler	()
{
	shedule_unregister			();
}
#endif // USE_SCHEDULER_IN_AGENT_MANAGER

void CAgentManager::remove_components	()
{
	xr_delete					(m_corpse);
	xr_delete					(m_enemy);
	xr_delete					(m_explosive);
	xr_delete					(m_location);
	xr_delete					(m_member);
	xr_delete					(m_memory);
	xr_delete					(m_brain);
}

void CAgentManager::remove_links		(CObject *object)
{
	get_corpse().remove_links		(object);
	get_enemy().remove_links		(object);
	get_explosive().remove_links	(object);
	get_location().remove_links		(object);
	get_member().remove_links		(object);
	get_memory().remove_links		(object);
	get_brain().remove_links		(object);
}

void CAgentManager::update_impl			()
{
	VERIFY						(!get_member().members().empty());

	get_memory().update				();
	get_corpse().update				();
	get_enemy().update				();
	get_explosive().update			();
	get_location().update			();
	get_member().update				();
	get_brain().update				();
}

#ifdef USE_SCHEDULER_IN_AGENT_MANAGER
void CAgentManager::shedule_Update		(u32 time_delta)
{
	START_PROFILE("Agent_Manager")

	ISheduled::shedule_Update	(time_delta);

	update_impl					();

	STOP_PROFILE
}

float CAgentManager::shedule_Scale		()
{
	return						(.5f);
}

#else // USE_SCHEDULER_IN_AGENT_MANAGER

void CAgentManager::update				()
{
	if (Device.dwTimeGlobal <= m_last_update_time)
		return;

	if (Device.dwTimeGlobal - m_last_update_time < m_update_rate)
		return;

	m_last_update_time			= Device.dwTimeGlobal;
	update_impl					();
}

#endif // USE_SCHEDULER_IN_AGENT_MANAGER