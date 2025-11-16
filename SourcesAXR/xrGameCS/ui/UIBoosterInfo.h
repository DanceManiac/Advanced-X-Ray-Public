#pragma once
#include "UIWindow.h"
#include "../EntityCondition.h"

class CUIXml;
class CUIStatic;
class CUITextWnd;
class UIBoosterInfoItem;
class CInventoryItem;

class CUIBoosterInfo : public CUIWindow
{
public:
	CUIBoosterInfo();
	virtual			~CUIBoosterInfo();
	void	InitFromXml(CUIXml& xml);
	void	SetInfo(CInventoryItem& pInvItem);

protected:
	enum
	{
		_item_start = 0,
		_item_boost_health_restore = _item_start,
		_item_boost_power_restore,
		_item_boost_radiation_restore,
		_item_boost_bleeding_restore,
		_item_boost_satiety_restore,
		_item_boost_thirst_restore,
		_item_boost_psy_health_restore,
		_item_boost_intoxication_restore,
		_item_boost_sleepeness_restore,
		_item_boost_alcohol_restore,
		_item_boost_alcoholism_restore,
		_item_boost_hangover_restore,
		_item_boost_drugs_restore,
		_item_boost_narcotism_restore,
		_item_boost_withdrawal_restore,
		_item_boost_frostbite_restore,
		_item_boost_max_weight,
		_item_boost_radiation_protection,
		_item_boost_telepat_protection,
		_item_boost_chemburn_protection,
		_item_boost_burn_immunity,
		_item_boost_shock_immunity,
		_item_boost_radiation_immunity,
		_item_boost_telepat_immunity,
		_item_boost_chemburn_immunity,
		_item_boost_explosion_immunity,
		_item_boost_strike_immunity,
		_item_boost_fire_wound_immunity,
		_item_boost_wound_immunity,

		eBoostExplImmunity
	};

	enum
	{
		_item_quick_start = 0,
		_item_quick_health = _item_quick_start,
		_item_quick_power,
		_item_quick_bleeding,

		_item_quick_satiety,

		//M.F.S Team additions
		_item_quick_thirst,
		_item_quick_psy_health,

		_item_quick_intoxication,
		_item_quick_radiation,
		_item_quick_sleepeness,

		//HoP
		_item_quick_alcohol,
		_item_quick_alcoholism,
		_item_quick_hangover,
		_item_quick_drugs,
		_item_quick_narcotism,
		_item_quick_withdrawal,

		_item_quick_frostbite,

		eQuickItemLast
	};
	UIBoosterInfoItem* m_booster_items[eBoostExplImmunity];
	UIBoosterInfoItem* m_quick_items[eQuickItemLast];
	UIBoosterInfoItem* m_portions;
	UIBoosterInfoItem* m_booster_time;

	CUIStatic* m_Prop_line;

}; // class CUIBoosterInfo

// -----------------------------------

class UIBoosterInfoItem : public CUIWindow
{
public:
	UIBoosterInfoItem();
	virtual		~UIBoosterInfoItem();

	void	Init(CUIXml& xml, LPCSTR section);
	void	SetCaption(LPCSTR name);
	void	SetValue(float value, int vle = 0, float max_val = 0);
	CUIStatic* m_value;

private:
	CUIStatic*	m_caption;
	float		m_magnitude;
	bool		m_show_sign;
	shared_str	m_unit_str;
	shared_str	m_unit_str_max;
	shared_str	m_texture;

	//Color
	u32			m_negative_color;
	u32			m_neutral_color;
	u32			m_positive_color;
	bool		clr_invert;
	bool		use_color;
	bool		clr_dynamic;

}; // class UIBoosterInfoItem