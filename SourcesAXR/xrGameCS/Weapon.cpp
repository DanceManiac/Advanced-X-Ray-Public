#include "stdafx.h"
#include "Weapon.h"
#include "ParticlesObject.h"
#include "HUDManager.h"
#include "entity_alive.h"
#include "inventory_item_impl.h"
#include "inventory.h"
#include "xrserver_objects_alife_items.h"
#include "actor.h"
#include "actoreffector.h"
#include "level.h"
#include "xr_level_controller.h"
#include "game_cl_base.h"
#include "../Include/xrRender/Kinematics.h"
#include "../xrCore/_vector3d_ext.h"
#include "ai_object_location.h"
#include "../xrphysics/mathutils.h"
#include "object_broker.h"
#include "player_hud.h"
#include "gamepersistent.h"
#include "effectorFall.h"
#include "debug_renderer.h"
#include "static_cast_checked.hpp"
#include "clsid_game.h"
#include "weaponBinocularsVision.h"
#include "UIGameCustom.h"
#include "ui/UIWindow.h"
#include "ui/UIXmlInit.h"
#include "ui\UIActorMenu.h"
#include "Torch.h"
#include "ActorNightVision.h"
#include "../xrEngine/x_ray.h"
#include "../xrEngine/LightAnimLibrary.h"
#include "../xrEngine/GameMtlLib.h"
#include "AdvancedXrayGameConstants.h"

constexpr auto WEAPON_REMOVE_TIME = 60000;
constexpr auto ROTATION_TIME = 0.25f;

BOOL	b_toggle_weapon_aim		= FALSE;
BOOL	b_hud_collision			= FALSE;
extern CUIXml*	pWpnScopeXml	= NULL;

ENGINE_API extern float psHUD_FOV_def;
static float last_hud_fov = psHUD_FOV_def;

CWeapon::CWeapon()
{
	SetState				(eHidden);
	SetNextState			(eHidden);
	m_sub_state				= eSubstateReloadBegin;
	m_bTriStateReload		= false;
	SetDefaults				();

	m_Offset.identity		();
	m_StrapOffset.identity	();

	iAmmoCurrent			= -1;
	m_dwAmmoCurrentCalcFrame= 0;

	iAmmoElapsed			= -1;
	iMagazineSize			= -1;
	m_ammoType				= 0;
	m_ammoName				= NULL;

	eHandDependence			= hdNone;

	m_zoom_params.m_fCurrentZoomFactor			= GameConstants::GetOGSE_WpnZoomSystem() ? 1.f : g_fov;
	m_zoom_params.m_fZoomRotationFactor			= 0.f;
	m_zoom_params.m_pVision						= NULL;
	m_zoom_params.m_pNight_vision				= NULL;
	m_zoom_params.m_fSecondVPFovFactor			= 0.0f;
	m_zoom_params.m_f3dZoomFactor				= 0.0f;

	ResetShootingEffect		();
	
	m_pCurrentAmmo					= NULL;


	m_pFlameParticles2		= NULL;
	m_sFlameParticles2		= NULL;


	m_fCurrentCartirdgeDisp = 1.f;

	m_strap_bone0			= 0;
	m_strap_bone1			= 0;
	m_strap_bone0_id = -1;
	m_strap_bone1_id = -1;
	m_StrapOffset.identity	();
	m_strapped_mode			= false;
	m_strapped_mode_rifle = false;
	m_can_be_strapped_rifle = false;
	m_can_be_strapped		= false;
	m_ef_main_weapon_type	= u32(-1);
	m_ef_weapon_type		= u32(-1);
	m_UIScope				= NULL;
	m_set_next_ammoType_on_reload = u32(-1);
	m_crosshair_inertion	= 0.f;
	m_cur_scope				= NULL;
	m_bRememberActorNVisnStatus = false;
	m_freelook_switch_back	= false;
	m_fLR_MovingFactor		= 0.f;
	m_fLR_CameraFactor		= 0.f;
	m_fLR_InertiaFactor		= 0.f;
	m_fUD_InertiaFactor		= 0.f;

	//Mortan: new params
	bUseAltScope		= false;
	bScopeIsHasTexture	= false;
	bNVsecondVPavaible	= false;
	bNVsecondVPstatus	= false;
	m_fZoomStepCount	= 3.0f;
	m_fZoomMinKoeff		= 0.3f;

	bHasBulletsToHide	= false;
	bullet_cnt			= 0;

	m_bUseAimAnmDirDependency = false;
	m_bUseScopeAimMoveAnims = true;
	m_bAltZoomEnabled	= false;
	m_bAltZoomEnabledScope = false;
	m_bAltZoomActive	= false;
}

const shared_str CWeapon::GetScopeName() const
{
	if (bUseAltScope)
	{
		return m_scopes[m_cur_scope];
	}
	else
	{
		return pSettings->r_string(m_scopes[m_cur_scope], "scope_name");
	}
}

void CWeapon::UpdateAltScope()
{
	if (m_eScopeStatus != ALife::eAddonAttachable || !bUseAltScope)
		return;

	shared_str sectionNeedLoad;

	sectionNeedLoad = IsScopeAttached() ? GetNameWithAttachment() : m_section_id;

	if (!pSettings->section_exist(sectionNeedLoad))
		return;

	shared_str vis = pSettings->r_string(sectionNeedLoad, "visual");

	if (vis != cNameVisual())
	{
		cNameVisual_set(vis);
	}

	shared_str new_hud = pSettings->r_string(sectionNeedLoad, "hud");
	if (new_hud != m_hud_sect)
	{
		m_hud_sect = new_hud;
	}
}

bool CWeapon::bChangeNVSecondVPStatus()
{
	if (!bNVsecondVPavaible || !IsZoomed())
		return false;

	bNVsecondVPstatus = !bNVsecondVPstatus;

	return true;
}

shared_str CWeapon::GetNameWithAttachment()
{
	string64 str;
	if (pSettings->line_exist(m_section_id.c_str(), "parent_section"))
	{
		shared_str parent = pSettings->r_string(m_section_id.c_str(), "parent_section");
		xr_sprintf(str, "%s_%s", parent.c_str(), GetScopeName().c_str());
	}
	else
	{
		xr_sprintf(str, "%s_%s", m_section_id.c_str(), GetScopeName().c_str());
	}
	return (shared_str)str;
}

int CWeapon::GetScopeX()
{
	if (bUseAltScope)
	{
		if (m_eScopeStatus != ALife::eAddonPermanent && IsScopeAttached())
			return (pSettings->r_s32(GetNameWithAttachment(), "scope_x") * UI().get_icons_kx());
		else
			return 0;
	}
	else
		return (pSettings->r_s32(m_scopes[m_cur_scope], "scope_x") * UI().get_icons_kx());
}

int CWeapon::GetScopeY()
{
	if (bUseAltScope)
	{
		if (m_eScopeStatus != ALife::eAddonPermanent && IsScopeAttached())
			return (pSettings->r_s32(GetNameWithAttachment(), "scope_y") * UI().get_icons_kx());
		else
			return 0;
	}
	else
		return (pSettings->r_s32(m_scopes[m_cur_scope], "scope_y") * UI().get_icons_kx());
}

CWeapon::~CWeapon		()
{
	xr_delete				(m_UIScope);
	delete_data				( m_scopes );

	laser_light_render.destroy();
	flashlight_render.destroy();
	flashlight_omni.destroy();
	flashlight_glow.destroy();
}

void CWeapon::Hit					(SHit* pHDS)
{
	inherited::Hit(pHDS);
}



void CWeapon::UpdateXForm	()
{
	if (Device.dwFrame == m_dwXF_Frame)
		return;

	m_dwXF_Frame				= Device.dwFrame;

	if (!GetHUDmode())
		UpdateAddonsTransform(false);

	if (!H_Parent())
		return;

	// Get access to entity and its visual
	CEntityAlive*			E = smart_cast<CEntityAlive*>(H_Parent());
	
	if (!E) {
		if (!IsGameTypeSingle())
		{
			UpdatePosition	(H_Parent()->XFORM());
		}
		return;
	}

	const CInventoryOwner	*parent = smart_cast<const CInventoryOwner*>(E);
	if (!parent || (parent && parent->use_simplified_visual()))
		return;

	if (!m_can_be_strapped_rifle) {
		if (parent->attached(this))
			return;
	}

	IKinematics*			V = smart_cast<IKinematics*>	(E->Visual());
	VERIFY					(V);

	// Get matrices
	int						boneL = -1, boneR = -1, boneR2 = -1;

	// this ugly case is possible in case of a CustomMonster, not a Stalker, nor an Actor
	if ((m_strap_bone0_id == -1 || m_strap_bone1_id == -1) && m_can_be_strapped_rifle)
	{
		m_strap_bone0_id = V->LL_BoneID(m_strap_bone0);
		m_strap_bone1_id = V->LL_BoneID(m_strap_bone1);
	}

	if (parent->inventory().GetActiveSlot() != RIFLE_SLOT && m_can_be_strapped_rifle /* && parent->inventory().InSlot(this)*/)
	{
		boneR = m_strap_bone0_id;
		boneR2 = m_strap_bone1_id;
		boneL = boneR;

		if (!m_strapped_mode_rifle)
			m_strapped_mode_rifle = true;
	}
	else {
		E->g_WeaponBones(boneL, boneR, boneR2);

		if (m_strapped_mode_rifle)
			m_strapped_mode_rifle = false;
	}

	if (boneR == -1)		return;

#ifdef DEBUG
	static std::unordered_set<std::string> loggedVisuals;
	std::string visualStr = E->cNameVisual().c_str();
#endif

	if (m_strap_bone0_id == BI_NONE)
	{
#ifdef DEBUG
		if (loggedVisuals.find(visualStr) == loggedVisuals.end())
		{
			loggedVisuals.insert(visualStr);
			Msg("! Bone [%s] not found in entity [%s](%s) with visual [%s]!", m_strap_bone0, E->cNameSect().c_str(), E->Name(), E->cNameVisual().c_str());
		}
#endif
		return;
	}
	else if (m_strap_bone1_id == BI_NONE)
	{
#ifdef DEBUG
		if (loggedVisuals.find(visualStr) == loggedVisuals.end())
		{
			loggedVisuals.insert(visualStr);
			Msg("! Bone [%s] not found in entity [%s]([%s]) with visual [%s]!", m_strap_bone1, E->cNameSect().c_str(), E->Name(), E->cNameVisual().c_str());
		}
#endif

		return;
	}

	if ((HandDependence() == hd1Hand) || (GetState() == eReload) || (!E->g_Alive()))
		boneL				= boneR2;

	V->CalculateBones_Invalidate();
	V->CalculateBones(true);
	Fmatrix& mL				= V->LL_GetTransform(u16(boneL));
	Fmatrix& mR				= V->LL_GetTransform(u16(boneR));
	// Calculate
	Fmatrix					mRes;
	Fvector					R,D,N;
	D.sub					(mL.c,mR.c);	

	if(fis_zero(D.magnitude())) {
		mRes.set			(E->XFORM());
		mRes.c.set			(mR.c);
	}
	else {		
		D.normalize			();
		R.crossproduct		(mR.j,D);

		N.crossproduct		(D,R);			
		N.normalize			();

		mRes.set			(R,N,D,mR.c);
		mRes.mulA_43		(E->XFORM());
	}

	UpdatePosition			(mRes);
}

void CWeapon::UpdateFireDependencies_internal()
{
	if (Device.dwFrame != m_dwFP_Frame)
	{
		m_dwFP_Frame = Device.dwFrame;

		UpdateXForm			();

		if ( GetHUDmode() )
		{
			HudItemData()->setup_firedeps		(m_current_firedeps);
			VERIFY(_valid(m_current_firedeps.m_FireParticlesXForm));
		} else 
		{
			// 3rd person or no parent
			Fmatrix& parent			= XFORM();
			Fvector& fp				= vLoadedFirePoint;
			Fvector& fp2			= vLoadedFirePoint2;
			Fvector& sp				= vLoadedShellPoint;

			parent.transform_tiny	(m_current_firedeps.vLastFP,fp);
			parent.transform_tiny	(m_current_firedeps.vLastFP2,fp2);
			parent.transform_tiny	(m_current_firedeps.vLastSP,sp);
			
			m_current_firedeps.vLastFD.set	(0.f,0.f,1.f);
			parent.transform_dir	(m_current_firedeps.vLastFD);

			m_current_firedeps.m_FireParticlesXForm.set(parent);
			VERIFY(_valid(m_current_firedeps.m_FireParticlesXForm));
		}
	}
}

void CWeapon::ForceUpdateFireParticles()
{
	if ( !GetHUDmode() )
	{//update particlesXFORM real bullet direction

		if (!H_Parent())		return;

		Fvector					p, d; 
		smart_cast<CEntity*>(H_Parent())->g_fireParams	(this, p,d);

		Fmatrix						_pxf;
		_pxf.k						= d;
		_pxf.i.crossproduct			(Fvector().set(0.0f,1.0f,0.0f),	_pxf.k);
		_pxf.j.crossproduct			(_pxf.k,		_pxf.i);
		_pxf.c						= XFORM().c;
		
		m_current_firedeps.m_FireParticlesXForm.set	(_pxf);
	}
}

LPCSTR wpn_scope_def_bone = "wpn_scope";
LPCSTR wpn_silencer_def_bone = "wpn_silencer";
LPCSTR wpn_launcher_def_bone = "wpn_launcher";
LPCSTR wpn_laser_def_bone = "wpn_laser";
LPCSTR wpn_torch_def_bone = "wpn_torch";

