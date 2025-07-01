////////////////////////////////////////////////////////////////////////////
//	Module 		: embedded_editor_postprocess.cpp
//	Created 	: 25.06.2025
//  Modified 	: 26.06.2025
//	Author		: Dance Maniac (M.F.S. Team)
//	Description : ImGui Postprocess InGame Editor
////////////////////////////////////////////////////////////////////////////

#include "stdAfx.h"
#include "embedded_editor_helper.h"
#include "imgui_internal.h"

#include "Actor.h"
#include "ActorEffector.h"
#include "PostprocessAnimator.h"
#include "ai\monsters\ai_monster_effector.h"
#include "Inventory.h"
#include "Weapon.h"
#include "string_table.h"

xr_vector<SPPEffect> m_effects{};
xr_vector<shared_str> m_sections_vec{};
int m_current_effect = -1;
CInifile pIni(nullptr, false, false, true);

struct SCustomEffector
{
    SPPInfo	ppi;
    float	time{};
    float	time_attack{};
    float	time_release{};

    // custom camera effects
    float	ce_time{};
    float	ce_amplitude{};
    float	ce_period_number{};
    float	ce_power{};
};

SCustomEffector m_CustomEffector{};

void ApplyEffect(const SPPEffect& eff)
{

    if (eff.m_bIsCustomEffect)
    {
        m_CustomEffector.ppi.duality.h = eff.duality_h;
        m_CustomEffector.ppi.duality.v = eff.duality_v;
        m_CustomEffector.ppi.gray = eff.gray;
        m_CustomEffector.ppi.blur = eff.blur;
        m_CustomEffector.ppi.noise.intensity = eff.noise_intensity;
        m_CustomEffector.ppi.noise.grain = eff.noise_grain;
        m_CustomEffector.ppi.noise.fps = eff.noise_fps;
        m_CustomEffector.ppi.color_base.set(eff.color_base.x, eff.color_base.y, eff.color_base.z);
        m_CustomEffector.ppi.color_gray.set(eff.color_gray.x, eff.color_gray.y, eff.color_gray.z);
        m_CustomEffector.ppi.color_add.set(eff.color_add.x, eff.color_add.y, eff.color_add.z);

        m_CustomEffector.ce_time = eff.ce_time;
        m_CustomEffector.ce_amplitude = eff.ce_amplitude;
        m_CustomEffector.ce_period_number = eff.ce_period_number;
        m_CustomEffector.ce_power = eff.ce_power;

        m_CustomEffector.time = eff.time;
        m_CustomEffector.time_attack = eff.time_attack;
        m_CustomEffector.time_release = eff.time_release;

        CMonsterEffector* m_effector = xr_new<CMonsterEffector>(m_CustomEffector.ppi, m_CustomEffector.time, m_CustomEffector.time_attack, m_CustomEffector.time_release, 1.0f);
        CMonsterEffectorHit* m_cam_effector = xr_new<CMonsterEffectorHit>(m_CustomEffector.ce_time, m_CustomEffector.ce_amplitude, m_CustomEffector.ce_period_number, m_CustomEffector.ce_power);

        m_effector->bOverlap = eff.m_bOverlap;
        m_effector->SetType((EEffectorPPType)effPostprocessEditor);

        m_cam_effector->SetType((ECamEffectorType)eCEPostprocessEditor);

        Actor()->Cameras().AddCamEffector(m_cam_effector);
        Actor()->Cameras().AddPPEffector(m_effector);
    }

    CEffectorCam* effector = Actor()->Cameras().GetCamEffector((ECamEffectorType)eCEPostprocessEditor);

    if (eff.section.size())
    {
        if (eff.m_bIsShootEffector)
        {
            float effector_intensity = eff.m_fShootEffectorFactor;
            float effector_intensity_aim = eff.m_fShootEffectorFactorAim;
            float effector_intensity_crouch = eff.m_fShootEffectorFactorCrouch;

            CWeapon* Wpn = smart_cast<CWeapon*>(Actor()->inventory().ActiveItem());

            if (!effector)
                AddEffectorEditor(Actor(), eCEPostprocessEditor, eff.section.c_str(), (Wpn && Wpn->IsZoomed()) ? effector_intensity_aim : Actor()->is_actor_crouch() ? effector_intensity_crouch : effector_intensity, &eff);

        }
        else
        {
            if (!effector)
                AddEffectorEditor(Actor(), eCEPostprocessEditor, eff.section.c_str(), 1.0f, &eff);
        }
    }
}

