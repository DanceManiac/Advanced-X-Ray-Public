#pragma once

class CActorSkills
{
public:
			CActorSkills();
	virtual ~CActorSkills();

	enum ESkills
	{
		noSkill = 0,
		survivalSkill,
		powerSkill,
		repairSkill,
		enduranceSkill
	};

	int skills_points;
	int skill_cost;
	int survivalSkillLevel;
	int powerSkillLevel;
	int repairSkillLevel;
	int enduranceSkillLevel;

	void save(NET_Packet& output_packet);
	void load(IReader& input_packet);

	void set_skills_points(int num);
	void inc_skills_points(int num);
	void dec_skills_points(int num);
	int	get_skills_points();

	void set_survival_skill(int num);
	void inc_survival_skill(int num);
	void dec_survival_skill(int num);
	int get_survival_skill();

	void set_power_skill(int num);
	void inc_power_skill(int num);
	void dec_power_skill(int num);
	int get_power_skill();

	void set_repair_skill(int num);
	void inc_repair_skill(int num);
	void dec_repair_skill(int num);
	int get_repair_skill();

	void set_endurance_skill(int num);
	void inc_endurance_skill(int num);
	void dec_endurance_skill(int num);
	int get_endurance_skill();

	void BuySkill(int skill);
};