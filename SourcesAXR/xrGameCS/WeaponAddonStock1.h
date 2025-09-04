///////////////////////////////////////////////////////////////
// Scope.h
// Scope - апгрейд оружия снайперский прицел
///////////////////////////////////////////////////////////////

#pragma once

#include "inventory_item_object.h"
#include "script_export_space.h"

class CStock : public CInventoryItemObject
{
private:
	typedef CInventoryItemObject inherited;
public:
	CStock();
	virtual ~CStock();
	DECLARE_SCRIPT_REGISTER_FUNCTION
};
add_to_type_list(CStock)
#undef script_type_list
#define script_type_list save_type_list(CStock)