void StopEffect(const SPPEffect& eff)
{
    if (eff.m_bIsCustomEffect)
    {
        Actor()->Cameras().RemovePPEffector((EEffectorPPType)effPostprocessEditor);
        Actor()->Cameras().RemoveCamEffector((ECamEffectorType)eCEPostprocessEditor);
    }

    RemoveEffector(Actor(), eCEPostprocessEditor);
}

void ParseEffect(const shared_str& section, bool isCustomEffect)
{
    SPPEffect eff;
    eff.section = section;

    eff.m_bOverlap = READ_IF_EXISTS(pSettings, r_bool, section, "pp_eff_overlap", false);
    eff.m_PPE_Cyclic = READ_IF_EXISTS(pSettings, r_u32, section, "pp_eff_cyclic", 0);
    eff.m_bHudAffect = READ_IF_EXISTS(pSettings, r_bool, section, "cam_eff_hud_affect", false);
    eff.m_CamEffectCyclic = READ_IF_EXISTS(pSettings, r_u32, section, "cam_eff_cyclic", 0);

    if (isCustomEffect)
        eff.m_bIsCustomEffect = isCustomEffect;

    eff.duality_h = READ_IF_EXISTS(pSettings, r_float, section, "duality_h", 0.f);
    eff.duality_v = READ_IF_EXISTS(pSettings, r_float, section, "duality_v", 0.f);
    eff.blur = READ_IF_EXISTS(pSettings, r_float, section, "blur", 0.f);
    eff.gray = READ_IF_EXISTS(pSettings, r_float, section, "gray", 0.f);
    eff.noise = READ_IF_EXISTS(pSettings, r_float, section, "noise", 0.f);
    eff.noise_scale = READ_IF_EXISTS(pSettings, r_float, section, "noise_scale", 0.f);
    eff.noise_intensity = READ_IF_EXISTS(pSettings, r_float, section, "noise_intensity", 0.f);
    eff.noise_grain = READ_IF_EXISTS(pSettings, r_float, section, "noise_grain", 0.f);
    eff.noise_fps = READ_IF_EXISTS(pSettings, r_float, section, "noise_fps", 0.f);

    if (pSettings->line_exist(section, "noise_color"))
        sscanf(pSettings->r_string(section, "noise_color"), "%f,%f,%f,%f", &eff.noise_color.x, &eff.noise_color.y, &eff.noise_color.z, &eff.noise_color.w);

    if (pSettings->line_exist(section, "color_base"))
        sscanf(pSettings->r_string(section, "color_base"), "%f,%f,%f", &eff.color_base.x, &eff.color_base.y, &eff.color_base.z);

    if (pSettings->line_exist(section, "color_gray"))
        sscanf(pSettings->r_string(section, "color_gray"), "%f,%f,%f", &eff.color_gray.x, &eff.color_gray.y, &eff.color_gray.z);

    if (pSettings->line_exist(section, "color_add"))
        sscanf(pSettings->r_string(section, "color_add"), "%f,%f,%f", &eff.color_add.x, &eff.color_add.y, &eff.color_add.z);

    eff.ce_time = READ_IF_EXISTS(pSettings, r_float, section, "ce_time", 0.f);
    eff.ce_amplitude = READ_IF_EXISTS(pSettings, r_float, section, "ce_amplitude", 0.f);
    eff.ce_period_number = READ_IF_EXISTS(pSettings, r_float, section, "ce_period_number", 0.f);
    eff.ce_power = READ_IF_EXISTS(pSettings, r_float, section, "ce_power", 0.f);

    eff.time = READ_IF_EXISTS(pSettings, r_float, section, "time", 0.f);
    eff.time_attack = READ_IF_EXISTS(pSettings, r_float, section, "time_attack", 0.f);
    eff.time_release = READ_IF_EXISTS(pSettings, r_float, section, "time_release", 0.f);

    eff.radius_min = READ_IF_EXISTS(pSettings, r_float, section, "radius_min", 0.f);
    eff.radius_max = READ_IF_EXISTS(pSettings, r_float, section, "radius_max", 0.f);
    eff.random_cam_effects = READ_IF_EXISTS(pSettings, r_bool, section, "random_cam_effects", false);

    if (pSettings->line_exist(section, "shoot_effector_factor"))
    {
        eff.m_fShootEffectorFactor = pSettings->r_float(section, "shoot_effector_factor");
        eff.m_bIsShootEffector = true;
    }

    if (pSettings->line_exist(section, "shoot_effector_factor_aim"))
    {
        eff.m_fShootEffectorFactorAim = pSettings->r_float(section, "shoot_effector_factor_aim");
        eff.m_bIsShootEffector = true;
    }

    if (pSettings->line_exist(section, "shoot_effector_factor_crouch"))
    {
        eff.m_fShootEffectorFactorCrouch = pSettings->r_float(section, "shoot_effector_factor_crouch");
        eff.m_bIsShootEffector = true;
    }

    int pp_index = 1;

    while (true)
    {
        string128 key;

        if (pp_index == 1)
            xr_strcpy(key, "pp_eff_name");
        else
            xr_sprintf(key, "pp_eff_name_%d", pp_index);

        if (!pSettings->line_exist(section, key))
            break;

        eff.pp_effects.push_back(pSettings->r_string(section, key));
        pp_index++;
    }

    int cam_index = 1;

    while (true)
    {
        string128 key;

        if (cam_index == 1)
            xr_strcpy(key, "cam_eff_name");
        else
            xr_sprintf(key, "cam_eff_name_%d", cam_index);

        if (!pSettings->line_exist(section, key))
            break;

        eff.cam_effects.push_back(pSettings->r_string(section, key));
        cam_index++;
    }

    m_effects.push_back(eff);
}

