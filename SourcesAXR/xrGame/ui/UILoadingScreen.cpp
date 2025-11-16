////////////////////////////////////////////////////////////////////////////
//  Created     : 19.06.2018
//  Authors     : Xottab_DUTY (OpenXRay project)
//                FozeSt
//                Unfainthful
//
//  Copyright (C) GSC Game World - 2018
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UILoadingScreen.h"

#include "../xrEngine/x_ray.h"
#include "../xrEngine/GameFont.h"
#include "UIHelper.h"
#include "xrUIXmlParser.h"
#include "UIXmlInit.h"
#include "string_table.h"

extern ENGINE_API int ps_rs_loading_stages;

UILoadingScreen::UILoadingScreen()
	: loadingLogo(nullptr), loadingProgress(nullptr), loadingProgressShape(nullptr), loadingProgressBackground(nullptr),
		loadingStage(nullptr), loadingHeader(nullptr), loadingProgressPercent(nullptr),
		loadingTipNumber(nullptr), loadingTip(nullptr)
{
	UILoadingScreen::Initialize();
}

void UILoadingScreen::Initialize()
{
	CUIXml uiXml;
	uiXml.Load(CONFIG_PATH, UI_PATH, "ui_mm_loading_screen.xml");

	const auto loadProgressBackground = [&]() { loadingProgressBackground = UIHelper::CreateStatic(uiXml, "loading_progress_background", this, false); };
	const auto loadProgressBar = [&]() { loadingProgress = UIHelper::CreateProgressBar(uiXml, "loading_progress", this, false); };
	const auto loadProgressShape = [&]() { loadingProgressShape = UIHelper::CreateProgressShape(uiXml, "loading_progress_shape", this, false); };

	const auto loadBackground = [&] { UIHelper::CreateStatic(uiXml, "background", this, false); };

	const auto node = uiXml.NavigateToNodeWithAttribute("loading_progress", "under_background", "1");
	if (node)
	{
		loadBackground();
		loadProgressBackground();
		loadProgressBar();
		loadProgressShape();
	}
	else
	{
		loadProgressBackground();
		loadProgressBar();
		loadProgressShape();
		loadBackground();
	}

	loadingLogo = UIHelper::CreateStatic(uiXml, "loading_logo", this, false);
	loadingProgressPercent = UIHelper::CreateStatic(uiXml, "loading_progress_percent", this, false);
	loadingStage = UIHelper::CreateStatic(uiXml, "loading_stage", this, false);
	loadingHeader = UIHelper::CreateStatic(uiXml, "loading_header", this, false);
	loadingTipNumber = UIHelper::CreateStatic(uiXml, "loading_tip_number", this, false);
	loadingTip = UIHelper::CreateStatic(uiXml, "loading_tip", this, false);
}

void UILoadingScreen::Update(const int stagesCompleted, const int stagesTotal)
{
	std::scoped_lock<decltype(loadingLock)> lock(loadingLock);

	if (!IsShown())
	{
		if (force_stop)
			force_stop = false;
		if (force_drop)
			force_drop = false;
	}
        
	if (loadingProgressShape && !(force_stop||force_drop))
		loadingProgressShape->SetPos(stagesCompleted, stagesTotal);

	if (loadingProgress && !(force_stop||force_drop))
	{
		const float progress = float(stagesCompleted) / stagesTotal * loadingProgress->GetRange_max();
			loadingProgress->ForceSetProgressPos(progress);

		if (loadingProgressPercent && ps_rs_loading_stages)
		{
			string16 buf;
			xr_sprintf(buf, "%.0f%%", loadingProgress->GetProgressPos());
			loadingProgressPercent->TextItemControl()->SetText(buf);
		}
	}

	CUIWindow::Update();
	Draw();
}

void UILoadingScreen::ForceDrop()
{
	std::scoped_lock<decltype(loadingLock)> lock(loadingLock);

	force_drop = true;

	if (loadingProgressShape)
		loadingProgressShape->SetPos(0, 30);

	if (!loadingProgress)
		return;

	const float prev = loadingProgress->m_inertion;
	const float maximal = loadingProgress->GetRange_max();

	loadingProgress->m_inertion = 0.0f;
	loadingProgress->SetProgressPos(loadingProgress->GetRange_min());

	for (int i = 0; i < int(maximal); ++i)
	{
		loadingProgress->Update();
	}

	loadingProgress->m_inertion = prev;
}

void UILoadingScreen::ForceFinish()
{
	std::scoped_lock<decltype(loadingLock)> lock(loadingLock);

	force_stop = true;

	if (loadingProgressShape)
		loadingProgressShape->SetPos(30, 30);

	if (!loadingProgress)
		return;

	const float prev = loadingProgress->m_inertion;
	const float maximal = loadingProgress->GetRange_max();

	loadingProgress->m_inertion = 0.0f;
	loadingProgress->ForceSetProgressPos(maximal);

	for (int i = 0; i < int(maximal); ++i)
	{
		loadingProgress->Update();
	}

	loadingProgress->m_inertion = prev;
}

void UILoadingScreen::SetLevelLogo(const char* name)
{
	std::scoped_lock<decltype(loadingLock)> lock(loadingLock);

	if (loadingLogo)
		loadingLogo->InitTexture(name);
}

void UILoadingScreen::SetStageTitle(const char* title)
{
	std::scoped_lock<decltype(loadingLock)> lock(loadingLock);

	if (loadingStage)
		loadingStage->TextItemControl()->SetText(title);
}

void UILoadingScreen::SetStageTip(const char* header, const char* tipNumber, const char* tip)
{
	if (loadingHeader)
		loadingHeader->TextItemControl()->SetText(header);
	if (loadingTipNumber)
		loadingTipNumber->TextItemControl()->SetText(tipNumber);
	if (loadingTip)
		loadingTip->TextItemControl()->SetText(tip);
}


void UILoadingScreen::Show(bool status)
{
	CUIWindow::Show(status);
}

bool UILoadingScreen::IsShown()
{
	return CUIWindow::IsShown();
}
