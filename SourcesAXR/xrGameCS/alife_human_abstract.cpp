////////////////////////////////////////////////////////////////////////////
//	Module 		: alife_human_abstract.cpp
//	Created 	: 27.10.2005
//  Modified 	: 27.10.2005
//	Author		: Dmitriy Iassenev
//	Description : ALife human abstract class
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "xrServer_objects_ALife_Monsters.h"
#include "alife_human_brain.h"
#include "alife_human_object_handler.h"
#include "ai_space.h"
#include "alife_simulator.h"
#include "alife_group_registry.h"
#include "relation_registry.h"

void CSE_ALifeHumanAbstract::update									()
{
	if (!bfActive())
		return;

	get_brain().update							();
}

bool CSE_ALifeHumanAbstract::bfPerformAttack						()
{
	return									(get_brain().perform_attack());
}

ALife::EMeetActionType CSE_ALifeHumanAbstract::tfGetActionType		(CSE_ALifeSchedulable *schedulable, int iGroupIndex, bool bMutualDetection)
{
	return									(get_brain().action_type(schedulable,iGroupIndex,bMutualDetection));
}

void CSE_ALifeHumanAbstract::vfDetachAll							(bool bFictitious)
{
	get_brain().objects().detach_all			(bFictitious);
}

void CSE_ALifeHumanAbstract::vfUpdateWeaponAmmo						()
{
	get_brain().objects().update_weapon_ammo	();
}

void CSE_ALifeHumanAbstract::vfProcessItems							()
{
	get_brain().objects().process_items			();
}

void CSE_ALifeHumanAbstract::vfAttachItems							(ALife::ETakeType tTakeType)
{
	get_brain().objects().attach_items			();
}

CSE_ALifeDynamicObject *CSE_ALifeHumanAbstract::tpfGetBestDetector	()
{
	return									(get_brain().objects().best_detector());
}

CSE_ALifeItemWeapon *CSE_ALifeHumanAbstract::tpfGetBestWeapon		(ALife::EHitType &tHitType, float &fHitPower)
{
	return									(get_brain().objects().best_weapon());
}

void CSE_ALifeHumanAbstract::on_register							()
{
	inherited2::on_register					();
	get_brain().on_register						();
	// because we need to load profile to setup graph vertex masks
	specific_character						();
}

void CSE_ALifeHumanAbstract::on_unregister							()
{
	CSE_ALifeMonsterAbstract::on_unregister							();
	
	RELATION_REGISTRY().ClearRelations								(ID);

	get_brain().on_unregister					();
	if (m_group_id != 0xffff)
		ai().alife().groups().object(m_group_id).unregister_member	(ID);
}

void CSE_ALifeHumanAbstract::spawn_supplies							()
{
	specific_character			();
	inherited1::spawn_supplies	();
	inherited2::spawn_supplies	();
}

void CSE_ALifeHumanAbstract::add_online								(const bool &update_registries)
{
	CSE_ALifeTraderAbstract::add_online		(update_registries);
	get_brain().on_switch_online				();
}

void CSE_ALifeHumanAbstract::add_offline							(const xr_vector<ALife::_OBJECT_ID> &saved_children, const bool &update_registries)
{
	CSE_ALifeTraderAbstract::add_offline	(saved_children,update_registries);
	get_brain().on_switch_offline				();
}
