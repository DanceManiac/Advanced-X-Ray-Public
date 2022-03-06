#pragma once

class CBlender_Hud_Bleeding : public IBlender
{
public:
	virtual		LPCSTR		getComment() { return "Hud Bleeding"; }
	virtual		BOOL		canBeDetailed() { return FALSE; }
	virtual		BOOL		canBeLMAPped() { return FALSE; }

	virtual		void		Compile(CBlender_Compile& C);

	CBlender_Hud_Bleeding();
	virtual ~CBlender_Hud_Bleeding();
};
