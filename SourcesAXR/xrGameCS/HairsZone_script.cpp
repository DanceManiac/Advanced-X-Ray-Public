#include "pch_script.h"
#include "HairsZone.h"
#include "NoGravityZone.h"

using namespace luabind;

#pragma optimize("s",on)
void CHairsZone::script_register	(lua_State *L)
{
	module(L)
	[
        class_<CHairsZone, CGameObject>("CHairsZone")
            .def(constructor<>()),
        class_<CNoGravityZone,CGameObject>("CNoGravityZone")
            .def(constructor<>())
	];
}
