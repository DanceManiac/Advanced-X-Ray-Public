// File:        UIComboBox_script.cpp
// Description: exports CUIComobBox to LUA environment
// Created:     11.12.2004
// Author:      Serhiy O. Vynnychenko
// Mail:        narrator@gsc-game.kiev.ua
//
// Copyright 2004 GSC Game World
//

#include "pch_script.h"
#include "UIComboBox.h"

using namespace luabind;

void CUIComboBox::Init(float x, float y, float width)
{
	InitComboBox(Fvector2().set(x, y), width);
}

#pragma optimize("s",on)
void CUIComboBox::script_register(lua_State *L)
{
	module(L)
	[
		class_<CUIComboBox, CUIWindow>("CUIComboBox")
		.def(						constructor<>())
		.def("Init",				(void (CUIComboBox::*)(float, float, float)) &CUIComboBox::Init)
		.def("Init",				(void (CUIComboBox::*)(float, float, float, float))   &CUIComboBox::Init)
		.def("SetVertScroll",		&CUIComboBox::SetVertScroll)
		.def("SetListLength",		&CUIComboBox::SetListLength)
		.def("CurrentID",			&CUIComboBox::CurrentID)
		.def("SetCurrentID",		&CUIComboBox::SetItem)
		.def("disable_id",			&CUIComboBox::disable_id)
		.def("enable_id",			&CUIComboBox::enable_id)
		.def("SetCurrentValue",		&CUIComboBox::SetCurrentValue)
		
	];
}