#pragma once

class CBlender_Hud_Acid_Rain : public IBlender
{
public:
	virtual		LPCSTR		getComment() { return "Hud Acid Rain"; }
	virtual		BOOL		canBeDetailed() { return FALSE; }
	virtual		BOOL		canBeLMAPped() { return FALSE; }

	virtual		void		Compile(CBlender_Compile& C);

	CBlender_Hud_Acid_Rain();
	virtual ~CBlender_Hud_Acid_Rain();
};

class CBlender_Hud_Radiation_Rain : public IBlender
{
public:
	virtual		LPCSTR		getComment() { return "Hud Radiation Rain"; }
	virtual		BOOL		canBeDetailed() { return FALSE; }
	virtual		BOOL		canBeLMAPped() { return FALSE; }

	virtual		void		Compile(CBlender_Compile& C);

	CBlender_Hud_Radiation_Rain();
	virtual ~CBlender_Hud_Radiation_Rain();
};