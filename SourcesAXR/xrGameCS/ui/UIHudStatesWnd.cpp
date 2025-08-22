#include "stdafx.h"
#include "UIHudStatesWnd.h"

#include "../Actor.h"
#include "../ActorCondition.h"
#include "../EntityCondition.h"
#include "../CustomOutfit.h"
#include "../ActorHelmet.h"
#include "../inventory.h"
#include "../RadioactiveZone.h"

#include "UIActorMenu.h"
#include "UIStatic.h"
#include "UIProgressBar.h"
#include "UIProgressShape.h"
#include "UIXmlInit.h"
#include "UIHelper.h"
#include "ui_arrow.h"
#include "UIInventoryUtilities.h"
#include "../HUDManager.h"
#include "../Weapon.h"
#include "../InventoryOwner.h"
#include "CustomDetector.h"
#include "WeaponMagazined.h"
#include "WeaponMagazinedWGrenade.h"
#include "../UIGameCustom.h"
#include "../UIGameSP.h"
#include "AnomalyDetector.h"
#include "Torch.h"
#include "AdvancedXrayGameConstants.h"

CUIHudStatesWnd::CUIHudStatesWnd() : m_b_force_update(true)
{
	m_last_time = Device.dwTimeGlobal;
	m_lanim_name         = NULL;
	
//-	Load_section();
}

CUIHudStatesWnd::~CUIHudStatesWnd()
{
}

void CUIHudStatesWnd::reset_ui()
{
	if ( g_pGameLevel )
	{
		Level().hud_zones_list->clear();
	}
}

