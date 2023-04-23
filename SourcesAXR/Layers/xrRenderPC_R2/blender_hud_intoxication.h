#pragma once

class CBlender_Hud_Intoxication : public IBlender
{
public:
	virtual		LPCSTR		getComment() { return "Hud Intoxication"; }
	virtual		BOOL		canBeDetailed() { return FALSE; }
	virtual		BOOL		canBeLMAPped() { return FALSE; }

	virtual		void		Compile(CBlender_Compile& C);

	CBlender_Hud_Intoxication();
	virtual ~CBlender_Hud_Intoxication();
};
