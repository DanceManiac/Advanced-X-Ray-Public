#pragma once
#include "xrScripts.h"
#include <Windows.h>

namespace import_ses
{
	LUACORE	void		 LuaLog		(const char* caMessage);
	LUACORE const char*	 user_name	();
}
