////////////////////////////////////////////////////////////////////////////
//	Module 		: agent_manager.h
//	Created 	: 24.05.2004
//  Modified 	: 24.05.2004
//	Author		: Dmitriy Iassenev
//	Description : Agent manager
////////////////////////////////////////////////////////////////////////////

#pragma once

class CAgentCorpseManager;
class CAgentEnemyManager;
class CAgentExplosiveManager;
class CAgentLocationManager;
class CAgentMemberManager;
class CAgentMemoryManager;
class CAgentManagerPlanner;

//#define USE_SCHEDULER_IN_AGENT_MANAGER

#ifdef USE_SCHEDULER_IN_AGENT_MANAGER
	class CAgentManager : public ISheduled {
#else // USE_SCHEDULER_IN_AGENT_MANAGER
	class CAgentManager {
#endif // USE_SCHEDULER_IN_AGENT_MANAGER

private:
	CAgentCorpseManager				*m_corpse;
	CAgentEnemyManager				*m_enemy;
	CAgentExplosiveManager			*m_explosive;
	CAgentLocationManager			*m_location;
	CAgentMemberManager				*m_member;
	CAgentMemoryManager				*m_memory;
	CAgentManagerPlanner			*m_brain;

#ifndef USE_SCHEDULER_IN_AGENT_MANAGER
private:
	u32								m_last_update_time;
	u32								m_update_rate;
#endif // USE_SCHEDULER_IN_AGENT_MANAGER

private:
			void					init_scheduler		();
			void					init_components		();
			void					remove_components	();
			void					update_impl			();

#ifdef USE_SCHEDULER_IN_AGENT_MANAGER
private:
			void					remove_scheduler	();
#endif // USE_SCHEDULER_IN_AGENT_MANAGER

public:
									CAgentManager		();
									// final class, no virtual destructor needed
									~CAgentManager		();
#ifdef USE_SCHEDULER_IN_AGENT_MANAGER
	virtual bool					shedule_Needed		()					{return true;};
	virtual float					shedule_Scale		();
	virtual void					shedule_Update		(u32 time_delta);	
	virtual	shared_str				shedule_Name		() const			{return shared_str("agent_manager"); };
#else // USE_SCHEDULER_IN_AGENT_MANAGER
			void					update				();
#endif // USE_SCHEDULER_IN_AGENT_MANAGER
			shared_str				cName				() const;
			void					remove_links		(CObject *object);

public:
	IC		CAgentCorpseManager		&get_corpse			() const;
	IC		CAgentEnemyManager		&get_enemy			() const;
	IC		CAgentExplosiveManager	&get_explosive		() const;
	IC		CAgentLocationManager	&get_location		() const;
	IC		CAgentMemberManager		&get_member			() const;
	IC		CAgentMemoryManager		&get_memory			() const;
	IC		CAgentManagerPlanner	&get_brain			() const;
};

#include "agent_manager_inline.h"