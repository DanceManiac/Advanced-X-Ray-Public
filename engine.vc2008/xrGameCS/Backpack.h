#pragma once

#include "inventory_item_object.h"

// Author: Charsi82

class CBackpack : public CInventoryItemObject
{
public:
    using inherited = CInventoryItemObject;
    float		m_additional_weight;
	float		m_additional_weight2;
	float		m_fPowerRestoreSpeed;
	float		m_fPowerLoss;

	float		m_fJumpSpeed;
	float		m_fWalkAccel;
	float		m_fOverweightWalkK;

    CBackpack() : m_additional_weight(0.f){};
    virtual ~CBackpack() = default;

    virtual void Load(LPCSTR section);

    float AdditionalInventoryWeight();
};