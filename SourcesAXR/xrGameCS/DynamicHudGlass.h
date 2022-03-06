#pragma once

namespace DynamicHudGlass
{
	void	UpdateDynamicHudGlass();
	extern bool	DynamicHudGlassEnabled;
	extern int	HudGlassElement;
	extern int	NightVisionType;
	IC int	GetHudGlassElement() { return HudGlassElement; }
	IC int	GetNightvisionType() { return NightVisionType; }
	IC bool	GetHudGlassEnabled() { return DynamicHudGlassEnabled; }
}