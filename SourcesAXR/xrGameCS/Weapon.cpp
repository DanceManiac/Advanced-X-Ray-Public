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
#include "WeaponMagazinedWGrenade.h"
#include "UIGameCustom.h"
#include "ui/UIWindow.h"
#include "ui/UIXmlInit.h"
#include "ui\UIActorMenu.h"
#include "Torch.h"
#include "ActorNightVision.h"
#include "../xrEngine/x_ray.h"
#include "../xrEngine/LightAnimLibrary.h"
#include "../xrEngine/GameMtlLib.h"
#include "PostprocessAnimator.h"
#include "../xrEngine/CameraBase.h"
#include "CharacterPhysicsSupport.h"
#include "AdvancedXrayGameConstants.h"
#include "string_table.h"

#include "pch_script.h"
#include "script_callback_ex.h"
#include "script_game_object.h"

constexpr auto WEAPON_REMOVE_TIME = 60000;
constexpr auto ROTATION_TIME = 0.25f;

BOOL	b_toggle_weapon_aim		= FALSE;
extern CUIXml*	pWpnScopeXml	= NULL;

ENGINE_API extern float psHUD_FOV_def;
static float last_hud_fov = psHUD_FOV_def;
extern BOOL g_use_aim_inertion = 1;
u32		lfo_scope_type = 1;

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
	m_zoom_params.m_fAltAimZoomFactor			= 50.f;

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

	//Mortan: new params
	bUseAltScope		= false;
	bScopeIsHasTexture	= false;
	bNVsecondVPavaible	= false;
	bNVsecondVPstatus	= false;
	m_fZoomStepCount	= 3.0f;
	m_fZoomMinKoeff		= 0.3f;

	bHasBulletsToHide	= false;
	m_bBulletsVisualization = false;
	bullet_cnt			= 0;
	IsCustomReloadAvaible = false;

	m_bUseAimScopeAnims = true;
	m_bUseScopeAimMoveAnims = true;
	m_bUseAimAnmDirDependency = false;
	m_bAltZoomEnabled	= false;
	m_bAltZoomEnabledScope = false;
	m_bAltZoomActive	= false;

	m_sSafetyBoneName		= nullptr;
	m_fSafetyRotationSpeed	= 1.0f;
	m_fSafetyRotationTime	= 0.0f;
	m_mSafetyRotation.identity();

	m_bBlockSilencerWithGL	= false;
	m_bLaserBlockedByAddon	= false;
	m_bFlashlightBlockedByAddon = false;
	m_bStockBlockedByAddon = false;
	m_bGripHBlockedByAddon = false;
	m_bGripVBlockedByAddon = false;

	m_fOverheatingSubRpm	= 0.0f;
	m_fOverheatingMisfire	= 0.0f;
	m_fOverheatingCond		= 0.0f;

	m_bIsRevolver			= false;
	m_bIsBoltRiffle			= false;
	m_bIsShotgun			= false;
	m_bLastShotRPM			= false;
	m_bUseRG6_AddCartridgeAlt = false;

	m_bWpnExplosion			= false;
	m_bWpnDestroyAfterExplode = false;
	m_fWpnExplodeChance		= 0.0f;

	m_bIndoorSoundsEnabled	= false;
	m_bMisfireBulletRemove	= false;

	m_bOutScopeAfterShot = false;
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
	overheat_glow.destroy();
	overheat_omni.destroy();
	overheat_render.destroy();
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
			Fvector& osp			= vLoadedOverheatingSmokePoint;

			parent.transform_tiny	(m_current_firedeps.vLastFP,fp);
			parent.transform_tiny	(m_current_firedeps.vLastFP2,fp2);
			parent.transform_tiny	(m_current_firedeps.vLastSP,sp);
			parent.transform_tiny	(m_current_firedeps.vLastOSP, osp);
			
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

LPCSTR wpn_scope_def_bone		= "wpn_scope";
LPCSTR wpn_silencer_def_bone	= "wpn_silencer";
LPCSTR wpn_launcher_def_bone	= "wpn_launcher";
LPCSTR wpn_laser_def_bone		= "wpn_laser";
LPCSTR wpn_torch_def_bone		= "wpn_torch";
LPCSTR wpn_overheat_def_bone	= "overheat_barrel";
LPCSTR wpn_stock_def_bone		= "wpn_stock";
LPCSTR wpn_grip_def_bone		= "wpn_grip_horizontal";
LPCSTR wpn_gripv_def_bone		= "wpn_grip_vertical";

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
	if (lfo_scope_type == 3)// SHOKER 3d Scopes PiP
	{
		m_eScopeStatus = (ALife::EWeaponAddonStatus)pSettings->r_s32(section, "scope_status_pip");
	}
	else
	{

		if (lfo_scope_type == 1 || lfo_scope_type == 2)
		{
			m_eScopeStatus = (ALife::EWeaponAddonStatus)pSettings->r_s32(section, "scope_status_2d");
		}
		else
		{
			m_eScopeStatus = (ALife::EWeaponAddonStatus)pSettings->r_s32(section, "scope_status");
		}
	}

	m_eSilencerStatus			= (ALife::EWeaponAddonStatus)pSettings->r_s32(section,"silencer_status");
	m_eGrenadeLauncherStatus	= (ALife::EWeaponAddonStatus)pSettings->r_s32(section,"grenade_launcher_status");
	m_eLaserDesignatorStatus	= (ALife::EWeaponAddonStatus)READ_IF_EXISTS(pSettings, r_s32, section, "laser_designator_status", 0);
	m_eTacticalTorchStatus		= (ALife::EWeaponAddonStatus)READ_IF_EXISTS(pSettings, r_s32, section, "tactical_torch_status", 0);
	m_eStockStatus				= (ALife::EWeaponAddonStatus)READ_IF_EXISTS(pSettings, r_s32, section, "stock_status", 0);
	m_eGripStatus				= (ALife::EWeaponAddonStatus)READ_IF_EXISTS(pSettings, r_s32, section, "grip_h_status", 0);
	m_eGripvStatus				= (ALife::EWeaponAddonStatus)READ_IF_EXISTS(pSettings, r_s32, section, "grip_v_status", 0);

	m_zoom_params.m_bZoomEnabled		= !!pSettings->r_bool(section,"zoom_enabled");

	if (lfo_scope_type == 3)// SHOKER 3d Scopes PiP
	{
		m_zoom_params.m_fZoomRotateTime = pSettings->r_float(section, "zoom_rotate_time_pip");
	}
	else
	{
		m_zoom_params.m_fZoomRotateTime = pSettings->r_float(section, "zoom_rotate_time");
	}

	m_bOutScopeAfterShot = READ_IF_EXISTS(pSettings, r_bool, section, "out_scope_after_shot", false);

	bUseAltScope = !!bLoadAltScopesParams(section);

	if (!bUseAltScope)
		LoadOriginalScopesParams(section);

	LoadSilencerParams(section);
	LoadGrenadeLauncherParams(section);
	LoadLaserDesignatorParams(section);
	LoadTacticalTorchParams(section);
	LoadOverheatLightParams(section);
	LoadStockParams(section);
	LoadGripParams(section);
	LoadGripvParams(section);

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

	if (psHUD_Flags.test(HUD_CROSSHAIR_AIM))
	{
		if (pSettings->line_exist(m_hud_sect, "zoom_hide_crosshair"))
			m_zoom_params.m_bHideCrosshairInZoom = !!pSettings->r_bool(m_hud_sect, "zoom_hide_crosshair");
	}

	Fvector			def_dof;
	def_dof.set		(-1,-1,-1);
//	m_zoom_params.m_ZoomDof		= READ_IF_EXISTS(pSettings, r_fvector3, section, "zoom_dof", Fvector().set(-1,-1,-1));
//	m_zoom_params.m_bZoomDofEnabled	= !def_dof.similar(m_zoom_params.m_ZoomDof);

	m_zoom_params.m_ReloadDof = READ_IF_EXISTS(pSettings, r_fvector4, section, "reload_dof", Fvector4().set(-1, -1, -1, -1));

	m_zoom_params.m_ReloadEmptyDof = READ_IF_EXISTS(pSettings, r_fvector4, section, "reload_empty_dof", Fvector4().set(-1, -1, -1, -1));



	m_bHasTracers			= !!READ_IF_EXISTS(pSettings, r_bool, section, "tracers", true);
	m_u8TracerColorID		= READ_IF_EXISTS(pSettings, r_u8, section, "tracers_color_ID", u8(-1));

	m_bBlockSilencerWithGL	= READ_IF_EXISTS(pSettings, r_bool, section, "block_sil_if_gl", false);
	m_bBlockGripWithGrip	= READ_IF_EXISTS(pSettings, r_bool, section, "grip_block_grip", false);

	m_fWeaponOverheatingInc = READ_IF_EXISTS(pSettings, r_float, section, "overheating_shot_inc", 0.0f);
	m_fWeaponOverheatingDec = READ_IF_EXISTS(pSettings, r_float, section, "overheating_shot_dec", m_fWeaponOverheatingInc);
	m_fOverheatingSubRpm	= READ_IF_EXISTS(pSettings, r_float, section, "overheating_rpm_factor", 0.0f);
	m_fOverheatingMisfire	= READ_IF_EXISTS(pSettings, r_float, section, "overheating_misfire_factor", 0.0f);
	m_fOverheatingCond		= READ_IF_EXISTS(pSettings, r_float, section, "overheating_condition_factor", 0.0f);

	m_bIndoorSoundsEnabled	= READ_IF_EXISTS(pSettings, r_bool, section, "indoor_sounds_enabled", false);
	m_bMisfireBulletRemove	= READ_IF_EXISTS(pSettings, r_bool, section, "misfire_bullet_remove", true);

	m_bBulletsVisualization = pSettings->line_exist(section, "bullet_bones");

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

	if (pSettings->line_exist(section, "stock_bone"))
		m_sWpn_stock_bone = pSettings->r_string(section, "stock_bone");
	else
		m_sWpn_stock_bone = wpn_stock_def_bone;

	if (pSettings->line_exist(section, "grip_h_bone"))
		m_sWpn_grip_bone = pSettings->r_string(section, "grip_h_bone");
	else
		m_sWpn_grip_bone = wpn_grip_def_bone;

	if (pSettings->line_exist(section, "grip_v_bone"))
		m_sWpn_gripv_bone = pSettings->r_string(section, "grip_v_bone");
	else
		m_sWpn_gripv_bone = wpn_gripv_def_bone;

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
	// SHOW-HIDE BONES
	LoadBoneNames(section, "def_show_bones", m_defShownBones);
	LoadBoneNames(section, "def_hide_bones", m_defHiddenBones);
	LoadBoneNames(section, "def_show_bones_aim_scope_permanent", m_defShownBonesScopePermanent);
	LoadBoneNames(section, "def_hide_bones_aim_scope_permanent", m_defHiddenBonesScopePermanent);
	LoadBoneNames(section, "def_hide_bones_aim_scope_permanent2", m_defHiddenBonesScopePermanent2);
	LoadBoneNames(section, "def_show_bones_with_scopes", m_defShownBonesScope);
	LoadBoneNames(section, "def_hide_bones_with_scopes", m_defHiddenBonesScope);
	LoadBoneNames(section, "hide_bones_override_when_silencer_attached", m_defSilHiddenBones);
	LoadBoneNames(section, "def_animated_3d_shell_bones", m_defShellBones);
	LoadBoneNames(section, "overheat_barrel_bones", m_defOverheatinBarrel);
	LoadBoneNames(section, "stock_default_bones", m_deleteStockBone);


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

	hud_recalc_koef = READ_IF_EXISTS(pSettings, r_float, m_hud_sect, "hud_recalc_koef", 1.35f); //На калаше при 1.35 вроде норм смотрится, другим стволам возможно придется подбирать другие значения.

	m_SuitableRepairKits.clear();
	m_ItemsForRepair.clear();
	m_ItemsForRepairNames.clear();

	LPCSTR repair_kits = READ_IF_EXISTS(pSettings, r_string, section, "suitable_repair_kits", "repair_kit");
	LPCSTR items_for_repair = READ_IF_EXISTS(pSettings, r_string, section, "items_for_repair", "");

	// Added by Axel, to enable optional condition use on any item
	m_flags.set(FUsingCondition, READ_IF_EXISTS(pSettings, r_bool, section, "use_condition", true));

	m_bShowWpnStats				= READ_IF_EXISTS(pSettings, r_bool, section, "show_wpn_stats", true);
	m_bEnableBoreDof			= READ_IF_EXISTS(pSettings, r_bool, section, "enable_bore_dof", true);
	m_bUseAimScopeAnims			= READ_IF_EXISTS(pSettings, r_bool, section, "enable_aim_scope_anims", true);
	m_bUseScopeAimMoveAnims		= READ_IF_EXISTS(pSettings, r_bool, section, "enable_scope_aim_move_anm", true);
	m_bUseAimAnmDirDependency	= READ_IF_EXISTS(pSettings, r_bool, section, "enable_aim_anm_dir_dependency", false);
	m_bUseSilShotAnim			= READ_IF_EXISTS(pSettings, r_bool, section, "enable_silencer_shoot_anm", false);
	m_sSafetyBoneName			= READ_IF_EXISTS(pSettings, r_string, section, "safety_bone", nullptr);

	if (lfo_scope_type == 4)//LFO 3d Scopes
	{
		m_bAltZoomEnabled = READ_IF_EXISTS(pSettings, r_bool, section, "enable_alternative_aim", false);
	}
	else
	{
		m_bAltZoomEnabled = READ_IF_EXISTS(pSettings, r_bool, section, "enable_alternative_aim_pip", false);
	}

	if (m_sSafetyBoneName.size())
	{
		m_vSafetyRotationAxis = pSettings->r_fvector3(section, "safety_rot_axis");
		m_fSafetyRotationSpeed = READ_IF_EXISTS(pSettings, r_float, section, "safety_rot_speed", 1.0f);

		for (u32 i{}; i <= 3; i++)
		{
			string128 safety_rot_param{};
			strconcat(sizeof(safety_rot_param), safety_rot_param, "safety_rot_step_mode_", std::to_string(i).c_str());
			m_fSafetyRotationSteps[i] = READ_IF_EXISTS(pSettings, r_float, section, safety_rot_param, 0.0f);
		}
	}

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
			{
				m_ItemsForRepair.push_back(std::make_pair(items_for_repair_sect, 0));
				m_ItemsForRepairNames.push_back(pSettings->r_string(items_for_repair_sect, "inv_name"));
			}
		}
	}

	m_bWpnExplosion				= READ_IF_EXISTS(pSettings, r_bool, section, "enable_weapon_explosion", false);
	m_bWpnDestroyAfterExplode	= READ_IF_EXISTS(pSettings, r_bool, section, "enable_weapon_destroying", false);
	m_fWpnExplodeChance			= READ_IF_EXISTS(pSettings, r_float, section, "weapon_explode_chance", 0.5f);
	sndWpnExplosion.create		(READ_IF_EXISTS(pSettings, r_string, section, "snd_explosion", "weapons\\f1_explode"), st_Effect, sg_SourceType);
	ppeWpnExplosion				= READ_IF_EXISTS(pSettings, r_string, section, "ppe_explosion", "snd_shock.ppe");

	ProcessBlendCamParams(READ_IF_EXISTS(pSettings, r_string, m_hud_sect, "blend_aim_start",	nullptr),	m_BlendAimStartCam);
	ProcessBlendCamParams(READ_IF_EXISTS(pSettings, r_string, m_hud_sect, "blend_aim_end",		nullptr),	m_BlendAimEndCam);
	ProcessBlendCamParams(READ_IF_EXISTS(pSettings, r_string, m_hud_sect, "blend_aim_idle",		nullptr),	m_BlendAimIdleCam);
	ProcessBlendCamParams(READ_IF_EXISTS(pSettings, r_string, m_hud_sect, "blend_aim_start_g",	nullptr),	m_BlendAimStartGL_Cam);
	ProcessBlendCamParams(READ_IF_EXISTS(pSettings, r_string, m_hud_sect, "blend_aim_end_g",	nullptr),	m_BlendAimEndGL_Cam);
	ProcessBlendCamParams(READ_IF_EXISTS(pSettings, r_string, m_hud_sect, "blend_aim_idle_g",	nullptr),	m_BlendAimIdleGL_Cam);
	ProcessBlendCamParams(READ_IF_EXISTS(pSettings, r_string, m_hud_sect, "blend_fakeshot",		nullptr),	m_BlendFakeShootCam);

	if (lfo_scope_type == 3)// SHOKER 3d Scopes PiP
	{
		m_zoom_params.m_fSecondVPFovFactor = READ_IF_EXISTS(pSettings, r_float, section, "scope_zoom_3d_fov", 0.0f);
		m_zoom_params.m_f3dZoomFactor = READ_IF_EXISTS(pSettings, r_float, section, "scope_zoom_factor_3d", 100.0f);
	}
	else
	{
		m_zoom_params.m_fSecondVPFovFactor = 0.0f;
		m_zoom_params.m_f3dZoomFactor = 0.0f;
	}
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

