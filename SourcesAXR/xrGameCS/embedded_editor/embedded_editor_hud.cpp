////////////////////////////////////////////////////////////////////////////
//	Module 		: embedded_editor_hud.cpp
//	Created 	: 05.05.2021
//  Modified 	: 16.10.2025
//	Author		: Dance Maniac (M.F.S. Team)
//	Description : ImGui Hud Editor
////////////////////////////////////////////////////////////////////////////

#include "stdAfx.h"
#include "embedded_editor_hud.h"
#include "embedded_editor_helper.h"
#include "imgui_internal.h"

#include "../../xrEngine/device.h"
#include "../player_hud.h"
#include "../Weapon.h"
#include "../WeaponAttaches.h"
#include "../Actor.h"
#include "../Inventory.h"
#include "../CustomDetector.h"
#include "../Grenade.h"

#include "string_table.h"

void ShowHudEditor(bool& show)
{
	ImguiWnd wnd(toUtf8(CStringTable().translate("st_editor_imgui_hud").c_str()).c_str(), &show);
	if (wnd.Collapsed)
		return;

	if (!g_player_hud)
		return;

    ImGuiIO& io = ImGui::GetIO();
    bool showSeparator = true;
    auto item = g_player_hud->attached_item(0);
	auto Wpn = smart_cast<CWeapon*>(Actor()->inventory().ActiveItem());
	auto Grenade = smart_cast<CGrenade*>(Actor()->inventory().ActiveItem());

	static float drag_intensity = 0.0001f;

	ImGui::DragFloat(toUtf8(CStringTable().translate("st_editor_imgui_drag_intensity").c_str()).c_str(), &drag_intensity, 0.000001f, 0.000001f, 1.0f, "%.6f");

	if (item)
	{
		if (showSeparator)
			ImGui::Separator();

		ImGui::Text(Wpn ? toUtf8(Wpn->NameItem()).c_str() : Grenade ? toUtf8(Grenade->NameItem()).c_str() : toUtf8(CStringTable().translate("st_hud_editor_item_1").c_str()).c_str());
		ImGui::DragFloat3("hands_position 0",				(float*)&item->m_measures.m_hands_attach[0],		drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("hands_orientation 0",			(float*)&item->m_measures.m_hands_attach[1],		drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("item_position 0",				(float*)&item->m_measures.m_item_attach[0],			drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("item_orientation 0",				(float*)&item->m_measures.m_item_attach[1],			drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_pos 0",			(float*)&item->m_measures.m_hands_offset[0][1],		drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_rot 0",			(float*)&item->m_measures.m_hands_offset[1][1],		drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("gl_hud_offset_pos 0",			(float*)&item->m_measures.m_hands_offset[0][2],		drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("gl_hud_offset_rot 0",			(float*)&item->m_measures.m_hands_offset[1][2],		drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_alt_hud_offset_pos 0",		(float*)&item->m_measures.m_hands_offset[0][3],		drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_alt_hud_offset_rot 0",		(float*)&item->m_measures.m_hands_offset[1][3],		drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("fire_point 0",					(float*)&item->m_measures.m_fire_point_offset[0],	drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("fire_point2 0",					(float*)&item->m_measures.m_fire_point2_offset[0],	drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("shell_point 0",					(float*)&item->m_measures.m_shell_point_offset[0],	drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("overheating_smoke_point 0",		(float*)&item->m_measures.m_overheating_smoke_offset[0], drag_intensity, NULL, NULL, "%.6f");

		if (READ_IF_EXISTS(pSettings, r_bool, item->m_sect_name, "hud_collision_enabled", false))
		{
			ImGui::DragFloat3("hud_collision_offset_pos 0",	(float*)&item->m_measures.m_collision_offset[0],	drag_intensity, NULL, NULL, "%.6f");
			ImGui::DragFloat3("hud_collision_offset_rot 0",	(float*)&item->m_measures.m_collision_offset[1],	drag_intensity, NULL, NULL, "%.6f");
		}

		if (Wpn)
		{
			// Laser light offsets
			if (Wpn->LaserAttachable())
			{
				ImGui::DragFloat3("laserdot_attach_offset 0",	(float*)&Wpn->laserdot_attach_offset, drag_intensity, NULL, NULL, "%.6f");
				ImGui::DragFloat3("laserdot_attach_rot 0",		(float*)&Wpn->laserdot_attach_rot, drag_intensity, NULL, NULL, "%.6f");
			}

			//Torch light offsets
			if (Wpn->TacticalTorchAttachable())
			{
				ImGui::DragFloat3("torch_attach_offset 0",		(float*)&Wpn->flashlight_attach_offset, drag_intensity, NULL, NULL, "%.6f");
				ImGui::DragFloat3("torch_omni_attach_offset 0", (float*)&Wpn->flashlight_omni_attach_offset, drag_intensity, NULL, NULL, "%.6f");
				ImGui::DragFloat3("torch_attach_rot 0",			(float*)&Wpn->flashlight_attach_rot, drag_intensity, NULL, NULL, "%.6f");
			}

			for (int i = 0; i < Wpn->m_weapon_attaches.size(); i++)
			{
				auto mesh = Wpn->m_weapon_attaches[i];

				string256 pos_name, orient_name;
				strconcat(sizeof(pos_name), pos_name, mesh->m_section.c_str(), "_position");
				strconcat(sizeof(orient_name), orient_name, mesh->m_section.c_str(), "_orientation");

				ImGui::DragFloat3(pos_name,			(float*)&mesh->hud_attach_pos[0],		drag_intensity, NULL, NULL, "%.6f");
				ImGui::DragFloat3(orient_name,		(float*)&mesh->hud_attach_pos[1],		drag_intensity, NULL, NULL, "%.6f");
			}
		}
	}

	item = g_player_hud->attached_item(1);
	auto Det = smart_cast<CCustomDetector*>(Actor()->inventory().ItemFromSlot(DETECTOR_SLOT));

	if (item)
	{
		if (showSeparator)
			ImGui::Separator();

		ImGui::Text(Det ? toUtf8(Det->NameItem()).c_str() : toUtf8(CStringTable().translate("st_hud_editor_item_2").c_str()).c_str());
		ImGui::DragFloat3("hands_position 1",		(float*)&item->m_measures.m_hands_attach[0][0],		drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("hands_orientation 1",	(float*)&item->m_measures.m_hands_attach[1][0],		drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("item_position 1",		(float*)&item->m_measures.m_item_attach[0],			drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("item_orientation 1",		(float*)&item->m_measures.m_item_attach[1],			drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("fire_point 1",			(float*)&item->m_measures.m_fire_point_offset[0],	drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("fire_point2 1",			(float*)&item->m_measures.m_fire_point2_offset[0],	drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("shell_point 1",			(float*)&item->m_measures.m_shell_point_offset[0],	drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("overheating_smoke_point 1", (float*)&item->m_measures.m_overheating_smoke_offset[0], drag_intensity, NULL, NULL, "%.6f");
	
		if (READ_IF_EXISTS(pSettings, r_bool, item->m_sect_name, "hud_collision_enabled", false))
		{
			ImGui::DragFloat3("hud_collision_offset_pos 1", (float*)&item->m_measures.m_collision_offset[0], drag_intensity, NULL, NULL, "%.6f");
			ImGui::DragFloat3("hud_collision_offset_rot 1", (float*)&item->m_measures.m_collision_offset[1], drag_intensity, NULL, NULL, "%.6f");
		}
	}

	if (ImGui::Button(toUtf8(CStringTable().translate("st_editor_imgui_save").c_str()).c_str()))
	{
		g_player_hud->SaveCfg(0);
		g_player_hud->SaveCfg(1);
	}
}

bool HudEditor_MouseWheel(float wheel)
{
	ImGui::Begin(toUtf8(CStringTable().translate("st_editor_imgui_hud").c_str()).c_str());

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