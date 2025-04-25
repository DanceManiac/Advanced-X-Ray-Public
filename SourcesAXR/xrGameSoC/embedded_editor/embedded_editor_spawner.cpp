////////////////////////////////////////////////////////////////////////////
//	Module 		: embedded_editor_spawner.cpp
//	Created 	: 17.02.2024
//  Modified 	: 17.02.2024
//	Author		: Dance Maniac (M.F.S. Team)
//	Description : ImGui Spawn Menu
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "embedded_editor_spawner.h"
#include "embedded_editor_helper.h"
#include <imgui.h>
#include "imgui_internal.h"

#include "Actor.h"
#include "CustomZone.h"
#include "clsid_game.h"
#include "alife_simulator.h"
#include "game_sv_single.h"
#include "ai_object_location.h"
#include "xrServer_Objects_ALife_Monsters.h"
#include "string_table.h"
#include "../ui/UIInventoryUtilities.h"
#include "../ui_base.h"

xr_map<xr_string, xr_string> m_ItemsVec{}, m_CarsVec{}, m_WeaponsVec{}, m_FoodVec{}, m_QuestItemsVec{}, m_DevicesVec{}, m_EntitiesVec{}, m_AnomaliesVec{}, m_ArtefactsVec{}, m_AmmoVec{}, m_OutfitVec{};
LPCSTR m_sSelectedName = nullptr, m_sSelectedSection = nullptr;
static int objects_type{10}, objects_count{1};
char* m_sSearchText = new char[512]{};

bool IsWeapon(CLASS_ID cls)
{
	std::vector<CLASS_ID> validClsIds = { TEXT2CLSID("WP_SCOPE"),TEXT2CLSID("WP_SILEN"),TEXT2CLSID("WP_GLAUN"),TEXT2CLSID("WP_LASER"),TEXT2CLSID("WP_TORCH"),TEXT2CLSID("WP_KNIFE"),TEXT2CLSID("WP_BM16"),TEXT2CLSID("WP_GROZA"),TEXT2CLSID("WP_SVD"),TEXT2CLSID("WP_AK74"),TEXT2CLSID("WP_LR300"),TEXT2CLSID("WP_HPSA"),TEXT2CLSID("WP_PM"),TEXT2CLSID("WP_RG6"),TEXT2CLSID("WP_RPG7"),TEXT2CLSID("WP_SHOTG"),TEXT2CLSID("WP_ASHTG"),TEXT2CLSID("WP_MAGAZ"),TEXT2CLSID("WP_SVU"),TEXT2CLSID("WP_USP45"),TEXT2CLSID("WP_VAL"),TEXT2CLSID("WP_VINT"),TEXT2CLSID("WP_WALTH"),TEXT2CLSID("W_STMGUN"),TEXT2CLSID("G_F1_S"),TEXT2CLSID("G_RGD5_S") };

	for (CLASS_ID validClsId : validClsIds)
	{
		if (cls == validClsId)
			return true;
	}

	return false;
}

bool IsEntity(CLASS_ID cls)
{
	std::vector<CLASS_ID> validClsIds = { TEXT2CLSID("SM_BLOOD"),TEXT2CLSID("SM_BOARW"),TEXT2CLSID("SM_BLOOD"),TEXT2CLSID("SM_DOG_S"),TEXT2CLSID("SM_FLESH"),TEXT2CLSID("SM_P_DOG"),TEXT2CLSID("SM_BURER"),TEXT2CLSID("SM_CAT_S"),TEXT2CLSID("SM_CHIMS"),TEXT2CLSID("SM_CONTR"),TEXT2CLSID("SM_IZLOM"),TEXT2CLSID("SM_POLTR"),TEXT2CLSID("SM_GIANT"),TEXT2CLSID("SM_ZOMBI"),TEXT2CLSID("SM_SNORK"),TEXT2CLSID("SM_TUSHK"),TEXT2CLSID("SM_DOG_P"),TEXT2CLSID("SM_DOG_F"),TEXT2CLSID("AI_STL_S"),TEXT2CLSID("AI_TRD_S"),CLSID_AI_CROW };

	for (CLASS_ID validClsId : validClsIds)
	{
		if (cls == validClsId)
			return true;
	}

	return false;
}

