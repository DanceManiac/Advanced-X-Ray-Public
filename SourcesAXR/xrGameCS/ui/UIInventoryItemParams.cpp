////////////////////////////////////////////////////////////////////////////
//	Module 		: UIInventoryItemParams.cpp
//	Created 	: 08.04.2021
//  Modified 	: 20.04.2021
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
#include "UIActorMenu.h"

#include "../Torch.h"
#include "../CustomDetector.h"
#include "../AnomalyDetector.h"
#include "../AdvancedXrayGameConstants.h"

CUIInventoryItem::CUIInventoryItem()
{
	m_af_radius = NULL;
	m_af_vis_radius = NULL;
	m_charge_level = NULL;
	m_max_charge = NULL;
	m_uncharge_speed = NULL;
}

CUIInventoryItem::~CUIInventoryItem()
{
	xr_delete(m_af_radius);
	xr_delete(m_af_vis_radius);
	xr_delete(m_charge_level);
	xr_delete(m_max_charge);
	xr_delete(m_uncharge_speed);
	xr_delete(m_Prop_line);
}

LPCSTR item_influence_caption[] =
{
	"ui_inv_af_radius",
	"ui_inv_af_vis_radius",
	"ui_inv_charge_level",
	"ui_inv_max_charge",
	"ui_inv_uncharge_speed"
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

	m_af_radius = xr_new<CUIInventoryItemInfo>();
	m_af_radius->Init(xml, "af_radius");
	m_af_radius->SetAutoDelete(false);
	LPCSTR name = CStringTable().translate("ui_inv_af_radius").c_str();
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
}

void CUIInventoryItem::SetInfo(shared_str const& section)
{
	DetachAll();
	AttachChild(m_Prop_line);

	CActor* actor = smart_cast<CActor*>(Level().CurrentViewEntity());
	if (!actor)
	{
		return;
	}

	float val = 0.0f, max_val = 1.0f;
	Fvector2 pos;
	float h = m_Prop_line->GetWndPos().y + m_Prop_line->GetWndSize().y;

	bool ShowCharge = GameConstants::GetTorchHasBattery() || GameConstants::GetArtDetectorUseBattery() || GameConstants::GetAnoDetectorUseBattery();

	if (pSettings->line_exist(section.c_str(), "af_radius"))
	{
		val = pSettings->r_float(section, "af_radius");
		if (!fis_zero(val))
		{
			m_af_radius->SetValue(val);
			pos.set(m_af_radius->GetWndPos());
			pos.y = h;
			m_af_radius->SetWndPos(pos);

			h += m_af_radius->GetWndSize().y;
			AttachChild(m_af_radius);
		}
	}

	if (pSettings->line_exist(section.c_str(), "af_vis_radius"))
	{
		val = pSettings->r_float(section, "af_vis_radius");
		if (!fis_zero(val))
		{
			m_af_vis_radius->SetValue(val);
			pos.set(m_af_vis_radius->GetWndPos());
			pos.y = h;
			m_af_vis_radius->SetWndPos(pos);

			h += m_af_vis_radius->GetWndSize().y;
			AttachChild(m_af_vis_radius);
		}
	}

	if (ShowCharge && pSettings->line_exist(section.c_str(), "max_charge_level"))
	{
		val = pSettings->r_float(section, "max_charge_level");
		if (!fis_zero(val))
		{
			m_max_charge->SetValue(val);
			pos.set(m_max_charge->GetWndPos());
			pos.y = h;
			m_max_charge->SetWndPos(pos);

			h += m_max_charge->GetWndSize().y;
			AttachChild(m_max_charge);
		}
	}

	if (ShowCharge && pSettings->line_exist(section.c_str(), "uncharge_speed"))
	{
		val = pSettings->r_float(section, "uncharge_speed");
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

	Fvector4 red = GameConstants::GetRedColor();
	Fvector4 green = GameConstants::GetGreenColor();
	Fvector4 neutral = GameConstants::GetNeutralColor();

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

void CUIInventoryItemInfo::SetValue(float value, int vle)
{
	value *= m_magnitude;
	string32 buf;
	if (m_show_sign)
		xr_sprintf(buf, "%+.0f", value);
	else
		xr_sprintf(buf, "%.0f", value);

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
				current.lerp(negative.set(m_positive_color), middle.set(m_neutral_color), positive.set(m_negative_color), value);
			else
				current.lerp(negative.set(m_negative_color), middle.set(m_neutral_color), positive.set(m_positive_color), value);
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
			if (vle > 2 || !clr_invert)
			{
				if (is_positive)
					current.lerp(negative.set(m_negative_color), middle.set(m_neutral_color), positive.set(m_positive_color), value);
				else
					current.lerp(negative.set(m_positive_color), middle.set(m_neutral_color), positive.set(m_negative_color), value);
			}
			else
			{
				if (is_positive)
					current.lerp(negative.set(m_positive_color), middle.set(m_neutral_color), positive.set(m_negative_color), value);
				else
					current.lerp(negative.set(m_negative_color), middle.set(m_neutral_color), positive.set(m_positive_color), value);
			}

			m_caption->SetTextureColor(current.get());
		}
		else
			m_caption->SetTextureColor(m_neutral_color);
	}
}

// -------------------------------------------------------------------------------------------------

CUIItemConditionParams::CUIItemConditionParams()
{
	AttachChild(&m_ProgressCurCharge);
	AttachChild(&m_icon_charge);
	AttachChild(&m_textCharge);
}

CUIItemConditionParams::~CUIItemConditionParams()
{
}

void CUIItemConditionParams::InitFromXml(CUIXml& xml_doc)
{
	if (!xml_doc.NavigateToNode("inventory_items_info", 0))	return;
	CUIXmlInit::InitStatic(xml_doc, "cap_current_charge_level", 0, &m_textCharge);
	m_ProgressCurCharge.InitFromXml(xml_doc, "progress_current_charge_level");
}

void CUIItemConditionParams::SetInfo(CInventoryItem const* slot_item, CInventoryItem const& cur_item)
{
	float cur_value = cur_item.GetConditionToShow() * 100.0f + 1.0f - EPS;
	float slot_value = cur_value;

	if (slot_item && (slot_item != &cur_item))
	{
		slot_value = slot_item->GetConditionToShow() * 100.0f + 1.0f - EPS;
	}
	m_ProgressCurCharge.SetTwoPos(cur_value, slot_value);
}