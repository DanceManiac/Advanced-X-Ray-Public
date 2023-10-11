// ActorCondition.h: класс состояния игрока
//

#pragma once

#include "EntityCondition.h"
#include "actor_defs.h"

template <typename _return_type>
class CScriptCallbackEx;
class CActor;
class CActorDeathEffector;
//class CPostprocessAnimatorLerp;

class CActorCondition: public CEntityCondition {
private:
	typedef CEntityCondition inherited;
	enum {	eCriticalPowerReached			=(1<<0),
			eCriticalMaxPowerReached		=(1<<1),
			eCriticalBleedingSpeed			=(1<<2),
			eCriticalSatietyReached			=(1<<3),
			eCriticalRadiationReached		=(1<<4),
			eWeaponJammedReached			=(1<<5),
			ePhyHealthMinReached			=(1<<6),
			eCantWalkWeight					=(1<<7),
			eCantWalkWeightReached			=(1<<8),
			eCriticalThirstReached			=(1<<9),
			eCriticalIntoxicationReached	=(1<<10),
			eCriticalSleepenessReached		=(1<<11),
			eCriticalAlcoholismReached		=(1<<12),
			eCriticalHangoverReached		=(1<<13),
			eCriticalNarcotismReached		=(1<<14),
			eCriticalWithdrawalReached		=(1<<15),
			};
	Flags16											m_condition_flags;
private:
	CActor*											m_object;
	CActorDeathEffector*							m_death_effector;
	void				UpdateTutorialThresholds	();
			void 		UpdateSatiety				();
			void 		UpdateThirst				();
			void 		UpdateIntoxication			();
			void 		UpdateSleepeness			();
			void 		UpdateAlcoholism			();
			void 		UpdateNarcotism				();
			void 		UpdatePsyHealth				();
	virtual void		UpdateRadiation				();
public:
						CActorCondition				(CActor *object);
	virtual				~CActorCondition			();

	//CActorDeathEffector*							m_eatable_effector;

	virtual void		LoadCondition				(LPCSTR section);
	virtual void		reinit						();

	virtual CWound*		ConditionHit				(SHit* pHDS);
	virtual void		UpdateCondition				();
			void		UpdateBoosters				();

	virtual void 		ChangeAlcohol				(const float value);
	virtual void 		ChangeSatiety				(const float value);
	virtual void 		ChangeThirst				(const float value);
	virtual void 		ChangeIntoxication			(const float value);
	virtual void 		ChangeSleepeness			(const float value);
	virtual void 		ChangeAlcoholism			(const float value);
	virtual void 		ChangeHangover				(const float value);
	virtual void 		ChangeNarcotism				(const float value);
	virtual void 		ChangeWithdrawal			(const float value);
	virtual void 		ChangeDrugs					(const float value);
	virtual void 		ChangePsyHealth				(const float value);

	void 				BoostParameters				(const SBooster& B, bool need_change_tf = true);
	void 				DisableBoostParameters		(const SBooster& B);
	void				WoundForEach				(const luabind::functor<bool>& funct);
	void				BoosterForEach				(const luabind::functor<bool>& funct);
	bool				ApplyBooster_script			(const SBooster& B, LPCSTR sect);
	void				ClearAllBoosters			();
	void				BoostMaxWeight				(const float value);
	void				BoostHpRestore				(const float value);
	void				BoostPowerRestore			(const float value);
	void				BoostRadiationRestore		(const float value);
	void				BoostBleedingRestore		(const float value);
	void				BoostBurnImmunity			(const float value);
	void				BoostShockImmunity			(const float value);
	void				BoostRadiationImmunity		(const float value);
	void				BoostTelepaticImmunity		(const float value);
	void				BoostChemicalBurnImmunity	(const float value);
	void				BoostExplImmunity			(const float value);
	void				BoostStrikeImmunity			(const float value);
	void				BoostFireWoundImmunity		(const float value);
	void				BoostWoundImmunity			(const float value);
	void				BoostRadiationProtection	(const float value);
	void				BoostTelepaticProtection	(const float value);
	void				BoostChemicalBurnProtection	(const float value);
	void				BoostTimeFactor				(const float value);
	void				BoostSatietyRestore			(const float value);
	void				BoostThirstRestore			(const float value);
	void				BoostPsyHealthRestore		(const float value);
	void				BoostIntoxicationRestore	(const float value);
	void				BoostSleepenessRestore		(const float value);
	void				BoostAlcoholRestore			(const float value);
	void				BoostAlcoholismRestore		(const float value);
	void				BoostHangoverRestore		(const float value);
	void				BoostDrugsRestore			(const float value);
	void				BoostNarcotismRestore		(const float value);
	void				BoostWithdrawalRestore		(const float value);
	BOOSTER_MAP			GetCurBoosterInfluences		() {return m_booster_influences;};

