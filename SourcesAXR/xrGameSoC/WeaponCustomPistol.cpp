#include "stdafx.h"

#include "Entity.h"
#include "WeaponCustomPistol.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CWeaponCustomPistol::CWeaponCustomPistol(LPCSTR name) : CWeaponMagazined(name,SOUND_TYPE_WEAPON_PISTOL)
{
}

CWeaponCustomPistol::~CWeaponCustomPistol()
{
}

void CWeaponCustomPistol::switch2_Fire	()
{
	if (GetCurrentFireMode() == 1)
	{
		m_bFireSingleShot = true;
		bWorking = false;
		m_iShotNum = 0;
		m_bStopedAfterQueueFired = false;
	}
	else
	{
		inherited::switch2_Fire();
	}
}

void CWeaponCustomPistol::FireEnd() 
{
	if (fTime <= 0 && GetCurrentFireMode() == 1)
	{
		SetPending(FALSE);
		inherited::FireEnd();
	}
	else
	{
		inherited::FireEnd();
	}
}

int	CWeaponCustomPistol::GetCurrentFireMode()
{
	return m_bHasDifferentFireModes ? m_aFireModes[m_iCurFireMode] : 1;
};