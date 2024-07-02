#include "stdafx.h"
#include "UIBoosterInfo.h"
#include "UIStatic.h"
#include "object_broker.h"
#include "../EntityCondition.h"
#include "..\actor.h"
#include "../ActorCondition.h"
#include "UIXmlInit.h"
#include "UIHelper.h"
#include "../string_table.h"
#include "../Inventory_Item.h"
#include "../eatable_item.h"
//#include "../AntigasFilter.h"
//#include "../RepairKit.h"
#include "../AdvancedXrayGameConstants.h"

CUIBoosterInfo::CUIBoosterInfo()
{
	for (u32 i = 0; i < eBoostExplImmunity; ++i)
	{
		m_booster_items[i] = nullptr;
	}

	for (u32 i = 0; i < eQuickItemLast; ++i)
	{
		m_quick_items[i] = nullptr;
	}

	m_portions = nullptr;
}

CUIBoosterInfo::~CUIBoosterInfo()
{
	delete_data(m_booster_items);
	delete_data(m_quick_items);
	xr_delete(m_portions);
}

LPCSTR ef_quick_eat_values_names[] =
{
	"eat_health",
	"eat_power",
	"wounds_heal_perc",
	"eat_satiety",
	"eat_thirst",
	"eat_psy_health",

	"charge_level",
	"filter_condition",
	"restore_condition",

	"eat_intoxication",
	"eat_radiation",
	"eat_sleepeness",

	//HoP
	"eat_alcohol",
	"eat_alcoholism",
	"eat_hangover",
	"eat_drugs",
	"eat_narcotism",
	"eat_withdrawal",

	"eat_frostbite"
};

LPCSTR quick_eat_influence_caption[] =
{
	"ui_inv_health",
	"ui_inv_power",
	"ui_inv_bleeding",
	"ui_inv_satiety",
	"ui_inv_thirst",
	"ui_inv_psy_health",

	//M.F.S Team additions
	"ui_inv_battery",
	"ui_inv_filter_condition",
	"ui_inv_repair_kit_condition",

	"ui_inv_intoxication",
	"ui_inv_radiation",
	"ui_inv_sleepeness",

	//HoP
	"ui_inv_alcohol",
	"ui_inv_alcoholism",
	"ui_inv_hangover",
	"ui_inv_drugs",
	"ui_inv_narcotism",
	"ui_inv_withdrawal",

	"ui_inv_frostbite"
};

LPCSTR ef_quick_eat_nodes_names[] =
{
	"quick_eat_health",
	"quick_eat_power",
	"quick_eat_bleeding",
	"quick_eat_satiety",
	"quick_eat_thirst",
	"quick_eat_psy_health",

	//M.F.S Team additions
	"quick_eat_battery",
	"quick_eat_filter_condition",
	"quick_eat_repair_kit_condition",

	"quick_eat_intoxication",
	"quick_eat_radiation",
	"quick_eat_sleepeness",

	//HoP
	"quick_eat_alcohol",
	"quick_eat_alcoholism",
	"quick_eat_hangover",
	"quick_eat_drugs",
	"quick_eat_narcotism",
	"quick_eat_withdrawal",

	"quick_eat_frostbite"
};

LPCSTR ef_boosters_values_names[] =
{
	"boost_health_restore",
	"boost_power_restore",
	"boost_radiation_restore",
	"boost_bleeding_restore",
	"boost_satiety_restore",
	"boost_thirst_restore",
	"boost_psy_health_restore",
	"boost_intoxication_restore",
	"boost_sleepeness_restore",
	"boost_alcohol_restore",
	"boost_alcoholism_restore",
	"boost_hangover_restore",
	"boost_drugs_restore",
	"boost_narcotism_restore",
	"boost_withdrawal_restore",
	"boost_frostbite_restore",
	"boost_max_weight",
	"boost_radiation_protection",
	"boost_telepat_protection",
	"boost_chemburn_protection",
	"boost_burn_immunity",
	"boost_shock_immunity",
	"boost_radiation_immunity",
	"boost_telepat_immunity",
	"boost_chemburn_immunity",
	"boost_explosion_immunity",
	"boost_strike_immunity",
	"boost_fire_wound_immunity",
	"boost_wound_immunity"
};

LPCSTR boost_influence_caption[] =
{
	"ui_inv_health",
	"ui_inv_power",
	"ui_inv_radiation",
	"ui_inv_bleeding",
	"ui_inv_satiety",
	"ui_inv_thirst",
	"ui_inv_psy_health",
	"ui_inv_intoxication",
	"ui_inv_sleepeness",
	"ui_inv_alcohol",
	"ui_inv_alcoholism",
	"ui_inv_hangover",
	"ui_inv_drugs",
	"ui_inv_narcotism",
	"ui_inv_withdrawal",
	"ui_inv_frostbite",
	"ui_inv_outfit_additional_weight",
	"ui_inv_outfit_radiation_protection",
	"ui_inv_outfit_telepatic_protection",
	"ui_inv_outfit_chemical_burn_protection",
	"ui_inv_outfit_burn_immunity",
	"ui_inv_outfit_shock_immunity",
	"ui_inv_outfit_radiation_immunity",
	"ui_inv_outfit_telepatic_immunity",
	"ui_inv_outfit_chemical_burn_immunity",
	"ui_inv_explosion_immunity",
	"ui_inv_strike_immunity",
	"ui_inv_fire_wound_immunity",
	"ui_inv_wound_immunity"
};

