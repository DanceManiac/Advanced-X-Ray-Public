#include "stdafx.h"
#include "pch_script.h"
#include "Actor.h"
#include "ActorCondition.h"
#include "embedded_editor_imgui_hud.h"
#include <imgui.h>

void ShowImguiHUD(bool& show)
{
	ImGui::SetNextWindowSize(ImVec2(100, 25), ImGuiCond_FirstUseEver);
	ShowHUD_ActorParams(show);
}

void ShowHUD_ActorParams(bool& show)
{
	/*if (!ImGui::Begin("", true, ImGuiWindowFlags_NoTitleBar))
	{
		ImGui::End();
		return;
	}*/

	ImGui::Begin("", &show, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);

	float max_health = Actor()->GetMaxHealth();
	float cur_health = Actor()->GetfHealth();
	float max_power = Actor()->conditions().GetMaxPower();
	float cur_power = Actor()->conditions().GetPower();

	ImGui::ProgressBar(cur_health, ImVec2(100.0f, 25.0f), "Health");
	ImGui::ProgressBar(cur_power, ImVec2(100.0f, 25.0f), "Stamina");
}