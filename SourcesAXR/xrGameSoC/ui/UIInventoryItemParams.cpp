////////////////////////////////////////////////////////////////////////////
//	Module 		: UIInventoryItemParams.cpp
//	Created 	: 08.04.2021
//  Modified 	: 21.05.2021
//	Author		: Dance Maniac (M.F.S. Team)
//	Description : Inventory Item Window Class
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIInventoryItemParams.h"
#include "UIStatic.h"
#include "object_broker.h"
#include "../EntityCondition.h"
#include "..\actor.h"
#include "../ActorCondition.h"
#include "UIXmlInit.h"
#include "UIHelper.h"
#include "../string_table.h"
#include "../Inventory.h"
#include "clsid_game.h"
//#include "UIActorMenu.h"
#include "UIInventoryUtilities.h"

#include "../Torch.h"
#include "../CustomDetector.h"
//#include "../AnomalyDetector.h"
//#include "../ArtefactContainer.h"
#include "../CustomBackpack.h"
#include "../AntigasFilter.h"
#include "../RepairKit.h"
#include "../AdvancedXrayGameConstants.h"

CUIInventoryItem::CUIInventoryItem()
{
	m_af_radius			= nullptr;
	m_af_vis_radius		= nullptr;
	m_charge_level		= nullptr;
	m_filter_cond		= nullptr;
	m_rep_kit_cond		= nullptr;
	m_max_charge		= nullptr;
	m_uncharge_speed	= nullptr;
	m_artefacts_count	= nullptr;
	m_additional_weight = nullptr;
	m_inv_capacity		= nullptr;

	m_iMaxAfCount		= 1;
	m_stArtefactsScale	= 1.0f;
}

CUIInventoryItem::~CUIInventoryItem()
{
	xr_delete(m_af_radius);
	xr_delete(m_af_vis_radius);
	xr_delete(m_charge_level);
	xr_delete(m_filter_cond);
	xr_delete(m_rep_kit_cond);
	xr_delete(m_max_charge);
	xr_delete(m_uncharge_speed);
	xr_delete(m_artefacts_count);
	xr_delete(m_additional_weight);
	xr_delete(m_inv_capacity);
	xr_delete(m_Prop_line);

	m_textArtefacts.clear();
	m_stArtefacts.clear();
}

LPCSTR item_influence_caption[] =
{
	"ui_inv_af_radius",
	"ui_inv_af_vis_radius",
	"ui_inv_charge_level",
	"ui_inv_max_charge",
	"ui_inv_uncharge_speed",
	"ui_inv_artefacts_count",
	"ui_inv_weight",
	"ui_inv_inventory_capacity"
};