void CWeapon::Load		(LPCSTR section)
{
	inherited::Load					(section);
	CShootingObject::Load			(section);

	
	if(pSettings->line_exist(section, "flame_particles_2"))
		m_sFlameParticles2 = pSettings->r_string(section, "flame_particles_2");

	// load ammo classes
	m_ammoTypes.clear	(); 
	LPCSTR				S = pSettings->r_string(section,"ammo_class");
	if (S && S[0]) 
	{
		string128		_ammoItem;
		int				count		= _GetItemCount	(S);
		for (int it=0; it<count; ++it)	
		{
			_GetItem				(S,it,_ammoItem);
			m_ammoTypes.push_back	(_ammoItem);
		}
		m_ammoName = pSettings->r_string(*m_ammoTypes[0],"inv_name_short");
	}
	else
		m_ammoName = 0;

	iAmmoElapsed		= pSettings->r_s32		(section,"ammo_elapsed"		);
	iMagazineSize		= pSettings->r_s32		(section,"ammo_mag_size"	);
	
	////////////////////////////////////////////////////
	// дисперсия стрельбы

	//подбрасывание камеры во время отдачи
	u8 rm = READ_IF_EXISTS( pSettings, r_u8, section, "cam_return", 1 );
	cam_recoil.ReturnMode = (rm == 1);
	
	rm = READ_IF_EXISTS( pSettings, r_u8, section, "cam_return_stop", 0 );
	cam_recoil.StopReturn = (rm == 1);

	float temp_f = 0.0f;
	temp_f					= pSettings->r_float( section,"cam_relax_speed" );
	cam_recoil.RelaxSpeed	= _abs( deg2rad( temp_f ) );
	VERIFY( !fis_zero(cam_recoil.RelaxSpeed) );
	if ( fis_zero(cam_recoil.RelaxSpeed) )
	{
		cam_recoil.RelaxSpeed = EPS_L;
	}

	cam_recoil.RelaxSpeed_AI = cam_recoil.RelaxSpeed;
	if ( pSettings->line_exist( section, "cam_relax_speed_ai" ) )
	{
		temp_f						= pSettings->r_float( section, "cam_relax_speed_ai" );
		cam_recoil.RelaxSpeed_AI	= _abs( deg2rad( temp_f ) );
		VERIFY( !fis_zero(cam_recoil.RelaxSpeed_AI) );
		if ( fis_zero(cam_recoil.RelaxSpeed_AI) )
		{
			cam_recoil.RelaxSpeed_AI = EPS_L;
		}
	}
	temp_f						= pSettings->r_float( section, "cam_max_angle" );
	cam_recoil.MaxAngleVert		= _abs( deg2rad( temp_f ) );
	VERIFY( !fis_zero(cam_recoil.MaxAngleVert) );
	if ( fis_zero(cam_recoil.MaxAngleVert) )
	{
		cam_recoil.MaxAngleVert = EPS;
	}
	
	temp_f						= pSettings->r_float( section, "cam_max_angle_horz" );
	cam_recoil.MaxAngleHorz		= _abs( deg2rad( temp_f ) );
	VERIFY( !fis_zero(cam_recoil.MaxAngleHorz) );
	if ( fis_zero(cam_recoil.MaxAngleHorz) )
	{
		cam_recoil.MaxAngleHorz = EPS;
	}
	
	temp_f						= pSettings->r_float( section, "cam_step_angle_horz" );
	cam_recoil.StepAngleHorz	= deg2rad( temp_f );
	
	cam_recoil.DispersionFrac	= _abs( READ_IF_EXISTS( pSettings, r_float, section, "cam_dispersion_frac", 0.7f ) );

	//подбрасывание камеры во время отдачи в режиме zoom ==> ironsight or scope
	//zoom_cam_recoil.Clone( cam_recoil ); ==== нельзя !!!!!!!!!!
	zoom_cam_recoil.RelaxSpeed		= cam_recoil.RelaxSpeed;
	zoom_cam_recoil.RelaxSpeed_AI	= cam_recoil.RelaxSpeed_AI;
	zoom_cam_recoil.DispersionFrac	= cam_recoil.DispersionFrac;
	zoom_cam_recoil.MaxAngleVert	= cam_recoil.MaxAngleVert;
	zoom_cam_recoil.MaxAngleHorz	= cam_recoil.MaxAngleHorz;
	zoom_cam_recoil.StepAngleHorz	= cam_recoil.StepAngleHorz;

	zoom_cam_recoil.ReturnMode		= cam_recoil.ReturnMode;
	zoom_cam_recoil.StopReturn		= cam_recoil.StopReturn;

	
	if ( pSettings->line_exist( section, "zoom_cam_relax_speed" ) )
	{
		zoom_cam_recoil.RelaxSpeed		= _abs( deg2rad( pSettings->r_float( section, "zoom_cam_relax_speed" ) ) );
		VERIFY( !fis_zero(zoom_cam_recoil.RelaxSpeed) );
		if ( fis_zero(zoom_cam_recoil.RelaxSpeed) )
		{
			zoom_cam_recoil.RelaxSpeed = EPS_L;
		}
	}
	if ( pSettings->line_exist( section, "zoom_cam_relax_speed_ai" ) )
	{
		zoom_cam_recoil.RelaxSpeed_AI	= _abs( deg2rad( pSettings->r_float( section,"zoom_cam_relax_speed_ai" ) ) ); 
		VERIFY( !fis_zero(zoom_cam_recoil.RelaxSpeed_AI) );
		if ( fis_zero(zoom_cam_recoil.RelaxSpeed_AI) )
		{
			zoom_cam_recoil.RelaxSpeed_AI = EPS_L;
		}
	}
	if ( pSettings->line_exist( section, "zoom_cam_max_angle" ) )
	{
		zoom_cam_recoil.MaxAngleVert	= _abs( deg2rad( pSettings->r_float( section, "zoom_cam_max_angle" ) ) );
		VERIFY( !fis_zero(zoom_cam_recoil.MaxAngleVert) );
		if ( fis_zero(zoom_cam_recoil.MaxAngleVert) )
		{
			zoom_cam_recoil.MaxAngleVert = EPS;
		}
	}
	if ( pSettings->line_exist( section, "zoom_cam_max_angle_horz" ) )
	{
		zoom_cam_recoil.MaxAngleHorz	= _abs( deg2rad( pSettings->r_float( section, "zoom_cam_max_angle_horz" ) ) ); 
		VERIFY( !fis_zero(zoom_cam_recoil.MaxAngleHorz) );
		if ( fis_zero(zoom_cam_recoil.MaxAngleHorz) )
		{
			zoom_cam_recoil.MaxAngleHorz = EPS;
		}
	}
	if ( pSettings->line_exist( section, "zoom_cam_step_angle_horz" ) )	{
		zoom_cam_recoil.StepAngleHorz	= deg2rad( pSettings->r_float( section, "zoom_cam_step_angle_horz" ) ); 
	}
	if ( pSettings->line_exist( section, "zoom_cam_dispersion_frac" ) )	{
		zoom_cam_recoil.DispersionFrac	= _abs( pSettings->r_float( section, "zoom_cam_dispersion_frac" ) );
	}

	m_pdm.m_fPDM_disp_base			= pSettings->r_float( section, "PDM_disp_base"			);
	m_pdm.m_fPDM_disp_vel_factor	= pSettings->r_float( section, "PDM_disp_vel_factor"	);
	m_pdm.m_fPDM_disp_accel_factor	= pSettings->r_float( section, "PDM_disp_accel_factor"	);
	m_pdm.m_fPDM_disp_crouch		= pSettings->r_float( section, "PDM_disp_crouch"		);
	m_pdm.m_fPDM_disp_crouch_no_acc	= pSettings->r_float( section, "PDM_disp_crouch_no_acc" );
	m_crosshair_inertion			= READ_IF_EXISTS(pSettings, r_float, section, "crosshair_inertion",	5.91f);
	m_first_bullet_controller.load	(section);

	fireDispersionConditionFactor = pSettings->r_float(section,"fire_dispersion_condition_factor"); 

	// Настройки стрейфа (боковая ходьба)
	const Fvector vZero = { 0.f, 0.f, 0.f };
	Fvector vDefStrafeValue;
	vDefStrafeValue.set(vZero);

	//--> Смещение в стрейфе
	m_strafe_offset[0][0] = READ_IF_EXISTS(pSettings, r_fvector3, section, "strafe_hud_offset_pos", vDefStrafeValue);
	m_strafe_offset[1][0] = READ_IF_EXISTS(pSettings, r_fvector3, section, "strafe_hud_offset_rot", vDefStrafeValue);

	//--> Поворот в стрейфе
	m_strafe_offset[0][1] = READ_IF_EXISTS(pSettings, r_fvector3, section, "strafe_aim_hud_offset_pos", vDefStrafeValue);
	m_strafe_offset[1][1] = READ_IF_EXISTS(pSettings, r_fvector3, section, "strafe_aim_hud_offset_rot", vDefStrafeValue);

	// Параметры стрейфа
	bool  bStrafeEnabled = READ_IF_EXISTS(pSettings, r_bool, section, "strafe_enabled", false);
	bool  bStrafeEnabled_aim = READ_IF_EXISTS(pSettings, r_bool, section, "strafe_aim_enabled", false);
	float fFullStrafeTime = READ_IF_EXISTS(pSettings, r_float, section, "strafe_transition_time", 0.01f);
	float fFullStrafeTime_aim = READ_IF_EXISTS(pSettings, r_float, section, "strafe_aim_transition_time", 0.01f);
	float fStrafeCamLFactor = READ_IF_EXISTS(pSettings, r_float, section, "strafe_cam_limit_factor", 0.5f);
	float fStrafeCamLFactor_aim = READ_IF_EXISTS(pSettings, r_float, section, "strafe_cam_limit_aim_factor", 1.0f);
	float fStrafeMinAngle = READ_IF_EXISTS(pSettings, r_float, section, "strafe_cam_min_angle", 0.0f);
	float fStrafeMinAngle_aim = READ_IF_EXISTS(pSettings, r_float, section, "strafe_cam_aim_min_angle", 7.0f);

	//--> (Data 1)
	m_strafe_offset[2][0].set((bStrafeEnabled ? 1.0f : 0.0f), fFullStrafeTime, NULL);         // normal
	m_strafe_offset[2][1].set((bStrafeEnabled_aim ? 1.0f : 0.0f), fFullStrafeTime_aim, NULL); // aim-GL

	//--> (Data 2)
	m_strafe_offset[3][0].set(fStrafeCamLFactor, fStrafeMinAngle, NULL); // normal
	m_strafe_offset[3][1].set(fStrafeCamLFactor_aim, fStrafeMinAngle_aim, NULL); // aim-GL
	////////////////////////////////////////////

	////////////////////////////////////////////
	m_lookout_offset[0][0] = READ_IF_EXISTS(pSettings, r_fvector3, section, "lookout_hud_offset_pos", vDefStrafeValue);
	m_lookout_offset[1][0] = READ_IF_EXISTS(pSettings, r_fvector3, section, "lookout_hud_offset_rot", vDefStrafeValue);

	m_lookout_offset[0][1] = READ_IF_EXISTS(pSettings, r_fvector3, section, "lookout_aim_hud_offset_pos", vDefStrafeValue);
	m_lookout_offset[1][1] = READ_IF_EXISTS(pSettings, r_fvector3, section, "lookout_aim_hud_offset_rot", vDefStrafeValue);

	m_lookout_offset[2][0].set(READ_IF_EXISTS(pSettings, r_bool, section, "lookout_enabled", false), READ_IF_EXISTS(pSettings, r_float, section, "lookout_transition_time", 0.7f), 0.f); // normal
	m_lookout_offset[2][1].set(READ_IF_EXISTS(pSettings, r_bool, section, "lookout_aim_enabled", false), READ_IF_EXISTS(pSettings, r_float, section, "lookout_aim_transition_time", 0.7f), 0.f); // aim-GL
	////////////////////////////////////////////

	////////////////////////////////////////////
	m_jump_offset[0][0] = READ_IF_EXISTS(pSettings, r_fvector3, section, "jump_hud_offset_pos", vDefStrafeValue);
	m_jump_offset[1][0] = READ_IF_EXISTS(pSettings, r_fvector3, section, "jump_hud_offset_rot", vDefStrafeValue);

	m_jump_offset[0][1] = READ_IF_EXISTS(pSettings, r_fvector3, section, "jump_aim_hud_offset_pos", vDefStrafeValue);
	m_jump_offset[1][1] = READ_IF_EXISTS(pSettings, r_fvector3, section, "jump_aim_hud_offset_rot", vDefStrafeValue);

	m_jump_offset[2][0].set(READ_IF_EXISTS(pSettings, r_bool, section, "jump_enabled", false), READ_IF_EXISTS(pSettings, r_float, section, "jump_transition_time", 0.7f), 0.f); // normal
	m_jump_offset[2][1].set(READ_IF_EXISTS(pSettings, r_bool, section, "jump_aim_enabled", false), READ_IF_EXISTS(pSettings, r_float, section, "jump_aim_transition_time", 0.7f), 0.f); // aim-GL
	////////////////////////////////////////////

	////////////////////////////////////////////
	m_fall_offset[0][0] = READ_IF_EXISTS(pSettings, r_fvector3, section, "fall_hud_offset_pos", vDefStrafeValue);
	m_fall_offset[1][0] = READ_IF_EXISTS(pSettings, r_fvector3, section, "fall_hud_offset_rot", vDefStrafeValue);

	m_fall_offset[0][1] = READ_IF_EXISTS(pSettings, r_fvector3, section, "fall_aim_hud_offset_pos", vDefStrafeValue);
	m_fall_offset[1][1] = READ_IF_EXISTS(pSettings, r_fvector3, section, "fall_aim_hud_offset_rot", vDefStrafeValue);

	m_fall_offset[2][0].set(READ_IF_EXISTS(pSettings, r_bool, section, "fall_enabled", false), READ_IF_EXISTS(pSettings, r_float, section, "fall_transition_time", 0.7f), 0.f); // normal
	m_fall_offset[2][1].set(READ_IF_EXISTS(pSettings, r_bool, section, "fall_aim_enabled", false), READ_IF_EXISTS(pSettings, r_float, section, "fall_aim_transition_time", 0.7f), 0.f); // aim-GL
	////////////////////////////////////////////

	////////////////////////////////////////////
	m_walk_effect[0].set(READ_IF_EXISTS(pSettings, r_fvector3, section, "walk_effect_position", vDefStrafeValue));
	m_walk_effect[1].set(READ_IF_EXISTS(pSettings, r_fvector3, section, "walk_effect_orientation", vDefStrafeValue));
	m_fWalkMaxTime				= READ_IF_EXISTS(pSettings, r_float, section, "walk_effect_time", 0.3f);
	m_fWalkEffectRestoreFactor	= READ_IF_EXISTS(pSettings, r_float, section, "walk_effect_restore_factor", 1.8f);
	////////////////////////////////////////////




	// modified by Peacemaker [17.10.08]
	misfireProbability				= READ_IF_EXISTS(pSettings, r_float, section, "misfire_probability",	0.0f);
	misfireConditionK				= READ_IF_EXISTS(pSettings, r_float, section, "misfire_condition_k",	1.0f);
	misfireStartCondition			= READ_IF_EXISTS(pSettings, r_float, section, "misfire_start_condition", 0.95f);
	misfireEndCondition				= READ_IF_EXISTS(pSettings, r_float, section, "misfire_end_condition", 0.f);
	misfireStartProbability			= READ_IF_EXISTS(pSettings, r_float, section, "misfire_start_prob", misfireProbability);
	misfireEndProbability			= READ_IF_EXISTS(pSettings, r_float, section, "misfire_end_prob", (misfireProbability + misfireConditionK) * 0.25f);
	conditionDecreasePerShot		= pSettings->r_float(section,"condition_shot_dec");
	conditionDecreasePerQueueShot	= READ_IF_EXISTS(pSettings, r_float, section, "condition_queue_shot_dec", conditionDecreasePerShot); 
	conditionDecreasePerShotOnHit	= READ_IF_EXISTS(pSettings, r_float, section, "condition_shot_dec_on_hit", 0.f);
		
	vLoadedFirePoint	= pSettings->r_fvector3		(section,"fire_point"		);
	
	if(pSettings->line_exist(section,"fire_point2")) 
		vLoadedFirePoint2= pSettings->r_fvector3	(section,"fire_point2");
	else 
		vLoadedFirePoint2= vLoadedFirePoint;

	// hands
	eHandDependence		= EHandDependence(pSettings->r_s32(section,"hand_dependence"));
	m_bIsSingleHanded	= true;
	if (pSettings->line_exist(section, "single_handed"))
		m_bIsSingleHanded	= !!pSettings->r_bool(section, "single_handed");
	// 
	m_fMinRadius		= pSettings->r_float		(section,"min_radius");
	m_fMaxRadius		= pSettings->r_float		(section,"max_radius");


	// информация о возможных апгрейдах и их визуализации в инвентаре
	m_eScopeStatus			 = (ALife::EWeaponAddonStatus)pSettings->r_s32(section,"scope_status");
	m_eSilencerStatus		 = (ALife::EWeaponAddonStatus)pSettings->r_s32(section,"silencer_status");
	m_eGrenadeLauncherStatus = (ALife::EWeaponAddonStatus)pSettings->r_s32(section,"grenade_launcher_status");
	m_eLaserDesignatorStatus = (ALife::EWeaponAddonStatus)READ_IF_EXISTS(pSettings, r_s32, section, "laser_designator_status", 0);
	m_eTacticalTorchStatus	 = (ALife::EWeaponAddonStatus)READ_IF_EXISTS(pSettings, r_s32, section, "tactical_torch_status", 0);

	m_zoom_params.m_bZoomEnabled		= !!pSettings->r_bool(section,"zoom_enabled");
	m_zoom_params.m_fZoomRotateTime		= pSettings->r_float(section,"zoom_rotate_time");

	bUseAltScope = !!bLoadAltScopesParams(section);

	if (!bUseAltScope)
		LoadOriginalScopesParams(section);

	LoadSilencerParams(section);
	LoadGrenadeLauncherParams(section);
	LoadLaserDesignatorParams(section);
	LoadTacticalTorchParams(section);

	UpdateAltScope();
	InitAddons();
	if(pSettings->line_exist(section,"weapon_remove_time"))
		m_dwWeaponRemoveTime = pSettings->r_u32(section,"weapon_remove_time");
	else
		m_dwWeaponRemoveTime = WEAPON_REMOVE_TIME;

	if(pSettings->line_exist(section,"auto_spawn_ammo"))
		m_bAutoSpawnAmmo = pSettings->r_bool(section,"auto_spawn_ammo");
	else
		m_bAutoSpawnAmmo = TRUE;

	m_zoom_params.m_bHideCrosshairInZoom		= true;

	if(pSettings->line_exist(m_hud_sect, "zoom_hide_crosshair"))
		m_zoom_params.m_bHideCrosshairInZoom = !!pSettings->r_bool(m_hud_sect, "zoom_hide_crosshair");	

	Fvector			def_dof;
	def_dof.set		(-1,-1,-1);
//	m_zoom_params.m_ZoomDof		= READ_IF_EXISTS(pSettings, r_fvector3, section, "zoom_dof", Fvector().set(-1,-1,-1));
//	m_zoom_params.m_bZoomDofEnabled	= !def_dof.similar(m_zoom_params.m_ZoomDof);

	m_zoom_params.m_ReloadDof = READ_IF_EXISTS(pSettings, r_fvector4, section, "reload_dof", Fvector4().set(-1, -1, -1, -1));

	m_zoom_params.m_ReloadEmptyDof = READ_IF_EXISTS(pSettings, r_fvector4, section, "reload_empty_dof", Fvector4().set(-1, -1, -1, -1));



	m_bHasTracers			= !!READ_IF_EXISTS(pSettings, r_bool, section, "tracers", true);
	m_u8TracerColorID		= READ_IF_EXISTS(pSettings, r_u8, section, "tracers_color_ID", u8(-1));

	string256						temp;
	for (int i=egdNovice; i<egdCount; ++i) 
	{
		strconcat					(sizeof(temp),temp,"hit_probability_",get_token_name(difficulty_type_token,i));
		m_hit_probability[i]		= READ_IF_EXISTS(pSettings,r_float,section,temp,1.f);
	}

	if (pSettings->line_exist(section, "silencer_bone"))
		m_sWpn_silencer_bone = pSettings->r_string(section, "silencer_bone");
	else
		m_sWpn_silencer_bone = wpn_silencer_def_bone;

	if (pSettings->line_exist(section, "launcher_bone"))
		m_sWpn_launcher_bone = pSettings->r_string(section, "launcher_bone");
	else
		m_sWpn_launcher_bone = wpn_launcher_def_bone;

	//Кости самих аддонов
	m_sWpn_laser_bone = READ_IF_EXISTS(pSettings, r_string, section, "laser_attach_bone", wpn_laser_def_bone);
	m_sWpn_flashlight_bone = READ_IF_EXISTS(pSettings, r_string, section, "torch_attach_bone", wpn_torch_def_bone);

	//Кости для эффектов
	m_sWpn_laser_ray_bone = READ_IF_EXISTS(pSettings, r_string, section, "laser_ray_bones", "");
	m_sWpn_flashlight_cone_bone = READ_IF_EXISTS(pSettings, r_string, section, "torch_cone_bones", "");
	m_sHud_wpn_laser_ray_bone = READ_IF_EXISTS(pSettings, r_string, m_hud_sect, "laser_ray_bones", m_sWpn_laser_ray_bone);
	m_sHud_wpn_flashlight_cone_bone = READ_IF_EXISTS(pSettings, r_string, m_hud_sect, "torch_cone_bones", m_sWpn_flashlight_cone_bone);

	auto LoadBoneNames = [](pcstr section, pcstr line, RStringVec& list)
	{
		list.clear();
		if (pSettings->line_exist(section, line))
		{
			pcstr lineStr = pSettings->r_string(section, line);
			for (int j = 0, cnt = _GetItemCount(lineStr); j < cnt; ++j)
			{
				string128 bone_name;
				_GetItem(lineStr, j, bone_name);
				list.push_back(bone_name);
			}
			return true;
		}
		return false;
	};

	// Default shown bones
	LoadBoneNames(section, "def_show_bones", m_defShownBones);

	// Default hidden bones
	LoadBoneNames(section, "def_hide_bones", m_defHiddenBones);

	if (pSettings->line_exist(section, "scopes_sect"))
	{
		pcstr ScopeSect = pSettings->r_string(section, "scopes_sect");

		for (int i = 0; i < _GetItemCount(ScopeSect); i++)
		{
			string128 scope;
			_GetItem(ScopeSect, i, scope);

			if (pSettings->line_exist(scope, "bones"))
			{
				shared_str bone = pSettings->r_string(scope, "bones");
				m_all_scope_bones.push_back(bone);
			}
		}
	}

	if (!laser_light_render && m_eLaserDesignatorStatus)
	{
		has_laser = true;

		laserdot_attach_bone = READ_IF_EXISTS(pSettings, r_string, section, "laserdot_attach_bone", m_sWpn_laser_bone);
		laserdot_attach_offset = Fvector{ READ_IF_EXISTS(pSettings, r_float, section, "laserdot_attach_offset_x", 0.0f), READ_IF_EXISTS(pSettings, r_float, section, "laserdot_attach_offset_y", 0.0f), READ_IF_EXISTS(pSettings, r_float, section, "laserdot_attach_offset_z", 0.0f) };
		laserdot_world_attach_offset = Fvector{ READ_IF_EXISTS(pSettings, r_float, section, "laserdot_world_attach_offset_x", 0.0f), READ_IF_EXISTS(pSettings, r_float, section, "laserdot_world_attach_offset_y", 0.0f), READ_IF_EXISTS(pSettings, r_float, section, "laserdot_world_attach_offset_z", 0.0f) };

		const bool b_r2 = psDeviceFlags.test(rsR2) || psDeviceFlags.test(rsR4);

		const char* m_light_section = pSettings->r_string(m_sLaserName, "laser_light_section");

		laser_lanim = LALib.FindItem(READ_IF_EXISTS(pSettings, r_string, m_light_section, "color_animator", ""));

		laser_light_render = ::Render->light_create();
		laser_light_render->set_type(IRender_Light::SPOT);
		laser_light_render->set_shadow(true);
		laser_light_render->set_moveable(true);

		const Fcolor clr = READ_IF_EXISTS(pSettings, r_fcolor, m_light_section, b_r2 ? "color_r2" : "color", (Fcolor{ 1.0f, 0.0f, 0.0f, 1.0f }));
		laser_fBrightness = clr.intensity();
		laser_light_render->set_color(clr);
		const float range = READ_IF_EXISTS(pSettings, r_float, m_light_section, b_r2 ? "range_r2" : "range", 100.f);
		laser_light_render->set_range(range);
		laser_light_render->set_cone(deg2rad(READ_IF_EXISTS(pSettings, r_float, m_light_section, "spot_angle", 1.f)));
		laser_light_render->set_texture(READ_IF_EXISTS(pSettings, r_string, m_light_section, "spot_texture", nullptr));
	}

	if (!flashlight_render && m_eTacticalTorchStatus)
	{
		has_flashlight = true;

		flashlight_attach_bone = READ_IF_EXISTS(pSettings, r_string, section, "torch_light_bone", m_sWpn_flashlight_bone);
		flashlight_attach_offset = Fvector{ READ_IF_EXISTS(pSettings, r_float, section, "torch_attach_offset_x", 0.0f), READ_IF_EXISTS(pSettings, r_float, section, "torch_attach_offset_y", 0.0f), READ_IF_EXISTS(pSettings, r_float, section, "torch_attach_offset_z", 0.0f) };
		flashlight_omni_attach_offset = Fvector{ READ_IF_EXISTS(pSettings, r_float, section, "torch_omni_attach_offset_x", 0.0f), READ_IF_EXISTS(pSettings, r_float, section, "torch_omni_attach_offset_y", 0.0f), READ_IF_EXISTS(pSettings, r_float, section, "torch_omni_attach_offset_z", 0.0f) };
		flashlight_world_attach_offset = Fvector{ READ_IF_EXISTS(pSettings, r_float, section, "torch_world_attach_offset_x", 0.0f), READ_IF_EXISTS(pSettings, r_float, section, "torch_world_attach_offset_y", 0.0f), READ_IF_EXISTS(pSettings, r_float, section, "torch_world_attach_offset_z", 0.0f) };
		flashlight_omni_world_attach_offset = Fvector{ READ_IF_EXISTS(pSettings, r_float, section, "torch_omni_world_attach_offset_x", 0.0f), READ_IF_EXISTS(pSettings, r_float, section, "torch_omni_world_attach_offset_y", 0.0f), READ_IF_EXISTS(pSettings, r_float, section, "torch_omni_world_attach_offset_z", 0.0f) };

		const bool b_r2 = psDeviceFlags.test(rsR2) || psDeviceFlags.test(rsR4);

		const char* m_light_section = pSettings->r_string(m_sTacticalTorchName, "flashlight_section");

		flashlight_lanim = LALib.FindItem(READ_IF_EXISTS(pSettings, r_string, m_light_section, "color_animator", ""));

		flashlight_render = ::Render->light_create();
		flashlight_render->set_type(IRender_Light::SPOT);
		flashlight_render->set_shadow(true);
		flashlight_render->set_moveable(true);

		const Fcolor clr = READ_IF_EXISTS(pSettings, r_fcolor, m_light_section, b_r2 ? "color_r2" : "color", (Fcolor{ 0.6f, 0.55f, 0.55f, 1.0f }));
		flashlight_fBrightness = clr.intensity();
		flashlight_render->set_color(clr);
		const float range = READ_IF_EXISTS(pSettings, r_float, m_light_section, b_r2 ? "range_r2" : "range", 50.f);
		flashlight_render->set_range(range);
		flashlight_render->set_cone(deg2rad(READ_IF_EXISTS(pSettings, r_float, m_light_section, "spot_angle", 60.f)));
		flashlight_render->set_texture(READ_IF_EXISTS(pSettings, r_string, m_light_section, "spot_texture", nullptr));

		flashlight_omni = ::Render->light_create();
		flashlight_omni->set_type((IRender_Light::LT)(READ_IF_EXISTS(pSettings, r_u8, m_light_section, "omni_type", 2))); //KRodin: вообще omni это обычно поинт, но поинт светит во все стороны от себя, поэтому тут спот используется по умолчанию.
		flashlight_omni->set_shadow(false);
		flashlight_omni->set_moveable(true);

		const Fcolor oclr = READ_IF_EXISTS(pSettings, r_fcolor, m_light_section, b_r2 ? "omni_color_r2" : "omni_color", (Fcolor{ 1.0f , 1.0f , 1.0f , 0.0f }));
		flashlight_omni->set_color(oclr);
		const float orange = READ_IF_EXISTS(pSettings, r_float, m_light_section, b_r2 ? "omni_range_r2" : "omni_range", 0.25f);
		flashlight_omni->set_range(orange);

		flashlight_glow = ::Render->glow_create();
		flashlight_glow->set_texture(READ_IF_EXISTS(pSettings, r_string, m_light_section, "glow_texture", "glow\\glow_torch_r2"));
		flashlight_glow->set_color(clr);
		flashlight_glow->set_radius(READ_IF_EXISTS(pSettings, r_float, m_light_section, "glow_radius", 0.3f));
	}

	hud_recalc_koef = READ_IF_EXISTS(pSettings, r_float, m_hud_sect, "hud_recalc_koef", 1.35f); //На калаше при 1.35 вроде норм смотрится, другим стволам возможно придется подбирать другие значения.

	m_SuitableRepairKits.clear();
	m_ItemsForRepair.clear();

	LPCSTR repair_kits = READ_IF_EXISTS(pSettings, r_string, section, "suitable_repair_kits", "repair_kit");
	LPCSTR items_for_repair = READ_IF_EXISTS(pSettings, r_string, section, "items_for_repair", "");

	// Added by Axel, to enable optional condition use on any item
	m_flags.set(FUsingCondition, READ_IF_EXISTS(pSettings, r_bool, section, "use_condition", true));

	m_bShowWpnStats = READ_IF_EXISTS(pSettings, r_bool, section, "show_wpn_stats", true);
	m_bEnableBoreDof = READ_IF_EXISTS(pSettings, r_bool, section, "enable_bore_dof", true);
	m_bUseAimAnmDirDependency = READ_IF_EXISTS(pSettings, r_bool, section, "enable_aim_anm_dir_dependency", false);
	m_bUseScopeAimMoveAnims = READ_IF_EXISTS(pSettings, r_bool, section, "enable_scope_aim_move_anm", true);
	m_bUseAimSilShotAnim = READ_IF_EXISTS(pSettings, r_bool, section, "enable_aim_silencer_shoot_anm", false);
	m_bAltZoomEnabled = READ_IF_EXISTS(pSettings, r_bool, section, "enable_alternative_aim", false);

	if (repair_kits && repair_kits[0])
	{
		string128 repair_kits_sect;
		int count = _GetItemCount(repair_kits);
		for (int it = 0; it < count; ++it)
		{
			_GetItem(repair_kits, it, repair_kits_sect);
			m_SuitableRepairKits.push_back(repair_kits_sect);
		}
	}

	if (items_for_repair && items_for_repair[0])
	{
		string128 items_for_repair_sect;
		int count = _GetItemCount(items_for_repair);

		for (int it = 0; it < count; ++it)
		{
			_GetItem(items_for_repair, it, items_for_repair_sect);

			if ((it % 2 != 0 && it != 0) || it == 1)
				m_ItemsForRepair[it / 2].second = std::stoi(items_for_repair_sect);
			else
				m_ItemsForRepair.push_back(std::make_pair(items_for_repair_sect, 0));
		}
	}

	ProcessBlendCamParams(READ_IF_EXISTS(pSettings, r_string, m_hud_sect, "blend_aim_start",	nullptr),	m_BlendAimStartCam);
	ProcessBlendCamParams(READ_IF_EXISTS(pSettings, r_string, m_hud_sect, "blend_aim_end",		nullptr),	m_BlendAimEndCam);
	ProcessBlendCamParams(READ_IF_EXISTS(pSettings, r_string, m_hud_sect, "blend_aim_idle",		nullptr),	m_BlendAimIdleCam);
	ProcessBlendCamParams(READ_IF_EXISTS(pSettings, r_string, m_hud_sect, "blend_fakeshot",		nullptr),	m_BlendFakeShootCam);
}