void CUIBoosterInfo::InitFromXml(CUIXml& xml)
{
	LPCSTR base = "booster_params";
	XML_NODE* stored_root = xml.GetLocalRoot();
	XML_NODE* base_node = xml.NavigateToNode(base, 0);
	if (!base_node)
		return;

	CUIXmlInit::InitWindow(xml, base, 0, this);
	xml.SetLocalRoot(base_node);

	for (u32 i = 0; i < eBoostExplImmunity; ++i)
	{
		m_booster_items[i] = xr_new<UIBoosterInfoItem>();
		m_booster_items[i]->Init(xml, ef_boosters_section_names[i]);
		m_booster_items[i]->SetAutoDelete(false);

		LPCSTR name = CStringTable().translate(boost_influence_caption[i]).c_str();
		m_booster_items[i]->SetCaption(name);

		xml.SetLocalRoot(base_node);
	}

	for (u32 i = 0; i < eQuickItemLast; ++i)
	{
		m_quick_items[i] = xr_new<UIBoosterInfoItem>();
		m_quick_items[i]->Init(xml, ef_quick_eat_nodes_names[i]);
		m_quick_items[i]->SetAutoDelete(false);
		LPCSTR name = CStringTable().translate(quick_eat_influence_caption[i]).c_str();
		m_quick_items[i]->SetCaption(name);
		xml.SetLocalRoot(base_node);
	}

	m_booster_time = xr_new<UIBoosterInfoItem>();
	m_booster_time->Init(xml, "boost_time");
	m_booster_time->SetAutoDelete(false);
	LPCSTR name = CStringTable().translate("ui_inv_effect_time").c_str();
	m_booster_time->SetCaption(name);
	xml.SetLocalRoot(base_node);

	//Portions
	m_portions = xr_new<UIBoosterInfoItem>();
	m_portions->Init(xml, "item_portions");
	m_portions->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_portions").c_str();
	m_portions->SetCaption(name);
	xml.SetLocalRoot(stored_root);

	xml.SetLocalRoot(stored_root);
}