void CUIInventoryItem::InitFromXml(CUIXml& xml)
{
	LPCSTR base = "inventory_items_info";
	XML_NODE* stored_root = xml.GetLocalRoot();
	XML_NODE* base_node = xml.NavigateToNode(base, 0);
	if (!base_node)
		return;

	CUIXmlInit::InitWindow(xml, base, 0, this);
	xml.SetLocalRoot(base_node);

	m_Prop_line = xr_new<CUIStatic>();
	AttachChild(m_Prop_line);
	m_Prop_line->SetAutoDelete(false);
	CUIXmlInit::InitStatic(xml, "prop_line", 0, m_Prop_line);

	m_charge_level = xr_new<CUIInventoryItemInfo>();
	m_charge_level->Init(xml, "charge_level");
	m_charge_level->SetAutoDelete(false);
	LPCSTR name = CStringTable().translate("ui_inv_battery").c_str();
	m_charge_level->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_filter_cond = xr_new<CUIInventoryItemInfo>();
	m_filter_cond->Init(xml, "filter_condition");
	m_filter_cond->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_filter_condition").c_str();
	m_filter_cond->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_rep_kit_cond = xr_new<CUIInventoryItemInfo>();
	m_rep_kit_cond->Init(xml, "repair_kit_condition");
	m_rep_kit_cond->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_repair_kit_condition").c_str();
	m_rep_kit_cond->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_af_radius = xr_new<CUIInventoryItemInfo>();
	m_af_radius->Init(xml, "af_radius");
	m_af_radius->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_af_radius").c_str();
	m_af_radius->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_af_vis_radius = xr_new<CUIInventoryItemInfo>();
	m_af_vis_radius->Init(xml, "af_vis_radius");
	m_af_vis_radius->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_af_vis_radius").c_str();
	m_af_vis_radius->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_max_charge = xr_new<CUIInventoryItemInfo>();
	m_max_charge->Init(xml, "max_charge");
	m_max_charge->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_max_charge").c_str();
	m_max_charge->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_uncharge_speed = xr_new<CUIInventoryItemInfo>();
	m_uncharge_speed->Init(xml, "uncharge_speed");
	m_uncharge_speed->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_uncharge_speed").c_str();
	m_uncharge_speed->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_artefacts_count = xr_new<CUIInventoryItemInfo>();
	m_artefacts_count->Init(xml, "container_size");
	m_artefacts_count->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_artefacts_count").c_str();
	m_artefacts_count->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_additional_weight = xr_new<CUIInventoryItemInfo>();
	m_additional_weight->Init(xml, "additional_weight");
	m_additional_weight->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_weight").c_str();
	m_additional_weight->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_inv_capacity = xr_new<CUIInventoryItemInfo>();
	m_inv_capacity->Init(xml, "inventory_capacity");
	m_inv_capacity->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_inventory_capacity").c_str();
	m_inv_capacity->SetCaption(name);
	xml.SetLocalRoot(base_node);

	if (xml.NavigateToNode("artefacts_inside", 0))
	{
		m_iMaxAfCount = xml.ReadAttribInt("artefacts_inside", 0, "max_af_count", 1);
		m_iIncWndHeight = xml.ReadAttribInt("artefacts_inside", 0, "inc_height", 2);
		m_stArtefactsScale = xml.ReadAttribFlt("artefacts_inside", 0, "scale", 1.0f);
	}

	for (int i = 0; i < m_iMaxAfCount; ++i)
	{
		string256 capTextName{};
		string256 staticTextName{};

		strconcat(sizeof(capTextName), capTextName, "artefacts_inside:cap_artefacts_stats_", std::to_string(i).c_str());

		m_textArtefacts.push_back(xr_new<CUIStatic>());
		AttachChild(m_textArtefacts[i]);
		m_textArtefacts[i]->SetAutoDelete(false);
		CUIXmlInit::InitStatic(xml, capTextName, 0, m_textArtefacts[i]);

		strconcat(sizeof(staticTextName), staticTextName, "artefacts_inside:static_artefact_", std::to_string(i).c_str());

		m_stArtefacts.push_back(xr_new<CUIStatic>());
		AttachChild(m_stArtefacts[i]);
		m_stArtefacts[i]->SetAutoDelete(false);
		CUIXmlInit::InitStatic(xml, staticTextName, 0, m_stArtefacts[i]);

		xml.SetLocalRoot(base_node);
	}
	xml.SetLocalRoot(stored_root);
}

