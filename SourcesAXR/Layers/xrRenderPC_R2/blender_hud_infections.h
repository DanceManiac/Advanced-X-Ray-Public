#pragma once

class CBlender_Hud_Infections : public IBlender
{
public:
	virtual		LPCSTR		getComment() { return "Hud Infections"; }
	virtual		BOOL		canBeDetailed() { return FALSE; }
	virtual		BOOL		canBeLMAPped() { return FALSE; }

	virtual		void		Compile(CBlender_Compile& C);

	CBlender_Hud_Infections();
	virtual ~CBlender_Hud_Infections();
};
