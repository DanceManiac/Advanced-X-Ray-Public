////////////////////////////////////////////////////////////////////////////
//	Module 		: script_token_list.h
//	Created 	: 21.05.2004
//  Modified 	: 21.05.2004
//	Author		: Dmitriy Iassenev
//	Description : Script token list class
////////////////////////////////////////////////////////////////////////////
#pragma once
#include "script_export_space.h"
#include "../xrScripts.h"

class SCRIPT_API CScriptTokenList 
{
protected:
	using TOKEN_LIST = xvector<xr_token>;
	using iterator = TOKEN_LIST::iterator;
	using const_iterator = TOKEN_LIST::const_iterator;

protected:
	struct CTokenPredicateName 
	{
		const char*			m_name;

		IC				CTokenPredicateName(const char* name) { m_name = name; }
		IC		bool		operator()(const xr_token &token) const { return (token.name && !xstr::strcmp(token.name,m_name)); }
	};
	
	struct CTokenPredicateID 
	{
		int				m_id;

		IC				CTokenPredicateID(int id){ m_id	= id; }
		IC		bool	operator()(const xr_token &token) const { return (token.name && (token.id == m_id)); }
	};

protected:
	TOKEN_LIST					m_token_list;

protected:
	IC		iterator			token				(const char* name);
	IC		iterator			token				(int id);

public:
	IC							CScriptTokenList	();
								~CScriptTokenList	();
	IC		void				add					(const char* name, int id);
	IC		void				remove				(const char* name);
	IC		void				clear				();
	IC		int					id					(const char* name);
	IC		const char*			name				(int id);
	IC		const TOKEN_LIST	&tokens				() const;
	IC		TOKEN_LIST			&tokens				();
	DECLARE_SCRIPT_REGISTER_FUNCTION
};
add_to_type_list(CScriptTokenList)
#undef script_type_list
#define script_type_list save_type_list(CScriptTokenList)

#include "script_token_list_inline.h"