void CUIInventoryItem::SetInfo(CInventoryItem& pInvItem)
{
	DetachAll();
	AttachChild(m_Prop_line);

	CActor* actor = smart_cast<CActor*>(Level().CurrentViewEntity());

	if (!actor)
		return;

	shared_str section = pInvItem.object().cNameSect();
	CCustomDetector* pDet = smart_cast<CCustomDetector*>(&pInvItem);
	//CDetectorAnomaly* pAnomDet = smart_cast<CDetectorAnomaly*>(&pInvItem);
	CTorch* pTorch = smart_cast<CTorch*>(&pInvItem);
	//CArtefactContainer* pAfContainer = smart_cast<CArtefactContainer*>(&pInvItem);
	CCustomBackpack* pBackpack = smart_cast<CCustomBackpack*>(&pInvItem);
	CBattery* pBattery = smart_cast<CBattery*>(&pInvItem);
	CAntigasFilter* pFilter = smart_cast<CAntigasFilter*>(&pInvItem);
	CRepairKit* pKit = smart_cast<CRepairKit*>(&pInvItem);

	float val = 0.0f, max_val = 1.0f;
	Fvector2 pos;
	float h = m_Prop_line->GetWndPos().y + m_Prop_line->GetWndSize().y;

	bool ShowChargeTorch = GameConstants::GetTorchHasBattery();
	bool ShowChargeArtDet = GameConstants::GetArtDetectorUseBattery();
	bool ShowChargeAnomDet = GameConstants::GetAnoDetectorUseBattery();
	
	if (ShowChargeTorch && pTorch || ShowChargeArtDet && pDet /*|| ShowChargeAnomDet && pAnomDet*/ || pBattery)
	{
		if (pBattery)
			val = pBattery->GetCurrentChargeLevel() <= 0 ? 0 : pBattery->GetCurrentChargeLevel() * 100.f;
		else
			val = pInvItem.GetChargeToShow() <= 0 ? 0 : pInvItem.GetChargeToShow() * 100.f;
		// (!fis_zero(val))
		{
			m_charge_level->SetValue(val, 0, 0);
			pos.set(m_charge_level->GetWndPos());
			pos.y = h;
			m_charge_level->SetWndPos(pos);

			h += m_charge_level->GetWndSize().y;
			AttachChild(m_charge_level);
		}
	}

	if (ShowChargeTorch && pTorch || ShowChargeArtDet && pDet /*|| ShowChargeAnomDet && pAnomDet*/)
	{
		val = pInvItem.GetMaxChargeLevel();
		//if (!fis_zero(val))
		{
			m_max_charge->SetValue(val);
			pos.set(m_max_charge->GetWndPos());
			pos.y = h;
			m_max_charge->SetWndPos(pos);

			h += m_max_charge->GetWndSize().y;
			AttachChild(m_max_charge);
		}
	}

	if (ShowChargeTorch && pTorch || ShowChargeArtDet && pDet /*|| ShowChargeAnomDet && pAnomDet*/)
	{
		val = pInvItem.GetUnChargeLevel();
		if (!fis_zero(val))
		{
			m_uncharge_speed->SetValue(val);
			pos.set(m_uncharge_speed->GetWndPos());
			pos.y = h;
			m_uncharge_speed->SetWndPos(pos);

			h += m_uncharge_speed->GetWndSize().y;
			AttachChild(m_uncharge_speed);
		}
	}

	if (pFilter)
	{
		val = pFilter->GetFilterCondition() <= 0 ? 0.f : pFilter->GetFilterCondition() * 100.f;
		// (!fis_zero(val))
		{
			m_filter_cond->SetValue(val, 0, 0);
			pos.set(m_filter_cond->GetWndPos());
			pos.y = h;
			m_filter_cond->SetWndPos(pos);

			h += m_filter_cond->GetWndSize().y;
			AttachChild(m_filter_cond);
		}
	}

	if (pKit)
	{
		val = pKit->GetRepairKitCondition() <= 0 ? 0.f : pKit->GetRepairKitCondition() * 100.f;
		// (!fis_zero(val))
		{
			m_rep_kit_cond->SetValue(val, 0, 0);
			pos.set(m_rep_kit_cond->GetWndPos());
			pos.y = h;
			m_rep_kit_cond->SetWndPos(pos);

			h += m_rep_kit_cond->GetWndSize().y;
			AttachChild(m_rep_kit_cond);
		}
	}

	/*if (pDet)
	{
		val = pDet->GetAfDetectRadius();
		if (!fis_zero(val))
		{
			m_af_radius->SetValue(val, 0, 2);
			pos.set(m_af_radius->GetWndPos());
			pos.y = h;
			m_af_radius->SetWndPos(pos);

			h += m_af_radius->GetWndSize().y;
			AttachChild(m_af_radius);
		}

		val = pDet->GetAfVisRadius();
		if (!fis_zero(val))
		{
			m_af_vis_radius->SetValue(val, 0, 2);
			pos.set(m_af_vis_radius->GetWndPos());
			pos.y = h;
			m_af_vis_radius->SetWndPos(pos);

			h += m_af_vis_radius->GetWndSize().y;
			AttachChild(m_af_vis_radius);
		}
	}*/

	if (pDet) // For anomaly detectors
	{
		val = pDet->GetVisRadius();
		if (!fis_zero(val))
		{
			m_af_vis_radius->SetValue(val, 0, 2);
			pos.set(m_af_vis_radius->GetWndPos());
			pos.y = h;
			m_af_vis_radius->SetWndPos(pos);

			h += m_af_vis_radius->GetWndSize().y;
			AttachChild(m_af_vis_radius);
		}
	}

	if (pBackpack)
	{
		val = pBackpack->m_additional_weight;
		if (!fis_zero(val))
		{
			m_additional_weight->SetValue(val, 2, 2);

			pos.set(m_additional_weight->GetWndPos());
			pos.y = h;
			m_additional_weight->SetWndPos(pos);

			h += m_additional_weight->GetWndSize().y;
			AttachChild(m_additional_weight);
		}
	}

	if (GameConstants::GetLimitedInventory() && pBackpack)
	{
		val = pBackpack->GetInventoryCapacity();
		if (!fis_zero(val))
		{
			m_inv_capacity->SetValue(val, 2, 2);

			pos.set(m_inv_capacity->GetWndPos());
			pos.y = h;
			m_inv_capacity->SetWndPos(pos);

			h += m_inv_capacity->GetWndSize().y;
			AttachChild(m_inv_capacity);
		}
	}

	/*if (pAfContainer)
	{
		val = pAfContainer->GetContainerSize();
		if (!fis_zero(val))
		{
			m_artefacts_count->SetValue(val);
			pos.set(m_artefacts_count->GetWndPos());
			pos.y = h;
			m_artefacts_count->SetWndPos(pos);

			h += m_artefacts_count->GetWndSize().y;
			AttachChild(m_artefacts_count);
		}

		static float h2{};
		SetHeight(h + h2);

		xr_vector<std::pair<shared_str, u16>> af_sections_inside;
		xr_vector<CArtefact*> artefacts_inside = pAfContainer->GetArtefactsInside();

		for (int i = 0; i < m_iMaxAfCount; ++i)
		{
			m_textArtefacts[i]->Show(false);
			m_stArtefacts[i]->Show(false);
			AttachChild(m_stArtefacts[i]);
			AttachChild(m_textArtefacts[i]);
		}

		auto findIndexByValue = [&](const u16& targetValue, int iteration)
		{
			auto it = af_sections_inside.begin();

			std::advance(it, iteration);

			if (af_sections_inside[iteration].second != targetValue)
			{
				af_sections_inside.erase(it);

				af_sections_inside.pop_back();

				return false;
			}

			return true;
		};

		string128 str{}, str2{};

		if (!artefacts_inside.empty())
		{

			for (auto artefact_ptr : artefacts_inside)
				af_sections_inside.emplace_back(artefact_ptr->cNameSect(), artefact_ptr->ID());

			for (int i = 0; i < artefacts_inside.size(); ++i)
			{
				m_textArtefacts[i]->Show((GameConstants::GetArtefactsDegradation() || GameConstants::GetAfRanks()) && artefacts_inside[i]->ID() == af_sections_inside[i].second);
				m_stArtefacts[i]->Show(artefacts_inside[i]->ID() == af_sections_inside[i].second);

				if (GameConstants::GetArtefactsDegradation())
				{
					strconcat(sizeof(str), str, std::to_string(static_cast<int>(artefacts_inside[i]->GetCurrentChargeLevel() * 100)).c_str(), "%");

					if (GameConstants::GetAfRanks())
						strconcat(sizeof(str2), str2, str, " | ", std::to_string(artefacts_inside[i]->GetCurrentAfRank()).c_str());
					else
						strconcat(sizeof(str2), str2, str, "");
				}
				else if (GameConstants::GetAfRanks())
					strconcat(sizeof(str2), str2, CStringTable().translate("ui_inv_af_rank").c_str(), ": ", std::to_string(artefacts_inside[i]->GetCurrentAfRank()).c_str());

				m_textArtefacts[i]->SetTextST(str2);

				m_stArtefacts[i]->SetShader(InventoryUtilities::GetEquipmentIconsShader());

				Frect				tex_rect;
				tex_rect.x1 = float(pSettings->r_u32(af_sections_inside[i].first.c_str(), "inv_grid_x") * INV_GRID_WIDTH(GameConstants::GetUseHQ_Icons()));
				tex_rect.y1 = float(pSettings->r_u32(af_sections_inside[i].first.c_str(), "inv_grid_y") * INV_GRID_HEIGHT(GameConstants::GetUseHQ_Icons()));
				tex_rect.x2 = float(pSettings->r_u32(af_sections_inside[i].first.c_str(), "inv_grid_width") * INV_GRID_WIDTH(GameConstants::GetUseHQ_Icons()));
				tex_rect.y2 = float(pSettings->r_u32(af_sections_inside[i].first.c_str(), "inv_grid_height") * INV_GRID_HEIGHT(GameConstants::GetUseHQ_Icons()));
				tex_rect.rb.add(tex_rect.lt);
				m_stArtefacts[i]->SetOriginalRect(tex_rect);
				m_stArtefacts[i]->TextureOn();
				m_stArtefacts[i]->SetStretchTexture(true);

				if (GameConstants::GetUseHQ_Icons())
					m_stArtefacts[i]->SetWndSize(Fvector2().set((tex_rect.x2 - tex_rect.x1) * UI().get_current_kx() / 2 * m_stArtefactsScale, (tex_rect.y2 - tex_rect.y1) / 2 * m_stArtefactsScale));
				else
					m_stArtefacts[i]->SetWndSize(Fvector2().set((tex_rect.x2 - tex_rect.x1) * UI().get_current_kx() * m_stArtefactsScale, (tex_rect.y2 - tex_rect.y1) * m_stArtefactsScale));

				if (af_sections_inside.size() == 1)
				{
					tex_rect.set(0, 0, 1, 1);
				}
				else
				{
					if (i + 1 < af_sections_inside.size())
					{
						tex_rect.x1 = float(pSettings->r_u32(af_sections_inside[i + 1].first.c_str(), "inv_grid_x") * INV_GRID_WIDTH(GameConstants::GetUseHQ_Icons()));
						tex_rect.y1 = float(pSettings->r_u32(af_sections_inside[i + 1].first.c_str(), "inv_grid_y") * INV_GRID_HEIGHT(GameConstants::GetUseHQ_Icons()));
						tex_rect.x2 = float(pSettings->r_u32(af_sections_inside[i + 1].first.c_str(), "inv_grid_width") * INV_GRID_WIDTH(GameConstants::GetUseHQ_Icons()));
						tex_rect.y2 = float(pSettings->r_u32(af_sections_inside[i + 1].first.c_str(), "inv_grid_height") * INV_GRID_HEIGHT(GameConstants::GetUseHQ_Icons()));
						tex_rect.rb.add(tex_rect.lt);
					}
				}
			}

			if (i == 1)
			{
				SetHeight(h + (m_stArtefacts[i]->GetWndPos().y * 1.7f));
				h2 = m_stArtefacts[i]->GetWndPos().y * 1.4f;
			}
			else if (i % (m_iIncWndHeight - 1) == 0 && i < 5)
			{
				SetHeight(h + m_stArtefacts[i]->GetWndPos().y * 1.7f);
				h2 = m_stArtefacts[i]->GetWndPos().y * 1.7f;
			}
			else if ((i % m_iIncWndHeight) == 0 && i > 5)
			{
				SetHeight(h + m_stArtefacts[i]->GetWndPos().y * 1.7f);
				h2 = m_stArtefacts[i]->GetWndPos().y * 1.7f;
			}

			return;
		}
	}*/

	SetHeight(h);
}

