#pragma once

class CBlender_ChromaticAberration : public IBlender
{
public:
	LPCSTR getComment() override { return "Chromatic Aberration"; }
	BOOL canBeDetailed() override { return FALSE; }
	BOOL canBeLMAPped() override { return FALSE; }

	void Compile(CBlender_Compile& C) override;

	CBlender_ChromaticAberration();
	virtual ~CBlender_ChromaticAberration();
};

class CBlender_ChromaticAberration2 : public IBlender
{
public:
	LPCSTR getComment() override { return "Chromatic Aberration Acid"; }
	BOOL canBeDetailed() override { return FALSE; }
	BOOL canBeLMAPped() override { return FALSE; }

	void Compile(CBlender_Compile& C) override;

	CBlender_ChromaticAberration2();
	virtual ~CBlender_ChromaticAberration2();
};