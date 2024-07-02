////////////////////////////////////////////////////////////////////////////
//	Module 		: Battery.cpp
//	Created 	: 07.04.2021
//  Modified 	: 01.07.2024
//	Author		: Dance Maniac (M.F.S. Team)
//	Description : Torch battery
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Battery.h"
#include "Torch.h"
#include "Actor.h"
#include "inventory.h"
#include "CustomDetector.h"
//#include "AnomalyDetector.h"

CBattery::CBattery()
{
	m_iPortionsNum = -1;
	m_iUseFor = 0;
	m_fBatteryChargeLevel = 1.0f;
	//m_physic_item = 0;
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
	CCustomDetector* detector = nullptr;

	TIItemContainer& it_belt = Actor()->inventory().m_belt;

	for (TIItemContainer::iterator l_it = it_belt.begin(); it_belt.end() != l_it; ++l_it)
	{
		CCustomDetector* detector = smart_cast<CCustomDetector*>(*l_it);
	}

	if (flashlight || detector)
	{
		if (flashlight)
		{
			if (flashlight->GetChargeLevel() <= 0.99f)
			{
				if (flashlight->IsNecessaryItem(this->cNameSect().c_str(), flashlight->m_SuitableBatteries))
					return true; // separated for debug
			}
		}
		if (detector)
		{
			if (detector->GetChargeLevel() <= 0.99f)
			{
				if (detector->IsNecessaryItem(this->cNameSect().c_str(), detector->m_SuitableBatteries))
					return true; // separated for debug
			}
		}
		else
			return false;
	}
	else
		return false;
}

void CBattery::UseBy(CEntityAlive* entity_alive)
{
	CTorch* flashlight = smart_cast<CTorch*>(Actor()->inventory().ItemFromSlot(TORCH_SLOT));
	CCustomDetector* detector = nullptr;

	TIItemContainer& it_belt = Actor()->inventory().m_belt;

	for (TIItemContainer::iterator l_it = it_belt.begin(); it_belt.end() != l_it; ++l_it)
	{
		detector = smart_cast<CCustomDetector*>(*l_it);
	}

	if (m_iUseFor == 0)
	{
		if (flashlight && !detector)
			ChargeTorch(flashlight);
		else if (!flashlight && detector)
			ChargeDetector(detector);
		else if (flashlight && detector)
		{
			float torch_battery = flashlight->GetChargeLevel();
			float art_det_battery = detector->GetChargeLevel();
			if (torch_battery < art_det_battery)
				ChargeTorch(flashlight);
			else
				ChargeDetector(detector);
		}
	}
	else if (m_iUseFor == 1)
		ChargeTorch(flashlight);
	else if (m_iUseFor == 2)
		ChargeDetector(detector);

	m_iUseFor = 0;
}

void CBattery::ChargeTorch(CTorch* flashlight)
{
	if (flashlight)
	{
		flashlight->Recharge(m_fBatteryChargeLevel);

		if (m_iPortionsNum > 0)
			--m_iPortionsNum;
		else
			m_iPortionsNum = 0;
	}

	//Msg("Battery Charge is: %f", m_fBatteryChargeLevel); //Для тестов
}

void CBattery::ChargeDetector(CCustomDetector* detector)
{
	if (detector)
	{
		detector->Recharge(m_fBatteryChargeLevel);

		if (m_iPortionsNum > 0)
			--m_iPortionsNum;
		else
			m_iPortionsNum = 0;
	}

	//Msg("Battery Charge is: %f", m_fBatteryChargeLevel); //Для тестов
}

/*void CBattery::ChargeAnomalyDetector()
{
	CDetectorAnomaly* anomaly_detector = smart_cast<CDetectorAnomaly*>(Actor()->inventory().ItemFromSlot(DOSIMETER_SLOT));

	if (anomaly_detector)
	{
		anomaly_detector->Recharge(m_fBatteryChargeLevel);

		if (m_iPortionsNum > 0)
			--m_iPortionsNum;
		else
			m_iPortionsNum = 0;
	}

	//Msg("Battery Charge is: %f", m_fBatteryChargeLevel); //Для тестов
}  */

float CBattery::GetCurrentChargeLevel() const
{
	return m_fBatteryChargeLevel;
}