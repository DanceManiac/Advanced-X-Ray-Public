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

CUIBoosterInfo::CUIBoosterInfo()
{
	m_booster_satiety = NULL;
	m_booster_health = NULL;
	m_booster_radiation = NULL;
	m_booster_power = NULL;
	m_booster_bleeding = NULL;
	m_booster_battery = NULL;
	m_booster_thirst = NULL;
	m_booster_intoxication = NULL;
	m_booster_sleepeness = NULL;
	m_booster_alcoholism = NULL;
	m_booster_hangover = NULL;
	m_booster_narcotism = NULL;
	m_booster_withdrawal = NULL;
	m_portions = NULL;
}

CUIBoosterInfo::~CUIBoosterInfo()
{
	xr_delete(m_booster_satiety);
	xr_delete(m_booster_health);
	xr_delete(m_booster_radiation);
	xr_delete(m_booster_power);
	xr_delete(m_booster_bleeding);
	xr_delete(m_booster_battery);
	xr_delete(m_booster_thirst);
	xr_delete(m_booster_intoxication);
	xr_delete(m_booster_sleepeness);
	xr_delete(m_booster_alcoholism);
	xr_delete(m_booster_hangover);
	xr_delete(m_booster_narcotism);
	xr_delete(m_booster_withdrawal);
	xr_delete(m_portions);
	xr_delete(m_Prop_line);
}

