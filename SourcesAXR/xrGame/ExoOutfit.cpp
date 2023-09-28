///////////////////////////////////////////////////////////////
// ExoOutfit.h
// ExoOutfit - защитный костюм с усилением
///////////////////////////////////////////////////////////////

#pragma once

#include "pch_script.h"
#include "exooutfit.h"

CExoOutfit::CExoOutfit()
{
}

CExoOutfit::~CExoOutfit() 
{
}

using namespace luabind;

#pragma optimize("s",on)
void CExoOutfit::script_register	(lua_State *L)
{
	module(L)
	[
		class_<CExoOutfit,CGameObject>("CExoOutfit")
			.def(constructor<>())
	];
}
