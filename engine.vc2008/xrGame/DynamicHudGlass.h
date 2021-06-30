#pragma once

namespace DynamicHudGlass
{
	void	UpdateDynamicHudGlass();
	extern bool	DynamicHudGlassEnabled;
	extern int	HudGlassElement;
	IC int	GetHudGlassElement() { return HudGlassElement; }
	IC bool	GetHudGlassEnabled() { return DynamicHudGlassEnabled; }
}