#include "stdafx.h"
#include "pch_script.h"
#include "embedded_editor_weather.h"
#include "embedded_editor_helper.h"
#include "../../xrEngine/Environment.h"
#include "../../xrEngine/IGame_Level.h"
#include "../../xrEngine/thunderbolt.h"
#include "../../xrEngine/xr_efflensflare.h"
#include "../../xrEngine/x_ray.h"
#include "../GamePersistent.h"
#include "../Level.h"
#include "../ai_space.h"
#include "../xrServerEntities/script_engine.h"
#include <imgui.h>
#include "imgui_internal.h"

#include "string_table.h"

Fvector convert(const Fvector& v)
{
    Fvector result;
    result.set(v.z, v.y, v.x);
    return result;
}

Fvector4 convert(const Fvector4& v)
{
    Fvector4 result;
    result.set(v.z, v.y, v.x, v.w);
    return result;
}

bool enumCycle(void* data, int idx, const char** item)
{
    xr_vector<shared_str>* cycles = (xr_vector<shared_str>*)data;
    *item = (*cycles)[idx].c_str();
    return true;
}

bool enumWeather(void* data, int idx, const char** item)
{
    xr_vector<CEnvDescriptor*>* envs = (xr_vector<CEnvDescriptor*>*)data;
    *item = (*envs)[idx]->m_identifier.c_str();
    return true;
}

const char* empty = "";
bool enumIniWithEmpty(void* data, int idx, const char** item)
{
	if (idx == 0)
		*item = empty;
	else {
		CInifile* ini = (CInifile*)data;
		*item = ini->sections()[idx - 1]->Name.c_str();
	}
	return true;
}

bool enumIni(void* data, int idx, const char** item)
{
	CInifile* ini = (CInifile*)data;
	*item = ini->sections()[idx]->Name.c_str();
	return true;
}

bool getScriptWeather()
{
    luabind::object benchmark = ai().script_engine().name_space("benchmark");
    return benchmark["weather"].type() == LUA_TBOOLEAN ? !luabind::object_cast<bool>(benchmark["weather"]) : true;
}

void setScriptWeather(bool b)
{
    luabind::object benchmark = ai().script_engine().name_space("benchmark");
    benchmark["weather"] = !b;
}

xr_set<shared_str> modifiedWeathers;