void CUIHudStatesWnd::InitFromXml( CUIXml& xml, LPCSTR path )
{
	ZoneScoped;

	CUIXmlInit::InitWindow( xml, path, 0, this );
	XML_NODE* stored_root = xml.GetLocalRoot();
	
	XML_NODE* new_root = xml.NavigateToNode( path, 0 );
	xml.SetLocalRoot( new_root );

	m_back            = UIHelper::CreateStatic( xml, "back", this );
	m_back_v          = UIHelper::CreateStatic( xml, "back_v", this );
	m_static_armor    = UIHelper::CreateStatic( xml, "static_armor", this );
	
	m_back_hud_lfo_a			= UIHelper::CreateStatic(xml, "background_lfo_base_a", this);
	m_back_hud_lfo_a			= UIHelper::CreateStatic(xml, "background_lfo_base_b", this);
	m_back_hud_lfo_temp			= UIHelper::CreateStatic(xml, "background_lfo_base_c", this);
	m_back_hud_lfo_stamina		= UIHelper::CreateStatic(xml, "background_lfo_stamina", this);
	m_back_hud_lfo_bleeding		= UIHelper::CreateStatic(xml, "background_lfo_bleeding", this);
	m_back_hud_lfo_quickslots   = UIHelper::CreateStatic(xml, "background_lfo_quickslots", this);
	m_back_hud_lfo_anomaly		= UIHelper::CreateStatic(xml, "background_lfo_anomaly_indicator", this);

	if (psHUD_Flags.test(HUD_AF_INDICATORS))
	{
		m_back_hud_lfo_battery = UIHelper::CreateStatic(xml, "background_lfo_indicators", this);
	}

	if (psHUD_Flags.test(HUD_AF_PANEL))
	{
		m_back_hud_lfo_artefacts	= UIHelper::CreateStatic(xml, "background_lfo_artefact_slots", this);
	}

	m_resist_back[ALife::infl_rad]  = UIHelper::CreateStatic( xml, "resist_back_rad", this );
	m_resist_back[ALife::infl_fire] = UIHelper::CreateStatic( xml, "resist_back_fire", this );
	m_resist_back[ALife::infl_acid] = UIHelper::CreateStatic( xml, "resist_back_acid", this );
	m_resist_back[ALife::infl_psi]  = UIHelper::CreateStatic( xml, "resist_back_psi", this );
	// electra = no has CStatic!!

	m_indik[ALife::infl_rad]  = UIHelper::CreateStatic( xml, "indik_rad", this );
	m_indik[ALife::infl_fire] = UIHelper::CreateStatic( xml, "indik_fire", this );
	m_indik[ALife::infl_acid] = UIHelper::CreateStatic( xml, "indik_acid", this );
	m_indik[ALife::infl_psi]  = UIHelper::CreateStatic( xml, "indik_psi", this );

	m_lanim_name._set( xml.ReadAttrib( "indik_rad", 0, "light_anim", "" ) );

	m_ui_weapon_cur_ammo		= UIHelper::CreateStatic( xml, "static_cur_ammo", this );
	m_ui_weapon_fmj_ammo		= UIHelper::CreateStatic( xml, "static_fmj_ammo", this );
	m_ui_weapon_ap_ammo			= UIHelper::CreateStatic( xml, "static_ap_ammo", this );
	m_fire_mode					= UIHelper::CreateStatic( xml, "static_fire_mode", this );
	m_ui_grenade				= UIHelper::CreateStatic( xml, "static_grenade", this );

	m_ui_weapon_icon			= UIHelper::CreateStatic( xml, "static_wpn_icon", this );
//	m_ui_weapon_icon->SetShader	(InventoryUtilities::GetEquipmentIconsShader());
	m_ui_weapon_icon_rect		= m_ui_weapon_icon->GetWndRect();
	m_ui_weapon_icon_scale		= xml.ReadAttribFlt("static_wpn_icon", 0,"scale", 1.f);
	
	m_ui_health_bar				= UIHelper::CreateProgressBar(xml, "progress_bar_health", this);
	m_ui_armor_bar				= UIHelper::CreateProgressBar(xml, "progress_bar_armor", this);
	m_ui_satiety_bar			= UIHelper::CreateProgressBar(xml, "progress_bar_satiety", this);
	m_ui_thirsty_bar			= UIHelper::CreateProgressBar(xml, "progress_bar_thirsty", this);
	m_ui_tired_bar				= UIHelper::CreateProgressBar(xml, "progress_bar_tired", this);
	m_ui_infection_bar			= UIHelper::CreateProgressBar(xml, "progress_bar_infection", this);

	m_progress_self = xr_new<CUIProgressShape>();
	m_progress_self->SetAutoDelete(true);
	AttachChild( m_progress_self );
	CUIXmlInit::InitProgressShape( xml, "progress", 0, m_progress_self );

	m_arrow				= xr_new<UI_Arrow>();
	m_arrow_shadow		= xr_new<UI_Arrow>();

	m_arrow->init_from_xml( xml, "arrow", this );
	m_arrow_shadow->init_from_xml( xml, "arrow_shadow", this );

	m_back_over_arrow = UIHelper::CreateStatic( xml, "back_over_arrow", this );

	m_ui_stamina_bar  = UIHelper::CreateProgressBar( xml, "progress_bar_stamina", this );

	m_bleeding = UIHelper::CreateStatic( xml, "bleeding", this );
	m_bleeding->Show( false );

	for ( int i = 0; i < it_max; ++i )
	{
		m_cur_state_LA[i] = true;
		SwitchLA( false, (ALife::EInfluenceType)i );
	}
	
	/////////////////////////////////////////
	// Dynamic status icons system -- cari0us
	_ind_position_set			= UIHelper::CreateStatic( xml, "indicator_line",			this, false);
	_ind_block_icon				= UIHelper::CreateStatic( xml, "indicator_block_icon",		this, false);
	_ind_max_icons				= xml.ReadAttribInt("indicator_line", 0, "max_icons", 50);
	_ind_is_vertical_attrib		= xml.ReadAttribInt("indicator_line", 0, "vertical", 0) == 1;
	_ind_clr_only_one_attrib	= xml.ReadAttribInt("indicator_line", 0, "clr_only_one", 0) == 1;
	_ind_clr_fixed				= xml.ReadAttribInt("indicator_line", 0, "clr_fixed", 255);
	_ind_pos_shift_add			= xml.ReadAttribInt("indicator_line", 0, "pos_shift_add", 0);
	_ind_dir_left				= xml.ReadAttribInt("indicator_line", 0, "dir_left", 0) == 1;
	_ind_dir_bottom				= xml.ReadAttribInt("indicator_line", 0, "dir_bottom", 0) == 1;
	//_ind_is_centered			= xml.ReadAttribInt("indicator_line", 0, "centered", 0) == 1;

	_ind_bleeding				= UIHelper::CreateStatic( xml, "indicator_bleeding",		this, false);
	_ind_radiation				= UIHelper::CreateStatic( xml, "indicator_radiation",		this, false);
	_ind_starvation				= UIHelper::CreateStatic( xml, "indicator_starvation",		this, false);
	_ind_thirst					= UIHelper::CreateStatic( xml, "indicator_thirst",			this, false);
	_ind_alcohol				= UIHelper::CreateStatic( xml, "indicator_alcohol",			this, false);
	_ind_psyhealth				= UIHelper::CreateStatic( xml, "indicator_psy_health",		this, false);
	_ind_weapon_broken			= UIHelper::CreateStatic( xml, "indicator_weapon_broken",	this, false);
	_ind_overweight				= UIHelper::CreateStatic( xml, "indicator_overweight",		this, false);
	_ind_stamina				= UIHelper::CreateStatic( xml, "indicator_stamina",			this, false);
	_ind_health					= UIHelper::CreateStatic( xml, "indicator_health",			this, false);

	_ind_intoxication			= UIHelper::CreateStatic( xml, "indicator_intoxication",	this, false);
	_ind_sleepeness				= UIHelper::CreateStatic( xml, "indicator_sleepeness",		this, false);
	_ind_alcoholism				= UIHelper::CreateStatic( xml, "indicator_alcoholism",		this, false);
	_ind_hangover				= UIHelper::CreateStatic( xml, "indicator_hangover",		this, false);
	_ind_narcotism				= UIHelper::CreateStatic( xml, "indicator_narcotism",		this, false);
	_ind_withdrawal				= UIHelper::CreateStatic( xml, "indicator_withdrawal",		this, false);
	_ind_drugs					= UIHelper::CreateStatic( xml, "indicator_drugs",			this, false);
	_ind_frostbite				= UIHelper::CreateStatic( xml, "indicator_frostbite",		this, false);
	_ind_heating				= UIHelper::CreateStatic( xml, "indicator_heating",			this, false);
	_ind_outfit_broken			= UIHelper::CreateStatic( xml, "indicator_outfit_broken",	this, false);
	_ind_helmet_broken			= UIHelper::CreateStatic( xml, "indicator_helmet_broken",	this, false);
	_ind_helmet_2_broken		= UIHelper::CreateStatic( xml, "indicator_helmet_2_broken",	this, false);
	_ind_filter					= UIHelper::CreateStatic( xml, "indicator_filter",			this, false);
	_ind_battery				= UIHelper::CreateStatic( xml, "indicator_torch_battery",	this, false);

	xml.SetLocalRoot( stored_root );
}

