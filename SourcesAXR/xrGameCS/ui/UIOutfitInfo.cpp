////////////////////////////////////////////////////////////////////////////
//	Module 		: UIOutfitInfo.cpp
//	Created 	: 12.11.2023
//  Modified 	: 12.11.2023
//	Author		: Dance Maniac (M.F.S. Team)
//	Description : Outfit Window Class
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIOutfitInfo.h"
#include "UIXmlInit.h"
#include "UIStatic.h"
#include "UIHelper.h"
#include "UIDoubleProgressBar.h"

#include "..\CustomOutfit.h"
#include "..\string_table.h"
#include "..\actor.h"
#include "..\ActorCondition.h"
#include "..\player_hud.h"
#include "..\AdvancedXrayGameConstants.h"

LPCSTR immunity_names[] =
{
	"burn_immunity",
	"shock_immunity",
	"strike_immunity",
	"wound_immunity",
	"radiation_immunity",
	"telepatic_immunity",
	"chemical_burn_immunity",
	"explosion_immunity",
	"fire_wound_immunity",
};

LPCSTR immunity_st_names[] =
{
	"ui_inv_outfit_burn_protection",
	"ui_inv_outfit_shock_protection",
	"ui_inv_outfit_strike_protection",
	"ui_inv_outfit_wound_protection",
	"ui_inv_outfit_radiation_protection",
	"ui_inv_outfit_telepatic_protection",
	"ui_inv_outfit_chemical_burn_protection",
	"ui_inv_outfit_explosion_protection",
	"ui_inv_outfit_fire_wound_protection",
};

CUIOutfitItem::CUIOutfitItem()
{
	for (u32 i = 0; i < max_count; ++i)
	{
		m_items[i] = nullptr;
	}

	m_outfit_filter_condition = nullptr;

	m_artefacts_count	= nullptr;
	m_additional_weight = nullptr;
	m_inv_capacity		= nullptr;
}

CUIOutfitItem::~CUIOutfitItem()
{
	for (u32 i = 0; i < max_count; ++i)
	{
		xr_delete(m_items[i]);
	}

	xr_delete(m_outfit_filter_condition);

	xr_delete(m_artefacts_count);
	xr_delete(m_additional_weight);
	xr_delete(m_inv_capacity);
}

void CUIOutfitItem::InitFromXml(CUIXml& xml)
{
	LPCSTR base = "outfit_info";
	XML_NODE* stored_root = xml.GetLocalRoot();
	XML_NODE* base_node = xml.NavigateToNode(base, 0);
	if (!base_node)
		return;

	CUIXmlInit::InitWindow(xml, base, 0, this);
	xml.SetLocalRoot(base_node);

	for (u32 i = 0; i < max_count; ++i)
	{
		m_items[i] = xr_new<CUIOutfitItemInfo>();
		m_items[i]->Init(xml, immunity_names[i], 0);
		m_items[i]->SetAutoDelete(false);
		LPCSTR name = CStringTable().translate(immunity_st_names[i]).c_str();
		m_items[i]->SetCaption(name);
		xml.SetLocalRoot(base_node);
	}

	m_outfit_filter_condition = xr_new<CUIOutfitItemInfo>();
	m_outfit_filter_condition->Init(xml, "antigas_filter", 1);
	m_outfit_filter_condition->SetAutoDelete(false);
	LPCSTR name = CStringTable().translate("ui_inv_filter_condition").c_str();
	m_outfit_filter_condition->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_artefacts_count = xr_new<CUIOutfitItemInfo>();
	m_artefacts_count->Init(xml, "artefact_count");
	m_artefacts_count->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_outfit_artefacts_count").c_str();
	m_artefacts_count->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_additional_weight = xr_new<CUIOutfitItemInfo>();
	m_additional_weight->Init(xml, "additional_weight");
	m_additional_weight->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_weight").c_str();
	m_additional_weight->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_inv_capacity = xr_new<CUIOutfitItemInfo>();
	m_inv_capacity->Init(xml, "inventory_capacity");
	m_inv_capacity->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_outfit_inventory_capacity").c_str();
	m_inv_capacity->SetCaption(name);
	xml.SetLocalRoot(base_node);

	xml.SetLocalRoot(stored_root);
}

