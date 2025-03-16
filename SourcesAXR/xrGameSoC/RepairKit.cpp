////////////////////////////////////////////////////////////////////////////
//	Module 		: RepairKit.cpp
//	Created 	: 08.02.2023
//  Modified 	: 08.02.2023
//	Author		: Dance Maniac (M.F.S. Team)
//	Description : Repair kit
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RepairKit.h"
#include "Actor.h"
#include "inventory.h"
#include "CustomOutfit.h"
#include "ActorHelmet.h"
#include "Weapon.h"
#include "WeaponKnife.h"

CRepairKit::CRepairKit()
{
	m_iPortionsNum = -1;
	m_iUseFor = 0;
	m_fRestoreCondition = 0.0f;
	//m_physic_item = 0;
}

CRepairKit::~CRepairKit()
{
}

void CRepairKit::Load(LPCSTR section)
{
	inherited::Load(section);

	m_iPortionsNum = pSettings->r_s32(section, "eat_portions_num");
	m_fRestoreCondition = READ_IF_EXISTS(pSettings, r_float, section, "restore_condition", 0.5f);
	VERIFY(m_iPortionsNum < 10000);
}

BOOL CRepairKit::net_Spawn(CSE_Abstract* DC)
{
	if (!inherited::net_Spawn(DC)) return FALSE;

	return TRUE;
};

bool CRepairKit::Useful() const
{
	if (!inherited::Useful()) return false;

	//проверить не все ли еще съедено
	if (m_iPortionsNum == 0) return false;

	return true;
}

bool CRepairKit::UseAllowed()
{
	if (Actor()->ActorSkills && Actor()->ActorSkills->repairSkillLevel < 1) return false;

	CCustomOutfit* outfit = smart_cast<CCustomOutfit*>(Actor()->inventory().ItemFromSlot(OUTFIT_SLOT));
	CHelmet* helmet = smart_cast<CHelmet*>(Actor()->inventory().ItemFromSlot(HELMET_SLOT));
	CHelmet* helmet2 = smart_cast<CHelmet*>(Actor()->inventory().ItemFromSlot(SECOND_HELMET_SLOT));
	CWeapon* knife = smart_cast<CWeapon*>(Actor()->inventory().ItemFromSlot(KNIFE_SLOT));
	CWeapon* wpn1 = smart_cast<CWeapon*>(Actor()->inventory().ItemFromSlot(PISTOL_SLOT));
	CWeapon* wpn2 = smart_cast<CWeapon*>(Actor()->inventory().ItemFromSlot(RIFLE_SLOT));

	if (outfit || helmet || helmet2 || knife || wpn1 || wpn2)
	{
		if (outfit && outfit->GetCondition() < 0.9f && outfit->GetCondition() >= 0.4f && outfit->IsNecessaryItem(this->cNameSect().c_str(), outfit->m_SuitableRepairKits))
		{
			if (Actor()->HasItemsForRepair(outfit->m_ItemsForRepair))
				return true;
		}
		else if (helmet && helmet->GetCondition() < 0.9f && helmet->GetCondition() >= 0.4f && helmet->IsNecessaryItem(this->cNameSect().c_str(), helmet->m_SuitableRepairKits))
		{
			if (Actor()->HasItemsForRepair(helmet->m_ItemsForRepair))
				return true;
		}
		else if (helmet2 && helmet2->GetCondition() < 0.9f && helmet2->GetCondition() >= 0.4f && helmet2->IsNecessaryItem(this->cNameSect().c_str(), helmet2->m_SuitableRepairKits))
		{
			if (Actor()->HasItemsForRepair(helmet2->m_ItemsForRepair))
				return true;
		}
		else if (knife && knife->GetCondition() < 0.9f && knife->GetCondition() >= 0.4f && knife->IsNecessaryItem(this->cNameSect().c_str(), knife->m_SuitableRepairKits))
		{
			if (Actor()->HasItemsForRepair(knife->m_ItemsForRepair))
				return true;
		}
		else if (wpn1 && wpn1->GetCondition() < 0.9f && wpn1->GetCondition() >= 0.4f && wpn1->IsNecessaryItem(this->cNameSect().c_str(), wpn1->m_SuitableRepairKits))
		{
			if (Actor()->HasItemsForRepair(wpn1->m_ItemsForRepair))
				return true;
		}
		else if (wpn2 && wpn2->GetCondition() < 0.9f && wpn2->GetCondition() >= 0.4f && wpn2->IsNecessaryItem(this->cNameSect().c_str(), wpn2->m_SuitableRepairKits))
		{
			if (Actor()->HasItemsForRepair(wpn2->m_ItemsForRepair))
				return true;
		}
		else
			return false;
	}
	return false;
}

bool CRepairKit::UseBy(CEntityAlive* entity_alive)
{
	if (!inherited::Useful()) return false;

	if (m_iUseFor == 1)
		ChangeInOutfit();
	else if (m_iUseFor == 2)
		ChangeInHelmet();
	else if (m_iUseFor == 3)
		ChangeInSecondHelmet();
	else if (m_iUseFor == 4)
		ChangeInKnife();
	else if (m_iUseFor == 5)
		ChangeInWpn1();
	else if (m_iUseFor == 6)
		ChangeInWpn2();
	else
		return false;

	m_iUseFor = 0;

	return true;
}

