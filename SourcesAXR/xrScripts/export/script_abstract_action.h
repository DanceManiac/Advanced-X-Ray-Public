////////////////////////////////////////////////////////////////////////////
//	Module 		: script_abstract_action.h
//	Created 	: 30.09.2003
//  Modified 	: 29.06.2004
//	Author		: Dmitriy Iassenev
//	Description : Script abstract action
////////////////////////////////////////////////////////////////////////////
#pragma once
#include "../xrScripts.h"

class SCRIPT_API CScriptAbstractAction 
{
public:
	bool			m_bCompleted;

public:
					CScriptAbstractAction	();
	virtual			~CScriptAbstractAction	() = default;
	virtual	bool	completed				();
};