bool IsCustomZone(CLASS_ID cls)
{
	std::vector<CLASS_ID> validClsIds = { CLSID_ZONE,CLSID_Z_MBALD,CLSID_Z_MINCER,CLSID_Z_ACIDF,CLSID_Z_GALANT,CLSID_Z_RADIO,CLSID_Z_BFUZZ,CLSID_Z_RUSTYH,CLSID_Z_DEAD,CLSID_Z_TORRID,CLSID_Z_NOGRAVITY,TEXT2CLSID("ZS_BFUZZ"),TEXT2CLSID("ZS_MBALD"),TEXT2CLSID("ZS_GALAN"),TEXT2CLSID("ZS_MINCE") };

	for (CLASS_ID validClsId : validClsIds)
	{
		if (cls == validClsId)
			return true;
	}

	return false;
}

bool IsVehicle(CLASS_ID cls)
{
	std::vector<CLASS_ID> validClsIds = { TEXT2CLSID("SCRPTCAR"),TEXT2CLSID("C_NIVA"),TEXT2CLSID("C_HLCP_S"),CLSID_CAR };

	for (CLASS_ID validClsId : validClsIds)
	{
		if (cls == validClsId)
			return true;
	}

	return false;
}

bool IsArtefact(CLASS_ID cls)
{
	std::vector<CLASS_ID> validClsIds = { TEXT2CLSID("SCRPTART"),CLSID_AF_MERCURY_BALL,CLSID_AF_GRAVI,CLSID_AF_BLACKDROPS,CLSID_AF_NEEDLES,CLSID_AF_BAST,CLSID_AF_BLACK_GRAVI,CLSID_AF_DUMMY,CLSID_AF_ZUDA,CLSID_AF_THORN,CLSID_AF_FADED_BALL,CLSID_AF_ELECTRIC_BALL,CLSID_AF_RUSTY_HAIR,CLSID_AF_GALANTINE,CLSID_ARTEFACT };

	for (CLASS_ID validClsId : validClsIds)
	{
		if (cls == validClsId)
			return true;
	}

	return false;
}

bool IsOutfit(CLASS_ID cls)
{
	std::vector<CLASS_ID> validClsIds = { TEXT2CLSID("E_STLK"),TEXT2CLSID("E_HLMET"),TEXT2CLSID("E_BKPK") };

	for (CLASS_ID validClsId : validClsIds)
	{
		if (cls == validClsId)
			return true;
	}

	return false;
}

bool IsAmmo(CLASS_ID cls)
{
	std::vector<CLASS_ID> validClsIds = { TEXT2CLSID("AMMO_S"),TEXT2CLSID("S_VOG25"),TEXT2CLSID("S_OG7B"),TEXT2CLSID("S_M209"),TEXT2CLSID("S_EXPLO"),CLSID_OBJECT_AMMO,CLSID_OBJECT_A_VOG25,CLSID_OBJECT_A_OG7B,CLSID_OBJECT_A_M209 };

	for (CLASS_ID validClsId : validClsIds)
	{
		if (cls == validClsId)
			return true;
	}

	return false;
}

bool IsFood(CLASS_ID cls)
{
	std::vector<CLASS_ID> validClsIds = { TEXT2CLSID("S_MEDKI"),TEXT2CLSID("S_BANDG"),TEXT2CLSID("S_ANTIR"),TEXT2CLSID("S_FOOD"),TEXT2CLSID("S_BOTTL"),TEXT2CLSID("S_FOOD"),CLSID_IITEM_MEDKIT,CLSID_IITEM_BANDAGE,CLSID_IITEM_ANTIRAD,CLSID_IITEM_FOOD,CLSID_IITEM_BOTTLE };

	for (CLASS_ID validClsId : validClsIds)
	{
		if (cls == validClsId)
			return true;
	}

	return false;
}

