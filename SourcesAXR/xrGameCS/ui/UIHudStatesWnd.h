#pragma once

#include "UIWindow.h"
#include "..\..\XrServerEntitiesCS\alife_space.h"
#include "..\..\xrServerEntitiesCS\inventory_space.h"
#include "..\actor_defs.h"

class CUIStatic;
class CUIProgressBar;
class CUIProgressShape;
class CUIXml;
class UI_Arrow;
class CActor;

int const it_max = ALife::infl_max_count - 5;

class CUIHudStatesWnd : public CUIWindow
{
private:
	typedef CUIWindow						inherited;
//-	typedef ALife::EInfluenceType	EIndicatorType;

public:
	CUIStatic*			m_back;
	CUIStatic*			m_back_v;
	CUIStatic*			m_back_over_arrow;
	CUIStatic*			m_static_armor;

	CUIStatic*			m_back_hud_lfo_a;
	CUIStatic*			m_back_hud_lfo_b;
	CUIStatic*			m_back_hud_lfo_temp;
	CUIStatic*			m_back_hud_lfo_stamina;
	CUIStatic*			m_back_hud_lfo_bleeding;
	CUIStatic*			m_back_hud_lfo_quickslots;
	CUIStatic*			m_back_hud_lfo_anomaly;
	CUIStatic*			m_back_hud_lfo_artefacts;
	CUIStatic*			m_back_hud_lfo_battery;

	CUIStatic*			m_resist_back[it_max];
	CUIStatic*			m_indik[it_max];

	CUIStatic*			m_ui_weapon_cur_ammo;
	CUIStatic*			m_ui_weapon_fmj_ammo;
	CUIStatic*			m_ui_weapon_ap_ammo;
	CUIStatic*			m_fire_mode;
	CUIStatic*			m_ui_grenade;
	II_BriefInfo		m_item_info;
	
	CUIStatic*			m_ui_weapon_icon;
	Frect				m_ui_weapon_icon_rect;
	float				m_ui_weapon_icon_scale;

	CUIProgressBar*		m_ui_health_bar;
	CUIProgressBar*		m_ui_armor_bar;
	CUIProgressBar*		m_ui_stamina_bar;
	CUIProgressBar*		m_ui_satiety_bar;
	CUIProgressBar*		m_ui_thirsty_bar;
	CUIProgressBar*		m_ui_tired_bar;
	CUIProgressBar*		m_ui_infection_bar;

	CUIProgressShape*	m_progress_self;
	CUIStatic*			m_radia_damage;
	UI_Arrow*			m_arrow;
	UI_Arrow*			m_arrow_shadow;
	
	CUIStatic*			m_bleeding;
	
//	float				m_actor_radia_factor;
	shared_str			m_lanim_name;

	u32					m_last_time;

	bool				m_cur_state_LA[it_max];
	bool				m_b_force_update;
	
protected:

	CUIStatic*			_ind_position_set;
	CUIStatic*			_ind_block_icon;
	int					_ind_max_icons{};
	bool				_ind_is_vertical_attrib{};
	bool				_ind_clr_only_one_attrib{};
	u32					_ind_clr_fixed{};
	u32					_ind_pos_shift_add{};
	bool				_ind_dir_left{};
	bool				_ind_dir_bottom{};
	//bool				_ind_is_centered{};
	CUIStatic*			_ind_radiation;
	CUIStatic*			_ind_alcohol;
	CUIStatic*			_ind_starvation;
	CUIStatic*			_ind_thirst;
	CUIStatic*			_ind_weapon_broken;
	CUIStatic*			_ind_bleeding;
	CUIStatic*			_ind_psyhealth;
	CUIStatic*			_ind_overweight;
	CUIStatic*			_ind_stamina;
	CUIStatic*			_ind_health;

	CUIStatic*			_ind_intoxication;
	CUIStatic*			_ind_sleepeness;
	CUIStatic*			_ind_alcoholism;
	CUIStatic*			_ind_hangover;
	CUIStatic*			_ind_narcotism;
	CUIStatic*			_ind_withdrawal;
	CUIStatic*			_ind_drugs;
	CUIStatic*			_ind_frostbite;
	CUIStatic*			_ind_heating;
	CUIStatic*			_ind_outfit_broken;
	CUIStatic*			_ind_filter;
	CUIStatic*			_ind_helmet_broken;
	CUIStatic*			_ind_helmet_2_broken;
	CUIStatic*			_ind_battery;

public:
//	CZoneList*	m_zones_list; <----- in Level()

	UI_Arrow*		get_arrow			() {return m_arrow;}
	UI_Arrow*		get_arrow_shadow	() {return m_arrow_shadow;}

					CUIHudStatesWnd		();
	virtual			~CUIHudStatesWnd	();

			void	InitFromXml			( CUIXml& xml, LPCSTR path );
			void	Load_section		();
	virtual void	Update				();
//	virtual void	Draw				();

			void	on_connected		();
			void	reset_ui			();
			void	UpdateHealth		( CActor* actor );
			void	UpdateIndicatorIcons	( CActor* actor );
			void	SetAmmoIcon			( const shared_str& sect_name );
			void	UpdateActiveItemInfo( CActor* actor );

			void	UpdateIndicators	( CActor* actor );

protected:

			void	Load_section_type	( ALife::EInfluenceType type, LPCSTR section );
			void	UpdateIndicatorType	( CActor* actor, ALife::EInfluenceType type );
			void	SwitchLA			( bool state, ALife::EInfluenceType type );

}; // class CUIHudStatesWnd
