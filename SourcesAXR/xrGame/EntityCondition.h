#pragma once

class CWound;
class NET_Packet;
class CEntityAlive;
class CLevel;

#include "hit_immunity.h"
#include "Hit.h"
#include "Level.h"
enum EBoostParams{
	eBoostHpRestore = 0,
	eBoostPowerRestore,
	eBoostRadiationRestore,
	eBoostBleedingRestore,
	eBoostMaxWeight,
	eBoostRadiationProtection,
	eBoostTelepaticProtection,
	eBoostChemicalBurnProtection,
	eBoostBurnImmunity,
	eBoostShockImmunity,
	eBoostRadiationImmunity,
	eBoostTelepaticImmunity,
	eBoostChemicalBurnImmunity,
	eBoostExplImmunity,
	eBoostStrikeImmunity,
	eBoostFireWoundImmunity,
	eBoostWoundImmunity,
	eBoostSatiety,
	eBoostTimeFactor,
	eBoostMaxCount,
};

static const LPCSTR ef_boosters_section_names[] =
{
	"boost_health_restore",
	"boost_power_restore",
	"boost_radiation_restore",
	"boost_bleeding_restore",
	"boost_max_weight",
	"boost_radiation_protection",
	"boost_telepat_protection",
	"boost_chemburn_protection",
	/*"boost_burn_immunity",
	"boost_shock_immunity",
	"boost_radiation_immunity",
	"boost_telepat_immunity",
	"boost_chemburn_immunity",
	"boost_explosion_immunity",
	"boost_strike_immunity",
	"boost_fire_wound_immunity",
	"boost_wound_immunity",*/
	"boost_satiety",

	//M.F.S team additions
	"boost_battery",
	"boost_thirst",
	"boost_psy_health",
	"boost_intoxication",
	"boost_sleepeness",

	//HoP
	"boost_alcoholism",
	"boost_hangover",
	"boost_narcotism",
	"boost_withdrawal",

	"boost_filter_condition",
	"boost_time_factor"
};

struct SBooster{
	float fBoostTime;
	float fBoostValue;
	EBoostParams m_type;
	SBooster():fBoostTime(-1.0f){};
	void Load(const shared_str& sect, EBoostParams type);
};

struct SMedicineInfluenceValues{
	float fHealth;
	float fPower;
	float fSatiety;
	float fThirst;
	float fIntoxication;
	float fSleepeness;
	float fAlcoholism;
	float fHangover;
	float fNarcotism;
	float fWithdrawal;
	float fRadiation;
	float fPsyHealth;
	float fWoundsHeal;
	float fMaxPowerUp;
	float fAlcohol;
	float fDrugs;
	float fTimeTotal;
	float fTimeCurrent;

	SMedicineInfluenceValues():fTimeCurrent(-1.0f){}
	bool InProcess (){return fTimeCurrent>0.0f;}
	void Load(const shared_str& sect);
};

class CEntityConditionSimple
{
	float					m_fHealth;
	float					m_fHealthMax;
public:
							CEntityConditionSimple	();
	virtual					~CEntityConditionSimple	();

	IC		 float				GetHealth				() const				{return m_fHealth;}
	IC		 void				SetHealth				( const float value ) 	{ m_fHealth = value; }
	IC		 float 				GetMaxHealth			() const				{return m_fHealthMax;}
	IC const float&				health					() const				{return	m_fHealth;}
	IC		 float&				max_health				()						{return	m_fHealthMax;}
};

class CEntityCondition: public CEntityConditionSimple, public CHitImmunity
{
private:
	bool					m_use_limping_state;
	CEntityAlive			*m_object;

public:
							CEntityCondition		(CEntityAlive *object);
	virtual					~CEntityCondition		();

	virtual void			LoadCondition			(LPCSTR section);
	virtual void			LoadTwoHitsDeathParams	(LPCSTR section);
	virtual void			remove_links			(const CObject *object);

	virtual void			save					(NET_Packet &output_packet);
	virtual void			load					(IReader &input_packet);

