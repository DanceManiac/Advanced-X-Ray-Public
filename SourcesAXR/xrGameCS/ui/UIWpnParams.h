#pragma once
#include "UIWindow.h"

#include "UIDoubleProgressBar.h"

class CUIXml;
class CInventoryItem;

#include "../../XrServerEntitiesCS/script_export_space.h"

struct SLuaWpnParams;

class CUIWpnParams : public CUIWindow 
{
public:
							CUIWpnParams		();
	virtual					~CUIWpnParams		();

	void 					InitFromXml			(CUIXml& xml_doc);
	void					SetInfo				(CInventoryItem* slot_wpn, CInventoryItem& cur_wpn);
	bool 					Check				(CInventoryItem& wpn_section);
	int						ammo_types_size;	//Lex Addon

protected:
	CUIDoubleProgressBar	m_progressAccuracy; // red or green
	CUIDoubleProgressBar	m_progressHandling;
	CUIDoubleProgressBar	m_progressDamage;
	CUIDoubleProgressBar	m_progressRPM;
	CUIDoubleProgressBar	m_progressRange;
	CUIDoubleProgressBar	m_progressDamageSil;
	CUIDoubleProgressBar	m_progressImp;

	CUIStatic				m_textAccuracy;
	CUIStatic				m_textHandling;
	CUIStatic				m_textDamage;
	CUIStatic				m_textRPM;
	CUIStatic				m_textDamageSil;
	CUIStatic				m_textRange;
	CUIStatic				m_textImp;

	CUIStatic				m_stAmmo;
	CUIStatic				m_textAmmoTypes;
	CUIStatic				m_textAmmoUsedType;
	CUIStatic				m_textAmmoCount;
	CUIStatic				m_textAmmoCount2;
	CUIStatic				m_stAmmoType1;
	CUIStatic				m_stAmmoType2;
	//	CUIStatic				m_stAmmoType3;
};

// -------------------------------------------------------------------------------------------------

class CUIConditionParams : public CUIWindow 
{
public:
							CUIConditionParams	();
	virtual					~CUIConditionParams	();

	void 					InitFromXml			(CUIXml& xml_doc);
	void					SetInfo				(CInventoryItem const* slot_wpn, CInventoryItem const& cur_wpn);

protected:
	CUIDoubleProgressBar	m_progress; // red or green
	CUIStatic				m_text;
};
