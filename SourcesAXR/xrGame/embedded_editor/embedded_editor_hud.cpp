#include "stdAfx.h"
#include "embedded_editor_hud.h"
#include "embedded_editor_helper.h"
//#include <addons/ImGuizmo/ImGuizmo.h>
#include "imgui_internal.h"

#include "../../xrEngine/device.h"
#include "../player_hud.h"
#include "../Weapon.h"
#include "../WeaponAttaches.h"
#include "../Actor.h"
#include "../Inventory.h"

void ShowHudEditor(bool& show)
{
	ImguiWnd wnd("HUD Editor", &show);
	if (wnd.Collapsed)
		return;

	if (!g_player_hud)
		return;

    ImGuiIO& io = ImGui::GetIO();
    //ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    //ImGuizmo::OPERATION mode = io.KeyCtrl ? ImGuizmo::ROTATE : ImGuizmo::TRANSLATE;
    bool showSeparator = true;
    auto item = g_player_hud->attached_item(0);
	auto Wpn = smart_cast<CWeapon*>(Actor()->inventory().ActiveItem());

	if (item)
	{
		if (showSeparator)
			ImGui::Separator();

		ImGui::Text("Item 0");
		ImGui::InputFloat3("hands_position 0", (float*)&item->m_measures.m_hands_attach[0]);
		ImGui::InputFloat3("hands_orientation 0", (float*)&item->m_measures.m_hands_attach[1]);
		ImGui::InputFloat3("item_position 0", (float*)&item->m_measures.m_item_attach[0]);
		ImGui::InputFloat3("item_orientation 0", (float*)&item->m_measures.m_item_attach[1]);
		ImGui::InputFloat3("aim_hud_offset_pos 0", (float*)&item->m_measures.m_hands_offset[0][1]);
		ImGui::InputFloat3("aim_hud_offset_rot 0", (float*)&item->m_measures.m_hands_offset[1][1]);
		ImGui::InputFloat3("gl_hud_offset_pos 0", (float*)&item->m_measures.m_hands_offset[0][2]);
		ImGui::InputFloat3("gl_hud_offset_rot 0", (float*)&item->m_measures.m_hands_offset[1][2]);
		ImGui::InputFloat3("aim_alt_hud_offset_pos 0", (float*)&item->m_measures.m_hands_offset[0][3]);
		ImGui::InputFloat3("aim_alt_hud_offset_rot 0", (float*)&item->m_measures.m_hands_offset[1][3]);
		ImGui::InputFloat3("fire_point 0", (float*)&item->m_measures.m_fire_point_offset[0]);
		ImGui::InputFloat3("fire_point2 0", (float*)&item->m_measures.m_fire_point2_offset[0]);
		ImGui::InputFloat3("shell_point 0", (float*)&item->m_measures.m_shell_point_offset[0]);

		for (int i = 0; i < Wpn->m_weapon_attaches.size(); i++)
		{
			auto mesh = Wpn->m_weapon_attaches[i];
			std::string pos_name = "attach_" + std::to_string(i + 1) + "_position";
			std::string orient_name = "attach_" + std::to_string(i + 1) + "_orientation";
			ImGui::InputFloat3(pos_name.c_str(), (float*)&mesh->hud_attach_pos[0]);
			ImGui::InputFloat3(orient_name.c_str(), (float*)&mesh->hud_attach_pos[1]);
		}

		/*ImGuizmo::Manipulate((float*)&Device.mView, (float*)&Device.mProject, mode, ImGuizmo::WORLD, (float*)&item->m_attach_offset);

		if (ImGuizmo::IsUsing())
		{
			Fvector ypr;
			item->m_attach_offset.getHPB(ypr.x, ypr.y, ypr.z);
			ypr.mul(180.f / PI);
			item->m_measures.m_hands_attach[1] = ypr;
			item->m_measures.m_hands_attach[0] = item->m_attach_offset.c;
		}*/
	}

	item = g_player_hud->attached_item(1);

	if (item)
	{
		if (showSeparator)
			ImGui::Separator();

		ImGui::Text("Item 1");
		ImGui::InputFloat3("hands_position 1", (float*)&item->m_measures.m_hands_attach[0][0]);
		ImGui::InputFloat3("hands_orientation 1", (float*)&item->m_measures.m_hands_attach[1][0]);
		ImGui::InputFloat3("item_position 1", (float*)&item->m_measures.m_item_attach[0]);
		ImGui::InputFloat3("item_orientation 1", (float*)&item->m_measures.m_item_attach[1]);
		ImGui::InputFloat3("fire_point 1", (float*)&item->m_measures.m_fire_point_offset[0]);
		ImGui::InputFloat3("fire_point2 1", (float*)&item->m_measures.m_fire_point2_offset[0]);
		ImGui::InputFloat3("shell_point 1", (float*)&item->m_measures.m_shell_point_offset[0]);

		/*ImGuizmo::Manipulate((float*)&Device.mView, (float*)&Device.mProject, mode, ImGuizmo::WORLD, (float*)&item->m_attach_offset);

		if (ImGuizmo::IsUsing())
		{
			Fvector ypr;
			item->m_attach_offset.getHPB(ypr.x, ypr.y, ypr.z);
			ypr.mul(180.f / PI);
			item->m_measures.m_hands_attach[1] = ypr;
			item->m_measures.m_hands_attach[0] = item->m_attach_offset.c;
		}*/
	}

	if (ImGui::Button("Save"))
	{
		g_player_hud->SaveCfg(0);
		g_player_hud->SaveCfg(1);
	}
}

bool HudEditor_MouseWheel(float wheel)
{
	ImGui::Begin("HUD Editor");

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