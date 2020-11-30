#pragma once

class CBlender_Hud_Blood: public IBlender
{
public:
	virtual		LPCSTR		getComment() { return "Hud Blood"; }
	virtual		BOOL		canBeDetailed() { return FALSE; }
	virtual		BOOL		canBeLMAPped() { return FALSE; }

	virtual		void		Compile(CBlender_Compile& C);

	CBlender_Hud_Blood();
	virtual ~CBlender_Hud_Blood();
};
