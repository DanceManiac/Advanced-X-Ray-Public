// Weapon.cpp: implementation of the CWeapon class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Weapon.h"
#include "ParticlesObject.h"
#include "HUDManager.h"
#include "player_hud.h"
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
#include "ai_object_location.h"
#include "clsid_game.h"
#include "mathutils.h"
#include "object_broker.h"
#include "../xrEngine/igame_persistent.h"
#include "GamePersistent.h"

#include "AdvancedXrayGameConstants.h"

#define WEAPON_REMOVE_TIME		60000
#define ROTATION_TIME			0.25f

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWeapon::CWeapon(LPCSTR name)
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

	m_fZoomFactor			= g_fov;
	m_fZoomRotationFactor	= 0.f;


	m_pAmmo					= NULL;


	m_pFlameParticles2		= NULL;
	m_sFlameParticles2		= NULL;


	m_fCurrentCartirdgeDisp = 1.f;

	m_strap_bone0			= 0;
	m_strap_bone1			= 0;
	m_strap_bone0_id		= -1;
	m_strap_bone1_id		= -1;
	m_StrapOffset.identity	();
	m_strapped_mode			= false;
	m_strapped_mode_rifle	= false;
	m_can_be_strapped_rifle = false;
	m_can_be_strapped		= false;
	m_ef_main_weapon_type	= u32(-1);
	m_ef_weapon_type		= u32(-1);
	m_UIScope				= NULL;
	m_set_next_ammoType_on_reload = u32(-1);
	m_freelook_switch_back	= false;
}

CWeapon::~CWeapon		()
{
	xr_delete	(m_UIScope);
}

//void CWeapon::Hit(float P, Fvector &dir,	
//		    CObject* who, s16 element,
//		    Fvector position_in_object_space, 
//		    float impulse, 
//		    ALife::EHitType hit_type)
void CWeapon::Hit					(SHit* pHDS)
{
//	inherited::Hit(P, dir, who, element, position_in_object_space,impulse,hit_type);
	inherited::Hit(pHDS);
}



void CWeapon::UpdateXForm	()
{
	if (Device.dwFrame == dwXF_Frame)
		return;

	dwXF_Frame = Device.dwFrame;

	if (!H_Parent())
		return;

	// Get access to entity and its visual
	CEntityAlive*	E		= smart_cast<CEntityAlive*>(H_Parent());
		
	if (!E) {
		if (!IsGameTypeSingle())
			UpdatePosition(H_Parent()->XFORM());
		return;
	}

	const CInventoryOwner	*parent = smart_cast<const CInventoryOwner*>(E);
	if (parent && parent->use_simplified_visual())
			return;

	if (!m_can_be_strapped_rifle)
	{
		if (parent->attached(this))
			return;
	}

	R_ASSERT		(E);
	IKinematics*	V		= smart_cast<IKinematics*>	(E->Visual());
	VERIFY			(V);

	// Get matrices
	int				boneL,boneR,boneR2;

	if ((m_strap_bone0_id == -1 || m_strap_bone1_id == -1) && m_can_be_strapped_rifle)
	{
		m_strap_bone0_id = V->LL_BoneID(m_strap_bone0);
		m_strap_bone1_id = V->LL_BoneID(m_strap_bone1);
	}

	if (parent->inventory().GetActiveSlot() != RIFLE_SLOT && m_can_be_strapped_rifle /*&& parent->inventory().InSlot(this)*/ )
	{
		boneR = m_strap_bone0_id;
		boneR2 = m_strap_bone1_id;
		boneL = boneR;
		
		if (!m_strapped_mode_rifle) m_strapped_mode_rifle = true;
	}
	else
	{
			E->g_WeaponBones(boneL, boneR, boneR2);

		if (m_strapped_mode_rifle) m_strapped_mode_rifle = false;
	}

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
		boneL = boneR2;
#pragma todo("TO ALL: serious performance problem")
	V->CalculateBones	();
	Fmatrix& mL			= V->LL_GetTransform(u16(boneL));
	Fmatrix& mR			= V->LL_GetTransform(u16(boneR));
	// Calculate
	Fmatrix				mRes;
	Fvector				R,D,N;
	D.sub				(mL.c,mR.c);	

	if(fis_zero(D.magnitude()))
	{
		mRes.set(E->XFORM());
		mRes.c.set(mR.c);
	}
	else
	{		
		D.normalize();
		R.crossproduct	(mR.j,D);

		N.crossproduct	(D,R);			
		N.normalize();

		mRes.set		(R,N,D,mR.c);
		mRes.mulA_43	(E->XFORM());
	}

	UpdatePosition	(mRes);
}


