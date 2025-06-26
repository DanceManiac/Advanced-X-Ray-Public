////////////////////////////////////////////////////////////////////////////
//	Module 		: embedded_editor_pos_informer.cpp
//	Created 	: 20.11.2022
//  Modified 	: 23.11.2022
//	Author		: Dance Maniac (M.F.S. Team)
//	Description : ImGui Position Informer
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "pch_script.h"
#include "Actor.h"
#include "ai_object_location.h"
#include "embedded_editor_pos_informer.h"
#include "embedded_editor_helper.h"
#include <imgui.h>
#include "imgui_internal.h"
#include <fstream>

#include "string_table.h"

string256 section_name = "section";

void SavePosition(string256 sect)
{
	std::string userDir = FS.get_path("$app_data_root$")->m_Path, fileToWrite = "//position_info.txt";
	std::string path = userDir + fileToWrite;
	std::fstream informer;

	informer.open(path, std::fstream::app);

	if (informer.is_open())
	{
		informer << "[" << sect << "]" << "\n";
		informer << "position = " << Actor()->Position().x << ", " << Actor()->Position().y << ", " << Actor()->Position().z << "\n";
		informer << "direction = " << Actor()->Direction().x << ", " << Actor()->Direction().y << ", " << Actor()->Direction().z << "\n";
		informer << "game_vertex_id = " << Actor()->ai_location().game_vertex_id() << "\n";
		informer << "level_vertex_id = " << Actor()->ai_location().level_vertex_id() << "\n\n";
	}
}

void ShowPositionInformer(bool& show)
{
	ImguiWnd wnd(toUtf8(CStringTable().translate("st_editor_imgui_pos_informer").c_str()).c_str(), &show);

	if (wnd.Collapsed)
		return;

	Fvector actor_position = Actor()->Position();
	Fvector actor_direction = Actor()->Direction();
	int actor_game_vertex = Actor()->ai_location().game_vertex_id();
	int actor_level_vertex = Actor()->ai_location().level_vertex_id();

	ImGui::InputText("section name", (char*)&section_name, 256);
	ImGui::InputFloat3("position", (float*)&actor_position);
	ImGui::InputFloat3("direction", (float*)&actor_direction);
	ImGui::InputInt("game_vertex_id", (int*)&actor_game_vertex);
	ImGui::InputInt("level_vertex_id", (int*)&actor_level_vertex);

	if (ImGui::Button(toUtf8(CStringTable().translate("st_editor_imgui_save").c_str()).c_str()))
	{
		SavePosition(section_name);
	}
}

bool PositionInformer_MouseWheel(float wheel)
{
	ImGui::Begin(toUtf8(CStringTable().translate("st_editor_imgui_pos_informer").c_str()).c_str());

	if (!ImGui::IsWindowFocused())
	{
		ImGui::End();
		return false;
	}

	ImGuiWindow* window = ImGui::GetCurrentWindow();

	if (wheel != 0.0f)
	{
		float scroll{};
		scroll -= wheel * 35;
		ImGui::SetScrollY(window, window->Scroll.y - scroll);
	}

	ImGui::End();

	return true;
}