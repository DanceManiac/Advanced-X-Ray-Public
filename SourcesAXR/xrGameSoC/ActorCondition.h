// ActorCondition.h: класс состояния игрока
//

#pragma once

#include "EntityCondition.h"
#include "actor_defs.h"

template <typename _return_type>
class CScriptCallbackEx;



class CActor;
//class CUIActorSleepVideoPlayer;

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
			eCriticalFrostbiteReached		=(1<<15),
			};
	Flags16											m_condition_flags;
private:
	CActor*											m_object;
	void				UpdateTutorialThresholds	();
	void 				UpdateSatiety				();
	void 				UpdateThirst				();
	void 				UpdateIntoxication			();
	void 				UpdateSleepeness			();
	void 				UpdateAlcoholism			();
	void 				UpdateNarcotism				();
	void 				UpdatePsyHealth				();
	void 				UpdateFrostbite				();
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
	virtual void 		ChangePsyHealth				(const float value);
	virtual void 		ChangeFrostbite				(const float value);

	// хромание при потере сил и здоровья
	virtual	bool		IsLimping					() const;
	virtual bool		IsCantWalk					() const;
	virtual bool		IsCantWalkWeight			();
	virtual bool		IsCantSprint				() const;

			void		ConditionJump				(float weight);
			void		ConditionWalk				(float weight, bool accel, bool sprint);
			void		ConditionStand				(float weight);
	IC		float		MaxWalkWeight				() const { return m_MaxWalkWeight; }
			
			float	xr_stdcall	GetAlcohol			()	{return m_fAlcohol;}
			float	xr_stdcall	GetPsy				()	{return 1.0f-GetPsyHealth();}
			float				GetSatiety			()  {return m_fSatiety;}
			  float	xr_stdcall	GetSatietyPower		() const { return m_fV_SatietyPower * m_fSatiety; };
			float	xr_stdcall	GetIntoxication		()	{return m_fIntoxication;}
			float	xr_stdcall	GetSleepeness		()	{return m_fSleepeness;}
			float	xr_stdcall	GetAlcoholism		()	{return m_fAlcoholism;}
			float	xr_stdcall	GetNarcotism		()	{return m_fNarcotism;}
			float	xr_stdcall	GetWithdrawal		()	{return m_fWithdrawal;}
			float	xr_stdcall	GetHangover			()	{return m_fHangover;}
			float	xr_stdcall	GetDrugs			()	{return m_fDrugs;}
			float	xr_stdcall	GetFrostbite		()	{return m_fFrostbite;}

public:
	IC		CActor		&object						() const
	{
		VERIFY			(m_object);
		return			(*m_object);
	}
	virtual void			save					(NET_Packet &output_packet);
	virtual void			load					(IReader &input_packet);

	IC		float const&	V_Thirst				()  { return m_fV_Thirst; }
	IC		float const&	V_ThirstPower			()  { return m_fV_ThirstPower; }
	IC		float const&	V_ThirstHealth			()  { return m_fV_ThirstHealth; }
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
	IC		float const&	V_Frostbite				() { return m_fV_Frostbite; }
	IC		float const&	V_FrostbiteHealth		() { return m_fV_FrostbiteHealth; }
	IC		float const&	FrostbiteCritical		() { return m_fFrostbiteCritical; }

			void			WoundForEach			(const luabind::functor<bool>& funct);

	float m_fAlcohol;
	float m_fV_Alcohol;
//--
	float m_fSatiety;
	float m_fV_Satiety;
	float m_fV_SatietyPower;
	float m_fV_SatietyHealth;
//--

//--M.F.S. Team
	float m_fV_Thirst;
	float m_fV_ThirstPower;
	float m_fV_ThirstHealth;
	float m_fThirstAccelTemp;
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
	float m_fV_Frostbite;
	float m_fV_FrostbiteAdd;
	float m_fFrostbiteIncTemp;
	float m_fFrostbiteDecTemp;
	float m_fV_FrostbiteHealth;
	float m_fFrostbiteCritical;

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
	float m_fV_FrostbiteSkill;
	float m_fV_FrostbiteAddSkill;
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

	DECLARE_SCRIPT_REGISTER_FUNCTION
};

add_to_type_list(CActorCondition)
#undef script_type_list
#define script_type_list save_type_list(CActorCondition)