	IC float				GetPower				() const			{return m_fPower;}	
	IC void					SetPower				(float value)		{ m_fPower = value; clamp(m_fPower, 0.f, m_fPowerMax); }
	IC float				GetRadiation			() const			{return m_fRadiation;}
	IC float				GetPsyHealth			() const			{return m_fPsyHealth;}
	IC float				GetSatiety				() const			{return m_fSatiety;}
	IC float				GetThirst				() const			{return m_fThirst;}
	IC float				GetIntoxication			() const			{return m_fIntoxication;}
	IC float				GetSleepeness			() const			{return m_fSleepeness;}
	IC float				GetAlcoholism			() const			{return m_fAlcoholism;}
	IC float				GetAlcohol				() const			{return m_fAlcohol;}
	IC float				GetHangover				() const			{return m_fHangover;}
	IC float				GetNarcotism			() const			{return m_fNarcotism;}
	IC float				GetWithdrawal			() const			{return m_fWithdrawal;}

	IC float 				GetEntityMorale			() const			{return m_fEntityMorale;}

	IC float 				GetHealthLost			() const			{return m_fHealthLost;}

	virtual bool 			IsLimping				() const;

	virtual void			ChangeSatiety			(const float value)		{};
	virtual void			ChangeThirst			(const float value)		{};
	virtual void			ChangeIntoxication		(const float value)		{};
	virtual void			ChangeSleepeness		(const float value)		{};
	virtual void			ChangeAlcoholism		(const float value)		{};
	virtual void			ChangeHangover			(const float value)		{};
	virtual void			ChangeNarcotism			(const float value)		{};
	virtual void			ChangeWithdrawal		(const float value)		{};
	virtual void 			ChangeDrugs				(const float value)		{};
	void 					ChangeHealth			(const float value);
	void 					ChangePower				(const float value);
	void 					ChangeRadiation			(const float value);
	void 					ChangePsyHealth			(const float value);
	virtual void 			ChangeAlcohol			(const float value){};

	IC void					MaxPower				()					{m_fPower = m_fPowerMax;};
	IC void					SetMaxPower				(const float val)	{m_fPowerMax = val; clamp(m_fPowerMax,0.1f,1.0f);};
	IC float				GetMaxPower				() const			{return m_fPowerMax;};

	void 					ChangeBleeding			(const float percent);

	void 					ChangeCircumspection	(const float value);
	void 					ChangeEntityMorale		(const float value);

	virtual CWound*			ConditionHit			(SHit* pHDS);
	//обновлени€ состо€ни€ с течением времени
	virtual void			UpdateCondition			();
	void					UpdateWounds			();
	void					UpdateConditionTime		();
	IC void					SetConditionDeltaTime	(float DeltaTime) { m_fDeltaTime = DeltaTime; };

	
	//скорость потери крови из всех открытых ран 
	float					BleedingSpeed			();

	CObject*				GetWhoHitLastTime		() {return m_pWho;}
	u16						GetWhoHitLastTimeID		() {return m_iWhoID;}

	CWound*					AddWound				(float hit_power, ALife::EHitType hit_type, u16 element);

	IC void 				SetCanBeHarmedState		(bool CanBeHarmed) 			{m_bCanBeHarmed = CanBeHarmed;}
	IC bool					CanBeHarmed				() const					{return OnServer() && m_bCanBeHarmed;};
	virtual bool			ApplyInfluence			(const SMedicineInfluenceValues& V, const shared_str& sect);
	virtual bool			ApplyBooster			(const SBooster& B, const shared_str& sect);
	void					ClearWounds();

	IC float				GetBoostRadiationImmunity() const {return m_fBoostRadiationImmunity;};

	typedef					xr_map<EBoostParams, SBooster> BOOSTER_MAP;
protected:
	void					UpdateHealth			();
	void					UpdatePower				();
	virtual void			UpdateRadiation			();
	void					UpdatePsyHealth			();

	void					UpdateEntityMorale		();


	//изменение силы хита в зависимости от надетого костюма
	//(только дл€ InventoryOwner)
	float					HitOutfitEffect			(float hit_power, ALife::EHitType hit_type, s16 element, float ap, bool& add_wound );
	//изменение потери сил в зависимости от надетого костюма
	float					HitPowerEffect			(float power_loss);
	
