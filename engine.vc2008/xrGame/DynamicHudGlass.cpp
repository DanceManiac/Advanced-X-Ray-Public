////////////////////////////////////////////////////////////////////////////
//	Module 		: DynamicHudGlass.cpp
//	Created 	: 12.05.2021
//  Modified 	: 12.05.2021
//	Author		: Dance Maniac (M.F.S. Team)
//	Description : Dynamic HUD glass functions and variables
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "DynamicHudGlass.h"
#include "ActorHelmet.h"
#include "CustomOutfit.h"
#include "Actor.h"
#include "Inventory.h"

namespace DynamicHudGlass
{
	bool DynamicHudGlassEnabled = false;
	int	HudGlassElement = 0;

	void UpdateDynamicHudGlass()
	{
		CHelmet* helmet = smart_cast<CHelmet*>(Actor()->inventory().ItemFromSlot(HELMET_SLOT));
		CCustomOutfit* outfit = smart_cast<CCustomOutfit*>(Actor()->inventory().ItemFromSlot(OUTFIT_SLOT));

		if (helmet)
		{
			float condition = helmet->GetCondition();
			HudGlassElement = 0;
			DynamicHudGlassEnabled = true;
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
		if (outfit)
		{
			float condition = outfit->GetCondition();
			HudGlassElement = 0;
			DynamicHudGlassEnabled = true;
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
		}
	}
}