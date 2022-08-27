#pragma once
#include "UIWindow.h"
#include "../EntityCondition.h"
#include "UIDoubleProgressBar.h"

class CUIXml;
class CUIStatic;
class CUITextWnd;
class CUIInventoryItemInfo;
class CInventoryItem;

class CUIInventoryItem : public CUIWindow
{
public:
	CUIInventoryItem();
	virtual				~CUIInventoryItem();
	void				InitFromXml(CUIXml& xml);
	void				SetInfo(const shared_str& section);

protected:
	CUIInventoryItemInfo*	m_af_radius;
	CUIInventoryItemInfo*	m_af_vis_radius;
	CUIInventoryItemInfo*	m_charge_level;
	CUIInventoryItemInfo*	m_max_charge;
	CUIInventoryItemInfo*	m_uncharge_speed;
	CUIStatic*				m_Prop_line;

}; // class CUIInventoryItem

// -----------------------------------

class CUIInventoryItemInfo : public CUIWindow
{
public:
	CUIInventoryItemInfo();
	virtual		~CUIInventoryItemInfo();

	void	Init(CUIXml& xml, LPCSTR section);
	void	SetCaption(LPCSTR name);
	void	SetValue(int vle, float value);

private:
	CUIStatic*	m_caption;
	CUIStatic*	m_value;
	float		m_magnitude;
	bool		m_show_sign;
	shared_str	m_unit_str;
	shared_str	m_texture_minus;
	shared_str	m_texture_plus;

}; // class CUIInventoryItemInfo

// -------------------------------------------------------------------------------------------------

class CUIItemConditionParams : public CUIWindow
{
public:
	CUIItemConditionParams();
	virtual					~CUIItemConditionParams();

	void 					InitFromXml(CUIXml& xml_doc);
	void					SetInfo(CInventoryItem const* slot_item, CInventoryItem const& cur_item);

protected:
	CUIStatic				m_icon_charge;
	CUIStatic				m_textCharge;
	CUIDoubleProgressBar	m_ProgressCurCharge;
};