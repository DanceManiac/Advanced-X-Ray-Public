////////////////////////////////////////////////////////////////////////////
//	Module 		: embedded_editor_spawner.cpp
//	Created 	: 17.02.2024
//  Modified 	: 24.09.2025
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

#include <regex>

xr_map<xr_string, xr_string> m_ItemsVec{}, m_CarsVec{}, m_WeaponsVec{}, m_FoodVec{}, m_QuestItemsVec{}, m_DevicesVec{}, m_EntitiesVec{}, m_AnomaliesVec{}, m_ArtefactsVec{}, m_AmmoVec{}, m_OutfitVec{};
LPCSTR m_sSelectedName = nullptr, m_sSelectedSection = nullptr;
static int objects_type{10}, objects_count{1};
char* m_sSearchText = new char[512]{};

// Отображение текста с переносом
void DrawTextWithEllipsis(const char* text, float maxWidth)
{
	const char* ellipsis = "...";
	const float ellipsisWidth = ImGui::CalcTextSize(ellipsis).x;

	ImVec2 textSize = ImGui::CalcTextSize(text);

	if (textSize.x <= maxWidth)
	{
		ImGui::TextUnformatted(text);
		return;
	}

	ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + maxWidth);
	ImGui::TextUnformatted(text);
	ImGui::PopTextWrapPos();

	bool wrapped = (ImGui::GetItemRectSize().y > ImGui::GetTextLineHeight());

	if (!wrapped)
	{
		const char* end = text;

		while (*end && ImGui::CalcTextSize(text, end + 1).x <= (maxWidth - ellipsisWidth))
			end++;

		if (end > text)
		{
			xr_string truncated(text, end);
			truncated += ellipsis;
			ImGui::TextUnformatted(truncated.c_str());
		}
		else
			ImGui::TextUnformatted(ellipsis);
	}
}

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
		// Игнорируем мультиплеерные предметы
		const std::string& sectionName = sect->Name.c_str();

		if (std::regex_search(sectionName, std::regex("(^|_)mp($|_)")))
			continue;

		if (sect->line_exist("class") && sect->line_exist("$spawn"))
		{
			LPCSTR item_name = sectionName.c_str();

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
	static const std::vector<xr_map<xr_string, xr_string>*> itemLists = {
		&m_CarsVec, &m_WeaponsVec, &m_FoodVec, &m_DevicesVec,
		&m_EntitiesVec, &m_AnomaliesVec, &m_ArtefactsVec,
		&m_AmmoVec, &m_OutfitVec, &m_QuestItemsVec, &m_ItemsVec
	};

	auto& itemsList = *itemLists[mode >= 0 && mode < 10 ? mode : 10];

	// Поиск
	ImGui::InputText(toUtf8(CStringTable().translate("st_spawner_search").c_str()).c_str(), m_sSearchText, 512);
	xr_string searchTextLower = m_sSearchText;
	ToLowerUtf8RU(searchTextLower);

	const auto surfaceParams = ::Render->getSurface(EQUIPMENT_ICONS);
	const float iconsKx = UI().get_icons_kx();
	const float invGridKx = UI().inv_grid_kx();
	const float padding = 8.0f;
	const float textHeight = ImGui::GetTextLineHeight();

	float maxIconWidth = 2.0f * invGridKx / iconsKx;
	float maxIconHeight = maxIconWidth;

	for (auto& item : itemsList)
	{
		if (pSettings->line_exist(item.first.c_str(), "inv_grid_width") && pSettings->line_exist(item.first.c_str(), "inv_grid_height"))
		{
			float w = pSettings->r_float(item.first.c_str(), "inv_grid_width") * invGridKx / iconsKx;
			float h = pSettings->r_float(item.first.c_str(), "inv_grid_height") * invGridKx / iconsKx;
			maxIconWidth = std::max(maxIconWidth, w);
			maxIconHeight = std::max(maxIconHeight, h);
		}
	}

	const float cellWidth = maxIconWidth + padding * 2;
	const float cellHeight = std::max(100.0f, maxIconHeight + textHeight + padding * 3);
	const int columns = std::max(1, (int)((ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ScrollbarSize) / cellWidth));

	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { padding, padding });
	ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, { 0.1f, 0.1f, 0.1f, 0.5f });

	if (ImGui::BeginTable("ItemsGrid", columns, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit))
	{
		for (int i = 0; i < columns; i++)
			ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed, cellWidth);

		std::unordered_set<std::string> displayedItems;

		for (auto& [section, name] : itemsList)
		{
			xr_string itemNameLower = toUtf8(name.c_str()).c_str();
			ToLowerUtf8RU(itemNameLower);

			if ((m_sSearchText[0] && xr_string_find(itemNameLower, searchTextLower) == std::u32string::npos) || displayedItems.count(name.c_str()))
				continue;

			displayedItems.insert(name.c_str());
			ImGui::TableNextColumn();

			shared_str displayName = toUtf8(name.c_str()).c_str();
			const float frameWidth = maxIconWidth + padding * 2;
			const float frameHeight = maxIconHeight + padding * 2;

			ImGui::BeginChildFrame(ImGui::GetID((void*)(intptr_t)section.c_str()), { frameWidth, frameHeight }, ImGuiWindowFlags_NoScrollbar);
			{
				bool has_icon = (pSettings->line_exist(section.c_str(), "inv_grid_x") &&
					pSettings->line_exist(section.c_str(), "inv_grid_y") &&
					pSettings->line_exist(section.c_str(), "inv_grid_width") &&
					pSettings->line_exist(section.c_str(), "inv_grid_height"));

				if (has_icon)
				{
					// Отображение иконки
					float x = pSettings->r_float(section.c_str(), "inv_grid_x") * invGridKx;
					float y = pSettings->r_float(section.c_str(), "inv_grid_y") * invGridKx;
					float w = pSettings->r_float(section.c_str(), "inv_grid_width") * invGridKx;
					float h = pSettings->r_float(section.c_str(), "inv_grid_height") * invGridKx;

					ImVec2 uv0(x / surfaceParams.w, y / surfaceParams.h);
					ImVec2 uv1((x + w) / surfaceParams.w, (y + h) / surfaceParams.h);
					ImVec2 size(w / iconsKx, h / iconsKx);

					ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - size.x) * 0.5f);
					ImGui::SetCursorPosY((ImGui::GetContentRegionAvail().y - size.y) * 0.5f);
					ImGui::Image(surfaceParams.Surface, size, uv0, uv1);
				}
				else
				{
					// Элементы без иконки
					const float size = maxIconWidth;
					ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - size) * 0.5f);
					ImGui::SetCursorPosY((ImGui::GetContentRegionAvail().y - size) * 0.5f);

					const char* text = "N";
					ImVec2 textSize = ImGui::CalcTextSize(text);
					ImVec2 pos = ImGui::GetCursorScreenPos();
					pos.x += (size - textSize.x) * 0.5f;
					pos.y += (size - textSize.y) * 0.5f;

					ImGui::GetWindowDrawList()->AddText(pos, ImGui::GetColorU32({ 1,1,1,0.9f }), text);
					ImGui::Dummy({ size, size });
				}
			}
			ImGui::EndChildFrame();

			DrawTextWithEllipsis(displayName.c_str(), cellWidth - padding * 2);

			if (ImGui::IsItemClicked())
			{
				m_sSelectedName = displayName.c_str();
				m_sSelectedSection = section.c_str();
			}
		}
		ImGui::EndTable();
	}
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
}

void ShowSpawner(bool& show)
{
	ImguiWnd wnd(toUtf8(CStringTable().translate("st_spawner_name").c_str()).c_str(), &show);

	if (wnd.Collapsed)
		return;

	ImGui::BeginChild(toUtf8(CStringTable().translate("st_spawner_objects_to_spawn").c_str()).c_str(), ImVec2(700, 420), true);
		DrawObjectsList(objects_type);

	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild(toUtf8(CStringTable().translate("st_spawner_spawn_options").c_str()).c_str(), ImVec2(400, 420), true);
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