#pragma once

class CBlender_Hud_Frost : public IBlender
{
public:
	virtual		LPCSTR		getComment() { return "Hud Frost"; }
	virtual		BOOL		canBeDetailed() { return FALSE; }
	virtual		BOOL		canBeLMAPped() { return FALSE; }

	virtual		void		Compile(CBlender_Compile& C);

	CBlender_Hud_Frost();
	virtual ~CBlender_Hud_Frost();
};
