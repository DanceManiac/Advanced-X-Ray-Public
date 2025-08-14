#include "stdafx.h"

#include "pch_script.h"
#include "CustomPsyHelmet.h"
#include "ActorHelmet.h"

CPsyHelmet::CPsyHelmet()
{
}

CPsyHelmet::~CPsyHelmet() 
{
}

using namespace luabind;

#pragma optimize("s",on)
void CPsyHelmet::script_register	(lua_State *L)
{
	module(L)
	[
		class_<CPsyHelmet,CGameObject>("CPsyHelmet")
		.def(constructor<>())

	];
}
