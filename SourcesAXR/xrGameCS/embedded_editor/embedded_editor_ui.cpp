#include "stdAfx.h"
#include "embedded_editor_ui.h"
#include "embedded_editor_helper.h"
#include "imgui_internal.h"
#include "../../xrEngine/device.h"
#include "ui_base.h"
#include "string_table.h"

CUIDebuggable::~CUIDebuggable()
{
    UnregisterDebuggable();
}

void CUIDebuggable::RegisterDebuggable()
{
    UI().Debugger().Register(this);
}

void CUIDebuggable::UnregisterDebuggable()
{
    UI().Debugger().Unregister(this);
}

void CUIDebugger::Register(CUIDebuggable* debuggable)
{
#ifndef MASTER_GOLD
    m_root_windows.emplace_back(debuggable);
#endif
}

void CUIDebugger::Unregister(CUIDebuggable* debuggable)
{
#ifndef MASTER_GOLD
    const auto it = std::find(m_root_windows.begin(), m_root_windows.end(), debuggable);
    if (it != m_root_windows.end())
        m_root_windows.erase(it);
#endif
}

void CUIDebugger::SetSelected(CUIDebuggable* debuggable)
{
    m_state.selected = debuggable;
    m_state.newSelected = debuggable;
}

CUIDebugger::CUIDebugger()
{
    ImGui::SetAllocatorFunctions(
        [](size_t size, void* /*user_data*/)
        {
            return xr_malloc(size);
        },
        [](void* ptr, void* /*user_data*/)
        {
            xr_free(ptr);
        }
        );
    //ImGui::SetCurrentContext(Device.editor().GetImGuiContext());
}

void ShowUIEditor(bool& show)
{
#ifndef MASTER_GOLD
	ImguiWnd wnd(toUtf8(CStringTable().translate("st_editor_imgui_ui_debugger").c_str()).c_str(), &show);
	
	if (wnd.Collapsed)
		return;

	ImGuiIO& io = ImGui::GetIO();
    static auto m_debugger = UI().Debugger();

    if (ImGui::Begin(toUtf8(CStringTable().translate("st_editor_imgui_ui_debugger").c_str()).c_str(), &show))
    {
        ImGui::Checkbox(toUtf8(CStringTable().translate("st_editor_imgui_ui_draw_rects").c_str()).c_str(), &m_debugger.m_state.drawWndRects);
        ImGui::BeginDisabled(!m_debugger.m_state.drawWndRects);
        ImGui::Checkbox(toUtf8(CStringTable().translate("st_editor_imgui_ui_colored_rects").c_str()).c_str(), &m_debugger.m_state.coloredRects);
        ImGui::EndDisabled();

        constexpr ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
            ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingFixedFit |
            ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY;

        if (ImGui::BeginTable(toUtf8(CStringTable().translate("st_editor_imgui_ui_tree_and_props").c_str()).c_str(), 2, flags))
        {
            ImGui::TableSetupColumn(toUtf8(CStringTable().translate("st_editor_imgui_ui_tree").c_str()).c_str());
            ImGui::TableSetupColumn(toUtf8(CStringTable().translate("st_editor_imgui_ui_selected_props").c_str()).c_str(), ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();
            ImGui::TableNextColumn();

            for (const auto& window : m_debugger.m_root_windows)
            {
                window->FillDebugTree(m_debugger.m_state);
                if (m_debugger.m_state.selected != m_debugger.m_state.newSelected)
                    m_debugger.m_state.selected = m_debugger.m_state.newSelected;
            }

            ImGui::TableNextColumn();

            if (m_debugger.m_state.selected)
                m_debugger.m_state.selected->FillDebugInfo();

            ImGui::EndTable();
        }
    }

    ImGui::End();
#endif
}

bool UIEditor_MouseWheel(float wheel)
{
    ImGui::Begin(toUtf8(CStringTable().translate("st_editor_imgui_ui_debugger").c_str()).c_str());

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