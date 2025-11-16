#pragma once
#include "missile.h"
#include "../xrphysics/DamageSource.h"
class CBolt :
					public CMissile,
					public IDamageSource
{
	typedef CMissile inherited;
	u16			m_thrower_id;
public:
				CBolt				();
	virtual		~CBolt				();

	virtual void Load				(LPCSTR section);
	virtual void OnH_A_Chield		();
	
	virtual	void SetInitiator		(u16 id);
	virtual	u16	 Initiator			();

	virtual void Throw				();
	virtual bool Action				(u16 cmd, u32 flags);
	virtual bool Useful				() const;
    virtual void activate_physic_shell	();

	void		 PutNextToSlot		();
	virtual void State				(u32 state);
	virtual void OnAnimationEnd		(u32 state);

	virtual BOOL UsedAI_Locations	() {return FALSE;}
	virtual IDamageSource*	cast_IDamageSource			()	{return this;}

	virtual bool GetBriefInfo		(II_BriefInfo& info);
};
