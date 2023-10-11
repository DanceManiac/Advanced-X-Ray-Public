#pragma once

#include "inventory_item.h"

class CPhysicItem;
class CEntityAlive;
class CActor;

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
	virtual void			save						(NET_Packet &output_packet);
	virtual void			load						(IReader &input_packet);
			void			UpdateInRuck				(CActor* actor);
			void			UpdateUseAnim				(CActor* actor);
			void			HideWeapon					();
			void			StartAnimation				();
			void SetRemainingUses(u8 value) { if (value > m_iConstPortions) return; m_iPortionsNum = value; };
			u8 GetMaxUses() const { return m_iConstPortions; };
	virtual	bool			UseBy						(CEntityAlive* npc);
	virtual	bool			Empty						()						{return m_iPortionsNum==0;};
	virtual	u32				Cost						()	const;
	virtual float			Weight						()	const;

	IC		u32				GetPortionsNum				()	const				{return m_iPortionsNum;}
			u32				m_iConstPortions;
			u32				m_iPortionsNum;
			bool			m_bHasAnimation;
			bool			m_bUnlimited;
			bool			m_bActivated;
			bool			m_bItmStartAnim;
			bool			m_bNeedDestroyNotUseful;
			int				m_iAnimHandsCnt;
			int				m_iAnimLength;
			float			m_fEffectorIntensity;
			float			m_fRadioactivity;
			float			m_fIrradiationCoef;
			float			m_fIrradiationZonePower;
			LPCSTR			anim_sect;
			shared_str		use_cam_effector;
			ref_sound		m_using_sound;
};

