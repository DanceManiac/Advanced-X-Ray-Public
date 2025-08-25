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

class CBlender_Hud_Cold : public IBlender
{
public:
	virtual		LPCSTR		getComment() { return "Hud Cold"; }
	virtual		BOOL		canBeDetailed() { return FALSE; }
	virtual		BOOL		canBeLMAPped() { return FALSE; }

	virtual		void		Compile(CBlender_Compile& C);

	CBlender_Hud_Cold();
	virtual ~CBlender_Hud_Cold();
};

class CBlender_Hud_Snowfall : public IBlender
{
public:
	virtual		LPCSTR		getComment() { return "Hud Snowfall"; }
	virtual		BOOL		canBeDetailed() { return FALSE; }
	virtual		BOOL		canBeLMAPped() { return FALSE; }

	virtual		void		Compile(CBlender_Compile& C);

	CBlender_Hud_Snowfall();
	virtual ~CBlender_Hud_Snowfall();
};