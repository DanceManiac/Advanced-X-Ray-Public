#pragma once

class CBlender_hud_mask : public IBlender
{
public:
	virtual		LPCSTR		getComment() { return "Hud Mask"; }
	virtual		BOOL		canBeDetailed() { return FALSE; }
	virtual		BOOL		canBeLMAPped() { return FALSE; }

	virtual		void		Compile(CBlender_Compile& C);

	CBlender_hud_mask();
	virtual ~CBlender_hud_mask();
};