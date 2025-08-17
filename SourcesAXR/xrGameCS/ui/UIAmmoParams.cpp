#include "stdafx.h"
#include "UIAmmoParams.h"
#include "UIStatic.h"
#include "UIXmlInit.h"
#include "UIHelper.h"
#include "../string_table.h"
#include "../WeaponAmmo.h"
#include "../actor.h"
#include "../ActorCondition.h"
#include "../Level.h"

u32 const red_clr = color_argb(255, 210, 50, 50);
u32 const green_clr = color_argb(255, 50, 210, 50);
u32 const neutral_clr = color_argb(255, 170, 170, 170);

CUIAmmoParams::CUIAmmoParams()
{
	m_Prop_line = nullptr;
	m_ammo_box_size = nullptr;
	m_ammo_dist = nullptr;
	m_ammo_hit_prc = nullptr;
	m_ammo_ap = nullptr;
//	m_ammo_speed = nullptr;
	m_ammo_disp = nullptr;
	m_ammo_buck_shot = nullptr;
	m_ammo_impair = nullptr;
	m_ammo_tracer = nullptr;
}

CUIAmmoParams::~CUIAmmoParams()
{
	xr_delete(m_Prop_line);
	xr_delete(m_ammo_box_size);
	xr_delete(m_ammo_dist);
	xr_delete(m_ammo_hit_prc);
	xr_delete(m_ammo_ap);
//	xr_delete(m_ammo_speed);
	xr_delete(m_ammo_disp);
	xr_delete(m_ammo_buck_shot);
	xr_delete(m_ammo_impair);
	xr_delete(m_ammo_tracer);
}

void CUIAmmoParams::InitFromXml( CUIXml& xml )
{
	LPCSTR base = "ammo_params";

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

	{
		m_ammo_hit_prc = xr_new<UIAmmoParamItem>();
		m_ammo_hit_prc->Init(xml, "ammo_hit_prc");
		m_ammo_hit_prc->SetAutoDelete(false);

		LPCSTR name = CStringTable().translate("ui_inv_damage").c_str();
		m_ammo_hit_prc->SetCaption(name);

		xml.SetLocalRoot(base_node);
	}
	{
		m_ammo_ap = xr_new<UIAmmoParamItem>();
		m_ammo_ap->Init(xml, "ammo_ap");
		m_ammo_ap->SetAutoDelete(false);

		LPCSTR name = CStringTable().translate("st_stat_ap").c_str();
		m_ammo_ap->SetCaption(name);

		xml.SetLocalRoot(base_node);
	}
	{
		m_ammo_impair = xr_new<UIAmmoParamItem>();
		m_ammo_impair->Init(xml, "ammo_impair");
		m_ammo_impair->SetAutoDelete(false);

		LPCSTR name = CStringTable().translate("st_stat_impact").c_str();
		m_ammo_impair->SetCaption(name);

		xml.SetLocalRoot(base_node);
	}
	{
		m_ammo_box_size = xr_new<UIAmmoParamItem>();
		m_ammo_box_size->Init(xml, "ammo_box_size");
		m_ammo_box_size->SetAutoDelete(false);

		LPCSTR name = CStringTable().translate("st_stat_box_size").c_str();
		m_ammo_box_size->SetCaption(name);

		xml.SetLocalRoot(base_node);
	}
	{
		m_ammo_dist = xr_new<UIAmmoParamItem>();
		m_ammo_dist->Init(xml, "ammo_dist");
		m_ammo_dist->SetAutoDelete(false);

		LPCSTR name = CStringTable().translate("st_stat_k_dist").c_str();
		m_ammo_dist->SetCaption(name);

		xml.SetLocalRoot(base_node);
	}/*
	{
		m_ammo_speed = xr_new<UIAmmoParamItem>();
		m_ammo_speed->Init(xml, "ammo_speed");
		m_ammo_speed->SetAutoDelete(false);

		LPCSTR name = CStringTable().translate("st_stat_bullet_speed").c_str();
		m_ammo_speed->SetCaption(name);

		xml.SetLocalRoot(base_node);
	}*/
	{
		m_ammo_disp = xr_new<UIAmmoParamItem>();
		m_ammo_disp->Init(xml, "ammo_disp");
		m_ammo_disp->SetAutoDelete(false);

		LPCSTR name = CStringTable().translate("st_stat_k_disp").c_str();
		m_ammo_disp->SetCaption(name);

		xml.SetLocalRoot(base_node);
	}
	{
		m_ammo_buck_shot = xr_new<UIAmmoParamItem>();
		m_ammo_buck_shot->Init(xml, "ammo_buck_shot");
		m_ammo_buck_shot->SetAutoDelete(false);

		LPCSTR name = CStringTable().translate("st_stat_buck_shot").c_str();
		m_ammo_buck_shot->SetCaption(name);

		xml.SetLocalRoot(base_node);
	}
	{
		m_ammo_tracer = xr_new<UIAmmoParamItem>();
		m_ammo_tracer->Init(xml, "ammo_tracer");
		m_ammo_tracer->SetAutoDelete(false);

		LPCSTR name = CStringTable().translate("st_stat_ammo_tracer").c_str();
		m_ammo_tracer->SetCaption(name);

		xml.SetLocalRoot(base_node);
	}

	xml.SetLocalRoot(stored_root);
}

