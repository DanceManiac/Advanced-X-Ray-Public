#include "StdAfx.h"
#include "Backpack.h"

void CBackpack::Load(LPCSTR section)
{
    inherited::Load(section);
    m_additional_weight = pSettings->r_float(section, "additional_inventory_weight");
}

float CBackpack::AdditionalInventoryWeight() 
{
    return m_additional_weight;
}