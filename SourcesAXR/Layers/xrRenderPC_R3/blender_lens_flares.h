#pragma once

class CBlender_LFX : public IBlender
{
public:
	virtual		LPCSTR		getComment() { return "SFZ Lens Flares"; }
	virtual		BOOL		canBeDetailed() { return FALSE; }
	virtual		BOOL		canBeLMAPped() { return FALSE; }

	virtual		void		Compile(CBlender_Compile& C);

	CBlender_LFX();
	virtual ~CBlender_LFX();
};