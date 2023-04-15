#pragma once

class CBlender_FilmGrain : public IBlender
{
public:
	LPCSTR getComment() override { return "Chromatic Aberration"; }
	BOOL canBeDetailed() override { return FALSE; }
	BOOL canBeLMAPped() override { return FALSE; }

	void Compile(CBlender_Compile& C) override;

	CBlender_FilmGrain();
	virtual ~CBlender_FilmGrain();
};