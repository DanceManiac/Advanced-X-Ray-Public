#pragma once

#include "inventory_item_object.h"

// Author: Charsi82

class CBackpack : public CInventoryItemObject
{
private:
    using inherited = CInventoryItemObject;
    float m_additional_weight;

public:
    CBackpack() : m_additional_weight(0.f){};
    virtual ~CBackpack() = default;

    virtual void Load(LPCSTR section);

    float AdditionalInventoryWeight();
};