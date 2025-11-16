#include "stdafx.h"
#include "pch_script.h"
#include "embedded_editor_helper.h"
#include <luabind/class_info.hpp>

xr_string to_string(const luabind::object& o, xr_string offset)
{
    int type = o.type();
    xr_string s;
    switch (type) {
    case LUA_TNIL:
        s = "nil";
        break;
    case LUA_TBOOLEAN:
        s = luabind::object_cast<bool>(o) ? "true" : "false";
        break;
    case LUA_TNUMBER: {
        std::string temp = std::to_string(luabind::object_cast<float>(o));
        std::string::size_type n = temp.find('.');
        if (n != std::string::npos) {
            bool isZ = true;
            for (std::string::size_type i = n + 1; i != temp.size(); ++i)
                if (temp[i] != '0')
                    isZ = false;
            if (isZ)
                temp[n] = '\0';
        }
        s = temp.c_str();
    } break;
    case LUA_TSTRING:
        s = luabind::object_cast<LPCSTR>(o);
        break;
    case LUA_TTABLE: {
        s = "{\n";
        xr_string newOffset = offset + "  ";
        for (luabind::object::iterator i = o.begin(), e = o.end(); i != e; ++i)
            s += newOffset + to_string(i.key(), newOffset) + "=" + to_string(*i, newOffset) + "\n";
        s += offset + "}";
    } break;
    case LUA_TUSERDATA: {
        s = "<userdata=";
        auto info = luabind::get_class_info(o);
        s += info.name.c_str();
        s += ">";
    } break;
    default:
        s = "<type=";
        s += std::to_string(type).c_str();
        s += ">";
    }
    return s;
}

bool ImGui_ListBox(const char* label, int* current_item, bool(*items_getter)(void*, int, const char**), void* data,
	int items_count, const ImVec2& size_arg = ImVec2(0, 0))
{
	if (!ImGui::ListBoxHeader(label, size_arg))
		return false;

	// Assume all items have even height (= 1 line of text). If you need items of different or variable sizes you can
	// create a custom version of ListBox() in your code without using the clipper.
	bool value_changed = false;
	ImGuiListClipper clipper(items_count, ImGui::GetTextLineHeightWithSpacing()); // We know exactly our line height
																				  // here so we pass it as a minor
																				  // optimization, but generally you
																				  // don't need to.
	while (clipper.Step())
		for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
			const bool item_selected = (i == *current_item);
			const char* item_text;
			if (!items_getter(data, i, &item_text))
				item_text = "*Unknown item*";

			ImGui::PushID(i);
			if (ImGui::Selectable(item_text, item_selected)) {
				*current_item = i;
				value_changed = true;
			}
			ImGui::PopID();
		}
	ImGui::ListBoxFooter();
	return value_changed;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// from https://www.strchr.com/natural_sorting
////////////////////////////////////////////////////////////////////////////////////////////////////

int count_digits(const char* s)
{
    const char* p = s;
    while (isdigit(*p))
        p++;
    return (int)(p - s);
}

int compare_naturally(const void* a_ptr, const void* b_ptr)
{
    const char* a = (const char*)a_ptr;
    const char* b = (const char*)b_ptr;

    for (;;) {
        if (isdigit(*a) && isdigit(*b)) {
            int a_count = count_digits(a);
            int diff = a_count - count_digits(b);
            if (diff)
                return diff;
            diff = memcmp(a, b, a_count);
            if (diff)
                return diff;
            a += a_count;
            b += a_count;
        }
        if (*a != *b)
            return *a - *b;
        if (*a == '\0')
            return 0;
        a++, b++;
    }
}