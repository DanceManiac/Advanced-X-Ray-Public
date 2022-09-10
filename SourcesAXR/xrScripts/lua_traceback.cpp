////////////////////////////////////////////////////////////////////////////
//	Module 		: lua_tools.h
//	Created 	: 29.07.2014
//	Author		: Alexander Petrov
////////////////////////////////////////////////////////////////////////////
//	Module 		: lua_traceback.h
//  Created 	: 12.07.2017
//	Author		: ForserX
//	Description : Lua functionality extension
////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
extern "C"
{
#include <lua/lua.h>
#include "luajit/luajit.h"
};
#include "lua_traceback.hpp"


SCRIPT_API const char* get_traceback(lua_State *L, int depth)
{
	// alpet: Lua traceback added
	int top = lua_gettop(L);
	lua_getfield(L, LUA_GLOBALSINDEX, "debug");
	lua_getfield(L, -1, "traceback");
	lua_pushstring(L, "\t");
	lua_pushinteger(L, 1);

	const char* m_traceback = "cannot get Lua traceback ";

	if (!lua_pcall(L, 2, 1, 0))
	{
		m_traceback = lua_tostring(L, -1);
		lua_pop(L, 1);
	}

	lua_settop (L, top);

	return m_traceback;
}