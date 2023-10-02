#include "pch_script.h"
#include "scope.h"
#include "LaserDesignator.h"

CScope::CScope	()
{
}

CScope::~CScope	() 
{
}

using namespace luabind;

#pragma optimize("s",on)
void CScope::script_register	(lua_State *L)
{
	module(L)
	[
		class_<CScope,CGameObject>("CScope")
			.def(constructor<>()),
		class_<CLaserDesignator, CGameObject>("CLaserDesignator")
			.def(constructor<>())
	];
}
