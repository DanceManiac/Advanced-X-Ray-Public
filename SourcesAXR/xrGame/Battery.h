#pragma once

#include "eatable_item_object.h"

class CBattery : public CEatableItemObject
{
    using inherited = CEatableItemObject;

public:
							CBattery();
	virtual					~CBattery();

	virtual void			Load(LPCSTR section);
	virtual bool			Useful() const;

	virtual BOOL			net_Spawn(CSE_Abstract* DC);

	virtual	bool			UseBy(CEntityAlive* npc);
	virtual	bool			Empty() { return PortionsNum() == 0; };
			int				PortionsNum()	const { return m_iPortionsNum; }
			int				m_iUseFor;
			float			m_fBatteryChargeLevel;
			void			ChargeTorch();
			void			ChargeArtifactDetector();
			void			ChargeAnomalyDetector();
protected:
			int				m_iPortionsNum;
};