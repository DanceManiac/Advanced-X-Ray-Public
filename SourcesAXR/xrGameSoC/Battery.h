#pragma once

#include "eatable_item_object.h"

class CCustomDetector;
class CTorch;

class CBattery : public CEatableItemObject
{
    using inherited = CEatableItemObject;

public:
							CBattery();
	virtual					~CBattery();

	virtual void			Load(LPCSTR section);
	virtual bool			Useful() const;

	virtual BOOL			net_Spawn(CSE_Abstract* DC);

	virtual	void			UseBy(CEntityAlive* npc);
	virtual	bool			Empty() { return m_iPortionsNum == 0; };
			int				m_iUseFor;
			float			m_fBatteryChargeLevel;
			float			GetCurrentChargeLevel(void) const;
	virtual	void			ChangeChargeLevel(float val);
			void			ChargeTorch(CTorch* flashlight);
			void			ChargeDetector(CCustomDetector* detector);
			//void			ChargeAnomalyDetector();

	virtual CBattery* cast_battery() { return this; }
protected:
			int				m_iPortionsNum;
};