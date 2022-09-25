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
	virtual void			save						(NET_Packet &output_packet);
	virtual void			load						(IReader &input_packet);
			void			UpdateInRuck				();
			void			UpdateUseAnim				();
	virtual	bool			UseBy						(CEntityAlive* npc);
	virtual	bool			Empty						()						{return PortionsNum()==0;};
			int				PortionsNum					()	const				{return m_iPortionsNum;}

			void			StartAnimation				();
			void			HideWeapon					();

	IC		u32				GetPortionsNum				()	const				{return m_iPortionsNum;}
			void			SetPortionsNum				(u32 value)				{m_iPortionsNum = value;}
			u32				m_iPortionsNum;
			bool			m_bHasAnimation;
			bool			m_bActivated;
			bool			m_bTimerEnd;
			bool			ItmStartAnim;
			bool			m_bUnlimited;
			float			m_fEffectorIntensity;
			int				m_iTiming;
			u32				UseTimer;
			LPCSTR			anim_sect;
			shared_str		use_cam_effector;
			u16				last_slot_id;
			ref_sound		m_using_sound;
};

