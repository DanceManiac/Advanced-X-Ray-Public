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
	void				SetInfo(CInventoryItem& pInvItem);

protected:
	CUIInventoryItemInfo*	m_af_radius;
	CUIInventoryItemInfo*	m_af_vis_radius;
	CUIInventoryItemInfo*	m_charge_level;
	CUIInventoryItemInfo*	m_filter_cond;
	CUIInventoryItemInfo*	m_rep_kit_cond;
	CUIInventoryItemInfo*	m_max_charge;
	CUIInventoryItemInfo*	m_uncharge_speed;
	CUIInventoryItemInfo*	m_artefacts_count;
	CUIInventoryItemInfo*	m_additional_weight;
	CUIInventoryItemInfo*	m_inv_capacity;

	xr_vector<CUIStatic*>	m_textArtefacts;
	xr_vector<CUIStatic*>	m_stArtefacts;

	CUIStatic*				m_Prop_line;

	int						m_iMaxAfCount;
	int						m_iIncWndHeight;
	float					m_stArtefactsScale;

}; // class CUIInventoryItem

// -----------------------------------

class CUIInventoryItemInfo : public CUIWindow
{
public:
	CUIInventoryItemInfo();
	virtual		~CUIInventoryItemInfo();

	void	Init(CUIXml& xml, LPCSTR section);
	void	SetCaption(LPCSTR name);
	void	SetValue(float value, int vle = 0, int accuracy = 0);

private:
	CUIStatic*	m_caption;
	CUIStatic*	m_value;
	float		m_magnitude;
	bool		m_show_sign;
	shared_str	m_unit_str;
	shared_str	m_texture;

	//Color
	u32			m_negative_color;
	u32			m_neutral_color;
	u32			m_positive_color;
	bool		clr_invert;
	bool		use_color;
	bool		clr_dynamic;

}; // class CUIInventoryItemInfo