void CWeapon::LoadGripParams(LPCSTR section)
{
	if (m_eGripStatus == ALife::eAddonAttachable)
	{
		if (pSettings->line_exist(section, "grip_h_attach_sect"))
		{
			m_sGripAttachSection = pSettings->r_string(section, "grip_h_attach_sect");
			m_sGripName = pSettings->r_string(m_sGripAttachSection, "grip_h_name");

			m_iGripX = pSettings->r_s32(m_sGripAttachSection, "grip_h_x");
			m_iGripY = pSettings->r_s32(m_sGripAttachSection, "grip_h_y");
		}
		else
		{
			m_sGripName = pSettings->r_string(section, "grip_h_name");

			m_iGripX = pSettings->r_s32(section, "grip_h_x");
			m_iGripY = pSettings->r_s32(section, "grip_h_y");
		}
	}
	else if (m_eGripStatus == ALife::eAddonPermanent)
	{
		m_sGripName = READ_IF_EXISTS(pSettings, r_string, section, "grip_h_name", "");
		m_sGripAttachSection = READ_IF_EXISTS(pSettings, r_string, section, "grip_h_attach_sect", "");
	}
}

void CWeapon::LoadGripvParams(LPCSTR section)
{
	if (m_eGripvStatus == ALife::eAddonAttachable)
	{
		if (pSettings->line_exist(section, "grip_v_attach_sect"))
		{
			m_sGripvAttachSection = pSettings->r_string(section, "grip_v_attach_sect");
			m_sGripvName = pSettings->r_string(m_sGripvAttachSection, "grip_v_name");

			m_iGripvX = pSettings->r_s32(m_sGripvAttachSection, "grip_v_x");
			m_iGripvY = pSettings->r_s32(m_sGripvAttachSection, "grip_v_y");
		}
		else
		{
			m_sGripvName = pSettings->r_string(section, "grip_v_name");

			m_iGripvX = pSettings->r_s32(section, "grip_v_x");
			m_iGripvY = pSettings->r_s32(section, "grip_v_y");
		}
	}
	else if (m_eGripvStatus == ALife::eAddonPermanent)
	{
		m_sGripvName = READ_IF_EXISTS(pSettings, r_string, section, "grip_v_name", "");
		m_sGripvAttachSection = READ_IF_EXISTS(pSettings, r_string, section, "grip_v_attach_sect", "");
	}
}

void CWeapon::LoadStockParams(LPCSTR section)
{
	if (m_eStockStatus == ALife::eAddonAttachable)
	{
		if (pSettings->line_exist(section, "stock_attach_sects"))
		{
			LPCSTR stock_sects = pSettings->r_string(section, "stock_attach_sects");
			string128 single_sect;
			int count = _GetItemCount(stock_sects);

			for (int i = 0; i < count; ++i)
			{
				_GetItem(stock_sects, i, single_sect);
				m_availableStocks.push_back(single_sect);
			}

			if (!m_availableStocks.empty() && !m_sStockAttachSection)
			{
				m_sStockAttachSection = m_availableStocks[0];
				m_sStockName = pSettings->r_string(m_sStockAttachSection, "stock_name");
				m_iStockX = pSettings->r_s32(m_sStockAttachSection, "stock_x");
				m_iStockY = pSettings->r_s32(m_sStockAttachSection, "stock_y");
			}
		}
		else if (pSettings->line_exist(section, "stock_name"))
		{
			m_sStockName = pSettings->r_string(section, "stock_name");

			m_iStockX = pSettings->r_s32(section, "stock_x");
			m_iStockY = pSettings->r_s32(section, "stock_y");
		}
	}
	else if (m_eStockStatus == ALife::eAddonPermanent)
	{
		m_sStockName = READ_IF_EXISTS(pSettings, r_string, section, "stock_name", "");
		m_sStockAttachSection = READ_IF_EXISTS(pSettings, r_string, section, "stock_attach_sect", "");
	}
}