void CRepairKit::ChangeInOutfit()
{
	CCustomOutfit* outfit = smart_cast<CCustomOutfit*>(Actor()->inventory().ItemFromSlot(OUTFIT_SLOT));
	float rnd_cond = ::Random.randF(0.1f, m_fRestoreCondition);
	int repair_skill_level_inverted = 5;

	if (Actor()->ActorSkills)
		repair_skill_level_inverted -= Actor()->ActorSkills->repairSkillLevel;
	else
		repair_skill_level_inverted = 1;

	if (repair_skill_level_inverted)
		rnd_cond /= repair_skill_level_inverted;

	if (outfit)
	{
		Actor()->RemoveItemsForRepair(outfit->m_ItemsForRepair);
		outfit->ChangeCondition(rnd_cond);

		if (m_iPortionsNum > 0)
			--m_iPortionsNum;
		else
			m_iPortionsNum = 0;
	}
}

void CRepairKit::ChangeInHelmet()
{
	CHelmet* helmet = smart_cast<CHelmet*>(Actor()->inventory().ItemFromSlot(HELMET_SLOT));
	float rnd_cond = ::Random.randF(0.1f, m_fRestoreCondition);
	int repair_skill_level_inverted = 5;

	if (Actor()->ActorSkills)
		repair_skill_level_inverted -= Actor()->ActorSkills->repairSkillLevel;
	else
		repair_skill_level_inverted = 1;

	if (repair_skill_level_inverted)
		rnd_cond /= repair_skill_level_inverted;

	if (helmet)
	{
		Actor()->RemoveItemsForRepair(helmet->m_ItemsForRepair);
		helmet->ChangeCondition(rnd_cond);

		if (m_iPortionsNum > 0)
			--m_iPortionsNum;
		else
			m_iPortionsNum = 0;
	}
}

void CRepairKit::ChangeInSecondHelmet()
{
	CHelmet* helmet = smart_cast<CHelmet*>(Actor()->inventory().ItemFromSlot(SECOND_HELMET_SLOT));
	float rnd_cond = ::Random.randF(0.1f, m_fRestoreCondition);
	int repair_skill_level_inverted = 5;

	if (Actor()->ActorSkills)
		repair_skill_level_inverted -= Actor()->ActorSkills->repairSkillLevel;
	else
		repair_skill_level_inverted = 1;

	if (repair_skill_level_inverted)
		rnd_cond /= repair_skill_level_inverted;

	if (helmet)
	{
		Actor()->RemoveItemsForRepair(helmet->m_ItemsForRepair);
		helmet->ChangeCondition(rnd_cond);

		if (m_iPortionsNum > 0)
			--m_iPortionsNum;
		else
			m_iPortionsNum = 0;
	}
}

void CRepairKit::ChangeInKnife()
{
	CWeapon* knife = smart_cast<CWeapon*>(Actor()->inventory().ItemFromSlot(KNIFE_SLOT));
	float rnd_cond = ::Random.randF(0.1f, m_fRestoreCondition);
	int repair_skill_level_inverted = 5;

	if (Actor()->ActorSkills)
		repair_skill_level_inverted -= Actor()->ActorSkills->repairSkillLevel;
	else
		repair_skill_level_inverted = 1;

	if (repair_skill_level_inverted)
		rnd_cond /= repair_skill_level_inverted;

	if (knife)
	{
		Actor()->RemoveItemsForRepair(knife->m_ItemsForRepair);
		knife->ChangeCondition(rnd_cond);

		if (m_iPortionsNum > 0)
			--m_iPortionsNum;
		else
			m_iPortionsNum = 0;
	}
}

void CRepairKit::ChangeInWpn1()
{
	CWeapon* wpn = smart_cast<CWeapon*>(Actor()->inventory().ItemFromSlot(PISTOL_SLOT));
	float rnd_cond = ::Random.randF(0.1f, m_fRestoreCondition);
	int repair_skill_level_inverted = 5;

	if (Actor()->ActorSkills)
		repair_skill_level_inverted -= Actor()->ActorSkills->repairSkillLevel;
	else
		repair_skill_level_inverted = 1;

	if (repair_skill_level_inverted)
		rnd_cond /= repair_skill_level_inverted;

	if (wpn)
	{
		Actor()->RemoveItemsForRepair(wpn->m_ItemsForRepair);
		wpn->ChangeCondition(rnd_cond);

		if (m_iPortionsNum > 0)
			--m_iPortionsNum;
		else
			m_iPortionsNum = 0;
	}
}

void CRepairKit::ChangeInWpn2()
{
	CWeapon* wpn = smart_cast<CWeapon*>(Actor()->inventory().ItemFromSlot(RIFLE_SLOT));
	float rnd_cond = ::Random.randF(0.1f, m_fRestoreCondition);
	int repair_skill_level_inverted = 5;

	if (Actor()->ActorSkills)
		repair_skill_level_inverted -= Actor()->ActorSkills->repairSkillLevel;
	else
		repair_skill_level_inverted = 1;

	if (repair_skill_level_inverted)
		rnd_cond /= repair_skill_level_inverted;

	if (wpn)
	{
		Actor()->RemoveItemsForRepair(wpn->m_ItemsForRepair);
		wpn->ChangeCondition(rnd_cond);

		if (m_iPortionsNum > 0)
			--m_iPortionsNum;
		else
			m_iPortionsNum = 0;
	}
}

void CRepairKit::ChangeRepairKitCondition(float val)
{
	m_fRestoreCondition += val;
	clamp(m_fRestoreCondition, 0.f, 1.f);
}

float CRepairKit::GetRepairKitCondition() const
{
	return m_fRestoreCondition;
}