/// ----------------------------------------------------------------

CUIInventoryItemInfo::CUIInventoryItemInfo()
{
	m_caption = NULL;
	m_value = NULL;
	m_magnitude = 1.0f;
	m_show_sign = false;

	m_unit_str._set("");
	m_texture._set("");
}

CUIInventoryItemInfo::~CUIInventoryItemInfo()
{
}

void CUIInventoryItemInfo::Init(CUIXml& xml, LPCSTR section)
{
	CUIXmlInit::InitWindow(xml, section, 0, this);
	xml.SetLocalRoot(xml.NavigateToNode(section));

	m_caption = UIHelper::CreateStatic(xml, "caption", this);
	m_value	= UIHelper::CreateStatic(xml, "value", this);
	m_magnitude = xml.ReadAttribFlt("value", 0, "magnitude", 1.0f);
	m_show_sign = (xml.ReadAttribInt("value", 0, "show_sign", 1) == 1);

	LPCSTR unit_str = xml.ReadAttrib("value", 0, "unit_str", "");
	m_unit_str._set(CStringTable().translate(unit_str));

	if (xml.NavigateToNode("caption:texture", 0))
	{
		use_color = xml.ReadAttribInt("caption:texture", 0, "use_color", 0);
		clr_invert = xml.ReadAttribInt("caption:texture", 0, "clr_invert", 0);
		clr_dynamic = xml.ReadAttribInt("caption:texture", 0, "clr_dynamic", 0);
		LPCSTR texture = xml.Read("caption:texture", 0, "");
		m_texture._set(texture);
	}

	Ivector4 red = GameConstants::GetRedColor();
	Ivector4 green = GameConstants::GetGreenColor();
	Ivector4 neutral = GameConstants::GetNeutralColor();

	if (xml.NavigateToNode("caption:min_color", 0))
		m_negative_color = CUIXmlInit::GetColor(xml, "caption:min_color", 0, color_rgba(red.x, red.y, red.z, red.w));
	else
		m_negative_color = color_rgba(red.x, red.y, red.z, red.w);

	if (xml.NavigateToNode("caption:middle_color", 0))
		m_neutral_color = CUIXmlInit::GetColor(xml, "caption:middle_color", 0, color_rgba(neutral.x, neutral.y, neutral.z, neutral.w));
	else
		m_neutral_color = color_rgba(neutral.x, neutral.y, neutral.z, neutral.w);

	if (xml.NavigateToNode("caption:max_color", 0))
		m_positive_color = CUIXmlInit::GetColor(xml, "caption:max_color", 0, color_rgba(green.x, green.y, green.z, green.w));
	else
		m_positive_color = color_rgba(green.x, green.y, green.z, green.w);
}