void CUIHudStatesWnd::on_connected()
{
	Load_section();
}

void CUIHudStatesWnd::Load_section()
{
	VERIFY( g_pGameLevel );
	if ( !Level().hud_zones_list )
	{
		Level().create_hud_zones_list();
		VERIFY( Level().hud_zones_list );
	}
	
//	m_actor_radia_factor = pSettings->r_float( "radiation_zone_detector", "actor_radia_factor" );
	Level().hud_zones_list->load( "all_zone_detector", "zone" );

	Load_section_type( ALife::infl_rad,     "radiation_zone_detector" );
	Load_section_type( ALife::infl_fire,    "fire_zone_detector" );
	Load_section_type( ALife::infl_acid,    "acid_zone_detector" );
	Load_section_type( ALife::infl_psi,     "psi_zone_detector" );
	Load_section_type( ALife::infl_electra, "electra_zone_detector" );	//no uistatic
}

void CUIHudStatesWnd::Load_section_type( ALife::EInfluenceType type, LPCSTR section )
{
	/*m_zone_max_power[type] = pSettings->r_float( section, "max_power" );
	if ( m_zone_max_power[type] <= 0.0f )
	{
		m_zone_max_power[type] = 1.0f;
	}*/
	HUD().GetUI()->UIGame()->m_zone_feel_radius[type] = pSettings->r_float( section, "zone_radius" );

	if (HUD().GetUI()->UIGame()->m_zone_feel_radius[type] <= 0.0f )
	{
		HUD().GetUI()->UIGame()->m_zone_feel_radius[type] = 1.0f;
	}

	if (HUD().GetUI()->UIGame()->m_zone_feel_radius_max < HUD().GetUI()->UIGame()->m_zone_feel_radius[type] )
	{
		HUD().GetUI()->UIGame()->m_zone_feel_radius_max = HUD().GetUI()->UIGame()->m_zone_feel_radius[type];
	}

	HUD().GetUI()->UIGame()->m_zone_threshold[type] = pSettings->r_float( section, "threshold" );
}

