#include "stdafx.h"
#include "CustomOutfit.h"
#include "ActorHelmet.h"

using namespace luabind;

#pragma optimize("s",on)
void CCustomOutfit::script_register(lua_State* L)
{
    module(L)
    [
        class_<CCustomOutfit, CGameObject>("CCustomOutfit")
            .def(constructor<>())
    ];
};