void CUIInventoryItemInfo::SetCaption(LPCSTR name)
{
	m_caption->SetText(name);
}

void CUIInventoryItemInfo::SetValue(float value, int vle, int accuracy)
{
	value *= m_magnitude;
	string32 buf;
	if (m_show_sign)
	{
		xr_sprintf(buf, "%+.0f", value);
		if (accuracy == 0)
			xr_sprintf(buf, "%+.0f", value);
		else if (accuracy == 1)
			xr_sprintf(buf, "%+.1f", value);
		else if (accuracy == 2)
			xr_sprintf(buf, "%+.2f", value);
		else if (accuracy == 3)
			xr_sprintf(buf, "%+.3f", value);
	}
	else
	{
		if (accuracy == 0)
			xr_sprintf(buf, "%.0f", value);
		else if (accuracy == 1)
			xr_sprintf(buf, "%.1f", value);
		else if (accuracy == 2)
			xr_sprintf(buf, "%.2f", value);
		else if (accuracy == 3)
			xr_sprintf(buf, "%.3f", value);
	}

	LPSTR str;
	if (m_unit_str.size())
		STRCONCAT(str, buf, " ", m_unit_str.c_str());
	else
		STRCONCAT(str, buf);

	m_value->SetText(str);

	bool is_positive = (value >= 0.0f);
	Fcolor current{}, negative{}, middle{}, positive{};

	value /= m_magnitude;
	clamp(value, 0.01f, 1.0f);

	if (GameConstants::GetColorizeValues())
	{
		if (vle == 0)
		{
			m_value->SetTextColor(m_neutral_color);
		}
		else if (vle == 1)
		{
			if (is_positive)
				current.lerp(negative.set(m_negative_color), middle.set(m_neutral_color), positive.set(m_positive_color), value);
			else
				current.lerp(negative.set(m_positive_color), middle.set(m_neutral_color), positive.set(m_negative_color), value);
		}
		else if (vle == 2)
		{
			if (is_positive)
				current.lerp(negative.set(m_positive_color), middle.set(m_neutral_color), positive.set(m_negative_color), 0.25f);
			else
				current.lerp(negative.set(m_negative_color), middle.set(m_neutral_color), positive.set(m_positive_color), 0.25f);
		}

		if (vle == 1 || vle == 2)
			m_value->SetTextColor(current.get());
	}
	else
		m_value->SetTextColor(color_rgba(170, 170, 170, 255));

	m_caption->InitTexture(m_texture.c_str());

	if (GameConstants::GetColorizeValues() || use_color)
	{
		if (clr_dynamic)
		{
			if (vle >= 2 || !clr_invert)
			{
				if (is_positive)
					current.lerp(negative.set(m_negative_color), positive.set(m_positive_color), value);
				else
					current.lerp(negative.set(m_positive_color), positive.set(m_negative_color), value);
			}
			else
			{
				if (is_positive)
					current.lerp(negative.set(m_positive_color), positive.set(m_negative_color), value);
				else
					current.lerp(negative.set(m_negative_color), positive.set(m_positive_color), value);
			}

			m_caption->SetTextureColor(current.get());
		}
		else
			m_caption->SetTextureColor(m_neutral_color);
	}
}