void CUIHudStatesWnd::Update()
{
	CActor* actor = smart_cast<CActor*>( Level().CurrentViewEntity() );
	if ( !actor )
	{
		return;
	}
	/*if ( Device.dwTimeGlobal - m_last_time > 50 )
	{
		m_last_time = Device.dwTimeGlobal;
	}
	*/
	UpdateHealth( actor );
	UpdateIndicatorIcons( actor );
	UpdateActiveItemInfo( actor );
	UpdateIndicators( actor );

	inherited::Update();
}

void CUIHudStatesWnd::UpdateHealth( CActor* actor )
{

	m_ui_satiety_bar->SetProgressPos(actor->conditions().GetSatiety() * 100.0f);
	m_ui_thirsty_bar->SetProgressPos(actor->conditions().GetThirst() * 100.0f);
	m_ui_tired_bar->SetProgressPos(actor->conditions().GetSleepeness() * 100.0f);
	m_ui_infection_bar->SetProgressPos(actor->conditions().GetInfection() * 100.0f);
	m_ui_health_bar->SetProgressPos( actor->GetfHealth() * 100.0f );
	m_ui_stamina_bar->SetProgressPos( actor->conditions().GetPower()*100.0f );

	CCustomOutfit* outfit = actor->GetOutfit();
	if ( outfit )
	{
		m_static_armor->Show( true );
		m_ui_armor_bar->Show( true );
		m_ui_armor_bar->SetProgressPos( outfit->GetCondition() * 100.0f );
	}
	else
	{
		m_static_armor->Show( false );
		m_ui_armor_bar->Show( false );
	}
	
	if ( actor->conditions().BleedingSpeed() > 0.01f )
	{
		m_bleeding->Show( true );
	}
	else
	{
		m_bleeding->Show( false );
	}
	m_progress_self->SetPos(HUD().GetUI()->UIGame()->m_radia_self);
}

void CUIHudStatesWnd::UpdateActiveItemInfo( CActor* actor )
{
	PIItem item = actor->inventory().ActiveItem();
	if ( item )
	{
		if(m_b_force_update)
		{
			if(item->cast_weapon())
				item->cast_weapon()->ForceUpdateAmmo();
			m_b_force_update		= false;
		}

		item->GetBriefInfo			( m_item_info );

//		UIWeaponBack.SetText		( str_name.c_str() );
		m_fire_mode->SetText		( m_item_info.fire_mode.c_str() );
		SetAmmoIcon					( m_item_info.icon.c_str() );
		
		m_ui_weapon_cur_ammo->Show	( true );
		m_ui_weapon_fmj_ammo->Show	( true );
		m_ui_weapon_ap_ammo->Show	( true );
		m_fire_mode->Show			( true );
		m_ui_grenade->Show			( true );

		m_ui_weapon_cur_ammo->SetText	( m_item_info.cur_ammo.c_str() );
		m_ui_weapon_fmj_ammo->SetText	( m_item_info.fmj_ammo.c_str() );
		m_ui_weapon_ap_ammo->SetText	( m_item_info.ap_ammo.c_str() );
		
		m_ui_grenade->SetText	( m_item_info.grenade.c_str() );

		CWeaponMagazinedWGrenade* wpn = smart_cast<CWeaponMagazinedWGrenade*>(item);
		if(wpn && wpn->m_bGrenadeMode)
		{
			m_ui_weapon_fmj_ammo->SetTextColor(color_rgba(238,155,23,150));
			m_ui_grenade->SetTextColor(color_rgba(238,155,23,255));
		}
		else
		{
			m_ui_weapon_fmj_ammo->SetTextColor(color_rgba(238,155,23,255));
			m_ui_grenade->SetTextColor(color_rgba(238,155,23,150));
		}
	}
	else
	{
		m_ui_weapon_icon->Show		( false );

		m_ui_weapon_cur_ammo->Show	( false );
		m_ui_weapon_fmj_ammo->Show	( false );
		m_ui_weapon_ap_ammo->Show	( false );
		m_fire_mode->Show			( false );
		m_ui_grenade->Show			( false );
	}
}

