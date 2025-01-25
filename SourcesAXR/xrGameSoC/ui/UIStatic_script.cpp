#include "pch_script.h"
#include "UIStatic.h"
#include "UILines.h"
#include "script_game_object.h"

void CUIStatic_SetColorA(CUIStatic* self, u8 alpha) 
{ 
	self->SetColor(subst_alpha(self->GetColor(), alpha)); 
}

void CUIStatic_SetTextColor(CUIStatic* self, int a, int r, int g, int b) 
{ 
	self->SetTextColor(color_argb(a, r, g, b)); 
}

void CUIStatic_SetTextColorA(CUIStatic* self, u8 alpha)
{
	self->SetTextColor(subst_alpha(self->GetColor(), alpha));
}

using namespace luabind;

#pragma optimize("s",on)
void CUIStatic::script_register(lua_State *L)
{
	module(L)
	[
		class_<CUIStatic, CUIWindow>("CUIStatic")
		.def(						constructor<>())

		.def("SetText",					(void (CUIStatic::*)(LPCSTR)) (&CUIStatic::SetText) )
		.def("SetTextST",				(void (CUIStatic::*)(LPCSTR)) (&CUIStatic::SetTextST) )
		.def("GetText",					&CUIStatic::GetText)

		.def("SetTextX",				&CUIStatic::SetTextX)
		.def("SetTextY",				&CUIStatic::SetTextY)
		.def("GetTextX",				&CUIStatic::GetTextX)
		.def("GetTextY",				&CUIStatic::GetTextY)
		
		.def("SetColor",				&CUIStatic::SetColor)
		.def("GetColor",				&CUIStatic::GetColor)
		//.def("SetTextColor",			&CUIStatic::SetTextColor_script)
		.def("SetColorA",				&CUIStatic_SetColorA)
		.def("SetTextColor",			&CUIStatic_SetTextColor)
		.def("SetTextColorEx",			(void(CUIStatic::*)(u32)) & CUIStatic::SetTextColor)
		.def("SetTextColorA",			&CUIStatic_SetTextColorA)
		.def("InitTexture",				&CUIStatic::InitTexture)
		.def("SetTextureOffset",		(void(CUIStatic::*)(float, float))&CUIStatic::SetTextureOffset)
		.def("SetBaseTextureOffset",	(void(CUIStatic::*)(float, float))&CUIStatic::SetBaseTextureOffset)
		.def("GetTextureOffset",		&CUIStatic::GetTextureOffset)
		.def("GetBaseTextureOffset",	&CUIStatic::GetBaseTextureOffset)

		.def("SetOriginalRect",			&CUIStatic::SetOriginalRect_script)
		.def("GetOriginalRect",			&CUIStatic::GetOriginalRect_script)
		.def("SetStretchTexture",		&CUIStatic::SetStretchTexture)
		.def("GetStretchTexture",		&CUIStatic::GetStretchTexture)

		.def("SetHint",					&CUIStatic::SetHint) //MNP

		.def("SetTextAlign",			&CUIStatic::SetTextAlign_script)
		.def("GetTextAlign",			&CUIStatic::GetTextAlign_script)

		.def("SetVTextAlign",			&CUIStatic::SetVTextAlignment)
		.def("GetVTextAlign",			&CUIStatic::GetVTextAlignment)

		.def("SetHeading",				&CUIStatic::SetHeading)
		.def("GetHeading",				&CUIStatic::GetHeading)
	
		.def("ClipperOn",				&CUIStatic::ClipperOn)
		.def("ClipperOff",				(void(CUIStatic::*)(void))&CUIStatic::ClipperOff)
		.def("GetClipperState",			&CUIStatic::GetClipperState)
		.def("SetElipsis",				&CUIStatic::SetEllipsis)

		.def("SetTextComplexMode",		&CUIStatic::SetTextComplexMode)
		.def("AdjustWidthToText",		&CUIStatic::AdjustWidthToText)
		.def("AdjustHeightToText",		&CUIStatic::AdjustHeightToText)
		
	];
}