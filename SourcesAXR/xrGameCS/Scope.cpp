#include "pch_script.h"
#include "scope.h"
#include "LaserDesignator.h"
#include "TacticalTorch.h"

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
			.def(constructor<>()),
		class_<CTacticalTorch, CGameObject>("CTacticalTorch")
			.def(constructor<>())
	];
}
