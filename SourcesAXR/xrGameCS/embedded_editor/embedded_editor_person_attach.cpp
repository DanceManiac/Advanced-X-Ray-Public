#include "stdAfx.h"
#include "embedded_editor_hud.h"
#include "embedded_editor_helper.h"
#include "imgui_internal.h"
#include "../Include/xrRender/Kinematics.h"
#include "../../xrEngine/device.h"
#include "../Actor.h"
#include "../attachable_item.h"
#include "../Inventory.h"
#include "Weapon.h"
#include "string_table.h"

void EditWeaponParameters(CWeapon* weapon, float drag_intensity)
{
	if (!weapon)
		return;

	ImGui::Separator();

	ImGui::Text(toUtf8(weapon->NameItem()).c_str());

	Fvector pos = weapon->GetHandsOffsetPos();
	Fvector ypr = weapon->GetHandsOffsetRot();

	string256 pos_name{}, orient_name{};
	strconcat(sizeof(pos_name), pos_name, weapon->m_section_id.c_str(), "_position");
	strconcat(sizeof(orient_name), orient_name, weapon->m_section_id.c_str(), "_orientation");

	ImGui::Text(toUtf8(CStringTable().translate("st_npc_attaches_editor_hands_position_params").c_str()).c_str());

	if (ImGui::DragFloat3(pos_name, (float*)&pos, drag_intensity, NULL, NULL, "%.6f"))
		weapon->SetHandsPosRot(pos, ypr);

	if (ImGui::DragFloat3(orient_name, (float*)&ypr, drag_intensity, NULL, NULL, "%.6f"))
		weapon->SetHandsPosRot(pos, ypr);

	if (weapon->can_be_strapped())
	{
		ImGui::Text(toUtf8(CStringTable().translate("st_npc_attaches_editor_strap_params").c_str()).c_str());

		static Fvector strapPos = weapon->GetStrapOffsetPos();
		static Fvector strapYpr = weapon->GetStrapOffsetRot();

		string256 strap_pos_name{}, strap_orient_name{}, alt_strap_pos_name{}, alt_strap_orient_name{}, bone0_option{}, bone1_option{};
		strconcat(sizeof(strap_pos_name), strap_pos_name, weapon->m_section_id.c_str(), "_strap_position");
		strconcat(sizeof(strap_orient_name), strap_orient_name, weapon->m_section_id.c_str(), "_strap_orientation");
		//strconcat(sizeof(alt_strap_pos_name), alt_strap_pos_name, weapon->m_section_id.c_str(), "_strap_position_alt");
		//strconcat(sizeof(alt_strap_orient_name), alt_strap_orient_name, weapon->m_section_id.c_str(), "_strap_orientation_alt");
		strconcat(sizeof(bone0_option), bone0_option, weapon->m_section_id.c_str(), "_strap_bone0");
		strconcat(sizeof(bone1_option), bone1_option, weapon->m_section_id.c_str(), "_strap_bone1");

		if (ImGui::DragFloat3(strap_pos_name, (float*)&strapPos, drag_intensity, NULL, NULL, "%.6f"))
			weapon->SetStrapPosRot(strapPos, strapYpr);

		if (ImGui::DragFloat3(strap_orient_name, (float*)&strapYpr, drag_intensity, NULL, NULL, "%.6f"))
			weapon->SetStrapPosRot(strapPos, strapYpr);

		if (IKinematics::accel* accel = smart_cast<IKinematics*>(Actor()->Visual())->LL_Bones())
		{
			std::vector<const char*> bones_names_cstr;

			bones_names_cstr.reserve(accel->size());

			for (int i = 0; i < accel->size(); ++i)
				bones_names_cstr.push_back(accel->at(i).first.c_str());

			auto it = std::find(bones_names_cstr.begin(), bones_names_cstr.end(), weapon->strap_bone0());
			auto it2 = std::find(bones_names_cstr.begin(), bones_names_cstr.end(), weapon->strap_bone1());
			static int selected_bone_1 = 0;
			static bool bone_changed_1 = false;
			static int selected_bone_2 = 0;
			static bool bone_changed_2 = false;

			if (it != bones_names_cstr.end() && !bone_changed_1)
				selected_bone_1 = std::distance(bones_names_cstr.begin(), it);

			if (ImGui::Combo(bone0_option, &selected_bone_1, bones_names_cstr.data(), bones_names_cstr.size()))
			{
				weapon->SetStrapBone0(accel->at(selected_bone_1).first.c_str());
				weapon->SetStrapBone0_ID(smart_cast<IKinematics*>(Actor()->Visual())->LL_BoneID(weapon->strap_bone0()));
				bone_changed_1 = true;
			}

			if (it != bones_names_cstr.end() && !bone_changed_2)
				selected_bone_2 = std::distance(bones_names_cstr.begin(), it2);

			if (ImGui::Combo(bone1_option, &selected_bone_2, bones_names_cstr.data(), bones_names_cstr.size()))
			{
				weapon->SetStrapBone1(accel->at(selected_bone_2).first.c_str());
				weapon->SetStrapBone1_ID(smart_cast<IKinematics*>(Actor()->Visual())->LL_BoneID(weapon->strap_bone1()));
				bone_changed_2 = true;
			}
		}

		/*static Fvector strapPosAlt = weapon->GetStrapOffsetAltPos();
		static Fvector strapYprAlt = weapon->GetStrapOffsetAltRot();

		if (ImGui::DragFloat3(alt_strap_pos_name, (float*)&strapPosAlt, drag_intensity, NULL, NULL, "%.6f"))
			weapon->SetStrapAltPosRot(strapPosAlt, strapYprAlt);

		if (ImGui::DragFloat3(alt_strap_orient_name, (float*)&strapYprAlt, drag_intensity, NULL, NULL, "%.6f"))
			weapon->SetStrapAltPosRot(strapPosAlt, strapYprAlt); */
	}
}

