#include "StdAfx.h"
#include "ActorSkills.h"
#include "object_broker.h"

CActorSkills::CActorSkills()
{
	skills_points = 0;
	skill_cost = 10;
	survivalSkillLevel = 0;
	powerSkillLevel = 0;
	repairSkillLevel = 0;
	enduranceSkillLevel = 0;
}

CActorSkills::~CActorSkills() {}

void CActorSkills::save(NET_Packet& packet)
{
	save_data(skills_points, packet);
	save_data(survivalSkillLevel, packet);
	save_data(powerSkillLevel, packet);
	save_data(repairSkillLevel, packet);
	save_data(enduranceSkillLevel, packet);
}

void CActorSkills::load(IReader& packet)
{
	load_data(skills_points, packet);
	load_data(survivalSkillLevel, packet);
	load_data(powerSkillLevel, packet);
	load_data(repairSkillLevel, packet);
	load_data(enduranceSkillLevel, packet);
}

void CActorSkills::set_skills_points(int num)
{
	skills_points = num;
}

void CActorSkills::inc_skills_points(int num)
{
	skills_points += num;
}

void CActorSkills::dec_skills_points(int num)
{
	skills_points -= num;

	if (skills_points < 0)
		skills_points = 0;
}

int CActorSkills::get_skills_points()
{
	return skills_points;
}

void CActorSkills::set_survival_skill(int num)
{
	survivalSkillLevel = num;
	clamp(survivalSkillLevel, 0, 5);
}

void CActorSkills::inc_survival_skill(int num)
{
	survivalSkillLevel += num;
	clamp(survivalSkillLevel, 0, 5);
}

void CActorSkills::dec_survival_skill(int num)
{
	survivalSkillLevel -= num;
	clamp(survivalSkillLevel, 0, 5);
}

int CActorSkills::get_survival_skill()
{
	return survivalSkillLevel;
}

void CActorSkills::set_power_skill(int num)
{
	powerSkillLevel = num;
	clamp(powerSkillLevel, 0, 5);
}

void CActorSkills::inc_power_skill(int num)
{
	powerSkillLevel += num;
	clamp(powerSkillLevel, 0, 5);
}

void CActorSkills::dec_power_skill(int num)
{
	powerSkillLevel -= num;
	clamp(powerSkillLevel, 0, 5);
}

int CActorSkills::get_power_skill()
{
	return powerSkillLevel;
}

void CActorSkills::set_repair_skill(int num)
{
	repairSkillLevel = num;
	clamp(repairSkillLevel, 0, 5);
}

void CActorSkills::inc_repair_skill(int num)
{
	repairSkillLevel += num;
	clamp(repairSkillLevel, 0, 5);
}

void CActorSkills::dec_repair_skill(int num)
{
	repairSkillLevel -= num;
	clamp(repairSkillLevel, 0, 5);
}

int CActorSkills::get_repair_skill()
{
	return repairSkillLevel;
}

void CActorSkills::set_endurance_skill(int num)
{
	enduranceSkillLevel = num;
	clamp(enduranceSkillLevel, 0, 5);
}

void CActorSkills::inc_endurance_skill(int num)
{
	enduranceSkillLevel += num;
	clamp(enduranceSkillLevel, 0, 5);
}

void CActorSkills::dec_endurance_skill(int num)
{
	enduranceSkillLevel -= num;
	clamp(enduranceSkillLevel, 0, 5);
}

int CActorSkills::get_endurance_skill()
{
	return enduranceSkillLevel;
}

void CActorSkills::BuySkill(int skill)
{
	int current_skill_cost = skill_cost;

	switch (skill)
	{
		case 1:
		{
			current_skill_cost *= (survivalSkillLevel + 1);

			if (skills_points < current_skill_cost)
				return;

			survivalSkillLevel++;
			skills_points -= current_skill_cost;
		} break;
		case 2:
		{
			current_skill_cost *= (powerSkillLevel + 1);

			if (skills_points < current_skill_cost)
				return;

			powerSkillLevel++;
			skills_points -= current_skill_cost;


		} break;
		case 3:
		{
			current_skill_cost *= (repairSkillLevel + 1);

			if (skills_points < current_skill_cost)
				return;

			repairSkillLevel++;
			skills_points -= current_skill_cost;
		} break;
		case 4:
		{
			current_skill_cost *= (enduranceSkillLevel + 1);

			if (skills_points < current_skill_cost)
				return;

			enduranceSkillLevel++;
			skills_points -= current_skill_cost;
		} break;
	}
}