	// хромание при потере сил и здоровья
	virtual	bool		IsLimping					() const;
	virtual bool		IsCantWalk					() const;
	virtual bool		IsCantWalkWeight			();
	virtual bool		IsCantSprint				() const;

			void		PowerHit					(float power, bool apply_outfit);
			float		GetPower					() const { return m_fPower; }

			void		ConditionJump				(float weight);
			void		ConditionWalk				(float weight, bool accel, bool sprint);
			void		ConditionStand				(float weight);
	IC		float		MaxWalkWeight				() const	{ return m_MaxWalkWeight; }
			
			float	xr_stdcall	GetAlcohol			()	{return m_fAlcohol;}
			float	xr_stdcall	GetPsy				()	{return 1.0f-GetPsyHealth();}
	IC		float				GetSatietyPower		() const {return m_fV_SatietyPower*m_fSatiety;};
	IC		float				GetThirstPower		() const { return m_fV_ThirstPower * m_fThirst; };
			float	xr_stdcall	GetIntoxication		() { return m_fIntoxication; }
			float	xr_stdcall	GetSleepeness		() { return m_fSleepeness; }
			float	xr_stdcall	GetAlcoholism		() { return m_fAlcoholism; }
			float	xr_stdcall	GetHangover			() { return m_fHangover; }
			float	xr_stdcall	GetNarcotism		() { return m_fNarcotism; }
			float	xr_stdcall	GetWithdrawal		() { return m_fWithdrawal; }
			float	xr_stdcall	GetDrugs			() { return m_fDrugs; }

			void		AffectDamage_InjuriousMaterialAndMonstersInfluence();
			float		GetInjuriousMaterialDamage	();
			
			void		SetZoneDanger				(float danger, ALife::EInfluenceType type);
			float		GetZoneDanger				() const;

public:
	IC		CActor		&object						() const
	{
		VERIFY			(m_object);
		return			(*m_object);
	}
	virtual void			save					(NET_Packet &output_packet);
	virtual void			load					(IReader &input_packet);
	IC		float const&	V_Satiety				()	{ return m_fV_Satiety; }
	IC		float const&	V_SatietyPower			()	{ return m_fV_SatietyPower; }
	IC		float const&	V_SatietyHealth			()	{ return m_fV_SatietyHealth; }
	IC		float const&	SatietyCritical			()	{ return m_fSatietyCritical; }
	IC		float const&	V_Thirst				() { return m_fV_Thirst; }
	IC		float const&	V_ThirstPower			() { return m_fV_ThirstPower; }
	IC		float const&	V_ThirstHealth			() { return m_fV_ThirstHealth; }
	IC		float const&	ThirstCritical			() { return m_fThirstCritical; }
	IC		float const&	V_Intoxication			() { return m_fV_Intoxication; }
	IC		float const&	V_IntoxicationHealth	() { return m_fV_IntoxicationHealth; }
	IC		float const&	IntoxicationCritical	() { return m_fIntoxicationCritical; }
	IC		float const&	V_Sleepeness			() { return m_fV_Sleepeness; }
	IC		float const&	V_SleepenessPower		() { return m_fV_SleepenessPower; }
	IC		float const&	V_SleepenessPsyHealth	() { return m_fV_SleepenessPsyHealth; }
	IC		float const&	SleepenessCritical		() { return m_fSleepenessCritical; }
	IC		float const&	Sleepeness_V_Sleep		() { return m_fSleepeness_V_Sleep; }
	IC		float const&	V_Alcoholism			() { return m_fV_Alcoholism; }
	IC		float const&	V_Hangover				() { return m_fV_Hangover; }
	IC		float const&	V_Narcotism				() { return m_fV_Narcotism; }
	IC		float const&	V_Withdrawal			() { return m_fV_Withdrawal; }
	
	float	GetZoneMaxPower							(ALife::EInfluenceType type) const;
	float	GetZoneMaxPower							(ALife::EHitType hit_type) const;