void CWeapon::LoadLaserDesignatorParams(LPCSTR section)
{
	if (m_eLaserDesignatorStatus == ALife::eAddonAttachable)
	{
		if (pSettings->line_exist(section, "laser_designator_attach_sects"))
		{
			LPCSTR laser_sects = pSettings->r_string(section, "laser_designator_attach_sects");
			string128 single_sect;
			int count = _GetItemCount(laser_sects);

			for (int i = 0; i < count; ++i)
			{
				_GetItem(laser_sects, i, single_sect);
				m_availableLasers.push_back(single_sect);
			}

			if (!m_availableLasers.empty() && !m_sLaserAttachSection)
			{
				m_sLaserAttachSection = m_availableLasers[0];
				m_sLaserName = pSettings->r_string(m_sLaserAttachSection, "laser_designator_name");
				m_iLaserX = pSettings->r_s32(m_sLaserAttachSection, "laser_designator_x");
				m_iLaserY = pSettings->r_s32(m_sLaserAttachSection, "laser_designator_y");
			}
		}
		else if (pSettings->line_exist(section, "laser_designator_name"))
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

void CWeapon::LoadLaserLightParams(LPCSTR section)
{
	if (!laser_light_render && m_eLaserDesignatorStatus)
	{
		has_laser = true;

		laserdot_attach_bone = READ_IF_EXISTS(pSettings, r_string, section, "laserdot_attach_bone", m_sWpn_laser_bone);
		laserdot_attach_offset = READ_IF_EXISTS(pSettings, r_fvector3, section, "laserdot_attach_offset", Fvector3().set(0.f, 0.f, 0.f));
		laserdot_world_attach_offset = READ_IF_EXISTS(pSettings, r_fvector3, section, "laserdot_world_attach_offset", Fvector3().set(0.f, 0.f, 0.f));

		const bool b_r2 = psDeviceFlags.test(rsR2) || psDeviceFlags.test(rsR4);

		shared_str current_laser_section = m_sLaserName;
		if (!current_laser_section.size() && !m_availableLasers.empty())
		{
			current_laser_section = pSettings->r_string(m_availableLasers[0], "laser_designator_name");
		}

		if (current_laser_section.size())
		{
			const char* m_light_section = pSettings->r_string(current_laser_section, "laser_light_section");

			laser_lanim = LALib.FindItem(READ_IF_EXISTS(pSettings, r_string, m_light_section, "color_animator", ""));

			laser_light_render = ::Render->light_create();
			laser_light_render->set_type(IRender_Light::SPOT);
			laser_light_render->set_shadow(true);

			Fcolor clr = READ_IF_EXISTS(pSettings, r_fcolor, m_light_section, b_r2 ? "color_r2" : "color", (Fcolor{ 1.0f, 0.0f, 0.0f, 1.0f }));
			laser_fBrightness = clr.intensity() * 2.f;
			clr.mul_rgb(laser_fBrightness);
			laser_light_render->set_color(clr);

			const float range = READ_IF_EXISTS(pSettings, r_float, m_light_section, b_r2 ? "range_r2" : "range", 100.f);
			laser_light_render->set_range(range);
			laser_light_render->set_cone(deg2rad(READ_IF_EXISTS(pSettings, r_float, m_light_section, "spot_angle", 1.f)));
			laser_light_render->set_texture(READ_IF_EXISTS(pSettings, r_string, m_light_section, "spot_texture", nullptr));
		}
		else
		{
			Msg("! [%s]: No valid laser section found. Weapon section: [%s]", __FUNCTION__, section);
		}
	}
}

void CWeapon::LoadTacticalTorchParams(LPCSTR section)
{
	if (m_eTacticalTorchStatus == ALife::eAddonAttachable)
	{
		if (pSettings->line_exist(section, "tactical_torch_attach_sects"))
		{
			LPCSTR flashlight_sects = pSettings->r_string(section, "tactical_torch_attach_sects");
			string128 single_sect;
			int count = _GetItemCount(flashlight_sects);

			for (int i = 0; i < count; ++i)
			{
				_GetItem(flashlight_sects, i, single_sect);
				m_availableFlashlights.push_back(single_sect);
			}

			if (!m_availableFlashlights.empty() && !m_sTacticalTorchAttachSection)
			{
				m_sTacticalTorchAttachSection = m_availableFlashlights[0];
				m_sTacticalTorchName = pSettings->r_string(m_sTacticalTorchAttachSection, "tactical_torch_name");
				m_iTacticalTorchX = pSettings->r_s32(m_sTacticalTorchAttachSection, "tactical_torch_x");
				m_iTacticalTorchY = pSettings->r_s32(m_sTacticalTorchAttachSection, "tactical_torch_y");
			}
		}
		else if (pSettings->line_exist(section, "tactical_torch_name"))
		{
			m_sTacticalTorchName = pSettings->r_string(section, "tactical_torch_name");

			m_iTacticalTorchX = pSettings->r_s32(section, "tactical_torch_x");
			m_iTacticalTorchY = pSettings->r_s32(section, "tactical_torch_y");
		}
	}
	else if (m_eTacticalTorchStatus == ALife::eAddonPermanent)
	{
		m_sTacticalTorchName = READ_IF_EXISTS(pSettings, r_string, section, "tactical_torch_name", "");
		m_sTacticalTorchAttachSection = READ_IF_EXISTS(pSettings, r_string, section, "tactical_torch_attach_sect", "");
	}
}

void CWeapon::LoadTacticalTorchLightParams(LPCSTR section)
{
	if (!flashlight_render && m_eTacticalTorchStatus)
	{
		has_flashlight = true;
		
		flashlight_attach_bone = READ_IF_EXISTS(pSettings, r_string, section, "torch_light_bone", m_sWpn_flashlight_bone);
		flashlight_attach_offset = READ_IF_EXISTS(pSettings, r_fvector3, section, "torch_attach_offset", Fvector3().set(0.f, 0.f, 0.f));
		flashlight_omni_attach_offset = READ_IF_EXISTS(pSettings, r_fvector3, section, "torch_omni_attach_offset", Fvector3().set(0.f, 0.f, 0.f));
		flashlight_world_attach_offset = READ_IF_EXISTS(pSettings, r_fvector3, section, "torch_world_attach_offset", Fvector3().set(0.f, 0.f, 0.f));
		flashlight_omni_world_attach_offset = READ_IF_EXISTS(pSettings, r_fvector3, section, "torch_omni_world_attach_offset", Fvector3().set(0.f, 0.f, 0.f));

		const bool b_r2 = psDeviceFlags.test(rsR2) || psDeviceFlags.test(rsR4);

		shared_str current_flashlight_section = m_sTacticalTorchName;
		if (!current_flashlight_section.size() && !m_availableFlashlights.empty())
		{
			current_flashlight_section = pSettings->r_string(m_availableFlashlights[0], "tactical_torch_name");
		}

		if (current_flashlight_section.size())
		{
			const char* m_light_section = pSettings->r_string(current_flashlight_section, "flashlight_section");

			flashlight_lanim = LALib.FindItem(READ_IF_EXISTS(pSettings, r_string, m_light_section, "color_animator", ""));

			flashlight_render = ::Render->light_create();
			flashlight_render->set_type(IRender_Light::SPOT);
			flashlight_render->set_shadow(true);
			flashlight_render->set_flare(true);

			if (ParentIsActor())
				flashlight_render->set_volumetric(!!READ_IF_EXISTS(pSettings, r_bool, m_light_section, "volumetric_for_actor", 0));
			else
				flashlight_render->set_volumetric(!!READ_IF_EXISTS(pSettings, r_bool, m_light_section, "volumetric", 0));

			flashlight_render->set_volumetric_quality(READ_IF_EXISTS(pSettings, r_float, m_light_section, "volumetric_quality", 1.f));
			flashlight_render->set_volumetric_intensity(READ_IF_EXISTS(pSettings, r_float, m_light_section, "volumetric_intensity", 1.f));
			flashlight_render->set_volumetric_distance(READ_IF_EXISTS(pSettings, r_float, m_light_section, "volumetric_distance", 1.f));

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

			const Fcolor oclr = READ_IF_EXISTS(pSettings, r_fcolor, m_light_section, b_r2 ? "omni_color_r2" : "omni_color", (Fcolor{ 1.0f , 1.0f , 1.0f , 0.0f }));
			flashlight_omni->set_color(oclr);
			const float orange = READ_IF_EXISTS(pSettings, r_float, m_light_section, b_r2 ? "omni_range_r2" : "omni_range", 0.25f);
			flashlight_omni->set_range(orange);

			flashlight_glow = ::Render->glow_create();
			flashlight_glow->set_texture(READ_IF_EXISTS(pSettings, r_string, m_light_section, "glow_texture", "glow\\glow_torch_r2"));
			flashlight_glow->set_color(clr);
			flashlight_glow->set_radius(READ_IF_EXISTS(pSettings, r_float, m_light_section, "glow_radius", 0.3f));
		}
		else
		{
			Msg("! [%s]: No valid tactical torch section found. Weapon section: [%s]", __FUNCTION__, section);
		}
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
	UpdateAltAimZoomFactor();
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
	if (lfo_scope_type == 4)//LFO 3d Scopes
	{

		if (lfo_scope_type == 2)//LFO Open 2d Scopes
		{
			if (pSettings->line_exist(section, "scope_texture_for_3d_open"))
			{
				scope_tex_name = pSettings->r_string(section, "scope_texture_for_3d_open");
				if (xr_strcmp(scope_tex_name, "none") != 0)
					bScopeIsHasTexture = true;
			}
		}
		else
		{
			if (pSettings->line_exist(section, "scope_texture_for_3d"))
			{
				scope_tex_name = pSettings->r_string(section, "scope_texture_for_3d");
				if (xr_strcmp(scope_tex_name, "none") != 0)
					bScopeIsHasTexture = true;
			}
		}
	}
	else
	{
		if (lfo_scope_type == 2)//LFO Open 2d Scopes
		{
			if (pSettings->line_exist(section, "scope_texture_open"))
			{
				scope_tex_name = pSettings->r_string(section, "scope_texture_open");
				if (xr_strcmp(scope_tex_name, "none") != 0)
					bScopeIsHasTexture = true;
			}
		}
		else
		{
			if (pSettings->line_exist(section, "scope_texture"))
			{
				scope_tex_name = pSettings->r_string(section, "scope_texture");
				if (xr_strcmp(scope_tex_name, "none") != 0)
					bScopeIsHasTexture = true;
			}
		}
	}


	if (lfo_scope_type == 3)// SHOKER 3d Scopes PiP
	{
		if (lfo_scope_type == 2)//LFO Open 2d Scopes
		{
			if (pSettings->line_exist(section, "scope_texture_open"))
			{
				scope_tex_name = pSettings->r_string(section, "scope_texture_open");
				if (xr_strcmp(scope_tex_name, "none") != 0)
					bScopeIsHasTexture = true;
			}
		}
		else
		{
			if (pSettings->line_exist(section, "scope_texture"))
			{
				scope_tex_name = pSettings->r_string(section, "scope_texture");
				if (xr_strcmp(scope_tex_name, "none") != 0)
					bScopeIsHasTexture = true;
			}
		}
	}

	string256 attach_sect;
	strconcat(sizeof(attach_sect), attach_sect, m_eScopeStatus == ALife::eAddonPermanent ? "scope" : m_scopes[m_cur_scope].c_str(), "_attach_sect");

	if (attach_sect && pSettings->line_exist(m_section_id.c_str(), attach_sect))
		m_sScopeAttachSection = READ_IF_EXISTS(pSettings, r_string, m_section_id.c_str(), attach_sect, "");

	if (lfo_scope_type == 4)//LFO 3d Scopes
	{
		m_bAltZoomEnabled = READ_IF_EXISTS(pSettings, r_bool, section, "enable_alternative_aim", false);
	}

	if (lfo_scope_type != 4)//LFO 3d Scopes
	{
		m_zoom_params.m_fScopeZoomFactor = pSettings->r_float(section, "scope_zoom_factor");

		Load3DScopeParams(section);

		if (bIsSecondVPZoomPresent())
		{
			bScopeIsHasTexture = false;
			m_zoom_params.m_fScopeZoomFactor = pSettings->r_float(section, "scope_zoom_factor_3d_pip");
		}
		else
		{
			m_zoom_params.m_fScopeZoomFactor = pSettings->r_float(section, "scope_zoom_factor");
		}
	}
	else
	{
		m_zoom_params.m_fScopeZoomFactor = pSettings->r_float(section, "scope_zoom_factor_3d");
	}

	if (bScopeIsHasTexture || bIsSecondVPZoomPresent())
	{
		if (bIsSecondVPZoomPresent())
			bNVsecondVPavaible = !!pSettings->line_exist(section, "scope_nightvision");
		else
			m_zoom_params.m_sUseZoomPostprocess = READ_IF_EXISTS(pSettings, r_string, section, "scope_nightvision", 0);

		m_zoom_params.m_bUseDynamicZoom = READ_IF_EXISTS(pSettings, r_bool, section, "scope_zoom_dynamic", FALSE);

		if (m_zoom_params.m_bUseDynamicZoom)
		{
			m_fZoomStepCount = READ_IF_EXISTS(pSettings, r_u8, section, "scope_zoom_steps", 3.0f);
			m_fZoomMinKoeff = READ_IF_EXISTS(pSettings, r_u8, section, "scope_zoom_min_koeff", 0.3f);
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
	if (lfo_scope_type == 3)// SHOKER 3d Scopes PiP
	{
		m_zoom_params.m_fSecondVPFovFactor = READ_IF_EXISTS(pSettings, r_float, section, "scope_zoom_3d_fov", 0.0f);
		m_zoom_params.m_f3dZoomFactor = READ_IF_EXISTS(pSettings, r_float, section, "scope_zoom_factor_3d", 100.0f);
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
	P.w_u16					(m_flagsAddOnState);
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

	u16						NewAddonState;
	P.r_u16					(NewAddonState);

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

	save_data		(m_sLaserName,					output_packet);
	save_data		(m_sLaserAttachSection,			output_packet);
	save_data		(m_sTacticalTorchName,			output_packet);
	save_data		(m_sTacticalTorchAttachSection,	output_packet);

	/*		*/
	save_data		(m_sStockName,					output_packet);
	save_data		(m_sStockAttachSection,			output_packet);
	save_data		(m_sGripName,					output_packet);
	save_data		(m_sGripAttachSection,			output_packet);
	save_data		(m_sGripvName,					output_packet);
	save_data		(m_sGripvAttachSection,			output_packet);

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

	load_data		(m_sLaserName,					input_packet);
	load_data		(m_sLaserAttachSection,			input_packet);
	load_data		(m_sTacticalTorchName,			input_packet);
	load_data		(m_sTacticalTorchAttachSection, input_packet);

	/*		*/
	load_data		(m_sStockName,					input_packet);
	load_data		(m_sStockAttachSection,			input_packet);
	load_data		(m_sGripName,					input_packet);
	load_data		(m_sGripAttachSection,			input_packet);
	load_data		(m_sGripvName,					input_packet);
	load_data		(m_sGripvAttachSection,			input_packet);

}


void CWeapon::OnEvent(NET_Packet& P, u16 type) 
{
	switch (type)
	{
	case GE_ADDON_CHANGE:
		{
			P.r_u16					(m_flagsAddOnState);
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

	if (H_Parent() == Level().CurrentEntity())
	{
		if (CActor* pA = smart_cast<CActor*>(H_Parent()))
		{
			if (!fis_zero(m_fWeaponOverheating))
			{
				m_fWeaponOverheating -= m_fWeaponOverheatingDec;
				clamp(m_fWeaponOverheating, 0.0f, 1.0f);

				if (this == pA->inventory().ActiveItem() && m_fWeaponOverheating >= 0.5f)
				{
					Fvector vel{};
					PHGetLinearVell(vel);
					StartOverheatingParticles(get_LastOSP(), vel);
					UpdateOverheatingParticles();
				}
				else
					StopOverheatingParticles();
			}
			else
			{
				if (this == pA->inventory().ActiveItem())
					StopOverheatingParticles();
			}

			if (this == pA->inventory().ActiveItem())
				g_pGamePersistent->devices_shader_data.cur_weapon_overheating = m_fWeaponOverheating;
		}
	}
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

void CWeapon::on_a_hud_attach()
{
	inherited::on_a_hud_attach();

	if (m_sSafetyBoneName.size())
		SetHudSafetyBoneCallback();
}

void CWeapon::on_b_hud_detach()
{
	inherited::on_b_hud_detach();

	if (m_sSafetyBoneName.size())
		ResetHudSafetyBoneCallback();
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
	UpdateOverheatLights	();
	UpdateAddonsBlocks		();

	UpdateGLAttached();
	UpdateAltAimZoomFactor();
	UpdateAltAimZoomFactor2();

	//нарисовать партиклы
	UpdateFlameParticles	();
	UpdateFlameParticles2	();
	UpdateOverheatingAfterShootParticles();

	if(!IsGameTypeSingle())
		make_Interpolation		();

	if (m_sSafetyBoneName.size())
		UpdateSafetyRotation();
	
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
	
	if (IsGrenadeMode())
	{
		if (m_BlendAimIdleGL_Cam.name.size() && !g_player_hud->IsBlendAnmActive(m_BlendAimStartGL_Cam.name.c_str()) && !g_player_hud->IsBlendAnmActive(m_BlendAimIdleGL_Cam.name.c_str()) && GetHUDmode() && IsZoomed())
		{
			if (m_BlendAimStartGL_Cam.name.size())
				g_player_hud->StopBlendAnm(m_BlendAimStartGL_Cam.name.c_str());

			g_player_hud->PlayBlendAnm(m_BlendAimIdleGL_Cam.name.c_str(), 2, m_BlendAimIdleGL_Cam.speed, m_BlendAimIdleGL_Cam.power, true, false);
		}
	}
	else
	{
		if (m_BlendAimIdleCam.name.size() && !g_player_hud->IsBlendAnmActive(m_BlendAimStartCam.name.c_str()) && !g_player_hud->IsBlendAnmActive(m_BlendAimIdleCam.name.c_str()) && GetHUDmode() && IsZoomed())
		{
			if (m_BlendAimStartCam.name.size())
				g_player_hud->StopBlendAnm(m_BlendAimStartCam.name.c_str());

			g_player_hud->PlayBlendAnm(m_BlendAimIdleCam.name.c_str(), 2, m_BlendAimIdleCam.speed, m_BlendAimIdleCam.power, true, false);
		}
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

void CWeapon::UpdateAddonsBlocks()
{
	if (IsLaserAttached())
	{
		if (m_sLaserAttachSection.size())
			m_bFlashlightBlockedByAddon = READ_IF_EXISTS(pSettings, r_bool, m_sLaserAttachSection, "block_flashlight", false);
		else
			m_bFlashlightBlockedByAddon = READ_IF_EXISTS(pSettings, r_bool, cNameSect(), "laser_block_flashlight", false);
	}
	else
		m_bFlashlightBlockedByAddon = false;

	if (IsTacticalTorchAttached())
	{
		if (m_sTacticalTorchAttachSection.size())
			m_bLaserBlockedByAddon = READ_IF_EXISTS(pSettings, r_bool, m_sTacticalTorchAttachSection, "block_laser", false);
		else
			m_bLaserBlockedByAddon = READ_IF_EXISTS(pSettings, r_bool, cNameSect(), "flashlight_block_laser", false);
	}
	else
		m_bLaserBlockedByAddon = false;


	if (IsGripAttached())
	{
		if (m_sGripAttachSection.size())
			m_bGripVBlockedByAddon = READ_IF_EXISTS(pSettings, r_bool, m_sGripAttachSection, "block_grip", false);
		else
			m_bGripVBlockedByAddon = READ_IF_EXISTS(pSettings, r_bool, cNameSect(), "grip_vertical_block_grip_horizontal", false);
	}
	else
		m_bGripVBlockedByAddon = false;

	if (IsGripvAttached())
	{
		if (m_sGripvAttachSection.size())
			m_bGripHBlockedByAddon = READ_IF_EXISTS(pSettings, r_bool, m_sGripvAttachSection, "block_grip", false);
		else
			m_bGripHBlockedByAddon = READ_IF_EXISTS(pSettings, r_bool, cNameSect(), "grip_horizontal_block_grip_vertical", false);
	}
	else
		m_bGripHBlockedByAddon = false;
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

	if (!GetHUDmode())
		UpdateAddonsTransform(false);

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
			if (IsZoomEnabled())
			{
				if (b_toggle_weapon_aim)
				{
					if (flags & CMD_START)
					{
						if (!IsZoomed())
						{
							if (!IsPending())
							{
								if (GetState() != eIdle)
									if (psActorFlags.test(AF_WPN_ZOOM_OUT_SHOOT))
									{
										if (IsOutScopeAfterShot() && GetState() != eFire) // Чтобы не глючила для продолжительных анимаций, не знаю зачем тут этот стейт
										{
											SwitchState(eIdle);
										}
									}
									else
									{
										SwitchState(eIdle);
									}
								if (IsOutScopeAfterShot())
								{
									if (GetState() != eFire)
									{
										OnZoomIn();
									}
								}
								else
								{
									OnZoomIn();
								}
							}
						}
						else
							OnZoomOut();
					}
				}
				else
				{
					if (flags & CMD_START)
					{
						if (!IsZoomed() && !IsPending())
						{
							if (GetState() != eIdle)

								if (psActorFlags.test(AF_WPN_ZOOM_OUT_SHOOT))
								{
									if (IsOutScopeAfterShot() && GetState() != eFire) // Чтобы не глючила для продолжительных анимаций, не знаю зачем тут этот стейт
									{
										SwitchState(eIdle);
									}
								}
								else
								{
									SwitchState(eIdle);
								}
							OnZoomIn();
						}
					}
					else
						if (IsZoomed())
							OnZoomOut();
				}
				return true;
			}
			else
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
	if (OnClient())
		return FALSE;

	if (m_bWpnExplosion && (GetCondition() < 0.2f) && (::Random.randF(0.0f, 1.0f) < m_fWpnExplodeChance))
	{
		WpnExplosion();

		return FALSE;
	}

	float rnd = ::Random.randF(0.f,1.f);
	float mp = GetConditionMisfireProbability();
	mp += m_fOverheatingMisfire * m_fWeaponOverheating;

	if ((rnd < mp) && (!m_bMisfireBulletRemove || iAmmoElapsed > 1))
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
	if (!psActorFlags3.test(AF_LFO_WPN_AIM_RELOADS))
	{
		OnZoomOut();
	}

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

bool CWeapon::IsStockAttached() const
{
	return (ALife::eAddonAttachable == m_eStockStatus &&
		0 != (m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonStock)) ||
		ALife::eAddonPermanent == m_eStockStatus;
}

bool CWeapon::IsGripAttached() const
{
	return (ALife::eAddonAttachable == m_eGripStatus &&
		0 != (m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonGrip)) ||
		ALife::eAddonPermanent == m_eGripStatus;
}

bool CWeapon::IsGripvAttached() const
{
	return (ALife::eAddonAttachable == m_eGripvStatus &&
		0 != (m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonGripv)) ||
		ALife::eAddonPermanent == m_eGripvStatus;
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
bool CWeapon::StockAttachable()
{
	return (ALife::eAddonAttachable == m_eStockStatus);
}
bool CWeapon::GripAttachable()
{
	return (ALife::eAddonAttachable == m_eGripStatus);
}
bool CWeapon::GripvAttachable()
{
	return (ALife::eAddonAttachable == m_eGripvStatus);
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
	if (!GetHUDmode())										return;

	//.	return;

	u16 bone_id = HudItemData()->m_model->LL_BoneID(wpn_scope_def_bone);

	auto SetBoneVisible = [&](const shared_str& boneName, BOOL visibility)
		{
			HudItemData()->set_bone_visible(boneName, visibility, TRUE);
		};

	//---------------------------------------------------------------------------
	//---------------------------------------------------------------------------
		// Hide default bones
	for (const shared_str& bone : m_defHiddenBones)
	{
		SetBoneVisible(bone, FALSE);
	}

	for (const shared_str& bone : m_defHiddenBonesScope)
	{
		if (IsScopeAttached())
		{
			SetBoneVisible(bone, FALSE);
		}
	}

	if (IsSilencerAttached())
	{
		for (const shared_str& bone : m_defSilHiddenBones)
		{
			SetBoneVisible(bone, FALSE);
		}
	}

	for (const shared_str& bone : m_upgHideBones)
	{
		SetBoneVisible(bone, FALSE);
	}
	//---------------------------------------------------------------------------
		// Show default bones
	for (const shared_str& bone : m_defShownBones)
	{
		if (!IsScopeAttached())
		{
			SetBoneVisible(bone, TRUE);
		}
		else
		{
			SetBoneVisible(bone, FALSE);
		}
	}

	for (const shared_str& bone : m_defOverheatinBarrel)
	{
		if (m_fWeaponOverheating >= 0.5f)
		{
		//	Msg("!NO OVERHEATING SHOW BONES");
			SetBoneVisible(bone, TRUE);
		}
		else
		{
		//	Msg("!NO OVERHEATING HIDE BONES");
			SetBoneVisible(bone, FALSE);
		}
	}

	for (const shared_str& bone : m_deleteStockBone)
	{
		if (!IsStockAttached())
		{
			SetBoneVisible(bone, TRUE);
		}
		else
		{
			SetBoneVisible(bone, FALSE);
		}
	}

	for (const shared_str& bone : m_defShellBones)
	{
		if (psActorFlags3.test(AF_LFO_SHOT_SHELL_ANIMATIONS))
		{
			if (GetState() != eFire)
			{
				SetBoneVisible(bone, FALSE);
			}
			else
			{
				SetBoneVisible(bone, TRUE);
			}
		}
		else
		{
			SetBoneVisible(bone, FALSE);
		}

	}

	for (const shared_str& bone : m_defShownBonesScope)
	{
		if (IsScopeAttached())
		{
			SetBoneVisible(bone, TRUE);
		}
		else
		{
			SetBoneVisible(bone, FALSE);
		}
	}

	for (const shared_str& bone : m_upgShowBones)
	{
		SetBoneVisible(bone, TRUE);
	}

	if (lfo_scope_type == 4)//LFO 3d Scopes
	{
		// for lfo scope system and weapon with permament scope
		bool WeaponHavePermanentScope = false;

		if (pSettings->line_exist(m_section_id.c_str(), "weapon_have_permanent_scope"))
			WeaponHavePermanentScope = READ_IF_EXISTS(pSettings, r_bool, m_section_id, "weapon_have_permanent_scope", false);

		for (const shared_str& bone : m_defShownBonesScopePermanent)
		{
			if (WeaponHavePermanentScope)
			{
				if (!IsZoomed() && GetZRotatingFactor() < .9f)
				{
					if (!IsScopeAttached())
					{
						if (!IsScopeAttached())
						{
							SetBoneVisible(bone, TRUE);
						}
						else
						{
							SetBoneVisible(bone, FALSE);
						}
					}
					else
					{
						if (IsGrenadeMode())
						{
							SetBoneVisible(bone, TRUE);
						}
						else
						{
							SetBoneVisible(bone, FALSE);
						}
					}
				}
			}
		}

		for (const shared_str& bone : m_defHiddenBonesScopePermanent)
		{
			if (WeaponHavePermanentScope)
			{
				if (!IsZoomed() && GetZRotatingFactor() < .9f)
				{
					if (!IsScopeAttached())
					{
						SetBoneVisible(bone, TRUE);
					}
					else
					{
						SetBoneVisible(bone, FALSE);
					}
				}
				else
				{
					if (IsGrenadeMode())
					{
						SetBoneVisible(bone, TRUE);
					}
					else
					{
						SetBoneVisible(bone, FALSE);
					}
				}
			}
		}
	}

	bool WeaponHavePermanentScope2 = false;

	if (pSettings->line_exist(m_section_id.c_str(), "weapon_need_secundary_hide_bones"))
		WeaponHavePermanentScope2 = READ_IF_EXISTS(pSettings, r_bool, m_section_id, "weapon_need_secundary_hide_bones", false);

	for (const shared_str& bone : m_defHiddenBonesScopePermanent2)
	{
		if (WeaponHavePermanentScope2)
		{
			if (!IsZoomed() && GetZRotatingFactor() < .9f)
			{
				SetBoneVisible(bone, FALSE);
			}
			else
			{
				if (lfo_scope_type == 3)
				{
					SetBoneVisible(bone, FALSE);
				}
				else
				{
					SetBoneVisible(bone, TRUE);
				}
			}
		}
	}

	//---------------------------------------------------------------------------
	//---------------------------------------------------------------------------

	for (int i = 0; i < m_all_scope_bones.size(); i++)
		SetBoneVisible(m_all_scope_bones[i], FALSE);

	shared_str cur_scope_sect = (m_sScopeAttachSection.size() ? m_sScopeAttachSection : (m_eScopeStatus == ALife::eAddonAttachable) ? m_scopes[m_cur_scope].c_str() : "scope");

	bool UseScopeAimBone = false;
	if (pSettings->line_exist(cur_scope_sect, "scope_use_aim_bone"))
		UseScopeAimBone = READ_IF_EXISTS(pSettings, r_bool, cur_scope_sect, "scope_use_aim_bone", false);

	bool UseScopeRailBone = false;
	if (pSettings->line_exist(cur_scope_sect, "scope_use_rail_bone"))
		UseScopeRailBone = READ_IF_EXISTS(pSettings, r_bool, cur_scope_sect, "scope_use_rail_bone", false);

	bool UseScopeAltAimBone = false;
	if (pSettings->line_exist(cur_scope_sect, "scope_use_alt_aim_bones"))
		UseScopeAltAimBone = READ_IF_EXISTS(pSettings, r_bool, cur_scope_sect, "scope_use_alt_aim_bones", false);

	bool UseScopeAltAimBoneIronsight = false;
	if (pSettings->line_exist(cur_scope_sect, "scope_use_alt_aim_bones_ironsight"))
		UseScopeAltAimBoneIronsight = READ_IF_EXISTS(pSettings, r_bool, cur_scope_sect, "scope_use_alt_aim_bones_ironsight", false);

	bool WeaponNeedAltAimBoneIronsight = false;
	if (pSettings->line_exist(m_section_id.c_str(), "enable_alternative_aim_ironsight"))
		//	WeaponNeedAltAimBoneIronsight = READ_IF_EXISTS(pSettings, r_bool, m_section_id, "enable_alternative_aim_ironsight", false);
		WeaponNeedAltAimBoneIronsight = READ_IF_EXISTS(pSettings, r_bool, cur_scope_sect, "enable_alternative_aim_ironsight", false);

	if (!IsZoomed() && GetZRotatingFactor() < .9f)
		//	if (!IsZoomed() && !IsRotatingFromZoom())
	{
		// BONE FOR SCOPE IDLE MODEL
		if (m_cur_scope_bone != NULL)
			SetBoneVisible(m_cur_scope_bone, TRUE);

		// BONE FOR SCOPE IDLE MODEL
		if (m_cur_scope_bone_part != NULL)
			SetBoneVisible(m_cur_scope_bone_part, TRUE);

		if (lfo_scope_type == 3 && IsScopeAttached())// SHOKER 3d Scopes PiP
		{
			// BONE FOR SCOPE AIM MODEL
			if (m_cur_scope_bone_aim != NULL)
				SetBoneVisible(m_cur_scope_bone_aim, FALSE);

			// BONE FOR SCOPE AIM MODEL
			if (m_cur_scope_bone_aim_part != NULL)
				SetBoneVisible(m_cur_scope_bone_aim_part, FALSE);
		}
		else
		{
			if (UseScopeAimBone)
			{
				// BONE FOR SCOPE AIM MODEL
				if (m_cur_scope_bone_aim != NULL)
					SetBoneVisible(m_cur_scope_bone_aim, FALSE);

				if (m_cur_scope_bone_aim_part != NULL)
					SetBoneVisible(m_cur_scope_bone_aim_part, FALSE);
			}
			else
			{
				// BONE FOR SCOPE AIM MODEL
				if (m_cur_scope_bone_aim != NULL)
					SetBoneVisible(m_cur_scope_bone_aim, TRUE);

				if (m_cur_scope_bone_aim_part != NULL)
					SetBoneVisible(m_cur_scope_bone_aim_part, TRUE);
			}
		}

		if (UseScopeRailBone)
		{
			// BONE FOR Picatinny RAILS SHOW
			if (m_cur_scope_show_bone_rail != NULL)
				SetBoneVisible(m_cur_scope_show_bone_rail, TRUE);

			// BONE FOR Picatinny RAILS HIDE
			if (m_cur_scope_hide_bone_rail != NULL)
				SetBoneVisible(m_cur_scope_hide_bone_rail, FALSE);
		}
	}
	else
	{
		if (lfo_scope_type == 3 && IsScopeAttached())// SHOKER 3d Scopes PiP
		{

			// BONE FOR SCOPE IDLE MODEL
			if (m_cur_scope_bone != NULL)
				SetBoneVisible(m_cur_scope_bone, TRUE);

			if (m_cur_scope_bone_part != NULL)
				SetBoneVisible(m_cur_scope_bone_part, TRUE);

			// BONE FOR SCOPE AIM MODEL
			if (m_cur_scope_bone_aim != NULL)
				SetBoneVisible(m_cur_scope_bone_aim, FALSE);

			if (m_cur_scope_bone_aim_part != NULL)
				SetBoneVisible(m_cur_scope_bone_aim_part, FALSE);
		}
		else
		{
			if (UseScopeAimBone)
			{
				if (IsZoomed() && GetAltZoomStatus())		//WeaponNeedAltAimBoneIronsight
				{
					if (UseScopeAltAimBone)
					{
						// BONE FOR SCOPE IDLE MODEL
						if (m_cur_scope_bone != NULL)
							SetBoneVisible(m_cur_scope_bone, FALSE);

						if (m_cur_scope_bone_part != NULL)
							SetBoneVisible(m_cur_scope_bone_part, FALSE);

						// BONE FOR SCOPE AIM MODEL
						if (m_cur_scope_bone_aim != NULL)
							SetBoneVisible(m_cur_scope_bone_aim, TRUE);

						if (m_cur_scope_bone_aim_part != NULL)
							SetBoneVisible(m_cur_scope_bone_aim_part, TRUE);
						else
							// BONE FOR SCOPE IDLE MODEL
							if (m_cur_scope_bone != NULL)
								SetBoneVisible(m_cur_scope_bone, TRUE);

						if (m_cur_scope_bone_part != NULL)
							SetBoneVisible(m_cur_scope_bone_part, TRUE);

						// BONE FOR SCOPE AIM MODEL
						if (m_cur_scope_bone_aim != NULL)
							SetBoneVisible(m_cur_scope_bone_aim, TRUE);

						if (m_cur_scope_bone_aim_part != NULL)
							SetBoneVisible(m_cur_scope_bone_aim_part, TRUE);
					}
					else
					{
						// BONE FOR SCOPE IDLE MODEL
						if (m_cur_scope_bone != NULL)
							SetBoneVisible(m_cur_scope_bone, TRUE);

						if (m_cur_scope_bone_part != NULL)
							SetBoneVisible(m_cur_scope_bone_part, TRUE);

						// BONE FOR SCOPE AIM MODEL
						if (m_cur_scope_bone_aim != NULL)
							SetBoneVisible(m_cur_scope_bone_aim, FALSE);

						if (m_cur_scope_bone_aim_part != NULL)
							SetBoneVisible(m_cur_scope_bone_aim_part, FALSE);
						else
							// BONE FOR SCOPE IDLE MODEL
							if (m_cur_scope_bone != NULL)
								SetBoneVisible(m_cur_scope_bone, FALSE);

						if (m_cur_scope_bone_part != NULL)
							SetBoneVisible(m_cur_scope_bone_part, FALSE);

						// BONE FOR SCOPE AIM MODEL
						if (m_cur_scope_bone_aim != NULL)
							SetBoneVisible(m_cur_scope_bone_aim, FALSE);

						if (m_cur_scope_bone_aim_part != NULL)
							SetBoneVisible(m_cur_scope_bone_aim_part, FALSE);
						//}
					}

					if (UseScopeAltAimBoneIronsight)
					{
						// BONE FOR SCOPE IDLE MODEL
						if (m_cur_scope_bone != NULL)
							SetBoneVisible(m_cur_scope_bone, TRUE);

						// BONE FOR SCOPE AIM MODEL
						if (m_cur_scope_bone_aim != NULL)
							SetBoneVisible(m_cur_scope_bone_aim, FALSE);
					}
				}
				else
				{
					if (IsGrenadeMode())
					{
						// BONE FOR SCOPE IDLE MODEL
						if (m_cur_scope_bone != NULL)
							SetBoneVisible(m_cur_scope_bone, TRUE);

						if (m_cur_scope_bone_part != NULL)
							SetBoneVisible(m_cur_scope_bone_part, FALSE);

						// BONE FOR SCOPE AIM MODEL
						if (m_cur_scope_bone_aim != NULL)
							SetBoneVisible(m_cur_scope_bone_aim,FALSE);

						if (m_cur_scope_bone_aim_part != NULL)
							SetBoneVisible(m_cur_scope_bone_aim_part, FALSE);
					}
					else
					{
						// BONE FOR SCOPE IDLE MODEL
						if (m_cur_scope_bone != NULL)
							SetBoneVisible(m_cur_scope_bone, FALSE);

						if (m_cur_scope_bone_part != NULL)
							SetBoneVisible(m_cur_scope_bone_part, FALSE);

						// BONE FOR SCOPE AIM MODEL
						if (m_cur_scope_bone_aim != NULL)
							SetBoneVisible(m_cur_scope_bone_aim, TRUE);

						if (m_cur_scope_bone_aim_part != NULL)
							SetBoneVisible(m_cur_scope_bone_aim_part, TRUE);
					}
				}
			}
			else
			{
				// BONE FOR SCOPE IDLE MODEL
				if (m_cur_scope_bone != NULL)
					SetBoneVisible(m_cur_scope_bone, TRUE);

				if (m_cur_scope_bone_part != NULL)
					SetBoneVisible(m_cur_scope_bone_part, TRUE);

				// BONE FOR SCOPE AIM MODEL
				if (m_cur_scope_bone_aim != NULL)
					SetBoneVisible(m_cur_scope_bone_aim, TRUE);

				if (m_cur_scope_bone_aim_part != NULL)
					SetBoneVisible(m_cur_scope_bone_aim_part, TRUE);
			}
		}

		if (UseScopeRailBone)
		{
			// BONE FOR Picatinny RAILS SHOW
			if (m_cur_scope_show_bone_rail != NULL)
				SetBoneVisible(m_cur_scope_show_bone_rail, TRUE);

			// BONE FOR Picatinny RAILS HIDE
			if (m_cur_scope_hide_bone_rail != NULL)
				SetBoneVisible(m_cur_scope_hide_bone_rail, FALSE);
		}
	}

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
		SetBoneVisible(m_sWpn_silencer_bone, IsSilencerAttached());
	}
	if (m_eSilencerStatus == ALife::eAddonDisabled)
	{
		SetBoneVisible(m_sWpn_silencer_bone, FALSE);
	}
	else
		if (m_eSilencerStatus == ALife::eAddonPermanent)
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

	if (StockAttachable())
	{
		SetBoneVisible(m_sWpn_stock_bone, IsStockAttached() && !m_sStockAttachSection.size());
	}
	if (m_eStockStatus == ALife::eAddonDisabled || m_sStockAttachSection.size())
	{
		SetBoneVisible(m_sWpn_stock_bone, FALSE);
	}
	else if (m_eStockStatus == ALife::eAddonPermanent && !m_sStockAttachSection.size())
		SetBoneVisible(m_sWpn_stock_bone, TRUE);

	if (GripAttachable())
	{
		SetBoneVisible(m_sWpn_grip_bone, IsGripAttached() && !m_sGripAttachSection.size());
	}
	if (m_eGripStatus == ALife::eAddonDisabled || m_sGripAttachSection.size())
	{
		SetBoneVisible(m_sWpn_grip_bone, FALSE);
	}
	else if (m_eGripStatus == ALife::eAddonPermanent && !m_sGripAttachSection.size())
		SetBoneVisible(m_sWpn_grip_bone, TRUE);

	if (GripvAttachable())
	{
		SetBoneVisible(m_sWpn_gripv_bone, IsGripvAttached() && !m_sGripvAttachSection.size());
	}
	if (m_eGripvStatus == ALife::eAddonDisabled || m_sGripvAttachSection.size())
	{
		SetBoneVisible(m_sWpn_gripv_bone, FALSE);
	}
	else if (m_eGripvStatus == ALife::eAddonPermanent && !m_sGripvAttachSection.size())
		SetBoneVisible(m_sWpn_gripv_bone, TRUE);

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

void CWeapon::UpdateAddonsVisibility(IKinematics* visual)
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

	//---------------------------------------------------------------------------
	//---------------------------------------------------------------------------
		// Hide default bones
	for (const shared_str& bone : m_defHiddenBones)
	{
		SetBoneVisible(bone, FALSE);
	}

	for (const shared_str& bone : m_defHiddenBonesScope)
	{
		SetBoneVisible(bone, FALSE);
	}

	if (IsSilencerAttached())
	{
		for (const auto& boneName : m_defSilHiddenBones)
		{
			bone_id = pWeaponVisual->LL_BoneID(boneName);
			if (bone_id != BI_NONE && pWeaponVisual->LL_GetBoneVisible(bone_id))
				pWeaponVisual->LL_SetBoneVisible(bone_id, FALSE, TRUE);
		}
	}

	for (const auto& boneName : m_upgHideBones)
	{
		bone_id = pWeaponVisual->LL_BoneID(boneName);
		if (bone_id != BI_NONE)
			pWeaponVisual->LL_SetBoneVisible(bone_id, FALSE, TRUE);
	}
	//---------------------------------------------------------------------------
		// Show default bones
	for (const shared_str& bone : m_defShownBones)
	{
		SetBoneVisible(bone, TRUE);
	}

	for (const shared_str& bone : m_defShownBonesScope)
	{
		SetBoneVisible(bone, TRUE);
	}

	for (const auto& boneName : m_upgShowBones)
	{
		bone_id = pWeaponVisual->LL_BoneID(boneName);
		if (bone_id != BI_NONE)
			pWeaponVisual->LL_SetBoneVisible(bone_id, TRUE, TRUE);
	}
	//---------------------------------------------------------------------------
	//---------------------------------------------------------------------------

	for (int i = 0; i < m_all_scope_bones.size(); i++)
		SetBoneVisible(m_all_scope_bones[i], FALSE);

	bool UseScopeAimBone = false;
	bool UseScopeRailBone = false;

	if (!IsZoomed() && !IsRotatingFromZoom())
	{
		// BONE FOR SCOPE IDLE MODEL
		if (m_cur_scope_bone != NULL)
			SetBoneVisible(m_cur_scope_bone, TRUE);

		if (lfo_scope_type == 3 && IsScopeAttached())// SHOKER 3d Scopes PiP
		{
			// BONE FOR SCOPE AIM MODEL
			if (m_cur_scope_bone_aim != NULL)
				SetBoneVisible(m_cur_scope_bone_aim, FALSE);

		}
		else
		{
			if (UseScopeAimBone)
			{
				// BONE FOR SCOPE AIM MODEL
				if (m_cur_scope_bone_aim != NULL)
					SetBoneVisible(m_cur_scope_bone_aim, TRUE);

			}
			else
			{
				// BONE FOR SCOPE AIM MODEL
				if (m_cur_scope_bone_aim != NULL)
					SetBoneVisible(m_cur_scope_bone_aim, TRUE);
			}
		}

		if (UseScopeRailBone)
		{
			// BONE FOR Picatinny RAILS SHOW
			if (m_cur_scope_show_bone_rail != NULL)
				SetBoneVisible(m_cur_scope_show_bone_rail, TRUE);

			// BONE FOR Picatinny RAILS HIDE
			if (m_cur_scope_hide_bone_rail != NULL)
				SetBoneVisible(m_cur_scope_hide_bone_rail, FALSE);
		}
	}
	else
	{
		if (lfo_scope_type == 3 && IsScopeAttached())// SHOKER 3d Scopes PiP
		{

			// BONE FOR SCOPE IDLE MODEL
			if (m_cur_scope_bone != NULL)
				SetBoneVisible(m_cur_scope_bone, TRUE);

			// BONE FOR SCOPE AIM MODEL
			if (m_cur_scope_bone_aim != NULL)
				SetBoneVisible(m_cur_scope_bone_aim, FALSE);
		}
		else
		{
			if (UseScopeAimBone)
			{
				// BONE FOR SCOPE IDLE MODEL
				if (m_cur_scope_bone != NULL)
					SetBoneVisible(m_cur_scope_bone, FALSE);

				// BONE FOR SCOPE AIM MODEL
				if (m_cur_scope_bone_aim != NULL)
					SetBoneVisible(m_cur_scope_bone_aim, TRUE);

			}
			else
			{
				// BONE FOR SCOPE IDLE MODEL
				if (m_cur_scope_bone != NULL)
					SetBoneVisible(m_cur_scope_bone, TRUE);

				// BONE FOR SCOPE AIM MODEL
				if (m_cur_scope_bone_aim != NULL)
					SetBoneVisible(m_cur_scope_bone_aim, TRUE);
			}
		}

		if (UseScopeRailBone)
		{
			// BONE FOR Picatinny RAILS SHOW
			if (m_cur_scope_show_bone_rail != NULL)
				SetBoneVisible(m_cur_scope_show_bone_rail, TRUE);

			// BONE FOR Picatinny RAILS HIDE
			if (m_cur_scope_hide_bone_rail != NULL)
				SetBoneVisible(m_cur_scope_hide_bone_rail, FALSE);
		}
	}

	if (ScopeAttachable())
	{
		if (IsScopeAttached() && !m_sScopeAttachSection.size())
		{
			if (bone_id != BI_NONE && !pWeaponVisual->LL_GetBoneVisible(bone_id))
				pWeaponVisual->LL_SetBoneVisible(bone_id, TRUE, TRUE);
		}
		else
		{
			if (bone_id != BI_NONE && pWeaponVisual->LL_GetBoneVisible(bone_id))
				pWeaponVisual->LL_SetBoneVisible(bone_id, FALSE, TRUE);
		}
	}
	if ((m_eScopeStatus == ALife::eAddonDisabled || m_sScopeAttachSection.size()) && bone_id != BI_NONE &&
		pWeaponVisual->LL_GetBoneVisible(bone_id))
	{
		pWeaponVisual->LL_SetBoneVisible(bone_id, FALSE, TRUE);
		//		Log("scope", pWeaponVisual->LL_GetBoneVisible		(bone_id));
	}

	bone_id = pWeaponVisual->LL_BoneID(m_sWpn_silencer_bone);

	if (SilencerAttachable())
	{
		if (IsSilencerAttached())
		{
			if (!pWeaponVisual->LL_GetBoneVisible(bone_id))
				pWeaponVisual->LL_SetBoneVisible(bone_id, TRUE, TRUE);
		}
		else
		{
			if (pWeaponVisual->LL_GetBoneVisible(bone_id))
				pWeaponVisual->LL_SetBoneVisible(bone_id, FALSE, TRUE);
		}
	}
	if (m_eSilencerStatus == ALife::eAddonDisabled && bone_id != BI_NONE &&
		pWeaponVisual->LL_GetBoneVisible(bone_id))
	{
		pWeaponVisual->LL_SetBoneVisible(bone_id, FALSE, TRUE);
		//		Log("silencer", pWeaponVisual->LL_GetBoneVisible	(bone_id));
	}

	bone_id = pWeaponVisual->LL_BoneID(m_sWpn_launcher_bone);
	if (GrenadeLauncherAttachable())
	{
		if (IsGrenadeLauncherAttached() && !m_sGrenadeLauncherAttachSection.size())
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
	if ((m_eGrenadeLauncherStatus == ALife::eAddonDisabled || m_sGrenadeLauncherAttachSection.size()) && bone_id != BI_NONE &&
		pWeaponVisual->LL_GetBoneVisible(bone_id))
	{
		pWeaponVisual->LL_SetBoneVisible(bone_id, FALSE, TRUE);
		//		Log("gl", pWeaponVisual->LL_GetBoneVisible			(bone_id));
	}

	bone_id = pWeaponVisual->LL_BoneID(m_sWpn_grip_bone);
	if (GripAttachable())
	{
		if (IsGripAttached() && !m_sGripAttachSection.size())
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
	if ((m_eGripStatus == ALife::eAddonDisabled || m_sGripAttachSection.size()) && bone_id != BI_NONE &&
		pWeaponVisual->LL_GetBoneVisible(bone_id))
	{
		pWeaponVisual->LL_SetBoneVisible(bone_id, FALSE, TRUE);
		//		Log("gl", pWeaponVisual->LL_GetBoneVisible			(bone_id));
	}

	bone_id = pWeaponVisual->LL_BoneID(m_sWpn_gripv_bone);
	if (GripvAttachable())
	{
		if (IsGripvAttached() && !m_sGripvAttachSection.size())
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
	if ((m_eGripvStatus == ALife::eAddonDisabled || m_sGripvAttachSection.size()) && bone_id != BI_NONE &&
		pWeaponVisual->LL_GetBoneVisible(bone_id))
	{
		pWeaponVisual->LL_SetBoneVisible(bone_id, FALSE, TRUE);
		//		Log("gl", pWeaponVisual->LL_GetBoneVisible			(bone_id));
	}

	bone_id = pWeaponVisual->LL_BoneID(m_sWpn_stock_bone);
	if (StockAttachable())
	{
		if (IsStockAttached() && !m_sStockAttachSection.size())
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
	if ((m_eStockStatus == ALife::eAddonDisabled || m_sStockAttachSection.size()) && bone_id != BI_NONE && pWeaponVisual->LL_GetBoneVisible(bone_id))
	{
		pWeaponVisual->LL_SetBoneVisible(bone_id, FALSE, TRUE);
		//		Log("laser", pWeaponVisual->LL_GetBoneVisible	(bone_id));
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

	pWeaponVisual->CalculateBones_Invalidate();
	pWeaponVisual->CalculateBones(TRUE);
}


void CWeapon::InitAddons()
{
}

float CWeapon::CurrentZoomFactor()
{
	if (lfo_scope_type == 3 && IsScopeAttached())// SHOKER 3d Scopes PiP
	{
		return m_bAltZoomActive ? m_zoom_params.m_fAltAimZoomFactor : GameConstants::GetOGSE_WpnZoomSystem() ? 1.0f : bIsSecondVPZoomPresent() ? m_zoom_params.m_f3dZoomFactor : m_zoom_params.m_fScopeZoomFactor; // no change to main fov zoom when use second vp
	}
	else
	{
		return (IsScopeAttached()) ? !m_bAltZoomActive ? m_zoom_params.m_fScopeZoomFactor : m_zoom_params.m_fAltAimZoomFactor : m_zoom_params.m_fIronSightZoomFactor;
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

	EnableHudInertion(g_use_aim_inertion);

	//if(m_zoom_params.m_bZoomDofEnabled && !IsScopeAttached())
	//	GamePersistent().SetEffectorDOF	(m_zoom_params.m_ZoomDof);

	if (GetHUDmode())
	{
		last_hud_fov = psHUD_FOV_def;
		GamePersistent().SetPickableEffectorDOF(true);
		UpdateAimOffsets();
		UpdateAimFOV();
		UpdateAltAimZoomFactor();
		UpdateHudAltAimHud();
		UpdateHudAltAimHudMode2();

		if (IsGrenadeMode())
		{
			if (m_BlendAimStartGL_Cam.name.size())
				g_player_hud->PlayBlendAnm(m_BlendAimStartGL_Cam.name.c_str(), 2, m_BlendAimStartGL_Cam.speed, m_BlendAimStartGL_Cam.power, false, false);
		}
		else
		{
			if (m_BlendAimStartCam.name.size())
				g_player_hud->PlayBlendAnm(m_BlendAimStartCam.name.c_str(), 2, m_BlendAimStartCam.speed, m_BlendAimStartCam.power, false, false);
		}
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
	if (psActorFlags3.test(AF_LFO_WPN_MOVEMENT_LAYER))
	{
		g_player_hud->updateMovementLayerState();
	}
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

	if (!bIsSecondVPZoomPresent() || lfo_scope_type != 3)// SHOKER 3d Scopes PiP
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
		UpdateHudAltAimHud();
		UpdateHudAltAimHudMode2();

		if (m_BlendAimEndCam.name.size())
		{
			if (m_BlendAimIdleCam.name.size())
				g_player_hud->StopBlendAnm(m_BlendAimIdleCam.name.c_str());

			g_player_hud->PlayBlendAnm(m_BlendAimEndCam.name.c_str(), 2, m_BlendAimEndCam.speed, m_BlendAimEndCam.power, false, false);
		}

		if (IsGrenadeMode())
		{
			if (m_BlendAimEndGL_Cam.name.size())
			{
				if (m_BlendAimIdleGL_Cam.name.size())
					g_player_hud->StopBlendAnm(m_BlendAimIdleGL_Cam.name.c_str());

				g_player_hud->PlayBlendAnm(m_BlendAimEndGL_Cam.name.c_str(), 2, m_BlendAimEndGL_Cam.speed, m_BlendAimEndGL_Cam.power, false, false);
			}
		}
		else
		{
			if (m_BlendAimEndCam.name.size())
			{
				if (m_BlendAimIdleCam.name.size())
					g_player_hud->StopBlendAnm(m_BlendAimIdleCam.name.c_str());

				g_player_hud->PlayBlendAnm(m_BlendAimEndCam.name.c_str(), 2, m_BlendAimEndCam.speed, m_BlendAimEndCam.power, false, false);
			}
		}
	}

	ResetSubStateTime					();

	xr_delete							(m_zoom_params.m_pVision);
	if(m_zoom_params.m_pNight_vision)
	{
		m_zoom_params.m_pNight_vision->StopForScope(100000.0f, false);
		xr_delete(m_zoom_params.m_pNight_vision);
	}
	if (psActorFlags3.test(AF_LFO_WPN_MOVEMENT_LAYER))
	{
		g_player_hud->updateMovementLayerState();
	}
}

CUIWindow* CWeapon::ZoomTexture()
{
	if (UseScopeTexture() && !bIsSecondVPZoomPresent())
		return m_UIScope;
	else
		return NULL;

	EnableHudInertion(TRUE);
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
	if (IsStockAttached() && GetStockName().size())
	{
		res += pSettings->r_float(GetStockName(), "inv_weight");
	}
	if (IsGripvAttached() && GetGripvName().size())
	{
		res += pSettings->r_float(GetGripvName(), "inv_weight");
	}
	if (IsGripvAttached() && GetGripvName().size())
	{
		res += pSettings->r_float(GetGripvName(), "inv_weight");
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

void CWeapon::OnDrop()
{
	inherited::OnDrop();

	if (&HUD().GetUI()->UIGame()->ActorMenu() && HUD().GetUI()->UIGame()->ActorMenu().GetMenuMode() == mmUndefined)
	{
		ps_ssfx_wpn_dof_1 = GameConstants::GetSSFX_DefaultDoF();
		ps_ssfx_wpn_dof_2 = GameConstants::GetSSFX_DefaultDoF().z;
	}
}

void CWeapon::OnStateSwitch	(u32 S)
{
	inherited::OnStateSwitch(S);
	m_dwAmmoCurrentCalcFrame = 0;

	if (H_Parent() == Level().CurrentEntity())
	{
		CActor* current_actor = smart_cast<CActor*>(H_Parent());

		if (psActorFlags2.test(AF_HUD_DOF))
		{
			if (&HUD().GetUI()->UIGame()->ActorMenu() && HUD().GetUI()->UIGame()->ActorMenu().GetMenuMode() == mmUndefined)
			{
				if ((GetState() == eReload || GetState() == eUnMisfire || (GetState() == eBore && (GameConstants::GetSSFX_EnableBoreDoF() && m_bEnableBoreDof))) && current_actor)
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
	if (lfo_scope_type != 4)//LFO 3d Scopes
	{
		ZoomDynamicMod(true, false);
	}
}

void CWeapon::ZoomDec()
{
	if (lfo_scope_type != 4)//LFO 3d Scopes
	{
		ZoomDynamicMod(false, false);
	}
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
	if (IsStockAttached() && GetStockName().size()) {
		res += pSettings->r_u32(GetStockName(), "cost");
	}
	if (IsGripAttached() && GetGripName().size()) {
		res += pSettings->r_u32(GetGripName(), "cost");
	}
	if (IsGripvAttached() && GetGripvName().size()) {
		res += pSettings->r_u32(GetGripvName(), "cost");
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

		CWeaponMagazinedWGrenade* wpn_wgrenade = smart_cast<CWeaponMagazinedWGrenade*>(this);

		bool magazine_empty = false;

		if (wpn_wgrenade && wpn_wgrenade->IsGrenadeMode())
			magazine_empty = wpn_wgrenade->IsMainMagazineEmpty();
		else
			magazine_empty = IsMagazineEmpty();

		if (state && state & mcAnyMove)
		{
			if (IsGripAttached())
			{
				if (IsScopeAttached() && m_bUseScopeAimMoveAnims)
					return strconcat(sizeof(guns_aim_anm), guns_aim_anm, GenerateAimAnimName("anm_idle_aim_scope_moving_grip_h"), (IsMisfire()) ? "_jammed" : (magazine_empty) ? "_empty" : "");
				else
				{
					if (m_bUseAimAnmDirDependency)
						return strconcat(sizeof(guns_aim_anm), guns_aim_anm, GenerateAimAnimName("anm_idle_aim_moving_grip_h"), (state & mcFwd) ? "_forward" : ((state & mcBack) ? "_back" : ""), (state & mcLStrafe) ? "_left" : ((state & mcRStrafe) ? "_right" : ""), (IsMisfire() ? "_jammed" : (magazine_empty) ? "_empty" : ""));
					else
						return strconcat(sizeof(guns_aim_anm), guns_aim_anm, GenerateAimAnimName("anm_idle_aim_moving_grip_h"), (IsMisfire() ? "_jammed" : (magazine_empty) ? "_empty" : ""));
				}
			}
			else if (IsGripvAttached())
			{
				if (IsScopeAttached() && m_bUseScopeAimMoveAnims)
					return strconcat(sizeof(guns_aim_anm), guns_aim_anm, GenerateAimAnimName("anm_idle_aim_scope_moving_grip_v"), (IsMisfire()) ? "_jammed" : (magazine_empty) ? "_empty" : "");
				else
				{
					if (m_bUseAimAnmDirDependency)
						return strconcat(sizeof(guns_aim_anm), guns_aim_anm, GenerateAimAnimName("anm_idle_aim_moving_grip_v"), (state & mcFwd) ? "_forward" : ((state & mcBack) ? "_back" : ""), (state & mcLStrafe) ? "_left" : ((state & mcRStrafe) ? "_right" : ""), (IsMisfire() ? "_jammed" : (magazine_empty) ? "_empty" : ""));
					else
						return strconcat(sizeof(guns_aim_anm), guns_aim_anm, GenerateAimAnimName("anm_idle_aim_moving_grip_v"), (IsMisfire() ? "_jammed" : (magazine_empty) ? "_empty" : ""));
				}
			}
			else
			{
				if (IsScopeAttached() && m_bUseScopeAimMoveAnims)
					return strconcat(sizeof(guns_aim_anm), guns_aim_anm, GenerateAimAnimName("anm_idle_aim_scope_moving"), (IsMisfire()) ? "_jammed" : (magazine_empty) ? "_empty" : "");
				else
				{
					if (m_bUseAimAnmDirDependency)
						return strconcat(sizeof(guns_aim_anm), guns_aim_anm, GenerateAimAnimName("anm_idle_aim_moving"), (state & mcFwd) ? "_forward" : ((state & mcBack) ? "_back" : ""), (state & mcLStrafe) ? "_left" : ((state & mcRStrafe) ? "_right" : ""), (IsMisfire() ? "_jammed" : (magazine_empty) ? "_empty" : ""));
					else
						return strconcat(sizeof(guns_aim_anm), guns_aim_anm, GenerateAimAnimName("anm_idle_aim_moving"), (IsMisfire() ? "_jammed" : (magazine_empty) ? "_empty" : ""));
				}
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
	if (!IsZoomed())
	{
		!m_bAltZoomActive ? m_bAltZoomActive = true : m_bAltZoomActive = false;
	}
	else
	{
		shared_str cur_scope_sect = (m_sScopeAttachSection.size() ? m_sScopeAttachSection : (m_eScopeStatus == ALife::eAddonAttachable) ? m_scopes[m_cur_scope].c_str() : "scope");

		bool HudFovFromScope = false;
		HudFovFromScope = READ_IF_EXISTS(pSettings, r_bool, cur_scope_sect, "cur_scope_hud_fov", false);

		if (HudFovFromScope)
		{
			if (lfo_scope_type == 4)//LFO 3d Scopes
			{
				psHUD_FOV_def = READ_IF_EXISTS(pSettings, r_float, cur_scope_sect, !m_bAltZoomActive ? "aim_hud_fov" : "alt_aim_hud_fov", GetHudFov());
				m_zoom_params.m_fCurrentZoomFactor = CurrentZoomFactor();
			}
			else
			{
				if (psActorFlags.test(ALTERNATIV_ALT_AIM))
				{
					psHUD_FOV_def = READ_IF_EXISTS(pSettings, r_float, cur_scope_sect, !m_bAltZoomActive ? "aim_hud_fov_pip_2" : "alt_aim_hud_fov_pip_2", GetHudFov());
					m_zoom_params.m_fCurrentZoomFactor = CurrentZoomFactor();
				}
				else
				{
					psHUD_FOV_def = READ_IF_EXISTS(pSettings, r_float, cur_scope_sect, !m_bAltZoomActive ? "aim_hud_fov_pip" : "alt_aim_hud_fov_pip", GetHudFov());
					m_zoom_params.m_fCurrentZoomFactor = CurrentZoomFactor();
				}
			}
		}
	}
	UpdateAltAimZoomFactor();
}

void CWeapon::SwitchLaser(bool on)
{
	if (!has_laser || !IsLaserAttached())
		return;

	if (on)
	{
		if (isHUDAnimationExist("anm_laser_on"))
			SwitchState(eLaserSwitch);
	}
	else
	{
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
		if (isHUDAnimationExist("anm_torch_on"))
			SwitchState(eFlashlightSwitch);
	}
	else
	{
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

	for (auto mesh : m_weapon_attaches)
	{
		mesh->UpdateAttachesPosition(model, base_model_trans, for_hud);

		if (!for_hud)
			mesh->RenderAttach(false);
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

void CWeapon::SafetyBoneCallback(CBoneInstance* P)
{
	CWeapon* weapon = static_cast<CWeapon*>(P->callback_param());
	P->mTransform.mulB_43(weapon->m_mSafetyRotation);
}

void CWeapon::SetSafetyBoneCallback()
{
	IKinematics* worldVisual = smart_cast<IKinematics*>(Visual());
	u16 boneId = worldVisual->LL_BoneID(m_sSafetyBoneName);
	CBoneInstance& safetyBone = worldVisual->LL_GetBoneInstance(boneId);
	safetyBone.set_callback(bctCustom, &SafetyBoneCallback, this);
}

void CWeapon::ResetSafetyBoneCallback()
{
	IKinematics* worldVisual = smart_cast<IKinematics*>(Visual());
	u16 boneId = worldVisual->LL_BoneID(m_sSafetyBoneName);
	CBoneInstance& safetyBone = worldVisual->LL_GetBoneInstance(boneId);
	safetyBone.reset_callback();
}

void CWeapon::SetHudSafetyBoneCallback()
{
	attachable_hud_item* hudItem = HudItemData();
	IKinematics* hudVisual = hudItem->m_model;
	u16 boneId = hudVisual->LL_BoneID(m_sSafetyBoneName);
	CBoneInstance& safetyBone = hudVisual->LL_GetBoneInstance(boneId);
	safetyBone.set_callback(bctCustom, &SafetyBoneCallback, this);
}

void CWeapon::ResetHudSafetyBoneCallback()
{
	attachable_hud_item* hudItem = HudItemData();
	IKinematics* hudVisual = hudItem->m_model;
	u16 boneId = hudVisual->LL_BoneID(m_sSafetyBoneName);
	CBoneInstance& safetyBone = hudVisual->LL_GetBoneInstance(boneId);
	safetyBone.reset_callback();
}

void CWeapon::UpdateSafetyRotation()
{
	if (m_fSafetyRotationTime > 0.0f)
	{
		CalculateSafetyRotation(m_fSafetyRotationSpeed * Device.fTimeDelta);
		m_fSafetyRotationTime -= Device.fTimeDelta;
	}
}

void CWeapon::CalculateSafetyRotation(float value)
{
	Fmatrix rotation;
	rotation.identity();
	rotation.rotation(m_vSafetyRotationAxis, value);
	m_mSafetyRotation.mulB_43(rotation);
}

void CWeapon::RecalculateSafetyRotation(bool reverse, u32 step)
{
	m_fSafetyRotationTime = 0.0f;
	m_mSafetyRotation.identity();
	CalculateSafetyRotation((reverse ? PI_MUL_2 + step : PI_MUL_2 - step));
}

// AVO: for custom added sounds check if sound exists
bool CWeapon::WeaponSoundExist(LPCSTR section, LPCSTR sound_name, bool log) const
{
	pcstr str;
	bool sec_exist = process_if_exists_set(section, sound_name, &CInifile::r_string, str, true);
	if (sec_exist)
		return true;
#ifdef DEBUG
	if (log)
		Msg("~ [WARNING] ------ Sound [%s] does not exist in [%s]", sound_name, section);
#endif
	return false;
}

void CWeapon::WpnExplosion()
{
	// Если актор - обладатель оружия
	CActor* pActor = smart_cast<CActor*>(H_Parent());
	if (!pActor) return;

	// Проигрываем партиклы
	CParticlesObject* Particles;
	Particles = CParticlesObject::Create(*m_sSmokeParticles, TRUE);
	Particles->play_at_pos(Position());

	// Проигрываем звук
	if (sndWpnExplosion._feedback())
		sndWpnExplosion.stop();
 
	sndWpnExplosion.play(this, sm_2D);
 
	// Проигрываем .ppe
	auto ActorPPE = xr_new<CPostprocessAnimator>(3871, false);
	ActorPPE->Load(*ppeWpnExplosion);
	pActor->Cameras().AddPPEffector(ActorPPE);

	// развернуть камеру
	if (pActor->cam_Active()) {
		pActor->cam_Active()->Move(Random.randI(2) ? kRIGHT : kLEFT, Random.randF(0.1f, 0.8f));
		pActor->cam_Active()->Move(Random.randI(2) ? kUP : kDOWN, Random.randF(0.1f, 0.8f));
	}

	// Нанести хит
	NET_Packet			l_P;
	SHit				HS;

	HS.GenHeader		(GE_HIT, pActor->ID());
	HS.whoID			= (ID());
	HS.weaponID			= (ID());
	HS.dir				= (Fvector().set(0.f,1.f,0.f));
	HS.power			= (1.0f);
	HS.boneID			= (smart_cast<IKinematics*>(pActor->Visual())->LL_GetBoneRoot());
	HS.p_in_bone_space	= (Fvector().set(0.f,0.f,0.f));
	HS.impulse			= (80 * pActor->character_physics_support()->get_movement()->GetMass());
	HS.hit_type			= ( ALife::eHitTypeStrike);	// eHitTypeExplosion ?
	HS.Write_Packet		(l_P);
	u_EventSend			(l_P);	

	// Роняем оружие
	pActor->g_PerformDrop();
 
	// Уничтожаем оружие
	if (m_bWpnDestroyAfterExplode)
		DestroyObject();
}

void CWeapon::UpdateAimOffsets()
{
	shared_str cur_scope_sect = (m_sScopeAttachSection.size() ? m_sScopeAttachSection : (m_eScopeStatus == ALife::eAddonAttachable) ? m_scopes[m_cur_scope].c_str() : "scope");
	bool UseScopeAimBone = false;

	bool AimOffsetsFromScope = false;
	AimOffsetsFromScope = READ_IF_EXISTS(pSettings, r_bool, cur_scope_sect, "cur_scope_aim_offsets", false);

	bool NeedAnotherOffset = false;
	NeedAnotherOffset = READ_IF_EXISTS(pSettings, r_bool, cur_scope_sect, "cur_scope_aim_offsets_3d", false);


	bool AimOffsetsPermanentScope = false;
	if (pSettings->line_exist(m_section_id.c_str(), "weapon_have_permanent_scope"))
		AimOffsetsPermanentScope = READ_IF_EXISTS(pSettings, r_bool, m_section_id, "weapon_have_permanent_scope", false);


	if (!IsScopeAttached() || (!IsZoomed() && !IsRotatingFromZoom()) || !cur_scope_sect.size())
	{
		attachable_hud_item* hi = HudItemData();

		if (!hi)
			return;

		bool is_16x9 = UI().is_widescreen();
		string64	_prefix;
		xr_sprintf(_prefix, "%s", is_16x9 ? "_16x9" : "");
		string128	val_name;

		if (AimOffsetsPermanentScope)
		{
			strconcat(sizeof(val_name), val_name, "aim_hud_offset_3d_pos", _prefix);
			hi->m_measures.m_hands_offset[0][1] = pSettings->r_fvector3(m_hud_sect, val_name);
			strconcat(sizeof(val_name), val_name, "aim_hud_offset_3d_rot", _prefix);
			hi->m_measures.m_hands_offset[1][1] = pSettings->r_fvector3(m_hud_sect, val_name);
		}
		else
		{
			strconcat(sizeof(val_name), val_name, "aim_hud_offset_pos", _prefix);
			hi->m_measures.m_hands_offset[0][1] = pSettings->r_fvector3(m_hud_sect, val_name);
			strconcat(sizeof(val_name), val_name, "aim_hud_offset_rot", _prefix);
			hi->m_measures.m_hands_offset[1][1] = pSettings->r_fvector3(m_hud_sect, val_name);
		}
		
		strconcat(sizeof(val_name), val_name, "aim_hud_offset_3d_alt_pos", _prefix);
		hi->m_measures.m_hands_offset[0][3] = READ_IF_EXISTS(pSettings, r_fvector3, m_hud_sect, val_name, hi->m_measures.m_hands_offset[0][1]);
		strconcat(sizeof(val_name), val_name, "aim_hud_offset_3d_alt_rot", _prefix);
		hi->m_measures.m_hands_offset[1][3] = READ_IF_EXISTS(pSettings, r_fvector3, m_hud_sect, val_name, hi->m_measures.m_hands_offset[1][1]);

		strconcat(sizeof(val_name), val_name, "gl_hud_offset_pos", _prefix);
		hi->m_measures.m_hands_offset[0][2] = pSettings->r_fvector3(m_hud_sect, val_name);
		strconcat(sizeof(val_name), val_name, "gl_hud_offset_rot", _prefix);
		hi->m_measures.m_hands_offset[1][2] = pSettings->r_fvector3(m_hud_sect, val_name);

		return;
	}

	if (AimOffsetsFromScope)
	{
		attachable_hud_item* hi = HudItemData();

		if (!hi)
			return;

		bool is_16x9 = UI().is_widescreen();
		string64	_prefix;
		xr_sprintf(_prefix, "%s", is_16x9 ? "_16x9" : "");
		string128	val_name;

		if (NeedAnotherOffset && !bIsSecondVPZoomPresent())
		{
			if (lfo_scope_type != 3)// SHOKER 3d Scopes PiP
			{
				strconcat(sizeof(val_name), val_name, "aim_hud_offset_3d_pos", _prefix);
				hi->m_measures.m_hands_offset[0][1] = pSettings->r_fvector3(cur_scope_sect, val_name);
				strconcat(sizeof(val_name), val_name, "aim_hud_offset_3d_rot", _prefix);
				hi->m_measures.m_hands_offset[1][1] = pSettings->r_fvector3(cur_scope_sect, val_name);

			}
			else
			{
				strconcat(sizeof(val_name), val_name, "aim_hud_offset_3d_pos", _prefix);
				hi->m_measures.m_hands_offset[0][1] = pSettings->r_fvector3(cur_scope_sect, val_name);
				strconcat(sizeof(val_name), val_name, "aim_hud_offset_3d_rot", _prefix);
				hi->m_measures.m_hands_offset[1][1] = pSettings->r_fvector3(cur_scope_sect, val_name);
			}

			strconcat(sizeof(val_name), val_name, "aim_hud_offset_3d_alt_pos", _prefix);
			hi->m_measures.m_hands_offset[0][3] = READ_IF_EXISTS(pSettings, r_fvector3, cur_scope_sect, val_name, hi->m_measures.m_hands_offset[0][1]);
			strconcat(sizeof(val_name), val_name, "aim_hud_offset_3d_alt_rot", _prefix);
			hi->m_measures.m_hands_offset[1][3] = READ_IF_EXISTS(pSettings, r_fvector3, cur_scope_sect, val_name, hi->m_measures.m_hands_offset[1][1]);
		}
		else
		{

			if (lfo_scope_type != 3)// SHOKER 3d Scopes PiP
			{
				strconcat(sizeof(val_name), val_name, "aim_hud_offset_pos", _prefix);
				hi->m_measures.m_hands_offset[0][1] = pSettings->r_fvector3(cur_scope_sect, val_name);
				strconcat(sizeof(val_name), val_name, "aim_hud_offset_rot", _prefix);
				hi->m_measures.m_hands_offset[1][1] = pSettings->r_fvector3(cur_scope_sect, val_name);
			}
			else
			{
				strconcat(sizeof(val_name), val_name, "aim_hud_offset_pos", _prefix);
				hi->m_measures.m_hands_offset[0][1] = pSettings->r_fvector3(cur_scope_sect, val_name);
				strconcat(sizeof(val_name), val_name, "aim_hud_offset_rot", _prefix);
				hi->m_measures.m_hands_offset[1][1] = pSettings->r_fvector3(cur_scope_sect, val_name);
			}
	
			strconcat(sizeof(val_name), val_name, "aim_hud_offset_alt_pos", _prefix);
			hi->m_measures.m_hands_offset[0][3] = READ_IF_EXISTS(pSettings, r_fvector3, cur_scope_sect, val_name, hi->m_measures.m_hands_offset[0][1]);
			strconcat(sizeof(val_name), val_name, "aim_hud_offset_alt_rot", _prefix);
			hi->m_measures.m_hands_offset[1][3] = READ_IF_EXISTS(pSettings, r_fvector3, cur_scope_sect, val_name, hi->m_measures.m_hands_offset[1][1]);
		}

		strconcat(sizeof(val_name), val_name, "gl_hud_offset_pos", _prefix);
		hi->m_measures.m_hands_offset[0][2] = pSettings->r_fvector3(cur_scope_sect, val_name);
		strconcat(sizeof(val_name), val_name, "gl_hud_offset_rot", _prefix);
		hi->m_measures.m_hands_offset[1][2] = pSettings->r_fvector3(cur_scope_sect, val_name);
	}
}

void CWeapon::UpdateAltAimZoomFactor()	//FOR SCOPED WEAPONS
{
	shared_str cur_scope_sect = (m_sScopeAttachSection.size() ? m_sScopeAttachSection : (m_eScopeStatus == ALife::eAddonAttachable) ? m_scopes[m_cur_scope].c_str() : "scope");
	if (lfo_scope_type == 4)//LFO 3d Scopes
	{
		m_zoom_params.m_fScopeZoomFactor = READ_IF_EXISTS(pSettings, r_float, cur_scope_sect, !m_bAltZoomActive ? "scope_zoom_factor_3d" : "scope_zoom_factor_3d_alt", GetHudFov());
	}
}

void CWeapon::UpdateAltAimZoomFactor2()	//FOR NONE SCOPED WEAPONS
{
	luabind::functor<void> funct;
	bool ShowHudAltAimNoScope = false;
	
	if (pSettings->line_exist(m_section_id, "enable_alternative_aim_no_scope"))
		ShowHudAltAimNoScope = READ_IF_EXISTS(pSettings, r_bool, m_section_id, "enable_alternative_aim_no_scope", false);

	if (ShowHudAltAimNoScope)
	{
		if (m_bAltZoomEnabled)
		{
			if (m_bAltZoomActive)
			{
				//	Msg("!Weapon No Scope Alt Aim Enabled");
				if (ai().script_engine().functor("lfo_weapons.on_actor_weapon_test_process_on", funct))
					funct();

				m_zoom_params.m_fScopeZoomFactor = pSettings->r_float(cNameSect(), "scope_zoom_factor_alt");
			}
			else
			{
				//	Msg("!Weapon No Scope Alt Aim Disabled");
				if (ai().script_engine().functor("lfo_weapons.on_actor_weapon_test_process_off", funct))
					funct();

				m_zoom_params.m_fScopeZoomFactor = pSettings->r_float(cNameSect(), "scope_zoom_factor");
			}
		}
	}
}

void CWeapon::UpdateHudAltAimHud()
{
	luabind::functor<void> funct;
	shared_str cur_scope_sect = (m_sScopeAttachSection.size() ? m_sScopeAttachSection : (m_eScopeStatus == ALife::eAddonAttachable) ? m_scopes[m_cur_scope].c_str() : "scope");
	bool ShowHudAltAimPossibleScopedWeapon = false;
	bool ShowHudAltAimPossible = false;

	if (pSettings->line_exist(cur_scope_sect, "enable_alternative_aim_scope_info"))
		ShowHudAltAimPossibleScopedWeapon = READ_IF_EXISTS(pSettings, r_bool, cur_scope_sect, "enable_alternative_aim_scope_info", false);

	if (pSettings->line_exist(m_section_id, "enable_alternative_aim_info"))
		ShowHudAltAimPossible = READ_IF_EXISTS(pSettings, r_bool, m_section_id, "enable_alternative_aim_info", false);

	if (lfo_scope_type == 4 && psActorFlags3.test(AF_LFO_HUD_FUNCTIONS_ICONS))//LFO 3d Scopes
	{
		if (ShowHudAltAimPossibleScopedWeapon)
		{
			//Msg("!Weapon Scoped Alt Aim Enabled");

			if (ai().script_engine().functor("lfo_weapons.on_actor_weapon_alt_aim_on", funct))
				funct();
		}
		else
		{
			//Msg("!Weapon Scoped Alt Aim Disabled");

			if (ai().script_engine().functor("lfo_weapons.on_actor_weapon_alt_aim_off", funct))
				funct();
		}

		if (ShowHudAltAimPossible)
		{
			//Msg("!Weapon Alt Aim Enabled");

			if (ai().script_engine().functor("lfo_weapons.on_actor_weapon_alt_aim_on", funct))
				funct();
		}
		else
		{
			//Msg("!Weapon Scoped Alt Aim Disabled");

			if (ai().script_engine().functor("lfo_weapons.on_actor_weapon_alt_aim_off", funct))
				funct();
		}
	}
}

void CWeapon::UpdateHudAltAimHudMode2() //Let us make unique for Survival mode
{
	luabind::functor<void> funct;
	shared_str cur_scope_sect = (m_sScopeAttachSection.size() ? m_sScopeAttachSection : (m_eScopeStatus == ALife::eAddonAttachable) ? m_scopes[m_cur_scope].c_str() : "scope");
	bool ShowHudAltAimPossibleScopedWeapon = false;
	bool ShowHudAltAimPossible = false;

	if (pSettings->line_exist(cur_scope_sect, "enable_alternative_aim_scope_info"))
		ShowHudAltAimPossibleScopedWeapon = READ_IF_EXISTS(pSettings, r_bool, cur_scope_sect, "enable_alternative_aim_scope_info", false);

	if (pSettings->line_exist(m_section_id, "enable_alternative_aim_info"))
		ShowHudAltAimPossible = READ_IF_EXISTS(pSettings, r_bool, m_section_id, "enable_alternative_aim_info", false);

	if (lfo_scope_type == 4 && psActorFlags3.test(AF_LFO_SURVIVAL_MODE))//LFO 3d Scopes
	{
		if (ShowHudAltAimPossibleScopedWeapon)
		{
			//Msg("!Weapon Scoped Alt Aim Enabled");

			if (ai().script_engine().functor("lfo_weapons.on_actor_weapon_alt_aim2_on", funct))
				funct();
		}
		else
		{
			//Msg("!Weapon Scoped Alt Aim Disabled");

			if (ai().script_engine().functor("lfo_weapons.on_actor_weapon_alt_aim_off", funct))
				funct();
		}

		if (ShowHudAltAimPossible)
		{
			//Msg("!Weapon Alt Aim Enabled");

			if (ai().script_engine().functor("lfo_weapons.on_actor_weapon_alt_aim2_on", funct))
				funct();
		}
		else
		{
			//Msg("!Weapon Scoped Alt Aim Disabled");

			if (ai().script_engine().functor("lfo_weapons.on_actor_weapon_alt_aim_off", funct))
				funct();
		}
	}
}

void CWeapon::UpdateCheckWeaponHaveLaser()
{
	string64 str;
	luabind::functor<void> funct;
	bool WeaponHaveLaser = false;

	if (pSettings->line_exist(m_section_id.c_str(), "weapon_use_laser"))
		WeaponHaveLaser = READ_IF_EXISTS(pSettings, r_bool, m_section_id, "weapon_use_laser", false);

	if (psHUD_Flags.test(HUD_CROSSHAIR))
	{
		if (WeaponHaveLaser)
		{
		//	Msg("!Weapon Have Laser");

			if (ai().script_engine().functor("lfo_weapons.on_actor_weapon_have_laser", funct))
				funct();

			psHUD_Flags.set(HUD_CROSSHAIR_RT, FALSE);
		}
		else
		{
		//	Msg("!Weapon Dont Have Laser");
	
			if (ai().script_engine().functor("lfo_weapons.on_actor_weapon_dont_have_laser", funct))
				funct();

			psHUD_Flags.set(HUD_CROSSHAIR_RT, TRUE);
		}
	}
}

void CWeapon::UpdateGLAttached()	//FOR NONE SCOPED WEAPONS
{
	luabind::functor<void> funct;
	bool ShowHudAltAimNoScope = false;

	if (psActorFlags3.test(AF_LFO_HUD_FUNCTIONS_ICONS)) 
	{
		if (IsGrenadeLauncherAttached() && GetGrenadeLauncherName().size())
		{
			//	Msg("!Weapon GL ATTACHED");
			if (ai().script_engine().functor("lfo_weapons.on_actor_weapon_gl_attached", funct))
				funct();
		}
	}
}

void CWeapon::UpdateAimFOV()
{
	shared_str cur_scope_sect = (m_sScopeAttachSection.size() ? m_sScopeAttachSection : (m_eScopeStatus == ALife::eAddonAttachable) ? m_scopes[m_cur_scope].c_str() : "scope");
	psHUD_FOV_def = last_hud_fov;

	bool HudFovFromScope = false;
	HudFovFromScope = READ_IF_EXISTS(pSettings, r_bool, cur_scope_sect, "cur_scope_hud_fov", false);

	if (HudFovFromScope && IsScopeAttached() && !IsRotatingFromZoom())	//SCOPED WEAPONS
	{
		if (lfo_scope_type == 4)//LFO 3d Scopes
		{
			psHUD_FOV_def = READ_IF_EXISTS(pSettings, r_float, cur_scope_sect, !m_bAltZoomActive ? "aim_hud_fov" : "alt_aim_hud_fov", GetHudFov());
			m_zoom_params.m_fCurrentZoomFactor = CurrentZoomFactor();
		}
		else
		{
			if (psActorFlags.test(ALTERNATIV_ALT_AIM))
			{
				psHUD_FOV_def = READ_IF_EXISTS(pSettings, r_float, cur_scope_sect, !m_bAltZoomActive ? "aim_hud_fov_pip_2" : "alt_aim_hud_fov_pip_2", GetHudFov());
				m_zoom_params.m_fCurrentZoomFactor = CurrentZoomFactor();
			}
			else
			{
				psHUD_FOV_def = READ_IF_EXISTS(pSettings, r_float, cur_scope_sect, !m_bAltZoomActive ? "aim_hud_fov_pip" : "alt_aim_hud_fov_pip", GetHudFov());
				m_zoom_params.m_fCurrentZoomFactor = CurrentZoomFactor();
			}
		}
	}

	bool HudFovFromNoScope = false;
	HudFovFromNoScope = READ_IF_EXISTS(pSettings, r_bool, m_section_id, "cur_no_scope_hud_fov", false);

	if (HudFovFromNoScope && !IsScopeAttached())	//NONE SCOPED WEAPONS
	{
		psHUD_FOV_def = READ_IF_EXISTS(pSettings, r_float, m_section_id, !m_bAltZoomActive ? "aim_hud_fov_ironsight" : "alt_aim_hud_fov_ironsight", GetHudFov());
		m_zoom_params.m_fCurrentZoomFactor = CurrentZoomFactor();
	}

}

void CWeapon::LoadOverheatLightParams(LPCSTR section)
{
	if (psActorFlags3.test(AF_LFO_WPN_OVERHEAT_LIGHTS))
	{
		m_bLightsEnabled = READ_IF_EXISTS(pSettings, r_string, section, "overheat_light_enabled", false);
		m_sWpn_overheating_bone = READ_IF_EXISTS(pSettings, r_string, section, "overheat_attach_bone", wpn_overheat_def_bone);

		overheat_attach_bone = READ_IF_EXISTS(pSettings, r_string, section, "overheat_light_bone", m_sWpn_overheating_bone);
		overheat_attach_offset = READ_IF_EXISTS(pSettings, r_fvector3, section, "overheat_attach_offset", Fvector3().set(0.f, 0.f, 0.f));
		overheat_omni_attach_offset = READ_IF_EXISTS(pSettings, r_fvector3, section, "overheat_omni_attach_offset", Fvector3().set(0.f, 0.f, 0.f));
		overheat_world_attach_offset = READ_IF_EXISTS(pSettings, r_fvector3, section, "overheat_world_attach_offset", Fvector3().set(0.f, 0.f, 0.f));
		overheat_omni_world_attach_offset = READ_IF_EXISTS(pSettings, r_fvector3, section, "overheat_omni_world_attach_offset", Fvector3().set(0.f, 0.f, 0.f));

		if (!overheat_light)
		{
			overheat_light = ::Render->light_create();
			overheat_light->set_shadow(READ_IF_EXISTS(pSettings, r_string, section, "overheat_light_shadow", false));

			if (psActorFlags.test(HUD_ITEM_VOL_LIGHTS))
			{
				m_bVolumetricLights = READ_IF_EXISTS(pSettings, r_bool, section, "overheat_volumetric_lights", false);
				m_fVolumetricQuality = READ_IF_EXISTS(pSettings, r_float, section, "overheat_volumetric_quality", 1.0f);
				m_fVolumetricDistance = READ_IF_EXISTS(pSettings, r_float, section, "overheat_volumetric_distance", 0.3f);
				m_fVolumetricIntensity = READ_IF_EXISTS(pSettings, r_float, section, "overheat_volumetric_intensity", 0.5f);
			}

			m_iLightType = READ_IF_EXISTS(pSettings, r_u8, section, "overheat_light_type", 1);
			light_lanim = LALib.FindItem(READ_IF_EXISTS(pSettings, r_string, section, "overheat_color_animator", ""));

			const Fcolor clr = READ_IF_EXISTS(pSettings, r_fcolor, section, "overheat_light_color", (Fcolor{ 1.0f, 0.0f, 0.0f, 1.0f }));

			fBrightness = clr.intensity();
			overheat_light->set_color(clr);

			const float range = READ_IF_EXISTS(pSettings, r_float, section, "overheat_light_range", 1.f);

			overheat_render = ::Render->light_create();
			overheat_render->set_type(IRender_Light::SPOT);
			overheat_render->set_shadow(true);

			overheat_light->set_range(range);
			overheat_light->set_hud_mode(true);
			overheat_light->set_type((IRender_Light::LT)m_iLightType);
			overheat_light->set_cone(deg2rad(READ_IF_EXISTS(pSettings, r_float, section, "overheat_light_spot_angle", 1.f)));
			overheat_light->set_texture(READ_IF_EXISTS(pSettings, r_string, section, "overheat_light_spot_texture", nullptr));

			if (psActorFlags.test(HUD_ITEM_VOL_LIGHTS))
			{
				overheat_light->set_volumetric(m_bVolumetricLights);
				overheat_light->set_volumetric_quality(m_fVolumetricQuality);
				overheat_light->set_volumetric_distance(m_fVolumetricDistance);
				overheat_light->set_volumetric_intensity(m_fVolumetricIntensity);
			}

			m_bOverheatGlowEnabled = READ_IF_EXISTS(pSettings, r_string, section, "overheat_glow_enabled", false);

			if (!overheat_glow && m_bOverheatGlowEnabled)
			{
				overheat_glow = ::Render->glow_create();
				overheat_glow->set_texture(READ_IF_EXISTS(pSettings, r_string, section, "overheat_glow_texture", nullptr));
				overheat_glow->set_color(clr);
				overheat_glow->set_radius(READ_IF_EXISTS(pSettings, r_float, section, "overheat_glow_radius", 0.3f));
			}
		}
	}
}

void CWeapon::UpdateOverheatLights()
{
	if (psActorFlags3.test(AF_LFO_WPN_OVERHEAT_LIGHTS))
	{
		if (overheat_light && m_fWeaponOverheating >= 0.5f)
		{
			const u32 state = GetState();
			auto io = smart_cast<CInventoryOwner*>(H_Parent());

			if (!overheat_light->get_active() && (state == eShowing || state == eIdle))
			{
				//Fvector overheat_pos, overheat_pos_omni, overheat_dir, overheat_dir_omni;
				Fvector overheat_pos = get_LastFP(), overheat_dir = get_LastFD();

				//	Msg("!OVERHEATING LIGHT");
				overheat_light->set_active(true);
				overheat_glow->set_active(true);

				if (GetHUDmode())
				{
					if (overheat_attach_bone.size())
					{
					//	Msg("!OVERHEATING UPDATE BONE");
						overheat_dir = get_LastFD();
						GetBoneOffsetPosDir(overheat_attach_bone, overheat_pos, overheat_dir, overheat_attach_offset);
						CorrectDirFromWorldToHud(overheat_dir);
					}
				}
				else
				{
					overheat_dir = get_LastFD();
					XFORM().transform_tiny(overheat_pos, overheat_world_attach_offset);
				}

				Fmatrix overheatXForm;
				overheatXForm.identity();
				overheatXForm.k.set(overheat_dir);
				Fvector::generate_orthonormal_basis_normalized(overheatXForm.k, overheatXForm.j, overheatXForm.i);
				overheat_render->set_position(overheat_pos);
				overheat_render->set_rotation(overheatXForm.k, overheatXForm.i);

				if (!overheat_light->get_active() && (state == eShowing || state == eIdle))
				{
					overheat_light->set_active(true);

					if (overheat_glow && !overheat_glow->get_active() && m_bGlowEnabled)
						overheat_glow->set_active(true);
				}
				else if (overheat_light->get_active() && (state == eHiding || state == eHidden))
				{
					overheat_light->set_active(false);

					if (overheat_glow && overheat_glow->get_active() && m_bGlowEnabled)
						overheat_glow->set_active(false);
				}
			}

			if (overheat_light->get_active() && HudItemData())
			{
				if (GetHUDmode())
				{
					firedeps fd;
					HudItemData()->setup_firedeps(fd);
					overheat_light->set_position(fd.vLastFP2);

					if (overheat_glow && overheat_glow->get_active())
						overheat_glow->set_position(fd.vLastFP2);
				}

				// calc color animator
				if (light_lanim)
				{
					int frame{};
					u32 clr = light_lanim->CalculateRGB(Device.fTimeGlobal, frame);
					Fcolor fclr;
					fclr.set(clr);
					overheat_light->set_color(fclr);
				}
			}
		}

		if (overheat_light && m_fWeaponOverheating <= 0.4f)
		{
		//	Msg("!NO OVERHEATING LIGHT DISABLED");
			overheat_light->set_active(false);

			if (overheat_glow && overheat_glow->get_active() && m_bOverheatGlowEnabled)
				overheat_glow->set_active(false);
		}
	}
}