bool IsDevice(CLASS_ID cls)
{
	std::vector<CLASS_ID> validClsIds = { TEXT2CLSID("TORCH_S"),TEXT2CLSID("DET_SCIE"),TEXT2CLSID("DET_ELIT"),TEXT2CLSID("DET_ADVA"),TEXT2CLSID("DET_SIMP"),TEXT2CLSID("S_BATTE"),TEXT2CLSID("DET_ANOM"),TEXT2CLSID("WP_BINOC"),TEXT2CLSID("S_FILTE"),TEXT2CLSID("S_REPKI"),TEXT2CLSID("S_AFCON"),TEXT2CLSID("S_SLBAG"),TEXT2CLSID("S_PDA"),CLSID_DETECTOR_SIMPLE,CLSID_DEVICE_PDA,CLSID_DEVICE_AF_MERGER,CLSID_IITEM_BOLT };

	for (CLASS_ID validClsId : validClsIds)
	{
		if (cls == validClsId)
			return true;
	}

	return false;
}

void FillSectionsList()
{
	for (auto sect : pSettings->sections())
	{
		if (sect->line_exist("class") && sect->line_exist("$spawn"))
		{
			LPCSTR item_name = sect->Name.c_str();

			if (pSettings->line_exist(item_name, "inv_name") && CStringTable().translate(pSettings->r_string(item_name, "inv_name")).size())
				item_name = CStringTable().translate(pSettings->r_string(item_name, "inv_name")).c_str();

			if (sect->line_exist("quest_item") && pSettings->r_bool(sect->Name.c_str(), "quest_item"))
			{
				m_QuestItemsVec.insert(std::make_pair(sect->Name.c_str(), item_name));
			}
			else
			{
				if (IsVehicle(pSettings->r_clsid(sect->Name.c_str(), "class")))
					m_CarsVec.insert(std::make_pair(sect->Name.c_str(), item_name));
				else if (IsEntity(pSettings->r_clsid(sect->Name.c_str(), "class")))
					m_EntitiesVec.insert(std::make_pair(sect->Name.c_str(), item_name));
				else if (IsArtefact(pSettings->r_clsid(sect->Name.c_str(), "class")))
					m_ArtefactsVec.insert(std::make_pair(sect->Name.c_str(), item_name));
				else if (IsWeapon(pSettings->r_clsid(sect->Name.c_str(), "class")))
					m_WeaponsVec.insert(std::make_pair(sect->Name.c_str(), item_name));
				else if (IsCustomZone(pSettings->r_clsid(sect->Name.c_str(), "class")))
					m_AnomaliesVec.insert(std::make_pair(sect->Name.c_str(), item_name));
				else if (IsFood(pSettings->r_clsid(sect->Name.c_str(), "class")))
					m_FoodVec.insert(std::make_pair(sect->Name.c_str(), item_name));
				else if (IsOutfit(pSettings->r_clsid(sect->Name.c_str(), "class")))
					m_OutfitVec.insert(std::make_pair(sect->Name.c_str(), item_name));
				else if (IsAmmo(pSettings->r_clsid(sect->Name.c_str(), "class")))
					m_AmmoVec.insert(std::make_pair(sect->Name.c_str(), item_name));
				else if (IsDevice(pSettings->r_clsid(sect->Name.c_str(), "class")))
					m_DevicesVec.insert(std::make_pair(sect->Name.c_str(), item_name));
				else
					m_ItemsVec.insert(std::make_pair(sect->Name.c_str(), item_name));
			}
		}
	}
}

