// file:	lopen.cpp
// func:	Open lua modules-namespace
// author:	ForserX
#include "stdafx.h"
#include "luaopen.hpp"

extern "C"
{
	extern const struct luaL_Reg funcs[];
	int luaopen_LuaXML_lib(lua_State* L);
	int luaopen_lfs(lua_State *L);
}

int lopen::luaopen_ext(lua_State *L)
{
	luaL_openlib(L, "mrsh", funcs, 0);
	luaopen_LuaXML_lib(L);
	luaopen_lfs(L);
	return 1;
}

#ifndef DEBUG
void lopen::put_function(lua_State* state, u8 const* buffer, u32 const buffer_size, const char* package_id)
{
	lua_getglobal(state, "package");
	lua_pushstring(state, "preload");
	lua_gettable(state, -2);

	lua_pushstring(state, package_id);
	luaL_loadbuffer(state, (char*)buffer, buffer_size, package_id);
	lua_settable(state, -3);
}
#endif

void lopen::open_lib(lua_State *L, const char* module_name, lua_CFunction function)
{
	lua_pushcfunction(L, function);
	lua_pushstring(L, module_name);
	lua_call(L, 1, 0);
}

void lopen::open_luaicp(lua_State* Ls)
{
	const HMODULE hLib = GetModuleHandle("luaicp.dll");
	if (hLib)
	{
		Msg("Lua Interceptor found! Attaching :)");

		typedef void(WINAPI *LUA_CAPTURE)(lua_State *L);

		LUA_CAPTURE ExtCapture = (LUA_CAPTURE)GetProcAddress(hLib, "ExtCapture");
		if (ExtCapture)
			ExtCapture(Ls);
		else
			Msg("ExtCapture proc not found in luaicp.dll");
	}
}

static int report(lua_State *L, int status)
{
	if (status && !lua_isnil(L, -1))
	{
		const char *msg = lua_tostring(L, -1);
		if (!msg)
			msg = "(error object is not a string)";
		Msg("! [LUA_JIT] %s", msg);
		lua_pop(L, 1);
	}
	return status;
}

static int loadjitmodule(lua_State *L, const char *notfound)
{
	lua_getglobal(L, "require");
	lua_pushliteral(L, "jit.");
	lua_pushvalue(L, -3);
	lua_concat(L, 2);
	if (lua_pcall(L, 1, 1, 0)) {
		const char *msg = lua_tostring(L, -1);
		if (msg && !strncmp(msg, "module ", 7)) {
			Msg("! [LUA_JIT] %s", notfound);
			return 1;
		}
		else
			return report(L, 1);
	}
	lua_getfield(L, -1, "start");
	lua_remove(L, -2);  /* drop module table */
	return 0;
}

SCRIPT_API int dojitcmd(lua_State *L, const char *cmd)
{
	const char *val = strchr(cmd, '=');
	lua_pushlstring(L, cmd, val ? val - cmd : xstr::strlen(cmd));
	lua_getglobal(L, "jit");  /* get jit.* table */
	lua_pushvalue(L, -2);
	lua_gettable(L, -2);  /* lookup library function */
	if (!lua_isfunction(L, -1)) {
		lua_pop(L, 2);  /* drop non-function and jit.* table, keep module name */
		if (loadjitmodule(L, "unknown luaJIT command"))
			return 1;
	}
	else {
		lua_remove(L, -2);  /* drop jit.* table */
	}
	lua_remove(L, -2);  /* drop module name */
	if (val) lua_pushstring(L, val + 1);
	return report(L, lua_pcall(L, val ? 1 : 0, 0, 0));
}