///////////////////////////////////////////////////////////////
// MilitaryOutfit.h
// MilitaryOutfit - защитный костюм военного
///////////////////////////////////////////////////////////////


#pragma once

#include "customoutfit.h"
#include "script_export_space.h"

class CMilitaryOutfit: public CCustomOutfit
{
private:
	typedef	CCustomOutfit inherited;
public:
	CMilitaryOutfit(void);
	virtual ~CMilitaryOutfit(void);

	DECLARE_SCRIPT_REGISTER_FUNCTION
};
add_to_type_list(CMilitaryOutfit)
#undef script_type_list
#define script_type_list save_type_list(CMilitaryOutfit)