void DrawObjectsList(int mode)
{
	ImGui::InputText(toUtf8(CStringTable().translate("st_spawner_search").c_str()).c_str(), m_sSearchText, 512);

	ImGui::BeginListBox(toUtf8(CStringTable().translate("st_spawner_items_list").c_str()).c_str(), ImVec2(300, 400));

	xr_string searchTextLower = m_sSearchText;
	ToLowerUtf8RU(searchTextLower);

	auto& itemsList = [&]() -> xr_map<xr_string, xr_string>&
	{
		switch (mode)
		{
			case 0: return m_CarsVec;
			case 1: return m_WeaponsVec;
			case 2: return m_FoodVec;
			case 3: return m_DevicesVec;
			case 4: return m_EntitiesVec;
			case 5: return m_AnomaliesVec;
			case 6: return m_ArtefactsVec;
			case 7: return m_AmmoVec;
			case 8: return m_OutfitVec;
			case 9: return m_QuestItemsVec;
			default: return m_ItemsVec;
		}
	} ();

	std::unordered_set<std::string> displayedItems;

	for (auto it = itemsList.begin(); it != itemsList.end(); ++it)
	{
		const char* t = it->second.c_str();

		xr_string itemNameLower = toUtf8(it->second.c_str()).c_str();
		ToLowerUtf8RU(itemNameLower);

		if (m_sSearchText[0] == '\0' || xr_string_find(itemNameLower, searchTextLower) != std::u32string::npos)
		{
			if (displayedItems.find(it->second.c_str()) == displayedItems.end())
			{
				shared_str itemNameUtf8 = toUtf8(it->second.c_str()).c_str();

				if (ImGui::Selectable(itemNameUtf8.c_str()))
				{
					m_sSelectedName = itemNameUtf8.c_str();
					m_sSelectedSection = it->first.c_str();
				}

				displayedItems.insert(it->second.c_str());
			}
		}
	}

	ImGui::EndListBox();
}

