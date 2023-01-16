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
			void			HideWeapon					();
			void			StartAnimation				();
	virtual	bool			UseBy						(CEntityAlive* npc);
	virtual	bool			Empty						()						{return PortionsNum()==0;};
	virtual	u32				Cost						()	const;
	virtual float			Weight						()	const;
			int				PortionsNum					()	const				{return m_iPortionsNum;}

	IC		u32				GetPortionsNum				()	const				{return m_iPortionsNum;}
			void			SetPortionsNum				(u32 value)				{m_iPortionsNum = value;}
			u32				m_iConstPortions;
			u32				m_iPortionsNum;
			bool			m_bHasAnimation;
			bool			m_bUnlimited;
			bool			m_bActivated;
			bool			m_bItmStartAnim;
			int				m_iAnimHandsCnt;
			int				m_iAnimLength;
			float			m_fEffectorIntensity;
			LPCSTR			anim_sect;
			shared_str		use_cam_effector;
			ref_sound		m_using_sound;
};

