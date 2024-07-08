#pragma once

#include "inventory_item_object.h"

class CSleepingBag : public CInventoryItemObject
{
	typedef CInventoryItemObject inherited;

public:
	CSleepingBag();
	~CSleepingBag();

	void StartSleep();
};