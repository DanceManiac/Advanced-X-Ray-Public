////////////////////////////////////////////////////////////////////////////
//  Created     : 19.06.2018
//  Authors     : Xottab_DUTY (OpenXRay project)
//                FozeSt
//                Unfainthful
//
//  Copyright (C) GSC Game World - 2018
////////////////////////////////////////////////////////////////////////////
#pragma once

#include "../../xrEngine/ILoadingScreen.h"
#include "UIStatic.h"
#include "UIWindow.h"
#include "UIProgressBar.h"
#include "UIProgressShape.h"
#include <mutex>

class UILoadingScreen : public ILoadingScreen, public CUIWindow
{
	std::recursive_mutex loadingLock;

	CUIStatic* loadingProgressBackground;
	CUIProgressBar* loadingProgress;
	CUIProgressShape* loadingProgressShape;
	CUIStatic* loadingProgressPercent;
	CUIStatic* loadingLogo;

	CUIStatic* loadingStage;
	CUIStatic* loadingHeader;
	CUIStatic* loadingTipNumber;
	CUIStatic* loadingTip;

	u32 maxTip;

	bool force_stop = false;
	bool force_drop = false;

public:
	UILoadingScreen();

	void Initialize() override;

	void Show(bool status) override;
	bool IsShown() override;

	void Update(const int stagesCompleted, const int stagesTotal) override;
	void ForceDrop() override;
	void ForceFinish() override;

	void SetLevelLogo(const char* name) override;
	void SetStageTitle(const char* title) override;
	void SetStageTip(const char* header, const char* tipNumber, const char* tip) override;
	pcstr GetDebugType() override { return "UILoadingScreen"; }
};