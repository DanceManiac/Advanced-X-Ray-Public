#pragma once
#include "../BaseMonster/base_monster.h"
#include "../../../../XrServerEntitiesCS/script_export_space.h"

class CStateManagerFracture;

class CFracture : public CBaseMonster {
	typedef		CBaseMonster		inherited;
	
public:
					CFracture 			();
	virtual			~CFracture 			();	

	virtual void	Load				(LPCSTR section);
	virtual void	CheckSpecParams		(u32 spec_params);

	DECLARE_SCRIPT_REGISTER_FUNCTION
};

add_to_type_list(CFracture)
#undef script_type_list
#define script_type_list save_type_list(CFracture)
