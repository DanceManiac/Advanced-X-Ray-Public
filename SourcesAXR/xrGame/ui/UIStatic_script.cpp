#include "pch_script.h"
#include "UIStatic.h"
#include "UIAnimatedStatic.h"
#include "script_game_object.h"

void CUIStatic_SetColorA(CUIStatic* self, u8 alpha) 
{ 
	self->SetTextureColor(subst_alpha(self->GetTextureColor(), alpha));
}

void CUITextWnd_SetTextColor(CUIStatic* self, int a, int r, int g, int b) 
{ 
	self->TextItemControl()->SetTextColor(color_argb(a, r, g, b));
}

void CUITextWnd_SetTextColorA(CUIStatic* self, u8 alpha)
{
	self->TextItemControl()->SetTextColor(subst_alpha(self->TextItemControl()->GetTextColor(), alpha));
}

using namespace luabind;

#pragma optimize("s",on)
void CUIStatic::script_register(lua_State *L)
{
	module(L)
	[
		class_<CUILines>("CUILines")
		.def("SetFont",				&CUILines::SetFont)
		.def("SetText",				&CUILines::SetText)
		.def("SetTextST",			&CUILines::SetTextST)
		.def("GetText",				&CUILines::GetText)
		.def("SetElipsis",			&CUILines::SetEllipsis)
		.def("SetTextColor",		&CUILines::SetTextColor),

		class_<CUIStatic, CUIWindow>("CUIStatic")
		.def(						constructor<>())
		.def("TextControl",			&CUIStatic::TextItemControl)
		.def("InitTexture",			&CUIStatic::InitTexture )
		.def("SetColorA",			&CUIStatic_SetColorA)
		.def("SetTextureRect",		&CUIStatic::SetTextureRect_script)
		.def("SetHint",				&CUIStatic::SetHint) //MNP
		.def("SetNoShaderCache",	&CUIStatic::SetNoShaderCache)
		.def("SetStretchTexture",	&CUIStatic::SetStretchTexture)
		.def("GetTextureRect",		&CUIStatic::GetTextureRect_script),

		class_<CUITextWnd, CUIWindow>("CUITextWnd")
		.def(						constructor<>())
		.def("AdjustHeightToText",	&CUITextWnd::AdjustHeightToText)
		.def("AdjustWidthToText",	&CUITextWnd::AdjustWidthToText)
		.def("SetText",				&CUITextWnd::SetText)
		.def("SetTextST",			&CUITextWnd::SetTextST)
		.def("GetText",				&CUITextWnd::GetText)
		.def("SetFont",				&CUITextWnd::SetFont)
		.def("GetFont",				&CUITextWnd::GetFont)
		//.def("SetTextColor",		&CUITextWnd::SetTextColor)
		.def("SetTextColor",		&CUITextWnd_SetTextColor)
		.def("SetTextColorEx",		(void(CUITextWnd::*)(u32)) & CUITextWnd::SetTextColor)
		.def("SetTextColorA",		&CUITextWnd_SetTextColorA)
		
		.def("GetTextColor",		&CUITextWnd::GetTextColor)
		.def("SetTextComplexMode",	&CUITextWnd::SetTextComplexMode)
		.def("SetTextAlignment",	&CUITextWnd::SetTextAlignment)
		.def("GetTextAlignment",	&CUITextWnd::GetTextAlignment)
		.def("SetVTextAlignment",	&CUITextWnd::SetVTextAlignment)
		.def("GetVTextAlignment",	&CUITextWnd::GetVTextAlignment)
		.def("SetEllipsis",			&CUITextWnd::SetEllipsis)
		.def("SetTextOffset",		&CUITextWnd::SetTextOffset),
//		.def("",					&CUITextWnd::)

		class_<CUISleepStatic, CUIStatic>("CUISleepStatic")
		.def(						constructor<>())
	];
}