void FillSectionsListPPE()
{
    for (auto sect : pSettings->sections())
    {
        if (sect->line_exist("pp_eff_name") || sect->line_exist("cam_eff_name"))
        {
            m_sections_vec.push_back(sect->Name);
            ParseEffect(sect->Name, sect->line_exist("duality_h"));
        }
    }
}

void SaveEffect(SPPEffect& eff)
{
    string_path fname{};
    string256 file_name = "postprocess_new.ltx";

    FS.update_path(fname, "$app_data_root$", make_string("PostprocessEditor\\%s", file_name).c_str());

    if (!fis_zero(eff.m_fShootEffectorFactor))
        pIni.w_float(eff.section.c_str(), "shoot_effector_factor", eff.m_fShootEffectorFactor);

    if (!fis_zero(eff.m_fShootEffectorFactorAim))
        pIni.w_float(eff.section.c_str(), "shoot_effector_factor_aim", eff.m_fShootEffectorFactorAim);

    if (!fis_zero(eff.m_fShootEffectorFactorCrouch))
        pIni.w_float(eff.section.c_str(), "shoot_effector_factor_crouch", eff.m_fShootEffectorFactorCrouch);

    if (eff.m_bIsCustomEffect)
    {
        pIni.w_float(eff.section.c_str(), "duality_h", eff.duality_h);
        pIni.w_float(eff.section.c_str(), "duality_v", eff.duality_v);
        pIni.w_float(eff.section.c_str(), "blur", eff.blur);
        pIni.w_float(eff.section.c_str(), "gray", eff.gray);
        pIni.w_float(eff.section.c_str(), "noise", eff.noise);
        pIni.w_float(eff.section.c_str(), "noise_intensity", eff.noise_intensity);
        pIni.w_float(eff.section.c_str(), "noise_grain", eff.noise_grain);
        pIni.w_float(eff.section.c_str(), "noise_fps", eff.noise_fps);

        string256 color_str{};

        sprintf_s(color_str, "%f,%f,%f", eff.color_base.x, eff.color_base.y, eff.color_base.z);
        pIni.w_string(eff.section.c_str(), "color_base", color_str);

        sprintf_s(color_str, "%f,%f,%f", eff.color_gray.x, eff.color_gray.y, eff.color_gray.z);
        pIni.w_string(eff.section.c_str(), "color_gray", color_str);

        sprintf_s(color_str, "%f,%f,%f", eff.color_add.x, eff.color_add.y, eff.color_add.z);
        pIni.w_string(eff.section.c_str(), "color_add", color_str);

        pIni.w_float(eff.section.c_str(), "ce_time", eff.ce_time);
        pIni.w_float(eff.section.c_str(), "ce_amplitude", eff.ce_amplitude);
        pIni.w_float(eff.section.c_str(), "ce_period_number", eff.ce_period_number);
        pIni.w_float(eff.section.c_str(), "ce_power", eff.ce_power);

        pIni.w_float(eff.section.c_str(), "time", eff.time);
        pIni.w_float(eff.section.c_str(), "time_attack", eff.time_attack);
        pIni.w_float(eff.section.c_str(), "time_release", eff.time_release);

        pIni.w_float(eff.section.c_str(), "radius_min", eff.radius_min);
        pIni.w_float(eff.section.c_str(), "radius_max", eff.radius_max);
        pIni.w_bool(eff.section.c_str(), "random_cam_effects", eff.random_cam_effects);
    }

    for (size_t i = 0; i < eff.pp_effects.size(); i++)
    {
        string128 key;

        if (i == 0)
            xr_strcpy(key, "pp_eff_name");
        else
            xr_sprintf(key, "pp_eff_name_%d", i + 1);

        if (eff.pp_effects[i].size())
            pIni.w_string(eff.section.c_str(), key, eff.pp_effects[i].c_str());
    }

    pIni.w_u32(eff.section.c_str(), "pp_eff_cyclic", eff.m_PPE_Cyclic);
    pIni.w_bool(eff.section.c_str(), "pp_eff_overlap", eff.m_bOverlap);

    for (size_t i = 0; i < eff.cam_effects.size(); i++)
    {
        string128 key;

        if (i == 0)
            xr_strcpy(key, "cam_eff_name");
        else
            xr_sprintf(key, "cam_eff_name_%d", i + 1);

        if (eff.cam_effects[i].size())
            pIni.w_string(eff.section.c_str(), key, eff.cam_effects[i].c_str());
    }

    pIni.w_u32(eff.section.c_str(), "pp_eff_cyclic", eff.m_CamEffectCyclic);
    pIni.w_bool(eff.section.c_str(), "cam_eff_hud_affect", eff.m_bHudAffect);

    pIni.save_at_end(TRUE);
    pIni.save_as(fname);
}

