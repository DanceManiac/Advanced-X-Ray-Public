////////////////////////////////////////////////////////////////////////////
//	Module 		: Battery.cpp
//	Created 	: 07.04.2021
//  Modified 	: 20.04.2021
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
	m_iUseFor = 0;
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

	//проверить не все ли еще съедено
	if (m_iPortionsNum == 0) return false;

	CTorch* flashlight = smart_cast<CTorch*>(Actor()->inventory().ItemFromSlot(TORCH_SLOT));
	CCustomDetector* artifact_detector = smart_cast<CCustomDetector*>(Actor()->inventory().ItemFromSlot(DETECTOR_SLOT));
	CDetectorAnomaly* anomaly_detector = smart_cast<CDetectorAnomaly*>(Actor()->inventory().ItemFromSlot(DOSIMETER_SLOT));

	if (flashlight || artifact_detector || anomaly_detector)
	{
		if (flashlight && flashlight->m_fCurrentChargeLevel <= 0.99f && flashlight->IsNecessaryItem(this->cNameSect().c_str(), flashlight->m_SuitableBatteries))
			return true;
		else if (artifact_detector && artifact_detector->m_fCurrentChargeLevel <= 0.99f && artifact_detector->IsNecessaryItem(this->cNameSect().c_str(), artifact_detector->m_SuitableBatteries))
			return true;
		else if (anomaly_detector && anomaly_detector->m_fCurrentChargeLevel <= 0.99f && anomaly_detector->IsNecessaryItem(this->cNameSect().c_str(), anomaly_detector->m_SuitableBatteries))
			return true;
		else
			return false;
	}
	else
		return false;
}

void CBattery::UseBy(CEntityAlive* entity_alive)
{
	CTorch* flashlight = smart_cast<CTorch*>(Actor()->inventory().ItemFromSlot(TORCH_SLOT));
	CCustomDetector* artifact_detector = smart_cast<CCustomDetector*>(Actor()->inventory().ItemFromSlot(DETECTOR_SLOT));
	CDetectorAnomaly* anomaly_detector = smart_cast<CDetectorAnomaly*>(Actor()->inventory().ItemFromSlot(DOSIMETER_SLOT));

	if (m_iUseFor == 0)
	{
		if (flashlight && !artifact_detector && !anomaly_detector)
			ChargeTorch();
		else if (!flashlight && artifact_detector && !anomaly_detector)
			ChargeArtifactDetector();
		else if (!flashlight && !artifact_detector && anomaly_detector)
			ChargeAnomalyDetector();
		else if (flashlight && artifact_detector)
		{
			float torch_battery = flashlight->m_fCurrentChargeLevel;
			float art_det_battery = artifact_detector->m_fCurrentChargeLevel;
			if (torch_battery < art_det_battery)
				ChargeTorch();
			else
				ChargeArtifactDetector();
		}
	}
	else if (m_iUseFor == 1)
		ChargeTorch();
	else if (m_iUseFor == 2)
		ChargeArtifactDetector();
	else
		ChargeAnomalyDetector();

	//уменьшить количество порций
	if (m_iPortionsNum > 0)
		--(m_iPortionsNum);
	else
		m_iPortionsNum = 0;

	m_iUseFor = 0;
}

void CBattery::ChargeTorch()
{
	CTorch* flashlight = smart_cast<CTorch*>(Actor()->inventory().ItemFromSlot(TORCH_SLOT));

	if (flashlight)
		flashlight->Recharge(m_fBatteryChargeLevel);

	//Msg("Battery Charge is: %f", m_fBatteryChargeLevel); //Для тестов
}

void CBattery::ChargeArtifactDetector()
{
	CCustomDetector* artifact_detector = smart_cast<CCustomDetector*>(Actor()->inventory().ItemFromSlot(DETECTOR_SLOT));

	if (artifact_detector)
		artifact_detector->Recharge(m_fBatteryChargeLevel);

	//Msg("Battery Charge is: %f", m_fBatteryChargeLevel); //Для тестов
}

void CBattery::ChargeAnomalyDetector()
{
	CDetectorAnomaly* anomaly_detector = smart_cast<CDetectorAnomaly*>(Actor()->inventory().ItemFromSlot(DOSIMETER_SLOT));

	if (anomaly_detector)
		anomaly_detector->Recharge(m_fBatteryChargeLevel);

	//Msg("Battery Charge is: %f", m_fBatteryChargeLevel); //Для тестов
}

float CBattery::GetCurrentChargeLevel() const
{
	return m_fBatteryChargeLevel;
}