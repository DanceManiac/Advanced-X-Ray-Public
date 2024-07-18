///////////////////////////////////////////////////////////////
// Scope.h
// Scope - апгрейд оружия снайперский прицел
///////////////////////////////////////////////////////////////

#pragma once

#include "inventory_item_object.h"
#include "script_export_space.h"

class CTacticalTorch : public CInventoryItemObject
{
private:
	typedef CInventoryItemObject inherited;
public:
	CTacticalTorch();
	virtual ~CTacticalTorch();
	DECLARE_SCRIPT_REGISTER_FUNCTION
};
add_to_type_list(CTacticalTorch)
#undef script_type_list
#define script_type_list save_type_list(CTacticalTorch)