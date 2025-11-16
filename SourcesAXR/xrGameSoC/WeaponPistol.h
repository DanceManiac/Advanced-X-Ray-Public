#pragma once
#include "weaponcustompistol.h"

class CWeaponPistol :
	public CWeaponCustomPistol
{
	typedef CWeaponCustomPistol inherited;
public:
	CWeaponPistol	(LPCSTR name);
	virtual ~CWeaponPistol(void);
	
	virtual void	switch2_Reload	();

	virtual void	OnAnimationEnd	(u32 state);
	virtual void	net_Destroy		();
	virtual void	OnH_B_Chield	();

protected:	
	virtual bool	AllowFireWhileWorking() {return true;}

	HUD_SOUND_ITEM		sndClose;
	ESoundTypes			m_eSoundClose;

	bool m_opened;
};