LPCSTR boost_influence_caption[] =
{
	"ui_inv_health",
	"ui_inv_power",
	"ui_inv_radiation",
	"ui_inv_bleeding",
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

	m_Prop_line = xr_new<CUIStatic>();
	AttachChild(m_Prop_line);
	m_Prop_line->SetAutoDelete(false);
	CUIXmlInit::InitStatic(xml, "prop_line", 0, m_Prop_line);

	m_booster_health = xr_new<UIBoosterInfoItem>();
	m_booster_health->Init(xml, "boost_health_restore");
	m_booster_health->SetAutoDelete(false);
	LPCSTR name = CStringTable().translate("ui_inv_health").c_str();
	m_booster_health->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_booster_radiation = xr_new<UIBoosterInfoItem>();
	m_booster_radiation->Init(xml, "boost_radiation_restore");
	m_booster_radiation->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_radiation").c_str();
	m_booster_radiation->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_booster_satiety = xr_new<UIBoosterInfoItem>();
	m_booster_satiety->Init(xml, "boost_satiety");
	m_booster_satiety->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_satiety").c_str();
	m_booster_satiety->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_booster_power = xr_new<UIBoosterInfoItem>();
	m_booster_power->Init(xml, "boost_power_restore");
	m_booster_power->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_power").c_str();
	m_booster_power->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_booster_bleeding = xr_new<UIBoosterInfoItem>();
	m_booster_bleeding->Init(xml, "boost_bleeding_restore");
	m_booster_bleeding->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_bleeding").c_str();
	m_booster_bleeding->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_booster_battery = xr_new<UIBoosterInfoItem>();
	m_booster_battery->Init(xml, "boost_battery");
	m_booster_battery->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_battery").c_str();
	m_booster_battery->SetCaption(name);
	xml.SetLocalRoot(base_node);

	//M.F.S. Team Thirst
	m_booster_thirst = xr_new<UIBoosterInfoItem>();
	m_booster_thirst->Init(xml, "boost_thirst");
	m_booster_thirst->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_thirst").c_str();
	m_booster_thirst->SetCaption(name);
	xml.SetLocalRoot(base_node);

	//M.F.S. Team Intoxication
	m_booster_intoxication = xr_new<UIBoosterInfoItem>();
	m_booster_intoxication->Init(xml, "boost_intoxication");
	m_booster_intoxication->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_intoxication").c_str();
	m_booster_intoxication->SetCaption(name);
	xml.SetLocalRoot(base_node);

	//M.F.S. Team Sleepeness
	m_booster_sleepeness = xr_new<UIBoosterInfoItem>();
	m_booster_sleepeness->Init(xml, "boost_sleepeness");
	m_booster_sleepeness->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_sleepeness").c_str();
	m_booster_sleepeness->SetCaption(name);
	xml.SetLocalRoot(base_node);

	//M.F.S. Team Alcoholism (HoP)
	m_booster_alcoholism = xr_new<UIBoosterInfoItem>();
	m_booster_alcoholism->Init(xml, "boost_alcoholism");
	m_booster_alcoholism->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_alcoholism").c_str();
	m_booster_alcoholism->SetCaption(name);
	xml.SetLocalRoot(base_node);

	//M.F.S. Team Hangover (HoP)
	m_booster_hangover = xr_new<UIBoosterInfoItem>();
	m_booster_hangover->Init(xml, "boost_hangover");
	m_booster_hangover->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_hangover").c_str();
	m_booster_hangover->SetCaption(name);
	xml.SetLocalRoot(base_node);

	//M.F.S. Team Narcotism (HoP)
	m_booster_narcotism = xr_new<UIBoosterInfoItem>();
	m_booster_narcotism->Init(xml, "boost_narcotism");
	m_booster_narcotism->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_narcotism").c_str();
	m_booster_narcotism->SetCaption(name);
	xml.SetLocalRoot(base_node);

	//M.F.S. Team Drug Withdrawal (HoP)
	m_booster_withdrawal = xr_new<UIBoosterInfoItem>();
	m_booster_withdrawal->Init(xml, "boost_withdrawal");
	m_booster_withdrawal->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_withdrawal").c_str();
	m_booster_withdrawal->SetCaption(name);
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

	if (pSettings->line_exist(section.c_str(), "eat_health"))
	{
		val = pSettings->r_float(section, "eat_health");
		if (!fis_zero(val))
		{
			m_booster_health->SetValue(val);
			pos.set(m_booster_health->GetWndPos());
			pos.y = h;
			m_booster_health->SetWndPos(pos);

			h += m_booster_health->GetWndSize().y;
			AttachChild(m_booster_health);
		}
	}

	if (pSettings->line_exist(section.c_str(), "eat_radiation"))
	{
		val = pSettings->r_float(section, "eat_radiation");
		if (!fis_zero(val))
		{
			m_booster_radiation->SetValue(val);
			pos.set(m_booster_radiation->GetWndPos());
			pos.y = h;
			m_booster_radiation->SetWndPos(pos);

			h += m_booster_radiation->GetWndSize().y;
			AttachChild(m_booster_radiation);
		}
	}

	if (pSettings->line_exist(section.c_str(), "eat_power"))
	{
		val = pSettings->r_float(section, "eat_power");
		if (!fis_zero(val))
		{
			m_booster_power->SetValue(val);
			pos.set(m_booster_power->GetWndPos());
			pos.y = h;
			m_booster_power->SetWndPos(pos);

			h += m_booster_power->GetWndSize().y;
			AttachChild(m_booster_power);
		}
	}

	if (pSettings->line_exist(section.c_str(), "wounds_heal_perc"))
	{
		val = pSettings->r_float(section, "wounds_heal_perc");
		if (!fis_zero(val))
		{
			m_booster_bleeding->SetValue(val);
			pos.set(m_booster_bleeding->GetWndPos());
			pos.y = h;
			m_booster_bleeding->SetWndPos(pos);

			h += m_booster_bleeding->GetWndSize().y;
			AttachChild(m_booster_bleeding);
		}
	}

	if (pSettings->line_exist(section.c_str(), "eat_satiety"))
	{
		val = pSettings->r_float(section, "eat_satiety");
		if (!fis_zero(val))
		{
			m_booster_satiety->SetValue(val);
			pos.set(m_booster_satiety->GetWndPos());
			pos.y = h;
			m_booster_satiety->SetWndPos(pos);

			h += m_booster_satiety->GetWndSize().y;
			AttachChild(m_booster_satiety);
		}
	}

	if (pSettings->line_exist(section.c_str(), "charge_level"))
	{
		val = pSettings->r_float(section, "charge_level");
		if (!fis_zero(val))
		{
			m_booster_battery->SetValue(val);
			pos.set(m_booster_battery->GetWndPos());
			pos.y = h;
			m_booster_battery->SetWndPos(pos);

			h += m_booster_battery->GetWndSize().y;
			AttachChild(m_booster_battery);
		}
	}

	//M.F.S. Team Thirst
	if (pSettings->line_exist(section.c_str(), "eat_thirst"))
	{
		val = pSettings->r_float(section, "eat_thirst");
		if (!fis_zero(val))
		{
			m_booster_thirst->SetValue(val);
			pos.set(m_booster_thirst->GetWndPos());
			pos.y = h;
			m_booster_thirst->SetWndPos(pos);

			h += m_booster_thirst->GetWndSize().y;
			AttachChild(m_booster_thirst);
		}
	}

	//M.F.S. Team Intoxication
	if (pSettings->line_exist(section.c_str(), "eat_intoxication"))
	{
		val = pSettings->r_float(section, "eat_intoxication");
		if (!fis_zero(val))
		{
			m_booster_intoxication->SetValue(val);
			pos.set(m_booster_intoxication->GetWndPos());
			pos.y = h;
			m_booster_intoxication->SetWndPos(pos);

			h += m_booster_intoxication->GetWndSize().y;
			AttachChild(m_booster_intoxication);
		}
	}

	//M.F.S. Team Sleepeness
	if (pSettings->line_exist(section.c_str(), "eat_sleepeness"))
	{
		val = pSettings->r_float(section, "eat_sleepeness");
		if (!fis_zero(val))
		{
			m_booster_sleepeness->SetValue(val);
			pos.set(m_booster_sleepeness->GetWndPos());
			pos.y = h;
			m_booster_sleepeness->SetWndPos(pos);

			h += m_booster_sleepeness->GetWndSize().y;
			AttachChild(m_booster_sleepeness);
		}
	}

	//M.F.S. Team Alcoholism (HoP)
	if (pSettings->line_exist(section.c_str(), "eat_alcoholism"))
	{
		val = pSettings->r_float(section, "eat_alcoholism");
		if (!fis_zero(val))
		{
			m_booster_alcoholism->SetValue(val);
			pos.set(m_booster_alcoholism->GetWndPos());
			pos.y = h;
			m_booster_alcoholism->SetWndPos(pos);

			h += m_booster_alcoholism->GetWndSize().y;
			AttachChild(m_booster_alcoholism);
		}
	}

	//M.F.S. Team Hangover (HoP)
	if (pSettings->line_exist(section.c_str(), "eat_hangover"))
	{
		val = pSettings->r_float(section, "eat_hangover");
		if (!fis_zero(val))
		{
			m_booster_hangover->SetValue(val);
			pos.set(m_booster_hangover->GetWndPos());
			pos.y = h;
			m_booster_hangover->SetWndPos(pos);

			h += m_booster_hangover->GetWndSize().y;
			AttachChild(m_booster_hangover);
		}
	}

	//M.F.S. Team Narcotism (HoP)
	if (pSettings->line_exist(section.c_str(), "eat_narcotism"))
	{
		val = pSettings->r_float(section, "eat_narcotism");
		if (!fis_zero(val))
		{
			m_booster_narcotism->SetValue(val);
			pos.set(m_booster_narcotism->GetWndPos());
			pos.y = h;
			m_booster_narcotism->SetWndPos(pos);

			h += m_booster_narcotism->GetWndSize().y;
			AttachChild(m_booster_narcotism);
		}
	}

	//M.F.S. Team Drug Withdrawal (HoP)
	if (pSettings->line_exist(section.c_str(), "eat_withdrawal"))
	{
		val = pSettings->r_float(section, "eat_withdrawal");
		if (!fis_zero(val))
		{
			m_booster_withdrawal->SetValue(val);
			pos.set(m_booster_withdrawal->GetWndPos());
			pos.y = h;
			m_booster_withdrawal->SetWndPos(pos);

			h += m_booster_withdrawal->GetWndSize().y;
			AttachChild(m_booster_withdrawal);
		}
	}

	//Portions
	if (eatable)
	{
		val = eatable->m_iPortionsNum;

		if (!fis_zero(val))
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

void UIBoosterInfoItem::SetValue(float value)
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
	m_value->SetTextColor(color_rgba(170, 170, 170, 255));

	if (m_texture_minus.size())
	{
		if (positive)
			m_caption->InitTexture(m_texture_plus.c_str());
		else
			m_caption->InitTexture(m_texture_minus.c_str());
	}
}