void ShowPostprocessEditor(bool& show)
{
	ImguiWnd wnd(toUtf8(CStringTable().translate("st_editor_imgui_ppe").c_str()).c_str(), &show);
	
    if (wnd.Collapsed)
		return;

    static char newEffectName[128] = "new_effector";

	ImGui::BeginChild("LeftPane", ImVec2(350, 0), true);

    ImGui::InputText("##NewEffectName", newEffectName, sizeof(newEffectName));
    ImGui::SameLine();
    if (ImGui::Button(toUtf8(CStringTable().translate("st_pp_editor_create").c_str()).c_str()))
    {
        if (strlen(newEffectName) > 0)
        {
            SPPEffect new_eff;
            new_eff.section = newEffectName;
            m_effects.push_back(new_eff);
            m_current_effect = m_effects.size() - 1;
            strcpy(newEffectName, "new_effector");
        }
    }

	for (size_t i = 0; i < m_effects.size(); i++)
	{
		if (ImGui::Selectable(m_effects[i].section.c_str(), m_current_effect == i))
			m_current_effect = i;
	}
	ImGui::EndChild();

	ImGui::SameLine();

    ImGui::BeginGroup();
    if (m_current_effect >= 0 && m_current_effect < (int)m_effects.size())
    {
        auto& eff = m_effects[m_current_effect];

        xr_string cur_effector_name = toUtf8(CStringTable().translate("st_pp_editor_editing_effector").c_str());
        ImGui::Text(make_string("%s %s", cur_effector_name.c_str(), eff.section.c_str()).c_str());

        ImGui::Checkbox(toUtf8(CStringTable().translate("st_pp_editor_is_custom_effect").c_str()).c_str(), &eff.m_bIsCustomEffect);

        if (eff.m_bIsCustomEffect)
        {
            ImGui::Separator();
            ImGui::Text(toUtf8(CStringTable().translate("st_pp_editor_custom_pp_params").c_str()).c_str());

            ImGui::DragFloat("Duality Horizontal", &eff.duality_h, 0.001f, 0.f, 1.f);
            ImGui::DragFloat("Duality Vertical", &eff.duality_v, 0.001f, 0.f, 1.f);
            ImGui::DragFloat("Blur", &eff.blur, 0.001f, 0.f, 1.f);
            ImGui::DragFloat("Gray", &eff.gray, 0.001f, 0.f, 1.f);
            ImGui::DragFloat("Noise", &eff.noise, 0.001f, 0.f, 1.f);
            ImGui::DragFloat("Noise Scale", &eff.noise_scale, 0.001f, 0.f, 10.f);
            ImGui::DragFloat("Noise Intensity", &eff.noise_intensity, 0.001f, 0.f, 10.f);
            ImGui::DragFloat("Noise Grain", &eff.noise_grain, 0.001f, 0.f, 10.f);
            ImGui::DragFloat("Noise FPS", &eff.noise_fps, 0.1f, 0.f, 120.f);
            ImGui::ColorEdit4("Noise Color", (float*)&eff.noise_color);
            ImGui::ColorEdit3("Color Base", (float*)&eff.color_base);
            ImGui::ColorEdit3("Color Gray", (float*)&eff.color_gray);
            ImGui::ColorEdit3("Color Add", (float*)&eff.color_add);

            ImGui::Separator();
            ImGui::Text(toUtf8(CStringTable().translate("st_pp_editor_custom_camera_params").c_str()).c_str());

            ImGui::DragFloat("Cam Effect Time", &eff.ce_time, 0.01f, 0.f, 100.f);
            ImGui::DragFloat("Cam Effect Amplitude", &eff.ce_amplitude, 0.001f, 0.f, 100.f);
            ImGui::DragFloat("Cam Effect Period Number", &eff.ce_period_number, 0.1f, 0.f, 100.f);
            ImGui::DragFloat("Cam Effect Power", &eff.ce_power, 0.01f, 0.f, 100.f);

            ImGui::Separator();
            ImGui::Text(toUtf8(CStringTable().translate("st_pp_editor_custom_effector_params").c_str()).c_str());

            ImGui::DragFloat("Time", &eff.time, 0.001f, 0.f, 100.f);
            ImGui::DragFloat("Time Attack", &eff.time_attack, 0.1f, 0.f, 100.f);
            ImGui::DragFloat("Time Release", &eff.time_release, 0.1f, 0.f, 100.f);
        }

        ImGui::Separator();
        ImGui::Text(toUtf8(CStringTable().translate("st_pp_editor_main_params").c_str()).c_str());

        ImGui::DragFloat("Radius Min", &eff.radius_min, 0.001f, 0.f, 1.f);
        ImGui::DragFloat("Radius Max", &eff.radius_max, 0.001f, 0.f, 1.f);
        ImGui::Checkbox("Random Camera Effects", &eff.random_cam_effects);
        ImGui::Checkbox("PP Effect Cyclic", (bool*)(& eff.m_PPE_Cyclic));
        ImGui::Checkbox("PP Effect Overlap", &eff.m_bOverlap);
        ImGui::Checkbox("Cam Effect Cyclic", (bool*)(&eff.m_CamEffectCyclic));
        ImGui::Checkbox("Cam Effect Hud Affect", &eff.m_bHudAffect);

        ImGui::Separator();
        ImGui::Text(toUtf8(CStringTable().translate("st_pp_editor_shoot_eff_params").c_str()).c_str());

        if (!fis_zero(eff.m_fShootEffectorFactor))
            ImGui::DragFloat("Shoot Factor", &eff.m_fShootEffectorFactor, 0.001f, 0.01f, 250.f);

        if (!fis_zero(eff.m_fShootEffectorFactorAim))
            ImGui::DragFloat("Shoot Factor Aim", &eff.m_fShootEffectorFactorAim, 0.001f, 0.01f, 250.f);

        if (!fis_zero(eff.m_fShootEffectorFactorCrouch))
            ImGui::DragFloat("Shoot Factor Crouch", &eff.m_fShootEffectorFactorCrouch, 0.001f, 0.01f, 250.f);

        ImGui::Separator();
        ImGui::Text(toUtf8(CStringTable().translate("st_pp_editor_pp_effects").c_str()).c_str());

        for (size_t i = 0; i < eff.pp_effects.size(); i++)
        {
            ImGui::PushID(i);
            char buf[256];
            strncpy(buf, eff.pp_effects[i].c_str(), sizeof(buf));

            if (ImGui::InputText("##pp", buf, sizeof(buf)))
                eff.pp_effects[i] = buf;

            ImGui::SameLine();
            if (ImGui::Button("X"))
            {
                eff.pp_effects.erase(eff.pp_effects.begin() + i);
                ImGui::PopID();
                break;
            }

            ImGui::SameLine();
            if (ImGui::Button(toUtf8(CStringTable().translate("st_pp_editor_play").c_str()).c_str()))
            {
                auto parse_effect_params = [](LPCSTR params, float default_factor = 1.0f) -> std::pair<shared_str, float>
                    {
                        if (!params || !params[0])
                            return std::make_pair(shared_str(params), default_factor);

                        string512 buffer;
                        float factor_mod = 1.0f;
                        int count = _GetItemCount(params);

                        _GetItem(params, 0, buffer);
                        shared_str effect_name = buffer;

                        // Dance Maniac: Factor modifiers
                        if (count > 1)
                        {
                            _GetItem(params, 1, buffer);
                            factor_mod = std::stof(buffer);
                        }

                        return std::make_pair(effect_name, factor_mod);
                    };

                LPCSTR params = eff.pp_effects[i].c_str();
                auto [effect_name, factor_mod] = parse_effect_params(params, 1.0f);

                CPostprocessAnimatorLerpConst* pp_anm = xr_new<CPostprocessAnimatorLerpConst>();

                pp_anm->SetType((EEffectorPPType)effPostprocessEditor);
                pp_anm->SetCyclic(eff.m_PPE_Cyclic);
                pp_anm->SetPower(factor_mod);
                pp_anm->bOverlap = eff.m_bOverlap;
                pp_anm->Load(effect_name.c_str());
                Actor()->Cameras().AddPPEffector(pp_anm);
            }

            ImGui::SameLine();
            if (ImGui::Button(toUtf8(CStringTable().translate("st_pp_editor_stop").c_str()).c_str()))
            {
                Actor()->Cameras().RemovePPEffector((EEffectorPPType)effPostprocessEditor);
            }

            ImGui::PopID();
        }

        if (ImGui::Button(toUtf8(CStringTable().translate("st_pp_editor_add_pp_effect").c_str()).c_str()))
            eff.pp_effects.push_back("");

        ImGui::Separator();
        ImGui::Text(toUtf8(CStringTable().translate("st_pp_editor_camera_effects").c_str()).c_str());

        for (size_t i = 0; i < eff.cam_effects.size(); i++)
        {
            ImGui::PushID(i + 1000);
            char buf[256];
            strncpy(buf, eff.cam_effects[i].c_str(), sizeof(buf));

            if (ImGui::InputText("##cam", buf, sizeof(buf)))
                eff.cam_effects[i] = buf;

            ImGui::SameLine();
            if (ImGui::Button("X"))
            {
                eff.cam_effects.erase(eff.cam_effects.begin() + i);
                ImGui::PopID();
                break;
            }

            ImGui::SameLine();
            if (ImGui::Button(toUtf8(CStringTable().translate("st_pp_editor_play").c_str()).c_str()))
            {
                auto parse_effect_params = [](LPCSTR params, float default_factor = 1.0f) -> std::pair<shared_str, float>
                    {
                        if (!params || !params[0])
                            return std::make_pair(shared_str(params), default_factor);

                        string512 buffer;
                        float factor_mod = 1.0f;
                        int count = _GetItemCount(params);

                        _GetItem(params, 0, buffer);
                        shared_str effect_name = buffer;

                        // Dance Maniac: Factor modifiers
                        if (count > 1)
                        {
                            _GetItem(params, 1, buffer);
                            factor_mod = std::stof(buffer);
                        }

                        return std::make_pair(effect_name, factor_mod);
                    };

                LPCSTR params = eff.cam_effects[i].c_str();
                auto [effect_name, factor_mod] = parse_effect_params(params, 1.0f);

                if (Actor()->Cameras().GetCamEffector((ECamEffectorType)eCEPostprocessEditor))
                    Actor()->Cameras().RemoveCamEffector((ECamEffectorType)eCEPostprocessEditor);

                CAnimatorCamLerpEffectorConst* e = xr_new<CAnimatorCamLerpEffectorConst>();

                e->SetType((ECamEffectorType)eCEPostprocessEditor);
                e->SetHudAffect(eff.m_bHudAffect);
                e->SetCyclic(eff.m_CamEffectCyclic);
                e->SetFactor(1.0f);
                e->SetFactorMod(factor_mod);;
                e->Start(effect_name.c_str());

                Actor()->Cameras().AddCamEffector(e);
            }

            ImGui::SameLine();
            if (ImGui::Button(toUtf8(CStringTable().translate("st_pp_editor_stop").c_str()).c_str()))
            {
                Actor()->Cameras().RemoveCamEffector((ECamEffectorType)eCEPostprocessEditor);
            }

            ImGui::PopID();
        }

        if (ImGui::Button(toUtf8(CStringTable().translate("st_pp_editor_add_cam_effect").c_str()).c_str()))
            eff.cam_effects.push_back("");

        if (ImGui::Button(toUtf8(CStringTable().translate("st_pp_editor_play_effector").c_str()).c_str()))
        {
            ApplyEffect(eff);
        }

        ImGui::SameLine();
        if (ImGui::Button(toUtf8(CStringTable().translate("st_pp_editor_stop_effector").c_str()).c_str()))
        {
            StopEffect(eff);
        }

        ImGui::SameLine();
        if (ImGui::Button(toUtf8(CStringTable().translate("st_editor_imgui_save").c_str()).c_str()))
        {
            SaveEffect(eff);
        }
    }
    ImGui::EndGroup();
}

bool PPE_Editor_MouseWheel(float wheel)
{
	ImGui::Begin(toUtf8(CStringTable().translate("st_editor_imgui_ppe").c_str()).c_str());

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