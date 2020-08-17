#pragma once

#include "UIWindow.h"
#include "../../XrServerEntitiesCS/script_export_space.h"

class CUIScrollView;

class CUIMapInfo : public CUIWindow 
{
public:
				CUIMapInfo			();
				~CUIMapInfo			();
			void InitMapInfo		(Fvector2 pos,Fvector2 size);
			void InitMap			(LPCSTR map_name, LPCSTR map_ver);
		LPCSTR	 GetLargeDesc		();

	DECLARE_SCRIPT_REGISTER_FUNCTION
protected:
	CUIScrollView*	m_view;
	shared_str		m_large_desc;
};

add_to_type_list(CUIMapInfo)
#undef script_type_list
#define script_type_list save_type_list(CUIMapInfo)
