///////////////////////////////////////////////////////////////
// MilitaryOutfit.h
// MilitaryOutfit - защитный костюм военного
///////////////////////////////////////////////////////////////

#pragma once

#include "pch_script.h"
#include "MilitaryOutfit.h"

CMilitaryOutfit::CMilitaryOutfit()
{
}

CMilitaryOutfit::~CMilitaryOutfit() 
{
}

using namespace luabind;

#pragma optimize("s",on)
void CMilitaryOutfit::script_register	(lua_State *L)
{
	module(L)
	[
		class_<CMilitaryOutfit,CGameObject>("CMilitaryOutfit")
			.def(constructor<>())
	];
}
