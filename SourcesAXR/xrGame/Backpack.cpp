#include "StdAfx.h"
#include "Backpack.h"

void CBackpack::Load(LPCSTR section)
{
    inherited::Load(section);
    m_additional_weight = pSettings->r_float(section, "additional_inventory_weight");
	m_additional_weight2 = pSettings->r_float(section, "additional_inventory_weight2");
	m_fJumpSpeed = READ_IF_EXISTS(pSettings, r_float, section, "jump_speed", 1.f);
	m_fWalkAccel = READ_IF_EXISTS(pSettings, r_float, section, "walk_accel", 1.f);
	m_fOverweightWalkK = READ_IF_EXISTS(pSettings, r_float, section, "overweight_walk_accel", 1.f);
	m_fPowerRestoreSpeed = READ_IF_EXISTS(pSettings, r_float, section, "power_restore_speed", 0.0f);
	m_fPowerLoss = READ_IF_EXISTS(pSettings, r_float, section, "power_loss", 1.0f);
	clamp(m_fPowerLoss, EPS, 1.0f);
}

float CBackpack::AdditionalInventoryWeight() 
{
    return m_additional_weight;
}