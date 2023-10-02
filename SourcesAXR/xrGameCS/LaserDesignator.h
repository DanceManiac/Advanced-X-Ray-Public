///////////////////////////////////////////////////////////////
// Scope.h
// Scope - ������� ������ ����������� ������
///////////////////////////////////////////////////////////////

#pragma once

#include "inventory_item_object.h"
#include "script_export_space.h"

class CLaserDesignator : public CInventoryItemObject
{
private:
	typedef CInventoryItemObject inherited;
public:
	CLaserDesignator();
	virtual ~CLaserDesignator();
	DECLARE_SCRIPT_REGISTER_FUNCTION
};
add_to_type_list(CLaserDesignator)
#undef script_type_list
#define script_type_list save_type_list(CLaserDesignator)