void saveWeather(shared_str name, const xr_vector<CEnvDescriptor*>& env)
{
	CInifile f(nullptr, FALSE, FALSE, FALSE);
	for (auto el : env) {
		if (el->env_ambient)
			f.w_string(el->m_identifier.c_str(), "ambient", el->env_ambient->name().c_str());
		f.w_fvector3(el->m_identifier.c_str(), "ambient_color", el->ambient);
		f.w_fvector4(el->m_identifier.c_str(), "clouds_color", el->clouds_color);
		f.w_string(el->m_identifier.c_str(), "clouds_texture", el->clouds_texture_name.c_str());
		f.w_float(el->m_identifier.c_str(), "clouds_velocity_0", el->clouds_velocity_0);
		f.w_float(el->m_identifier.c_str(), "clouds_velocity_1", el->clouds_velocity_1);
		f.w_float(el->m_identifier.c_str(), "far_plane", el->far_plane);
		f.w_float(el->m_identifier.c_str(), "fog_distance", el->fog_distance);
		f.w_float(el->m_identifier.c_str(), "fog_density", el->fog_density);
		f.w_float(el->m_identifier.c_str(), "lowland_fog_density", el->lowland_fog_density);
		f.w_float(el->m_identifier.c_str(), "lowland_fog_height", el->lowland_fog_height);
		f.w_fvector3(el->m_identifier.c_str(), "fog_color", el->fog_color);
		f.w_fvector3(el->m_identifier.c_str(), "rain_color", el->rain_color);
		f.w_float(el->m_identifier.c_str(), "rain_density", el->rain_density);
		f.w_fvector3(el->m_identifier.c_str(), "sky_color", el->sky_color);
		f.w_float(el->m_identifier.c_str(), "sky_rotation", rad2deg(el->sky_rotation));
		f.w_string(el->m_identifier.c_str(), "sky_texture", el->sky_texture_name.c_str());
		f.w_fvector3(el->m_identifier.c_str(), "sun_color", el->sun_color);
		f.w_float(el->m_identifier.c_str(), "sun_shafts_intensity", el->m_fSunShaftsIntensity);
		f.w_string(el->m_identifier.c_str(), "sun", el->lens_flare_id.c_str());
		f.w_string(el->m_identifier.c_str(), "thunderbolt_collection", el->tb_id.c_str());
		f.w_float(el->m_identifier.c_str(), "thunderbolt_duration", el->bolt_duration);
		f.w_float(el->m_identifier.c_str(), "thunderbolt_period", el->bolt_period);
		f.w_float(el->m_identifier.c_str(), "water_intensity", el->m_fWaterIntensity);
		f.w_float(el->m_identifier.c_str(), "wind_direction", rad2deg(el->wind_direction));
		f.w_float(el->m_identifier.c_str(), "wind_velocity", el->wind_velocity);
		f.w_fvector4(el->m_identifier.c_str(), "hemisphere_color", el->hemi_color);
		f.w_float(el->m_identifier.c_str(), "sun_altitude", rad2deg(el->sun_dir.getH()));
		f.w_float(el->m_identifier.c_str(), "sun_longitude", rad2deg(el->sun_dir.getP()));
		f.w_float(el->m_identifier.c_str(), "tree_amplitude_intensity", el->m_fTreeAmplitudeIntensity);
		f.w_float(el->m_identifier.c_str(), "swing_normal_amp1", el->m_cSwingDesc[0].amp1);
		f.w_float(el->m_identifier.c_str(), "swing_normal_amp2", el->m_cSwingDesc[0].amp2);
		f.w_float(el->m_identifier.c_str(), "swing_normal_rot1", el->m_cSwingDesc[0].rot1);
		f.w_float(el->m_identifier.c_str(), "swing_normal_rot2", el->m_cSwingDesc[0].rot2);
		f.w_float(el->m_identifier.c_str(), "swing_normal_speed", el->m_cSwingDesc[0].speed);
		f.w_float(el->m_identifier.c_str(), "swing_fast_amp1", el->m_cSwingDesc[1].amp1);
		f.w_float(el->m_identifier.c_str(), "swing_fast_amp2", el->m_cSwingDesc[1].amp2);
		f.w_float(el->m_identifier.c_str(), "swing_fast_rot1", el->m_cSwingDesc[1].rot1);
		f.w_float(el->m_identifier.c_str(), "swing_fast_rot2", el->m_cSwingDesc[1].rot2);
		f.w_float(el->m_identifier.c_str(), "swing_fast_speed", el->m_cSwingDesc[1].speed);
		f.w_fvector4(el->m_identifier.c_str(), "color_grading", el->color_grading);
		f.w_fvector3(el->m_identifier.c_str(), "dof", el->dof_value);
		f.w_float(el->m_identifier.c_str(), "dof_kernel", el->dof_kernel);
		f.w_float(el->m_identifier.c_str(), "dof_sky", el->dof_sky);
		f.w_float(el->m_identifier.c_str(), "air_temperature", el->m_fAirTemperature);
		f.w_string(el->m_identifier.c_str(), "weather_type", el->m_sWeatherType.c_str());
		f.w_float(el->m_identifier.c_str(), "bloom_threshold", el->bloom_threshold);
		f.w_float(el->m_identifier.c_str(), "bloom_exposure", el->bloom_exposure);
		f.w_float(el->m_identifier.c_str(), "bloom_sky_intensity", el->bloom_sky_intensity);
	}
	string_path fileName;
	FS.update_path(fileName, "$game_weathers$", name.c_str());
	strconcat(sizeof(fileName), fileName, fileName, ".ltx");
	f.save_as(fileName);
}

