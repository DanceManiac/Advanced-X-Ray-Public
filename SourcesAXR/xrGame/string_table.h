//////////////////////////////////////////////////////////////////////////
// string_table.h:		таблица строк используемых в игре
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "string_table_defs.h"

DEFINE_MAP		(STRING_ID, STRING_VALUE, STRING_TABLE_MAP, STRING_TABLE_MAP_IT);

struct STRING_TABLE_DATA
{
	shared_str				m_sLanguage;
	
	STRING_TABLE_MAP		m_StringTable;
	
	STRING_TABLE_MAP		m_string_key_binding;
};


class CStringTable 
{
public:
								CStringTable			();
			void				Init					();

	static void					Destroy					();
	
	STRING_VALUE				translate				(const STRING_ID& str_id) const;
	STRING_VALUE				ReturnLanguage			() { return (translate(pData->m_sLanguage)); }
			void				rescan					();
			void				ReloadLanguage			();

	static	BOOL				m_bWriteErrorsToLog;
			int					LanguagesNum;
			xr_token*			GetLanguagesToken		() const;
	static	u32					LanguageID;
	static	void				ReparseKeyBindings		();
private:
			void				FillLanguageToken		();
			void				SetLanguage				();
			void				Load					(LPCSTR xml_file);
	static STRING_VALUE			ParseLine				(LPCSTR str, LPCSTR key, bool bFirst);
	static STRING_TABLE_DATA*	pData;
	static xr_vector<xr_token>	languagesToken;
};