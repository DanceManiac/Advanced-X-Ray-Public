////////////////////////////////////////////////////////////////////////////
//	Module 		: script_token_list.cpp
//	Created 	: 21.05.2004
//  Modified 	: 21.05.2004
//	Author		: Dmitriy Iassenev
//	Description : Script token list class
////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "script_token_list.h"

CScriptTokenList::~CScriptTokenList	()
{
	for(auto it : tokens())
		xr_free(it.name);
}