void nextTexture(char* tex, int texSize, int offset)
{
	string_path dir, fn;
	_splitpath(tex, nullptr, dir, fn, nullptr);
	strconcat(sizeof(fn), fn, fn, ".dds");
	xr_vector<LPSTR>* files = FS.file_list_open("$game_textures$", dir, FS_ListFiles);
	if (!files)
		return;
	size_t index = 0;
	for (size_t i = 0; i != files->size(); i++)
		if (strcmp((*files)[i], fn) == 0) {
			index = i;
			break;
		}
	size_t newIndex = index;
	while (true) {
		newIndex = (newIndex + offset + files->size()) % files->size();
		if (strstr((*files)[newIndex], "#small") == nullptr && strstr((*files)[newIndex], ".thm") == nullptr)
			break;
	}
	string_path newFn;
	_splitpath((*files)[newIndex], nullptr, nullptr, newFn, nullptr);
	strconcat(texSize, tex, dir, newFn);
	FS.file_list_close(files);
}

bool ImGui_ListBox(const char* label, int* current_item, bool(*items_getter)(void*, int, const char**), void* data, int items_count, const ImVec2& size_arg = ImVec2(0, 0));

static bool stristr(const xr_string& str, const char* search)
{
	if (search[0] == '\0')
		return true;

	const char* s = str.c_str();
	const char* p = search;

	for (; *s != '\0'; s++)
	{
		if (tolower(*s) == tolower(*p))
		{
			const char* s2 = s + 1;
			const char* p2 = p + 1;
			while (*s2 != '\0' && *p2 != '\0' && tolower(*s2) == tolower(*p2))
			{
				s2++;
				p2++;
			}

			if (*p2 == '\0')
				return true;
		}
	}
	return false;
}

