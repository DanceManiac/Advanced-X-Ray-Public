#pragma once

#include "inventory_item.h"
#include "Actor.h"

class CPhysicItem;
class CEntityAlive;
class CActor;

class CEatableItem : public CInventoryItem {
private:
	typedef CInventoryItem	inherited;

private:
	CPhysicItem		*m_physic_item;

public:
							CEatableItem				();
	virtual					~CEatableItem				();
	virtual	DLL_Pure*		_construct					();
	virtual CEatableItem	*cast_eatable_item			()	{return this;}

	virtual void			Load						(LPCSTR section);
	virtual bool			Useful						() const;

	virtual BOOL			net_Spawn					(CSE_Abstract* DC);

	virtual void			OnH_B_Independent			(bool just_before_destroy);
	virtual	void			UseBy						(CEntityAlive* npc);
	virtual void			save						(NET_Packet &output_packet);
	virtual void			load						(IReader &input_packet);
			bool			Empty						()	const				{return m_iPortionsNum==0;};
	virtual	u32				Cost						()	const;
	virtual float			Weight						()	const;
			void			HitFromActorHit				(SHit* pHDS);

			void			SetRemainingUses			(u32 value) { if (value > m_iConstPortions) return; m_iPortionsNum = value; };
			u32				GetMaxUses					() const { return m_iConstPortions; };

	IC		u32				GetPortionsNum				()	const				{return m_iPortionsNum;}
			void			SetPortionsNum				(u32 value)				{m_iPortionsNum = value;}
	IC		u32				GetConstPortionsNum			()	const				{return m_iConstPortions;}

			void			UpdateInRuck				(CActor* actor);
			void			UpdateUseAnim				(CActor* actor);
			void			StartAnimation				();
			void			HideWeapon					();

			bool			m_bHasAnimation;
			bool			m_bActivated;
			bool			m_bItmStartAnim;
			bool			m_bUnlimited;
			bool			m_bNeedDestroyNotUseful;
			int				m_iAnimHandsCnt;
			int				m_iAnimLength;
			float			m_fEffectorIntensity;
			float			m_fRadioactivity;
			float			m_fIrradiationCoef;
			float			m_fIrradiationZonePower;
			float			m_fSpoliage;
			float			m_fFoodRottingCoef;
			LPCSTR			anim_sect;
			shared_str		use_cam_effector;
			ref_sound		m_using_sound;
protected:	
	//влияние при поедании вещи на параметры игрока
	float					m_fHealthInfluence;
	float					m_fPowerInfluence;
	float					m_fSatietyInfluence;
	float					m_fRadiationInfluence;
	float					m_fMaxPowerUpInfluence;
	float					m_fThirstInfluence;
	float					m_fIntoxicationInfluence;
	float					m_fSleepenessInfluence;
	float					m_fAlcoholismInfluence;
	float					m_fHangoverInfluence;
	float					m_fNarcotismInfluence;
	float					m_fWithdrawalInfluence;
	float					m_fPsyHealthInfluence;
	float					m_fFrostbiteInfluence;
	float					m_alcohol;
	float					m_drugs;
	//заживление ран на кол-во процентов
	float					m_fWoundsHealPerc;

	//количество порций еды, 
	//-1 - порция одна и больше не бывает (чтоб не выводить надпись в меню)
	u32						m_iConstPortions;
	u32						m_iPortionsNum;
};

