///////////////////////////////////////////////////////////////
// ScientificOutfit.cpp
// ScientificOutfit - �������� ������ �������
///////////////////////////////////////////////////////////////

#pragma once

#include "pch_script.h"
#include "scientificoutfit.h"

CScientificOutfit::CScientificOutfit()
{
}

CScientificOutfit::~CScientificOutfit() 
{
}

using namespace luabind;

#pragma optimize("s",on)
void CScientificOutfit::script_register	(lua_State *L)
{
	module(L)
	[
		class_<CScientificOutfit,CGameObject>("CScientificOutfit")
			.def(constructor<>())
	];
}
