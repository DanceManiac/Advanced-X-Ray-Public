#pragma once

class CBlender_Vignette : public IBlender
{
public:
	LPCSTR getComment() override { return "Vignette"; }
	BOOL canBeDetailed() override { return FALSE; }
	BOOL canBeLMAPped() override { return FALSE; }

	void Compile(CBlender_Compile& C) override;

	CBlender_Vignette();
	virtual ~CBlender_Vignette();
};