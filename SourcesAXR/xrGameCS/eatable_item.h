#pragma once

#include "inventory_item.h"

class CPhysicItem;
class CEntityAlive;

class CEatableItem : public CInventoryItem {
private:
	typedef CInventoryItem	inherited;

protected:
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
	virtual void			OnH_A_Independent			();
	virtual	void			UseBy						(CEntityAlive* npc);
	virtual void			save						(NET_Packet &output_packet);
	virtual void			load						(IReader &input_packet);
	virtual	bool			Empty						()						{return m_iPortionsNum==0;};
	virtual	u32				Cost						()	const;
	virtual float			Weight						()	const;
			void SetRemainingUses(u8 value) { if (value > m_iConstPortions) return; m_iPortionsNum = value; };
			u8 GetMaxUses() const { return m_iConstPortions; };

	IC		u32				GetPortionsNum				()	const				{return m_iPortionsNum;}
			void			SetPortionsNum				(u32 value)				{m_iPortionsNum = value;}

			void			UpdateInRuck				();
			void			UpdateUseAnim				();
			void			StartAnimation				();
			void			HideWeapon					();

			bool			m_bHasAnimation;
			bool			m_bActivated;
			bool			m_bItmStartAnim;
			int				m_iAnimHandsCnt;
			int				m_iAnimLength;
			bool			m_bUnlimited;
			float			m_fEffectorIntensity;
			LPCSTR			anim_sect;
			shared_str		use_cam_effector;
			ref_sound		m_using_sound;

	//������� ��� �������� ���� �� ��������� ������
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
	//���������� ��� �� ���-�� ���������
	float					m_fWoundsHealPerc;

	//���������� ������ ���, 
	//-1 - ������ ���� � ������ �� ������ (���� �� �������� ������� � ����)
	u32						m_iConstPortions;
	u32						m_iPortionsNum;
};