void EditMissileParameters(CAttachableItem* attachable, float drag_intensity)
{
	if (!attachable)
		return;

	ImGui::Separator();

	ImGui::Text(toUtf8(attachable->item().NameItem()).c_str());

	Fvector itm_offset = attachable->get_pos_offset(attachable);
	Fvector itm_rot = attachable->get_angle_offset(attachable);

	ImGui::Separator();

	string256 pos_name{}, orient_name{};
	strconcat(sizeof(pos_name), pos_name, attachable->item().m_section_id.c_str(), "_attach_position_offset");
	strconcat(sizeof(orient_name), orient_name, attachable->item().m_section_id.c_str(), "_attach_angle_offset");

	if (ImGui::DragFloat3(pos_name, (float*)&itm_offset, drag_intensity, NULL, NULL, "%.6f"))
		attachable->set_pos(attachable, itm_offset);

	if (ImGui::DragFloat3(orient_name, (float*)&itm_rot, drag_intensity, NULL, NULL, "%.6f"))
		attachable->set_rot(attachable, itm_rot);
}

void SaveToFile(CAttachmentOwner* owner)
{
	if (!owner)
		return;

	string_path fname{};
	string256 parent_section_attaches_fname = "actor_attached_items";

	FS.update_path(fname, "$app_data_root$", make_string("NPC_AttachesEditor\\%s\\%s.ltx", "Actor", parent_section_attaches_fname).c_str());

	CInifile pAttachesCfg(fname, false, false, true);

	for (int i = 0; i < owner->GetAttachedItems().size(); i++)
	{
		auto attached_item = owner->GetAttachedItems().at(i);

		pAttachesCfg.w_string(attached_item->item().m_section_id.c_str(),
			"attach_position_offset",
			make_string("%f,%f,%f", attached_item->get_pos_offset(attached_item).x, attached_item->get_pos_offset(attached_item).y, attached_item->get_pos_offset(attached_item).z)
			.c_str());

		pAttachesCfg.w_string(attached_item->item().m_section_id.c_str(),
			"attach_angle_offset",
			make_string("%f,%f,%f", attached_item->get_angle_offset(attached_item).x, attached_item->get_angle_offset(attached_item).y, attached_item->get_angle_offset(attached_item).z)
			.c_str());

		pAttachesCfg.w_string(attached_item->item().m_section_id.c_str(),
			"attach_bone_name",
			make_string("%s", attached_item->bone_name().c_str())
			.c_str());
	}

	if (CWeapon* WpnKnife = smart_cast<CWeapon*>(Actor()->inventory().ItemFromSlot(KNIFE_SLOT)))
	{
		// In hands
		pAttachesCfg.w_string(WpnKnife->m_section_id.c_str(),
			"position",
			make_string("%f,%f,%f", WpnKnife->GetHandsOffsetPos().x, WpnKnife->GetHandsOffsetPos().y, WpnKnife->GetHandsOffsetPos().z)
			.c_str());

		pAttachesCfg.w_string(WpnKnife->m_section_id.c_str(),
			"orientation",
			make_string("%f,%f,%f", WpnKnife->GetHandsOffsetRot().x, WpnKnife->GetHandsOffsetRot().y, WpnKnife->GetHandsOffsetRot().z)
			.c_str());
	}

	if (CWeapon* Wpn1 = smart_cast<CWeapon*>(Actor()->inventory().ItemFromSlot(PISTOL_SLOT)))
	{
		// In hands
		pAttachesCfg.w_string(Wpn1->m_section_id.c_str(),
			"position",
			make_string("%f,%f,%f", Wpn1->GetHandsOffsetPos().x, Wpn1->GetHandsOffsetPos().y, Wpn1->GetHandsOffsetPos().z)
			.c_str());

		pAttachesCfg.w_string(Wpn1->m_section_id.c_str(),
			"orientation",
			make_string("%f,%f,%f", Wpn1->GetHandsOffsetRot().x, Wpn1->GetHandsOffsetRot().y, Wpn1->GetHandsOffsetRot().z)
			.c_str());

		// Strap
		pAttachesCfg.w_string(Wpn1->m_section_id.c_str(),
			"strap_position",
			make_string("%f,%f,%f", Wpn1->GetStrapOffsetPos().x, Wpn1->GetStrapOffsetPos().y, Wpn1->GetStrapOffsetPos().z)
			.c_str());

		pAttachesCfg.w_string(Wpn1->m_section_id.c_str(),
			"strap_orientation",
			make_string("%f,%f,%f", Wpn1->GetStrapOffsetRot().x, Wpn1->GetStrapOffsetRot().y, Wpn1->GetStrapOffsetRot().z)
			.c_str());

		pAttachesCfg.w_string(Wpn1->m_section_id.c_str(),
			"strap_bone0",
			make_string("%s", Wpn1->strap_bone0())
			.c_str());

		pAttachesCfg.w_string(Wpn1->m_section_id.c_str(),
			"strap_bone1",
			make_string("%s", Wpn1->strap_bone1())
			.c_str());

		// Strap alt
		/*pAttachesCfg.w_string(Wpn1->m_section_id.c_str(),
			"strap_position_alt",
			make_string("%f,%f,%f", Wpn1->GetStrapOffsetAltPos().x, Wpn1->GetStrapOffsetAltPos().y, Wpn1->GetStrapOffsetAltPos().z)
			.c_str());

		pAttachesCfg.w_string(Wpn1->m_section_id.c_str(),
			"strap_orientation_alt",
			make_string("%f,%f,%f", Wpn1->GetStrapOffsetAltRot().x, Wpn1->GetStrapOffsetAltRot().y, Wpn1->GetStrapOffsetAltRot().z)
			.c_str()); */
	}

	if (CWeapon* Wpn2 = smart_cast<CWeapon*>(Actor()->inventory().ItemFromSlot(RIFLE_SLOT)))
	{
		// In hands
		pAttachesCfg.w_string(Wpn2->m_section_id.c_str(),
			"position",
			make_string("%f,%f,%f", Wpn2->GetHandsOffsetPos().x, Wpn2->GetHandsOffsetPos().y, Wpn2->GetHandsOffsetPos().z)
			.c_str());

		pAttachesCfg.w_string(Wpn2->m_section_id.c_str(),
			"orientation",
			make_string("%f,%f,%f", Wpn2->GetHandsOffsetRot().x, Wpn2->GetHandsOffsetRot().y, Wpn2->GetHandsOffsetRot().z)
			.c_str());

		// Strap
		pAttachesCfg.w_string(Wpn2->m_section_id.c_str(),
			"strap_position",
			make_string("%f,%f,%f", Wpn2->GetStrapOffsetPos().x, Wpn2->GetStrapOffsetPos().y, Wpn2->GetStrapOffsetPos().z)
			.c_str());

		pAttachesCfg.w_string(Wpn2->m_section_id.c_str(),
			"strap_orientation",
			make_string("%f,%f,%f", Wpn2->GetStrapOffsetRot().x, Wpn2->GetStrapOffsetRot().y, Wpn2->GetStrapOffsetRot().z)
			.c_str());

		pAttachesCfg.w_string(Wpn2->m_section_id.c_str(),
			"strap_bone0",
			make_string("%s", Wpn2->strap_bone0())
			.c_str());

		pAttachesCfg.w_string(Wpn2->m_section_id.c_str(),
			"strap_bone1",
			make_string("%s", Wpn2->strap_bone1())
			.c_str());

		// Strap alt
		/*pAttachesCfg.w_string(Wpn2->m_section_id.c_str(),
			"strap_position_alt",
			make_string("%f,%f,%f", Wpn2->GetStrapOffsetAltPos().x, Wpn2->GetStrapOffsetAltPos().y, Wpn2->GetStrapOffsetAltPos().z)
			.c_str());

		pAttachesCfg.w_string(Wpn2->m_section_id.c_str(),
			"strap_orientation_alt",
			make_string("%f,%f,%f", Wpn2->GetStrapOffsetAltRot().x, Wpn2->GetStrapOffsetAltRot().y, Wpn2->GetStrapOffsetAltRot().z)
			.c_str()); */
	}

	if (CWeapon* binocular = smart_cast<CWeapon*>(Actor()->inventory().ItemFromSlot(APPARATUS_SLOT)))
	{
		// In hands
		pAttachesCfg.w_string(binocular->m_section_id.c_str(),
			"position",
			make_string("%f,%f,%f", binocular->GetHandsOffsetPos().x, binocular->GetHandsOffsetPos().y, binocular->GetHandsOffsetPos().z)
			.c_str());

		pAttachesCfg.w_string(binocular->m_section_id.c_str(),
			"orientation",
			make_string("%f,%f,%f", binocular->GetHandsOffsetRot().x, binocular->GetHandsOffsetRot().y, binocular->GetHandsOffsetRot().z)
			.c_str());
	}

	if (CAttachableItem* bolt = smart_cast<CAttachableItem*>(Actor()->inventory().ItemFromSlot(BOLT_SLOT)))
	{
		pAttachesCfg.w_string(bolt->item().m_section_id.c_str(),
			"attach_position_offset",
			make_string("%f,%f,%f", bolt->get_pos_offset(bolt).x, bolt->get_pos_offset(bolt).y, bolt->get_pos_offset(bolt).z)
			.c_str());

		pAttachesCfg.w_string(bolt->item().m_section_id.c_str(),
			"attach_angle_offset",
			make_string("%f,%f,%f", bolt->get_angle_offset(bolt).x, bolt->get_angle_offset(bolt).y, bolt->get_angle_offset(bolt).z)
			.c_str());
	}

	if (CAttachableItem* grenade = smart_cast<CAttachableItem*>(Actor()->inventory().ItemFromSlot(GRENADE_SLOT)))
	{
		pAttachesCfg.w_string(grenade->item().m_section_id.c_str(),
			"attach_position_offset",
			make_string("%f,%f,%f", grenade->get_pos_offset(grenade).x, grenade->get_pos_offset(grenade).y, grenade->get_pos_offset(grenade).z)
			.c_str());

		pAttachesCfg.w_string(grenade->item().m_section_id.c_str(),
			"attach_angle_offset",
			make_string("%f,%f,%f", grenade->get_angle_offset(grenade).x, grenade->get_angle_offset(grenade).y, grenade->get_angle_offset(grenade).z)
			.c_str());
	}

	Msg("[%s] Actor attaches data saved to %s", __FUNCTION__, fname);
}