/// ----------------------------------------------------------------

CUIOutfitItemInfo::CUIOutfitItemInfo()
{
	m_caption = nullptr;
	m_value = nullptr;
	m_magnitude = 1.0f;
	m_show_sign = false;

	m_unit_str._set("");
	m_texture._set("");
}

CUIOutfitItemInfo::~CUIOutfitItemInfo()
{
}

void CUIOutfitItemInfo::Init(CUIXml& xml, LPCSTR section)
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

void CUIOutfitItemInfo::Init(CUIXml& xml, LPCSTR section, int mode)
{
	CUIXmlInit::InitWindow(xml, section, 0, this);
	xml.SetLocalRoot(xml.NavigateToNode(section));

	m_progress.InitFromXml(xml, "progress_bar");
	AttachChild(&m_progress);

	m_caption = UIHelper::CreateStatic(xml, "caption", this);
	m_value = UIHelper::CreateStatic(xml, "static_value", this);
	m_magnitude = xml.ReadAttribFlt("static_value", 0, "magnitude", 1.0f);
	m_show_sign = (xml.ReadAttribInt("static_value", 0, "show_sign", 1) == 1);

	LPCSTR unit_str = xml.ReadAttrib("static_value", 0, "unit_str", "");
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

void CUIOutfitItemInfo::SetCaption(LPCSTR name)
{
	m_caption->SetText(name);
}

void CUIOutfitItemInfo::SetValue(float value, float comp, int vle, int accuracy)
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
			if (value == comp)
			{
				m_value->SetTextColor(m_neutral_color);
			}
			else
			{
				if (is_positive)
					current.lerp(negative.set(m_negative_color), middle.set(m_neutral_color), positive.set(m_positive_color), value);
				else
					current.lerp(negative.set(m_positive_color), middle.set(m_neutral_color), positive.set(m_negative_color), value);
			}
		}
		else if (vle == 2)
		{
			if (value == comp)
			{
				m_value->SetTextColor(m_neutral_color);
			}
			else
			{
				if (is_positive)
					current.lerp(negative.set(m_positive_color), middle.set(m_neutral_color), positive.set(m_negative_color), 0.25f);
				else
					current.lerp(negative.set(m_negative_color), middle.set(m_neutral_color), positive.set(m_positive_color), 0.25f);
			}
		}

		if ((vle == 1 || vle == 2) && value != comp)
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

// -------------------------------------------------------------------------------------------------

void CUIOutfitItemInfo::SetProgressValue( float cur, float comp )
{
	cur  *= m_magnitude;
	comp *= m_magnitude;
	m_progress.SetTwoPos( cur, comp );
	string32 buf;

	Fvector4 red = GameConstants::GetRedColor();
	Fvector4 green = GameConstants::GetGreenColor();
	Fvector4 neutral = GameConstants::GetNeutralColor();
	u32 negative_color = color_rgba(red.x, red.y, red.z, red.w);
	u32 positive_color = color_rgba(green.x, green.y, green.z, green.w);
	u32 neutral_color = color_rgba(neutral.x, neutral.y, neutral.z, neutral.w);
//	xr_sprintf( buf, sizeof(buf), "%d %%", (int)cur );
	xr_sprintf( buf, sizeof(buf), "%.0f", cur );

	if (cur == comp)
		m_value->SetTextColor(neutral_color);
	else
		cur > comp ? m_value->SetTextColor(positive_color) : m_value->SetTextColor(negative_color);

	m_value->SetText( buf );
}

