#pragma once

#ifdef XR_SCRIPTS
#	define SCRIPT_API 		__declspec(dllexport)
#else 
#	define SCRIPT_API		__declspec(dllimport)
#endif // #ifdef XR_SCRIPTS

#define LUABIND_API		SCRIPT_API
#define LUASTUDIO_API	SCRIPT_API
#define LUACORE			SCRIPT_API
#ifdef LUA_API
#undef LUA_API
#endif
#define LUA_API			SCRIPT_API