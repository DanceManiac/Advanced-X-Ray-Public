// ActorCondition.h: класс состояния игрока
//

#pragma once

#include "EntityCondition.h"
#include "actor_defs.h"

template <typename _return_type>
class CScriptCallbackEx;
class CActor;
class CActorDeathEffector;

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
			eCriticalThirstReached			=(1<<8),
			eCriticalIntoxicationReached	=(1<<9),
			eCriticalSleepenessReached		=(1<<10),
			eCriticalAlcoholismReached		=(1<<11),
			eCriticalHangoverReached		=(1<<12),
			eCriticalNarcotismReached		=(1<<13),
			eCriticalWithdrawalReached		=(1<<14),
			};
	Flags16											m_condition_flags;
private:
	CActor*											m_object;
	CActorDeathEffector*							m_death_effector;
	void				UpdateTutorialThresholds	();
			void 		UpdateSatiety				();
			void 		UpdateThirst				();
			void 		UpdateSleepeness			();
			void 		UpdateIntoxication			();
			void 		UpdateAlcoholism			();
			void 		UpdateNarcotism				();
	virtual void		UpdateRadiation				();
public:
						CActorCondition				(CActor *object);
	virtual				~CActorCondition			(void);

	virtual void		LoadCondition				(LPCSTR section);
	virtual void		reinit						();

	virtual CWound*		ConditionHit				(SHit* pHDS);
	virtual void		UpdateCondition				();

	virtual void 		ChangeAlcohol				(float value);
	virtual void 		ChangeSatiety				(float value);
	virtual void 		ChangeThirst				(float value);
	virtual void 		ChangeIntoxication			(const float value);
	virtual void 		ChangeSleepeness			(const float value);
	virtual void 		ChangeAlcoholism			(const float value);
	virtual void 		ChangeHangover				(const float value);
	virtual void 		ChangeNarcotism				(const float value);
	virtual void 		ChangeWithdrawal			(const float value);
	virtual void 		ChangeDrugs					(const float value);

	// хромание при потере сил и здоровья
	virtual	bool		IsLimping					() const;
	virtual bool		IsCantWalk					() const;
	virtual bool		IsCantWalkWeight			();
	virtual bool		IsCantSprint				() const;

			void		ConditionJump				(float weight);
			void		ConditionWalk				(float weight, bool accel, bool sprint);
			void		ConditionStand				(float weight);
	IC		float		MaxWalkWeight				() const	{ return m_MaxWalkWeight; }
			
			float	xr_stdcall	GetAlcohol			()	{return m_fAlcohol;}
			float	xr_stdcall	GetPsy				()	{return 1.0f-GetPsyHealth();}
			float	xr_stdcall	GetIntoxication		()	{return m_fIntoxication;}
			float	xr_stdcall	GetSleepeness		()	{return m_fSleepeness;}
			float	xr_stdcall	GetAlcoholism		()	{return m_fAlcoholism;}
			float	xr_stdcall	GetNarcotism		()	{return m_fNarcotism;}
			float	xr_stdcall	GetWithdrawal		()	{return m_fWithdrawal;}
			float	xr_stdcall	GetHangover			()	{return m_fHangover;}
			float	xr_stdcall	GetDrugs			()	{return m_fDrugs;}

			void		AffectDamage_InjuriousMaterial();
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
	IC		float const&	V_Thirst				()  { return m_fV_Thirst; }
	IC		float const&	V_ThirstPower			()  { return m_fV_ThirstPower; }
	IC		float const&	V_ThirstHealth			()  { return m_fV_ThirstHealth; }
	IC		float const&	V_Intoxication			() { return m_fV_Intoxication; }
	IC		float const&	V_IntoxicationHealth	() { return m_fV_IntoxicationHealth; }
	IC		float const&	IntoxicationCritical	() { return m_fIntoxicationCritical; }
	IC		float const&	V_Sleepeness			() { return m_fV_Sleepeness; }
	IC		float const&	V_SleepenessPower		() { return m_fV_SleepenessPower; }
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

public:
	float m_fAlcohol;
	float m_fV_Alcohol;
//--
	float m_fV_Satiety;
	float m_fV_SatietyPower;
	float m_fV_SatietyHealth;
//--

//--M.F.S. Team
	float m_fV_Thirst;
	float m_fV_ThirstPower;
	float m_fV_ThirstHealth;
	float m_fV_Intoxication;
	float m_fV_IntoxicationHealth;
	float m_fIntoxicationCritical;
	float m_fV_Sleepeness;
	float m_fV_SleepenessPower;
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
};

class CActorDeathEffector
{
	CActorCondition*		m_pParent;
	ref_sound				m_death_sound;
	bool					m_b_actual;
	float					m_start_health;
	void 		xr_stdcall	OnPPEffectorReleased		();
public:
			CActorDeathEffector	(CActorCondition* parent, LPCSTR sect);	// -((
			~CActorDeathEffector();
	void	UpdateCL			();
	IC bool	IsActual			() {return m_b_actual;}
	void	Stop				();
};