void CUIHudStatesWnd::SetAmmoIcon(const shared_str& sect_name)
{
	if (!sect_name.size())
	{
		m_ui_weapon_icon->Show(false);
		return;
	}
	m_ui_weapon_icon->Show(true);

	if (psActorFlags3.test(AF_LFO_AMMO_ICONS))
	{
		if (pSettings->line_exist(sect_name, "icons_texture_hud"))
		{
			LPCSTR icons_texture = pSettings->r_string(sect_name, "icons_texture_hud");
			m_ui_weapon_icon->SetShader(InventoryUtilities::GetCustomIconTextureShader(icons_texture));
		}
		else
		{
			m_ui_weapon_icon->SetShader(InventoryUtilities::GetEquipmentIconsShader());
		}
	}
	else
	{
		if (pSettings->line_exist(sect_name, "icons_texture"))
		{
			LPCSTR icons_texture = pSettings->r_string(sect_name, "icons_texture");
			m_ui_weapon_icon->SetShader(InventoryUtilities::GetCustomIconTextureShader(icons_texture));
		}
		else
		{
			m_ui_weapon_icon->SetShader(InventoryUtilities::GetEquipmentIconsShader());
		}
	}

	if (psActorFlags3.test(AF_LFO_AMMO_ICONS))
	{
		//properties used by inventory menu

		float xPos = pSettings->r_float(sect_name, "inv_grid_hud_x") * UI().inv_grid_kx();
		float yPos = pSettings->r_float(sect_name, "inv_grid_hud_y") * UI().inv_grid_kx();
		float gridWidth = pSettings->r_float(sect_name, "inv_grid_hud_width") * UI().inv_grid_kx();
		float gridHeight = pSettings->r_float(sect_name, "inv_grid_hud_height") * UI().inv_grid_kx();

		m_ui_weapon_icon->GetUIStaticItem().SetOriginalRect(xPos, yPos, gridWidth, gridHeight);
		m_ui_weapon_icon->SetStretchTexture(true);

		// all others ammo (1x1, 1x2) will be not scaled (original picture)
		float h = gridHeight * 0.65f;
		float w = gridWidth * 0.65f;
		// now perform only width scale for ammo, which (W)size >2
		if (gridWidth > 2.01f * UI().inv_grid_kx())
		{
			w = UI().inv_grid() * 1.3f;
			h /= 0.8f;
		}
		if (GameConstants::GetUseHQ_Icons())
		{
			h /= 2;
			w /= 2;
		}

		bool is_16x10 = UI().is_widescreen();
		if (gridWidth < 1.01f * UI().inv_grid_kx())
		{
			m_ui_weapon_icon->SetTextureOffset((is_16x10) ? 8.33f : 10.0f, 0.0f);
		}
		else
		{
			if (gridWidth > 2.01f * UI().inv_grid_kx())
				m_ui_weapon_icon->SetTextureOffset(3.0f, -2.0f);
			else
				m_ui_weapon_icon->SetTextureOffset(0.0f, 0.0f);
		}
		m_ui_weapon_icon->SetWidth(w * UI().get_current_kx() * m_ui_weapon_icon_scale);
		m_ui_weapon_icon->SetHeight(h * m_ui_weapon_icon_scale);
	}
	else
	{
		//properties used by inventory menu

		float xPos = pSettings->r_float(sect_name, "inv_grid_x") * UI().inv_grid_kx();
		float yPos = pSettings->r_float(sect_name, "inv_grid_y") * UI().inv_grid_kx();
		float gridWidth = pSettings->r_float(sect_name, "inv_grid_width") * UI().inv_grid_kx();
		float gridHeight = pSettings->r_float(sect_name, "inv_grid_height") * UI().inv_grid_kx();

		m_ui_weapon_icon->GetUIStaticItem().SetOriginalRect(xPos, yPos, gridWidth, gridHeight);
		m_ui_weapon_icon->SetStretchTexture(true);

		// all others ammo (1x1, 1x2) will be not scaled (original picture)
		float h = gridHeight * 0.65f;
		float w = gridWidth * 0.65f;
		// now perform only width scale for ammo, which (W)size >2
		if (gridWidth > 2.01f * UI().inv_grid_kx())
		{
			w = UI().inv_grid_kx() * 1.3f;
			h /= 0.8f;
		}
		if (GameConstants::GetUseHQ_Icons())
		{
			h /= 2;
			w /= 2;
		}

		bool is_16x10 = UI().is_widescreen();
		if (gridWidth < 1.01f * UI().inv_grid_kx())
		{
			m_ui_weapon_icon->SetTextureOffset((is_16x10) ? 8.33f : 10.0f, 0.0f);
		}
		else
		{
			if (gridWidth > 2.01f * UI().inv_grid_kx())
				m_ui_weapon_icon->SetTextureOffset(3.0f, -2.0f);
			else
				m_ui_weapon_icon->SetTextureOffset(0.0f, 0.0f);
		}
		m_ui_weapon_icon->SetWidth(w * UI().get_current_kx() * m_ui_weapon_icon_scale);
		m_ui_weapon_icon->SetHeight(h * m_ui_weapon_icon_scale);
	}
}


