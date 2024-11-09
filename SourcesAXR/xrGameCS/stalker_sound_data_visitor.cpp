////////////////////////////////////////////////////////////////////////////
//	Module 		: stalker_sound_data_visitor.cpp
//	Created 	: 02.02.2005
//  Modified 	: 02.02.2005
//	Author		: Dmitriy Iassenev
//	Description : Stalker sound data visitor
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "stalker_sound_data_visitor.h"
#include "ai/stalker/ai_stalker.h"
#include "stalker_sound_data.h"
#include "agent_manager.h"
#include "agent_member_manager.h"
#include "memory_manager.h"
#include "hit_memory_manager.h"
#include "visual_memory_manager.h"
#include "enemy_manager.h"
#include "danger_manager.h"

CStalkerSoundDataVisitor::~CStalkerSoundDataVisitor	()
{
}

void CStalkerSoundDataVisitor::visit				(CStalkerSoundData *data)
{
	if (object().get_memory().get_enemy().selected())
		return;

	if (object().is_relation_enemy(&data->object()))
		return;

	if (!data->object().get_memory().get_enemy().selected()) {
		if (!object().get_memory().danger().selected() && data->object().get_memory().danger().selected())
			object().get_memory().danger().add	(*data->object().get_memory().danger().selected());
		return;
	}

	if (data->object().get_memory().get_enemy().selected()->getDestroy())
		return;

	if (!object().is_relation_enemy(data->object().get_memory().get_enemy().selected()))
		return;

	if (!data->object().g_Alive())
		return;

	if (!object().g_Alive())
		return;

	Msg								("%s : Adding fiction hit by sound info from stalker %s",*object().cName(),*data->object().cName());

	object().get_memory().make_object_visible_somewhen	(data->object().get_memory().get_enemy().selected());

//	const MemorySpace::CHitObject	*m = data->object().get_memory().hit().hit(data->object().get_memory().get_enemy().selected());
//	if (!m)
//		return;
//	object().get_memory().hit().add		(*m);
}
