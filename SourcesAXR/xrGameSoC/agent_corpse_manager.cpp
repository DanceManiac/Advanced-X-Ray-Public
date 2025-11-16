////////////////////////////////////////////////////////////////////////////
//	Module 		: agent_corpse_manager.cpp
//	Created 	: 24.05.2004
//  Modified 	: 14.01.2005
//	Author		: Dmitriy Iassenev
//	Description : Agent corpse manager
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "agent_corpse_manager.h"
#include "member_order.h"
#include "ai/stalker/ai_stalker.h"
#include "memory_manager.h"
#include "visual_memory_manager.h"
#include "agent_manager.h"
#include "agent_member_manager.h"

struct CRemoveMemberCorpsesPredicate {
	IC	bool operator()				(CMemberCorpse &corpse) const
	{
		return		(!!corpse.get_reactor());
	}
};

struct CRemoveOfflineCorpsesPredicate {
	CObject		*m_object;
	IC		 CRemoveOfflineCorpsesPredicate	(CObject *object)
	{
		VERIFY		(object);
		m_object	= object;
	}

	IC	bool operator()						(CMemberCorpse &corpse) const
	{
		return		(corpse.get_corpse()->ID() == m_object->ID());
	}
};

bool CAgentCorpseManager::process_corpse	(CMemberOrder &member)
{
	float			min_dist_sqr = flt_max;
	CMemberCorpse	*best_corpse = 0;
	xr_vector<CMemberCorpse>::iterator	I = m_corpses.begin();
	xr_vector<CMemberCorpse>::iterator	E = m_corpses.end();
	for ( ; I != E; ++I) {
		if (!member.object().get_memory().visual().visible_now((*I).get_corpse()))
			continue;

		float		dist_sqr = (*I).get_corpse()->Position().distance_to_sqr(member.object().Position());
		if (dist_sqr < min_dist_sqr) {
			if	(
					(*I).get_reactor() && 
					((*I).get_reactor()->Position().distance_to_sqr((*I).get_corpse()->Position()) <= min_dist_sqr)
				)
				continue;
			min_dist_sqr	= dist_sqr;
			best_corpse		= &*I;
		}
	}
	
	if (!best_corpse)
		return				(false);

	best_corpse->set_reactor	(&member.object());
	return					(true);
}

void CAgentCorpseManager::react_on_member_death			()
{
	for (;;) {
		bool						changed = false;
		CAgentMemberManager::MEMBER_STORAGE::iterator	I = object().get_member().combat_members().begin();
		CAgentMemberManager::MEMBER_STORAGE::iterator	E = object().get_member().combat_members().end();
		for ( ; I != E; ++I)
			if (!(*I)->member_death_reaction().m_processing)
				changed				= process_corpse(**I);

		if (!changed)
			break;
	}

	{
		MEMBER_CORPSES::iterator	I = m_corpses.begin();
		MEMBER_CORPSES::iterator	E = m_corpses.end();
		for ( ; I != E; ++I) {
			if (!(*I).get_reactor())
				continue;

			CMemberOrder::CMemberDeathReaction	&reaction = object().get_member().member((*I).get_reactor()).member_death_reaction();
			reaction.m_member		= (*I).get_corpse();
			reaction.m_time			= (*I).get_time();
			reaction.m_processing	= true;
		}

		m_corpses.erase				(
			std::remove_if(
				m_corpses.begin(),
				m_corpses.end(),
				CRemoveMemberCorpsesPredicate()
			),
			m_corpses.end()
		);
	}
}

void CAgentCorpseManager::remove_links	(CObject *object)
{
	m_corpses.erase				(
		std::remove_if(
			m_corpses.begin(),
			m_corpses.end(),
			CRemoveOfflineCorpsesPredicate(object)
		),
		m_corpses.end()
	);
}

void CAgentCorpseManager::update		()
{
}
