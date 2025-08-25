#pragma once


class CBlender_nightvision : public IBlender
{
public:
	virtual LPCSTR getComment() { return "nightvision"; }
	virtual BOOL canBeDetailed() { return FALSE; }
	virtual BOOL canBeLMAPped() { return FALSE; }

	virtual void Compile(CBlender_Compile& C);

	CBlender_nightvision();
	virtual ~CBlender_nightvision();
};


class CBlender_nightvision_hud_texture : public IBlender
{
public:
	virtual		LPCSTR		getComment() { return "Nightvision Hud Texture"; }
	virtual		BOOL		canBeDetailed() { return FALSE; }
	virtual		BOOL		canBeLMAPped() { return FALSE; }

	virtual		void		Compile(CBlender_Compile& C);

	CBlender_nightvision_hud_texture();
	virtual ~CBlender_nightvision_hud_texture();
};