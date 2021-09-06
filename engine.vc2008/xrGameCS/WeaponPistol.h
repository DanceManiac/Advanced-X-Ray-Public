#pragma once
#include "weaponcustompistol.h"

class CWeaponPistol :
	public CWeaponCustomPistol
{
	typedef CWeaponCustomPistol inherited;
public:
					CWeaponPistol	();
	virtual			~CWeaponPistol	();
	
	virtual void	switch2_Reload	();

	virtual void	OnAnimationEnd	(u32 state);
	virtual void	net_Destroy		();
	virtual void	OnH_B_Chield	();

protected:	
	virtual bool	AllowFireWhileWorking() {return true;}

	ESoundTypes			m_eSoundClose;
};