void ShowSpawner(bool& show)
{
	ImguiWnd wnd(toUtf8(CStringTable().translate("st_spawner_name").c_str()).c_str(), &show);

	if (wnd.Collapsed)
		return;

	ImGui::BeginChild(toUtf8(CStringTable().translate("st_spawner_objects_to_spawn").c_str()).c_str(), ImVec2(300, 420), true);
		DrawObjectsList(objects_type);

	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild(toUtf8(CStringTable().translate("st_spawner_spawn_options").c_str()).c_str(), ImVec2(400, 400), true);
	{
		//Название 
		if (m_sSelectedName)
		{
			ImGui::SetWindowFontScale(1.5f);
			ImGui::Text(m_sSelectedName);
			ImGui::SetWindowFontScale(1.0f);
		}

		//Иконка
		if (m_sSelectedSection)
		{
			bool m_bHasIcon = (pSettings->line_exist(m_sSelectedSection, "inv_grid_x") &&
				pSettings->line_exist(m_sSelectedSection, "inv_grid_y") &&
				pSettings->line_exist(m_sSelectedSection, "inv_grid_width") &&
				pSettings->line_exist(m_sSelectedSection, "inv_grid_height"));

			if (m_bHasIcon)
			{
				const auto surfaceParams = ::Render->getSurface(EQUIPMENT_ICONS);

				float x = pSettings->r_float(m_sSelectedSection, "inv_grid_x") * UI().inv_grid_kx();
				float y = pSettings->r_float(m_sSelectedSection, "inv_grid_y") * UI().inv_grid_kx();
				float w = pSettings->r_float(m_sSelectedSection, "inv_grid_width") * UI().inv_grid_kx();
				float h = pSettings->r_float(m_sSelectedSection, "inv_grid_height") * UI().inv_grid_kx();

				ImGui::Image(surfaceParams.Surface, { w * (1 / UI().get_icons_kx()), h * (1 / UI().get_icons_kx()) }, { x / surfaceParams.w, y / surfaceParams.h }, { (x + w) / surfaceParams.w, (y + h) / surfaceParams.h });
			}
		}

		//Описание
		if (m_sSelectedSection && pSettings->line_exist(m_sSelectedSection, "description"))
		{
			LPCSTR item_descr{};

			if (CStringTable().translate(pSettings->r_string(m_sSelectedSection, "description")).size())
				item_descr = CStringTable().translate(pSettings->r_string(m_sSelectedSection, "description")).c_str();

			if (item_descr)
				ImGui::TextWrapped(toUtf8(item_descr).c_str());
		}

		ImGui::Separator();

		//Количество
		ImGui::SliderInt(toUtf8(CStringTable().translate("st_spawner_spawn_count").c_str()).c_str(), (int*)&objects_count, 1, 50);

		if (ImGui::Button(toUtf8(CStringTable().translate("st_spawner_spawn_button").c_str()).c_str()))
		{
			if (!m_sSelectedSection)
				return;

			collide::rq_result RQ = Level().GetPickResult(Device.vCameraPosition, Device.vCameraDirection, 1000.0f, Level().CurrentControlEntity());
			Fvector pos = Fvector(Device.vCameraPosition).add(Fvector(Device.vCameraDirection).mul(RQ.range));

			if (auto tpGame = smart_cast<game_sv_Single*>(Level().Server->game))
			{
				for (int i = 0; i < objects_count; ++i)
				{
					CSE_Abstract* entity = tpGame->alife().spawn_item(m_sSelectedSection, pos, Actor()->ai_location().level_vertex_id(), Actor()->ai_location().game_vertex_id(), ALife::_OBJECT_ID(-1));

					if (CSE_ALifeAnomalousZone* anom = smart_cast<CSE_ALifeAnomalousZone*>(entity))
					{
						CShapeData::shape_def _shape;
						_shape.data.sphere.P.set(0.0f, 0.0f, 0.0f);
						_shape.data.sphere.R = 3.0f;
						_shape.type = CShapeData::cfSphere;
						anom->assign_shapes(&_shape, 1);
						anom->m_space_restrictor_type = RestrictionSpace::eRestrictorTypeNone;
					}
				}
			}
		}

		ImGui::SameLine();

		if (ImGui::Button(toUtf8(CStringTable().translate("st_spawner_spawn_to_inv_button").c_str()).c_str()))
		{
			if (!m_sSelectedSection)
				return;

			if (!pSettings->line_exist(m_sSelectedSection, "class") || !pSettings->line_exist(m_sSelectedSection, "inv_weight") || !pSettings->line_exist(m_sSelectedSection, "visual"))
				return;

			for (int i = 0; i < objects_count; ++i)
				Level().spawn_item(m_sSelectedSection, Actor()->Position(), false, Actor()->ID());
		}
	}
	ImGui::EndChild();

	static std::vector<xr_string> items_types = {
	toUtf8(CStringTable().translate("st_spawner_cars").c_str()).c_str(),
	toUtf8(CStringTable().translate("st_spawner_weapons").c_str()).c_str(),
	toUtf8(CStringTable().translate("st_spawner_food").c_str()).c_str(),
	toUtf8(CStringTable().translate("st_spawner_devices").c_str()).c_str(),
	toUtf8(CStringTable().translate("st_spawner_entities").c_str()).c_str(),
	toUtf8(CStringTable().translate("st_spawner_anomalies").c_str()).c_str(),
	toUtf8(CStringTable().translate("st_spawner_artefacts").c_str()).c_str(),
	toUtf8(CStringTable().translate("st_spawner_ammo").c_str()).c_str(),
	toUtf8(CStringTable().translate("st_spawner_outfit").c_str()).c_str(),
	toUtf8(CStringTable().translate("st_spawner_quest_items").c_str()).c_str(),
	toUtf8(CStringTable().translate("st_spawner_other_items").c_str()).c_str()
	};

	std::vector<const char*> items_types_cstr;
	items_types_cstr.reserve(items_types.size());

	for (const auto& str : items_types)
		items_types_cstr.push_back(str.c_str());

	ImGui::SetNextItemWidth(300);
	ImGui::SetNextWindowPos(ImVec2(0.0f, 420.0f), ImGuiCond_Once);

	if (ImGui::Combo(toUtf8(CStringTable().translate("st_spawner_objects_type").c_str()).c_str(), &objects_type, items_types_cstr.data(), items_types_cstr.size())) {}
}