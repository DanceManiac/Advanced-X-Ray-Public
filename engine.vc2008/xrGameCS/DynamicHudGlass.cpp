////////////////////////////////////////////////////////////////////////////
//	Module 		: DynamicHudGlass.cpp
//	Created 	: 12.05.2021
//  Modified 	: 11.07.2021
//	Author		: Dance Maniac (M.F.S. Team)
//	Description : Dynamic HUD glass functions and variables
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "DynamicHudGlass.h"
#include "CustomOutfit.h"
#include "Actor.h"
#include "Inventory.h"

namespace DynamicHudGlass
{
	bool DynamicHudGlassEnabled = false;
	int	HudGlassElement = 0;

	void UpdateDynamicHudGlass()
	{
		CCustomOutfit* outfit = smart_cast<CCustomOutfit*>(Actor()->inventory().ItemFromSlot(OUTFIT_SLOT));

		if (outfit)
		{
			float condition = outfit->GetCondition();
			bool OutfitHasGlass = outfit->m_b_HasGlass;
			HudGlassElement = 0;

			if (OutfitHasGlass)
			{
				DynamicHudGlassEnabled = true;
			}
			else
			{
				DynamicHudGlassEnabled = false;
			}

			if (condition < 0.85)
			{
				if (condition > 0.75)
					HudGlassElement = 1;
				else if (condition > 0.65)
					HudGlassElement = 2;
				else if (condition > 0.45)
					HudGlassElement = 3;
				else if (condition > 0.25)
					HudGlassElement = 4;
				else
					HudGlassElement = 5;
			}
		}
		else
		{
			HudGlassElement = 0;
			DynamicHudGlassEnabled = false;
		}
	}
}