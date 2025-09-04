#include "pch_script.h"
#include "Scope.h"
#include "Silencer.h"
#include "GrenadeLauncher.h"
#include "LaserDesignator.h"
#include "TacticalTorch.h"
#include "WeaponAddonStock1.h"
#include "WeaponAddonGripHorizontal.h"
#include "WeaponAddonGripVertical.h"

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
		class_<CSilencer,CGameObject>("CSilencer")
			.def(constructor<>()),
		class_<CGrenadeLauncher,CGameObject>("CGrenadeLauncher")
			.def(constructor<>()),
		class_<CStock, CGameObject>("CStock")
			.def(constructor<>()),
		class_<CHorGrip, CGameObject>("CHorGrip")
			.def(constructor<>()),
		class_<CVerGrip, CGameObject>("CVerGrip")
			.def(constructor<>()),
		class_<CLaserDesignator, CGameObject>("CLaserDesignator")
			.def(constructor<>()),
		class_<CTacticalTorch, CGameObject>("CTacticalTorch")
			.def(constructor<>())
	];
}
