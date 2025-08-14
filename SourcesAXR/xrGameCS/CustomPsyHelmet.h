///////////////////////////////////////////////////////////////
// CustomPsyHelmet.cpp
// CustomPsyHelmet - пси-шлем, устанавливаемый в отдельный слот
///////////////////////////////////////////////////////////////


#pragma once

#include "ActorHelmet.h"
#include "script_export_space.h"

class CPsyHelmet : public CHelmet {
private:
    typedef	CHelmet inherited;
public:
	CPsyHelmet(void);
	virtual ~CPsyHelmet(void);

	DECLARE_SCRIPT_REGISTER_FUNCTION
};
add_to_type_list(CPsyHelmet)
#undef script_type_list
#define script_type_list save_type_list(CPsyHelmet)