// ------------------------------------------------------------------------------------------------

void CUIHudStatesWnd::UpdateIndicators( CActor* actor )
{
	for ( int i = 0; i < it_max ; ++i ) // it_max = ALife::infl_max_count-1
	{
		UpdateIndicatorType( actor, (ALife::EInfluenceType)i );
	}
}

void CUIHudStatesWnd::UpdateIndicatorType( CActor* actor, ALife::EInfluenceType type )
{
	if ( type < ALife::infl_rad || ALife::infl_psi < type )
	{
		VERIFY2( 0, "Failed EIndicatorType for CStatic!" );
		return;
	}

	u32 c_white  = color_rgba( 255, 255, 255, 255 );
	u32 c_green  = color_rgba( 0, 255, 0, 255 );
	u32 c_yellow = color_rgba( 255, 255, 0, 255 );
	u32 c_red    = color_rgba( 255, 0, 0, 255 );

	float           hit_power = HUD().GetUI()->UIGame()->m_zone_cur_power[type];
	ALife::EHitType hit_type = HUD().GetUI()->UIGame()->m_zone_hit_type[type];
#pragma todo("it crashes in mp, please fix it")
	if (!IsGameTypeSingle())
		return;
	
	CCustomOutfit* outfit = actor->GetOutfit();
	CHelmet* helmet = smart_cast<CHelmet*>(actor->inventory().ItemFromSlot(HELMET_SLOT));
	float protect = (outfit) ? outfit->GetDefHitTypeProtection(hit_type) : 0.0f;
	protect += (helmet) ? helmet->GetDefHitTypeProtection(hit_type) : 0.0f;
	protect += actor->GetProtection_ArtefactsOnBelt(hit_type);

	float max_power = actor->conditions().GetZoneMaxPower( hit_type );
	protect = protect / max_power; // = 0..1

	CEntityCondition::BOOSTER_MAP cur_booster_influences = actor->conditions().GetCurBoosterInfluences();
	CEntityCondition::BOOSTER_MAP::const_iterator it;

	if (hit_type == ALife::eHitTypeChemicalBurn)
	{
		it = cur_booster_influences.find(eBoostChemicalBurnProtection);
		if (it != cur_booster_influences.end())
			protect += it->second.fBoostValue;
	}
	else if (hit_type == ALife::eHitTypeRadiation)
	{
		it = cur_booster_influences.find(eBoostRadiationProtection);
		if (it != cur_booster_influences.end())
			protect += it->second.fBoostValue;
	}
	else if (hit_type == ALife::eHitTypeTelepatic)
	{
		it = cur_booster_influences.find(eBoostTelepaticProtection);
		if (it != cur_booster_influences.end())
			protect += it->second.fBoostValue;
	}

	if ( hit_power < EPS )
	{
		m_indik[type]->SetColor( c_white );
		SwitchLA( false, type );
		actor->conditions().SetZoneDanger( 0.0f, type );
		return;
	}
	if ( hit_power < protect )
	{
		m_indik[type]->SetColor( c_green );
		SwitchLA( false, type );
		actor->conditions().SetZoneDanger( 0.0f, type );
		return;
	}
	if ( hit_power - protect < HUD().GetUI()->UIGame()->m_zone_threshold[type] )
	{
		m_indik[type]->SetColor( c_yellow );
		SwitchLA( false, type );
		actor->conditions().SetZoneDanger( 0.0f, type );
		return;
	}
	m_indik[type]->SetColor( c_red );
	SwitchLA( true, type );
	actor->conditions().SetZoneDanger( hit_power - protect, type );
}

void CUIHudStatesWnd::SwitchLA( bool state, ALife::EInfluenceType type )
{
	if ( state == m_cur_state_LA[type] )
	{
		return;
	}

	if ( state )
	{
		m_indik[type]->SetClrLightAnim( m_lanim_name.c_str(), true, false, false, true );
		m_cur_state_LA[type] = true;
//-		Msg( "LA = 1    type = %d", type );
	}
	else
	{
		m_indik[type]->SetClrLightAnim( NULL, false, false, false, false );//off
		m_cur_state_LA[type] = false;
//-		Msg( "__LA = 0    type = %d", type );
	}
}