void CUIOutfitItem::SetInfo(CCustomOutfit* cur_outfit, CCustomOutfit* slot_outfit)
{
	DetachAll();
	CActor* actor = smart_cast<CActor*>(Level().CurrentViewEntity());
	shared_str section = cur_outfit->cNameSect();
	if (!actor)
	{
		return;
	}

	float h = 0.0f, cur = 0.0f, slot = 0.0f, max_power = 1.0f;
	Fvector2 pos;
	pos.set(GetWndPos());

	for (u32 i = 0; i < max_count; ++i)
	{
		if (i == ALife::eHitTypeFireWound)
		{
			continue;
		}

		ALife::EHitType hit_type = (ALife::EHitType)i;
		float max_power = actor->conditions().GetZoneMaxPower(hit_type);

		float cur = cur_outfit->GetDefHitTypeProtection(hit_type);
		cur /= max_power; // = 0..1
		float slot = cur;

		if (slot_outfit)
		{
			slot = slot_outfit->GetDefHitTypeProtection(hit_type);
			slot /= max_power; //  = 0..1
		}
		m_items[i]->SetProgressValue(cur, slot);
		pos.set(m_items[i]->GetWndPos());
		pos.y = h;
		m_items[i]->SetWndPos(pos);
		h += m_items[i]->GetWndSize().y;
		AttachChild(m_items[i]);
	}

	if (m_items[ALife::eHitTypeFireWound])
	{
		IKinematics* ikv = smart_cast<IKinematics*>(actor->Visual());
		VERIFY(ikv);
		u16 spine_bone = ikv->LL_BoneID("bip01_spine");

		float cur = cur_outfit->GetBoneArmor(spine_bone) * cur_outfit->GetCondition();
		slot = slot_outfit ? slot_outfit->GetBoneArmor(spine_bone) * slot_outfit->GetCondition() : cur;
		float max_power = actor->conditions().GetMaxFireWoundProtection() != 0 ? actor->conditions().GetMaxFireWoundProtection() : 1.f;
		cur /= max_power;
		slot /= max_power;
		m_items[ALife::eHitTypeFireWound]->SetProgressValue(cur, slot);
		pos.set(m_items[ALife::eHitTypeFireWound]->GetWndPos());
		pos.y = h;
		m_items[ALife::eHitTypeFireWound]->SetWndPos(pos);
		h += m_items[ALife::eHitTypeFireWound]->GetWndSize().y;
		AttachChild(m_items[ALife::eHitTypeFireWound]);
	}

	float cur_filter = cur_outfit->GetFilterCondition() > 0.f ? (cur_outfit->GetFilterCondition() * 100.0f + 1.0f - EPS) : 0.f;

	if (GameConstants::GetOutfitUseFilters() && cur_outfit->m_bUseFilter)
	{
		m_outfit_filter_condition->SetProgressValue(cur_filter, cur_filter);

		pos.set(m_outfit_filter_condition->GetWndPos());
		pos.y = h;
		m_outfit_filter_condition->SetWndPos(pos);

		h += m_outfit_filter_condition->GetWndSize().y;
		AttachChild(m_outfit_filter_condition);
	}

	
	cur = cur_outfit->m_additional_weight2;
	slot = slot_outfit ? slot_outfit->m_additional_weight2 : cur;
	if (!fis_zero(cur))
	{
		m_additional_weight->SetValue(cur, slot, 2, 2);

		pos.set(m_additional_weight->GetWndPos());
		pos.y = h;
		m_additional_weight->SetWndPos(pos);

		h += m_additional_weight->GetWndSize().y;
		AttachChild(m_additional_weight);
	}

	if (GameConstants::GetLimitedInventory())
	{
		cur = cur_outfit->GetInventoryCapacity();
		slot = slot_outfit ? slot_outfit->GetInventoryCapacity() : cur;
		if (!fis_zero(cur))
		{
			m_inv_capacity->SetValue(cur, slot, 2, 2);

			pos.set(m_inv_capacity->GetWndPos());
			pos.y = h;
			m_inv_capacity->SetWndPos(pos);

			h += m_inv_capacity->GetWndSize().y;
			AttachChild(m_inv_capacity);
		}
	}

	cur = cur_outfit->get_artefact_count();
	slot = slot_outfit ? slot_outfit->get_artefact_count() : cur;
	if (!fis_zero(cur))
	{
		m_artefacts_count->SetValue(cur, slot, 0);
		pos.set(m_artefacts_count->GetWndPos());
		pos.y = h;
		m_artefacts_count->SetWndPos(pos);

		h += m_artefacts_count->GetWndSize().y;
		AttachChild(m_artefacts_count);
	}

	SetHeight(h);
}