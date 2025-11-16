#include "stdafx.h"
#include "pch_script.h"
#include "CustomOutfit.h"

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