void CWeapon::ProcessBlendCamParams(LPCSTR params, BlendCamParams& cam_params)
{
    if (params && params[0])
    {
        string512 anim_param{}, anim_name{};
        int count = _GetItemCount(params);

        for (int it = 0; it < count; ++it)
        {
            _GetItem(params, it, anim_param);

            switch (it)
            {
            case 0:
                {
                    strconcat(sizeof(anim_name), anim_name, "blend\\", anim_param, ".anm");
					cam_params.name = anim_name;
                } break;
            case 1:
                {
					cam_params.speed = std::stof(anim_param);
                } break;
            case 2:
                {
					cam_params.power = std::stof(anim_param);
                } break;
            default:
                break;
            }
        }
    }
}

void CWeapon::LoadFireParams		(LPCSTR section)
{
	cam_recoil.Dispersion = deg2rad( pSettings->r_float( section,"cam_dispersion" ) ); 
	cam_recoil.DispersionInc = 0.0f;

	if ( pSettings->line_exist( section, "cam_dispersion_inc" ) )	{
		cam_recoil.DispersionInc = deg2rad( pSettings->r_float( section, "cam_dispersion_inc" ) ); 
	}
	
	zoom_cam_recoil.Dispersion		= cam_recoil.Dispersion;
	zoom_cam_recoil.DispersionInc	= cam_recoil.DispersionInc;

	if ( pSettings->line_exist( section, "zoom_cam_dispersion" ) )	{
		zoom_cam_recoil.Dispersion		= deg2rad( pSettings->r_float( section, "zoom_cam_dispersion" ) ); 
	}
	if ( pSettings->line_exist( section, "zoom_cam_dispersion_inc" ) )	{
		zoom_cam_recoil.DispersionInc	= deg2rad( pSettings->r_float( section, "zoom_cam_dispersion_inc" ) ); 
	}

	CShootingObject::LoadFireParams(section);
};

void CWeapon::LoadGrenadeLauncherParams(LPCSTR section)
{
	if (m_eGrenadeLauncherStatus == ALife::eAddonAttachable)
	{
		if (pSettings->line_exist(section, "grenade_launcher_attach_sect"))
		{
			m_sGrenadeLauncherAttachSection = pSettings->r_string(section, "grenade_launcher_attach_sect");
			m_sGrenadeLauncherName = pSettings->r_string(m_sGrenadeLauncherAttachSection, "grenade_launcher_name");

			m_iGrenadeLauncherX = pSettings->r_s32(m_sGrenadeLauncherAttachSection, "grenade_launcher_x");
			m_iGrenadeLauncherY = pSettings->r_s32(m_sGrenadeLauncherAttachSection, "grenade_launcher_y");
		}
		else
		{
			m_sGrenadeLauncherName = pSettings->r_string(section, "grenade_launcher_name");

			m_iGrenadeLauncherX = pSettings->r_s32(section, "grenade_launcher_x");
			m_iGrenadeLauncherY = pSettings->r_s32(section, "grenade_launcher_y");
		}
	}
	else if (m_eGrenadeLauncherStatus == ALife::eAddonPermanent)
	{
		m_sGrenadeLauncherName = READ_IF_EXISTS(pSettings, r_string, section, "grenade_launcher_name", "");
		m_sGrenadeLauncherAttachSection = READ_IF_EXISTS(pSettings, r_string, section, "grenade_launcher_attach_sect", "");
	}
}

void CWeapon::LoadSilencerParams(LPCSTR section)
{
	if (m_eSilencerStatus == ALife::eAddonAttachable)
	{
		if (pSettings->line_exist(section, "silencer_attach_sect"))
		{
			m_sSilencerAttachSection = pSettings->r_string(section, "silencer_attach_sect");
			m_sSilencerName = pSettings->r_string(m_sSilencerAttachSection, "silencer_name");

			m_iSilencerX = pSettings->r_s32(m_sSilencerAttachSection, "silencer_x");
			m_iSilencerY = pSettings->r_s32(m_sSilencerAttachSection, "silencer_y");
		}
		else
		{
			m_sSilencerName = pSettings->r_string(section, "silencer_name");

			m_iSilencerX = pSettings->r_s32(section, "silencer_x");
			m_iSilencerY = pSettings->r_s32(section, "silencer_y");
		}
	}
	else if (m_eSilencerStatus == ALife::eAddonPermanent)
	{
		m_sSilencerName = READ_IF_EXISTS(pSettings, r_string, section, "silencer_name", "");
		m_sSilencerAttachSection = READ_IF_EXISTS(pSettings, r_string, section, "silencer_attach_sect", "");
	}
}

void CWeapon::LoadLaserDesignatorParams(LPCSTR section)
{
	if (m_eLaserDesignatorStatus == ALife::eAddonAttachable)
	{
		if (pSettings->line_exist(section, "laser_designator_attach_sect"))
		{
			m_sLaserAttachSection = pSettings->r_string(section, "laser_designator_attach_sect");
			m_sLaserName = pSettings->r_string(m_sLaserAttachSection, "laser_designator_name");

			m_iLaserX = pSettings->r_s32(m_sLaserAttachSection, "laser_designator_x");
			m_iLaserY = pSettings->r_s32(m_sLaserAttachSection, "laser_designator_y");
		}
		else
		{
			m_sLaserName = pSettings->r_string(section, "laser_designator_name");

			m_iLaserX = pSettings->r_s32(section, "laser_designator_x");
			m_iLaserY = pSettings->r_s32(section, "laser_designator_y");
		}
	}
	else if (m_eLaserDesignatorStatus == ALife::eAddonPermanent)
	{
		m_sLaserName = READ_IF_EXISTS(pSettings, r_string, section, "laser_designator_name", "");
		m_sLaserAttachSection = READ_IF_EXISTS(pSettings, r_string, section, "laser_designator_attach_sect", "");
	}
}

void CWeapon::LoadTacticalTorchParams(LPCSTR section)
{
	if (m_eTacticalTorchStatus == ALife::eAddonAttachable)
	{
		if (pSettings->line_exist(section, "tactical_torch_attach_sect"))
		{
			m_sTacticalTorchAttachSection = pSettings->r_string(section, "tactical_torch_attach_sect");
			m_sTacticalTorchName = pSettings->r_string(m_sTacticalTorchAttachSection, "tactical_torch_name");

			m_iTacticalTorchX = pSettings->r_s32(m_sTacticalTorchAttachSection, "tactical_torch_x");
			m_iTacticalTorchY = pSettings->r_s32(m_sTacticalTorchAttachSection, "tactical_torch_y");
		}
		else
		{
			m_sTacticalTorchName = pSettings->r_string(section, "tactical_torch_name");

			m_iTacticalTorchX = pSettings->r_s32(section, "tactical_torch_x");
			m_iTacticalTorchY = pSettings->r_s32(section, "tactical_torch_y");
		}
	}
	else if (m_eLaserDesignatorStatus == ALife::eAddonPermanent)
	{
		m_sTacticalTorchName = READ_IF_EXISTS(pSettings, r_string, section, "tactical_torch_name", "");
		m_sTacticalTorchAttachSection = READ_IF_EXISTS(pSettings, r_string, section, "tactical_torch_attach_sect", "");
	}
}

bool CWeapon::bReloadSectionScope(LPCSTR section)
{
	if (!pSettings->line_exist(section, "scopes"))
		return false;

	if (pSettings->r_string(section, "scopes") == NULL)
		return false;

	if (xr_strcmp(pSettings->r_string(section, "scopes"), "none") == 0)
		return false;

	return true;
}

bool CWeapon::bLoadAltScopesParams(LPCSTR section)
{
	if (!pSettings->line_exist(section, "scopes"))
		return false;

	if (pSettings->r_string(section, "scopes") == NULL)
		return false;

	if (xr_strcmp(pSettings->r_string(section, "scopes"), "none") == 0)
		return false;

	if (m_eScopeStatus == ALife::eAddonAttachable)
	{
		LPCSTR str = pSettings->r_string(section, "scopes");
		for (int i = 0, count = _GetItemCount(str); i < count; ++i)
		{
			string128 scope_section;
			_GetItem(str, i, scope_section);
			m_scopes.push_back(scope_section);
		}
	}
	else if (m_eScopeStatus == ALife::eAddonPermanent)
	{
		LoadCurrentScopeParams(section);
	}

	return true;
}

void CWeapon::LoadOriginalScopesParams(LPCSTR section)
{
	if (m_eScopeStatus == ALife::eAddonAttachable)
	{
		if (pSettings->line_exist(section, "scopes_sect"))
		{
			LPCSTR str = pSettings->r_string(section, "scopes_sect");
			for (int i = 0, count = _GetItemCount(str); i < count; ++i)
			{
				string128						scope_section;
				_GetItem(str, i, scope_section);
				m_scopes.push_back(scope_section);
			}
		}
		else
		{
			m_scopes.push_back(section);
		}
	}
	else if (m_eScopeStatus == ALife::eAddonPermanent)
	{
		LoadCurrentScopeParams(section);
	}
}

void createWpnScopeXML()
{
	if (!pWpnScopeXml)
	{
		pWpnScopeXml = xr_new<CUIXml>();
		pWpnScopeXml->Load(CONFIG_PATH, UI_PATH, "scopes.xml");
	}
}

void CWeapon::LoadCurrentScopeParams(LPCSTR section)
{
	shared_str scope_tex_name = "none";
	bScopeIsHasTexture = false;
	if (pSettings->line_exist(section, "scope_texture"))
	{
		scope_tex_name = pSettings->r_string(section, "scope_texture");
		if (xr_strcmp(scope_tex_name, "none") != 0)
			bScopeIsHasTexture = true;
	}

	string256 attach_sect;
	strconcat(sizeof(attach_sect), attach_sect, m_eScopeStatus == ALife::eAddonPermanent ? "scope" : m_scopes[m_cur_scope].c_str(), "_attach_sect");

	if (attach_sect && pSettings->line_exist(m_section_id.c_str(), attach_sect))
		m_sScopeAttachSection = READ_IF_EXISTS(pSettings, r_string, m_section_id.c_str(), attach_sect, "");

	m_zoom_params.m_fScopeZoomFactor = pSettings->r_float(section, "scope_zoom_factor");
	Load3DScopeParams(section);

	shared_str cur_scope_sect = (m_sScopeAttachSection.size() ? m_sScopeAttachSection : (m_eScopeStatus == ALife::eAddonAttachable) ? m_scopes[m_cur_scope].c_str() : "scope");
	m_bAltZoomEnabledScope = READ_IF_EXISTS(pSettings, r_bool, cur_scope_sect, "enable_alternative_aim", false);

	if (bIsSecondVPZoomPresent())
	{
		bScopeIsHasTexture = false;
		m_zoom_params.m_fScopeZoomFactor = pSettings->r_float(section, "3d_zoom_factor");
	}
	else
	{
		m_zoom_params.m_fScopeZoomFactor = pSettings->r_float(section, "scope_zoom_factor");
	}

	if (bScopeIsHasTexture || bIsSecondVPZoomPresent())
	{
		if (bIsSecondVPZoomPresent())
			bNVsecondVPavaible = !!pSettings->line_exist(section, "scope_nightvision");
		else
			m_zoom_params.m_sUseZoomPostprocess = READ_IF_EXISTS(pSettings, r_string, section, "scope_nightvision", 0);

		m_zoom_params.m_bUseDynamicZoom = READ_IF_EXISTS(pSettings, r_bool, section, "scope_dynamic_zoom", FALSE);

		if (m_zoom_params.m_bUseDynamicZoom)
		{
			m_fZoomStepCount = READ_IF_EXISTS(pSettings, r_u8, section, "scope_zoom_steps", 3.0f);
			m_fZoomMinKoeff = READ_IF_EXISTS(pSettings, r_u8, section, "min_zoom_k", 0.3f);
		}

		m_zoom_params.m_sUseBinocularVision = READ_IF_EXISTS(pSettings, r_string, section, "scope_alive_detector", 0);
	}
	else
	{
		bNVsecondVPavaible = false;
		bNVsecondVPstatus = false;
	}

	m_fScopeInertionFactor = READ_IF_EXISTS(pSettings, r_float, section, "scope_inertion_factor", m_fControlInertionFactor);

	if (GameConstants::GetOGSE_WpnZoomSystem())
	{
		float delta, min_zoom_factor;
		GetZoomData(m_zoom_params.m_fScopeZoomFactor, delta, min_zoom_factor);

		m_fRTZoomFactor = min_zoom_factor; // set minimal zoom by default for ogse mode
	}
	else
	{
		m_fRTZoomFactor = m_zoom_params.m_fScopeZoomFactor;
	}

	if (m_UIScope)
	{
		xr_delete(m_UIScope);
	}

	if (bScopeIsHasTexture)
	{
		m_UIScope = xr_new<CUIWindow>();
		createWpnScopeXML();
		CUIXmlInit::InitWindow(*pWpnScopeXml, scope_tex_name.c_str(), 0, m_UIScope);
	}
}

void CWeapon::Load3DScopeParams(LPCSTR section)
{
	if (psActorFlags.test(AF_3DSCOPE_ENABLE))
	{
		m_zoom_params.m_fSecondVPFovFactor = READ_IF_EXISTS(pSettings, r_float, section, "3d_fov", 0.0f);
		m_zoom_params.m_f3dZoomFactor = READ_IF_EXISTS(pSettings, r_float, section, "3d_zoom_factor", 100.0f);
	}
	else
	{
		m_zoom_params.m_fSecondVPFovFactor = 0.0f;
		m_zoom_params.m_f3dZoomFactor = 0.0f;
	}

}

BOOL CWeapon::net_Spawn		(CSE_Abstract* DC)
{
	m_fRTZoomFactor					= m_zoom_params.m_fScopeZoomFactor;
	BOOL bResult					= inherited::net_Spawn(DC);
	CSE_Abstract					*e	= (CSE_Abstract*)(DC);
	CSE_ALifeItemWeapon			    *E	= smart_cast<CSE_ALifeItemWeapon*>(e);

	//iAmmoCurrent					= E->a_current;
	iAmmoElapsed					= E->a_elapsed;
	m_flagsAddOnState				= E->m_addon_flags.get();
	m_ammoType						= E->ammo_type;
	if (E->cur_scope < m_scopes.size() && m_scopes.size()>1)
		m_cur_scope						= E->cur_scope;
	SetState						(E->wpn_state);
	SetNextState					(E->wpn_state);
	
	m_DefaultCartridge.Load(*m_ammoTypes[m_ammoType], u8(m_ammoType));	
	if(iAmmoElapsed) 
	{
		m_fCurrentCartirdgeDisp = m_DefaultCartridge.param_s.kDisp;
		for(int i = 0; i < iAmmoElapsed; ++i) 
			m_magazine.push_back(m_DefaultCartridge);
	}

	UpdateAltScope();
	UpdateAddonsVisibility();
	InitAddons();

	m_dwWeaponIndependencyTime = 0;

	VERIFY((u32)iAmmoElapsed == m_magazine.size());
	m_bAmmoWasSpawned		= false;

	return bResult;
}

void CWeapon::net_Destroy	()
{
	inherited::net_Destroy	();

	//удалить объекты партиклов
	StopFlameParticles	();
	StopFlameParticles2	();
	StopLight			();
	Light_Destroy		();

	while (m_magazine.size()) m_magazine.pop_back();
}

BOOL CWeapon::IsUpdating()
{	
	bool bIsActiveItem = m_pInventory && m_pInventory->ActiveItem()==this;
	return bIsActiveItem || bWorking;// || IsPending() || getVisible();
}

void CWeapon::net_Export(NET_Packet& P)
{
	inherited::net_Export	(P);

	P.w_float_q8			(GetCondition(),0.0f,1.0f);


	u8 need_upd				= IsUpdating() ? 1 : 0;
	P.w_u8					(need_upd);
	P.w_u16					(u16(iAmmoElapsed));
	P.w_u8					(m_flagsAddOnState);
	P.w_u8					((u8)m_ammoType);
	P.w_u8					((u8)GetState());
	P.w_u8					((u8)IsZoomed());
	P.w_u8					((u8)m_cur_scope);
}

void CWeapon::net_Import(NET_Packet& P)
{
	inherited::net_Import (P);
	
	float _cond;
	P.r_float_q8			(_cond,0.0f,1.0f);
	SetCondition			(_cond);

	u8 flags				= 0;
	P.r_u8					(flags);

	u16 ammo_elapsed = 0;
	P.r_u16					(ammo_elapsed);

	u8						NewAddonState;
	P.r_u8					(NewAddonState);

	m_flagsAddOnState		= NewAddonState;
	UpdateAddonsVisibility	();

	u8 ammoType, wstate;
	P.r_u8					(ammoType);
	P.r_u8					(wstate);

	u8 Zoom;
	P.r_u8					((u8)Zoom);

	u8 scope;
	P.r_u8(scope);
	
	m_cur_scope = scope;

	if (H_Parent() && H_Parent()->Remote())
	{
		if (Zoom) OnZoomIn();
		else OnZoomOut();
	};
	switch (wstate)
	{	
	case eFire:
	case eFire2:
	case eSwitch:
	case eReload:
		{
		}break;	
	default:
		{
			if (ammoType >= m_ammoTypes.size())
				Msg("!! Weapon [%d], State - [%d]", ID(), wstate);
			else
			{
				m_ammoType = ammoType;
				SetAmmoElapsed((ammo_elapsed));
			}
		}break;
	}
	
	VERIFY((u32)iAmmoElapsed == m_magazine.size());
}

void CWeapon::save(NET_Packet &output_packet)
{
	inherited::save	(output_packet);
	save_data		(iAmmoElapsed,					output_packet);
	save_data		(m_cur_scope, 					output_packet);
	save_data		(m_flagsAddOnState, 			output_packet);
	save_data		(m_ammoType,					output_packet);
	save_data		(m_zoom_params.m_bIsZoomModeNow,output_packet);
	save_data		(m_bRememberActorNVisnStatus,	output_packet);
	save_data		(bNVsecondVPstatus,				output_packet);
}

void CWeapon::load(IReader &input_packet)
{
	inherited::load	(input_packet);
	load_data		(iAmmoElapsed,					input_packet);
	load_data		(m_cur_scope,					input_packet);
	load_data		(m_flagsAddOnState,				input_packet);
	UpdateAddonsVisibility			();
	load_data		(m_ammoType,					input_packet);
	load_data		(m_zoom_params.m_bIsZoomModeNow,input_packet);

	if (m_zoom_params.m_bIsZoomModeNow)	
			OnZoomIn();
		else			
			OnZoomOut();

	load_data		(m_bRememberActorNVisnStatus,	input_packet);
	load_data		(bNVsecondVPstatus,				input_packet);
}


void CWeapon::OnEvent(NET_Packet& P, u16 type) 
{
	switch (type)
	{
	case GE_ADDON_CHANGE:
		{
			P.r_u8					(m_flagsAddOnState);
			InitAddons();
			UpdateAddonsVisibility();
		}break;

	case GE_WPN_STATE_CHANGE:
		{
			u8				state;
			P.r_u8			(state);
			P.r_u8			(m_sub_state);		
//			u8 NewAmmoType = 
				P.r_u8();
			u8 AmmoElapsed = P.r_u8();
			u8 NextAmmo = P.r_u8();
			if (NextAmmo == u8(-1))
				m_set_next_ammoType_on_reload = u32(-1);
			else
				m_set_next_ammoType_on_reload = u8(NextAmmo);

			if (OnClient()) SetAmmoElapsed(int(AmmoElapsed));			
			OnStateSwitch	(u32(state));
		}
		break;
	default:
		{
			inherited::OnEvent(P,type);
		}break;
	}
};

void CWeapon::shedule_Update	(u32 dT)
{
	// Queue shrink
//	u32	dwTimeCL		= Level().timeServer()-NET_Latency;
//	while ((NET.size()>2) && (NET[1].dwTimeStamp<dwTimeCL)) NET.pop_front();	

	// Inherited
	inherited::shedule_Update	(dT);
}

void CWeapon::OnH_B_Independent	(bool just_before_destroy)
{
	RemoveShotEffector			();

	inherited::OnH_B_Independent(just_before_destroy);

	FireEnd						();
	SetPending					(FALSE);
	SwitchState					(eHidden);

	m_strapped_mode				= false;
	m_strapped_mode_rifle = false;
	m_zoom_params.m_bIsZoomModeNow	= false;
	UpdateXForm					();
}

void CWeapon::OnH_A_Independent	()
{
	m_dwWeaponIndependencyTime = Level().timeServer();
	inherited::OnH_A_Independent();

	ResetShootingEffect			();

	Light_Destroy				();
	UpdateAddonsVisibility		();
};