void CUIAmmoParams::SetInfo(CWeaponAmmo* ammo)
{
	DetachAll();
	AttachChild(m_Prop_line);

	CActor* actor = smart_cast<CActor*>(Level().CurrentViewEntity());
	if (!actor)
		return;

	float val,curr = 0.0f;
	Fvector2 pos;
	float h = m_Prop_line->GetWndPos().y + m_Prop_line->GetWndSize().y;

	
	val = ammo->cartridge_param.kHit;
	if (!fis_zero(val))
	{
		m_ammo_hit_prc->SetValue(val);
		pos.set(m_ammo_hit_prc->GetWndPos());
		pos.y = h;
		m_ammo_hit_prc->SetWndPos(pos);
		h += m_ammo_hit_prc->GetWndSize().y;
		AttachChild(m_ammo_hit_prc);
	}

	val = ammo->cartridge_param.kAP;
	if (!fis_zero(val))
	{
		m_ammo_ap->SetValue(val);
		pos.set(m_ammo_ap->GetWndPos());
		pos.y = h;
		m_ammo_ap->SetWndPos(pos);
		h += m_ammo_ap->GetWndSize().y;
		AttachChild(m_ammo_ap);
	}

	val = ammo->cartridge_param.impair;
	if (val!=1)
	{
		float out_val = 0.f;
		if (val > 1)
			out_val = val - 1.f;
		else
			out_val = -1.f + val;
		m_ammo_impair->SetValue(out_val);
		pos.set(m_ammo_impair->GetWndPos());
		pos.y = h;
		m_ammo_impair->SetWndPos(pos);
		h += m_ammo_impair->GetWndSize().y;
		AttachChild(m_ammo_impair);
	}

	val = ammo->cartridge_param.kDist;
	if (!fis_zero(val))
	{
		m_ammo_dist->SetValue(val);
		pos.set(m_ammo_dist->GetWndPos());
		pos.y = h;
		m_ammo_dist->SetWndPos(pos);
		h += m_ammo_dist->GetWndSize().y;
		AttachChild(m_ammo_dist);
	}/*

	val = (float)ammo->cartridge_param.kSpeed;
	if (!fis_zero(val))
	{
		m_ammo_speed->SetValue(val);
		pos.set(m_ammo_speed->GetWndPos());
		pos.y = h;
		m_ammo_speed->SetWndPos(pos);
		h += m_ammo_speed->GetWndSize().y;
		AttachChild(m_ammo_speed);
	}*/

	/*val = READ_IF_EXISTS(pSettings, r_float, ammo_section, "k_disp", 0.f);
	if (!fis_zero(val))
	{
		m_ammo_disp->SetValue(val);
		pos.set(m_ammo_disp->GetWndPos());
		pos.y = h;
		m_ammo_disp->SetWndPos(pos);
		h += m_ammo_disp->GetWndSize().y;
		AttachChild(m_ammo_disp);
	}*/
	{
		m_ammo_box_size->SetValue((float)ammo->m_boxSize, (float)ammo->m_boxCurr);
		pos.set(m_ammo_box_size->GetWndPos());
		pos.y = h;
		m_ammo_box_size->SetWndPos(pos);
		h += m_ammo_box_size->GetWndSize().y;
		AttachChild(m_ammo_box_size);
	}

	val = (float)ammo->cartridge_param.buckShot;
	if (val > 1)
	{
		m_ammo_buck_shot->SetValue(val);
		pos.set(m_ammo_buck_shot->GetWndPos());
		pos.y = h;
		m_ammo_buck_shot->SetWndPos(pos);
		h += m_ammo_buck_shot->GetWndSize().y;
		AttachChild(m_ammo_buck_shot);
	}

	bool v = ammo->m_tracer;
	if (v)
	{
		pos.set(m_ammo_tracer->GetWndPos());
		pos.y = h;
		m_ammo_tracer->SetWndPos(pos);
		h += m_ammo_tracer->GetWndSize().y;
		AttachChild(m_ammo_tracer);
	}
	
	SetHeight(h);
}

