#pragma once

class CBlender_cut : public IBlender
{
public:
	virtual		LPCSTR		getComment() { return "cut"; }
	virtual		BOOL		canBeDetailed() { return FALSE; }
	virtual		BOOL		canBeLMAPped() { return FALSE; }

	virtual		void		Compile(CBlender_Compile& C);

	CBlender_cut();
	virtual ~CBlender_cut();
};
