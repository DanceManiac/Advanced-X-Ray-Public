#pragma once

#include "WeaponMagazined.h"

class CWeaponCustomPistol: public CWeaponMagazined
{
private:
	typedef CWeaponMagazined inherited;
public:
					CWeaponCustomPistol	();
	virtual			~CWeaponCustomPistol();
	virtual	int		GetCurrentFireMode();
protected:
	virtual void	FireEnd				();
	virtual void	switch2_Fire		();
};
