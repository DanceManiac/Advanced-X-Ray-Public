////////////////////////////////////////////////////////////////////////////
//	Module 		: Battery.cpp
//	Created 	: 07.04.2021
//  Modified 	: 19.04.2021
//	Author		: Dance Maniac (M.F.S. Team)
//	Description : Torch battery
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Battery.h"
#include "Torch.h"
#include "Actor.h"
#include "inventory.h"
#include "CustomDetector.h"
#include "AnomalyDetector.h"

CBattery::CBattery()
{
	m_iPortionsNum = -1;
	m_fBatteryChargeLevel = 1.0f;
	m_physic_item = 0;
}

CBattery::~CBattery()
{
}

void CBattery::Load(LPCSTR section)
{
	inherited::Load(section);

	m_iPortionsNum = pSettings->r_s32(section, "eat_portions_num");
	m_fBatteryChargeLevel = READ_IF_EXISTS(pSettings, r_float, section, "charge_level", 1.0f);
	VERIFY(m_iPortionsNum < 10000);
}

BOOL CBattery::net_Spawn(CSE_Abstract* DC)
{
	if (!inherited::net_Spawn(DC)) return FALSE;

	return TRUE;
};

bool CBattery::Useful() const
{
	if (!inherited::Useful()) return false;

	CTorch* flashlight = smart_cast<CTorch*>(Actor()->inventory().ItemFromSlot(TORCH_SLOT));
	CCustomDetector* artifact_detector = smart_cast<CCustomDetector*>(Actor()->inventory().ItemFromSlot(DETECTOR_SLOT));
	CDetectorAnomaly* anomaly_detector = smart_cast<CDetectorAnomaly*>(Actor()->inventory().ItemFromSlot(DOSIMETER_SLOT));

	if (!flashlight || !artifact_detector || !anomaly_detector) return false;

	//��������� �� ��� �� ��� �������
	if (m_iPortionsNum == 0) return false;

	return true;
}

bool CBattery::UseBy(CEntityAlive* entity_alive)
{
	if (!inherited::Useful()) return false;

	if (m_iPortionsNum > 0)
		--m_iPortionsNum;
	else
		m_iPortionsNum = 0;

	return true;
}

void CBattery::ChargeTorch()
{
	CTorch* flashlight = smart_cast<CTorch*>(Actor()->inventory().ItemFromSlot(TORCH_SLOT));

	if (flashlight)
		flashlight->Recharge(m_fBatteryChargeLevel);

	//Msg("Battery Charge is: %f", m_fBatteryChargeLevel); //��� ������
}

void CBattery::ChargeArtifactDetector()
{
	CCustomDetector* artifact_detector = smart_cast<CCustomDetector*>(Actor()->inventory().ItemFromSlot(DETECTOR_SLOT));

	if (artifact_detector)
		artifact_detector->Recharge(m_fBatteryChargeLevel);

	//Msg("Battery Charge is: %f", m_fBatteryChargeLevel); //��� ������
}

void CBattery::ChargeAnomalyDetector()
{
	CDetectorAnomaly* anomaly_detector = smart_cast<CDetectorAnomaly*>(Actor()->inventory().ItemFromSlot(DOSIMETER_SLOT));

	if (anomaly_detector)
		anomaly_detector->Recharge(m_fBatteryChargeLevel);

	//Msg("Battery Charge is: %f", m_fBatteryChargeLevel); //��� ������
}