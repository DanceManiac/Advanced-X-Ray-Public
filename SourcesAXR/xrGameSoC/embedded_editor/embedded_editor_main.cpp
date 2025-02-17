#include "stdAfx.h"
#include "embedded_editor_main.h"
#include "../../xrEngine/xr_input.h"
#include "../../xrEngine/x_ray.h"
#include "../xr_level_controller.h"
#include "embedded_editor_helper.h"
#include "embedded_editor_weather.h"
#include "embedded_editor_hud.h"
#include "embedded_editor_person_attach.h"
#include "embedded_editor_pos_informer.h"
#include "embedded_editor_spawner.h"
#include "embedded_editor_ui.h"
#include "../../build_config_defines.h"
#include <addons/imguinodegrapheditor/imguinodegrapheditor.h>
#include <dinput.h>
#include <imgui.h>
#include "imgui_internal.h"

#include "string_table.h"

bool bShowWindow = true;
bool show_test_window = true;
bool show_weather_window = false;
//bool show_hud_editor = false;      // Требует адаптации
bool show_person_attach_editor = false;
bool show_position_informer = false;
bool show_spawn_menu = false;
bool show_ui_editor = false;
/*bool show_info_window = false;
bool show_prop_window = false;
bool show_restr_window = false;
bool show_shader_window = false;
bool show_occ_window = false;
bool show_node_editor = false;*/

static bool isAlt = false;
static bool sections_list_created = false;

enum class EditorStage {
    None,
    Light,
    Full,

    Count,
};
EditorStage stage = EditorStage::None;

bool IsEditorActive() { return stage == EditorStage::Full || (stage == EditorStage::Light && isAlt); }

bool IsEditor() { return stage != EditorStage::None; }

bool isRenderAiMap = false;
bool isRenderSpawnElement = false;

