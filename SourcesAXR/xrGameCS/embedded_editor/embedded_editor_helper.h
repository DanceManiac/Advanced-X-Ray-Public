#pragma once

#include <imgui.h>

namespace luabind {
class object;
}

struct ImguiWnd {
  //  ImguiWnd(const char* name, bool* pOpen = nullptr) { Collapsed = !ImGui::Begin(name, pOpen); }
    ImguiWnd(const char* name, bool* pOpen = nullptr, ImGuiWindowFlags flags = 0) { Collapsed = !ImGui::Begin(name, pOpen, flags); }
    ~ImguiWnd() { ImGui::End(); }

    bool Collapsed;
};

xr_string to_string(const luabind::object& o, xr_string offset = xr_string());
xr_string toUtf8(const char* s);
int count_digits(const char* s);
int compare_naturally(const void* a_ptr, const void* b_ptr);