bool editTexture(const char* label, shared_str& texName)
{
	char tex[100];
	strncpy(tex, texName.data(), 100);
	bool changed = false;
	static shared_str prevValue;
	ImGui::PushID(label);
	if (ImGui::InputText("", tex, 100)) {
		texName = tex;
		changed = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("...")) {
		ImGui::OpenPopup(toUtf8(CStringTable().translate("st_editor_imgui_choose_texture").c_str()).c_str());
		prevValue = texName;
	}
	ImGui::SameLine();
	ImGui::Text(label);
	ImGui::SetNextWindowSize(ImVec2(250, 400), ImGuiCond_FirstUseEver);
	if (ImGui::BeginPopupModal(toUtf8(CStringTable().translate("st_editor_imgui_choose_texture").c_str()).c_str(), NULL, 0)) {
		string_path dir, fn;
		_splitpath(tex, nullptr, dir, fn, nullptr);
		strconcat(sizeof(fn), fn, fn, ".dds");
		static xr_map<xr_string, xr_vector<xr_string>> dirs;
		static char searchStr[128] = "";

		ImGui::InputText(toUtf8(CStringTable().translate("st_spawner_search").c_str()).c_str(), searchStr, IM_ARRAYSIZE(searchStr));

		auto filtered = dirs[dir];
		if (filtered.empty()) {
			xr_vector<LPSTR>* files = FS.file_list_open("$game_textures$", dir, FS_ListFiles);
			if (files) {
				filtered.resize(files->size());
				auto e = std::copy_if(files->begin(), files->end(), filtered.begin(),
					[](auto x) { return strstr(x, "#small") == nullptr && strstr(x, ".thm") == nullptr; });
				filtered.resize(e - filtered.begin());
				std::sort(filtered.begin(), filtered.end(),
					[](auto a, auto b) { return compare_naturally(a.c_str(), b.c_str()) < 0; });
				dirs[dir] = filtered;
			}
			FS.file_list_close(files);
		}

		xr_vector<xr_string> displayList;
		for (const auto& name : filtered) {
			if (searchStr[0] == '\0' || stristr(name, searchStr)) {
				displayList.push_back(name);
			}
		}

		int cur = -1;
		for (size_t i = 0; i < displayList.size(); i++)
			if (displayList[i] == fn) {
				cur = (int)i;
				break;
			}

		if (ImGui_ListBox("", &cur,
			[](void* data, int idx, const char** out_text) -> bool {
				xr_vector<xr_string>* textures = (xr_vector<xr_string>*)data;
				if (idx < 0 || idx >= (int)textures->size()) return false;
				*out_text = (*textures)[idx].c_str();
				return true;
			},
			&displayList, (int)displayList.size(), ImVec2(-1.0f, -20.0f)))
		{
			if (cur >= 0 && cur < (int)displayList.size()) {
				string_path newFn;
				_splitpath(displayList[cur].c_str(), nullptr, nullptr, newFn, nullptr);
				strconcat(100, tex, dir, newFn);
				texName = tex;
				changed = true;
			}
		}

		if (ImGui::Button(toUtf8(CStringTable().translate("st_weather_editor_btn_ok").c_str()).c_str(), ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button(toUtf8(CStringTable().translate("st_weather_editor_btn_cancel").c_str()).c_str(), ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
			string_path newFn;
			_splitpath(prevValue.data(), nullptr, nullptr, newFn, nullptr);
			strconcat(100, tex, dir, newFn);
			texName = tex;
			changed = true;
		}
		ImGui::EndPopup();
	}
	ImGui::PopID();
	return changed;
}

float editor_altitude = 0.f;
float editor_longitude = 0.f;

void ShowWeatherEditor(bool& show)
{
	if (!ImGui::Begin(modifiedWeathers.empty() ? "Weather###Weather" : "Weather*###Weather", &show)) {
        ImGui::End();
        return;
    }
    CEnvironment& env = GamePersistent().Environment();
    CEnvDescriptor* cur = env.Current[0];
    u64 time = Level().GetEnvironmentGameTime() / 1000;
    ImGui::Text("Time: %02d:%02d:%02d", int(time / (60 * 60) % 24), int(time / 60 % 60), int(time % 60));
    float tf = Level().GetEnvironmentTimeFactor();
    if (ImGui::SliderFloat(toUtf8(CStringTable().translate("st_weather_editor_time_factor").c_str()).c_str(), &tf, 0.0f, 1000.0f, "%.3f", ImGuiSliderFlags_Logarithmic))
        Level().SetEnvironmentTimeFactor(tf);
    xr_vector<shared_str> cycles;
    int iCycle = -1;
    for (const auto& el : env.WeatherCycles) {
        cycles.push_back(el.first);
        if (el.first == env.CurrentWeatherName)
            iCycle = cycles.size() - 1;
    }

	ImGui::Text(toUtf8(CStringTable().translate("st_weather_editor_main_options").c_str()).c_str());

	if (ImGui::Combo(toUtf8(CStringTable().translate("st_weather_editor_wth_cycle").c_str()).c_str(), &iCycle, enumCycle, &cycles, env.WeatherCycles.size()))
		env.SetWeather(cycles[iCycle], true);
	int sel = -1;
	for (u32 i = 0; i != env.CurrentWeather->size(); i++)
		if (cur->m_identifier == env.CurrentWeather->at(i)->m_identifier)
			sel = i;
	if (ImGui::Combo(toUtf8(CStringTable().translate("st_weather_editor_cur_sect").c_str()).c_str(), &sel, enumWeather, env.CurrentWeather, env.CurrentWeather->size())) {
		env.SetGameTime(env.CurrentWeather->at(sel)->exec_time + 0.5f, tf);
		time = time / (24 * 60 * 60) * 24 * 60 * 60 * 1000;
		time += u64(env.CurrentWeather->at(sel)->exec_time * 1000 + 0.5f);
		Level().SetEnvironmentGameTimeFactor(time, tf);
		env.SetWeather(cycles[iCycle], true);
	}
	static bool b = getScriptWeather();
	if (ImGui::Checkbox(toUtf8(CStringTable().translate("st_weather_editor_script_wth").c_str()).c_str(), &b))
		setScriptWeather(b);
	ImGui::Separator();
	bool changed = false;
	sel = -1;
	for (u32 i = 0; i != env.m_ambients_config->sections().size(); i++)
	{
		if (cur->env_ambient->name() == env.m_ambients_config->sections()[i]->Name)
			sel = i;
	}

	ImGui::Text(toUtf8(CStringTable().translate("st_weather_editor_amb_light_options").c_str()).c_str());

	if (ImGui::Combo("ambient", &sel, enumIni, env.m_ambients_config, env.m_ambients_config->sections().size())) {
		cur->env_ambient = env.AppendEnvAmb(env.m_ambients_config->sections()[sel]->Name);
		changed = true;
	}
	if (ImGui::ColorEdit3("ambient_color", (float*)&cur->ambient))
		changed = true;

	ImGui::Text(toUtf8(CStringTable().translate("st_weather_editor_clouds_options").c_str()).c_str());

	if (ImGui::ColorEdit4("clouds_color", (float*)&cur->clouds_color, ImGuiColorEditFlags_AlphaBar))
		changed = true;
	char buf[100];
	if (editTexture("clouds_texture", cur->clouds_texture_name))
	{
		cur->on_device_create();
		changed = true;
	}

	if (!bWeatherWindInfluenceKoef)
	{
		if (ImGui::SliderFloat("clouds_velocity_0", &cur->clouds_velocity_0, 0.0f, 0.1f))
			changed = true;

		if (ImGui::SliderFloat("clouds_velocity_1", &cur->clouds_velocity_1, 0.0f, 0.5f))
			changed = true;
	}

	ImGui::Text(toUtf8(CStringTable().translate("st_weather_editor_fog_options").c_str()).c_str());

	if (ImGui::SliderFloat("far_plane", &cur->far_plane, 0.01f, 10000.0f))
		changed = true;
	if (ImGui::SliderFloat("fog_distance", &cur->fog_distance, 0.0f, 10000.0f))
		changed = true;
	if (ImGui::SliderFloat("fog_density", &cur->fog_density, 0.0f, 10.0f))
		changed = true;
	if (ImGui::SliderFloat("lowland_fog_density", &cur->lowland_fog_density, 0.0f, 5.0f))
		changed = true;
	if (ImGui::SliderFloat("lowland_fog_height", &cur->lowland_fog_height, 0.0f, 100.0f))
		changed = true;
	if (ImGui::ColorEdit3("fog_color", (float*)&cur->fog_color))
		changed = true;

	ImGui::Text(toUtf8(CStringTable().translate("st_weather_editor_hemi_options").c_str()).c_str());

	if (ImGui::ColorEdit4("hemisphere_color", (float*)&cur->hemi_color, ImGuiColorEditFlags_AlphaBar))
		changed = true;

	ImGui::Text(toUtf8(CStringTable().translate("st_weather_editor_rain_options").c_str()).c_str());

	if (ImGui::SliderFloat("rain_density", &cur->rain_density, 0.0f, 10.0f))
		changed = true;
	if (ImGui::ColorEdit3("rain_color", (float*)&cur->rain_color))
		changed = true;

	ImGui::Text(toUtf8(CStringTable().translate("st_weather_editor_sky_options").c_str()).c_str());

	if (ImGui::ColorEdit3("sky_color", (float*)&cur->sky_color))
		changed = true;

	if (ImGui::SliderFloat("sky_rotation", &cur->sky_rotation, 0.0f, 6.28318f))
		changed = true;

	if (editTexture("sky_texture", cur->sky_texture_name)) {
		strconcat(sizeof(buf), buf, cur->sky_texture_name.data(), "#small");
		cur->sky_texture_env_name = buf;
		cur->on_device_create();
		changed = true;
	}

	ImGui::Text(toUtf8(CStringTable().translate("st_weather_editor_sun_options").c_str()).c_str());

	sel = -1;
	for (u32 i = 0; i != env.m_suns_config->sections().size(); i++)
		if (cur->lens_flare_id == env.m_suns_config->sections()[i]->Name)
			sel = i;
	if (ImGui::Combo("sun", &sel, enumIni, env.m_suns_config, env.m_suns_config->sections().size())) {
		cur->lens_flare_id
			= env.eff_LensFlare->AppendDef(env, env.m_suns_config, env.m_suns_config->sections()[sel]->Name.c_str());
		env.eff_LensFlare->Invalidate();
		changed = true;
	}
	if (ImGui::ColorEdit3("sun_color", (float*)&cur->sun_color))
		changed = true;
	if (ImGui::SliderFloat("sun_altitude", &editor_altitude, -360.0f, 360.0f))
	{
		changed = true;

		cur->sun_dir.setHP(deg2rad(editor_longitude), deg2rad(editor_altitude));
	}
	if (ImGui::SliderFloat("sun_longitude", &editor_longitude, -360.0f, 360.0f))
	{
		changed = true;

		cur->sun_dir.setHP(deg2rad(editor_longitude), deg2rad(editor_altitude));
	}
	if (ImGui::SliderFloat("sun_shafts_intensity", &cur->m_fSunShaftsIntensity, 0.0f, 2.0f))
		changed = true;
	sel = 0;
	for (u32 i = 0; i != env.m_thunderbolt_collections_config->sections().size(); i++)
	{
		if (cur->tb_id == env.m_thunderbolt_collections_config->sections()[i]->Name)
			sel = i + 1;
	}

	ImGui::Text(toUtf8(CStringTable().translate("st_weather_editor_thunder_bolt_options").c_str()).c_str());

	if (ImGui::Combo("thunderbolt_collection", &sel, enumIniWithEmpty, env.m_thunderbolt_collections_config,
		env.m_thunderbolt_collections_config->sections().size() + 1)) {
		cur->tb_id = (sel == 0)
			? env.eff_Thunderbolt->AppendDef(env, env.m_thunderbolt_collections_config, env.m_thunderbolts_config, "")
			: env.eff_Thunderbolt->AppendDef(env, env.m_thunderbolt_collections_config, env.m_thunderbolts_config,
				env.m_thunderbolt_collections_config->sections()[sel - 1]->Name.c_str());
		changed = true;
	}
	if (ImGui::SliderFloat("thunderbolt_duration", &cur->bolt_duration, 0.0f, 2.0f))
		changed = true;
	if (ImGui::SliderFloat("thunderbolt_period", &cur->bolt_period, 0.0f, 10.0f))
		changed = true;

	ImGui::Text(toUtf8(CStringTable().translate("st_weather_editor_water_options").c_str()).c_str());

	if (ImGui::SliderFloat("water_intensity", &cur->m_fWaterIntensity, 0.0f, 2.0f))
		changed = true;

	ImGui::Text(toUtf8(CStringTable().translate("st_weather_editor_wind_options").c_str()).c_str());

	if (ImGui::SliderFloat("wind_velocity", &cur->wind_velocity, 0.0f, 100.0f))
		changed = true;
	if (ImGui::SliderFloat("wind_direction", &cur->wind_direction, 0.0f, 360.0f))
		changed = true;

	if (!bWeatherWindInfluenceKoef)
	{
		ImGui::Text(toUtf8(CStringTable().translate("st_weather_editor_trees_options").c_str()).c_str());

		if (ImGui::SliderFloat("trees_amplitude_intensity", &cur->m_fTreeAmplitudeIntensity, 0.01f, 0.250f))
			changed = true;

		ImGui::Text(toUtf8(CStringTable().translate("st_weather_editor_grass_swing_options").c_str()).c_str());

		if (ImGui::SliderFloat("swing_normal_amp1", &cur->m_cSwingDesc[0].amp1, 0.0f, 10.0f))
			changed = true;
		if (ImGui::SliderFloat("swing_normal_amp2", &cur->m_cSwingDesc[0].amp2, 0.0f, 10.0f))
			changed = true;
		if (ImGui::SliderFloat("swing_normal_rot1", &cur->m_cSwingDesc[0].rot1, 0.0f, 300.0f))
			changed = true;
		if (ImGui::SliderFloat("swing_normal_rot2", &cur->m_cSwingDesc[0].rot2, 0.0f, 300.0f))
			changed = true;
		if (ImGui::SliderFloat("swing_normal_speed", &cur->m_cSwingDesc[0].speed, 0.0f, 10.0f))
			changed = true;
		if (ImGui::SliderFloat("swing_fast_amp1", &cur->m_cSwingDesc[1].amp1, 0.0f, 10.0f))
			changed = true;
		if (ImGui::SliderFloat("swing_fast_amp2", &cur->m_cSwingDesc[1].amp2, 0.0f, 10.0f))
			changed = true;
		if (ImGui::SliderFloat("swing_fast_rot1", &cur->m_cSwingDesc[1].rot1, 0.0f, 300.0f))
			changed = true;
		if (ImGui::SliderFloat("swing_fast_rot2", &cur->m_cSwingDesc[1].rot2, 0.0f, 300.0f))
			changed = true;
		if (ImGui::SliderFloat("swing_fast_speed", &cur->m_cSwingDesc[1].speed, 0.0f, 10.0f))
			changed = true;
	}

	if (bWeatherColorGrading)
	{
		ImGui::Text(toUtf8(CStringTable().translate("st_weather_editor_clr_grad_options").c_str()).c_str());

		if (ImGui::ColorEdit4("color_grading", (float*)&cur->color_grading))
			changed = true;
	}

	ImGui::Text(toUtf8(CStringTable().translate("st_weather_editor_bloom_options").c_str()).c_str());

	if (ImGui::SliderFloat("bloom_threshold", &cur->bloom_threshold, 0.0f, 2.5f))
		changed = true;

	if (ImGui::SliderFloat("bloom_exposure", &cur->bloom_exposure, 0.0f, 10.0f))
		changed = true;

	if (ImGui::SliderFloat("bloom_sky_intensity", &cur->bloom_sky_intensity, 0.0f, 1.0f))
		changed = true;

	if (bDofWeather)
	{
		ImGui::Text(toUtf8(CStringTable().translate("st_weather_editor_dof_options").c_str()).c_str());

		if (ImGui::InputFloat3("dof", (float*)&cur->dof_value), 3)
			changed = true;
		if (ImGui::SliderFloat("dof_kernel", &cur->dof_kernel, 0.0f, 10.0f))
			changed = true;
		if (ImGui::SliderFloat("dof_sky", &cur->dof_sky, -10000.0f, 10000.0f))
			changed = true;
	}

	ImGui::Text(toUtf8(CStringTable().translate("st_weather_editor_other_options").c_str()).c_str());

	if (ImGui::SliderFloat("air_temperature", &cur->m_fAirTemperature, -50.0f, 50.0f))
		changed = true;

	static char newType[256]{};

	if (cur->m_sWeatherType.size())
		strcpy(newType, cur->m_sWeatherType.c_str());
	else
		memset(newType, 0, sizeof(newType));

	if (ImGui::InputText("weather_type", newType, 512))
		changed = true;

	if (changed)
	{
		cur->m_sWeatherType = newType;
		modifiedWeathers.insert(env.CurrentWeatherName);
	}

	if (ImGui::Button(toUtf8(CStringTable().translate("st_editor_imgui_save").c_str()).c_str())) {
		for (auto name : modifiedWeathers)
			saveWeather(name, env.WeatherCycles[name]);
		modifiedWeathers.clear();
	}
    ImGui::End();
} 

bool WeatherEditor_MouseWheel(float wheel)
{
	ImGui::Begin(modifiedWeathers.empty() ? "Weather###Weather" : "Weather*###Weather");

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