void CWeapon::UpdateFireDependencies_internal()
{
	if (Device.dwFrame!=dwFP_Frame) 
	{
		dwFP_Frame			= Device.dwFrame;

		UpdateXForm			();

		if ( GetHUDmode() )
		{
			HudItemData()->setup_firedeps		(m_current_firedeps);
			VERIFY(_valid(m_current_firedeps.m_FireParticlesXForm));
		}
		else 
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

		/*CInventoryOwner* io		= smart_cast<CInventoryOwner*>(H_Parent());
		if(NULL == io->inventory().ActiveItem())
		{
				Log("current_state", GetState() );
				Log("next_state", GetNextState());
				Log("state_time", m_dwStateTime);
				Log("item_sect", cNameSect().c_str());
				Log("H_Parent", H_Parent()->cNameSect().c_str());
		} */

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

void CWeapon::Load		(LPCSTR section)
{
	inherited::Load					(section);
	CShootingObject::Load			(section);

	
	if(pSettings->line_exist(section, "flame_particles_2"))
		m_sFlameParticles2 = pSettings->r_string(section, "flame_particles_2");

#ifdef DEBUG
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
#endif

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
	camMaxAngle			= pSettings->r_float		(section,"cam_max_angle"	); 
	camMaxAngle			= deg2rad					(camMaxAngle);
	camRelaxSpeed		= pSettings->r_float		(section,"cam_relax_speed"	); 
	camRelaxSpeed		= deg2rad					(camRelaxSpeed);
	if (pSettings->line_exist(section, "cam_relax_speed_ai"))
	{
		camRelaxSpeed_AI		= pSettings->r_float		(section,"cam_relax_speed_ai"	); 
		camRelaxSpeed_AI		= deg2rad					(camRelaxSpeed_AI);
	}
	else
	{
		camRelaxSpeed_AI	= camRelaxSpeed;
	}
	
//	camDispersion		= pSettings->r_float		(section,"cam_dispersion"	); 
//	camDispersion		= deg2rad					(camDispersion);

	camMaxAngleHorz		= pSettings->r_float		(section,"cam_max_angle_horz"	); 
	camMaxAngleHorz		= deg2rad					(camMaxAngleHorz);
	camStepAngleHorz	= pSettings->r_float		(section,"cam_step_angle_horz"	); 
	camStepAngleHorz	= deg2rad					(camStepAngleHorz);	
	camDispertionFrac			= READ_IF_EXISTS(pSettings, r_float, section, "cam_dispertion_frac",	0.7f);
	//  [8/2/2005]
	//m_fParentDispersionModifier = READ_IF_EXISTS(pSettings, r_float, section, "parent_dispersion_modifier",1.0f);
	m_fPDM_disp_base			= READ_IF_EXISTS(pSettings, r_float, section, "PDM_disp_base",	1.0f);
	m_fPDM_disp_vel_factor		= READ_IF_EXISTS(pSettings, r_float, section, "PDM_disp_vel_factor",	1.0f);
	m_fPDM_disp_accel_factor	= READ_IF_EXISTS(pSettings, r_float, section, "PDM_disp_accel_factor",	1.0f);
	m_fPDM_disp_crouch			= READ_IF_EXISTS(pSettings, r_float, section, "PDM_disp_crouch",	1.0f);
	m_fPDM_disp_crouch_no_acc	= READ_IF_EXISTS(pSettings, r_float, section, "PDM_disp_crouch_no_acc",	1.0f);
	//  [8/2/2005]

	fireDispersionConditionFactor = pSettings->r_float(section,"fire_dispersion_condition_factor"); 
	misfireProbability			  = pSettings->r_float(section,"misfire_probability"); 
	misfireConditionK			  = READ_IF_EXISTS(pSettings, r_float, section, "misfire_condition_k",	1.0f);
	conditionDecreasePerShot	  = pSettings->r_float(section,"condition_shot_dec");
	conditionDecreasePerShotOnHit = READ_IF_EXISTS(pSettings, r_float, section, "condition_shot_dec_on_hit", 0.f);
		
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

	// Added by Axel, to enable optional condition use on any item
	m_flags.set(FUsingCondition, READ_IF_EXISTS(pSettings, r_bool, section, "use_condition", true));

	m_bZoomEnabled = !!pSettings->r_bool(section,"zoom_enabled");
	m_fZoomRotateTime = ROTATION_TIME;
	//if(m_bZoomEnabled && m_pHUD) LoadZoomOffset(*hud_sect, "");

	if(m_eScopeStatus == ALife::eAddonAttachable)
	{
		m_sScopeName = pSettings->r_string(section,"scope_name");

		if (GameConstants::GetUseHQ_Icons())
		{
			m_iScopeX = pSettings->r_s32(section, "scope_x") * 2;
			m_iScopeY = pSettings->r_s32(section, "scope_y") * 2;
		}
		else
		{
			m_iScopeX = pSettings->r_s32(section, "scope_x");
			m_iScopeY = pSettings->r_s32(section, "scope_y");
		}
	}

    
	if(m_eSilencerStatus == ALife::eAddonAttachable)
	{
		m_sSilencerName = pSettings->r_string(section,"silencer_name");

		if (GameConstants::GetUseHQ_Icons())
		{
			m_iSilencerX = pSettings->r_s32(section, "silencer_x") * 2;
			m_iSilencerY = pSettings->r_s32(section, "silencer_y") * 2;
		}
		else
		{
			m_iSilencerX = pSettings->r_s32(section, "silencer_x");
			m_iSilencerY = pSettings->r_s32(section, "silencer_y");
		}
	}

    
	if(m_eGrenadeLauncherStatus == ALife::eAddonAttachable)
	{
		m_sGrenadeLauncherName = pSettings->r_string(section,"grenade_launcher_name");

		if (GameConstants::GetUseHQ_Icons())
		{
			m_iGrenadeLauncherX = pSettings->r_s32(section, "grenade_launcher_x") * 2;
			m_iGrenadeLauncherY = pSettings->r_s32(section, "grenade_launcher_y") * 2;
		}
		else
		{
			m_iGrenadeLauncherX = pSettings->r_s32(section, "grenade_launcher_x");
			m_iGrenadeLauncherY = pSettings->r_s32(section, "grenade_launcher_y");
		}
	}

	InitAddons();

	//////////////////////////////////////
	//время убирания оружия с уровня
	if(pSettings->line_exist(section,"weapon_remove_time"))
		m_dwWeaponRemoveTime = pSettings->r_u32(section,"weapon_remove_time");
	else
		m_dwWeaponRemoveTime = WEAPON_REMOVE_TIME;
	//////////////////////////////////////
	if(pSettings->line_exist(section,"auto_spawn_ammo"))
		m_bAutoSpawnAmmo = pSettings->r_bool(section,"auto_spawn_ammo");
	else
		m_bAutoSpawnAmmo = TRUE;
	//////////////////////////////////////


	m_bHideCrosshairInZoom = true;
	if(pSettings->line_exist(hud_sect, "zoom_hide_crosshair"))
		m_bHideCrosshairInZoom = !!pSettings->r_bool(hud_sect, "zoom_hide_crosshair");	

	//////////////////////////////////////////////////////////

	m_bHasTracers = READ_IF_EXISTS(pSettings, r_bool, section, "tracers", true);
	m_u8TracerColorID = READ_IF_EXISTS(pSettings, r_u8, section, "tracers_color_ID", u8(-1));

	string256						temp;
	for (int i=egdNovice; i<egdCount; ++i) {
		strconcat					(sizeof(temp),temp,"hit_probability_",get_token_name(difficulty_type_token,i));
		m_hit_probability[i]		= READ_IF_EXISTS(pSettings,r_float,section,temp,1.f);
	}

	// Added by Axel, to enable optional condition use on any item
	m_flags.set						(FUsingCondition, READ_IF_EXISTS(pSettings, r_bool, section, "use_condition", true));
	m_bShowWpnStats					= READ_IF_EXISTS(pSettings, r_bool, section, "show_wpn_stats", true);

	m_SuitableRepairKits.clear();
	m_ItemsForRepair.clear();

	LPCSTR repair_kits = READ_IF_EXISTS(pSettings, r_string, section, "suitable_repair_kits", "repair_kit");
	LPCSTR items_for_repair = READ_IF_EXISTS(pSettings, r_string, section, "items_for_repair", "");

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
}

void CWeapon::LoadFireParams		(LPCSTR section, LPCSTR prefix)
{
	camDispersion		= pSettings->r_float		(section,"cam_dispersion"	); 
	camDispersion		= deg2rad					(camDispersion);

	if (pSettings->line_exist(section,"cam_dispersion_inc"))
	{
		camDispersionInc		= pSettings->r_float		(section,"cam_dispersion_inc"	); 
		camDispersionInc		= deg2rad					(camDispersionInc);
	}
	else
		camDispersionInc = 0;

	CShootingObject::LoadFireParams(section, prefix);
};

BOOL CWeapon::net_Spawn		(CSE_Abstract* DC)
{
	BOOL bResult					= inherited::net_Spawn(DC);
	CSE_Abstract					*e	= (CSE_Abstract*)(DC);
	CSE_ALifeItemWeapon			    *E	= smart_cast<CSE_ALifeItemWeapon*>(e);

	//iAmmoCurrent					= E->a_current;
	iAmmoElapsed					= E->a_elapsed;
	m_flagsAddOnState				= E->m_addon_flags.get();
	m_ammoType						= E->ammo_type;
	SetState						(E->wpn_state);
	SetNextState					(E->wpn_state);
	
	m_DefaultCartridge.Load(*m_ammoTypes[m_ammoType], u8(m_ammoType));	
	if(iAmmoElapsed) 
	{
		m_fCurrentCartirdgeDisp = m_DefaultCartridge.m_kDisp;
		for(int i = 0; i < iAmmoElapsed; ++i) 
			m_magazine.push_back(m_DefaultCartridge);
	}


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
	bool bIsActiveItem = m_pCurrentInventory && m_pCurrentInventory->ActiveItem()==this;
	return bIsActiveItem || bWorking || IsPending() || getVisible();
}

void CWeapon::net_Export(NET_Packet& P)
{
	inherited::net_Export	(P);

	P.w_float_q8			(m_fCondition,0.0f,1.0f);

	u8 need_upd				= IsUpdating() ? 1 : 0;
	P.w_u8					(need_upd);
	P.w_u16					(u16(iAmmoElapsed));
	P.w_u8					(m_flagsAddOnState);
	P.w_u8					((u8)m_ammoType);
	P.w_u8					((u8)GetState());
	P.w_u8					((u8)m_bZoomMode);
}

void CWeapon::net_Import(NET_Packet& P)
{
	inherited::net_Import (P);

	P.r_float_q8			(m_fCondition,0.0f,1.0f);

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
	save_data		(iAmmoElapsed,		output_packet);
	save_data		(m_flagsAddOnState, output_packet);
	save_data		(m_ammoType,		output_packet);
	save_data		(m_bZoomMode,		output_packet);
}

void CWeapon::load(IReader &input_packet)
{
	inherited::load	(input_packet);
	load_data		(iAmmoElapsed,		input_packet);
	load_data		(m_flagsAddOnState, input_packet);
	UpdateAddonsVisibility	();
	load_data		(m_ammoType,		input_packet);
	load_data		(m_bZoomMode,		input_packet);

	if (m_bZoomMode)	OnZoomIn();
		else			OnZoomOut();
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

	FireEnd();
	SetPending					(FALSE);
	SwitchState					(eHidden);

	m_strapped_mode				= false;
	m_strapped_mode_rifle		= false;
	//m_zoom_params.m_bIsZoomModeNow	= false;
	UpdateXForm					();

}

void CWeapon::OnH_A_Independent	()
{
	m_dwWeaponIndependencyTime = Level().timeServer();
	inherited::OnH_A_Independent();
	Light_Destroy				();
};

void CWeapon::OnH_A_Chield		()
{
	inherited::OnH_A_Chield		();

	UpdateAddonsVisibility		();
};

void CWeapon::OnActiveItem ()
{
	SwitchState					(eShowing);
	inherited::OnActiveItem		();
	//если мы занружаемся и оружие было в руках
//.	SetState					(eIdle);
//.	SetNextState				(eIdle);
}

void CWeapon::OnHiddenItem ()
{
	if(IsGameTypeSingle())
		SwitchState(eHiding);
	else
		SwitchState(eHidden);
	inherited::OnHiddenItem();
	SetState					(eHidden);
	SetNextState				(eHidden);
	m_set_next_ammoType_on_reload	= u32(-1);
}


void CWeapon::OnH_B_Chield		()
{
	m_dwWeaponIndependencyTime = 0;
	inherited::OnH_B_Chield		();

	OnZoomOut					();
	m_set_next_ammoType_on_reload	= u32(-1);
}


void CWeapon::UpdateCL		()
{
	inherited::UpdateCL		();
	UpdateHUDAddonsVisibility();
	//подсветка от выстрела
	UpdateLight				();

	//нарисовать партиклы
	UpdateFlameParticles	();
	UpdateFlameParticles2	();

	if(!IsGameTypeSingle())
		make_Interpolation		();
	
	VERIFY(smart_cast<IKinematics*>(Visual()));
	
	if( (GetNextState()==GetState()) && IsGameTypeSingle() && H_Parent()==Level().CurrentEntity())
	{
		/*CActor* pActor	= smart_cast<CActor*>(H_Parent());
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
		} */
	}
}

bool  CWeapon::need_renderable()
{
	return !( IsZoomed() && ZoomTexture() && !IsRotatingToZoom() );
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

	inherited::renderable_Render		();
}

void CWeapon::signal_HideComplete()
{
	if(H_Parent()) 
		setVisible			(FALSE);
	SetPending				(FALSE);
}

void CWeapon::SetDefaults()
{
	bWorking2			= false;
	SetPending			(FALSE);

	m_flags.set			(FUsingCondition, TRUE);
	bMisfire			= false;
	m_flagsAddOnState	= 0;
	m_bZoomMode			= false;
}

void CWeapon::UpdatePosition(const Fmatrix& trans)
{
	Position().set		(trans.c);

	if (m_strapped_mode || m_strapped_mode_rifle)
		XFORM().mul		(trans, m_StrapOffset);
	else
		XFORM().mul		(trans, m_Offset);

	VERIFY				(!fis_zero(DET(renderable.xform)));
}


bool CWeapon::Action(s32 cmd, u32 flags) 
{
	if(inherited::Action(cmd, flags)) return true;

	
	switch(cmd) 
	{
		case kWPN_FIRE: 
			{
				//если оружие чем-то занято, то ничего не делать
				{				
					if(IsPending())		
						return				false;

					if(flags&CMD_START) 
						FireStart			();
					else 
						FireEnd();
				};

			} 
			return true;
		case kWPN_NEXT: 
			{
				if(IsPending() || OnClient()) 
				{
					return false;
				}
									
				if(flags&CMD_START) 
				{
					u32 l_newType = m_ammoType;
					bool b1,b2;
					do 
					{
						l_newType = (l_newType+1)%m_ammoTypes.size();
						b1 = l_newType != m_ammoType;
						b2 = unlimited_ammo() ? false : (!m_pCurrentInventory->GetAny(*m_ammoTypes[l_newType]));						
					} while( b1 && b2);

					if(l_newType != m_ammoType) 
					{
						m_set_next_ammoType_on_reload = l_newType;						
/*						m_ammoType = l_newType;
						m_pAmmo = NULL;
						if (unlimited_ammo())
						{
							m_DefaultCartridge.Load(*m_ammoTypes[m_ammoType], u8(m_ammoType));
						};							
*/
						if(OnServer()) Reload();
					}
				}
			} 
            return true;

		case kWPN_ZOOM:
			if(IsZoomEnabled())
			{
                if(flags&CMD_START && !IsPending())
					OnZoomIn();
                else if(IsZoomed())
					OnZoomOut();
				return true;
			}else 
				return false;

		case kWPN_ZOOM_INC:
		case kWPN_ZOOM_DEC:
			if(IsZoomEnabled() && IsZoomed())
			{
				if(cmd==kWPN_ZOOM_INC)  ZoomInc();
				else					ZoomDec();
				return true;
			}else
				return false;
	}
	return false;
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
		D->s_gameid					= u8(GameID());
		D->s_RP						= 0xff;
		D->ID						= 0xffff;
		if (ParentID == 0xffffffff)	
			D->ID_Parent			= (u16)H_Parent()->ID();
		else
			D->ID_Parent			= (u16)ParentID;

		D->ID_Phantom				= 0xffff;
		D->s_flags.assign			(M_SPAWN_OBJECT_LOCAL);
		D->RespawnTime				= 0;
		l_pA->m_tNodeID				= g_dedicated_server ? u32(-1) : ai_location().level_vertex_id();

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

int CWeapon::GetAmmoCurrent(bool use_item_to_spawn) const
{
	int l_count = iAmmoElapsed;
	if(!m_pCurrentInventory) return l_count;

	//чтоб не делать лишних пересчетов
	if(m_pCurrentInventory->ModifyFrame()<=m_dwAmmoCurrentCalcFrame)
		return l_count + iAmmoCurrent;

 	m_dwAmmoCurrentCalcFrame = Device.dwFrame;
	iAmmoCurrent = 0;

	for(int i = 0; i < (int)m_ammoTypes.size(); ++i) 
	{
		LPCSTR l_ammoType = *m_ammoTypes[i];

		for(TIItemContainer::iterator l_it = m_pCurrentInventory->m_belt.begin(); m_pCurrentInventory->m_belt.end() != l_it; ++l_it) 
		{
			CWeaponAmmo *l_pAmmo = smart_cast<CWeaponAmmo*>(*l_it);

			if(l_pAmmo && !xr_strcmp(l_pAmmo->cNameSect(), l_ammoType)) 
			{
				iAmmoCurrent = iAmmoCurrent + l_pAmmo->m_boxCurr;
			}
		}

		for(TIItemContainer::iterator l_it = m_pCurrentInventory->m_ruck.begin(); m_pCurrentInventory->m_ruck.end() != l_it; ++l_it) 
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


int CWeapon::GetSuitableAmmoTotal(bool use_item_to_spawn) const
{
	int l_count = iAmmoElapsed;
	if (!m_pCurrentInventory) return l_count;

	//чтоб не делать лишних пересчетов
	if (m_pCurrentInventory->ModifyFrame() <= m_dwAmmoCurrentCalcFrame)
		return l_count + iAmmoCurrent;

	m_dwAmmoCurrentCalcFrame = Device.dwFrame;
	iAmmoCurrent = 0;

	for (int i = 0; i < (int)m_ammoTypes.size(); ++i)
	{
		LPCSTR l_ammoType = *m_ammoTypes[i];

		for (TIItemContainer::iterator l_it = m_pCurrentInventory->m_belt.begin(); m_pCurrentInventory->m_belt.end() != l_it; ++l_it)
		{
			CWeaponAmmo* l_pAmmo = smart_cast<CWeaponAmmo*>(*l_it);

			if (l_pAmmo && !xr_strcmp(l_pAmmo->cNameSect(), l_ammoType))
			{
				iAmmoCurrent = iAmmoCurrent + l_pAmmo->m_boxCurr;
			}
		}

		for (TIItemContainer::iterator l_it = m_pCurrentInventory->m_ruck.begin(); m_pCurrentInventory->m_ruck.end() != l_it; ++l_it)
		{
			CWeaponAmmo* l_pAmmo = smart_cast<CWeaponAmmo*>(*l_it);
			if (l_pAmmo && !xr_strcmp(l_pAmmo->cNameSect(), l_ammoType))
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

float CWeapon::GetConditionMisfireProbability() const
{
	if( GetCondition()>0.95f ) return 0.0f;

	float mis = misfireProbability+powf(1.f-GetCondition(), 3.f)*misfireConditionK;
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


bool CWeapon::IsGrenadeLauncherAttached() const
{
	return (CSE_ALifeItemWeapon::eAddonAttachable == m_eGrenadeLauncherStatus &&
			0 != (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher)) || 
			CSE_ALifeItemWeapon::eAddonPermanent == m_eGrenadeLauncherStatus;
}

bool CWeapon::IsScopeAttached() const
{
	return (CSE_ALifeItemWeapon::eAddonAttachable == m_eScopeStatus &&
			0 != (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonScope)) || 
			CSE_ALifeItemWeapon::eAddonPermanent == m_eScopeStatus;

}

bool CWeapon::IsSilencerAttached() const
{
	return (CSE_ALifeItemWeapon::eAddonAttachable == m_eSilencerStatus &&
			0 != (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonSilencer)) || 
			CSE_ALifeItemWeapon::eAddonPermanent == m_eSilencerStatus;
}

bool CWeapon::GrenadeLauncherAttachable()
{
	return (CSE_ALifeItemWeapon::eAddonAttachable == m_eGrenadeLauncherStatus);
}
bool CWeapon::ScopeAttachable()
{
	return (CSE_ALifeItemWeapon::eAddonAttachable == m_eScopeStatus);
}
bool CWeapon::SilencerAttachable()
{
	return (CSE_ALifeItemWeapon::eAddonAttachable == m_eSilencerStatus);
}

LPCSTR wpn_scope				= "wpn_scope";
LPCSTR wpn_silencer				= "wpn_silencer";
LPCSTR wpn_grenade_launcher		= "wpn_grenade_launcher";
LPCSTR wpn_launcher				= "wpn_launcher";

void CWeapon::UpdateHUDAddonsVisibility()
{//actor only
	if(!GetHUDmode())										return;

//.	return;

	if(ScopeAttachable())
	{
		HudItemData()->set_bone_visible(wpn_scope, IsScopeAttached() );
	}

	if(m_eScopeStatus==ALife::eAddonDisabled )
	{
		HudItemData()->set_bone_visible(wpn_scope, FALSE, TRUE );
	}else
		if(m_eScopeStatus==ALife::eAddonPermanent)
			HudItemData()->set_bone_visible(wpn_scope, TRUE, TRUE );

	if(SilencerAttachable())
	{
		HudItemData()->set_bone_visible(wpn_silencer, IsSilencerAttached());
	}
	if(m_eSilencerStatus==ALife::eAddonDisabled )
	{
		HudItemData()->set_bone_visible(wpn_silencer, FALSE, TRUE);
	}
	else
		if(m_eSilencerStatus==ALife::eAddonPermanent)
			HudItemData()->set_bone_visible(wpn_silencer, TRUE, TRUE);

	if(GrenadeLauncherAttachable())
	{
		HudItemData()->set_bone_visible(wpn_grenade_launcher, IsGrenadeLauncherAttached());
	}
	if(m_eGrenadeLauncherStatus==ALife::eAddonDisabled )
	{
		HudItemData()->set_bone_visible(wpn_grenade_launcher, FALSE, TRUE);
	}else
		if(m_eGrenadeLauncherStatus==ALife::eAddonPermanent)
			HudItemData()->set_bone_visible(wpn_grenade_launcher, TRUE, TRUE);

}

void CWeapon::UpdateAddonsVisibility()
{
	IKinematics* pWeaponVisual = smart_cast<IKinematics*>(Visual()); R_ASSERT(pWeaponVisual);

	u16  bone_id;
	UpdateHUDAddonsVisibility								();	

	bone_id = pWeaponVisual->LL_BoneID					(wpn_scope);
	if(ScopeAttachable())
	{
		if(IsScopeAttached())
		{
			if(FALSE==pWeaponVisual->LL_GetBoneVisible		(bone_id))
			pWeaponVisual->LL_SetBoneVisible				(bone_id,TRUE,TRUE);
		}else{
			if(pWeaponVisual->LL_GetBoneVisible				(bone_id))
				pWeaponVisual->LL_SetBoneVisible			(bone_id,FALSE,TRUE);
		}
	}
	if(m_eScopeStatus==CSE_ALifeItemWeapon::eAddonDisabled && bone_id!=BI_NONE && 
		pWeaponVisual->LL_GetBoneVisible(bone_id) )

		pWeaponVisual->LL_SetBoneVisible			(bone_id,FALSE,TRUE);

	bone_id = pWeaponVisual->LL_BoneID					(wpn_silencer);
	if(SilencerAttachable())
	{
		if(IsSilencerAttached()){
			if(FALSE==pWeaponVisual->LL_GetBoneVisible		(bone_id))
				pWeaponVisual->LL_SetBoneVisible			(bone_id,TRUE,TRUE);
		}else{
			if( pWeaponVisual->LL_GetBoneVisible				(bone_id))
				pWeaponVisual->LL_SetBoneVisible			(bone_id,FALSE,TRUE);
		}
	}
	if(m_eSilencerStatus==CSE_ALifeItemWeapon::eAddonDisabled && bone_id!=BI_NONE && 
		pWeaponVisual->LL_GetBoneVisible(bone_id) )

		pWeaponVisual->LL_SetBoneVisible			(bone_id,FALSE,TRUE);

	bone_id = pWeaponVisual->LL_BoneID					(wpn_launcher);
	if(GrenadeLauncherAttachable())
	{
		if(IsGrenadeLauncherAttached())
		{
			if(FALSE==pWeaponVisual->LL_GetBoneVisible		(bone_id))
				pWeaponVisual->LL_SetBoneVisible			(bone_id,TRUE,TRUE);
		}else{
			if(pWeaponVisual->LL_GetBoneVisible				(bone_id))
				pWeaponVisual->LL_SetBoneVisible			(bone_id,FALSE,TRUE);
		}
	}
	if(m_eGrenadeLauncherStatus==CSE_ALifeItemWeapon::eAddonDisabled && bone_id!=BI_NONE && 
		pWeaponVisual->LL_GetBoneVisible(bone_id) )

		pWeaponVisual->LL_SetBoneVisible			(bone_id,FALSE,TRUE);
	

	pWeaponVisual->CalculateBones_Invalidate				();
	pWeaponVisual->CalculateBones							();
}

bool CWeapon::Activate() 
{
	UpdateAddonsVisibility();
	return inherited::Activate();
}

void CWeapon::InitAddons()
{
}

float CWeapon::CurrentZoomFactor	()
{
	return IsScopeAttached() ? m_fScopeZoomFactor : m_fIronSightZoomFactor;
};

void CWeapon::OnZoomIn()
{
	//Alun: Force switch to first-person for zooming
	CActor* pA = smart_cast<CActor*>(H_Parent());
	if (pA && pA->active_cam() == eacLookAt)
	{
		pA->cam_Set(eacFirstEye);
		m_freelook_switch_back = true;
	}

	if (GetHUDmode())
	{
		GamePersistent().SetPickableEffectorDOF(true);
	}

	m_bZoomMode = true;
	m_fZoomFactor = CurrentZoomFactor();
	AllowHudInertion(FALSE);
}

void CWeapon::OnZoomOut()
{
	//Alun: Switch back to third-person if was forced
	if (m_freelook_switch_back)
	{
		CActor* pA = smart_cast<CActor*>(H_Parent());
		if (pA)
			pA->cam_Set(eacLookAt);

		m_freelook_switch_back = false;
	}

	if (GetHUDmode())
	{
		GamePersistent().SetPickableEffectorDOF(false);
	}

	m_bZoomMode = false;
	m_fZoomFactor = g_fov;

	AllowHudInertion(TRUE);
}

CUIStaticItem* CWeapon::ZoomTexture()
{
	if (UseScopeTexture())
		return m_UIScope;
	else
		return NULL;
}

void CWeapon::SwitchState(u32 S)
{
	if (OnClient()) return;

	SetNextState		( S );	// Very-very important line of code!!! :)
	if (CHudItem::object().Local() && !CHudItem::object().getDestroy()/* && (S!=NEXT_STATE)*/ 
		&& m_pCurrentInventory && OnServer())	
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
	m_can_be_strapped_rifle		= (GetSlot() == RIFLE_SLOT);
	m_strapped_mode				= false;
	m_strapped_mode_rifle		= false;
	
	if (pSettings->line_exist(section,"strap_bone0"))
		m_strap_bone0			= pSettings->r_string(section,"strap_bone0");
	else
	{
		m_can_be_strapped		= false;
		m_can_be_strapped_rifle = false;
	}
	
	if (pSettings->line_exist(section,"strap_bone1"))
		m_strap_bone1			= pSettings->r_string(section,"strap_bone1");
	else
	{
		m_can_be_strapped		= false;
		m_can_be_strapped_rifle = false;
	}

	if (m_eScopeStatus == ALife::eAddonAttachable) {
		m_addon_holder_range_modifier	= READ_IF_EXISTS(pSettings,r_float,m_sScopeName,"holder_range_modifier",m_holder_range_modifier);
		m_addon_holder_fov_modifier		= READ_IF_EXISTS(pSettings,r_float,m_sScopeName,"holder_fov_modifier",m_holder_fov_modifier);
	}
	else {
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
	else
	{
		m_can_be_strapped		= false;
		m_can_be_strapped_rifle = false;
	}

	m_ef_main_weapon_type	= READ_IF_EXISTS(pSettings,r_u32,section,"ef_main_weapon_type",u32(-1));
	m_ef_weapon_type		= READ_IF_EXISTS(pSettings,r_u32,section,"ef_weapon_type",u32(-1));
}

void CWeapon::create_physic_shell()
{
	CPhysicsShellHolder::create_physic_shell();
}

void CWeapon::activate_physic_shell()
{
	CPhysicsShellHolder::activate_physic_shell();
}

void CWeapon::setup_physic_shell()
{
	CPhysicsShellHolder::setup_physic_shell();
}

int		g_iWeaponRemove = 1;

bool CWeapon::NeedToDestroyObject()	const
{
	if (H_Parent()) return false;
	if (g_iWeaponRemove == -1) return false;
	if (g_iWeaponRemove == 0) return true;
	return (TimePassedAfterIndependant() > m_dwWeaponRemoveTime);
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
	if (GetAmmoCurrent(true) || m_ammoTypes.empty())
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


void CWeapon::UpdateHudAdditional(Fmatrix& trans)
{
	CActor* pActor	= smart_cast<CActor*>(H_Parent());
	if(!pActor)		return;


	if((pActor->IsZoomAimingMode() && m_fZoomRotationFactor <= 1.f) ||
		(!pActor->IsZoomAimingMode() && m_fZoomRotationFactor > 0.f))
	{
		u8 idx = GetCurrentHudOffsetIdx();
//		if(idx==0)					return;

		attachable_hud_item*		hi = HudItemData();
		R_ASSERT					(hi);
		Fvector						curr_offs, curr_rot;
		curr_offs					= hi->m_measures.m_hands_offset[0][idx];//pos,aim
		curr_rot					= hi->m_measures.m_hands_offset[1][idx];//rot,aim
		curr_offs.mul				(m_fZoomRotationFactor);
		curr_rot.mul				(m_fZoomRotationFactor);

		Fmatrix						hud_rotation;
		hud_rotation.identity		();
		hud_rotation.rotateX		(curr_rot.x);

		Fmatrix						hud_rotation_y;
		hud_rotation_y.identity		();
		hud_rotation_y.rotateY		(curr_rot.y);
		hud_rotation.mulA_43		(hud_rotation_y);

		hud_rotation_y.identity		();
		hud_rotation_y.rotateZ		(curr_rot.z);
		hud_rotation.mulA_43		(hud_rotation_y);

		hud_rotation.translate_over	(curr_offs);
		trans.mulB_43				(hud_rotation);

		if(pActor->IsZoomAimingMode())
			m_fZoomRotationFactor += Device.fTimeDelta/m_fZoomRotateTime;
		else
			m_fZoomRotationFactor -= Device.fTimeDelta/m_fZoomRotateTime;

		clamp(m_fZoomRotationFactor, 0.f, 1.f);
	}
}

void	CWeapon::SetAmmoElapsed	(int ammo_count)
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

void CWeapon::OnDrawUI()
{
	if(IsZoomed() && ZoomHideCrosshair()){
		if(ZoomTexture() && !IsRotatingToZoom()){
			ZoomTexture()->SetPos	(0,0);
			ZoomTexture()->SetRect	(0,0,UI_BASE_WIDTH, UI_BASE_HEIGHT);
			ZoomTexture()->Render	();

//			m_UILens.Draw();
		}
	}
}

u8 CWeapon::GetCurrentHudOffsetIdx()
{
	if (!ParentIsActor())
		return 0;

	bool b_aiming = ((IsZoomed() && m_fZoomRotationFactor <= 1.f) ||
		(!IsZoomed() && m_fZoomRotationFactor > 0.f));

	if (!b_aiming)
		return 0;
	else
		return 1;
}

bool CWeapon::MovingAnimAllowedNow()
{
	return !IsZoomed();
}

bool CWeapon::IsHudModeNow()
{
	return (HudItemData() != nullptr);
}

bool CWeapon::render_item_ui_query()
{
	bool b_is_active_item = (m_pCurrentInventory->ActiveItem() == this);
	bool res = b_is_active_item && IsZoomed() && ZoomHideCrosshair() && ZoomTexture() && !IsRotatingToZoom();
	return res;
}

void CWeapon::render_item_ui()
{
	/*if (m_pVision)
		m_pVision->Draw();

	ZoomTexture()->Update();
	ZoomTexture()->Draw();	*/
}

bool CWeapon::unlimited_ammo() 
{ 
	if (GameID() == GAME_SINGLE	)
		return psActorFlags.test(AF_UNLIMITEDAMMO) && 
				m_DefaultCartridge.m_flags.test(CCartridge::cfCanBeUnlimited); 

	return (GameID()!=GAME_ARTEFACTHUNT) && 
		m_DefaultCartridge.m_flags.test(CCartridge::cfCanBeUnlimited); 
			
};

LPCSTR	CWeapon::GetCurrentAmmo_ShortName	()
{
	if (m_magazine.empty()) return ("");
	CCartridge &l_cartridge = m_magazine.back();
	return *(l_cartridge.m_InvShortName);
}

float CWeapon::Weight() const
{
	float res = CInventoryItemObject::Weight();
	if (IsGrenadeLauncherAttached() && GetGrenadeLauncherName().size())
	{
		res += pSettings->r_float(GetGrenadeLauncherName(), "inv_weight");
	}
	if (IsScopeAttached() && GetScopeName().size())
	{
		res += pSettings->r_float(GetScopeName(), "inv_weight");
	}
	if (IsSilencerAttached() && GetSilencerName().size()) {
		res += pSettings->r_float(GetSilencerName(), "inv_weight");
	}

	if (iAmmoElapsed)
	{
		float w = pSettings->r_float(*m_ammoTypes[m_ammoType], "inv_weight");
		float bs = pSettings->r_float(*m_ammoTypes[m_ammoType], "box_size");

		res += w * (iAmmoElapsed / bs);
	}
	return res;
}

u32 CWeapon::Cost() const
{
	u32 res = CInventoryItem::Cost();
	if (IsGrenadeLauncherAttached() && GetGrenadeLauncherName().size()) {
		res += pSettings->r_u32(GetGrenadeLauncherName(), "cost");
	}
	if (IsScopeAttached() && GetScopeName().size()) {
		res += pSettings->r_u32(GetScopeName(), "cost");
	}
	if (IsSilencerAttached() && GetSilencerName().size()) {
		res += pSettings->r_u32(GetSilencerName(), "cost");
	}

	if (iAmmoElapsed)
	{
		float w = pSettings->r_float(m_ammoTypes[m_ammoType].c_str(), "cost");
		float bs = pSettings->r_float(m_ammoTypes[m_ammoType].c_str(), "box_size");

		res += iFloor(w * (iAmmoElapsed / bs));
	}
	return res;
}

void CWeapon::Hide		()
{
	if(IsGameTypeSingle())
		SwitchState(eHiding);
	else
		SwitchState(eHidden);

	OnZoomOut();
}

void CWeapon::Show		()
{
	SwitchState(eShowing);
}

bool CWeapon::show_crosshair()
{
	return !IsZoomed();
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
	CObject* O=H_Parent();
	CEntityAlive* EA=smart_cast<CEntityAlive*>(O);
	return EA->cast_actor()!=0;
}

const float &CWeapon::hit_probability	() const
{
	VERIFY					((g_SingleGameDifficulty >= egdNovice) && (g_SingleGameDifficulty <= egdMaster)); 
	return					(m_hit_probability[egdNovice]);
}

void CWeapon::OnBulletHit()
{
	if (!fis_zero(conditionDecreasePerShotOnHit))
		ChangeCondition(-conditionDecreasePerShotOnHit);
}

void CWeapon::OnStateSwitch(u32 S, u32 oldState)
{
	inherited::OnStateSwitch(S, oldState);
	m_dwAmmoCurrentCalcFrame = 0;

	if (H_Parent() == Level().CurrentEntity())
	{
		CActor* current_actor = smart_cast<CActor*>(H_Parent());

		/*if (&HUD().GetUI()->UIGame()->ActorMenu() && HUD().GetUI()->UIGame()->ActorMenu().GetMenuMode() == mmUndefined)
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
		} */
	}
}

void CWeapon::OnAnimationEnd(u32 state)
{
	inherited::OnAnimationEnd(state);
}