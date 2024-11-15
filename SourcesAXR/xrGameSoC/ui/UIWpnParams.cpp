#include "pch_script.h"
#include "UIWpnParams.h"
#include "UIXmlInit.h"
#include "../Level.h"
#include "game_base_space.h"
#include "../ai_space.h"
#include "script_engine.h"
#include "inventory_item_object.h"
#include "Weapon.h"
#include "WeaponBinoculars.h"
#include "WeaponKnife.h"
#include "Silencer.h"

struct SLuaWpnParams{
	luabind::functor<float>		m_functorRPM;
	luabind::functor<float>		m_functorAccuracy;
	luabind::functor<float>		m_functorDamage;
	luabind::functor<float>		m_functorDamageMP;
	luabind::functor<float>		m_functorHandling;
	SLuaWpnParams();
	~SLuaWpnParams();
};

SLuaWpnParams::SLuaWpnParams()
{
	bool	functor_exists;
	functor_exists	= ai().script_engine().functor("ui_wpn_params.GetRPM" ,		m_functorRPM);			VERIFY(functor_exists);
	functor_exists	= ai().script_engine().functor("ui_wpn_params.GetDamage" ,	m_functorDamage);		VERIFY(functor_exists);
	functor_exists	= ai().script_engine().functor("ui_wpn_params.GetDamageMP" ,m_functorDamageMP);	VERIFY(functor_exists);
	functor_exists	= ai().script_engine().functor("ui_wpn_params.GetHandling" ,m_functorHandling);	VERIFY(functor_exists);
	functor_exists	= ai().script_engine().functor("ui_wpn_params.GetAccuracy" ,m_functorAccuracy);	VERIFY(functor_exists);
}

SLuaWpnParams::~SLuaWpnParams()
{
}

SLuaWpnParams* g_lua_wpn_params = NULL;

void destroy_lua_wpn_params()
{
	if(g_lua_wpn_params)
		xr_delete(g_lua_wpn_params);
}

CUIWpnParams::CUIWpnParams(){
	AttachChild(&m_textAccuracy);
	AttachChild(&m_textDamage);
	AttachChild(&m_textHandling);
	AttachChild(&m_textRPM);

	AttachChild(&m_progressAccuracy);
	AttachChild(&m_progressDamage);
	AttachChild(&m_progressHandling);
	AttachChild(&m_progressRPM);
}

CUIWpnParams::~CUIWpnParams()
{
}

void CUIWpnParams::InitFromXml(CUIXml& xml_doc){
	if (!xml_doc.NavigateToNode("wpn_params", 0))	return;
	CUIXmlInit::InitWindow			(xml_doc, "wpn_params", 0, this);

	CUIXmlInit::InitStatic			(xml_doc, "wpn_params:cap_accuracy",		0, &m_textAccuracy);
	CUIXmlInit::InitStatic			(xml_doc, "wpn_params:cap_damage",			0, &m_textDamage);
	CUIXmlInit::InitStatic			(xml_doc, "wpn_params:cap_handling",		0, &m_textHandling);
	CUIXmlInit::InitStatic			(xml_doc, "wpn_params:cap_rpm",				0, &m_textRPM);

	CUIXmlInit::InitProgressBar		(xml_doc, "wpn_params:progress_accuracy",	0, &m_progressAccuracy);
	CUIXmlInit::InitProgressBar		(xml_doc, "wpn_params:progress_damage",		0, &m_progressDamage);
	CUIXmlInit::InitProgressBar		(xml_doc, "wpn_params:progress_handling",	0, &m_progressHandling);
	CUIXmlInit::InitProgressBar		(xml_doc, "wpn_params:progress_rpm",		0, &m_progressRPM);

	m_progressAccuracy.SetRange		(0, 100);
	m_progressDamage.SetRange		(0, 100);
	m_progressHandling.SetRange		(0, 100);
	m_progressRPM.SetRange			(0, 100);

}

void CUIWpnParams::SetInfo(CInventoryItem const& cur_wpn)
{
	if (!g_lua_wpn_params)
		g_lua_wpn_params = xr_new<SLuaWpnParams>();

	LPCSTR cur_section = cur_wpn.object().cNameSect().c_str();
	m_progressRPM.SetProgressPos		(g_lua_wpn_params->m_functorRPM(cur_section));
	m_progressAccuracy.SetProgressPos	(g_lua_wpn_params->m_functorAccuracy(cur_section));
	if (GameID() == GAME_SINGLE)
		m_progressDamage.SetProgressPos	(g_lua_wpn_params->m_functorDamage(cur_section));
	else
		m_progressDamage.SetProgressPos	(g_lua_wpn_params->m_functorDamageMP(cur_section));
	m_progressHandling.SetProgressPos	(g_lua_wpn_params->m_functorHandling(cur_section));

	const bool showAmmo = READ_IF_EXISTS(pSettings, r_bool, cur_section, "show_ammo", true);

	m_progressRPM.Show(showAmmo);
	m_progressAccuracy.Show(showAmmo);
	m_textAccuracy.Show(showAmmo);
	m_textRPM.Show(showAmmo);
}

bool CUIWpnParams::Check(CInventoryItem& wpn_section)
{
	LPCSTR wpn_sect = wpn_section.object().cNameSect().c_str();
	CWeapon* wpn = smart_cast<CWeapon*>(&wpn_section);
	if (pSettings->line_exist(wpn_sect, "fire_dispersion_base"))
	{
		if (smart_cast<CSilencer*>(&wpn_section))
			return false;
		if (smart_cast<CWeaponBinoculars*>(&wpn_section))
			return false;
		if (smart_cast<CWeaponKnife*>(&wpn_section))
			return false;
		if (!wpn->m_bShowWpnStats)
			return false;

		return true;
	}
	return false;
}