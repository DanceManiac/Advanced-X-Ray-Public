#pragma once

class CBlender_SMAA : public IBlender
{
public:
    LPCSTR getComment() override { return "SMAA"; }
    BOOL canBeDetailed() override { return FALSE; }
    BOOL canBeLMAPped() override { return FALSE; }

    void Compile(CBlender_Compile& C) override;

    CBlender_SMAA();
    virtual ~CBlender_SMAA();
};