void CWeapon::OnH_A_Chield		()
{
	inherited::OnH_A_Chield		();
	UpdateAddonsVisibility		();
};

void CWeapon::OnActiveItem ()
{
	//. from Activate
	UpdateAddonsVisibility();
	m_dwAmmoCurrentCalcFrame = 0;

//. Show
	SwitchState					(eShowing);
//-

	inherited::OnActiveItem		();
	//если мы занружаемся и оружие было в руках
//.	SetState					(eIdle);
//.	SetNextState				(eIdle);
}

void CWeapon::OnHiddenItem ()
{
	m_dwAmmoCurrentCalcFrame = 0;
	if(IsGameTypeSingle())
		SwitchState(eHiding);
	else
		SwitchState(eHidden);
	OnZoomOut();
	inherited::OnHiddenItem		();

	m_set_next_ammoType_on_reload = u32(-1);
}

void CWeapon::SendHiddenItem()
{
	if (!CHudItem::object().getDestroy() && m_pInventory)
	{
		// !!! Just single entry for given state !!!
		NET_Packet		P;
		CHudItem::object().u_EventGen		(P,GE_WPN_STATE_CHANGE,CHudItem::object().ID());
		P.w_u8			(u8(eHiding));
		P.w_u8			(u8(m_sub_state));
		P.w_u8			(u8(m_ammoType& 0xff));
		P.w_u8			(u8(iAmmoElapsed & 0xff));
		P.w_u8			(u8(m_set_next_ammoType_on_reload & 0xff));
		CHudItem::object().u_EventSend		(P, net_flags(TRUE, TRUE, FALSE, TRUE));
		SetPending		(TRUE);
	}
}


void CWeapon::OnH_B_Chield		()
{
	m_dwWeaponIndependencyTime = 0;
	inherited::OnH_B_Chield		();

	OnZoomOut					();
	m_set_next_ammoType_on_reload	= u32(-1);
}

extern int hud_adj_mode;

void CWeapon::UpdateCL		()
{
	inherited::UpdateCL		();
	UpdateHUDAddonsVisibility();
	//подсветка от выстрела
	UpdateLight				();
	UpdateLaser				();
	UpdateFlashlight		();

	//нарисовать партиклы
	UpdateFlameParticles	();
	UpdateFlameParticles2	();

	if(!IsGameTypeSingle())
		make_Interpolation		();
	
	if( (GetNextState()==GetState()) && IsGameTypeSingle() && H_Parent()==Level().CurrentEntity())
	{
		CActor* pActor	= smart_cast<CActor*>(H_Parent());
		if(pActor && !pActor->AnyMove() && this==pActor->inventory().ActiveItem())
		{
			if (hud_adj_mode==0 && 
				GetState()==eIdle && 
				(Device.dwTimeGlobal-m_dw_curr_substate_time>20000) && 
				!IsZoomed()&&
				g_player_hud->attached_item(1)==NULL)
			{
				if(AllowBore())
					SwitchState		(eBore);

				ResetSubStateTime	();
			}
		}
	}

	if(m_zoom_params.m_pNight_vision && !need_renderable())
	{
		if(!m_zoom_params.m_pNight_vision->IsActive())
		{
			CActor *pA = smart_cast<CActor *>(H_Parent());
			R_ASSERT(pA);
			if (pA->GetNightVisionStatus())
			{
				m_bRememberActorNVisnStatus = pA->GetNightVisionStatus();
				pA->SwitchNightVision(false, false, false);
			}
			m_zoom_params.m_pNight_vision->StartForScope(m_zoom_params.m_sUseZoomPostprocess, pA, false);
		}

	}
	else if(m_bRememberActorNVisnStatus)
	{
		m_bRememberActorNVisnStatus = false;
		EnableActorNVisnAfterZoom();
	}

	if(m_zoom_params.m_pVision)
		m_zoom_params.m_pVision->Update();

	if (m_BlendAimIdleCam.name.size() && !g_player_hud->IsBlendAnmActive(m_BlendAimStartCam.name.c_str()) && !g_player_hud->IsBlendAnmActive(m_BlendAimIdleCam.name.c_str()) && GetHUDmode() &&  IsZoomed())
	{
		if (m_BlendAimStartCam.name.size())
			g_player_hud->StopBlendAnm(m_BlendAimStartCam.name.c_str());

		g_player_hud->PlayBlendAnm(m_BlendAimIdleCam.name.c_str(), 2, m_BlendAimIdleCam.speed, m_BlendAimIdleCam.power, true, false);
	}
}

void CWeapon::GetBoneOffsetPosDir(const shared_str& bone_name, Fvector& dest_pos, Fvector& dest_dir, const Fvector& offset)
{
	const u16 bone_id = HudItemData()->m_model->LL_BoneID(bone_name);
	//ASSERT_FMT(bone_id != BI_NONE, "!![%s] bone [%s] not found in weapon [%s]", __FUNCTION__, bone_name.c_str(), cNameSect().c_str());
	Fmatrix& fire_mat = HudItemData()->m_model->LL_GetTransform(bone_id);
	fire_mat.transform_tiny(dest_pos, offset);
	HudItemData()->m_item_transform.transform_tiny(dest_pos);
	dest_pos.add(Device.vCameraPosition);
	dest_dir.set(0.f, 0.f, 1.f);
	HudItemData()->m_item_transform.transform_dir(dest_dir);
}

void CWeapon::CorrectDirFromWorldToHud(Fvector& dir)
{
	const auto& CamDir = Device.vCameraDirection;
	const float Fov = Device.fFOV;
	extern ENGINE_API float psHUD_FOV;
	const float HudFov = psHUD_FOV < 1.f ? psHUD_FOV * Device.fFOV : psHUD_FOV;
	const float diff = hud_recalc_koef * Fov / HudFov;
	dir.sub(CamDir);
	dir.mul(diff);
	dir.add(CamDir);
	dir.normalize();
}

void CWeapon::UpdateLaser()
{
	if (laser_light_render)
	{
		auto io = smart_cast<CInventoryOwner*>(H_Parent());
		if (!laser_light_render->get_active() && IsLaserOn() && (!H_Parent() || (io && this == io->inventory().ActiveItem())))
		{
			laser_light_render->set_active(true);
			UpdateAddonsVisibility();
		}
		else if (laser_light_render->get_active() && (!IsLaserOn() || !(!H_Parent() || (io && this == io->inventory().ActiveItem()))))
		{
			laser_light_render->set_active(false);
			UpdateAddonsVisibility();
		}

		if (laser_light_render->get_active())
		{
			Fvector laser_pos = get_LastFP(), laser_dir = get_LastFD();

			if (GetHUDmode())
			{
				if (laserdot_attach_bone.size())
				{
					GetBoneOffsetPosDir(laserdot_attach_bone, laser_pos, laser_dir, laserdot_attach_offset);
					CorrectDirFromWorldToHud(laser_dir);
				}
			}
			else
			{
				XFORM().transform_tiny(laser_pos, laserdot_world_attach_offset);
			}

			Fmatrix laserXForm;
			laserXForm.identity();
			laserXForm.k.set(laser_dir);
			Fvector::generate_orthonormal_basis_normalized(laserXForm.k, laserXForm.j, laserXForm.i);

			laser_light_render->set_position(laser_pos);
			laser_light_render->set_rotation(laserXForm.k, laserXForm.i);

			// calc color animator
			if (laser_lanim)
			{
				int frame;
				const u32 clr = laser_lanim->CalculateBGR(Device.fTimeGlobal, frame);

				Fcolor fclr{ (float)color_get_B(clr), (float)color_get_G(clr), (float)color_get_R(clr), 1.f };
				fclr.mul_rgb(laser_fBrightness / 255.f);
				laser_light_render->set_color(fclr);
			}
		}
	}
}

void CWeapon::UpdateFlashlight()
{
	if (flashlight_render)
	{
		auto io = smart_cast<CInventoryOwner*>(H_Parent());
		if (!flashlight_render->get_active() && IsFlashlightOn() && (!H_Parent() || (io && this == io->inventory().ActiveItem())))
		{
			flashlight_render->set_active(true);
			flashlight_omni->set_active(true);
			flashlight_glow->set_active(true);
			UpdateAddonsVisibility();
		}
		else if (flashlight_render->get_active() && (!IsFlashlightOn() || !(!H_Parent() || (io && this == io->inventory().ActiveItem()))))
		{
			flashlight_render->set_active(false);
			flashlight_omni->set_active(false);
			flashlight_glow->set_active(false);
			UpdateAddonsVisibility();
		}

		if (flashlight_render->get_active())
		{
			Fvector flashlight_pos, flashlight_pos_omni, flashlight_dir, flashlight_dir_omni;

			if (GetHUDmode())
			{
				GetBoneOffsetPosDir(flashlight_attach_bone, flashlight_pos, flashlight_dir, flashlight_attach_offset);
				CorrectDirFromWorldToHud(flashlight_dir);

				GetBoneOffsetPosDir(flashlight_attach_bone, flashlight_pos_omni, flashlight_dir_omni, flashlight_omni_attach_offset);
				CorrectDirFromWorldToHud(flashlight_dir_omni);
			}
			else
			{
				flashlight_dir = get_LastFD();
				XFORM().transform_tiny(flashlight_pos, flashlight_world_attach_offset);

				flashlight_dir_omni = get_LastFD();
				XFORM().transform_tiny(flashlight_pos_omni, flashlight_omni_world_attach_offset);
			}

			Fmatrix flashlightXForm;
			flashlightXForm.identity();
			flashlightXForm.k.set(flashlight_dir);
			Fvector::generate_orthonormal_basis_normalized(flashlightXForm.k, flashlightXForm.j, flashlightXForm.i);
			flashlight_render->set_position(flashlight_pos);
			flashlight_render->set_rotation(flashlightXForm.k, flashlightXForm.i);

			flashlight_glow->set_position(flashlight_pos);
			flashlight_glow->set_direction(flashlightXForm.k);

			Fmatrix flashlightomniXForm;
			flashlightomniXForm.identity();
			flashlightomniXForm.k.set(flashlight_dir_omni);
			Fvector::generate_orthonormal_basis_normalized(flashlightomniXForm.k, flashlightomniXForm.j, flashlightomniXForm.i);
			flashlight_omni->set_position(flashlight_pos_omni);
			flashlight_omni->set_rotation(flashlightomniXForm.k, flashlightomniXForm.i);

			// calc color animator
			if (flashlight_lanim)
			{
				int frame;
				const u32 clr = flashlight_lanim->CalculateBGR(Device.fTimeGlobal, frame);

				Fcolor fclr{ (float)color_get_B(clr), (float)color_get_G(clr), (float)color_get_R(clr), 1.f };
				fclr.mul_rgb(flashlight_fBrightness / 255.f);
				flashlight_render->set_color(fclr);
				flashlight_omni->set_color(fclr);
				flashlight_glow->set_color(fclr);
			}
		}
	}
}

void CWeapon::EnableActorNVisnAfterZoom()
{
	CActor* pA = smart_cast<CActor*>(H_Parent());
	if (IsGameTypeSingle() && !pA)
		pA = g_actor;

	if (pA)
	{
		pA->SwitchNightVision(true, false, false);
		pA->GetNightVision()->PlaySounds(CNightVisionEffector::eIdleSound);
	}
}

bool CWeapon::need_renderable()
{
	return Render->currentViewPort == MAIN_VIEWPORT && !(IsZoomed() && ZoomTexture() && !IsRotatingToZoom());
}

void CWeapon::renderable_Render		()
{
	UpdateXForm				();

	//нарисовать подсветку

	RenderLight				();	

	//если мы в режиме снайперки, то сам HUD рисовать не надо
	if(IsZoomed() && !IsRotatingToZoom() && ZoomTexture())
		RenderHud		(FALSE);
	else
		RenderHud		(TRUE);

	inherited::renderable_Render	();
}

void CWeapon::signal_HideComplete()
{
	if(H_Parent()) 
		setVisible			(FALSE);
	SetPending				(FALSE);

	m_fLR_MovingFactor = 0.f;
	m_fLR_CameraFactor = 0.f;
	m_fLR_InertiaFactor = 0.f;
	m_fUD_InertiaFactor = 0.f;
}

void CWeapon::SetDefaults()
{
	SetPending			(FALSE);

	m_flags.set			(FUsingCondition, TRUE);
	bMisfire			= false;
	m_flagsAddOnState	= 0;
	m_zoom_params.m_bIsZoomModeNow	= false;
}

void CWeapon::UpdatePosition(const Fmatrix& trans)
{
	Position().set		(trans.c);
	if (m_strapped_mode || m_strapped_mode_rifle)
		XFORM().mul(trans, m_StrapOffset);
	else
		XFORM().mul(trans, m_Offset);

	VERIFY				(!fis_zero(DET(renderable.xform)));
}

/*void CWeapon::UpdatePosition_alt(const Fmatrix& trans) {
	Position().set(trans.c);
	if (m_strapped_mode || m_strapped_mode_rifle)
		XFORM().mul(trans, m_StrapOffset_alt);
	else
		XFORM().mul(trans, m_Offset);

	VERIFY(!fis_zero(DET(renderable.xform)));
}*/


bool CWeapon::Action(s32 cmd, u32 flags) 
{
	if(inherited::Action(cmd, flags)) return true;

	
	switch(cmd) 
	{
		case kWPN_NV_CHANGE:
		{
			return bChangeNVSecondVPStatus();
		}
		case kWPN_FIRE: 
			{
				//если оружие чем-то занято, то ничего не делать
				{				
					if(IsPending())		
						return				false;

					if(flags&CMD_START) 
						FireStart			();
					else 
						FireEnd				();
				};
			} 
			return true;
		case kWPN_NEXT: 
			{
				return SwitchAmmoType(flags);
			} 

		case kWPN_ZOOM:
			if(IsZoomEnabled())
			{
				if(b_toggle_weapon_aim)
				{
					if(flags&CMD_START)
					{
						if(!IsZoomed())
						{
							if(!IsPending())
							{
								if(GetState()!=eIdle)
									SwitchState(eIdle);
								OnZoomIn	();
							}
						}else
							OnZoomOut	();
					}
				}else
				{
					if(flags&CMD_START)
					{
						if(!IsZoomed() && !IsPending())
						{
							if(GetState()!=eIdle)
								SwitchState(eIdle);
							OnZoomIn	();
						}
					}else 
						if(IsZoomed())
							OnZoomOut	();
				}
				return true;
			}else 
				return false;

		case kWPN_ZOOM_INC:
		case kWPN_ZOOM_DEC:
			if(IsZoomEnabled() && IsZoomed() && IsScopeAttached())
			{
				if(cmd==kWPN_ZOOM_INC)  ZoomInc();
				else					ZoomDec();
				return true;
			}else
				return false;
	}
	return false;
}

bool CWeapon::SwitchAmmoType( u32 flags ) 
{
	if ( IsPending() || OnClient() )
	{
		return false;
	}
	if ( !(flags & CMD_START) )
	{
		return false;
	}

	u32 l_newType = m_ammoType;
	bool b1, b2;
	do 
	{
		l_newType = (l_newType+1) % m_ammoTypes.size();
		b1 = l_newType != m_ammoType;
		b2 = unlimited_ammo() ? false : ( !m_pInventory->GetAny( *m_ammoTypes[l_newType] ) );						
	} while( b1 && b2 );

	if ( l_newType != m_ammoType )
	{
		m_set_next_ammoType_on_reload = l_newType;					
		if ( OnServer() )
		{
			Reload();
		}
	}
	return true;
}

void CWeapon::SpawnAmmo(u32 boxCurr, LPCSTR ammoSect, u32 ParentID) 
{
	if(!m_ammoTypes.size())			return;
	if (OnClient())					return;
	m_bAmmoWasSpawned				= true;
	
	int l_type						= 0;
	l_type							%= m_ammoTypes.size();

	if(!ammoSect) ammoSect			= *m_ammoTypes[l_type]; 
	
	++l_type; 
	l_type							%= m_ammoTypes.size();

	CSE_Abstract *D					= F_entity_Create(ammoSect);

	if (D->m_tClassID==CLSID_OBJECT_AMMO	||
		D->m_tClassID==CLSID_OBJECT_A_M209	||
		D->m_tClassID==CLSID_OBJECT_A_VOG25	||
		D->m_tClassID==CLSID_OBJECT_A_OG7B)
	{	
		CSE_ALifeItemAmmo *l_pA		= smart_cast<CSE_ALifeItemAmmo*>(D);
		R_ASSERT					(l_pA);
		l_pA->m_boxSize				= (u16)pSettings->r_s32(ammoSect, "box_size");
		D->s_name					= ammoSect;
		D->set_name_replace			("");
//.		D->s_gameid					= u8(GameID());
		D->s_RP						= 0xff;
		D->ID						= 0xffff;
		if (ParentID == 0xffffffff)	
			D->ID_Parent			= (u16)H_Parent()->ID();
		else
			D->ID_Parent			= (u16)ParentID;

		D->ID_Phantom				= 0xffff;
		D->s_flags.assign			(M_SPAWN_OBJECT_LOCAL);
		D->RespawnTime				= 0;
		l_pA->m_tNodeID				= ai_location().level_vertex_id();

		if(boxCurr == 0xffffffff) 	
			boxCurr					= l_pA->m_boxSize;

		while(boxCurr) 
		{
			l_pA->a_elapsed			= (u16)(boxCurr > l_pA->m_boxSize ? l_pA->m_boxSize : boxCurr);
			NET_Packet				P;
			D->Spawn_Write			(P, TRUE);
			Level().Send			(P,net_flags(TRUE));

			if(boxCurr > l_pA->m_boxSize) 
				boxCurr				-= l_pA->m_boxSize;
			else 
				boxCurr				= 0;
		}
	};
	F_entity_Destroy				(D);
}

int CWeapon::GetSuitableAmmoTotal(bool use_item_to_spawn) const
{
	int l_count = iAmmoElapsed;
	if(!m_pInventory) return l_count;

	//чтоб не делать лишних пересчетов
	if(m_pInventory->ModifyFrame()<=m_dwAmmoCurrentCalcFrame)
		return l_count + iAmmoCurrent;

 	m_dwAmmoCurrentCalcFrame = Device.dwFrame;
	iAmmoCurrent = 0;

	for(int i = 0; i < (int)m_ammoTypes.size(); ++i) 
	{
		LPCSTR l_ammoType = *m_ammoTypes[i];

		for(TIItemContainer::iterator l_it = m_pInventory->m_belt.begin(); m_pInventory->m_belt.end() != l_it; ++l_it) 
		{
			CWeaponAmmo *l_pAmmo = smart_cast<CWeaponAmmo*>(*l_it);

			if(l_pAmmo && !xr_strcmp(l_pAmmo->cNameSect(), l_ammoType)) 
			{
				iAmmoCurrent = iAmmoCurrent + l_pAmmo->m_boxCurr;
			}
		}

		for(TIItemContainer::iterator l_it = m_pInventory->m_ruck.begin(); m_pInventory->m_ruck.end() != l_it; ++l_it) 
		{
			CWeaponAmmo *l_pAmmo = smart_cast<CWeaponAmmo*>(*l_it);
			if(l_pAmmo && !xr_strcmp(l_pAmmo->cNameSect(), l_ammoType)) 
			{
				iAmmoCurrent = iAmmoCurrent + l_pAmmo->m_boxCurr;
			}
		}

		if (!use_item_to_spawn)
			continue;

		if (!inventory_owner().item_to_spawn())
			continue;

		iAmmoCurrent += inventory_owner().ammo_in_box_to_spawn();
	}
	return l_count + iAmmoCurrent;
}

int CWeapon::GetCurrentTypeAmmoTotal() const
{
	int l_count = iAmmoElapsed;
	if ( !m_pInventory )
	{
		return l_count;
	}

	//чтоб не делать лишних пересчетов
	if ( m_pInventory->ModifyFrame() <= m_dwAmmoCurrentCalcFrame )
	{
		return l_count + iAmmoCurrent;
	}

	m_dwAmmoCurrentCalcFrame = Device.dwFrame;
	iAmmoCurrent = 0;

	VERIFY( 0 <= m_ammoType && m_ammoType < m_ammoTypes.size() );
	{
		LPCSTR l_ammoType = m_ammoTypes[m_ammoType].c_str();

		for(TIItemContainer::iterator l_it = m_pInventory->m_belt.begin(); m_pInventory->m_belt.end() != l_it; ++l_it) 
		{
			CWeaponAmmo *l_pAmmo = smart_cast<CWeaponAmmo*>(*l_it);

			if(l_pAmmo && !xr_strcmp(l_pAmmo->cNameSect(), l_ammoType)) 
			{
				iAmmoCurrent = iAmmoCurrent + l_pAmmo->m_boxCurr;
			}
		}

		for(TIItemContainer::iterator l_it = m_pInventory->m_ruck.begin(); m_pInventory->m_ruck.end() != l_it; ++l_it) 
		{
			CWeaponAmmo *l_pAmmo = smart_cast<CWeaponAmmo*>(*l_it);
			if(l_pAmmo && !xr_strcmp(l_pAmmo->cNameSect(), l_ammoType)) 
			{
				iAmmoCurrent = iAmmoCurrent + l_pAmmo->m_boxCurr;
			}
		}
	}
	return l_count + iAmmoCurrent;
}

int CWeapon::GetAmmoCount(u32 ammo_type) const
{
	VERIFY(m_pInventory);
	R_ASSERT(ammo_type < m_ammoTypes.size());

	return GetAmmoCount_forType(m_ammoTypes[ammo_type]);
}

int CWeapon::GetAmmoCount_forType(shared_str const& ammo_type) const
{
	int res = 0;

	TIItemContainer::iterator itb = m_pInventory->m_belt.begin();
	TIItemContainer::iterator ite = m_pInventory->m_belt.end();
	for ( ; itb != ite; ++itb ) 
	{
		CWeaponAmmo*	pAmmo = smart_cast<CWeaponAmmo*>( *itb );
		if ( pAmmo && (pAmmo->cNameSect() == ammo_type) )
		{
			res += pAmmo->m_boxCurr;
		}
	}

	itb = m_pInventory->m_ruck.begin();
	ite = m_pInventory->m_ruck.end();
	for ( ; itb != ite; ++itb ) 
	{
		CWeaponAmmo*	pAmmo = smart_cast<CWeaponAmmo*>( *itb );
		if ( pAmmo && (pAmmo->cNameSect() == ammo_type) )
		{
			res += pAmmo->m_boxCurr;
		}
	}
	return res;
}

