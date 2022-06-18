#pragma once
#include "UIWindow.h"

#include "UIDoubleProgressBar.h"

class CUIXml;
class CInventoryItem;

#include "../../xrServerEntities/script_export_space.h"

struct SLuaWpnParams;

class CUIWpnParams : public CUIWindow 
{
public:
							CUIWpnParams		();
	virtual					~CUIWpnParams		();

	void 					InitFromXml			(CUIXml& xml_doc);
	void					SetInfo				(CInventoryItem* slot_wpn, CInventoryItem& cur_wpn);
	bool 					Check				(const shared_str& wpn_section);
	int						ammo_types_size; //Lex Addon

protected:
	CUIDoubleProgressBar	m_progressAccuracy; // red or green
	CUIDoubleProgressBar	m_progressHandling;
	CUIDoubleProgressBar	m_progressDamage;
	CUIDoubleProgressBar	m_progressRPM;

	CUIStatic				m_icon_acc;
	CUIStatic				m_icon_dam;
	CUIStatic				m_icon_han;
	CUIStatic				m_icon_rpm;

	CUIStatic				m_stAmmo;
	CUITextWnd				m_textAccuracy;
	CUITextWnd				m_textHandling;
	CUITextWnd				m_textDamage;
	CUITextWnd				m_textRPM;
	CUITextWnd				m_textAmmoTypes;
	CUITextWnd				m_textAmmoUsedType;
	CUITextWnd				m_textAmmoCount;
	CUITextWnd				m_textAmmoCount2;
	CUIStatic				m_stAmmoType1;
	CUIStatic				m_stAmmoType2;
	// Lex Addon (correct by Suhar_) 7.08.2018		(begin)
	// Вводим дополнительные переменные для отображения 3-го и 4-го типа патронов в свойствах оружия
	/*CUIStatic				m_stAmmoType3;
	CUIStatic				m_stAmmoType4;
	CUIStatic				m_stAmmoType5;
	CUIStatic				m_stAmmoType6;*/
	// Lex Addon (correct by Suhar_) 7.08.2018		(end)
	CUIStatic				m_Prop_line;
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