void CUIHudStatesWnd::UpdateIndicatorIcons(CActor* actor)
{
	if (!_ind_position_set)
		return;

	float pos_shift = 0.0f, pos_fixed = 0.0f;
	Fvector2 pos{};
	int icons_count = 0;

	pos_shift = _ind_is_vertical_attrib
		? _ind_position_set->GetWndPos().y
		: _ind_position_set->GetWndPos().x;

	pos_fixed = _ind_is_vertical_attrib
		? _ind_position_set->GetWndPos().x
		: _ind_position_set->GetWndPos().y;

	auto CreateIndicator = [&](CUIStatic* ind_name, float state, float state_crit, float state_start, bool reverse)
		{
			if (ind_name)
			{
				// чтобы не мозолила глаза в годмоде
				bool godmode = (ind_name == _ind_overweight) ? GodMode() : false;

				// возможно стоит добавить чтение и других слотов на будущее
				if (ind_name == _ind_weapon_broken)
				{
					u32 slot = actor->inventory().GetActiveSlot();
					if (slot == KNIFE_SLOT || slot == PISTOL_SLOT || slot == RIFLE_SLOT)
						if (auto weapon = smart_cast<CWeapon*>(actor->inventory().ItemFromSlot(slot)))
							state = weapon->GetCondition();
				}

				if ((ind_name == _ind_outfit_broken) || (ind_name == _ind_filter) || (ind_name == _ind_helmet_broken))
				{
					if (CCustomOutfit* outfit = smart_cast<CCustomOutfit*>(actor->inventory().ItemFromSlot(OUTFIT_SLOT)))
					{
						if (ind_name == _ind_outfit_broken)
							state = outfit->GetCondition();
						if ((ind_name == _ind_filter) && outfit->m_bUseFilter)
							state = outfit->GetFilterCondition();
					}

					if (CHelmet* helmet = smart_cast<CHelmet*>(actor->inventory().ItemFromSlot(HELMET_SLOT)))
					{
						if (ind_name == _ind_helmet_broken)
							state = helmet->GetCondition();
						if ((ind_name == _ind_filter) && helmet->m_bUseFilter)
							state = helmet->GetFilterCondition();
					}

					if (CHelmet* helmet2 = smart_cast<CHelmet*>(actor->inventory().ItemFromSlot(SECOND_HELMET_SLOT)))
					{
						if (ind_name == _ind_helmet_2_broken)
							state = helmet2->GetCondition();
						if ((ind_name == _ind_filter) && helmet2->m_bUseFilter)
							state = helmet2->GetFilterCondition();
					}
				}

				if (ind_name == _ind_battery)
				{
					if (CTorch* torch = smart_cast<CTorch*>(actor->inventory().ItemFromSlot(TORCH_SLOT)))
						state = torch->GetChargeLevel();

					if (CCustomDetector* artefact_detector = smart_cast<CCustomDetector*>(actor->inventory().ItemFromSlot(DETECTOR_SLOT)))
						state = artefact_detector->GetChargeLevel();

					if (CDetectorAnomaly* anomaly_detector = smart_cast<CDetectorAnomaly*>(actor->inventory().ItemFromSlot(DOSIMETER_SLOT)))
						state = anomaly_detector->GetChargeLevel();
				}

				float dif_state = (state / ((ind_name == _ind_overweight) ? state_crit : 1.0f));
				clamp(dif_state, 0.0f, 1.0f);

				u32 clr_fixed = _ind_clr_only_one_attrib ? _ind_clr_fixed : 255;
				u32 clr_state = (u32)((1 - pow((1 - dif_state), 3)) * clr_fixed);
				u32 clr_reverse_state = (u32)((1 - pow(dif_state, 3)) * clr_fixed);
				u32 clr_to_crit = reverse ? clr_state : clr_reverse_state;
				u32 clr_from_crit = reverse ? clr_reverse_state : clr_state;

				if (reverse ? (state < state_crit) : (state >= state_crit))
					ind_name->SetTextureColor(color_rgba(clr_fixed, 0, 0, 255));
				else
				{
					if (_ind_clr_only_one_attrib)
						ind_name->SetTextureColor(color_rgba(clr_fixed, clr_to_crit, clr_to_crit, 255));
					else
						ind_name->SetTextureColor(color_rgba(clr_from_crit, clr_to_crit, 0, 255));
				}

				ind_name->Show(false);

				if (!godmode && (reverse ? (dif_state < state_start) : (dif_state > state_start)))
				{
					pos.set(ind_name->GetWndPos());
					pos.x = _ind_is_vertical_attrib ? pos_fixed : pos_shift;
					pos.y = _ind_is_vertical_attrib ? pos_shift : pos_fixed;

					if ((icons_count < _ind_max_icons))
					{
						if (_ind_block_icon)
							_ind_block_icon->Show(false);

						ind_name->Show(true);
						ind_name->SetWndPos(pos);
						if (_ind_is_vertical_attrib)
						{
							(_ind_dir_bottom) ? pos_shift += ((ind_name->GetWndSize().y) + _ind_pos_shift_add)
								: pos_shift -= ((ind_name->GetWndSize().y) + _ind_pos_shift_add);
						}
						else
						{
							(_ind_dir_left) ? pos_shift -= ((ind_name->GetWndSize().x) + _ind_pos_shift_add)
								: pos_shift += ((ind_name->GetWndSize().x) + _ind_pos_shift_add);
						}
					}
					else
					{
						if (_ind_block_icon)
						{
							_ind_block_icon->Show(true);
							_ind_block_icon->SetWndPos(pos);
						}
					}
				}

				if (ind_name->IsShown())
					icons_count += 1;
			}
		};


	//				static_name			main_state								crit_state/sec_state	start	reverse
	CreateIndicator(_ind_bleeding,		actor->conditions().BleedingSpeed(),	0.9f,					0.01f,	false);
	CreateIndicator(_ind_radiation,		actor->conditions().GetRadiation(),		0.9f,					0.01f,	false);
	CreateIndicator(_ind_alcohol,		actor->conditions().GetAlcohol(),		0.9f,					0.01f,	false);
	CreateIndicator(_ind_starvation,	actor->conditions().GetSatiety(),		0.15f,					0.8f,	true);
	CreateIndicator(_ind_thirst,		actor->conditions().GetThirst(),		0.15f,					0.5f,	true);
	CreateIndicator(_ind_psyhealth,		actor->conditions().GetPsyHealth(),		0.15f,					0.8f,	true);
	CreateIndicator(_ind_stamina,		actor->conditions().GetPower(),			0.15f,					0.8f,	true);
	CreateIndicator(_ind_health,		actor->conditions().GetHealth(),		0.15f,					0.8f,	true);
	CreateIndicator(_ind_overweight,	actor->inventory().TotalWeight(),		actor->MaxWalkWeight(),	0.7f,	false);
	CreateIndicator(_ind_weapon_broken, 1.0f,									0.1f,					0.7f,	true);

	
	CreateIndicator(_ind_intoxication,	actor->conditions().GetIntoxication(),	0.9f,					0.01f,	false);
	CreateIndicator(_ind_sleepeness,	actor->conditions().GetSleepeness(),	0.9f,					0.5f,	false);
	CreateIndicator(_ind_alcoholism,	actor->conditions().GetAlcoholism(),	3.5f,					0.99f,	false);
	CreateIndicator(_ind_hangover,		actor->conditions().GetHangover(),		2.5f,					0.5f,	false);
	CreateIndicator(_ind_narcotism,		actor->conditions().GetNarcotism(),		9.0f,					0.99f,	false);
	CreateIndicator(_ind_withdrawal,	actor->conditions().GetWithdrawal(),	3.0f,					0.5f,	false);
	CreateIndicator(_ind_drugs,			actor->conditions().GetDrugs(),			0.9f,					0.15f,	false);
	CreateIndicator(_ind_frostbite,		actor->conditions().GetFrostbite(),		0.9f,					0.15f,	false);
	CreateIndicator(_ind_heating,		actor->GetCurrentHeating(),				9.5f,					0.01f,	false);
	CreateIndicator(_ind_outfit_broken,	1.0f,									0.1f,					0.7f,	true);
	CreateIndicator(_ind_helmet_broken,	1.0f,									0.1f,					0.7f,	true);
	CreateIndicator(_ind_helmet_2_broken,1.0f,									0.1f,					0.7f,	true);
	CreateIndicator(_ind_filter,		1.0f,									0.1f,					0.7f,	true);
	CreateIndicator(_ind_battery,		1.0f,									0.1f,					0.7f,	false);
}