/// ----------------------------------------------------------------

UIAmmoParamItem::UIAmmoParamItem()
{
	m_caption		= nullptr;
	m_value			= nullptr;
	m_magnitude		= 1.0f;
	m_accuracy		= 0.f;
	m_show_sign		= false;
	m_sign_inverse	= false;
	m_colorize		= false;

	m_unit_str._set	("");
	m_unit_str2._set("");
	m_texture._set	("");
}

UIAmmoParamItem::~UIAmmoParamItem()
{
}

void UIAmmoParamItem::Init( CUIXml& xml, LPCSTR section )
{
	CUIXmlInit::InitWindow(xml, section, 0, this);
	xml.SetLocalRoot(xml.NavigateToNode(section));

	m_caption		= UIHelper::CreateStatic(xml, "caption", this);
	m_value			= UIHelper::CreateStatic(xml, "value", this);
	m_magnitude		= xml.ReadAttribFlt("value", 0, "magnitude", 1.0f);
	m_accuracy		= xml.ReadAttribFlt("value", 0, "accuracy", 0.0f);
	m_show_sign		= (xml.ReadAttribInt("value", 0, "show_sign", 1) == 1);
	m_sign_inverse	= (xml.ReadAttribInt("value", 0, "sign_inverse", 0) == 1);
	m_colorize		= (xml.ReadAttribInt("value", 0, "colorize", 1) == 1);

	LPCSTR unit_str = xml.ReadAttrib("value", 0, "unit_str", "");
	m_unit_str._set (CStringTable().translate(unit_str));

	LPCSTR unit_str2 = xml.ReadAttrib("value", 0, "unit_str2", "");
	m_unit_str2._set (CStringTable().translate(unit_str2));

	LPCSTR texture	= xml.Read("caption:texture", 0, "");
	m_texture._set	(texture);
	VERIFY			(m_texture.size());
}

void UIAmmoParamItem::SetCaption( LPCSTR name )
{
	m_caption->SetText(name);
}

void UIAmmoParamItem::SetValue(float value, float curr)
{
	value *= m_magnitude;
	string32 buf;
	if (curr > -1)
	{
		if (value > curr)
			xr_sprintf(buf, "%.0f / %.0f", curr, value);
		else
		{
			if (m_show_sign)
				xr_sprintf(buf, "%+.0f", value);
			else
				xr_sprintf(buf, "%.0f", value);
		}
	}
	else if (value == NULL)
		xr_sprintf(buf, "");
	else
		if (m_show_sign)
			xr_sprintf(buf, "%+.0f", value);
		else
			xr_sprintf(buf, "%.0f", value);

	LPSTR str;
	if (m_unit_str.size())
		STRCONCAT(str, buf, " ", m_unit_str.c_str());
	else if (m_unit_str2.size())
		STRCONCAT(str, buf, "", m_unit_str2.c_str());
	else
		STRCONCAT(str, buf);
	m_value->SetText(str);

	bool positive = (value >= 0.0f);
	positive = (m_sign_inverse) ? !positive : positive;
	u32 color = (positive) ? neutral_clr : red_clr;

	m_caption->InitTexture(m_texture.c_str());
	if (!positive && m_colorize)
	{
		m_value->SetTextColor(color);
		m_caption->SetTextureColor(red_clr);
	}
}
