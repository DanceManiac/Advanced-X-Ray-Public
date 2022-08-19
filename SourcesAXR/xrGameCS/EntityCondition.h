#pragma once

class CWound;
class NET_Packet;
class CEntityAlive;
class CLevel;

#include "hit_immunity.h"
#include "Hit.h"
#include "Level.h"

class CEntityConditionSimple
{
	float					m_fHealth;
	float					m_fHealthMax;
public:
							CEntityConditionSimple	();
	virtual					~CEntityConditionSimple	();

	IC		 float				GetHealth				() const			{return m_fHealth;}
	IC		 void				SetHealth				( float value ) 	{ m_fHealth = value; }
	IC		 float 				GetMaxHealth			() const			{return m_fHealthMax;}
	IC const float&				health					() const			{return	m_fHealth;}
	IC		 float&				max_health				()					{return	m_fHealthMax;}
};

class CEntityCondition: public CEntityConditionSimple, public CHitImmunity
{
private:
	bool					m_use_limping_state;
	CEntityAlive			*m_object;

public:
							CEntityCondition		(CEntityAlive *object);
	virtual					~CEntityCondition		(void);

	virtual void			LoadCondition			(LPCSTR section);
	virtual void			remove_links			(const CObject *object);

	virtual void			save					(NET_Packet &output_packet);
	virtual void			load					(IReader &input_packet);

	IC float				GetPower				() const			{return m_fPower;}	
	IC float				GetRadiation			() const			{return m_fRadiation;}
	IC float				GetPsyHealth			() const			{return m_fPsyHealth;}

	IC float 				GetEntityMorale			() const			{return m_fEntityMorale;}

	IC float 				GetHealthLost			() const			{return m_fHealthLost;}

	virtual bool 			IsLimping				() const;

	virtual void			ChangeSatiety			(float value)		{};
	void 					ChangeHealth			(float value);
	void 					ChangePower				(float value);
	void 					ChangeRadiation			(float value);
	void 					ChangePsyHealth			(float value);
	virtual void 			ChangeAlcohol			(float value){};
	virtual void 			ChangeThirst			(float value){};
	virtual void			ChangeIntoxication		(const float value){};
	virtual void			ChangeSleepeness		(const float value){};
	virtual void			ChangeAlcoholism		(const float value){};
	virtual void			ChangeHangover			(const float value){};
	virtual void			ChangeNarcotism			(const float value){};
	virtual void			ChangeWithdrawal		(const float value){};
	virtual void 			ChangeDrugs				(float value){};

	IC void					MaxPower				()					{m_fPower = m_fPowerMax;};
	IC void					SetMaxPower				(float val)			{m_fPowerMax = val; clamp(m_fPowerMax,0.1f,1.0f);};
	IC float				GetMaxPower				() const			{return m_fPowerMax;};
	IC float				GetSatiety				() const			{return m_fSatiety;}
	IC float				GetThirst				() const			{return m_fThirst;}
	IC float				GetIntoxication			() const			{return m_fIntoxication;}
	IC float				GetSleepeness			() const			{return m_fSleepeness;}
	IC float				GetAlcoholism			() const			{return m_fAlcoholism;}
	IC float				GetHangover				() const			{return m_fHangover;}
	IC float				GetNarcotism			() const			{return m_fNarcotism;}
	IC float				GetWithdrawal			() const			{return m_fWithdrawal;}

	void 					ChangeBleeding			(float percent);

	void 					ChangeCircumspection	(float value);
	void 					ChangeEntityMorale		(float value);

	virtual CWound*			ConditionHit			(SHit* pHDS);
	//обновления состояния с течением времени
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
	
	void					ClearWounds();
protected:
	void					UpdateHealth			();
	void					UpdatePower				();
	virtual void			UpdateRadiation			();
	void					UpdatePsyHealth			();

	void					UpdateEntityMorale		();


	//изменение силы хита в зависимости от надетого костюма
	//(только для InventoryOwner)
	float					HitOutfitEffect			(float hit_power, ALife::EHitType hit_type, s16 element, float ap, bool& add_wound );
	//изменение потери сил в зависимости от надетого костюма
	float					HitPowerEffect			(float power_loss);
	
	//для подсчета состояния открытых ран,
	//запоминается кость куда был нанесен хит
	//и скорость потери крови из раны
	DEFINE_VECTOR(CWound*, WOUND_VECTOR, WOUND_VECTOR_IT);
	WOUND_VECTOR			m_WoundVector;
	//очистка массива ран
	

	//все величины от 0 до 1			
	float m_fPower;					//сила
	float m_fRadiation;				//доза радиактивного облучения
	float m_fPsyHealth;				//здоровье
	float m_fEntityMorale;			//мораль
	float m_fSatiety;				//голод
	float m_fThirst;				//жажда
	float m_fIntoxication;			//Интоксикация
	float m_fSleepeness;			//Потребность во сне
	float m_fAlcoholism;			//Алкоголизм
	float m_fHangover;				//Похмелье
	float m_fNarcotism;				//Наркомания
	float m_fWithdrawal;			//Ломки

	//максимальные величины
	//	float m_fSatietyMax;
	float m_fPowerMax;
	float m_fRadiationMax;
	float m_fPsyHealthMax;

	float m_fEntityMoraleMax;

	//величины изменения параметров на каждом обновлении
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

	//части хита, затрачиваемые на уменьшение здоровья и силы
	float				m_fHealthHitPart;
	float				m_fPowerHitPart;


	//потеря здоровья от последнего хита
	float				m_fHealthLost;


	//для отслеживания времени 
	u64					m_iLastTimeCalled;
	float				m_fDeltaTime;
	//кто нанес последний хит
	CObject*			m_pWho;
	u16					m_iWhoID;

	//для передачи параметров из DamageManager
	float				m_fHitBoneScale;
	float				m_fWoundBoneScale;

	float				m_limping_threshold;

	bool				m_bTimeValid;
	bool				m_bCanBeHarmed;

public:
	virtual void					reinit				();
	
	IC const	float				fdelta_time			() const 	{return		(m_fDeltaTime);			}
	IC const	WOUND_VECTOR&		wounds				() const	{return		(m_WoundVector);		}
	IC float&						radiation			()			{return		(m_fRadiation);			}
	IC float&						hit_bone_scale		()			{return		(m_fHitBoneScale);		}
	IC float&						wound_bone_scale	()			{return		(m_fWoundBoneScale);	}
	IC SConditionChangeV&			change_v			()			{return		(m_change_v);			}

};
