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
					CUIBoosterInfo		();
	virtual			~CUIBoosterInfo		();
			void	InitFromXml				(CUIXml& xml);
			void	SetInfo					(CInventoryItem& pInvItem);

protected:

	enum 
	{
		_item_start = 0,
		_item_boost_health_restore = _item_start,
		_item_boost_power_restore,
		_item_boost_radiation_restore,
		_item_boost_bleeding_restore,
		_item_boost_max_weight,
		_item_boost_radiation_protection,
		_item_boost_telepat_protection,
		_item_boost_chemburn_protection,
		/*_item_boost_burn_immunity,
		_item_boost_shock_immunity,
		_item_boost_radiation_immunity,
		_item_boost_telepat_immunity,
		_item_boost_chemburn_immunity,
		_item_boost_explosion_immunity,
		_item_boost_strike_immunity,
		_item_boost_fire_wound_immunity,
		_item_boost_wound_immunity,*/
		_item_satiety,

		//M.F.S Team additions
		_item_battery,
		_item_thirst,
		_item_intoxication,
		_item_sleepeness,

		//HoP
		_item_alcoholism,
		_item_hangover,
		_item_narcotism,
		_item_withdrawal,

		eBoostExplImmunity
	};
	UIBoosterInfoItem* m_booster_items[eBoostExplImmunity];
	UIBoosterInfoItem* m_portions;
	UIBoosterInfoItem* m_booster_anabiotic;
	UIBoosterInfoItem* m_booster_time;

	CUIStatic*			m_Prop_line;

}; // class CUIBoosterInfo

// -----------------------------------

class UIBoosterInfoItem : public CUIWindow
{
public:
				UIBoosterInfoItem	();
	virtual		~UIBoosterInfoItem();
		
		void	Init				( CUIXml& xml, LPCSTR section );
		void	SetCaption			( LPCSTR name );
		void	SetValue			( int vle, float value );
	
private:
	CUIStatic*	m_caption;
	CUITextWnd*	m_value;
	float		m_magnitude;
	bool		m_show_sign;
	shared_str	m_unit_str;
	shared_str	m_texture_minus;
	shared_str	m_texture_plus;

}; // class UIBoosterInfoItem
