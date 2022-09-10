#pragma once
extern "C"
{
	#include "luasocket/luasocket.h"
}
#include "xrScripts.h"
#include "../xrScripts/luajit/lua.hpp"

/* JIT engine control command: try jit library first or load add-on module */
SCRIPT_API int dojitcmd(lua_State *L, const char *cmd);
SCRIPT_API int dojitopt(lua_State *L, const char *opt);
extern "C" void pdebug_init(lua_State * L);
struct SCRIPT_API lopen
{
	static int luaopen_ext(lua_State *L);
	static void open_lib(lua_State *L, const char* module_name, lua_CFunction function);
	static void open_luaicp(lua_State* Ls);
#ifndef DEBUG
	static void put_function(lua_State* state, u8 const* buffer, u32 const buffer_size, const char* package_id);
#endif // #ifndef DEBUG
	__forceinline static void openlua(lua_State* L)
	{
		open_lib(L, "", luaopen_base);
		open_lib(L, LUA_LOADLIBNAME, luaopen_package);
		open_lib(L, LUA_TABLIBNAME, luaopen_table);
		open_lib(L, LUA_IOLIBNAME, luaopen_io);
		open_lib(L, LUA_OSLIBNAME, luaopen_os);
		open_lib(L, LUA_MATHLIBNAME, luaopen_math);
		open_lib(L, LUA_STRLIBNAME, luaopen_string);
		// Added sv3nk //---------------------------------------
		open_lib(L, LUA_BITLIBNAME, luaopen_bit);
		open_lib(L, LUA_FFILIBNAME, luaopen_ffi);
		luaopen_ext(L);
		// End //-----------------------------------------------
#ifndef MASTER_GOLD
#ifndef DEBUG
		if (strstr(Core.Params, "-debugger_lua"))
		{
#endif
			open_lib(L, "socket.core", luaopen_socket_core);
			pdebug_init(L);
			open_lib(L, LUA_DBLIBNAME, luaopen_debug);
#ifndef DEBUG
		}
#endif
#endif

#ifndef MASTER_GOLD
		if (!strstr(Core.Params, "-nojit"))
#endif
		{
			open_lib(L, LUA_JITLIBNAME, luaopen_jit);
			
		}


#ifdef LUACP_API
			open_luaicp(L);
#endif
	}
}; // struct lua;
