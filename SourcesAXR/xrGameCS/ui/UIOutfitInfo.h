#pragma once
#include "UIWindow.h"
#include "../EntityCondition.h"
#include "UIDoubleProgressBar.h"
#include "..\..\xrServerEntities\alife_space.h"

class CUIXml;
class CUIStatic;
class CUITextWnd;
class CUIOutfitItemInfo;
class CInventoryItem;

class CCustomOutfit;

class CUIOutfitItem : public CUIWindow
{
public:
	CUIOutfitItem();
	virtual				~CUIOutfitItem();
	void				InitFromXml(CUIXml& xml);
	void				SetInfo(CInventoryItem& pInvItem);
	void				SetInfo(CCustomOutfit* cur_outfit, CCustomOutfit* slot_outfit);

protected:

	enum { max_count = ALife::eHitTypeMax - 2 };

	CUIOutfitItemInfo*		m_artefacts_count;
	CUIOutfitItemInfo*		m_additional_weight;
	CUIOutfitItemInfo*		m_inv_capacity;

	CUIOutfitItemInfo*		m_items[max_count];
	CUIOutfitItemInfo*		m_outfit_filter_condition;

	CUIDoubleProgressBar	m_progress;
	CUIStatic*				m_value; // 100%
	float					m_magnitude;

}; // class CUIInventoryItem

// -----------------------------------

class CUIOutfitItemInfo : public CUIWindow
{
public:
	CUIOutfitItemInfo();
	virtual		~CUIOutfitItemInfo();

	void	Init(CUIXml& xml, LPCSTR section);
	void	Init(CUIXml& xml, LPCSTR section, int mode);
	void	SetCaption(LPCSTR name);
	void	SetValue(float value, int vle = 0);

	void	SetProgressValue(float cur, float comp);

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

	CUIDoubleProgressBar	m_progress;

}; // class CUIInventoryItemInfo