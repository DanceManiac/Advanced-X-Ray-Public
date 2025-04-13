#pragma once

#include "eatable_item_object.h"

class CCustomDetector;
class CDetectorAnomaly;
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
	virtual void			OnH_B_Independent(bool just_before_destroy);
	virtual void			OnH_A_Independent();

	virtual	bool			UseBy(CEntityAlive* npc);
	virtual	bool			Empty() { return m_iPortionsNum == 0; };
			int				m_iUseFor;
			float			m_fBatteryChargeLevel;
			float			GetCurrentChargeLevel(void) const;
	virtual	void			ChangeChargeLevel(float val);
			void			ChargeTorch(CTorch* flashlight);
			void			ChargeArtifactDetector(CCustomDetector* artifact_detector);
			void			ChargeAnomalyDetector(CDetectorAnomaly* anomaly_detector);
			bool			CanRechargeDevice() const;
protected:
			int				m_iPortionsNum;
};