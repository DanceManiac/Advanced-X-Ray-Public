#include "stdafx.h"
#include "pch_script.h"
#include "Actor.h"
#include "ai_object_location.h"
#include "embedded_editor_pos_informer.h"
#include "embedded_editor_helper.h"
#include <imgui.h>

string256 section_name = "section";

void SavePosition(string256 sect)
{
    CInifile f("position_info", false, true, true);
    string_path section{};
    if (f.section_exist(sect))
    {
        int sect_id = 2;
        strconcat(sizeof(section), section, sect, "_", std::to_string(sect_id).c_str());
        while (f.section_exist(section))
        {
            delete_data(section);
            sect_id++;
            strconcat(sizeof(section), section, sect, "_", std::to_string(sect_id).c_str());
        }
    }
    else
        strconcat(sizeof(section), section, sect, "");

    f.w_fvector3(section, "position", Actor()->Position());
    f.w_fvector3(section, "direction", Actor()->Direction());
    f.w_u32(section, "game_vertex_id", Actor()->ai_location().game_vertex_id());
    f.w_u32(section, "level_vertex_id", Actor()->ai_location().level_vertex_id());

    string_path fileName;
    FS.update_path(fileName, "$app_data_root$", "position_info");
    strconcat(sizeof(fileName), fileName, fileName, ".txt");
    f.save_as(fileName);
}

void ShowPositionInformer(bool& show)
{
	ImguiWnd wnd("Position Informer", &show);

	if (wnd.Collapsed)
		return;

	Fvector actor_position = Actor()->Position();
	Fvector actor_direction = Actor()->Direction();
	int actor_game_vertex = Actor()->ai_location().game_vertex_id();
	int actor_level_vertex = Actor()->ai_location().level_vertex_id();

	ImGui::InputText("section name", (char*)&section_name, 256);
	ImGui::InputFloat3("actor_position", (float*)&actor_position);
	ImGui::InputFloat3("actor_direction", (float*)&actor_direction);
	ImGui::InputInt("actor_game_vertex_id", (int*)&actor_game_vertex);
	ImGui::InputInt("actor_level_vertex_id", (int*)&actor_level_vertex);

	if (ImGui::Button("Save")) 
	{
		SavePosition(section_name);
	}
}