void ShowPersonAttachEditor(bool& show)
{
	ImguiWnd wnd(toUtf8(CStringTable().translate("st_editor_imgui_person_attach").c_str()).c_str(), &show);
	if (wnd.Collapsed)
		return;

	if (Actor() && Actor()->g_Alive())
	{
		if (Actor()->active_cam() != eacFreeLook)
			Actor()->cam_Set(eacFreeLook);

		static float drag_intensity = 0.001f;

		ImGui::DragFloat(toUtf8(CStringTable().translate("st_editor_imgui_drag_intensity").c_str()).c_str(), &drag_intensity, 0.000001f, 0.000001f, 1.0f, "%.6f");

		if (CAttachmentOwner* owner = smart_cast<CAttachmentOwner*>(Actor()))
		{
			for (int i = 0; i < owner->GetAttachedItems().size(); i++)
			{
				auto attached_item = owner->GetAttachedItems().at(i);

				Fvector itm_offset = attached_item->get_pos_offset(attached_item);
				Fvector itm_rot = attached_item->get_angle_offset(attached_item);

				ImGui::Separator();

				ImGui::Text(toUtf8(attached_item->item().NameItem()).c_str());
				
				ImGui::PushID(i);

				if (ImGui::DragFloat3("attach_position_offset", (float*)&itm_offset, drag_intensity, NULL, NULL, "%.6f"))
					attached_item->set_pos(attached_item, itm_offset);
				
				if (ImGui::DragFloat3("attach_angle_offset", (float*)&itm_rot, drag_intensity, NULL, NULL, "%.6f"))
					attached_item->set_rot(attached_item, itm_rot);

				if (IKinematics::accel* accel = smart_cast<IKinematics*>(Actor()->Visual())->LL_Bones())
				{
					xr_vector<const char*> bones_names_cstr;

					bones_names_cstr.reserve(accel->size());

					for (int j = 0; j < accel->size(); ++j)
						bones_names_cstr.push_back(accel->at(j).first.c_str());

					auto it = std::find(bones_names_cstr.begin(), bones_names_cstr.end(), attached_item->bone_name());

					static std::vector<int> selected_bones(owner->GetAttachedItems().size(), 0);
					static std::vector<bool> is_changed(owner->GetAttachedItems().size(), false);

					if (it != bones_names_cstr.end() && !is_changed[i])
						selected_bones[i] = std::distance(bones_names_cstr.begin(), it);

					if (ImGui::Combo("attach_bone_name", &selected_bones[i], bones_names_cstr.data(), bones_names_cstr.size()))
					{
						attached_item->set_bone_id(accel->at(selected_bones[i]).second);
						is_changed[i] = true;
					}
				}

				ImGui::PopID();
			}

			if (CWeapon* WpnKnife = smart_cast<CWeapon*>(Actor()->inventory().ItemFromSlot(KNIFE_SLOT)))
				EditWeaponParameters(WpnKnife, drag_intensity);

			if (CWeapon* Wpn1 = smart_cast<CWeapon*>(Actor()->inventory().ItemFromSlot(PISTOL_SLOT)))
				EditWeaponParameters(Wpn1, drag_intensity);

			if (CWeapon* Wpn2 = smart_cast<CWeapon*>(Actor()->inventory().ItemFromSlot(RIFLE_SLOT)))
				EditWeaponParameters(Wpn2, drag_intensity);

			if (CWeapon* binocular = smart_cast<CWeapon*>(Actor()->inventory().ItemFromSlot(APPARATUS_SLOT)))
				EditWeaponParameters(binocular, drag_intensity);

			if (CAttachableItem* bolt = smart_cast<CAttachableItem*>(Actor()->inventory().ItemFromSlot(BOLT_SLOT)))
				EditMissileParameters(bolt, drag_intensity);

			if (CAttachableItem* grenade = smart_cast<CAttachableItem*>(Actor()->inventory().ItemFromSlot(GRENADE_SLOT)))
				EditMissileParameters(grenade, drag_intensity);

			ImGui::Separator();

			if (ImGui::Button(toUtf8(CStringTable().translate("st_editor_imgui_save").c_str()).c_str()))
				SaveToFile(owner);
		}
	}
}

bool PersonAttachEditor_MouseWheel(float wheel)
{
	ImGui::Begin(toUtf8(CStringTable().translate("st_editor_imgui_person_attach").c_str()).c_str());

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