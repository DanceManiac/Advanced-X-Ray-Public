#pragma once

class CBlender_Hud_Rainfall : public IBlender
{
public:
	virtual		LPCSTR		getComment() { return "Hud Rainfall"; }
	virtual		BOOL		canBeDetailed() { return FALSE; }
	virtual		BOOL		canBeLMAPped() { return FALSE; }

	virtual		void		Compile(CBlender_Compile& C);

	CBlender_Hud_Rainfall();
	virtual ~CBlender_Hud_Rainfall();
};

class CBlender_Hud_Sweated : public IBlender
{
public:
	virtual		LPCSTR		getComment() { return "Hud Sweated"; }
	virtual		BOOL		canBeDetailed() { return FALSE; }
	virtual		BOOL		canBeLMAPped() { return FALSE; }

	virtual		void		Compile(CBlender_Compile& C);

	CBlender_Hud_Sweated();
	virtual ~CBlender_Hud_Sweated();
};

class CBlender_Hud_Rainfall_Acid : public IBlender
{
public:
	virtual		LPCSTR		getComment() { return "Hud Rainfall Acid"; }
	virtual		BOOL		canBeDetailed() { return FALSE; }
	virtual		BOOL		canBeLMAPped() { return FALSE; }

	virtual		void		Compile(CBlender_Compile& C);

	CBlender_Hud_Rainfall_Acid();
	virtual ~CBlender_Hud_Rainfall_Acid();
};