void ShowMain()
{
	ImguiWnd wnd(toUtf8(CStringTable().translate("st_editor_imgui_name").c_str()).c_str());

	if (wnd.Collapsed)
		return;

	ImGui::Text(toUtf8(CStringTable().translate("st_editor_imgui_name").c_str()).c_str());
	if (ImGui::Button(toUtf8(CStringTable().translate("st_editor_imgui_test_wnd").c_str()).c_str()))
		show_test_window ^= 1;
/*	if (ImGui::Button("Test Node Editor"))
		show_node_editor ^= 1;*/
	if (ImGui::Button(toUtf8(CStringTable().translate("st_editor_imgui_weather").c_str()).c_str()))
		show_weather_window ^= 1;
    if (ImGui::Button(toUtf8(CStringTable().translate("st_editor_imgui_pos_informer").c_str()).c_str()))
        show_position_informer = !show_position_informer;
	//if (ImGui::Button(toUtf8(CStringTable().translate("st_editor_imgui_hud").c_str()).c_str()))    // Требует адаптации
	//	show_hud_editor = !show_hud_editor;
    if (ImGui::Button(toUtf8(CStringTable().translate("st_editor_imgui_person_attach").c_str()).c_str()))
        show_person_attach_editor = !show_person_attach_editor;
    if (ImGui::Button(toUtf8(CStringTable().translate("st_editor_imgui_spawn").c_str()).c_str()))
        show_spawn_menu = !show_spawn_menu;
    if (ImGui::Button(toUtf8(CStringTable().translate("st_editor_imgui_ui_debugger").c_str()).c_str()))
        show_ui_editor = !show_ui_editor;

    if (psDeviceFlags.test(rsR1))
    {
        ImGui::Separator();
        ImGui::Text(toUtf8(CStringTable().translate("st_editor_imgui_level").c_str()).c_str());
        ImGui::Checkbox(toUtf8(CStringTable().translate("st_editor_imgui_draw_ai_map").c_str()).c_str(), &isRenderAiMap);
        ImGui::Checkbox(toUtf8(CStringTable().translate("st_editor_imgui_draw_spawn_element").c_str()).c_str(), &isRenderSpawnElement);
        ImGui::Separator();
    }

	bool full = stage == EditorStage::Full;
	if (ImGui::Checkbox(toUtf8(CStringTable().translate("st_editor_imgui_active").c_str()).c_str(), &full))
		stage = full ? EditorStage::Full : EditorStage::Light;
	ImGui::Text(
		"Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}

void ShowEditor()
{
    if (!IsEditor())
        return;

	ShowMain();

    if (show_test_window) {
        ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
        ImGui::ShowDemoWindow(&show_test_window);
    }
/*    if (show_node_editor) {
        ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Example: Custom Node Graph", &show_node_editor)) {
            ImGui::TestNodeGraphEditor();
        }
        ImGui::End();
    }*/
    if (show_weather_window)
        ShowWeatherEditor(show_weather_window);
    if (show_position_informer)
        ShowPositionInformer(show_position_informer);
	//if (show_hud_editor)   // Требует адаптации
	//	ShowHudEditor(show_hud_editor);
    if (show_person_attach_editor)
        ShowPersonAttachEditor(show_person_attach_editor);
    if (show_spawn_menu)
    {
        if (!sections_list_created)
        {
            FillSectionsList();
            sections_list_created = true;
        }
        ShowSpawner(show_spawn_menu);
    }
    if (show_ui_editor)
        ShowUIEditor(show_ui_editor);
    /*if (show_prop_window)
        ShowPropEditor(show_prop_window);
	if (show_lua_binder)
		ShowLuaBinder(show_lua_binder);
	if (show_logic_editor)
		ShowLogicEditor(show_logic_editor);*/
}

bool isRControl = false, isLControl = false, isRShift = false, isLShift = false;
bool Editor_KeyPress(int key)
{
	if (bDeveloperMode)
	{
		if (key == DIK_F10)
		{
			stage = static_cast<EditorStage>((static_cast<int>(stage) + 1) % static_cast<int>(EditorStage::Count));
		}
		else if (key == DIK_RALT || key == DIK_LALT)
			isAlt = true;
	}

    if (!IsEditorActive())
        return false;

    ImGuiIO& io = ImGui::GetIO();
    io.MouseDrawCursor = true;

    switch (key) {
    case DIK_RALT:
    case DIK_LALT:
    case DIK_F10:
        break;
    case DIK_RCONTROL:
        isRControl = true;
        io.KeyCtrl = true;
        break;
    case DIK_LCONTROL:
        isLControl = true;
        io.KeyCtrl = true;
        break;
    case DIK_RSHIFT:
        isRShift = true;
        io.KeyShift = true;
        break;
    case DIK_LSHIFT:
        isLShift = true;
        io.KeyShift = true;
        break;
    case MOUSE_1:
        io.MouseDown[0] = true;
        break;
    case MOUSE_2:
        io.MouseDown[1] = true;
        break;
    case MOUSE_3:
        io.MouseDown[2] = true;
        break;
	case DIK_NUMPAD0:
	case DIK_NUMPAD1:
	case DIK_NUMPAD2:
	case DIK_NUMPAD3:
	case DIK_NUMPAD4:
	case DIK_NUMPAD5:
	case DIK_NUMPAD6:
	case DIK_NUMPAD7:
	case DIK_NUMPAD8:
	case DIK_NUMPAD9: io.AddInputCharacter(unsigned int('0' + key - DIK_NUMPAD0));
		break;
    default: 
        if (key < 512)
            io.KeysDown[key] = true;
		if (key == DIK_SPACE && (pInput->iGetAsyncKeyState(DIK_RWIN) || pInput->iGetAsyncKeyState(DIK_LWIN)))
			ActivateKeyboardLayout((HKL)HKL_NEXT, 0);
		else {
			uint16_t ch[1];
			int n = pInput->scancodeToChar(key, ch);
			if (n > 0) {
				wchar_t buf;
				MultiByteToWideChar(CP_ACP, 0, (char*)ch, n, &buf, 1);
				io.AddInputCharacter(buf);
				//char utf8[3];
				//int c = WideCharToMultiByte(CP_UTF8, 0, &buf, 1, utf8, 3, nullptr, nullptr);
				//utf8[c] = '\0';
				//io.AddInputCharactersUTF8(utf8);
			}
		}
    }
    return true;
}

bool Editor_KeyRelease(int key)
{
    if (key == DIK_RALT || key == DIK_LALT)
        isAlt = false;
    bool active = IsEditorActive();

    ImGuiIO& io = ImGui::GetIO();
    if (!active)
        io.MouseDrawCursor = false;

    switch (key) {
    case DIK_RCONTROL:
        isRControl = false;
        io.KeyCtrl = isRControl || isLControl;
        break;
    case DIK_LCONTROL:
        isLControl = false;
        io.KeyCtrl = isRControl || isLControl;
        break;
    case DIK_RSHIFT:
        isRShift = false;
        io.KeyShift = isRShift || isLShift;
        break;
    case DIK_LSHIFT:
        isLShift = false;
        io.KeyShift = isRShift || isLShift;
        break;
    case MOUSE_1:
        io.MouseDown[0] = false;
        break;
    case MOUSE_2:
        io.MouseDown[1] = false;
        break;
    case MOUSE_3:
        io.MouseDown[2] = false;
        break;
    default:
        if (key < 512)
            io.KeysDown[key] = false;
    }
    return active;
}

bool Editor_KeyHold(int key)
{
    if (!IsEditorActive())
        return false;
    return true;
}

bool Editor_MouseMove(int dx, int dy)
{
    if (!IsEditorActive())
        return false;

    ImGuiIO& io = ImGui::GetIO();
    POINT p;
    GetCursorPos(&p);
    io.MousePos.x = p.x;
    io.MousePos.y = p.y;
    return true;
}

static int s_direction{};

bool Editor_MouseWheel(int direction)
{
    if (!IsEditorActive())
        return false;

    s_direction = direction;

    return true;
}

void Editor_OnFrame()
{
    if (s_direction)
    {
        ImGuiIO& io = ImGui::GetIO();

        io.MouseWheel += s_direction > 0 ? +1.0f : -1.0f;
        s_direction = 0;
    }
}