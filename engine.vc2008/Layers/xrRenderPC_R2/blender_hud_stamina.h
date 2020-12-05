#pragma once

class CBlender_Hud_Stamina : public IBlender
{
public:
	virtual		LPCSTR		getComment() { return "Hud Stamina"; }
	virtual		BOOL		canBeDetailed() { return FALSE; }
	virtual		BOOL		canBeLMAPped() { return FALSE; }

	virtual		void		Compile(CBlender_Compile& C);

	CBlender_Hud_Stamina();
	virtual ~CBlender_Hud_Stamina();
};