void CUIBoosterInfo::SetInfo(CInventoryItem& pInvItem)
{
	DetachAll();
	CActor* actor = smart_cast<CActor*>(Level().CurrentViewEntity());
	if (!actor)
	{
		return;
	}

	const shared_str& section = pInvItem.object().cNameSect();
	CEatableItem* eatable = pInvItem.cast_eatable_item();
	//CAntigasFilter* pFilter = pInvItem.cast_filter();
	//CRepairKit* pRepairKit = pInvItem.cast_repair_kit();
	CEntityCondition::BOOSTER_MAP boosters = actor->conditions().GetCurBoosterInfluences();

	float val = 0.0f, max_val = 1.0f, max_value = 0.0f;
	Fvector2 pos;
	float h = GetWndPos().y + GetWndSize().y;

	for (u32 i = 0; i < eBoostExplImmunity; ++i)
	{
		if (pSettings->line_exist(section.c_str(), ef_boosters_values_names[i]))
		{
			val = pSettings->r_float(section, ef_boosters_values_names[i]);
			if (fis_zero(val))
				continue;

			EBoostParams type = (EBoostParams)i;
			switch (type)
			{
			case eBoostHpRestore:
			case eBoostPowerRestore:
			case eBoostBleedingRestore:
			case eBoostPsyHealthRestore:
			case eBoostIntoxicationRestore:
			case eBoostSleepenessRestore:
			case eBoostAlcoholRestore:
			case eBoostAlcoholismRestore:
			case eBoostHangoverRestore:
			case eBoostDrugsRestore:
			case eBoostNarcotismRestore:
			case eBoostWithdrawalRestore:
			case eBoostFrostbiteRestore:
			case eBoostMaxWeight:
				max_val = 1.0f;
				break;
			case eBoostSatietyRestore:
			case eBoostThirstRestore:
			case eBoostRadiationRestore:
				max_val = -1.0f;
				break;
			/*case eBoostBurnImmunity:
				max_val = actor->conditions().GetZoneMaxPower(ALife::infl_fire);
				break;
			case eBoostShockImmunity:
				max_val = actor->conditions().GetZoneMaxPower(ALife::infl_electra);
				break;
			case eBoostRadiationImmunity:
			case eBoostRadiationProtection:
				max_val = actor->conditions().GetZoneMaxPower(ALife::infl_rad);
				break;
			case eBoostTelepaticImmunity:
			case eBoostTelepaticProtection:
				max_val = actor->conditions().GetZoneMaxPower(ALife::infl_psi);
				break;
			case eBoostChemicalBurnImmunity:
			case eBoostChemicalBurnProtection:
				max_val = actor->conditions().GetZoneMaxPower(ALife::infl_acid);
				break; */
			}
			val /= max_val;
			int vle = 2;
			//vle: 0 - color from node; 1 - negative value is green, positive value is red(radiaton for example); 2 - negative value is red, positive value is green(satiety, health for example)
			if (i == _item_boost_radiation_restore || (i >= _item_boost_intoxication_restore && i < _item_boost_max_weight))
				vle = 1;

			m_booster_items[i]->SetValue(val, vle);
			pos.set(m_booster_items[i]->GetWndPos());
			pos.y = h;
			m_booster_items[i]->SetWndPos(pos);
			h += m_booster_items[i]->GetWndSize().y;
			AttachChild(m_booster_items[i]);
		}
	}

	for (u32 i = 0; i < eQuickItemLast; ++i)
	{
		if (pSettings->line_exist(section.c_str(), ef_quick_eat_values_names[i]))
		{
			val = pSettings->r_float(section, ef_quick_eat_values_names[i]);

			//if (eatable && i == _item_quick_radiation)
			//	val += eatable->m_fRadioactivity;

			//if (eatable && i == _item_quick_intoxication)
			//	val += eatable->m_fSpoliage;

			//vle: 0 - color from node; 1 - negative value is green, positive value is red(radiaton for example); 2 - negative value is red, positive value is green(satiety, health for example)
			if (fis_zero(val))
				continue;

			int vle = 2;
			if (i >= _item_quick_intoxication)
				vle = 1;

			m_quick_items[i]->SetValue(val, vle);
			pos.set(m_quick_items[i]->GetWndPos());
			pos.y = h;
			m_quick_items[i]->SetWndPos(pos);
			h += m_quick_items[i]->GetWndSize().y;
			AttachChild(m_quick_items[i]);
		}
	}

	//Portions
	if (eatable)
	{
		val = eatable->GetPortionsNum();
		max_value = eatable->GetConstPortionsNum();

		if (max_value > 1)
		{
			m_portions->SetValue(val, 0, max_value);
			pos.set(m_portions->GetWndPos());
			pos.y = h;
			m_portions->SetWndPos(pos);

			h += m_portions->GetWndSize().y;
			AttachChild(m_portions);
		}
	}

	if (pSettings->line_exist(section.c_str(), "boost_time"))
	{
		val = pSettings->r_float(section, "boost_time");
		if (!fis_zero(val))
		{
			m_booster_time->SetValue(val);
			pos.set(m_booster_time->GetWndPos());
			pos.y = h;
			m_booster_time->SetWndPos(pos);

			h += m_booster_time->GetWndSize().y;
			AttachChild(m_booster_time);
		}
	}

	SetHeight(h);
}

/// ----------------------------------------------------------------

UIBoosterInfoItem::UIBoosterInfoItem()
{
	m_caption = NULL;
	m_value = NULL;
	m_magnitude = 1.0f;
	m_show_sign = false;

	m_unit_str._set("");
	m_unit_str_max._set("");
	m_texture._set("");
}

UIBoosterInfoItem::~UIBoosterInfoItem()
{
}

void UIBoosterInfoItem::Init(CUIXml& xml, LPCSTR section)
{
	CUIXmlInit::InitWindow( xml, section, 0, this );
	xml.SetLocalRoot( xml.NavigateToNode( section ) );

	m_caption   = UIHelper::CreateStatic( xml, "caption", this );
	m_value     = UIHelper::CreateStatic( xml, "value",   this );
	m_magnitude = xml.ReadAttribFlt( "value", 0, "magnitude", 1.0f );
	m_show_sign = (xml.ReadAttribInt("value", 0, "show_sign", 1) == 1);
	
	LPCSTR unit_str = xml.ReadAttrib( "value", 0, "unit_str", "" );
	m_unit_str._set( CStringTable().translate( unit_str ) );

	LPCSTR unit_str_max = xml.ReadAttrib("value", 0, "unit_str_max", "");
	m_unit_str_max._set(CStringTable().translate(unit_str_max));
	
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

void UIBoosterInfoItem::SetCaption(LPCSTR name)
{
	m_caption->SetText(name);
}

void UIBoosterInfoItem::SetValue(float value, int vle, float max_val)
{
	value *= m_magnitude;
	max_val *= m_magnitude;
	string32 buf, buf2;

	m_show_sign ? xr_sprintf(buf, "%+.0f", value) : xr_sprintf(buf, "%.0f", value);

	xr_sprintf(buf2, "%.0f", max_val);

	LPSTR str, str2, comp_str;
	if (m_unit_str.size())
		STRCONCAT(str, buf, " ", m_unit_str.c_str());
	else
		STRCONCAT(str, buf);

	if (m_unit_str_max.size())
		STRCONCAT(str2, buf2, " ", m_unit_str_max.c_str());
	else
		STRCONCAT(str2, buf2);

	STRCONCAT(comp_str, str, "/", str2);

	fis_zero(max_val) ? m_value->SetText(str) : m_value->SetText(comp_str);

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