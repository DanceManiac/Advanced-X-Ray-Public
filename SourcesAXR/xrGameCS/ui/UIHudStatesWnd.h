#ifndef	UI_HUD_STATES_WND_H_INCLUDED
#define UI_HUD_STATES_WND_H_INCLUDED

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
			void	SetAmmoIcon			( const shared_str& sect_name );
			void	UpdateActiveItemInfo( CActor* actor );

			void	UpdateIndicators	( CActor* actor );

protected:

			void	Load_section_type	( ALife::EInfluenceType type, LPCSTR section );
			void	UpdateIndicatorType	( CActor* actor, ALife::EInfluenceType type );
			void	SwitchLA			( bool state, ALife::EInfluenceType type );

}; // class CUIHudStatesWnd

#endif // UI_HUD_STATES_WND_H_INCLUDED