	bool	DisableSprint							(SHit* pHDS);
	bool	PlayHitSound							(SHit* pHDS);
	float	HitSlowmo								(SHit* pHDS);
	virtual bool			ApplyInfluence			(const SMedicineInfluenceValues& V, const shared_str& sect, CEatableItem* cur_eatable);
	virtual bool			ApplyBooster			(const SBooster& B, const shared_str& sect);
	float	GetMaxPowerRestoreSpeed					() {return m_max_power_restore_speed;};
	float	GetMaxWoundProtection					() {return m_max_wound_protection;};
	float	GetMaxFireWoundProtection				() {return m_max_fire_wound_protection;};

public:
	SMedicineInfluenceValues						m_curr_medicine_influence;
	float m_fAlcohol;
	float m_fV_Alcohol;
//--
	float m_fV_Satiety;
	float m_fV_SatietyPower;
	float m_fV_SatietyHealth;
	float m_fSatietyCritical;
//--

//--M.F.S. Team
	float m_fV_Thirst;
	float m_fV_ThirstPower;
	float m_fV_ThirstHealth;
	float m_fThirstCritical;

	float m_fV_Intoxication;
	float m_fV_IntoxicationHealth;
	float m_fIntoxicationCritical;

	float m_fV_Sleepeness;
	float m_fV_SleepenessPower;
	float m_fV_SleepenessPsyHealth;
	float m_fSleepenessCritical;
	float m_fSleepeness_V_Sleep;

	float m_fV_Alcoholism;
	float m_fV_Hangover;
	float m_fV_HangoverPower;
	float m_fHangoverCritical;

	float m_fV_Narcotism;
	float m_fV_Withdrawal;
	float m_fV_WithdrawalPower;
	float m_fV_WithdrawalHealth;
	float m_fWithdrawalCritical;
	float m_fDrugs;
	float m_fV_Drugs;

	float m_fV_PsyHealth_Health;
	bool m_bPsyHealthKillActor;

	//Skills System
	float m_fV_SatietySkill;
	float m_fV_HealthSkill;
	float m_fV_BleedingSkill;
	float m_fV_RadiationSkill;
	float m_fV_PowerSkill;
	float m_fV_ThirstSkill;
	float m_fV_IntoxicationSkill;
	float m_fV_SleepenessSkill;
	float m_fMaxWeightSkill;
	float m_fJumpSpeedSkill;
	float m_fWalkAccelSkill;
//--M.F.S. Team

	float m_fPowerLeakSpeed;

	float m_fJumpPower;
	float m_fStandPower;
	float m_fWalkPower;
	float m_fJumpWeightPower;
	float m_fWalkWeightPower;
	float m_fOverweightWalkK;
	float m_fOverweightJumpK;
	float m_fAccelK;
	float m_fSprintK;
	
	float	m_MaxWalkWeight;
	float	m_zone_max_power[ALife::infl_max_count];
	float	m_zone_danger[ALife::infl_max_count];
	float	m_f_time_affected;
	float	m_max_power_restore_speed;
	float	m_max_wound_protection;
	float	m_max_fire_wound_protection;

	mutable bool m_bLimping;
	mutable bool m_bCantWalk;
	mutable bool m_bCantSprint;

	//порог силы и здоровья меньше которого актер начинает хромать
	float m_fLimpingPowerBegin;
	float m_fLimpingPowerEnd;
	float m_fCantWalkPowerBegin;
	float m_fCantWalkPowerEnd;

	float m_fCantSprintPowerBegin;
	float m_fCantSprintPowerEnd;

	float m_fLimpingHealthBegin;
	float m_fLimpingHealthEnd;

	//typedef xr_vector<SMedicineInfluenceValues> BOOSTS_VECTOR;
	//typedef xr_vector<SMedicineInfluenceValues>::iterator BOOSTS_VECTOR_ITER;
	//BOOSTS_VECTOR m_vecBoosts;
	ref_sound m_use_sound;

	DECLARE_SCRIPT_REGISTER_FUNCTION
};

add_to_type_list(CActorCondition)
#undef script_type_list
#define script_type_list save_type_list(CActorCondition)

class CActorDeathEffector
{
	CActorCondition*		m_pParent;
	ref_sound				m_death_sound;
	bool					m_b_actual;
	float					m_start_health;
	void xr_stdcall			OnPPEffectorReleased		();

	/*float						r_min_perc;
	float						r_max_perc;
	float						m_radius;
	float						m_factor;
	CPostprocessAnimatorLerp*	m_pp_effector;
	shared_str					m_pp_fname;*/
public:
			CActorDeathEffector	(CActorCondition* parent, LPCSTR sect);	// -((
			~CActorDeathEffector();
	//void	Load(LPCSTR section);
	void	UpdateCL			();
	IC bool	IsActual			() {return m_b_actual;}
	void	Stop				();
};