	//дл€ подсчета состо€ни€ открытых ран,
	//запоминаетс€ кость куда был нанесен хит
	//и скорость потери крови из раны
	DEFINE_VECTOR(CWound*, WOUND_VECTOR, WOUND_VECTOR_IT);
	WOUND_VECTOR			m_WoundVector;
	//очистка массива ран
	

	//все величины от 0 до 1			
	float m_fPower;					//сила
	float m_fRadiation;				//доза радиактивного облучени€
	float m_fPsyHealth;				//здоровье
	float m_fEntityMorale;			//мораль
	float m_fSatiety;				//—ытость
	float m_fThirst;				//∆ажда
	float m_fIntoxication;			//»нтоксикаци€
	float m_fSleepeness;			//—онливость
	float m_fAlcoholism;			//јлкоголизм
	float m_fAlcohol;				//јлкоголь
	float m_fHangover;				//ѕохмелье
	float m_fNarcotism;				//Ќаркомани€
	float m_fWithdrawal;			//Ћомка

	//максимальные величины
	//	float m_fSatietyMax;
	float m_fPowerMax;
	float m_fRadiationMax;
	float m_fPsyHealthMax;

	float m_fEntityMoraleMax;

	//величины изменени€ параметров на каждом обновлении
	float m_fDeltaHealth;
	float m_fDeltaPower;
	float m_fDeltaRadiation;
	float m_fDeltaPsyHealth;

	float m_fDeltaCircumspection;
	float m_fDeltaEntityMorale;

	struct SConditionChangeV
	{
		float			m_fV_Radiation;
		float			m_fV_PsyHealth;
		float			m_fV_Circumspection;
		float			m_fV_EntityMorale;
		float			m_fV_RadiationHealth;
		float			m_fV_Bleeding;
		float			m_fV_WoundIncarnation;
		float			m_fV_HealthRestore;
		void			load(LPCSTR sect, LPCSTR prefix);
	};
	
	SConditionChangeV m_change_v;

	float				m_fMinWoundSize;
	bool				m_bIsBleeding;		//есть кровотечение

	//части хита, затрачиваемые на уменьшение здоровь€ и силы
	float				m_fHealthHitPart;
	float				m_fPowerHitPart;

	float				m_fBoostBurnImmunity;
	float				m_fBoostShockImmunity;
	float				m_fBoostRadiationImmunity;
	float 				m_fBoostTelepaticImmunity;
	float 				m_fBoostChemicalBurnImmunity;
	float 				m_fBoostExplImmunity;
	float 				m_fBoostStrikeImmunity;
	float 				m_fBoostFireWoundImmunity;
	float 				m_fBoostWoundImmunity;
	float 				m_fBoostRadiationProtection;
	float 				m_fBoostTelepaticProtection;
	float 				m_fBoostChemicalBurnProtection;
	float 				m_fBoostTimeFactor;

	//потер€ здоровь€ от последнего хита
	float				m_fHealthLost;

	float				m_fKillHitTreshold;
	float				m_fLastChanceHealth;
	float				m_fInvulnerableTime;
	float				m_fInvulnerableTimeDelta;
	//дл€ отслеживани€ времени 
	u64					m_iLastTimeCalled;
	float				m_fDeltaTime;
	//кто нанес последний хит
	CObject*			m_pWho;
	u16					m_iWhoID;

	//дл€ передачи параметров из DamageManager
	float				m_fHitBoneScale;
	float				m_fWoundBoneScale;

	float				m_limping_threshold;

	bool				m_bTimeValid;
	bool				m_bCanBeHarmed;
	BOOSTER_MAP			m_booster_influences;

public:
	virtual void					reinit				();
	
	IC const	float				fdelta_time			() const 	{return		(m_fDeltaTime);			}
	IC const	WOUND_VECTOR&		wounds				() const	{return		(m_WoundVector);		}
	IC float&						radiation			()			{return		(m_fRadiation);			}
	IC float&						hit_bone_scale		()			{return		(m_fHitBoneScale);		}
	IC float&						wound_bone_scale	()			{return		(m_fWoundBoneScale);	}
	IC SConditionChangeV&			change_v			()			{return		(m_change_v);			}

	DECLARE_SCRIPT_REGISTER_FUNCTION
};

add_to_type_list(CEntityCondition)
#undef script_type_list
#define script_type_list save_type_list(CEntityCondition)
