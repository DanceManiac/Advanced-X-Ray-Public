#include "stdAfx.h"
#include "embedded_editor_hud.h"
#include "../../xrEngine/device.h"
#include "../player_hud.h"
#include "embedded_editor_helper.h"
#include <addons/ImGuizmo/ImGuizmo.h>

void ShowHudEditor(bool& show)
{
    ImguiWnd wnd("HUD Editor", &show);
    if (wnd.Collapsed)
        return;

    if (!g_player_hud)
        return;

    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    ImGuizmo::OPERATION mode = io.KeyCtrl ? ImGuizmo::ROTATE : ImGuizmo::TRANSLATE;
    bool showSeparator = false;
    auto item = g_player_hud->attached_item(0);

/*	if (item)
	{
		if (showSeparator)
			ImGui::Separator();
		showSeparator = true;
		ImGui::Text("Item 0");
		ImGui::InputFloat3("item_position 0", (float*)&item->m_measures.m_hands_attach[0], 3);
		ImGui::InputFloat3("item_orientation 0", (float*)&item->m_measures.m_hands_attach[1], 3);
		ImGuizmo::Manipulate(
			(float*)&Device.mView, (float*)&Device.mProject, mode, ImGuizmo::WORLD, (float*)&item->m_attach_offset);
		Fvector ypr;
		item->m_attach_offset.getHPB(ypr.x, ypr.y, ypr.z);
		ypr.mul(180.f / PI);
		item->m_measures.m_hands_attach[1] = ypr;
		item->m_measures.m_hands_attach[0] = item->m_attach_offset.c;
	}*/

	if (item)
	{
		if (showSeparator)
			ImGui::Separator();
		ImGui::Text("Item 0");
		ImGui::InputFloat3("hands_position 0", (float*)&item->m_measures.m_hands_attach[0], 3);
		ImGui::InputFloat3("hands_orientation 0", (float*)&item->m_measures.m_hands_attach[1], 3);
		ImGuizmo::Manipulate(
			(float*)&Device.mView, (float*)&Device.mProject, mode, ImGuizmo::WORLD, (float*)&item->m_attach_offset);
		if (ImGuizmo::IsUsing()) 
		{
			Fvector ypr;
			item->m_attach_offset.getHPB(ypr.x, ypr.y, ypr.z);
			ypr.mul(180.f / PI);
			item->m_measures.m_hands_attach[1] = ypr;
			item->m_measures.m_hands_attach[0] = item->m_attach_offset.c;
		}
	}

    /*if (item) 
	{
        if (showSeparator)
            ImGui::Separator();
        showSeparator = true;
        ImGui::Text("Item 0");
        ImGui::InputFloat3("item_position 0", (float*)&item->m_measures.m_item_attach[0], 3);
        ImGui::InputFloat3("item_orientation 0", (float*)&item->m_measures.m_item_attach[1], 3);
        ImGuizmo::Manipulate(
            (float*)&Device.mView, (float*)&Device.mProject, mode, ImGuizmo::WORLD, (float*)&item->m_attach_offset);
        Fvector ypr;
        item->m_attach_offset.getHPB(ypr.x, ypr.y, ypr.z);
        ypr.mul(180.f / PI);
        item->m_measures.m_item_attach[1] = ypr;
        item->m_measures.m_item_attach[0] = item->m_attach_offset.c;
    }*/

    item = g_player_hud->attached_item(1);

	if (item)
	{
		if (showSeparator)
			ImGui::Separator();
		ImGui::Text("Item 1");
		ImGui::InputFloat3("hands_position 1", (float*)&item->m_measures.m_hands_attach[0], 3);
		ImGui::InputFloat3("hands_orientation 1", (float*)&item->m_measures.m_hands_attach[1], 3);
		ImGuizmo::Manipulate(
			(float*)&Device.mView, (float*)&Device.mProject, mode, ImGuizmo::WORLD, (float*)&item->m_attach_offset);
		if (ImGuizmo::IsUsing())
		{
			Fvector ypr;
			item->m_attach_offset.getHPB(ypr.x, ypr.y, ypr.z);
			ypr.mul(180.f / PI);
			item->m_measures.m_hands_attach[1] = ypr;
			item->m_measures.m_hands_attach[0] = item->m_attach_offset.c;
		}
	}

    /*if (item) 
	{
        if (showSeparator)
            ImGui::Separator();
        ImGui::Text("Item 1");
        ImGui::InputFloat3("item_position 1", (float*)&item->m_measures.m_item_attach[0], 3);
        ImGui::InputFloat3("item_orientation 1", (float*)&item->m_measures.m_item_attach[1], 3);
        ImGuizmo::Manipulate(
            (float*)&Device.mView, (float*)&Device.mProject, mode, ImGuizmo::WORLD, (float*)&item->m_item_transform);
        if (ImGuizmo::IsUsing()) 
		{
            Fvector ypr;
            item->m_attach_offset.getHPB(ypr.x, ypr.y, ypr.z);
            ypr.mul(180.f / PI);
            item->m_measures.m_item_attach[1] = ypr;
            item->m_measures.m_item_attach[0] = item->m_attach_offset.c;
        }
    }*/
}
