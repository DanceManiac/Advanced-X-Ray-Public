////////////////////////////////////////////////////////////////////////////
//	Module 		: Battery.cpp
//	Created 	: 07.04.2021
//  Modified 	: 12.04.2025
//	Author		: Dance Maniac (M.F.S. Team)
//	Description : Torch battery
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Battery.h"
#include "Torch.h"
#include "Actor.h"
#include "inventory.h"
#include "AnomalyDetector.h"
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

void CBattery::OnH_A_Independent()
{
	if (!inherited::Useful() && this->m_bCanUse)
	{
		if (Local() && OnServer())
			DestroyObject();
	}
}

void CBattery::OnH_B_Independent(bool just_before_destroy)
{
	if (!inherited::Useful())
	{
		setVisible(FALSE);
		setEnabled(FALSE);
	}

	inherited::OnH_B_Independent(just_before_destroy);
}

bool CBattery::CanRechargeDevice() const
{
	CTorch* flashlight = smart_cast<CTorch*>(Actor()->inventory().ItemFromSlot(TORCH_SLOT));
	CDetectorAnomaly* detector = nullptr;

	TIItemContainer& it_belt = Actor()->inventory().m_belt;

	for (TIItemContainer::iterator l_it = it_belt.begin(); it_belt.end() != l_it; ++l_it)
	{
		detector = smart_cast<CDetectorAnomaly*>(*l_it);
	}

	if (!detector)
		detector = smart_cast<CDetectorAnomaly*>(Actor()->inventory().ItemFromSlot(DETECTOR_SLOT));

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

	return false;
}

bool CBattery::Useful() const
{
	if (!inherited::Useful())
		return false;

	//проверить не все ли еще съедено
	if (m_iPortionsNum == 0)
		return false;

	if (!IsItemDropNowFlag())
		return CanRechargeDevice();

	return m_iPortionsNum > 0;
}

bool CBattery::UseBy(CEntityAlive* entity_alive)
{
	if (!inherited::Useful()) return false;

	CTorch* flashlight = smart_cast<CTorch*>(Actor()->inventory().ItemFromSlot(TORCH_SLOT));
	CDetectorAnomaly* anomaly_detector = nullptr;

	TIItemContainer& it_belt = Actor()->inventory().m_belt;

	for (TIItemContainer::iterator l_it = it_belt.begin(); it_belt.end() != l_it; ++l_it)
	{
		anomaly_detector = smart_cast<CDetectorAnomaly*>(*l_it);
	}

	if (!anomaly_detector)
		anomaly_detector = smart_cast<CDetectorAnomaly*>(Actor()->inventory().ItemFromSlot(DETECTOR_SLOT));

	if (m_iUseFor == 0)
	{
		if (flashlight && !anomaly_detector)
			ChargeTorch(flashlight);
		else if (!flashlight && anomaly_detector)
			ChargeAnomalyDetector(anomaly_detector);
		else if (flashlight && anomaly_detector)
		{
			float torch_battery = flashlight->GetChargeLevel();
			float art_det_battery = anomaly_detector->GetChargeLevel();
			if (torch_battery < art_det_battery)
				ChargeTorch(flashlight);
			else
				ChargeAnomalyDetector(anomaly_detector);
		}
	}
	else if (m_iUseFor == 1)
		ChargeTorch(flashlight);
	else if (m_iUseFor == 2)
		ChargeAnomalyDetector(anomaly_detector);

	m_iUseFor = 0;

	return true;
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

/*void CBattery::ChargeDetector(CCustomDetector* detector)
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
}*/

void CBattery::ChargeAnomalyDetector(CDetectorAnomaly* detector)
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

float CBattery::GetCurrentChargeLevel() const
{
	return m_fBatteryChargeLevel;
}

void CBattery::ChangeChargeLevel(float val)
{
	m_fBatteryChargeLevel += val;
	clamp(m_fBatteryChargeLevel, 0.f, 1.f);
}
