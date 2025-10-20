////////////////////////////////////////////////////////////////////////////
//	Module 		: embedded_editor_hud.cpp
//	Created 	: 05.05.2021
//  Modified 	: 20.10.2025
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

#ifdef DEBUG
#	include "debug_renderer.h"

void DrawLightCone(Fvector position, Fvector direction, float range, float angle, u32 color)
{
	Fmatrix cone_transform;
	cone_transform.identity();
	cone_transform.c = position;

	Fvector up = { 0, 1, 0 };

	if (_abs(direction.y) > 0.9f)
		up.set(1, 0, 0);

	Fvector right;
	right.crossproduct(direction, up).normalize();
	up.crossproduct(right, direction).normalize();

	cone_transform.i = right;
	cone_transform.j = up;
	cone_transform.k = direction;

	Level().debug_renderer().draw_cone(cone_transform, range, angle, color, true);
}

extern ENGINE_API float psHUD_FOV;

void DrawPointText(Fvector position, const char* text, u32 color = color_rgba(255, 255, 255, 255), float height_offset = 0.02f)
{
	Fvector text_pos = position;
	text_pos.y += height_offset;

	Fvector4 v_res;

	Device.mProject.build_projection(psHUD_FOV, Device.fASPECT, HUD_VIEWPORT_NEAR, g_pGamePersistent->Environment().CurrentEnv->far_plane);

	Fmatrix mFullTransform;
	mFullTransform.mul(Device.mProject, Device.mView);

	mFullTransform.transform(v_res, text_pos);

	if (v_res.z < 0 || v_res.w < 0)
		return;

	if (v_res.x < -1.f || v_res.x > 1.f || v_res.y < -1.f || v_res.y > 1.f)
		return;

	float x = (1.f + v_res.x) / 2.f * (Device.dwWidth);
	float y = (1.f - v_res.y) / 2.f * (Device.dwHeight);

	UI().Font().pFontMedium->SetAligment(CGameFont::alCenter);
	UI().Font().pFontMedium->SetColor(color);
	UI().Font().pFontMedium->OutSet(x, y);
	UI().Font().pFontMedium->OutNext(text);
}
#endif

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

		ImGui::Text(toUtf8(CStringTable().translate("st_hud_editor_hud").c_str()).c_str());
		ImGui::DragFloat3("hands_position 0", (float*)&item->m_measures.m_hands_attach[0], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("hands_orientation 0", (float*)&item->m_measures.m_hands_attach[1], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("hands_position_grip_h 0", (float*)&item->m_measures.m_hands_attach[0], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("hands_orientation_grip_h 0", (float*)&item->m_measures.m_hands_attach[1], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("hands_position_grip_v 0", (float*)&item->m_measures.m_hands_attach[0], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("hands_orientation_grip_v 0", (float*)&item->m_measures.m_hands_attach[1], drag_intensity, NULL, NULL, "%.6f");

		ImGui::DragFloat3("item_position 0", (float*)&item->m_measures.m_item_attach[0], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("item_orientation 0", (float*)&item->m_measures.m_item_attach[1], drag_intensity, NULL, NULL, "%.6f");

		ImGui::Text(toUtf8(CStringTable().translate("st_hud_editor_hud_aim").c_str()).c_str());
		ImGui::DragFloat3("aim_hud_offset_pos 0", (float*)&item->m_measures.m_hands_offset[0][1], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_rot 0", (float*)&item->m_measures.m_hands_offset[1][1], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_pos_grip_h 0", (float*)&item->m_measures.m_hands_offset[0][1], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_rot_grip_h 0", (float*)&item->m_measures.m_hands_offset[1][1], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_pos_grip_v 0", (float*)&item->m_measures.m_hands_offset[0][1], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_rot_grip_v 0", (float*)&item->m_measures.m_hands_offset[1][1], drag_intensity, NULL, NULL, "%.6f");

		ImGui::Text(toUtf8(CStringTable().translate("st_hud_editor_hud_aim_alt").c_str()).c_str());
		ImGui::DragFloat3("aim_hud_offset_alt_pos 0", (float*)&item->m_measures.m_hands_offset[0][1], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_alt_rot 0", (float*)&item->m_measures.m_hands_offset[1][1], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_alt_pos 0", (float*)&item->m_measures.m_hands_offset[0][3], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_alt_rot 0", (float*)&item->m_measures.m_hands_offset[1][3], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_grip_h_alt_pos 0", (float*)&item->m_measures.m_hands_offset[0][1], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_grip_h_alt_rot 0", (float*)&item->m_measures.m_hands_offset[1][1], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_grip_h_alt_pos 0", (float*)&item->m_measures.m_hands_offset[0][3], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_grip_h_alt_rot 0", (float*)&item->m_measures.m_hands_offset[1][3], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_grip_v_alt_pos 0", (float*)&item->m_measures.m_hands_offset[0][1], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_grip_v_alt_rot 0", (float*)&item->m_measures.m_hands_offset[1][1], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_grip_v_alt_pos 0", (float*)&item->m_measures.m_hands_offset[0][3], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_grip_v_alt_rot 0", (float*)&item->m_measures.m_hands_offset[1][3], drag_intensity, NULL, NULL, "%.6f");

		ImGui::Text(toUtf8(CStringTable().translate("st_hud_editor_hud_scope_lfo").c_str()).c_str());
		ImGui::DragFloat3("aim_hud_offset_3d_pos 0", (float*)&item->m_measures.m_hands_offset[0][1], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_3d_rot 0", (float*)&item->m_measures.m_hands_offset[1][1], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_3d_pos 0", (float*)&item->m_measures.m_hands_offset[0][3], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_3d_rot 0", (float*)&item->m_measures.m_hands_offset[1][3], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_grip_h_3d_pos 0", (float*)&item->m_measures.m_hands_offset[0][1], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_grip_h_3d_rot 0", (float*)&item->m_measures.m_hands_offset[1][1], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_grip_h_3d_pos 0", (float*)&item->m_measures.m_hands_offset[0][3], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_grip_h_3d_rot 0", (float*)&item->m_measures.m_hands_offset[1][3], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_grip_v_3d_pos 0", (float*)&item->m_measures.m_hands_offset[0][1], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_grip_v_3d_rot 0", (float*)&item->m_measures.m_hands_offset[1][1], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_grip_v_3d_pos 0", (float*)&item->m_measures.m_hands_offset[0][3], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_grip_v_3d_rot 0", (float*)&item->m_measures.m_hands_offset[1][3], drag_intensity, NULL, NULL, "%.6f");

		ImGui::Text(toUtf8(CStringTable().translate("st_hud_editor_hud_scope_lfo_alt").c_str()).c_str());
		ImGui::DragFloat3("aim_hud_offset_3d_alt_pos 0", (float*)&item->m_measures.m_hands_offset[0][1], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_3d_alt_rot 0", (float*)&item->m_measures.m_hands_offset[1][1], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_3d_alt_pos 0", (float*)&item->m_measures.m_hands_offset[0][3], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_3d_alt_rot 0", (float*)&item->m_measures.m_hands_offset[1][3], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_grip_h_3d_alt_pos 0", (float*)&item->m_measures.m_hands_offset[0][1], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_grip_h_3d_alt_rot 0", (float*)&item->m_measures.m_hands_offset[1][1], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_grip_h_3d_alt_pos 0", (float*)&item->m_measures.m_hands_offset[0][3], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_grip_h_3d_alt_rot 0", (float*)&item->m_measures.m_hands_offset[1][3], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_grip_v_3d_alt_pos 0", (float*)&item->m_measures.m_hands_offset[0][1], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_grip_v_3d_alt_rot 0", (float*)&item->m_measures.m_hands_offset[1][1], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_grip_v_3d_alt_pos 0", (float*)&item->m_measures.m_hands_offset[0][3], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("aim_hud_offset_grip_v_3d_alt_rot 0", (float*)&item->m_measures.m_hands_offset[1][3], drag_intensity, NULL, NULL, "%.6f");

		ImGui::Text(toUtf8(CStringTable().translate("st_hud_editor_hud_gl").c_str()).c_str());
		ImGui::DragFloat3("gl_hud_offset_pos 0", (float*)&item->m_measures.m_hands_offset[0][2], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("gl_hud_offset_rot 0", (float*)&item->m_measures.m_hands_offset[1][2], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("gl_hud_offset_grip_h_pos 0", (float*)&item->m_measures.m_hands_offset[0][2], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("gl_hud_offset_grip_h_rot 0", (float*)&item->m_measures.m_hands_offset[1][2], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("gl_hud_offset_grip_v_pos 0", (float*)&item->m_measures.m_hands_offset[0][2], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("gl_hud_offset_grip_v_rot 0", (float*)&item->m_measures.m_hands_offset[1][2], drag_intensity, NULL, NULL, "%.6f");

		ImGui::Text(toUtf8(CStringTable().translate("st_hud_editor_hud_particles").c_str()).c_str());
		ImGui::DragFloat3("fire_point 0", (float*)&item->m_measures.m_fire_point_offset[0], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("fire_point2 0", (float*)&item->m_measures.m_fire_point2_offset[0], drag_intensity, NULL, NULL, "%.6f");
		ImGui::DragFloat3("shell_point 0", (float*)&item->m_measures.m_shell_point_offset[0], drag_intensity, NULL, NULL, "%.6f");

		if (READ_IF_EXISTS(pSettings, r_bool, item->m_sect_name, "hud_collision_enabled", false))
		{
			ImGui::Text(toUtf8(CStringTable().translate("st_hud_editor_hud_collision").c_str()).c_str());

			ImGui::DragFloat3("hud_collision_offset_pos 0",	(float*)&item->m_measures.m_collision_offset[0],	drag_intensity, NULL, NULL, "%.6f");
			ImGui::DragFloat3("hud_collision_offset_rot 0",	(float*)&item->m_measures.m_collision_offset[1],	drag_intensity, NULL, NULL, "%.6f");
		}

#ifdef DEBUG
		firedeps fd;
		item->setup_firedeps(fd);

		DrawPointText(fd.vLastFP, "fire_point", color_rgba(255, 255, 0, 255));
		Level().debug_renderer().draw_aabb(fd.vLastFP, 0.002f, 0.002f, 0.002f, color_rgba(0, 255, 0, 255), true);

		DrawPointText(fd.vLastFP2, "fire_point2", color_rgba(255, 255, 0, 255));
		Level().debug_renderer().draw_aabb(fd.vLastFP2, 0.002f, 0.002f, 0.002f, color_rgba(0, 255, 0, 255), true);

		DrawPointText(fd.vLastSP, "shell_point", color_rgba(255, 255, 0, 255));
		Level().debug_renderer().draw_aabb(fd.vLastSP, 0.002f, 0.002f, 0.002f, color_rgba(0, 255, 0, 255), true);

		DrawPointText(fd.vLastOSP, "overheating_smoke_point", color_rgba(255, 255, 0, 255));
		Level().debug_renderer().draw_aabb(fd.vLastOSP, 0.002f, 0.002f, 0.002f, color_rgba(0, 255, 0, 255), true);
#endif

		if (Wpn)
		{
			ImGui::DragFloat3("overheat_attach_offset 0", (float*)&Wpn->overheat_attach_offset, drag_intensity, NULL, NULL, "%.6f");
			ImGui::DragFloat3("overheat_omni_attach_offset 0", (float*)&Wpn->overheat_omni_attach_offset, drag_intensity, NULL, NULL, "%.6f");

			ImGui::Text(toUtf8(CStringTable().translate("st_hud_editor_attachments").c_str()).c_str());

			// Laser light offsets
			if (Wpn->LaserAttachable())
			{
				ImGui::DragFloat3("laserdot_attach_offset 0",	(float*)&Wpn->laserdot_attach_offset, drag_intensity, NULL, NULL, "%.6f");
				ImGui::DragFloat3("laserdot_attach_rot 0",		(float*)&Wpn->laserdot_attach_rot, drag_intensity, NULL, NULL, "%.6f");

#ifdef DEBUG
				if (Wpn->IsLaserOn())
				{
					Fvector laser_pos = Wpn->get_LastFP(), laser_dir = Wpn->get_LastFD();
					Wpn->GetBoneOffsetPosDir(Wpn->laserdot_attach_bone, laser_pos, laser_dir, Wpn->laserdot_attach_offset, Wpn->laserdot_attach_rot);
					Wpn->CorrectDirFromWorldToHud(laser_dir);

					DrawPointText(laser_pos, "laserdot_attach_offset", color_rgba(255, 255, 0, 255));
					Level().debug_renderer().draw_aabb(laser_pos, 0.002f, 0.002f, 0.002f, color_rgba(0, 255, 0, 255), true);
					DrawLightCone(laser_pos, laser_dir, 0.5f, 0.002f, color_rgba(255, 0, 0, 255));
				}
#endif
			}

			//Torch light offsets
			if (Wpn->TacticalTorchAttachable())
			{
				ImGui::DragFloat3("torch_attach_offset 0",		(float*)&Wpn->flashlight_attach_offset, drag_intensity, NULL, NULL, "%.6f");
				ImGui::DragFloat3("torch_omni_attach_offset 0", (float*)&Wpn->flashlight_omni_attach_offset, drag_intensity, NULL, NULL, "%.6f");
				ImGui::DragFloat3("torch_attach_rot 0",			(float*)&Wpn->flashlight_attach_rot, drag_intensity, NULL, NULL, "%.6f");

#ifdef DEBUG
				if (Wpn->IsFlashlightOn())
				{
					Fvector flashlight_pos = Wpn->get_LastFP(), flashlight_dir = Wpn->get_LastFD();
					Wpn->GetBoneOffsetPosDir(Wpn->flashlight_attach_bone, flashlight_pos, flashlight_dir, Wpn->flashlight_attach_offset, Wpn->flashlight_attach_rot);
					Wpn->CorrectDirFromWorldToHud(flashlight_dir);

					DrawPointText(flashlight_pos, "torch_attach_offset", color_rgba(255, 255, 0, 255));
					Level().debug_renderer().draw_aabb(flashlight_pos, 0.002f, 0.002f, 0.002f, color_rgba(0, 255, 0, 255), true);
					DrawLightCone(flashlight_pos, flashlight_dir, 0.5f, 0.1f, color_rgba(255, 255, 255, 255));
				}
#endif
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
			ImGui::Text(toUtf8(CStringTable().translate("st_hud_editor_hud_collision").c_str()).c_str());
			ImGui::DragFloat3("hud_collision_offset_pos 1", (float*)&item->m_measures.m_collision_offset[0], drag_intensity, NULL, NULL, "%.6f");
			ImGui::DragFloat3("hud_collision_offset_rot 1", (float*)&item->m_measures.m_collision_offset[1], drag_intensity, NULL, NULL, "%.6f");
		}

#ifdef DEBUG
		if (Det)
		{
			if (Det->m_bLightsEnabled)
			{
				firedeps fd;
				item->setup_firedeps(fd);

				DrawPointText(fd.vLastFP, "fire_point", color_rgba(255, 255, 0, 255));
				Level().debug_renderer().draw_aabb(fd.vLastFP, 0.002f, 0.002f, 0.002f, color_rgba(0, 255, 0, 255), true);

				DrawPointText(fd.vLastFP2, "fire_point2", color_rgba(255, 255, 0, 255));
				Level().debug_renderer().draw_aabb(fd.vLastFP2, 0.002f, 0.002f, 0.002f, color_rgba(0, 255, 0, 255), true);

				DrawPointText(fd.vLastSP, "shell_point", color_rgba(255, 255, 0, 255));
				Level().debug_renderer().draw_aabb(fd.vLastSP, 0.002f, 0.002f, 0.002f, color_rgba(0, 255, 0, 255), true);

				DrawPointText(fd.vLastOSP, "overheating_smoke_point", color_rgba(255, 255, 0, 255));
				Level().debug_renderer().draw_aabb(fd.vLastOSP, 0.002f, 0.002f, 0.002f, color_rgba(0, 255, 0, 255), true);
			}
		}
#endif
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