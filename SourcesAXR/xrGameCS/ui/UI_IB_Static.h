// File:		UI_IB_Static.h
// Description:	Inheritance of UIInteractiveBackground template class with some
//				CUIStatic features
// Created:		09.02.2005
// Author:		Serge Vynnychenko
// Mail:		narrator@gsc-game.kiev.ua

// Copyright 2005 GSC Game World

#pragma once

#include "UIInteractiveBackground.h"
#include "UIStatic.h"

class CUI_IB_Static : public CUIInteractiveBackground<CUIStatic>
{
public:
	virtual void	SetTextureOffset		(float x, float y);
	virtual void	SetTextureOffset		(Fvector2 offset);
	virtual void	SetBaseTextureOffset	(float x, float y);
	virtual void	SetBaseTextureOffset	(Fvector2 offset);
	virtual void	SetStretchTexture		(bool stretch_texture);
};