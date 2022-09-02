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
#include "../AntigasFilter.h"
#include "../AdvancedXrayGameConstants.h"

CUIBoosterInfo::CUIBoosterInfo()
{
	for (u32 i = 0; i < eBoostExplImmunity; ++i)
	{
		m_booster_items[i] = NULL;
	}
	m_portions = NULL;
	m_filter = NULL;
}

CUIBoosterInfo::~CUIBoosterInfo()
{
	delete_data(m_booster_items);
	xr_delete(m_portions);
	xr_delete(m_filter);
	xr_delete(m_Prop_line);
}

LPCSTR ef_boosters_nodes_names[] =
{
	"boost_satiety",
	"boost_health_restore",
	"boost_radiation_restore",
	"boost_power_restore",
	"boost_bleeding_restore",

	//M.F.S Team additions
	"boost_battery",
	"boost_thirst",
	"boost_intoxication",
	"boost_sleepeness",

	//HoP
	"boost_alcoholism",
	"boost_hangover",
	"boost_narcotism",
	"boost_withdrawal",
};

LPCSTR ef_boosters_section_names[] =
{
	"eat_satiety",
	"eat_health",
	"eat_radiation",
	"eat_power",
	"wounds_heal_perc",

	//M.F.S team additions
	"charge_level",
	"eat_thirst",
	"eat_intoxication",
	"eat_sleepeness",

	//HoP
	"eat_alcoholism",
	"eat_hangover",
	"eat_narcotism",
	"eat_withdrawal",
};

LPCSTR boost_influence_caption[] =
{
	"ui_inv_satiety",
	"ui_inv_health",
	"ui_inv_radiation",
	"ui_inv_power",
	"ui_inv_bleeding",

	//M.F.S Team additions
	"ui_inv_battery",
	"ui_inv_thirst",
	"ui_inv_intoxication",
	"ui_inv_sleepeness",

	//HoP
	"ui_inv_alcoholism",
	"ui_inv_hangover",
	"ui_inv_narcotism",
	"ui_inv_withdrawal",
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
		m_booster_items[i]->Init(xml, ef_boosters_nodes_names [i]);
		m_booster_items[i]->SetAutoDelete(false);

		LPCSTR name = CStringTable().translate(boost_influence_caption[i]).c_str();
		m_booster_items[i]->SetCaption(name);

		xml.SetLocalRoot(base_node);
	}

	m_Prop_line = xr_new<CUIStatic>();
	AttachChild(m_Prop_line);
	m_Prop_line->SetAutoDelete(false);
	CUIXmlInit::InitStatic(xml, "prop_line", 0, m_Prop_line);

	//Filter
	m_filter = xr_new<UIBoosterInfoItem>();
	m_filter->Init(xml, "boost_filter_condition");
	m_filter->SetAutoDelete(false);
	LPCSTR name = CStringTable().translate("ui_inv_filter_condition").c_str();
	m_filter->SetCaption(name);
	xml.SetLocalRoot(base_node);

	//Portions
	m_portions = xr_new<UIBoosterInfoItem>();
	m_portions->Init(xml, "item_portions");
	m_portions->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_portions").c_str();
	m_portions->SetCaption(name);
	xml.SetLocalRoot(base_node);
}

void CUIBoosterInfo::SetInfo(CInventoryItem& pInvItem)
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

	const shared_str& section = pInvItem.object().cNameSect();
	CEatableItem* eatable = pInvItem.cast_eatable_item();
	CAntigasFilter* pFilter = pInvItem.cast_filter();

	for (u32 i = 0; i < eBoostExplImmunity; ++i)
	{
		if (pSettings->line_exist(section.c_str(), ef_boosters_section_names[i]))
		{
			val = pSettings->r_float(section, ef_boosters_section_names[i]);
			int vle = 2;
			//vle: 0 - color from node; 1 - negative value is green, positive value is red(radiaton for example); 2 - negative value is red, positive value is green(satiety, health for example)
			if (fis_zero(val))
				continue;

			if (i == _item_radiation_restore_speed || i == _item_bleeding_restore_speed || i >= _item_intoxication)
				vle = 1;
			m_booster_items[i]->SetValue(val, vle);

			pos.set(m_booster_items[i]->GetWndPos());
			pos.y = h;
			m_booster_items[i]->SetWndPos(pos);

			h += m_booster_items[i]->GetWndSize().y;
			AttachChild(m_booster_items[i]);
		}
	}

	//Filter
	if (pFilter)
	{
		val = pFilter->m_fCondition;

		m_filter->SetValue(val);
		pos.set(m_filter->GetWndPos());
		pos.y = h;
		m_filter->SetWndPos(pos);

		h += m_filter->GetWndSize().y;
		AttachChild(m_filter);
	}

	//Portions
	if (eatable)
	{
		val = eatable->m_iPortionsNum;

		if (val > 1)
		{
			m_portions->SetValue(val);
			pos.set(m_portions->GetWndPos());
			pos.y = h;
			m_portions->SetWndPos(pos);

			h += m_portions->GetWndSize().y;
			AttachChild(m_portions);
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
	m_texture_minus._set("");
	m_texture_plus._set("");
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
	
	LPCSTR texture_minus = xml.Read( "texture_minus", 0, "" );
	if ( texture_minus && xr_strlen(texture_minus) )
	{
		m_texture_minus._set( texture_minus );
		
		LPCSTR texture_plus = xml.Read( "caption:texture", 0, "" );
		m_texture_plus._set( texture_plus );
		VERIFY( m_texture_plus.size() );
	}
}

void UIBoosterInfoItem::SetCaption(LPCSTR name)
{
	m_caption->SetText(name);
}

void UIBoosterInfoItem::SetValue(float value, int vle)
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
	bool positive = (value >= 0.0f);
	Fvector4 red = GameConstants::GetRedColor();
	Fvector4 green = GameConstants::GetGreenColor();
	Fvector4 neutral = GameConstants::GetNeutralColor();
	u32 red_color = color_rgba(red.x, red.y, red.z, red.w);
	u32 green_color = color_rgba(green.x, green.y, green.z, green.w);
	u32 neutral_color = color_rgba(neutral.x, neutral.y, neutral.z, neutral.w);
	u32 color = (positive) ? green_color : red_color;

	if (GameConstants::GetColorizeValues())
	{
		if (vle == 0)
		{
			m_value->SetTextColor(neutral_color);
		}
		else if (vle == 1)
		{
			positive?m_value->SetTextColor(red_color):m_value->SetTextColor(green_color);
		}
		else if (vle == 2)
		{
			positive?m_value->SetTextColor(green_color):m_value->SetTextColor(red_color);
		}
	}
	else
		m_value->SetTextColor(color_rgba(170, 170, 170, 255));

	if (m_texture_minus.size())
	{
		if (positive)
			m_caption->InitTexture(m_texture_plus.c_str());
		else
			m_caption->InitTexture(m_texture_minus.c_str());
	}
}