float CWeapon::GetConditionMisfireProbability() const
{
// modified by Peacemaker [17.10.08]
//	if(GetCondition() > 0.95f) 
//		return 0.0f;
	if(GetCondition() > misfireStartCondition) 
		return 0.0f;
	if(GetCondition() < misfireEndCondition) 
		return misfireEndProbability;
//	float mis = misfireProbability+powf(1.f-GetCondition(), 3.f)*misfireConditionK;
	float mis = misfireStartProbability + (
		(misfireStartCondition - GetCondition()) *				// condition goes from 1.f to 0.f
		(misfireEndProbability - misfireStartProbability) /		// probability goes from 0.f to 1.f
		((misfireStartCondition == misfireEndCondition) ?		// !!!say "No" to devision by zero
			misfireStartCondition : 
			(misfireStartCondition - misfireEndCondition))
										  );
	clamp(mis,0.0f,0.99f);
	return mis;
}

BOOL CWeapon::CheckForMisfire	()
{
	if (OnClient()) return FALSE;

	float rnd = ::Random.randF(0.f,1.f);
	float mp = GetConditionMisfireProbability();
	if(rnd < mp)
	{
		FireEnd();

		bMisfire = true;
		SwitchState(eMisfire);		
		
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CWeapon::IsMisfire() const
{	
	return bMisfire;
}
void CWeapon::Reload()
{
	OnZoomOut();

	if (ParentIsActor() && !GameConstants::GetReloadIfSprint())
	{
		Actor()->StopSprint();
		Actor()->m_iTrySprintCounter = 0;
	}
}

BOOL CWeapon::IsEmptyMagazine() const
{
	return (iAmmoElapsed == 0);
}

bool CWeapon::IsGrenadeLauncherAttached() const
{
	return (ALife::eAddonAttachable == m_eGrenadeLauncherStatus &&
			0 != (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher)) || 
			ALife::eAddonPermanent == m_eGrenadeLauncherStatus;
}

bool CWeapon::IsScopeAttached() const
{
	return (ALife::eAddonAttachable == m_eScopeStatus &&
			0 != (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonScope)) || 
			ALife::eAddonPermanent == m_eScopeStatus;

}

bool CWeapon::IsSilencerAttached() const
{
	return (ALife::eAddonAttachable == m_eSilencerStatus &&
			0 != (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonSilencer)) || 
			ALife::eAddonPermanent == m_eSilencerStatus;
}

bool CWeapon::IsLaserAttached() const
{
	return (ALife::eAddonAttachable == m_eLaserDesignatorStatus &&
		0 != (m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonLaserDesignator)) ||
		ALife::eAddonPermanent == m_eLaserDesignatorStatus;
}

bool CWeapon::IsTacticalTorchAttached() const
{
	return (ALife::eAddonAttachable == m_eTacticalTorchStatus &&
		0 != (m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonTacticalTorch)) ||
		ALife::eAddonPermanent == m_eTacticalTorchStatus;
}

bool CWeapon::GrenadeLauncherAttachable()
{
	return (ALife::eAddonAttachable == m_eGrenadeLauncherStatus);
}
bool CWeapon::ScopeAttachable()
{
	return (ALife::eAddonAttachable == m_eScopeStatus);
}
bool CWeapon::SilencerAttachable()
{
	return (ALife::eAddonAttachable == m_eSilencerStatus);
}
bool CWeapon::LaserAttachable()
{
	return (ALife::eAddonAttachable == m_eLaserDesignatorStatus);
}
bool CWeapon::TacticalTorchAttachable()
{
	return (ALife::eAddonAttachable == m_eTacticalTorchStatus);
}

void CWeapon::HUD_VisualBulletUpdate(bool force, int force_idx)
{
	if (!bHasBulletsToHide)
		return;

	if (!GetHUDmode()) return;

	bool hide = true;

	if (last_hide_bullet == bullet_cnt || force) hide = false;

	for (u8 b = 0; b < bullet_cnt; b++)
	{
		u16 bone_id = HudItemData()->m_model->LL_BoneID(bullets_bones[b]);

		if (bone_id != BI_NONE)
			HudItemData()->set_bone_visible(bullets_bones[b], !hide);

		if (b == last_hide_bullet) hide = false;
	}
}

void CWeapon::UpdateHUDAddonsVisibility()
{//actor only
	if(!GetHUDmode())										return;

//.	return;

	u16 bone_id = HudItemData()->m_model->LL_BoneID(wpn_scope_def_bone);

	auto SetBoneVisible = [&](const shared_str& boneName, BOOL visibility)
	{
		HudItemData()->set_bone_visible(boneName, visibility, TRUE);
	};

	// Hide default bones
	for (const shared_str& bone : m_defHiddenBones)
	{
		SetBoneVisible(bone, FALSE);
	}

	// Show default bones
	for (const shared_str& bone : m_defShownBones)
	{
		SetBoneVisible(bone, TRUE);
	}

	for (int i = 0; i < m_all_scope_bones.size(); i++)
		SetBoneVisible(m_all_scope_bones[i], FALSE);

	if (m_cur_scope_bone != NULL)
		SetBoneVisible(m_cur_scope_bone, TRUE);

	if (bone_id != BI_NONE)
	{
		if (ScopeAttachable())
		{
			HudItemData()->set_bone_visible(wpn_scope_def_bone, IsScopeAttached() && !m_sScopeAttachSection.size());
		}

		if (m_eScopeStatus == ALife::eAddonDisabled || m_sScopeAttachSection.size())
		{
			HudItemData()->set_bone_visible(wpn_scope_def_bone, FALSE, TRUE);
		}
		else
			if (m_eScopeStatus == ALife::eAddonPermanent && !m_sScopeAttachSection.size())
				HudItemData()->set_bone_visible(wpn_scope_def_bone, TRUE, TRUE);
	}

	if (SilencerAttachable())
	{
		SetBoneVisible(m_sWpn_silencer_bone, IsSilencerAttached() && !m_sSilencerAttachSection.size());
	}
	if (m_eSilencerStatus == ALife::eAddonDisabled || m_sSilencerAttachSection.size())
	{
		SetBoneVisible(m_sWpn_silencer_bone, FALSE);
	}
	else
		if (m_eSilencerStatus == ALife::eAddonPermanent && !m_sSilencerAttachSection.size())
			SetBoneVisible(m_sWpn_silencer_bone, TRUE);

	if (GrenadeLauncherAttachable())
	{
		SetBoneVisible(m_sWpn_launcher_bone, IsGrenadeLauncherAttached() && !m_sGrenadeLauncherAttachSection.size());
	}
	if (m_eGrenadeLauncherStatus == ALife::eAddonDisabled || m_sGrenadeLauncherAttachSection.size())
	{
		SetBoneVisible(m_sWpn_launcher_bone, FALSE);
	}
	else if (m_eGrenadeLauncherStatus == ALife::eAddonPermanent && !m_sGrenadeLauncherAttachSection.size())
		SetBoneVisible(m_sWpn_launcher_bone, TRUE);

	if (LaserAttachable())
	{
		SetBoneVisible(m_sWpn_laser_bone, IsLaserAttached() && !m_sLaserAttachSection.size());
	}
	if (m_eLaserDesignatorStatus == ALife::eAddonDisabled || m_sLaserAttachSection.size())
	{
		SetBoneVisible(m_sWpn_laser_bone, FALSE);
	}
	else
		if (m_eLaserDesignatorStatus == ALife::eAddonPermanent && !m_sLaserAttachSection.size())
			SetBoneVisible(m_sWpn_laser_bone, TRUE);

	if (m_sHud_wpn_laser_ray_bone.size() && has_laser)
		SetBoneVisible(m_sHud_wpn_laser_ray_bone, IsLaserOn());

	if (TacticalTorchAttachable())
	{
		SetBoneVisible(m_sWpn_flashlight_bone, IsLaserAttached() && !m_sTacticalTorchAttachSection.size());
	}
	if (m_eTacticalTorchStatus == ALife::eAddonDisabled || m_sTacticalTorchAttachSection.size())
	{
		SetBoneVisible(m_sWpn_flashlight_bone, FALSE);
	}
	else
		if (m_eTacticalTorchStatus == ALife::eAddonPermanent && !m_sTacticalTorchAttachSection.size())
			SetBoneVisible(m_sWpn_laser_bone, TRUE);

	if (m_sHud_wpn_flashlight_cone_bone.size() && has_flashlight)
		SetBoneVisible(m_sHud_wpn_flashlight_cone_bone, IsFlashlightOn());
}

void CWeapon::UpdateAddonsVisibility()
{
	IKinematics* pWeaponVisual = smart_cast<IKinematics*>(Visual()); R_ASSERT(pWeaponVisual);

	u16 bone_id;
	UpdateHUDAddonsVisibility();	

	pWeaponVisual->CalculateBones_Invalidate();

	bone_id = pWeaponVisual->LL_BoneID(wpn_scope_def_bone);

	auto SetBoneVisible = [&](const shared_str& boneName, BOOL visibility)
	{
		u16 bone_id = pWeaponVisual->LL_BoneID(boneName);
		if (bone_id != BI_NONE && visibility != pWeaponVisual->LL_GetBoneVisible(bone_id))
			pWeaponVisual->LL_SetBoneVisible(bone_id, visibility, TRUE);
	};

	// Hide default bones
	for (const shared_str& bone : m_defHiddenBones)
	{
		SetBoneVisible(bone, FALSE);
	}

	// Show default bones
	for (const shared_str& bone : m_defShownBones)
	{
		SetBoneVisible(bone, TRUE);
	}

	for (int i = 0; i < m_all_scope_bones.size(); i++)
		SetBoneVisible(m_all_scope_bones[i], FALSE);

	if (m_cur_scope_bone != NULL)
		SetBoneVisible(m_cur_scope_bone, TRUE);

	if(ScopeAttachable())
	{
		if(IsScopeAttached() && !m_sScopeAttachSection.size())
		{
			if (bone_id != BI_NONE && !pWeaponVisual->LL_GetBoneVisible(bone_id))
				pWeaponVisual->LL_SetBoneVisible(bone_id,TRUE,TRUE);
		}
		else
		{
			if (bone_id != BI_NONE && pWeaponVisual->LL_GetBoneVisible(bone_id))
				pWeaponVisual->LL_SetBoneVisible(bone_id,FALSE,TRUE);
		}
	}
	if((m_eScopeStatus==ALife::eAddonDisabled || m_sScopeAttachSection.size()) && bone_id!=BI_NONE &&
		pWeaponVisual->LL_GetBoneVisible(bone_id) )
	{
		pWeaponVisual->LL_SetBoneVisible					(bone_id,FALSE,TRUE);
//		Log("scope", pWeaponVisual->LL_GetBoneVisible		(bone_id));
	}

	bone_id = pWeaponVisual->LL_BoneID						(m_sWpn_silencer_bone);

	if(SilencerAttachable())
	{
		if(IsSilencerAttached() && !m_sSilencerAttachSection.size())
		{
			if(!pWeaponVisual->LL_GetBoneVisible(bone_id))
				pWeaponVisual->LL_SetBoneVisible(bone_id,TRUE,TRUE);
		}
		else
		{
			if(bone_id != BI_NONE && pWeaponVisual->LL_GetBoneVisible(bone_id) && bone_id)
				pWeaponVisual->LL_SetBoneVisible(bone_id,FALSE,TRUE);
		}
	}
	if((m_eSilencerStatus==ALife::eAddonDisabled || m_sSilencerAttachSection.size()) && bone_id!=BI_NONE &&
		pWeaponVisual->LL_GetBoneVisible(bone_id) )
	{
		pWeaponVisual->LL_SetBoneVisible					(bone_id,FALSE,TRUE);
//		Log("silencer", pWeaponVisual->LL_GetBoneVisible	(bone_id));
	}

	bone_id = pWeaponVisual->LL_BoneID						(m_sWpn_launcher_bone);
	if(GrenadeLauncherAttachable())
	{
		if(IsGrenadeLauncherAttached() && !m_sGrenadeLauncherAttachSection.size())
		{
			if(!pWeaponVisual->LL_GetBoneVisible(bone_id))
				pWeaponVisual->LL_SetBoneVisible(bone_id,TRUE,TRUE);
		}
		else
		{
			if(bone_id != BI_NONE && pWeaponVisual->LL_GetBoneVisible(bone_id))
				pWeaponVisual->LL_SetBoneVisible(bone_id,FALSE,TRUE);
		}
	}
	if((m_eGrenadeLauncherStatus==ALife::eAddonDisabled || m_sGrenadeLauncherAttachSection.size()) && bone_id!=BI_NONE &&
		pWeaponVisual->LL_GetBoneVisible(bone_id) )
	{
		pWeaponVisual->LL_SetBoneVisible					(bone_id,FALSE,TRUE);
//		Log("gl", pWeaponVisual->LL_GetBoneVisible			(bone_id));
	}

	bone_id = pWeaponVisual->LL_BoneID(m_sWpn_laser_bone);
	if (LaserAttachable())
	{
		if (IsLaserAttached() && !m_sLaserAttachSection.size())
		{
			if (!pWeaponVisual->LL_GetBoneVisible(bone_id))
				pWeaponVisual->LL_SetBoneVisible(bone_id, TRUE, TRUE);
		}
		else
		{
			if (bone_id != BI_NONE && pWeaponVisual->LL_GetBoneVisible(bone_id))
				pWeaponVisual->LL_SetBoneVisible(bone_id, FALSE, TRUE);
		}
	}
	if ((m_eLaserDesignatorStatus == ALife::eAddonDisabled || m_sLaserAttachSection.size()) && bone_id != BI_NONE && pWeaponVisual->LL_GetBoneVisible(bone_id))
	{
		pWeaponVisual->LL_SetBoneVisible(bone_id, FALSE, TRUE);
		//		Log("laser", pWeaponVisual->LL_GetBoneVisible	(bone_id));
	}

	bone_id = pWeaponVisual->LL_BoneID(m_sWpn_flashlight_bone);
	if (TacticalTorchAttachable())
	{
		if (IsTacticalTorchAttached() && !m_sTacticalTorchAttachSection.size())
		{
			if (!pWeaponVisual->LL_GetBoneVisible(bone_id))
				pWeaponVisual->LL_SetBoneVisible(bone_id, TRUE, TRUE);
		}
		else
		{
			if (bone_id != BI_NONE && pWeaponVisual->LL_GetBoneVisible(bone_id))
				pWeaponVisual->LL_SetBoneVisible(bone_id, FALSE, TRUE);
		}
	}
	if ((m_eTacticalTorchStatus == ALife::eAddonDisabled || m_sTacticalTorchAttachSection.size()) && bone_id != BI_NONE && pWeaponVisual->LL_GetBoneVisible(bone_id))
	{
		pWeaponVisual->LL_SetBoneVisible(bone_id, FALSE, TRUE);
		//		Log("tactical torch", pWeaponVisual->LL_GetBoneVisible	(bone_id));
	}

	if (m_sWpn_laser_ray_bone.size() && has_laser)
	{
		bone_id = pWeaponVisual->LL_BoneID(m_sWpn_laser_ray_bone);

		if (bone_id != BI_NONE)
		{
			const bool laser_on = IsLaserOn();
			if (pWeaponVisual->LL_GetBoneVisible(bone_id) && !laser_on)
				pWeaponVisual->LL_SetBoneVisible(bone_id, FALSE, TRUE);
			else if (!pWeaponVisual->LL_GetBoneVisible(bone_id) && laser_on)
				pWeaponVisual->LL_SetBoneVisible(bone_id, TRUE, TRUE);
		}
	}

	///////////////////////////////////////////////////////////////////

	if (m_sWpn_flashlight_cone_bone.size() && has_flashlight)
	{
		bone_id = pWeaponVisual->LL_BoneID(m_sWpn_flashlight_cone_bone);

		if (bone_id != BI_NONE)
		{
			const bool flashlight_on = IsFlashlightOn();
			if (pWeaponVisual->LL_GetBoneVisible(bone_id) && !flashlight_on)
				pWeaponVisual->LL_SetBoneVisible(bone_id, FALSE, TRUE);
			else if (!pWeaponVisual->LL_GetBoneVisible(bone_id) && flashlight_on)
				pWeaponVisual->LL_SetBoneVisible(bone_id, TRUE, TRUE);
		}
	}

	///////////////////////////////////////////////////////////////////

	pWeaponVisual->CalculateBones_Invalidate				();
	pWeaponVisual->CalculateBones							(TRUE);
}


void CWeapon::InitAddons()
{
}

float CWeapon::CurrentZoomFactor()
{
	if (psActorFlags.test(AF_3DSCOPE_ENABLE) && IsScopeAttached())
	{
		return GameConstants::GetOGSE_WpnZoomSystem() ? 1.0f : bIsSecondVPZoomPresent() ? m_zoom_params.m_f3dZoomFactor : m_zoom_params.m_fScopeZoomFactor; // no change to main fov zoom when use second vp
	}
	else
	{
		return (IsScopeAttached() && !m_bAltZoomActive) ? m_zoom_params.m_fScopeZoomFactor : m_zoom_params.m_fIronSightZoomFactor;
	}
};

//      
float CWeapon::GetControlInertionFactor() const
{
	float fInertionFactor = inherited::GetControlInertionFactor();
	if (IsScopeAttached() && IsZoomed())
		return m_fScopeInertionFactor;

	return fInertionFactor;
}

void CWeapon::GetZoomData(const float scope_factor, float& delta, float& min_zoom_factor)
{
	float def_fov = GameConstants::GetOGSE_WpnZoomSystem() ? 1.f : bIsSecondVPZoomPresent() ? 75.0f : g_fov;
	float delta_factor_total = def_fov - scope_factor;
	VERIFY(delta_factor_total > 0);
	min_zoom_factor = def_fov - delta_factor_total * m_fZoomMinKoeff;
	delta = (delta_factor_total * (1 - m_fZoomMinKoeff)) / m_fZoomStepCount;
}

// Lex Addon (correct by Suhar_) 24.10.2018		(begin)
float LastZoomFactor = 0.0f;

void CWeapon::OnZoomIn()
{
	//Alun: Force switch to first-person for zooming
	CActor *pA = smart_cast<CActor *>(H_Parent());
	if (pA && pA->active_cam() == eacLookAt)
	{
		pA->cam_Set(eacFirstEye);
		m_freelook_switch_back = true;
	}

	m_zoom_params.m_bIsZoomModeNow = true;
	if (bIsSecondVPZoomPresent() && m_zoom_params.m_bUseDynamicZoom && IsScopeAttached())
	{
		if (LastZoomFactor)
			m_fRTZoomFactor = LastZoomFactor;
		else
			m_fRTZoomFactor = CurrentZoomFactor();
		float delta, min_zoom_factor;
		GetZoomData(m_zoom_params.m_fScopeZoomFactor, delta, min_zoom_factor);
		clamp(m_fRTZoomFactor, m_zoom_params.m_fScopeZoomFactor, min_zoom_factor);

		SetZoomFactor(m_fRTZoomFactor);
	}
	else
		m_zoom_params.m_fCurrentZoomFactor = CurrentZoomFactor();

		//SetZoomFactor(m_zoom_params.m_bUseDynamicZoom ? m_fRTZoomFactor : CurrentZoomFactor());
	// Lex Addon (correct by Suhar_) 24.10.2018		(end)

	//EnableHudInertion					(FALSE);


	//if(m_zoom_params.m_bZoomDofEnabled && !IsScopeAttached())
	//	GamePersistent().SetEffectorDOF	(m_zoom_params.m_ZoomDof);

	if (GetHUDmode())
	{
		last_hud_fov = psHUD_FOV_def;
		GamePersistent().SetPickableEffectorDOF(true);
		UpdateAimOffsets();

		if (m_BlendAimStartCam.name.size())
			g_player_hud->PlayBlendAnm(m_BlendAimStartCam.name.c_str(), 2, m_BlendAimStartCam.speed, m_BlendAimStartCam.power, false, false);
	}

	if (m_zoom_params.m_sUseBinocularVision.size() && IsScopeAttached() && NULL == m_zoom_params.m_pVision)
		m_zoom_params.m_pVision = xr_new<CBinocularsVision>(m_zoom_params.m_sUseBinocularVision);

	if (pA && IsScopeAttached())
	{
		if (psActorFlags.test(AF_PNV_W_SCOPE_DIS) && bIsSecondVPZoomPresent())
		{
			if (pA->GetNightVisionStatus())
			{
				OnZoomOut();
			}
		}
		else if (m_zoom_params.m_sUseZoomPostprocess.size())
		{
			if (NULL == m_zoom_params.m_pNight_vision)
			{
				m_zoom_params.m_pNight_vision = xr_new<CNightVisionEffector>(m_zoom_params.m_sUseZoomPostprocess);
			}
		}
	}
	g_player_hud->updateMovementLayerState();
}

void CWeapon::OnZoomOut()
{
	//Alun: Switch back to third-person if was forced
	if (m_freelook_switch_back)
	{
		CActor *pA = smart_cast<CActor *>(H_Parent());
		if (pA)
			pA->cam_Set(eacLookAt);
		m_freelook_switch_back = false;
	}

	if (!bIsSecondVPZoomPresent() || !psActorFlags.test(AF_3DSCOPE_ENABLE))
		m_fRTZoomFactor = GetZoomFactor(); //  
	m_zoom_params.m_bIsZoomModeNow = false;
	SetZoomFactor(GameConstants::GetOGSE_WpnZoomSystem() ? 1.f : g_fov);
	//EnableHudInertion					(TRUE);

// 	GamePersistent().RestoreEffectorDOF	();

	if (GetHUDmode())
	{
		psHUD_FOV_def = last_hud_fov;
		GamePersistent().SetPickableEffectorDOF(false);
		UpdateAimOffsets();

		if (m_BlendAimEndCam.name.size())
		{
			if (m_BlendAimIdleCam.name.size())
				g_player_hud->StopBlendAnm(m_BlendAimIdleCam.name.c_str());

			g_player_hud->PlayBlendAnm(m_BlendAimEndCam.name.c_str(), 2, m_BlendAimEndCam.speed, m_BlendAimEndCam.power, false, false);
		}
	}

	ResetSubStateTime					();

	xr_delete							(m_zoom_params.m_pVision);
	if(m_zoom_params.m_pNight_vision)
	{
		m_zoom_params.m_pNight_vision->StopForScope(100000.0f, false);
		xr_delete(m_zoom_params.m_pNight_vision);
	}
	g_player_hud->updateMovementLayerState();
}

CUIWindow* CWeapon::ZoomTexture()
{
	if (UseScopeTexture() && !bIsSecondVPZoomPresent() && !m_bAltZoomActive)
		return m_UIScope;
	else
		return NULL;
}

void CWeapon::SwitchState(u32 S)
{
	if (OnClient()) return;

#ifndef MASTER_GOLD
	if ( bDebug )
	{
		Msg("---Server is going to send GE_WPN_STATE_CHANGE to [%d], weapon_section[%s], parent[%s]",
			S, cNameSect().c_str(), H_Parent() ? H_Parent()->cName().c_str() : "NULL Parent");
	}
#endif // #ifndef MASTER_GOLD

	SetNextState		( S );
	if (CHudItem::object().Local() && !CHudItem::object().getDestroy() && m_pInventory && OnServer())	
	{
		// !!! Just single entry for given state !!!
		NET_Packet		P;
		CHudItem::object().u_EventGen		(P,GE_WPN_STATE_CHANGE,CHudItem::object().ID());
		P.w_u8			(u8(S));
		P.w_u8			(u8(m_sub_state));
		P.w_u8			(u8(m_ammoType& 0xff));
		P.w_u8			(u8(iAmmoElapsed & 0xff));
		P.w_u8			(u8(m_set_next_ammoType_on_reload & 0xff));
		CHudItem::object().u_EventSend		(P, net_flags(TRUE, TRUE, FALSE, TRUE));
	}
}

void CWeapon::OnMagazineEmpty	()
{
	VERIFY((u32)iAmmoElapsed == m_magazine.size());
}


void CWeapon::reinit			()
{
	CShootingObject::reinit		();
	CHudItemObject::reinit			();
}

void CWeapon::reload			(LPCSTR section)
{
	CShootingObject::reload		(section);
	CHudItemObject::reload			(section);
	
	m_can_be_strapped			= true;
	m_can_be_strapped_rifle = (GetSlot() == RIFLE_SLOT);
	m_strapped_mode				= false;
	m_strapped_mode_rifle = false;

	if (pSettings->line_exist(section,"strap_bone0"))
		m_strap_bone0			= pSettings->r_string(section,"strap_bone0");
	else {
		m_can_be_strapped		= false;
		m_can_be_strapped_rifle = false;
	}

	if (pSettings->line_exist(section,"strap_bone1"))
		m_strap_bone1			= pSettings->r_string(section,"strap_bone1");
	else {
		m_can_be_strapped = false;
		m_can_be_strapped_rifle = false;
	}

	bUseAltScope = !!bReloadSectionScope(section);

	if (m_eScopeStatus == ALife::eAddonAttachable)
	{
		m_addon_holder_range_modifier	= READ_IF_EXISTS(pSettings,r_float,GetScopeName(),"holder_range_modifier",m_holder_range_modifier);
		m_addon_holder_fov_modifier		= READ_IF_EXISTS(pSettings,r_float,GetScopeName(),"holder_fov_modifier",m_holder_fov_modifier);
	}
	else
	{
		m_addon_holder_range_modifier	= m_holder_range_modifier;
		m_addon_holder_fov_modifier		= m_holder_fov_modifier;
	}


	{
		Fvector				pos,ypr;
		pos					= pSettings->r_fvector3		(section,"position");
		ypr					= pSettings->r_fvector3		(section,"orientation");
		ypr.mul				(PI/180.f);

		m_Offset.setHPB			(ypr.x,ypr.y,ypr.z);
		m_Offset.translate_over	(pos);
	}

	m_StrapOffset			= m_Offset;
	if (pSettings->line_exist(section,"strap_position") && pSettings->line_exist(section,"strap_orientation")) {
		Fvector				pos,ypr;
		pos					= pSettings->r_fvector3		(section,"strap_position");
		ypr					= pSettings->r_fvector3		(section,"strap_orientation");
		ypr.mul				(PI/180.f);

		m_StrapOffset.setHPB			(ypr.x,ypr.y,ypr.z);
		m_StrapOffset.translate_over	(pos);
	}
	else {
		m_can_be_strapped = false;
		m_can_be_strapped_rifle = false;
	}

	m_ef_main_weapon_type	= READ_IF_EXISTS(pSettings,r_u32,section,"ef_main_weapon_type",u32(-1));
	m_ef_weapon_type		= READ_IF_EXISTS(pSettings,r_u32,section,"ef_weapon_type",u32(-1));
}

void CWeapon::create_physic_shell()
{
	CPhysicsShellHolder::create_physic_shell();
}

bool CWeapon::ActivationSpeedOverriden (Fvector& dest, bool clear_override)
{
	if ( m_activation_speed_is_overriden )
	{
		if ( clear_override )
		{
			m_activation_speed_is_overriden	=	false;
		}

		dest						=	m_overriden_activation_speed;
		return							true;
	}
	
	return								false;
}

void CWeapon::SetActivationSpeedOverride	(Fvector const& speed)
{
	m_overriden_activation_speed	=	speed;
	m_activation_speed_is_overriden	=	true;
}

void CWeapon::activate_physic_shell()
{
	UpdateXForm();
	CPhysicsShellHolder::activate_physic_shell();
}

void CWeapon::setup_physic_shell()
{
	CPhysicsShellHolder::setup_physic_shell();
}

int		g_iWeaponRemove = 1;

bool CWeapon::NeedToDestroyObject()	const
{
	if (GameID() == eGameIDSingle) return false;
	if (Remote()) return false;
	if (H_Parent()) return false;
	if (g_iWeaponRemove == -1) return false;
	if (g_iWeaponRemove == 0) return true;
	if (TimePassedAfterIndependant() > m_dwWeaponRemoveTime)
		return true;

	return false;
}

ALife::_TIME_ID	 CWeapon::TimePassedAfterIndependant()	const
{
	if(!H_Parent() && m_dwWeaponIndependencyTime != 0)
		return Level().timeServer() - m_dwWeaponIndependencyTime;
	else
		return 0;
}

bool CWeapon::can_kill	() const
{
	if (GetSuitableAmmoTotal(true) || m_ammoTypes.empty())
		return				(true);

	return					(false);
}

CInventoryItem *CWeapon::can_kill	(CInventory *inventory) const
{
	if (GetAmmoElapsed() || m_ammoTypes.empty())
		return				(const_cast<CWeapon*>(this));

	TIItemContainer::iterator I = inventory->m_all.begin();
	TIItemContainer::iterator E = inventory->m_all.end();
	for ( ; I != E; ++I) {
		CInventoryItem	*inventory_item = smart_cast<CInventoryItem*>(*I);
		if (!inventory_item)
			continue;
		
		xr_vector<shared_str>::const_iterator	i = std::find(m_ammoTypes.begin(),m_ammoTypes.end(),inventory_item->object().cNameSect());
		if (i != m_ammoTypes.end())
			return			(inventory_item);
	}

	return					(0);
}

const CInventoryItem *CWeapon::can_kill	(const xr_vector<const CGameObject*> &items) const
{
	if (m_ammoTypes.empty())
		return				(this);

	xr_vector<const CGameObject*>::const_iterator I = items.begin();
	xr_vector<const CGameObject*>::const_iterator E = items.end();
	for ( ; I != E; ++I) {
		const CInventoryItem	*inventory_item = smart_cast<const CInventoryItem*>(*I);
		if (!inventory_item)
			continue;

		xr_vector<shared_str>::const_iterator	i = std::find(m_ammoTypes.begin(),m_ammoTypes.end(),inventory_item->object().cNameSect());
		if (i != m_ammoTypes.end())
			return			(inventory_item);
	}

	return					(0);
}

bool CWeapon::ready_to_kill	() const
{
	return					(
		!IsMisfire() && 
		((GetState() == eIdle) || (GetState() == eFire) || (GetState() == eFire2)) && 
		GetAmmoElapsed()
	);
}

void _inertion(float& _val_cur, const float& _val_trgt, const float& _friction)
{
	float friction_i = 1.f - _friction;
	_val_cur = _val_cur * _friction + _val_trgt * friction_i;
}

float _lerp(const float& _val_a, const float& _val_b, const float& _factor)
{
	return (_val_a * (1.0 - _factor)) + (_val_b * _factor);
}

static BOOL pick_trace_callback(collide::rq_result& result, LPVOID params)
{
	collide::rq_result* RQ = (collide::rq_result*)params;
	if (!result.O)
	{
		// получить треугольник и узнать его материал
		CDB::TRI* T = Level().ObjectSpace.GetStaticTris() + result.element;
		if (T->material < GMLib.CountMaterial())
		{
			if (GMLib.GetMaterialByIdx(T->material)->Flags.is(SGameMtl::flPassable) || GMLib.GetMaterialByIdx(T->material)->Flags.is(SGameMtl::flActorObstacle))
				return TRUE;
		}
	}
	*RQ = result;
	return FALSE;
}

static float GetRayQueryDist()
{
	collide::rq_result RQ;
	g_pGameLevel->ObjectSpace.RayPick(Device.vCameraPosition, Device.vCameraDirection, 3.0f, collide::rqtStatic, RQ, Actor());
	if (!RQ.O)
	{
		CDB::TRI* T = Level().ObjectSpace.GetStaticTris() + RQ.element;
		if (T->material < GMLib.CountMaterial())
		{
			collide::rq_result  RQ2;
			collide::rq_results RQR;
			RQ2.range = 3.0f;
			collide::ray_defs RD(Device.vCameraPosition, Device.vCameraDirection, RQ2.range, CDB::OPT_CULL, collide::rqtStatic);
			if (Level().ObjectSpace.RayQuery(RQR, RD, pick_trace_callback, &RQ2, NULL, Level().CurrentEntity()))
			{
				clamp(RQ2.range, RQ.range, RQ2.range);
				return RQ2.range;
			}
		}
	}
	return RQ.range;
}

void CWeapon::UpdateHudAdditional(Fmatrix& trans)
{
	CActor* pActor = smart_cast<CActor*>(H_Parent());
	if (!pActor)
		return;

	Fvector summary_offset{}, summary_rotate{};

	attachable_hud_item* hi = HudItemData();
	R_ASSERT(hi);

	u8 idx = GetCurrentHudOffsetIdx();
	Fvector						curr_offs, curr_rot;
	curr_offs					= hi->m_measures.m_hands_offset[0][idx]; //pos,aim
	curr_rot					= hi->m_measures.m_hands_offset[1][idx]; //rot,aim

	const bool b_aiming = ((IsZoomed() && m_zoom_params.m_fZoomRotationFactor <= 1.f) || (!IsZoomed() && m_zoom_params.m_fZoomRotationFactor > 0.f));
	//============ Поворот ствола во время аима ===========//
	if (b_aiming)
	{

		curr_offs.mul				(m_zoom_params.m_fZoomRotationFactor);
		curr_rot.mul				(m_zoom_params.m_fZoomRotationFactor);

		if (pActor->IsZoomAimingMode())
			m_zoom_params.m_fZoomRotationFactor += Device.fTimeDelta / m_zoom_params.m_fZoomRotateTime;
		else
			m_zoom_params.m_fZoomRotationFactor -= Device.fTimeDelta / m_zoom_params.m_fZoomRotateTime;

		clamp(m_zoom_params.m_fZoomRotationFactor, 0.f, 1.f);
		
		summary_offset.add(curr_offs);
	}

	//============= Коллизия оружия =============//
	if (b_hud_collision)
	{
		float dist = GetRayQueryDist();

		Fvector curr_offs, curr_rot;
		curr_offs = hi->m_measures.m_collision_offset[0];//pos,aim
		curr_rot = hi->m_measures.m_collision_offset[1];//rot,aim
		curr_offs.mul(m_fFactor);
		curr_rot.mul(m_fFactor);

		float m_fColPosition;
		float m_fColRotation;

		if (dist <= 0.8 && !IsZoomed())
		{
			m_fColPosition = curr_offs.y + ((1 - dist - 0.2) * 5.0f);
			m_fColRotation = curr_rot.x + ((1 - dist - 0.2) * 5.0f);
		}
		else
		{
			m_fColPosition = curr_offs.y;
			m_fColRotation = curr_rot.x;
		}

		if (m_fFactor < m_fColPosition)
		{
			m_fFactor += Device.fTimeDelta / 0.3;
			if (m_fFactor > m_fColPosition)
				m_fFactor = m_fColPosition;
		}
		else if (m_fFactor > m_fColPosition)
		{
			m_fFactor -= Device.fTimeDelta / 0.3;
			if (m_fFactor < m_fColPosition)
				m_fFactor = m_fColPosition;
		}

		clamp(m_fFactor, 0.f, 1.f);
		
		summary_offset.add(curr_offs);
	}
	else
	{
		m_fFactor = 0.0;
	}

	//============= Подготавливаем общие переменные =============//
	clamp(idx, u8(0), u8(1));
	bool bForAim = (idx == 1);

	float fInertiaPower = GetInertionPowerFactor();

	float fYMag = pActor->fFPCamYawMagnitude;
	float fPMag = pActor->fFPCamPitchMagnitude;

	static float fAvgTimeDelta = Device.fTimeDelta;
	_inertion(fAvgTimeDelta, Device.fTimeDelta, 0.8f);

	//======== Проверяем доступность инерции и стрейфа ========//
	if (!g_player_hud->inertion_allowed())
		return;

	//============= Боковой стрейф с оружием =============//
	// Рассчитываем фактор боковой ходьбы
	float fStrafeMaxTime = m_strafe_offset[2][idx].y; // Макс. время в секундах, за которое мы наклонимся из центрального положения
	if (fStrafeMaxTime <= EPS)
		fStrafeMaxTime = 0.01f;

	float fStepPerUpd = fAvgTimeDelta / fStrafeMaxTime; // Величина изменение фактора поворота

	// Добавляем боковой наклон от движения камеры
	float fCamReturnSpeedMod = 0.5f; // Восколько ускоряем нормализацию наклона, полученного от движения камеры (только от бедра)
	// Высчитываем минимальную скорость поворота камеры для начала инерции
	float fStrafeMinAngle = _lerp(
		m_strafe_offset[3][0].y,
		m_strafe_offset[3][1].y,
		m_zoom_params.m_fZoomRotationFactor);

	// Высчитываем мксимальный наклон от поворота камеры
	float fCamLimitBlend = _lerp(
		m_strafe_offset[3][0].x,
		m_strafe_offset[3][1].x,
		m_zoom_params.m_fZoomRotationFactor);

	// Считаем стрейф от поворота камеры
	if (abs(fYMag) > (m_fLR_CameraFactor == 0.0f ? fStrafeMinAngle : 0.0f))
	{ //--> Камера крутится по оси Y
		m_fLR_CameraFactor -= (fYMag * fAvgTimeDelta * 0.25f);
		clamp(m_fLR_CameraFactor, -fCamLimitBlend, fCamLimitBlend);
	}
	else
	{ //--> Камера не поворачивается - убираем наклон
		if (m_fLR_CameraFactor < 0.0f)
		{
			m_fLR_CameraFactor += fStepPerUpd * (bForAim ? 1.0f : fCamReturnSpeedMod);
			clamp(m_fLR_CameraFactor, -fCamLimitBlend, 0.0f);
		}
		else
		{
			m_fLR_CameraFactor -= fStepPerUpd * (bForAim ? 1.0f : fCamReturnSpeedMod);
			clamp(m_fLR_CameraFactor, 0.0f, fCamLimitBlend);
		}
	}
	// Добавляем боковой наклон от ходьбы вбок
	float fChangeDirSpeedMod = 1; // Восколько быстро меняем направление направление наклона, если оно в другую сторону от текущего

	u32 iMovingState = pActor->MovingState();
	if ((iMovingState & mcLStrafe) != 0)
	{ // Движемся влево
		float fVal = (m_fLR_MovingFactor > 0.f ? fStepPerUpd * fChangeDirSpeedMod : fStepPerUpd);
		m_fLR_MovingFactor -= fVal;
	}
	else if ((iMovingState & mcRStrafe) != 0)
	{ // Движемся вправо
		float fVal = (m_fLR_MovingFactor < 0.f ? fStepPerUpd * fChangeDirSpeedMod : fStepPerUpd);
		m_fLR_MovingFactor += fVal;
	}
	else
	{ // Двигаемся в любом другом направлении - плавно убираем наклон
		if (m_fLR_MovingFactor < 0.0f)
		{
			m_fLR_MovingFactor += fStepPerUpd;
			clamp(m_fLR_MovingFactor, -1.0f, 0.0f);
		}
		else
		{
			m_fLR_MovingFactor -= fStepPerUpd;
			clamp(m_fLR_MovingFactor, 0.0f, 1.0f);
		}
	}

	clamp(m_fLR_MovingFactor, -1.0f, 1.0f); // Фактор боковой ходьбы не должен превышать эти лимиты

	// Вычисляем и нормализируем итоговый фактор наклона
	float fLR_Factor = m_fLR_MovingFactor + (m_fLR_CameraFactor * fInertiaPower);
	clamp(fLR_Factor, -1.0f, 1.0f); // Фактор боковой ходьбы не должен превышать эти лимиты

	// Производим наклон ствола для нормального режима и аима
	for (int _idx = 0; _idx <= 1; _idx++)//<-- Для плавного перехода
	{
		bool bEnabled = (m_strafe_offset[2][_idx].x != 0.0f);
		if (!bEnabled)
			goto WALK_FORWARD_EFFECTS;

		Fvector moving_offs, moving_rot;

		// Смещение позиции худа в стрейфе
		moving_offs = m_strafe_offset[0][_idx]; //pos
		moving_offs.mul(fLR_Factor);                   // Умножаем на фактор стрейфа

		// Поворот худа в стрейфе
		moving_rot = m_strafe_offset[1][_idx]; //rot
		moving_rot.mul(-PI / 180.f);                          // Преобразуем углы в радианы
		moving_rot.mul(fLR_Factor);                   // Умножаем на фактор стрейфа

		// Мягкий переход между бедром \ прицелом
		if (_idx == 0)
		{ // От бедра
			moving_offs.mul(1.f - m_zoom_params.m_fZoomRotationFactor);
			moving_rot.mul(1.f - m_zoom_params.m_fZoomRotationFactor);
		}
		else
		{ // Во время аима
			moving_offs.mul(m_zoom_params.m_fZoomRotationFactor);
			moving_rot.mul(m_zoom_params.m_fZoomRotationFactor);
		}

		
		summary_offset.add	(moving_offs);
		summary_rotate.add	(moving_rot);
	}
    //====================================================//


WALK_FORWARD_EFFECTS:
    //=============== Эффекты ходьбы ===============//
	{	// - carнадо будет 
		const float m_fWalkUpdtime = Device.fTimeDelta / m_fWalkMaxTime;
		m_fWalkEffectSideTimer += m_fWalkUpdtime;
		Fvector walk_effect_pos = m_walk_effect[0], walk_effect_rot = m_walk_effect[1];

		if (pActor->is_actor_running() && !(b_aiming))
		{
			m_fWalkEffectSetFactor += m_fWalkUpdtime;
			clamp(m_fWalkEffectSetFactor, 0.0f, 1.0f);
		}
		else{
			if (m_fWalkEffectSetFactor < 0.0f)
			{
				m_fWalkEffectSetFactor += m_fWalkUpdtime;
				clamp(m_fWalkEffectSetFactor, -1.0f, 0.0f);
			}
			else{
				m_fWalkEffectSetFactor -= m_fWalkUpdtime;
				clamp(m_fWalkEffectSetFactor, 0.0f, 1.0f);
			}
		}

		if (m_fWalkEffectSideTimer >= m_fWalkEffectRestoreFactor*m_fWalkMaxTime)
			m_fWalkEffectSideTimer = 0;

		walk_effect_pos.mul(m_fWalkEffectSetFactor);
		walk_effect_rot.mul(-PI / 180.f);
		walk_effect_rot.mul(m_fWalkEffectSetFactor);

		summary_offset.add(walk_effect_pos);
		summary_rotate.add(walk_effect_rot);

		//goto LOOKOUT_EFFECT;
	}

	//====================================================//

LOOKOUT_EFFECT:
	//=============== Эффекты выглядываний ===============//
	{
		const bool bEnabled = m_lookout_offset[2][idx].x;
		if (!bEnabled)
			goto JUMP_EFFECT;

		float fLookoutMaxTime = m_lookout_offset[2][idx].y; // Макс. время в секундах, за которое мы наклонимся из центрального положения
		if (fLookoutMaxTime <= EPS)
			fLookoutMaxTime = 0.01f;

		const float fStepPerUpdL = Device.fTimeDelta / fLookoutMaxTime; // Величина изменение фактора поворота

		if ((iMovingState & mcLLookout) && !(iMovingState & mcRLookout))
		{ // Выглядываем влево
			float fVal = (m_fLookout_MovingFactor > 0.f ? fStepPerUpdL * 3 : fStepPerUpdL);
			m_fLookout_MovingFactor -= fVal;
		}
		else if ((iMovingState & mcRLookout) && !(iMovingState & mcLLookout))
		{ // Выглядываем вправо
			float fVal = (m_fLookout_MovingFactor < 0.f ? fStepPerUpdL * 3 : fStepPerUpdL);
			m_fLookout_MovingFactor += fVal;
		}
		else
		{ // Двигаемся в любом другом направлении
			if (m_fLookout_MovingFactor < 0.0f)
			{
				m_fLookout_MovingFactor += fStepPerUpdL;
				clamp(m_fLookout_MovingFactor, -1.0f, 0.0f);
			}
			else
			{
				m_fLookout_MovingFactor -= fStepPerUpdL;
				clamp(m_fLookout_MovingFactor, 0.0f, 1.0f);
			}
		}

		clamp(m_fLookout_MovingFactor, -1.0f, 1.0f); // не должен превышать эти лимиты

		float koef{ 1.f };
		if ((iMovingState & mcCrouch) && (iMovingState & mcAccel))
			koef = 0.5; // во сколько раз менять амплитуду при полном присяде
		else if (iMovingState & mcCrouch)
			koef = 0.75; // во сколько раз менять амплитуду при присяде

		for (int _idx = 0; _idx <= 1; _idx++)//<-- Для плавного перехода
		{
			// Смещение позиции худа
			Fvector lookout_offs = m_lookout_offset[0][_idx]; //pos
			lookout_offs.mul(koef);
			lookout_offs.mul(m_fLookout_MovingFactor); // Умножаем на фактор наклона

			// Поворот худа
			Fvector lookout_rot = m_lookout_offset[1][_idx]; //rot
			lookout_rot.mul(koef);
			lookout_rot.mul(-PI / 180.f); // Преобразуем углы в радианы
			lookout_rot.mul(m_fLookout_MovingFactor); // Умножаем на фактор наклона

			if (_idx == 0)
			{ // От бедра
				lookout_offs.mul(1.f - m_zoom_params.m_fZoomRotationFactor);
				lookout_rot.mul(1.f - m_zoom_params.m_fZoomRotationFactor);
			}
			else
			{ // Во время аима
				lookout_offs.mul(m_zoom_params.m_fZoomRotationFactor);
				lookout_rot.mul(m_zoom_params.m_fZoomRotationFactor);
			}

			summary_offset.add(lookout_offs);
			summary_rotate.add(lookout_rot);
		}
	}
	//====================================================//

JUMP_EFFECT:
    //=============== Эффекты прыжка ===============//
    {
        const bool bEnabled = m_jump_offset[2][idx].x;
        if (!bEnabled)
            goto INERTION_EFFECT;

        float fJumpMaxTime
            = m_jump_offset[2][idx].y; // Макс. время в секундах, за которое произойдет смещение худа при прыжке
        float fFallMaxTime
            = m_fall_offset[2][idx].y; // Макс. время в секундах, за которое произойдет смещение худа при падении

        if (fJumpMaxTime <= EPS)
            fJumpMaxTime = 0.01f;

        if (fFallMaxTime <= EPS)
            fFallMaxTime = 0.01f;

        const float fJumpPerUpd = Device.fTimeDelta / fJumpMaxTime; // Величина изменение фактора смещения худа при прыжке
        const float fFallPerUpd = Device.fTimeDelta / fFallMaxTime; // Величина изменение фактора смещения худа при падении

        if (iMovingState & mcJump && m_fJump_MovingFactor < 1.f)
        { // Прыжок
            m_fJump_MovingFactor += fJumpPerUpd;
            m_fFall_MovingFactor -= fFallPerUpd;
        }
        else if (iMovingState & mcFall && m_fJump_MovingFactor > 0.f)
        { // Падание
            m_fJump_MovingFactor -= fJumpPerUpd;
            m_fFall_MovingFactor += fFallPerUpd;
        }
        else
        { // Двигаемся в любом другом направлении
            if (m_fJump_MovingFactor < 0.0f && m_fFall_MovingFactor < 0.0f)
            {
                m_fJump_MovingFactor += fJumpPerUpd;
                m_fFall_MovingFactor += fFallPerUpd;
            }
            else
            {
                m_fJump_MovingFactor -= fJumpPerUpd;
                m_fFall_MovingFactor -= fFallPerUpd;
            }
        }

        clamp(m_fJump_MovingFactor, 0.0f, 1.0f); // не должен превышать эти лимиты
        clamp(m_fFall_MovingFactor, 0.0f, 1.0f); // не должен превышать эти лимиты

		for (int _idx = 0; _idx <= 1; _idx++)//<-- Для плавного перехода
		{
	        // Смещение позиции худа в стрейфе
	        Fvector jump_offs = m_jump_offset[0][_idx]; // pos
	        jump_offs.mul(m_fJump_MovingFactor); // Умножаем на фактор эффекта

	        // Поворот худа в стрейфе
	        Fvector jump_rot = m_jump_offset[1][_idx]; // rot
	        jump_rot.mul(-PI / 180.f); // Преобразуем углы в радианы
	        jump_rot.mul(m_fJump_MovingFactor); // Умножаем на фактор эффекта

	        // Смещение позиции худа в стрейфе
	        Fvector fall_offs = m_fall_offset[0][_idx]; // pos
	        fall_offs.mul(m_fFall_MovingFactor); // Умножаем на фактор эффекта

	        // Поворот худа в стрейфе
	        Fvector fall_rot = m_fall_offset[1][_idx]; // rot
	        fall_rot.mul(-PI / 180.f); // Преобразуем углы в радианы
	        fall_rot.mul(m_fFall_MovingFactor); // Умножаем на фактор эффекта

	        if (_idx == 0) { // От бедра
	            jump_offs.mul(1.f - m_zoom_params.m_fZoomRotationFactor);
	            jump_rot.mul(1.f - m_zoom_params.m_fZoomRotationFactor);
	            fall_offs.mul(1.f - m_zoom_params.m_fZoomRotationFactor);
	            fall_rot.mul(1.f - m_zoom_params.m_fZoomRotationFactor);
	        } else { // Во время аима
	            jump_offs.mul(m_zoom_params.m_fZoomRotationFactor);
	            jump_rot.mul(m_zoom_params.m_fZoomRotationFactor);
	            fall_offs.mul(m_zoom_params.m_fZoomRotationFactor);
	            fall_rot.mul(m_zoom_params.m_fZoomRotationFactor);
	        }

			summary_offset.add(jump_offs + fall_offs);
			summary_rotate.add(jump_rot + fall_rot);
		}
    }
    //====================================================//



INERTION_EFFECT:
	//============= Инерция оружия =============//
	{
	   // Параметры инерции
		float fInertiaSpeedMod = _lerp(
			hi->m_measures.m_inertion_params.m_tendto_speed,
			hi->m_measures.m_inertion_params.m_tendto_speed_aim,
			m_zoom_params.m_fZoomRotationFactor);

		float fInertiaReturnSpeedMod = _lerp(
			hi->m_measures.m_inertion_params.m_tendto_ret_speed,
			hi->m_measures.m_inertion_params.m_tendto_ret_speed_aim,
			m_zoom_params.m_fZoomRotationFactor);

		float fInertiaMinAngle = _lerp(
			hi->m_measures.m_inertion_params.m_min_angle,
			hi->m_measures.m_inertion_params.m_min_angle_aim,
			m_zoom_params.m_fZoomRotationFactor);

		Fvector4 vIOffsets; // x = L, y = R, z = U, w = D
		vIOffsets.x = _lerp(
			hi->m_measures.m_inertion_params.m_offset_LRUD.x,
			hi->m_measures.m_inertion_params.m_offset_LRUD_aim.x,
			m_zoom_params.m_fZoomRotationFactor) * fInertiaPower;
		vIOffsets.y = _lerp(
			hi->m_measures.m_inertion_params.m_offset_LRUD.y,
			hi->m_measures.m_inertion_params.m_offset_LRUD_aim.y,
			m_zoom_params.m_fZoomRotationFactor) * fInertiaPower;
		vIOffsets.z = _lerp(
			hi->m_measures.m_inertion_params.m_offset_LRUD.z,
			hi->m_measures.m_inertion_params.m_offset_LRUD_aim.z,
			m_zoom_params.m_fZoomRotationFactor) * fInertiaPower;
		vIOffsets.w = _lerp(
			hi->m_measures.m_inertion_params.m_offset_LRUD.w,
			hi->m_measures.m_inertion_params.m_offset_LRUD_aim.w,
			m_zoom_params.m_fZoomRotationFactor) * fInertiaPower;

		// Высчитываем инерцию из поворотов камеры
		bool bIsInertionPresent = m_fLR_InertiaFactor != 0.0f || m_fUD_InertiaFactor != 0.0f;
		if (abs(fYMag) > fInertiaMinAngle || bIsInertionPresent)
		{
			float fSpeed = fInertiaSpeedMod;
			if (fYMag > 0.0f && m_fLR_InertiaFactor > 0.0f ||
				fYMag < 0.0f && m_fLR_InertiaFactor < 0.0f)
			{
				fSpeed *= 1.f; //--> Ускоряем инерцию при движении в противоположную сторону
			}

			m_fLR_InertiaFactor -= (fYMag * fAvgTimeDelta * fSpeed); // Горизонталь (м.б. > |1.0|)
		}

		if (abs(fPMag) > fInertiaMinAngle || bIsInertionPresent)
		{
			float fSpeed = fInertiaSpeedMod;
			if (fPMag > 0.0f && m_fUD_InertiaFactor > 0.0f ||
				fPMag < 0.0f && m_fUD_InertiaFactor < 0.0f)
			{
				fSpeed *= 2.f; //--> Ускоряем инерцию при движении в противоположную сторону
			}

			m_fUD_InertiaFactor -= (fPMag * fAvgTimeDelta * fSpeed); // Вертикаль (м.б. > |1.0|)
		}

		clamp(m_fLR_InertiaFactor, -1.0f, 1.0f);
		clamp(m_fUD_InertiaFactor, -1.0f, 1.0f);

		// Плавное затухание инерции (основное, но без линейной никогда не опустит инерцию до полного 0.0f)
		m_fLR_InertiaFactor *= clampr(1.f - fAvgTimeDelta * fInertiaReturnSpeedMod, 0.0f, 1.0f);
		m_fUD_InertiaFactor *= clampr(1.f - fAvgTimeDelta * fInertiaReturnSpeedMod, 0.0f, 1.0f);

		// Минимальное линейное затухание инерции при покое (горизонталь)
		if (fYMag == 0.0f)
		{
			float fRetSpeedMod = (fYMag == 0.0f ? 1.0f : 0.75f) * (fInertiaReturnSpeedMod * 0.075f);
			if (m_fLR_InertiaFactor < 0.0f)
			{
				m_fLR_InertiaFactor += fAvgTimeDelta * fRetSpeedMod;
				clamp(m_fLR_InertiaFactor, -1.0f, 0.0f);
			}
			else
			{
				m_fLR_InertiaFactor -= fAvgTimeDelta * fRetSpeedMod;
				clamp(m_fLR_InertiaFactor, 0.0f, 1.0f);
			}
		}

		// Минимальное линейное затухание инерции при покое (вертикаль)
		if (fPMag == 0.0f)
		{
			float fRetSpeedMod = (fPMag == 0.0f ? 1.0f : 0.75f) * (fInertiaReturnSpeedMod * 0.075f);
			if (m_fUD_InertiaFactor < 0.0f)
			{
				m_fUD_InertiaFactor += fAvgTimeDelta * fRetSpeedMod;
				clamp(m_fUD_InertiaFactor, -1.0f, 0.0f);
			}
			else
			{
				m_fUD_InertiaFactor -= fAvgTimeDelta * fRetSpeedMod;
				clamp(m_fUD_InertiaFactor, 0.0f, 1.0f);
			}
		}

		// Применяем инерцию к худу
		float fLR_lim = (m_fLR_InertiaFactor < 0.0f ? vIOffsets.x : vIOffsets.y);
		float fUD_lim = (m_fUD_InertiaFactor < 0.0f ? vIOffsets.z : vIOffsets.w);

		Fvector inertion_offs;
		inertion_offs = { fLR_lim * -1.f * m_fLR_InertiaFactor, fUD_lim * m_fUD_InertiaFactor, 0.0f };

        summary_offset.add(inertion_offs);

		//goto SHOOTING_EFFECT;
    }
    //====================================================//

SHOOTING_EFFECT:
	//============= Сдвиг оружия при стрельбе =============//
	// Параметры сдвига
	//--> Длительность (== скорость) затухания эффекта ...
	float fShootingStabilizeTime = (GetState() == eFire ?
		//--> ... во время анимации стрельбы
		_lerp(hi->m_measures.m_shooting_params.m_ret_time_fire[0], //--> от бедра
			hi->m_measures.m_shooting_params.m_ret_time_fire[1], //--> в зуме
			m_zoom_params.m_fZoomRotationFactor) :
		//--> ... после анимации стрельбы
		_lerp(hi->m_measures.m_shooting_params.m_ret_time[0], //--> от бедра
			hi->m_measures.m_shooting_params.m_ret_time[1], //--> в зуме
			m_zoom_params.m_fZoomRotationFactor));

	//--> Сдвиг по Z
	float fShootingBackwOffset = _lerp(
		hi->m_measures.m_shooting_params.m_shot_offset_BACKW, //--> от бедра
		hi->m_measures.m_shooting_params.m_shot_offset_BACKW_aim, //--> в зуме
		m_zoom_params.m_fZoomRotationFactor);
  		

	//--> Сдвиг по X, Y
	Fvector4 vShOffsets; // 0 = -X, 1 = +X, 2 = +Y, 3 = -Y
	vShOffsets[0] = _lerp(
		hi->m_measures.m_shooting_params.m_shot_max_offset_LRUD[0], //--> от бедра
		hi->m_measures.m_shooting_params.m_shot_max_offset_LRUD_aim[0], //--> в зуме
		m_zoom_params.m_fZoomRotationFactor);
	vShOffsets[1] = _lerp(
		hi->m_measures.m_shooting_params.m_shot_max_offset_LRUD[1], //--> от бедра
		hi->m_measures.m_shooting_params.m_shot_max_offset_LRUD_aim[1], //--> в зуме
		m_zoom_params.m_fZoomRotationFactor);
	vShOffsets[2] = _lerp(
		hi->m_measures.m_shooting_params.m_shot_max_offset_LRUD[2], //--> от бедра
		hi->m_measures.m_shooting_params.m_shot_max_offset_LRUD_aim[2], //--> в зуме
		m_zoom_params.m_fZoomRotationFactor);
	vShOffsets[3] = _lerp(
		hi->m_measures.m_shooting_params.m_shot_max_offset_LRUD[3], //--> от бедра
		hi->m_measures.m_shooting_params.m_shot_max_offset_LRUD_aim[3], //--> в зуме
		m_zoom_params.m_fZoomRotationFactor);

	//--> Поворот по оси X
	Fvector4 vShRotX; // 0 = При смещении вверх, 1 = при смещении вниз
	vShRotX[0] = _lerp(
		hi->m_measures.m_shooting_params.m_shot_max_rot_UD[0], //--> от бедра
		hi->m_measures.m_shooting_params.m_shot_max_rot_UD_aim[0], //--> в зуме
		m_zoom_params.m_fZoomRotationFactor);
	vShRotX[1] = _lerp(
		hi->m_measures.m_shooting_params.m_shot_max_rot_UD[1], //--> от бедра
		hi->m_measures.m_shooting_params.m_shot_max_rot_UD_aim[1], //--> в зуме
		m_zoom_params.m_fZoomRotationFactor);

	// Применяем сдвиг от стрельбы к HUD-у
	{
		Fvector shoot_offs, shoot_rot;

		//--> Рассчитываем основу сдвига
		shoot_offs = {
			//--> Горизонтальный сдвиг
			_lerp(vShOffsets[0], vShOffsets[1], m_fShootingFactorLR) * m_fShootingCurPowerLRUD,
			//--> Вертикальный сдвиг
			_lerp(vShOffsets[2], vShOffsets[3], m_fShootingFactorUD) * m_fShootingCurPowerLRUD,
			//--> Глубинный сдвиг
			-1.f * fShootingBackwOffset * m_fShootingCurPowerBACKW
		};
		shoot_rot = {
			//--> Поворот по вертикали
			_lerp(vShRotX[0], vShRotX[1], m_fShootingFactorUD) * m_fShootingCurPowerLRUD,
			0.0f,
			0.0f
		};
		shoot_rot.mul(-PI / 180.f);

		//--> Модифицируем её коэфицентом силы сдвига
		float fShootingKoef = GetShootingEffectKoef();
		shoot_offs.mul(fShootingKoef);
		shoot_rot.mul(fShootingKoef);


		//--> Применяем к HUD-у
		summary_offset.add(shoot_offs);
		summary_rotate.add(shoot_rot);
	}

	// Плавное затухание сдвига от стрельбы
	//--> Глубинный сдвиг
	float fBackwStabilizeTimeKoef = hi->m_measures.m_shooting_params.m_ret_time_backw_koef;
	m_fShootingCurPowerBACKW -= Device.fTimeDelta / (fShootingStabilizeTime * fBackwStabilizeTimeKoef);
	clamp(m_fShootingCurPowerBACKW, 0.0f, 1.0f);

	//--> Боковой сдвиг
	m_fShootingCurPowerLRUD -= Device.fTimeDelta / fShootingStabilizeTime;
	if (m_fShootingCurPowerLRUD <= 0.0f)
	{
		ResetShootingEffect(true);
	}


	//====================================================//

APPLY_EFFECTS:
	//================ Применение эффектов ===============//
	{
		// поворот с сохранением смещения by Zander
		Fvector _angle{}, _pos{ trans.c };
		trans.getHPB(_angle);
		_angle.add(-summary_rotate);
		trans.setHPB(_angle.x, _angle.y, _angle.z);
		trans.c = _pos;

		Fmatrix hud_rotation;
		hud_rotation.identity();

		if (b_aiming)
		{
			hud_rotation.rotateX(curr_rot.x);

			Fmatrix hud_rotation_y;
			hud_rotation_y.identity();
			hud_rotation_y.rotateY(curr_rot.y);
			hud_rotation.mulA_43(hud_rotation_y);

			hud_rotation_y.identity();
			hud_rotation_y.rotateZ(curr_rot.z);
			hud_rotation.mulA_43(hud_rotation_y);
		}
		hud_rotation.translate_over(summary_offset);
		trans.mulB_43(hud_rotation);
	}
	//====================================================//
}

// Добавить HUD-эффект сдвига оружия от выстрела
void CWeapon::AddHUDShootingEffect()
{
	if (IsHidden() || ParentIsActor() == false)
		return;

	attachable_hud_item* hi = HudItemData();
	if (hi == nullptr)
		return;

	// Отдача назад всегда максимальная на каждом выстреле
	m_fShootingCurPowerBACKW = 1.0f;

	// Отдача в бока становится сильнее при длительной стрельбе
	//--> Регулируем плавность перемещения оружия по экрану, ограничивая макс. сдвиг на каждый выстрел
	float fLRUDDiffPerShot = _lerp(hi->m_measures.m_shooting_params.m_shot_diff_per_shot[0],
		hi->m_measures.m_shooting_params.m_shot_diff_per_shot[1], m_zoom_params.m_fZoomRotationFactor);
	clamp(fLRUDDiffPerShot, 0.0f, 1.0f);

	m_fShootingFactorLR += ::Random.randF(-fLRUDDiffPerShot, fLRUDDiffPerShot); //--> m_fShootingFactorLR будет 0.5f на момент начала стрельбы
	clamp(m_fShootingFactorLR, 0.0f, 1.0f);

	m_fShootingFactorUD += ::Random.randF(-fLRUDDiffPerShot, fLRUDDiffPerShot); //--> m_fShootingFactorUD будет 0.5f на момент начала стрельбы
	clamp(m_fShootingFactorUD, 0.0f, 1.0f);

	//--> С каждым выстрелом разрешаем оружию всё ближе приближаться к текущим границам сдвига
	m_fShootingCurPowerLRUD += _lerp(hi->m_measures.m_shooting_params.m_shot_power_per_shot[0],
		hi->m_measures.m_shooting_params.m_shot_power_per_shot[1], m_zoom_params.m_fZoomRotationFactor);
	clamp(m_fShootingCurPowerLRUD, 0.0f, 1.0f);

	// Наклон ствола от стрельбы (стрейф)
	float fShotStrafeMin = _lerp(hi->m_measures.m_shooting_params.m_shot_offsets_strafe[0],
		hi->m_measures.m_shooting_params.m_shot_offsets_strafe_aim[0], m_zoom_params.m_fZoomRotationFactor);
	float fShotStrafeMax = _lerp(hi->m_measures.m_shooting_params.m_shot_offsets_strafe[1],
		hi->m_measures.m_shooting_params.m_shot_offsets_strafe_aim[1], m_zoom_params.m_fZoomRotationFactor);
    
	float fLRShotStrafeDir = (::Random.randF(-1.0f, 1.0f) >= 0.0f ? 1.0f : -1.0f);
	float fStrafeVal = (::Random.randF(fShotStrafeMin, fShotStrafeMax) * fLRShotStrafeDir);
	float fStrafePwr = clampr(m_fShootingCurPowerLRUD * 2.0f, 0.0f, 1.0f);

	m_fLR_MovingFactor += (fStrafeVal * GetShootingEffectKoef() * fStrafePwr);
}

// Получить коэфицент силы тряски HUD-a при стрельбе
float CWeapon::GetShootingEffectKoef()
{
	float fShakeKoef = 1.0f;

	//--> Глушитель
	//fShakeKoef *= cur_silencer_koef.shooting_shake;

	return fShakeKoef;
}

void CWeapon::SetAmmoElapsed(int ammo_count)
{
	iAmmoElapsed				= ammo_count;

	u32 uAmmo					= u32(iAmmoElapsed);

	if (uAmmo != m_magazine.size())
	{
		if (uAmmo > m_magazine.size())
		{
			CCartridge			l_cartridge; 
			l_cartridge.Load	(*m_ammoTypes[m_ammoType], u8(m_ammoType));
			while (uAmmo > m_magazine.size())
				m_magazine.push_back(l_cartridge);
		}
		else
		{
			while (uAmmo < m_magazine.size())
				m_magazine.pop_back();
		};
	};
}

u32	CWeapon::ef_main_weapon_type	() const
{
	VERIFY	(m_ef_main_weapon_type != u32(-1));
	return	(m_ef_main_weapon_type);
}

u32	CWeapon::ef_weapon_type	() const
{
	VERIFY	(m_ef_weapon_type != u32(-1));
	return	(m_ef_weapon_type);
}

bool CWeapon::IsNecessaryItem	    (const shared_str& item_sect)
{
	return (std::find(m_ammoTypes.begin(), m_ammoTypes.end(), item_sect) != m_ammoTypes.end() );
}

bool CWeapon::IsNecessaryItem(const shared_str& item_sect, xr_vector<shared_str> item)
{
	return (std::find(item.begin(), item.end(), item_sect) != item.end());
}

void CWeapon::modify_holder_params		(float &range, float &fov) const
{
	if (!IsScopeAttached()) {
		inherited::modify_holder_params	(range,fov);
		return;
	}
	range	*= m_addon_holder_range_modifier;
	fov		*= m_addon_holder_fov_modifier;
}

bool CWeapon::render_item_ui_query()
{
	bool b_is_active_item = (m_pInventory->ActiveItem()==this);
	bool res = b_is_active_item && IsZoomed() && ZoomHideCrosshair() && ZoomTexture() && !IsRotatingToZoom();
	return res;
}

void CWeapon::render_item_ui()
{
	if (m_zoom_params.m_pVision)
		m_zoom_params.m_pVision->Draw();

	ZoomTexture()->Update	();
	ZoomTexture()->Draw		();
}

bool CWeapon::unlimited_ammo() 
{ 
	if (m_pInventory)
		return inventory_owner().unlimited_ammo() && m_DefaultCartridge.m_flags.test(CCartridge::cfCanBeUnlimited);
	else
		return false;
};

LPCSTR	CWeapon::GetCurrentAmmo_ShortName	()
{
	if (m_magazine.empty()) return ("");
	CCartridge &l_cartridge = m_magazine.back();
	return *(l_cartridge.m_InvShortName);
}

float CWeapon::GetMagazineWeight(const decltype(CWeapon::m_magazine)& mag) const
{
	float res = 0;
	const char* last_type = nullptr;
	float last_ammo_weight = 0;
	for (auto& c : mag)
	{
		// Usually ammos in mag have same type, use this fact to improve performance
		if (last_type != c.m_ammoSect.c_str())
		{
			last_type = c.m_ammoSect.c_str();
			last_ammo_weight = c.Weight();
		}
		res += last_ammo_weight;
	}
	return res;
}

float CWeapon::Weight() const
{
	float res = CInventoryItemObject::Weight();
	if(IsGrenadeLauncherAttached()&&GetGrenadeLauncherName().size())
	{
		res += pSettings->r_float(GetGrenadeLauncherName(),"inv_weight");
	}
	if(IsScopeAttached()&&m_scopes.size())
	{
		res += pSettings->r_float(GetScopeName(),"inv_weight");
	}
	if(IsSilencerAttached()&&GetSilencerName().size()){
		res += pSettings->r_float(GetSilencerName(),"inv_weight");
	}
	if (IsLaserAttached() && GetLaserName().size()) {
		res += pSettings->r_float(GetLaserName(), "inv_weight");
	}
	if (IsTacticalTorchAttached() && GetTacticalTorchName().size()) {
		res += pSettings->r_float(GetTacticalTorchName(), "inv_weight");
	}
	
	res += GetMagazineWeight(m_magazine);

	return res;
}

bool CWeapon::show_crosshair()
{
	return !IsPending() && ( !IsZoomed() || !ZoomHideCrosshair() );
}

bool CWeapon::show_indicators()
{
	return ! ( IsZoomed() && ZoomTexture() );
}

float CWeapon::GetConditionToShow	() const
{
	return	(GetCondition());//powf(GetCondition(),4.0f));
}

BOOL CWeapon::ParentMayHaveAimBullet	()
{
	CObject* O=H_Parent();
	CEntityAlive* EA=smart_cast<CEntityAlive*>(O);
	return EA->cast_actor()!=0;
}

bool CWeapon::ParentIsActor	()
{
	CObject* O			= H_Parent();
	if (!O)
		return FALSE;

	CEntityAlive* EA	= smart_cast<CEntityAlive*>(O);
	if (!EA)
		return FALSE;

	return EA->cast_actor()!=0;
}

extern int hud_adj_mode;

bool CWeapon::ZoomHideCrosshair()
{
	if (hud_adj_mode != 0)
		return false;

	return m_zoom_params.m_bHideCrosshairInZoom || ZoomTexture();
}

void CWeapon::debug_draw_firedeps()
{
#ifdef DEBUG
	if(hud_adj_mode==5||hud_adj_mode==6||hud_adj_mode==7)
	{
		CDebugRenderer &render = Level().debug_renderer();

		if(hud_adj_mode==5)
			render.draw_aabb(get_LastFP(), 0.005f,0.005f,0.005f,color_xrgb(255,0,0));

		if(hud_adj_mode==6)
			render.draw_aabb(get_LastFP2(),	0.005f,0.005f,0.005f,color_xrgb(0,0,255));

		if(hud_adj_mode==7)
			render.draw_aabb(get_LastSP(), 0.005f,0.005f,0.005f,color_xrgb(0,255,0));
	}
#endif
}

const float &CWeapon::hit_probability	() const
{
	VERIFY					((g_SingleGameDifficulty >= egdNovice) && (g_SingleGameDifficulty <= egdMaster)); 
	return					(m_hit_probability[egdNovice]);
}

void CWeapon::OnStateSwitch	(u32 S)
{
	inherited::OnStateSwitch(S);
	m_dwAmmoCurrentCalcFrame = 0;

	if (H_Parent() == Level().CurrentEntity())
	{
		CActor* current_actor = smart_cast<CActor*>(H_Parent());
		bool fp_cam = current_actor->cam_Active() == current_actor->cam_FirstEye();

		if (&HUD().GetUI()->UIGame()->ActorMenu() && HUD().GetUI()->UIGame()->ActorMenu().GetMenuMode() == mmUndefined)
		{
			if ((GetState() == eReload || GetState() == eUnMisfire || (GetState() == eBore && (GameConstants::GetSSFX_EnableBoreDoF() && m_bEnableBoreDof))) && current_actor && fp_cam)
			{
				ps_ssfx_wpn_dof_1 = GameConstants::GetSSFX_FocusDoF();
				ps_ssfx_wpn_dof_2 = GameConstants::GetSSFX_FocusDoF().z;
			}
			else
			{
				ps_ssfx_wpn_dof_1 = GameConstants::GetSSFX_DefaultDoF();
				ps_ssfx_wpn_dof_2 = GameConstants::GetSSFX_DefaultDoF().z;
			}
		}
	}
}

void CWeapon::OnAnimationEnd(u32 state) 
{
	inherited::OnAnimationEnd(state);
}

u8 CWeapon::GetCurrentHudOffsetIdx()
{
	if (!ParentIsActor())
		return 0;
	
	bool b_aiming		= 	((IsZoomed() && m_zoom_params.m_fZoomRotationFactor<=1.f) ||
							(!IsZoomed() && m_zoom_params.m_fZoomRotationFactor>0.f));

	if(!b_aiming)
		return 0;
	else
	{
		if (!m_bAltZoomActive)
			return 1;
		else
			return 3;
	}
}

void CWeapon::render_hud_mode()
{
	RenderLight();

	for (auto mesh : m_weapon_attaches)
		mesh->RenderAttach();
}

bool CWeapon::MovingAnimAllowedNow()
{
	return !IsZoomed();
}

bool CWeapon::IsHudModeNow()
{
	return (HudItemData()!=NULL);
}

void CWeapon::ZoomDynamicMod(bool bIncrement, bool bForceLimit)
{
	if (!IsScopeAttached())
		return;

	if (!m_zoom_params.m_bUseDynamicZoom)
		return;

	float delta, min_zoom_factor, max_zoom_factor;

	max_zoom_factor = (bIsSecondVPZoomPresent() ? GetSecondVPZoomFactor() * 100.0f : m_zoom_params.m_fScopeZoomFactor);

	GetZoomData(max_zoom_factor, delta, min_zoom_factor);

	if (bForceLimit)
	{
		m_fRTZoomFactor = (bIncrement ? max_zoom_factor : min_zoom_factor);
	}
	else
	{
		float f = (bIsSecondVPZoomPresent() ? m_fRTZoomFactor : GetZoomFactor());

		f -= delta * (bIncrement ? 1.f : -1.f);

		clamp(f, max_zoom_factor, min_zoom_factor);


		if (bIsSecondVPZoomPresent())
			m_fRTZoomFactor = f;
		else
			SetZoomFactor(f);

		// Lex Addon (correct by Suhar_) 24.10.2018		(begin)  
		LastZoomFactor = f;
		// Lex Addon (correct by Suhar_) 24.10.2018		(end)
	}
}

void CWeapon::ZoomInc()
{
	ZoomDynamicMod(true, false);
}

void CWeapon::ZoomDec()
{
	ZoomDynamicMod(false, false);
}

u32 CWeapon::Cost() const
{
	u32 res = CInventoryItem::Cost();
	if(IsGrenadeLauncherAttached()&&GetGrenadeLauncherName().size()){
		res += pSettings->r_u32(GetGrenadeLauncherName(),"cost");
	}
	if(IsScopeAttached()&&m_scopes.size()){
		res += pSettings->r_u32(GetScopeName(),"cost");
	}
	if(IsSilencerAttached()&&GetSilencerName().size()){
		res += pSettings->r_u32(GetSilencerName(),"cost");
	}
	if (IsLaserAttached() && GetLaserName().size()) {
		res += pSettings->r_u32(GetLaserName(), "cost");
	}
	if (IsTacticalTorchAttached() && GetTacticalTorchName().size()) {
		res += pSettings->r_u32(GetTacticalTorchName(), "cost");
	}
	
	if(iAmmoElapsed)
	{
		float w		= pSettings->r_float(m_ammoTypes[m_ammoType].c_str(),"cost");
		float bs	= pSettings->r_float(m_ammoTypes[m_ammoType].c_str(),"box_size");

		res			+= iFloor(w*(iAmmoElapsed/bs));
	}
	return res;

}

//  FOV       
float CWeapon::GetSecondVPFov() const
{
	if (m_zoom_params.m_bUseDynamicZoom && bIsSecondVPZoomPresent())
		return (m_fRTZoomFactor / 100.f) * 75.f;//g_fov; 75.f

	return GetSecondVPZoomFactor() * 75.f;//g_fov; 75.f
}

//      +SecondVP+
//      
void CWeapon::UpdateSecondVP(bool bInGrenade)
{
	bool b_is_active_item = (m_pInventory != NULL) && (m_pInventory->ActiveItem() == this);
	R_ASSERT(ParentIsActor() && b_is_active_item); //           

	CActor* pActor = smart_cast<CActor*>(H_Parent());

	bool bCond_1 = bInZoomRightNow();		// Мы должны целиться  

	bool bCond_2 = bIsSecondVPZoomPresent();						// В конфиге должен быть прописан фактор зума для линзы (scope_lense_factor
																	// больше чем 0)
	bool bCond_3 = pActor->cam_Active() == pActor->cam_FirstEye();	// Мы должны быть от 1-го лица	



	Device.m_SecondViewport.SetSVPActive(bCond_1 && bCond_2 && bCond_3 /*&& !bInGrenade*/);
}

const char* CWeapon::GetAnimAimName()
{
	auto pActor = smart_cast<const CActor*>(H_Parent());
	if (pActor)
	{
		const u32 state = pActor->get_state();

		if (state && state & mcAnyMove)
		{
			if (IsScopeAttached() && m_bUseScopeAimMoveAnims)
				return strconcat(sizeof(guns_aim_anm), guns_aim_anm, GenerateAimAnimName("anm_idle_aim_scope_moving"), (IsMisfire()) ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : "");
			else
			{
				if (m_bUseAimAnmDirDependency)
					return strconcat(sizeof(guns_aim_anm), guns_aim_anm, GenerateAimAnimName("anm_idle_aim_moving"), (state & mcFwd) ? "_forward" : ((state & mcBack) ? "_back" : ""), (state & mcLStrafe) ? "_left" : ((state & mcRStrafe) ? "_right" : ""), (IsMisfire() ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : ""));
				else
					return strconcat(sizeof(guns_aim_anm), guns_aim_anm, GenerateAimAnimName("anm_idle_aim_moving"), (IsMisfire() ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : ""));
			}
		}
	}
	return nullptr;
}

const char* CWeapon::GenerateAimAnimName(string64 base_anim)
{
	auto pActor = smart_cast<const CActor*>(H_Parent());
	if (pActor)
	{
		const u32 state = pActor->get_state();

		string64 buff;

		if (state & mcAnyMove)
		{
			if (!(state & mcCrouch))
			{
				if (state & mcAccel) //Ходьба медленная (SHIFT)
					return strconcat(sizeof(buff), buff, base_anim, "_slow");
				else
					return base_anim;
			}
			else if (state & mcAccel) //Ходьба в присяде (CTRL+SHIFT)
			{
				return strconcat(sizeof(buff), buff, base_anim, "_crouch_slow");
			}
			else
			{
				return strconcat(sizeof(buff), buff, base_anim, "_crouch");
			}
		}
	}

	return base_anim;
}

void CWeapon::OnBulletHit() 
{
	if (!fis_zero(conditionDecreasePerShotOnHit))
		ChangeCondition(-conditionDecreasePerShotOnHit);
}

bool CWeapon::IsPartlyReloading()
{
	return (m_set_next_ammoType_on_reload == u32(-1) && GetAmmoElapsed() > 0 && !IsMisfire());
}

bool CWeapon::IsMisfireNow()
{
	return IsMisfire();
}

bool CWeapon::IsMagazineEmpty()
{
	return IsEmptyMagazine();
}

void CWeapon::SwitchZoomMode()
{
	!m_bAltZoomActive ? m_bAltZoomActive = true : m_bAltZoomActive = false;

	if (!IsZoomed())
		return;

	shared_str cur_scope_sect = (m_sScopeAttachSection.size() ? m_sScopeAttachSection : (m_eScopeStatus == ALife::eAddonAttachable) ? m_scopes[m_cur_scope].c_str() : "scope");

	bool HudFovFromScope = false;
	HudFovFromScope = READ_IF_EXISTS(pSettings, r_bool, cur_scope_sect, "cur_scope_hud_fov", false);

	if (HudFovFromScope)
		psHUD_FOV_def = READ_IF_EXISTS(pSettings, r_float, cur_scope_sect, !m_bAltZoomActive ? "aim_hud_fov" : "aim_alt_hud_fov", GetHudFov());

	m_zoom_params.m_fCurrentZoomFactor = CurrentZoomFactor();
}

void CWeapon::SwitchLaser(bool on)
{
	if (!has_laser || !IsLaserAttached())
		return;

	if (on)
	{
		m_flagsAddOnState |= CSE_ALifeItemWeapon::eWeaponAddonLaserOn;

		if (isHUDAnimationExist("anm_laser_on"))
			SwitchState(eLaserSwitch);
	}
	else
	{
		m_flagsAddOnState &= ~CSE_ALifeItemWeapon::eWeaponAddonLaserOn;

		if (isHUDAnimationExist("anm_laser_off"))
			SwitchState(eLaserSwitch);
	}
}

void CWeapon::SwitchFlashlight(bool on)
{
	if (!has_flashlight || !IsTacticalTorchAttached())
		return;

	if (on)
	{
		m_flagsAddOnState |= CSE_ALifeItemWeapon::eWeaponAddonFlashlightOn;

		if (isHUDAnimationExist("anm_torch_on"))
			SwitchState(eFlashlightSwitch);
	}
	else
	{
		m_flagsAddOnState &= ~CSE_ALifeItemWeapon::eWeaponAddonFlashlightOn;

		if (isHUDAnimationExist("anm_torch_off"))
			SwitchState(eFlashlightSwitch);
	}
}

void CWeapon::UpdateAddonsTransform(bool for_hud)
{
	Fmatrix base_model_trans = for_hud ? HudItemData()->m_item_transform : XFORM();
	IRenderVisual* model = for_hud ? HudItemData()->m_model->dcast_RenderVisual() : smart_cast<IRenderVisual*>(Visual());

	if (!for_hud)
	{
		Fmatrix scale, t;
		t = m_scopeAttachTransform;
		scale.scale(0.6666f, 0.6666f, 0.6666f);
		m_scopeAttachTransform.mul(t, scale); // rafa: hack for fucking gunslinger models
	}

	for (auto& mesh : m_weapon_attaches)
		mesh->UpdateAttachesPosition(model, base_model_trans, for_hud);
	
	if (!for_hud)
	{
		for (auto mesh : m_weapon_attaches)
			mesh->RenderAttach(false);
	}
}

void CWeapon::UpdateAimOffsets()
{
	shared_str cur_scope_sect = (m_sScopeAttachSection.size() ? m_sScopeAttachSection : (m_eScopeStatus == ALife::eAddonAttachable) ? m_scopes[m_cur_scope].c_str() : "scope");
	psHUD_FOV_def = last_hud_fov;

	static bool bNeedRestoreOffsets = false;

	if (bNeedRestoreOffsets && (!IsScopeAttached() || (!IsZoomed() && !IsRotatingFromZoom()) || !cur_scope_sect.size()))
	{
		attachable_hud_item* hi = HudItemData();

		if (!hi)
			return;

		bool is_16x9 = UI().is_widescreen();
		string64	_prefix;
		xr_sprintf(_prefix, "%s", is_16x9 ? "_16x9" : "");
		string128	val_name;

		strconcat(sizeof(val_name), val_name, "aim_hud_offset_pos", _prefix);
		hi->m_measures.m_hands_offset[0][1] = pSettings->r_fvector3(m_hud_sect, val_name);
		strconcat(sizeof(val_name), val_name, "aim_hud_offset_rot", _prefix);
		hi->m_measures.m_hands_offset[1][1] = pSettings->r_fvector3(m_hud_sect, val_name);

		strconcat(sizeof(val_name), val_name, "aim_alt_hud_offset_pos", _prefix);
		hi->m_measures.m_hands_offset[0][3] = READ_IF_EXISTS(pSettings, r_fvector3, m_hud_sect, val_name, hi->m_measures.m_hands_offset[0][1]);
		strconcat(sizeof(val_name), val_name, "aim_alt_hud_offset_rot", _prefix);
		hi->m_measures.m_hands_offset[1][3] = READ_IF_EXISTS(pSettings, r_fvector3, m_hud_sect, val_name, hi->m_measures.m_hands_offset[1][1]);

		strconcat(sizeof(val_name), val_name, "gl_hud_offset_pos", _prefix);
		hi->m_measures.m_hands_offset[0][2] = pSettings->r_fvector3(m_hud_sect, val_name);
		strconcat(sizeof(val_name), val_name, "gl_hud_offset_rot", _prefix);
		hi->m_measures.m_hands_offset[1][2] = pSettings->r_fvector3(m_hud_sect, val_name);

		m_bAltZoomEnabledScope = READ_IF_EXISTS(pSettings, r_bool, cur_scope_sect, "enable_alternative_aim", false);
		bNeedRestoreOffsets = false;

		return;
	}

	bool HudFovFromScope = false;
	HudFovFromScope = READ_IF_EXISTS(pSettings, r_bool, cur_scope_sect, "cur_scope_hud_fov", false);

	if (HudFovFromScope && !IsRotatingFromZoom())
		psHUD_FOV_def = READ_IF_EXISTS(pSettings, r_float, cur_scope_sect, !m_bAltZoomActive ? "aim_hud_fov" : "aim_alt_hud_fov", GetHudFov());

	if (m_bAltZoomEnabledScope)
	{
		attachable_hud_item* hi = HudItemData();

		if (!hi)
			return;

		bool is_16x9 = UI().is_widescreen();
		string64	_prefix;
		xr_sprintf(_prefix, "%s", is_16x9 ? "_16x9" : "");
		string128	val_name;

		strconcat(sizeof(val_name), val_name, "aim_hud_offset_pos", _prefix);
		hi->m_measures.m_hands_offset[0][1] = READ_IF_EXISTS(pSettings, r_fvector3, cur_scope_sect, val_name, hi->m_measures.m_hands_offset[0][1]);
		strconcat(sizeof(val_name), val_name, "aim_hud_offset_rot", _prefix);
		hi->m_measures.m_hands_offset[1][1] = READ_IF_EXISTS(pSettings, r_fvector3, cur_scope_sect, val_name, hi->m_measures.m_hands_offset[1][1]);

		strconcat(sizeof(val_name), val_name, "aim_alt_hud_offset_pos", _prefix);
		hi->m_measures.m_hands_offset[0][3] = READ_IF_EXISTS(pSettings, r_fvector3, cur_scope_sect, val_name, hi->m_measures.m_hands_offset[0][1]);
		strconcat(sizeof(val_name), val_name, "aim_alt_hud_offset_rot", _prefix);
		hi->m_measures.m_hands_offset[1][3] = READ_IF_EXISTS(pSettings, r_fvector3, cur_scope_sect, val_name, hi->m_measures.m_hands_offset[1][1]);

		bNeedRestoreOffsets = true;
	}
}

void CWeapon::RemoveBones(xr_vector<shared_str>& m_all_bones, const xr_vector<shared_str>& bones_to_remove)
{
	m_all_bones.erase(
		std::remove_if(m_all_bones.begin(), m_all_bones.end(),
			[&bones_to_remove](const shared_str& bone) {
				return std::find(bones_to_remove.begin(),
					bones_to_remove.end(), bone) !=
					bones_to_remove.end();
			}),
		m_all_bones.end());
}

void CWeapon::ModifyUpgradeBones(shared_str bones_names, bool show)
{
	auto SetBoneVisible = [&](const shared_str& boneName, BOOL visibility)
	{
		HudItemData()->set_bone_visible(boneName, visibility, TRUE);
	};

	xr_vector<shared_str> bones_to_remove_vec{};

	if (show)
	{
		for (int i = 0; i < _GetItemCount(*bones_names); i++)
		{
			string128 bone;
			_GetItem(*bones_names, i, bone);

			bones_to_remove_vec.push_back(bone);

			if (std::find_if(m_defShownBones.begin(), m_defShownBones.end(), [&](const shared_str& name) { return name == bone; }) == m_defShownBones.end())
				m_defShownBones.push_back(bone);
		}
		RemoveBones(m_defHiddenBones, bones_to_remove_vec);
	}
	else
	{
		for (int i = 0; i < _GetItemCount(*bones_names); i++)
		{
			string128 bone;
			_GetItem(*bones_names, i, bone);

			bones_to_remove_vec.push_back(bone);

			if (std::find_if(m_defHiddenBones.begin(), m_defHiddenBones.end(), [&](const shared_str& name) { return name == bone; }) == m_defHiddenBones.end())
				m_defHiddenBones.push_back(bone);
		}
		RemoveBones(m_defShownBones, bones_to_remove_vec);
	}
}

void CWeapon::DeviceSwitch()
{
	OnZoomOut();
}