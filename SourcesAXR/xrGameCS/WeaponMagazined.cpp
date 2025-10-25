#include "pch_script.h"
#include "hudmanager.h"
#include "WeaponMagazined.h"
#include "entity.h"
#include "actor.h"
#include "ParticlesObject.h"
#include "scope.h"
#include "silencer.h"
#include "GrenadeLauncher.h"
#include "LaserDesignator.h"
#include "TacticalTorch.h"
#include "WeaponAddonStock1.h"
#include "WeaponAddonGripHorizontal.h"
#include "WeaponAddonGripVertical.h"
#include "WeaponAddonHandguard.h"
#include "WeaponAddonPistolGrip.h"
#include "inventory.h"
#include "xrserver_objects_alife_items.h"
#include "ActorEffector.h"
#include "EffectorZoomInertion.h"
#include "xr_level_controller.h"
#include "level.h"
#include "object_broker.h"
#include "string_table.h"
#include "MPPlayersBag.h"
#include "ui/UIXmlInit.h"
#include "ui/UIStatic.h"
#include "game_object_space.h"
#include "script_callback_ex.h"
#include "script_game_object.h"
#include "player_hud.h"
#include "Torch.h"
#include "ActorNightVision.h"
#include "CustomDetector.h"
#include "AdvancedXrayGameConstants.h"

ENGINE_API  extern float psHUD_FOV;
ENGINE_API  extern float psHUD_FOV_def;

BOOL m_b_advanced_shoot_effectors = FALSE;
float m_b_advanced_se_factor = 1.0f;

//CUIXml*				pWpnScopeXml = NULL;

CWeaponMagazined::CWeaponMagazined(ESoundTypes eSoundType) : CWeapon()
{
	m_eSoundShow				= ESoundTypes(SOUND_TYPE_ITEM_TAKING | eSoundType);
	m_eSoundHide				= ESoundTypes(SOUND_TYPE_ITEM_HIDING | eSoundType);
	m_eSoundShot				= ESoundTypes(SOUND_TYPE_WEAPON_SHOOTING | eSoundType);
	m_eSoundEmptyClick			= ESoundTypes(SOUND_TYPE_WEAPON_EMPTY_CLICKING | eSoundType);
	m_eSoundReload				= ESoundTypes(SOUND_TYPE_WEAPON_RECHARGING | eSoundType);
	m_eSoundClose				= ESoundTypes(SOUND_TYPE_WEAPON_RECHARGING);

	m_sounds_enabled			= true;

	psWpnAnimsFlag = { 0 };
	
	m_sSilencerFlameParticles	= m_sSilencerSmokeParticles = NULL;

	m_bFireSingleShot			= false;
	m_iShotNum					= 0;
	m_fOldBulletSpeed			= 0;
	bullet_cnt					= 0;
	m_iQueueSize				= WEAPON_ININITE_QUEUE;
	m_bLockType					= false;
	m_bNeedBulletInGun			= false;
	m_bHasDifferentFireModes	= false;
	m_opened					= false;
	m_bUseFiremodeChangeAnim	= true;
	bHasBulletsToHide			= false;

	m_sSndShotCurrent			= nullptr;
	m_bIsRevolver				= false;
	m_iMagClickStartRound		= 0;
}

CWeaponMagazined::~CWeaponMagazined()
{
	// sounds
}


void CWeaponMagazined::net_Destroy()
{
	inherited::net_Destroy();
}

void CWeaponMagazined::SetAnimFlag(u32 flag, LPCSTR anim_name)
{
	if (pSettings->line_exist(m_hud_sect, anim_name))
		psWpnAnimsFlag.set(flag, TRUE);
	else
		psWpnAnimsFlag.set(flag, FALSE);
}

void CWeaponMagazined::Load	(LPCSTR section)
{
	inherited::Load		(section);

	// Ïðîâåðÿåì íàëè÷èå àíèìàöèé
	SetAnimFlag(ANM_SHOW_EMPTY,		"anm_show_empty");
	SetAnimFlag(ANM_HIDE_EMPTY,		"anm_hide_empty");
	SetAnimFlag(ANM_IDLE_EMPTY,		"anm_idle_empty");
	SetAnimFlag(ANM_AIM_EMPTY,		"anm_idle_aim_empty");
	SetAnimFlag(ANM_BORE_EMPTY,		"anm_inspect_weapon_empty"/*"anm_bore_empty"*/);
	SetAnimFlag(ANM_SHOT_EMPTY,		"anm_shot_l");
	SetAnimFlag(ANM_SPRINT_EMPTY,	"anm_idle_sprint_empty");
	SetAnimFlag(ANM_MOVING_EMPTY,	"anm_idle_moving_empty");
	SetAnimFlag(ANM_RELOAD_EMPTY,	"anm_reload_empty");
	SetAnimFlag(ANM_MISFIRE,		"anm_reload_misfire");
	SetAnimFlag(ANM_SHOT_AIM,		"anm_shots_when_aim");
	
	// Sounds
	m_sounds.LoadSound(section,"snd_draw",			"sndShow",			false,	m_eSoundShow		);
	m_sounds.LoadSound(section,"snd_holster",		"sndHide",			false,	m_eSoundHide		);

	//Alundaio: LAYERED_SND_SHOOT
	m_sounds.LoadSound(section, "snd_shoot", "sndShot", false, m_eSoundShot);
	m_sounds.LoadSound(section, "snd_mag_shot_hip", "sndMagShotHip", false, m_eSoundShot);
	m_sounds.LoadSound(section, "snd_mag_shot", "sndMagShot", false, m_eSoundShot);

	if (WeaponSoundExist(section, "snd_shoot_actor", true))
	{
		m_sounds.LoadSound(section, "snd_shoot_actor", "sndShotActor", false, m_eSoundShot);
		m_sounds.LoadSound(section, "snd_shoot_actor_last", "sndShotActorLast", false, m_eSoundShot);
		m_sounds.LoadSound(section, "snd_silncer_shoot_actor", "sndSilencerShotActor", false, m_eSoundShot);
		m_sounds.LoadSound(section, "snd_silncer_shoot_actor_last", "sndSilencerShotActorLast", false, m_eSoundShot);

		m_sounds.LoadSound(section, "snd_shoot_actor_hip", "sndShotActorh", false, m_eSoundShot);
		m_sounds.LoadSound(section, "snd_shoot_actor_hip_last", "sndShotActorhLasth", false, m_eSoundShot);
		m_sounds.LoadSound(section, "snd_silncer_shoot_actor_hip", "sndSilencerShotActorh", false, m_eSoundShot);
		m_sounds.LoadSound(section, "snd_silncer_shoot_actor_hip_last", "sndSilencerShotActorhLasth", false, m_eSoundShot);
	}
	//-Alundaio

	m_sounds.LoadSound(section, "snd_shoot_last", "sndShotLast", false, m_eSoundShot);
	m_sounds.LoadSound(section, "snd_silncer_shoot_last", "sndSilencerShotLast", false, m_eSoundShot);

	if (m_bIndoorSoundsEnabled)
	{
		m_sounds.LoadSound(section, "snd_shoot_indoor", "sndShotIndoor", false, m_eSoundShot);
		m_sounds.LoadSound(section, "snd_shoot_indoor_last", "sndShotLastIndoor", false, m_eSoundShot);
		m_sounds.LoadSound(section, "snd_silncer_shoot_indoor", "sndSilencerShotIndoor", false, m_eSoundShot);
		m_sounds.LoadSound(section, "snd_silncer_shoot_indoor_last", "sndSilencerShotLastIndoor", false, m_eSoundShot);

		if (WeaponSoundExist(section, "snd_shoot_actor", true))
		{
			m_sounds.LoadSound(section, "snd_shoot_actor_indoor", "sndShotActorIndoor", false, m_eSoundShot);
			m_sounds.LoadSound(section, "snd_shoot_actor_indoor_last", "sndShotActorLastIndoor", false, m_eSoundShot);
			m_sounds.LoadSound(section, "snd_silncer_shoot_actor_indoor", "sndSilencerShotActorIndoor", false, m_eSoundShot);
			m_sounds.LoadSound(section, "snd_silncer_shoot_actor_indoor_last", "sndSilencerShotActorLastIndoor", false, m_eSoundShot);

			m_sounds.LoadSound(section, "snd_shoot_actor_hip_indoor", "sndShotActorhIndoorh", false, m_eSoundShot);
			m_sounds.LoadSound(section, "snd_shoot_actor_hip_indoor_last", "sndShotActorhLasthIndoorh", false, m_eSoundShot);
			m_sounds.LoadSound(section, "snd_silncer_shoot_actor_hip_indoor", "sndSilencerShotActorhIndoorh", false, m_eSoundShot);
			m_sounds.LoadSound(section, "snd_silncer_shoot_actor_hip_indoor_last", "sndSilencerShotActorhLasthIndoorh", false, m_eSoundShot);

		}
	}

	m_sSndShotCurrent = IsSilencerAttached() ? "sndSilencerShot" : "sndShot";

	if (WeaponSoundExist(section, "snd_inspect_weapon", true))
		m_sounds.LoadSound(section, "snd_inspect_weapon",		"sndInspectWeapon", false, m_eSoundEmptyClick);

	if (WeaponSoundExist(section, "snd_inspect_weapon_empty", true))
		m_sounds.LoadSound(section, "snd_inspect_weapon_empty", "sndInspectWeaponEmpty", false, m_eSoundEmptyClick);

	if (WeaponSoundExist(section, "snd_inspect_weapon_misfire", true))
		m_sounds.LoadSound(section, "snd_inspect_weapon_misfire", "sndInspectWeaponMisfire", false, m_eSoundEmptyClick);

	if (WeaponSoundExist(section, "snd_inspect_weapon_gl", true))
		m_sounds.LoadSound(section, "snd_inspect_weapon_gl", "sndInspectWeaponGl", false, m_eSoundEmptyClick);

	if (WeaponSoundExist(section, "snd_inspect_weapon_empty_gl", true))
		m_sounds.LoadSound(section, "snd_inspect_weapon_empty_gl", "sndInspectWeaponEmptyGl", false, m_eSoundEmptyClick);

	if (WeaponSoundExist(section, "snd_inspect_weapon_misfire_gl", true))
		m_sounds.LoadSound(section, "snd_inspect_weapon_misfire_gl", "sndInspectWeaponMisfireGl", false, m_eSoundEmptyClick);

	if (WeaponSoundExist(section, "snd_inspect_weapon_gl_used", true))
		m_sounds.LoadSound(section, "snd_inspect_weapon_gl_used", "sndInspectWeaponGlUsed", false, m_eSoundEmptyClick);

	if (WeaponSoundExist(section, "snd_inspect_weapon_empty_gl_used", true))
		m_sounds.LoadSound(section, "snd_inspect_weapon_empty_gl_used", "sndInspectWeaponEmptyGlUsed", false, m_eSoundEmptyClick);

	if (WeaponSoundExist(section, "snd_inspect_weapon_misfire_gl_used", true))
		m_sounds.LoadSound(section, "snd_inspect_weapon_misfire_gl_used", "sndInspectWeaponMisfireGlUsed", false, m_eSoundEmptyClick);


	m_sounds.LoadSound(section,"snd_empty",				"sndEmptyClick",		false,	m_eSoundEmptyClick);
	m_sounds.LoadSound(section, "snd_reflect", "sndReflect", true, m_eSoundReflect);

	if (WeaponSoundExist(section, "snd_changefiremode", true))
		m_sounds.LoadSound(section, "snd_changefiremode", "sndFireModes", false, m_eSoundEmptyClick	);

	if (WeaponSoundExist(section, "snd_laser_on", true))
		m_sounds.LoadSound(section, "snd_laser_on", "sndLaserOn", false, m_eSoundEmptyClick);
	if (WeaponSoundExist(section, "snd_laser_off", true))
		m_sounds.LoadSound(section, "snd_laser_off", "sndLaserOff", false, m_eSoundEmptyClick);
	if (WeaponSoundExist(section, "snd_torch_on", true))
		m_sounds.LoadSound(section, "snd_torch_on", "sndFlashlightOn", false, m_eSoundEmptyClick);
	if (WeaponSoundExist(section, "snd_torch_off", true))
		m_sounds.LoadSound(section, "snd_torch_off", "sndFlashlightOff", false, m_eSoundEmptyClick);

	if (WeaponSoundExist(section, "snd_change_zoom", true))
		m_sounds.LoadSound(section, "snd_change_zoom", "sndChangeZoom", m_eSoundEmptyClick);

	// Çâóêè èç êëàññà ïèñòîëåòà
	if (WeaponSoundExist(section, "snd_close", true))
		m_sounds.LoadSound(section, "snd_close", "sndClose", false, m_eSoundClose);

	m_sounds.LoadSound(section, "snd_reload_grip_h", "sndReloadGripH", true, m_eSoundReload);
	m_sounds.LoadSound(section, "snd_reload_grip_v", "sndReloadGripV", true, m_eSoundReload);
	m_sounds.LoadSound(section, "snd_reload", "sndReload", true, m_eSoundReload);

	if (WeaponSoundExist(section, "snd_reload_empty", true))
		m_sounds.LoadSound(section, "snd_reload_empty", "sndReloadEmpty", true, m_eSoundReload);
	if (WeaponSoundExist(section, "snd_reload_empty_grip_h", true))
		m_sounds.LoadSound(section, "snd_reload_empty_grip_h", "sndReloadEmptyGripH", true, m_eSoundReload);
	if (WeaponSoundExist(section, "snd_reload_empty_grip_v", true))
		m_sounds.LoadSound(section, "snd_reload_empty_grip_v", "sndReloadEmptyGripV", true, m_eSoundReload);

	if (WeaponSoundExist(section, "snd_reload_misfire", true))
		m_sounds.LoadSound(section, "snd_reload_misfire", "sndReloadMisfire", true, m_eSoundReload);
	if (WeaponSoundExist(section, "snd_reload_misfire_grip_h", true))
		m_sounds.LoadSound(section, "snd_reload_misfire_grip_h", "sndReloadMisfireGripH", true, m_eSoundReload);
	if (WeaponSoundExist(section, "snd_reload_misfire_grip_v", true))
		m_sounds.LoadSound(section, "snd_reload_misfire_grip_v", "sndReloadMisfireGripV", true, m_eSoundReload);

	if (WeaponSoundExist(section, "snd_reload_misfire_empty", true))
		m_sounds.LoadSound(section, "snd_reload_misfire_empty", "sndReloadMisfireEmpty", true, m_eSoundReload);
	if (WeaponSoundExist(section, "snd_reload_misfire_empty_grip_h", true))
		m_sounds.LoadSound(section, "snd_reload_misfire_empty_grip_h", "sndReloadMisfireEmptyGripH", true, m_eSoundReload);
	if (WeaponSoundExist(section, "snd_reload_misfire_empty_grip_v", true))
		m_sounds.LoadSound(section, "snd_reload_misfire_empty_grip_v", "sndReloadMisfireEmptyGripV", true, m_eSoundReload);

	if (WeaponSoundExist(section, "snd_reload_jammed", true))
		m_sounds.LoadSound(section, "snd_reload_jammed", "sndReloadJammed", true, m_eSoundReload);
	if (WeaponSoundExist(section, "snd_reload_jammed_grip_h", true))
		m_sounds.LoadSound(section, "snd_reload_jammed_grip_h", "sndReloadJammedGripH", true, m_eSoundReload);
	if (WeaponSoundExist(section, "snd_reload_jammed_grip_v", true))
		m_sounds.LoadSound(section, "snd_reload_jammed_grip_v", "sndReloadJammedGripV", true, m_eSoundReload);

	if (WeaponSoundExist(section, "snd_reload_jammed_empty", true))
		m_sounds.LoadSound(section, "snd_reload_jammed_empty", "sndReloadJammedEmpty", true, m_eSoundReload);
	if (WeaponSoundExist(section, "snd_reload_jammed_empty_grip_h", true))
		m_sounds.LoadSound(section, "snd_reload_jammed_empty_grip_h", "sndReloadJammedEmptyGripH", true, m_eSoundReload);
	if (WeaponSoundExist(section, "snd_reload_jammed_empty_grip_v", true))
		m_sounds.LoadSound(section, "snd_reload_jammed_empty_grip_v", "sndReloadJammedEmptyGripV", true, m_eSoundReload);

	if (WeaponSoundExist(section, "snd_pump_gun", true))
		m_sounds.LoadSound(section, "snd_pump_gun", "sndPumpGun", true, m_eSoundReload);

	// Revolvers reload
	m_bIsRevolver = READ_IF_EXISTS(pSettings, r_bool, section, "is_revolver", false);

	if (m_bIsRevolver)
	{
		if (WeaponSoundExist(section, "snd_reload_6", true))
			m_sounds.LoadSound(section, "snd_reload_6", "sndReload6", true, m_eSoundReload);
		if (WeaponSoundExist(section, "snd_reload_5", true))
			m_sounds.LoadSound(section, "snd_reload_5", "sndReload5", true, m_eSoundReload);
		if (WeaponSoundExist(section, "snd_reload_4", true))
			m_sounds.LoadSound(section, "snd_reload_4", "sndReload4", true, m_eSoundReload);
		if (WeaponSoundExist(section, "snd_reload_3", true))
			m_sounds.LoadSound(section, "snd_reload_3", "sndReload3", true, m_eSoundReload);
		if (WeaponSoundExist(section, "snd_reload_2", true))
			m_sounds.LoadSound(section, "snd_reload_2", "sndReload2", true, m_eSoundReload);
		if (WeaponSoundExist(section, "snd_reload_1", true))
			m_sounds.LoadSound(section, "snd_reload_1", "sndReload1", true, m_eSoundReload);
	}
		
	//çâóêè è ïàðòèêëû ãëóøèòåëÿ, åñëèò òàêîé åñòü
	if ( m_eSilencerStatus == ALife::eAddonAttachable || m_eSilencerStatus == ALife::eAddonPermanent )
	{
		if(pSettings->line_exist(section, "silencer_flame_particles"))
			m_sSilencerFlameParticles = pSettings->r_string(section, "silencer_flame_particles");
		if(pSettings->line_exist(section, "silencer_smoke_particles"))
			m_sSilencerSmokeParticles = pSettings->r_string(section, "silencer_smoke_particles");
		
		m_sounds.LoadSound(section,"snd_silncer_shoot", "sndSilencerShot", false, m_eSoundShot);
	}

	if (WeaponSoundExist(section, "snd_aim_up", true))
		m_sounds.LoadSound(section, "snd_aim_up", "sndZoomIn", true, m_eSoundEmptyClick);
	if (WeaponSoundExist(section, "snd_aim_down", true))
		m_sounds.LoadSound(section, "snd_aim_down", "sndZoomOut", true, m_eSoundEmptyClick);
	if (WeaponSoundExist(section, "snd_sprint_start", true))
		m_sounds.LoadSound(section, "snd_sprint_start", "sndSprintStart", true, m_eSoundEmptyClick);
	if (WeaponSoundExist(section, "snd_sprint_end", true))
		m_sounds.LoadSound(section, "snd_sprint_end", "sndSprintEnd", true, m_eSoundEmptyClick);
	if (WeaponSoundExist(section, "snd_sprint_idle", true))
		m_sounds.LoadSound(section, "snd_sprint_idle", "sndSprintIdle", true, m_eSoundEmptyClick);

	m_iBaseDispersionedBulletsCount = READ_IF_EXISTS(pSettings, r_u8, section, "base_dispersioned_bullets_count", 0);
	m_fBaseDispersionedBulletsSpeed = READ_IF_EXISTS(pSettings, r_float, section, "base_dispersioned_bullets_speed", m_fStartBulletSpeed);

	m_iMagClickStartRound = READ_IF_EXISTS(pSettings, r_u32, section, "mag_click_start_round", 0);

	if (pSettings->line_exist(section, "fire_modes"))
	{
		m_bHasDifferentFireModes = true;
		shared_str FireModesList = pSettings->r_string(section, "fire_modes");
		int ModesCount = _GetItemCount(FireModesList.c_str());
		m_aFireModes.clear();
		
		for (int i=0; i<ModesCount; i++)
		{
			string16 sItem;
			_GetItem(FireModesList.c_str(), i, sItem);
			m_aFireModes.push_back	((s8)atoi(sItem));
		}
		
		m_iCurFireMode = ModesCount - 1;
		m_iPrefferedFireMode = READ_IF_EXISTS(pSettings, r_s16,section,"preffered_fire_mode",-1);
	}
	else
	{
		m_bHasDifferentFireModes = false;
	}

	LoadSilencerKoeffs();

	m_bUseFiremodeChangeAnim = READ_IF_EXISTS(pSettings, r_bool, section, "use_firemode_change_anim", false);

	if (pSettings->line_exist(section, "bullet_bones"))
	{
		bHasBulletsToHide = true;
		LPCSTR str = pSettings->r_string(section, "bullet_bones");
		for (int i = 0, count = _GetItemCount(str); i < count; ++i)
		{
			string128 bullet_bone_name;
			_GetItem(str, i, bullet_bone_name);
			bullets_bones.push_back(bullet_bone_name);
			bullet_cnt++;
		}

	}
}

bool CWeaponMagazined::UseScopeTexture()
{
	return bScopeIsHasTexture;
}

void CWeaponMagazined::FireStart		()
{
	if(!IsMisfire())
	{
		if(IsValid()) 
		{
			if(!IsWorking() || AllowFireWhileWorking())
			{
				if (GetState() == eReload)
					return;
				if (GetState() == eShowing)
					return;
				if (GetState() == eHiding)
					return;
				if (GetState() == eMisfire)
					return;
				if (GetState() == eUnMisfire)
					return;
				if (GetState() == eFiremodePrev)
					return;
				if (GetState() == eFiremodeNext)
					return;
				if (GetState() == eLaserSwitch)
					return;
				if (GetState() == eFlashlightSwitch)
					return;

				inherited::FireStart();
				
				if (iAmmoElapsed == 0) 
					OnMagazineEmpty();
				else{
					R_ASSERT(H_Parent());
					SwitchState(eFire);
				}
			}
		}else 
		{
			if(eReload!=GetState()) 
				OnMagazineEmpty();
		}
	}
	else
	{
		//misfire
		CGameObject* object = smart_cast<CGameObject*>(H_Parent());
		if (object)
			object->callback(GameObject::eOnWeaponJammed)(object->lua_game_object(), this->lua_game_object());

		if(smart_cast<CActor*>(this->H_Parent()) && (Level().CurrentViewEntity()==H_Parent()) )
			HUD().GetUI()->AddInfoMessage("gun_jammed");

		OnEmptyClick();
	}
}

void CWeaponMagazined::FireEnd() 
{
	inherited::FireEnd();

	if (psActorFlags2.test(AF_WPN_RELOAD_TYPE))
	{
		if (Actor()->mstate_real & (mcSprint) && !GameConstants::GetReloadIfSprint())
			return;

		if (m_pInventory && !iAmmoElapsed && ParentIsActor() && GetState() != eReload)
			Reload();
	}
}

void CWeaponMagazined::Reload() 
{
	inherited::Reload();
	TryReload();
}

void CWeaponMagazined::EngineMotionMarksUpdate(u32 state, const motion_marks& M)
{
	if (strstr(*M.name, "rotate_safety") == *M.name)
	{
		bool reverse = false;
		
		switch (m_iCurFireMode)
		{
		case 0:
			{
				reverse = false;
			} break;
		case 1:
			{
				if (m_aFireModes.size() == 3)
					reverse = false;
				else
					reverse = true;
			} break;
		case 2:
			{
				reverse = true;
			} break;
		default:
			break;
		}

		RecalculateSafetyRotation(reverse, m_fSafetyRotationSteps[m_iCurFireMode]);
	}
	else if (strstr(*M.name, "device_switch") == *M.name)
	{
		CActor* actor = Actor();

		if (!actor)
		{
			HeadLampSwitch			= false;
			NightVisionSwitch		= false;
			CleanMaskAction			= false;
			LaserSwitchAction		= false;
			FlashlightSwitchAction	= false;
			return;
		}

		if (HeadLampSwitch)
		{
			if (CTorch* pActorTorch = smart_cast<CTorch*>(actor->inventory().ItemFromSlot(TORCH_SLOT)))
				pActorTorch->Switch(!pActorTorch->IsSwitchedOn());

			HeadLampSwitch = false;
		}
		else if (NightVisionSwitch)
		{
			if (actor->GetNightVision())
				actor->SwitchNightVision(!actor->GetNightVisionStatus());

			NightVisionSwitch = false;
		}
		else if (CleanMaskAction)
		{
			actor->SetMaskClear(true);
			CleanMaskAction = false;
		}
		else if (LaserSwitchAction)
		{
			if (!IsLaserOn())
				m_flagsAddOnState |= CSE_ALifeItemWeapon::eWeaponAddonLaserOn;
			else
				m_flagsAddOnState &= ~CSE_ALifeItemWeapon::eWeaponAddonLaserOn;

			LaserSwitchAction = false;
		}
		else if (FlashlightSwitchAction)
		{
			if (!IsFlashlightOn())
				m_flagsAddOnState |= CSE_ALifeItemWeapon::eWeaponAddonFlashlightOn;
			else
				m_flagsAddOnState &= ~CSE_ALifeItemWeapon::eWeaponAddonFlashlightOn;

			FlashlightSwitchAction = false;
		}
	}
	else if ((xr_strcmp(M.name.c_str(), "pistol_reload") == 0))
	{
		if (!GetHUDmode() || bullets_bones.empty())
			return;

		u8 bullets_to_show = iMagazineSize;

		if (!unlimited_ammo())
		{
			u8 available = GetAvailableCartridgesToLoad(true);
			bullets_to_show = (available >= iMagazineSize) ? iMagazineSize : (available + iAmmoElapsed);
		}

		for (size_t i = 0; i < bullets_bones.size(); ++i)
		{
			u16 bone_id = HudItemData()->m_model->LL_BoneID(bullets_bones[i]);

			if (bone_id == BI_NONE)
				continue;

			bool should_show = (i >= (bullets_bones.size() - bullets_to_show));
			HudItemData()->set_bone_visible(bullets_bones[i], should_show);

			string64 spring_bone_name{};
			strconcat(sizeof(spring_bone_name), spring_bone_name, "prujina", std::to_string(i + 1).c_str());

			u16 spring_bone_id = HudItemData()->m_model->LL_BoneID(spring_bone_name);

			if (spring_bone_id != BI_NONE)
			{
				bool spring_visible = !(i >= (bullets_bones.size() - bullets_to_show));
				HudItemData()->set_bone_visible(spring_bone_name, spring_visible);
			}
		}
	}
}

void CWeaponMagazined::OnMotionMark(u32 state, const motion_marks& M)
{
	inherited::OnMotionMark(state, M);
	
	if ((state == eReload && (xr_strcmp(M.name.c_str(), "lmg_reload") == 0)) || ((xr_strcmp(M.name.c_str(), "shotgun_reload") == 0) && ((iAmmoElapsed + 1) == iMagazineSize)))
	{
		u8 ammo_type = m_ammoType;
		int ae = CheckAmmoBeforeReload(ammo_type);

		if (ammo_type == m_ammoType)
		{
			Msg("Ammo elapsed: %d", iAmmoElapsed);
			ae += iAmmoElapsed;
		}

		last_hide_bullet = ae >= bullet_cnt ? bullet_cnt : bullet_cnt - ae - 1;

		Msg("Next reload: count %d with type %d", ae, ammo_type);

		HUD_VisualBulletUpdate();
	}

	EngineMotionMarksUpdate(state, M);
}

bool CWeaponMagazined::TryReload() 
{
	if(m_pInventory) 
	{
		if(IsGameTypeSingle() && ParentIsActor())
		{
			int	AC					= GetSuitableAmmoTotal();
			Actor()->callback(GameObject::eWeaponNoAmmoAvailable)(lua_game_object(), AC);
		}
		if (m_ammoType < m_ammoTypes.size())
			m_pCurrentAmmo = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(*m_ammoTypes[m_ammoType] ));
		else
			m_pCurrentAmmo = NULL;

		/*
		if (ParentIsActor())
		{
			luabind::functor<void> funct;
			if (ai().script_engine().functor("lfo_weapons.on_actor_reloading", funct))
				funct();
		}
		*/

		if (IsMisfire() && !IsGrenadeMode())
		{
			SetPending			(TRUE);
			SwitchState			(eUnMisfire);
			return				true;
		}

		if (GetHUDmode() && !bullets_bones.empty())
		{
			u8 visible_bullets = iAmmoElapsed;

			for (size_t i = 0; i < bullets_bones.size(); ++i)
			{
				u16 bone_id = HudItemData()->m_model->LL_BoneID(bullets_bones[i]);
				if (bone_id == BI_NONE)
					continue;

				bool should_show = (i < visible_bullets);
				HudItemData()->set_bone_visible(bullets_bones[i], should_show);

				string64 spring_bone_name{};
				strconcat(sizeof(spring_bone_name), spring_bone_name, "prujina", std::to_string(i + 1).c_str());

				u16 spring_bone_id = HudItemData()->m_model->LL_BoneID(spring_bone_name);

				if (spring_bone_id != BI_NONE)
				{
					bool spring_visible = !(i < visible_bullets);
					HudItemData()->set_bone_visible(spring_bone_name, spring_visible);
				}
			}
		}

		if (m_pCurrentAmmo || unlimited_ammo())
		{
			SetPending			(TRUE);
			SwitchState			(eReload); 
			return				true;
		}
 
		else for(u32 i = 0; i < m_ammoTypes.size(); ++i) 
		{
			for (u32 i = 0; i < m_ammoTypes.size(); ++i)
			{
				m_pCurrentAmmo = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(*m_ammoTypes[i]));

				if (m_pCurrentAmmo)
				{
					m_set_next_ammoType_on_reload = i;
					SetPending(TRUE);
					SwitchState(eReload);
					return true;
				}
			}
		}

	}
	
	if(GetState()!=eIdle)
		SwitchState(eIdle);

	return false;
}

bool CWeaponMagazined::IsAmmoAvailable()
{
	if (smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(*m_ammoTypes[m_ammoType])))
		return true;
	else
	{
		for (u32 i = 0; i < m_ammoTypes.size(); ++i)
		{
			if (smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(*m_ammoTypes[i])))
				return true;
		}
	}
	return false;
}

void CWeaponMagazined::OnMagazineEmpty() 
{
	if (IsGameTypeSingle() && ParentIsActor())
	{
		int AC = GetSuitableAmmoTotal();
		Actor()->callback(GameObject::eOnWeaponMagazineEmpty)(lua_game_object(), AC);
	}

	if(GetState() == eIdle) 
	{
		OnEmptyClick			();
		return;
	}

	if( GetNextState() != eMagEmpty && GetNextState() != eReload)
	{
		SwitchState(eMagEmpty);
	}

	inherited::OnMagazineEmpty();
}

void CWeaponMagazined::UnloadMagazine(bool spawn_ammo)
{
	last_hide_bullet = -1;
	HUD_VisualBulletUpdate();

	xr_map<LPCSTR, u16> l_ammo;
	
	while(!m_magazine.empty()) 
	{
		CCartridge &l_cartridge = m_magazine.back();
		xr_map<LPCSTR, u16>::iterator l_it;
		for(l_it = l_ammo.begin(); l_ammo.end() != l_it; ++l_it) 
		{
            if(!xr_strcmp(*l_cartridge.m_ammoSect, l_it->first)) 
            { 
				 ++(l_it->second); 
				 break; 
			}
		}

		if(l_it == l_ammo.end()) l_ammo[*l_cartridge.m_ammoSect] = 1;
		m_magazine.pop_back(); 
		--iAmmoElapsed;
	}

	VERIFY((u32)iAmmoElapsed == m_magazine.size());

	if (iAmmoElapsed < 0)
		iAmmoElapsed = 0;

	if (IsGameTypeSingle() && ParentIsActor())
	{
		int AC = GetSuitableAmmoTotal();
		Actor()->callback(GameObject::eOnWeaponMagazineEmpty)(lua_game_object(), AC);
	}

	if (!spawn_ammo)
		return;

	xr_map<LPCSTR, u16>::iterator l_it;
	for(l_it = l_ammo.begin(); l_ammo.end() != l_it; ++l_it) 
	{
		if(m_pInventory)
		{
			CWeaponAmmo *l_pA = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(l_it->first));
			if(l_pA) 
			{
				u16 l_free = l_pA->m_boxSize - l_pA->m_boxCurr;
				l_pA->m_boxCurr = l_pA->m_boxCurr + (l_free < l_it->second ? l_free : l_it->second);
				l_it->second = l_it->second - (l_free < l_it->second ? l_free : l_it->second);
			}
		}
		if(l_it->second && !unlimited_ammo()) SpawnAmmo(l_it->second, l_it->first);
	}

	if (iAmmoElapsed < 0)
		iAmmoElapsed = 0;

	SwitchState(eIdle);
}

int CWeaponMagazined::CheckAmmoBeforeReload(u8& v_ammoType)
{
	if (m_set_next_ammoType_on_reload != u32(-1))
		v_ammoType = m_set_next_ammoType_on_reload;

	Msg("Ammo type in next reload : %d", m_set_next_ammoType_on_reload);

	if (m_ammoTypes.size() <= v_ammoType)
	{
		Msg("Ammo type is wrong : %d", v_ammoType);
		return 0;
	}

	LPCSTR tmp_sect_name = m_ammoTypes[v_ammoType].c_str();

	if (!tmp_sect_name)
	{
		Msg("Sect name is wrong");
		return 0;
	}

	CWeaponAmmo* ammo = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(tmp_sect_name));

	if (!ammo && !m_bLockType)
	{
		for (u8 i = 0; i < u8(m_ammoTypes.size()); ++i)
		{
			//Ð¿Ñ€Ð¾Ð²ÐµÑ€Ð¸Ñ‚ÑŒ Ð¿Ð°Ñ‚Ñ€Ð¾Ð½Ñ‹ Ð²ÑÐµÑ… Ð¿Ð¾Ð´Ñ…Ð¾Ð´ÑÑ‰Ð¸Ñ… Ñ‚Ð¸Ð¿Ð¾Ð²
			ammo = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(m_ammoTypes[i].c_str()));
			if (ammo)
			{
				v_ammoType = i;
				break;
			}
		}
	}

	Msg("Ammo type %d", v_ammoType);

	return GetAmmoCount(v_ammoType);

}

void CWeaponMagazined::ReloadMagazine() 
{
	m_dwAmmoCurrentCalcFrame = 0;	

	//óñòðàíèòü îñå÷êó ïðè ïåðåçàðÿäêå
	if (IsMisfire() && !IsGrenadeMode())
		bMisfire = false;
	
	if (!m_bLockType)
	{
		m_ammoName	= NULL;
		m_pCurrentAmmo		= NULL;
	}
	
	if (!m_pInventory) return;

	if(m_set_next_ammoType_on_reload != u32(-1)){		
		m_ammoType						= m_set_next_ammoType_on_reload;
		m_set_next_ammoType_on_reload	= u32(-1);
	}
	
	if(!unlimited_ammo()) 
	{
		//ïîïûòàòüñÿ íàéòè â èíâåíòàðå ïàòðîíû òåêóùåãî òèïà 
		m_pCurrentAmmo = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(*m_ammoTypes[m_ammoType]));
		
		if(!m_pCurrentAmmo && !m_bLockType) 
		{
			for(u32 i = 0; i < m_ammoTypes.size(); ++i) 
			{
				//ïðîâåðèòü ïàòðîíû âñåõ ïîäõîäÿùèõ òèïîâ
				m_pCurrentAmmo = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(*m_ammoTypes[i]));
				if(m_pCurrentAmmo) 
				{ 
					m_ammoType = i; 
					break; 
				}
			}
		}
	}
	else
		m_ammoType = m_ammoType;


	//íåò ïàòðîíîâ äëÿ ïåðåçàðÿäêè
	if(!m_pCurrentAmmo && !unlimited_ammo() ) return;

	//Ìîäåðíèçèðóåì ïðîâåðêó íà ñîîòâåñòâèå ïàòðîíîâ, áóäåì ïðîâåðÿòü òàê æå ïîñëåäíèé ïàòðîí
	//ðàçðÿäèòü ìàãàçèí, åñëè çàãðóæàåì ïàòðîíàìè äðóãîãî òèïà
	if (!m_bLockType && !m_magazine.empty() && (!m_pCurrentAmmo || xr_strcmp(m_pCurrentAmmo->cNameSect(), *m_magazine.front().m_ammoSect)))
	{
		UnloadMagazine();
	}

	VERIFY((u32)iAmmoElapsed == m_magazine.size());

	if (m_DefaultCartridge.m_LocalAmmoType != m_ammoType)
		m_DefaultCartridge.Load(*m_ammoTypes[m_ammoType], u8(m_ammoType));
	CCartridge l_cartridge = m_DefaultCartridge;
	while(iAmmoElapsed < iMagazineSize)
	{
		if (!unlimited_ammo())
		{
			if (!m_pCurrentAmmo->Get(l_cartridge)) break;
		}
		++iAmmoElapsed;
		l_cartridge.m_LocalAmmoType = u8(m_ammoType);
		m_magazine.push_back(l_cartridge);
	}
	m_ammoName = (m_pCurrentAmmo) ? m_pCurrentAmmo->m_nameShort : NULL;

	VERIFY((u32)iAmmoElapsed == m_magazine.size());

	//âûêèíóòü êîðîáêó ïàòðîíîâ, åñëè îíà ïóñòàÿ
	if(m_pCurrentAmmo && !m_pCurrentAmmo->m_boxCurr && OnServer()) 
		m_pCurrentAmmo->SetDropManual(TRUE);

	if(iMagazineSize > iAmmoElapsed)
	{ 
		m_bLockType = true; 
		ReloadMagazine(); 
		m_bLockType = false; 
	}

	VERIFY((u32)iAmmoElapsed == m_magazine.size());
}

void CWeaponMagazined::DeviceSwitch()
{
	inherited::DeviceSwitch();
	SetPending(TRUE);
	SwitchState(eDeviceSwitch);
}

void CWeaponMagazined::OnStateSwitch	(u32 S)
{
	HUD_VisualBulletUpdate();

	inherited::OnStateSwitch(S);
	CInventoryOwner* owner = smart_cast<CInventoryOwner*>(this->H_Parent());
	switch (S)
	{
	case eFiremodeNext:
	{
		PlaySound("sndFireModes", get_LastFP());
		switch2_ChangeFireMode();
	}break;
	case eFiremodePrev:
	{
		PlaySound("sndFireModes", get_LastFP());
		switch2_ChangeFireMode();
	}break;
	case eLaserSwitch:
	{
		if (IsLaserOn())
			PlaySound("sndLaserOn", get_LastFP());
		else
			PlaySound("sndLaserOff", get_LastFP());

		switch2_LaserSwitch();
	}break;
	case eFlashlightSwitch:
	{
		if (IsFlashlightOn())
			PlaySound("sndFlashlightOn", get_LastFP());
		else
			PlaySound("sndFlashlightOff", get_LastFP());

		switch2_FlashlightSwitch();
	}break;
	case eIdle:
		switch2_Idle	();
		break;
	case eFire:
		switch2_Fire	();
		break;
	case eUnMisfire:
		if (owner)
			m_sounds_enabled = owner->CanPlayShHdRldSounds();
		switch2_Unmis();
		break;
	case eMisfire:
		if(smart_cast<CActor*>(this->H_Parent()) && (Level().CurrentViewEntity()==H_Parent()) )
			HUD().GetUI()->AddInfoMessage("gun_jammed");
		break;
	case eMagEmpty:
		switch2_Empty	();
		break;
	case eReload:
		if(owner)
			m_sounds_enabled = owner->CanPlayShHdRldSounds();
		switch2_Reload	();
		break;
	case eShowing:
		if(owner)
			m_sounds_enabled = owner->CanPlayShHdRldSounds();
		switch2_Showing	();
		break;
	case eHiding:
		if(owner)
			m_sounds_enabled = owner->CanPlayShHdRldSounds();
		switch2_Hiding	();
		break;
	case eHidden:
		switch2_Hidden	();
		break;
	case eDeviceSwitch:
		PlayAnimDeviceSwitch();
		SetPending(TRUE);
		break;
	}
}

void CWeaponMagazined::DeviceUpdate()
{
	if (ParentIsActor())
	{
		CActor* actor = Actor();
		if (HeadLampSwitch)
		{
			auto pActorTorch = smart_cast<CTorch*>(actor->inventory().ItemFromSlot(TORCH_SLOT));
			pActorTorch->Switch(!pActorTorch->IsSwitchedOn());
			HeadLampSwitch = false;
		}
		else if (NightVisionSwitch)
		{
			if (actor->GetNightVision())
			{
				actor->SwitchNightVision(!actor->GetNightVisionStatus());
				NightVisionSwitch = false;
			}
		}
		else if (CleanMaskAction)
		{
			actor->SetMaskClear(true);
			CleanMaskAction = false;
		}
		else if (LaserSwitchAction)
		{
			if (!IsLaserOn())
				m_flagsAddOnState |= CSE_ALifeItemWeapon::eWeaponAddonLaserOn;
			else
				m_flagsAddOnState &= ~CSE_ALifeItemWeapon::eWeaponAddonLaserOn;

			LaserSwitchAction = false;
		}
		else if (FlashlightSwitchAction)
		{
			if (!IsFlashlightOn())
				m_flagsAddOnState |= CSE_ALifeItemWeapon::eWeaponAddonFlashlightOn;
			else
				m_flagsAddOnState &= ~CSE_ALifeItemWeapon::eWeaponAddonFlashlightOn;

			FlashlightSwitchAction = false;
		}
	}
}

void CWeaponMagazined::UpdateCL()
{
	inherited::UpdateCL	();
	float dt = Device.fTimeDelta;

	//êîãäà ïðîèñõîäèò àïäåéò ñîñòîÿíèÿ îðóæèÿ
	//íè÷åãî äðóãîãî íå äåëàòü
	if(GetNextState() == GetState())
	{
		switch (GetState())
		{
		case eShowing:
		case eHiding:
		case eReload:
		case eSprintStart:
		case eSprintEnd:
		case eDeviceSwitch:
		case eIdle:
			{
				fShotTimeCounter	-=	dt;
				clamp				(fShotTimeCounter, 0.0f, flt_max);

				luabind::functor<void> funct;
				if (ai().script_engine().functor("lfo_weapons.on_actor_not_shooting", funct))
					funct();

			}break;
		case eFire:			
			{
				state_Fire		(dt);
			}break;
		case eMisfire:		state_Misfire	(dt);	break;
		case eMagEmpty:		state_MagEmpty	(dt);	break;
		case eHidden:		break;
		}
	}

	UpdateSounds		();
	
	if (IsActionInProcessNow())
		TimeLockAnimation();
}

void CWeaponMagazined::UpdateSounds	()
{
	if (Device.dwFrame == dwUpdateSounds_Frame)  
		return;
	
	dwUpdateSounds_Frame = Device.dwFrame;

	Fvector P						= get_LastFP();
	m_sounds.SetPosition("sndShow", P);
	m_sounds.SetPosition("sndHide", P);
	if (psWpnAnimsFlag.test(ANM_HIDE_EMPTY) && WeaponSoundExist(m_section_id.c_str(), "snd_close"))
		m_sounds.SetPosition("sndClose", P);
	if (WeaponSoundExist(m_section_id.c_str(), "snd_changefiremode"))
		m_sounds.SetPosition("sndFireModes", P);
	if (WeaponSoundExist(m_section_id.c_str(), "snd_laser_on"))
		m_sounds.SetPosition("sndLaserOn", P);
	if (WeaponSoundExist(m_section_id.c_str(), "snd_laser_off"))
		m_sounds.SetPosition("sndLaserOff", P);
	if (WeaponSoundExist(m_section_id.c_str(), "snd_torch_on"))
		m_sounds.SetPosition("sndFlashlightOn", P);
	if (WeaponSoundExist(m_section_id.c_str(), "snd_torch_off"))
		m_sounds.SetPosition("sndFlashlightOff", P);
	if (WeaponSoundExist(m_section_id.c_str(), "snd_change_zoom"))
		m_sounds.SetPosition("sndChangeZoom", P);
	if (WeaponSoundExist(m_section_id.c_str(), "snd_aim_up"))
		m_sounds.SetPosition("sndZoomIn", P);
	if (WeaponSoundExist(m_section_id.c_str(), "snd_aim_down"))
		m_sounds.SetPosition("sndZoomOut", P);
	if (WeaponSoundExist(m_section_id.c_str(), "snd_sprint_start"))
		m_sounds.SetPosition("sndSprintStart", P);
	if (WeaponSoundExist(m_section_id.c_str(), "snd_sprint_end"))
		m_sounds.SetPosition("sndSprintEnd", P);
	if (WeaponSoundExist(m_section_id.c_str(), "snd_sprint_idle"))
		m_sounds.SetPosition("sndSprintIdle", P);
	if (m_sounds.FindSoundItem("sndAimUp", false))
		m_sounds.SetPosition("sndAimUp", P);
	if (m_sounds.FindSoundItem("sndAimDown", false))
		m_sounds.SetPosition("sndAimDown", P);
	// LFO INSPECT WEAPONS
	if (WeaponSoundExist(m_section_id.c_str(), "sndInspectWeapon"))
		m_sounds.SetPosition("sndInspectWeapon", P);
	if (WeaponSoundExist(m_section_id.c_str(), "sndInspectWeaponEmpty"))
		m_sounds.SetPosition("sndInspectWeaponEmpty", P);
	if (WeaponSoundExist(m_section_id.c_str(), "sndInspectWeaponMisfire"))
		m_sounds.SetPosition("sndInspectWeaponMisfire", P);

	if (WeaponSoundExist(m_section_id.c_str(), "sndInspectWeaponGl"))
		m_sounds.SetPosition("sndInspectWeaponGl", P);
	if (WeaponSoundExist(m_section_id.c_str(), "sndInspectWeaponEmptyGl"))
		m_sounds.SetPosition("sndInspectWeaponEmptyGl", P);
	if (WeaponSoundExist(m_section_id.c_str(), "sndInspectWeaponMisfireGl"))
		m_sounds.SetPosition("sndInspectWeaponMisfireGl", P);

	if (WeaponSoundExist(m_section_id.c_str(), "sndInspectWeaponGlUsed"))
		m_sounds.SetPosition("sndInspectWeaponGlUsed", P);
	if (WeaponSoundExist(m_section_id.c_str(), "sndInspectWeaponEmptyGlUsed"))
		m_sounds.SetPosition("sndInspectWeaponEmptyGlUsed", P);
	if (WeaponSoundExist(m_section_id.c_str(), "sndInspectWeaponMisfireGlUsed"))
		m_sounds.SetPosition("sndInspectWeaponMisfireGlUsed", P);

//. nah	m_sounds.SetPosition("sndShot", P);
	m_sounds.SetPosition("sndReload", P);
//. nah	m_sounds.SetPosition("sndEmptyClick", P);
}

void CWeaponMagazined::state_Fire(float dt)
{
	if(iAmmoElapsed > 0)
	{
		VERIFY(fOneShotTime>0.f);

		Fvector					p1, d; 
		p1.set(get_LastFP());
		d.set(get_LastFD());

		if (!H_Parent()) return;
		if (smart_cast<CMPPlayersBag*>(H_Parent()) != NULL)
		{
			Msg("! WARNING: state_Fire of object [%d][%s] while parent is CMPPlayerBag...", ID(), cNameSect().c_str());
			return;
		}

		CInventoryOwner* io		= smart_cast<CInventoryOwner*>(H_Parent());
		if(NULL == io->inventory().ActiveItem())
		{
				Log("current_state", GetState() );
				Log("next_state", GetNextState());
				Log("item_sect", cNameSect().c_str());
				Log("H_Parent", H_Parent()->cNameSect().c_str());
				StopShooting();
				return;
				//Alundaio: This is not supposed to happen but it does. GSC was aware but why no return here? Known to cause crash on game load if npc immediatly enters combat.
		}

		CEntity* E = smart_cast<CEntity*>(H_Parent());
		E->g_fireParams	(this, p1,d);

		if( !E->g_stateFire() )
			StopShooting();

		if (m_iShotNum == 0)
		{
			m_vStartPos = p1;
			m_vStartDir = d;
		}
		
		VERIFY(!m_magazine.empty());

		while (!m_magazine.empty() && fShotTimeCounter < 0 && (IsWorking() || m_bFireSingleShot) && (m_iQueueSize < 0 || m_iShotNum < m_iQueueSize))
		{
			m_bFireSingleShot		= false;

			//Alundaio: Use fModeShotTime instead of fOneShotTime if current fire mode is 2-shot burst
			//Alundaio: Cycle down RPM after two shots; used for Abakan/AN-94
			bool b_mod_shot_time = (GetCurrentFireMode() == 3 || GetCurrentFireMode() == 2 || (bCycleDown == true && m_iShotNum < 1));

			fShotTimeCounter		+=	b_mod_shot_time ? fModeShotTime : fOneShotTime;
			fShotTimeCounter		+= ((b_mod_shot_time ? fModeShotTime : fOneShotTime) * (m_fOverheatingSubRpm / 100.f)) * m_fWeaponOverheating;

			++m_iShotNum;
			
			if (!IsGrenadeMode())
				CheckForMisfire();

			OnShot					();

			if (m_iShotNum>m_iBaseDispersionedBulletsCount)
				FireTrace		(p1,d);
			else
				FireTrace		(m_vStartPos, m_vStartDir);
		}
	
		if(m_iShotNum == m_iQueueSize)
			m_bStopedAfterQueueFired = true;

		UpdateSounds			();
	}

	if(fShotTimeCounter<0 || (!iAmmoElapsed && !m_bLastShotRPM))
	{
/*
		if(bDebug && H_Parent() && (H_Parent()->ID() != Actor()->ID()))
		{
			Msg("stop shooting w=[%s] magsize=[%d] sshot=[%s] qsize=[%d] shotnum=[%d]",
					IsWorking()?"true":"false", 
					m_magazine.size(),
					m_bFireSingleShot?"true":"false",
					m_iQueueSize,
					m_iShotNum);
		}
*/
		if(iAmmoElapsed == 0)
			OnMagazineEmpty();

		StopShooting();
	}
	else
	{
		fShotTimeCounter			-=	dt;
	}

	//if (m_fFactor > 0)
		//StopShooting();
}

void CWeaponMagazined::state_Misfire	(float dt)
{
	if (!m_bIsShotgun)
	{
		OnEmptyClick();
		SwitchState(eIdle);
	}
	
	bMisfire				= true;

	UpdateSounds			();
}

void CWeaponMagazined::state_MagEmpty	(float dt)
{
}

void CWeaponMagazined::SetDefaults	()
{
	CWeapon::SetDefaults		();
}

void CWeaponMagazined::PlaySoundShot()
{
	if (ParentIsActor())
	{
		if (bMisfire)
		{
			string128 sndNameMisfire;
			strconcat(sizeof(sndNameMisfire), sndNameMisfire, m_sSndShotCurrent.c_str(), "MisfireActor");
			if (m_sounds.FindSoundItem(sndNameMisfire, false))
			{
				m_sounds.PlaySound(sndNameMisfire, get_LastFP(), H_Root(), !!GetHUDmode(), false, (u8)-1);
				return;
			}
		}

		string128 sndName;
		strconcat(sizeof(sndName), sndName, m_sSndShotCurrent.c_str(), "Actor");
		if (m_sounds.FindSoundItem(sndName, false))
		{
			m_sounds.PlaySound(sndName, get_LastFP(), H_Root(), !!GetHUDmode(), false, (u8)-1);
			return;
		}
	}

	if (bMisfire)
	{
		string128 sndNameMisfire;
		strconcat(sizeof(sndNameMisfire), sndNameMisfire, m_sSndShotCurrent.c_str(), "Misfire");
		if (m_sounds.FindSoundItem(sndNameMisfire, false))
		{
			m_sounds.PlaySound(sndNameMisfire, get_LastFP(), H_Root(), !!GetHUDmode(), false, (u8)-1);
			return;
		}
	}

	m_sounds.PlaySound(m_sSndShotCurrent.c_str(), get_LastFP(), H_Root(), !!GetHUDmode(), false, (u8)-1);
}


void CWeaponMagazined::OnShot()
{
	if (psActorFlags.test(AF_WPN_ZOOM_OUT_SHOOT))
	{
		if (IsOutScopeAfterShot())
			OnZoomOut();
	}
	// Åñëè àêòîð áåæèò - îñòàíàâëèâàåì åãî
	if (ParentIsActor() && GameConstants::GetStopActorIfShoot())
		Actor()->set_state_wishful(Actor()->get_state_wishful() & (~mcSprint));

	// Camera	
	AddShotEffector				();

	// Animation
	if (IsGripAttached())
	{
		PlayAnimShootHorGrip();
	}
	else if (IsGripvAttached())
	{
		PlayAnimShootVerGrip();
	}
	else
	{
		PlayAnimShoot();
	}

	HUD_VisualBulletUpdate();

	// Shell Drop
	Fvector vel; 
	PHGetLinearVell				(vel);
	OnShellDrop					(get_LastSP(), vel);
	
	// Îãîíü èç ñòâîëà
	StartFlameParticles			();

	//äûì èç ñòâîëà
	ForceUpdateFireParticles	();
	StartSmokeParticles			(get_LastFP(), vel);

	// Ïðîèãðàåì çâóê ïîìïû îòäåëüíî, åñëè íå áóäåò ðàáîòàòü òî áóäåì äóìàòü ÷òî äåëàòü è êàê áûòü
	if (m_sounds.FindSoundItem("sndPumpGun", false))
		PlaySound("sndPumpGun", get_LastFP());

	CGameObject* object = smart_cast<CGameObject*>(H_Parent());
	if (object)
		object->callback(GameObject::eOnWeaponFired)(object->lua_game_object(), this->lua_game_object(), iAmmoElapsed);

	// Ýôôåêò ñäâèãà (îòäà÷à)
	{
		AddHUDShootingEffect();

		// Dance Maniac: Äîïîëíèòåëüíûé ýôôåêòîð ñòðåëüáû
		if (m_b_advanced_shoot_effectors && IsGameTypeSingle() && ParentIsActor())
		{
			CEffectorCam* effector = Actor()->Cameras().GetCamEffector((ECamEffectorType)eCEWeaponAction2);

			string128 effector_sect{};
			xr_sprintf(effector_sect, "%s_shoot_effector", m_section_id.c_str());

			if (pSettings->section_exist(effector_sect))
			{
				float effector_intensity		= (READ_IF_EXISTS(pSettings, r_float, effector_sect, "shoot_effector_factor", 1.0f) * m_b_advanced_se_factor);
				float effector_intensity_crouch	= (READ_IF_EXISTS(pSettings, r_float, effector_sect, "shoot_effector_factor_crouch", 1.0f) * m_b_advanced_se_factor);
				float effector_intensity_aim	= (READ_IF_EXISTS(pSettings, r_float, effector_sect, "shoot_effector_factor_aim", 1.0f) * m_b_advanced_se_factor);

				float disp = ((GetFireDispersion(m_fCurrentCartirdgeDisp, IsZoomed()) * 150.f) * (IsZoomed() ? effector_intensity_aim : Actor()->is_actor_crouch() ? effector_intensity_crouch : effector_intensity)) * m_b_advanced_se_factor;

				if (!effector)
					AddEffector(Actor(), eCEWeaponAction2, effector_sect, disp);
			}
		}
	}

	bool bIndoor = false;

	if (m_bIndoorSoundsEnabled && g_pGamePersistent)
		bIndoor = g_pGamePersistent->IsActorInHideout();

	if (ParentIsActor())
	{
		luabind::functor<void> funct;
		if (ai().script_engine().functor("lfo_weapons.on_actor_shooting", funct))
			funct();

		float threshold = 0.0f;
		float volume = 0.0f;

		if (IsZoomed())
		{
			if (auto mag_shot_snd = m_sounds.FindSoundItem("sndMagShot", false))
			{
				if (m_iMagClickStartRound)
					threshold = (float)m_iMagClickStartRound;
				else
					threshold = iMagazineSize * 0.30f;

				if (iAmmoElapsed <= threshold && threshold > 0)
				{
					float threshold = iMagazineSize * 0.30f;
					float volume = 0.0f;

					if (iAmmoElapsed <= threshold && threshold > 0)
					{
						volume = 2.0f * (1.0f - (iAmmoElapsed / threshold));
						clamp(volume, 0.0f, 2.0f);

						if (mag_shot_snd->m_activeSnd && mag_shot_snd->m_activeSnd->volume > volume)
							mag_shot_snd->m_activeSnd->volume = volume;

						m_sounds.PlaySound("sndMagShot", get_LastFP(), H_Root(), !!GetHUDmode(), false, (u8)-1);
						mag_shot_snd->m_activeSnd->volume = volume;
					}
				}
			}
		}
		else
		{
			if (auto mag_shot_snd = m_sounds.FindSoundItem("sndMagShotHip", false))
			{
				if (m_iMagClickStartRound)
					threshold = (float)m_iMagClickStartRound;
				else
					threshold = iMagazineSize * 0.30f;

				if (iAmmoElapsed <= threshold && threshold > 0)
				{
					volume = 2.0f * (1.0f - (iAmmoElapsed / threshold));
					clamp(volume, 0.0f, 2.0f);

					if (mag_shot_snd->m_activeSnd && mag_shot_snd->m_activeSnd->volume > volume)
						mag_shot_snd->m_activeSnd->volume = volume;

					m_sounds.PlaySound("sndMagShotHip", get_LastFP(), H_Root(), !!GetHUDmode(), false, (u8)-1);
					mag_shot_snd->m_activeSnd->volume = volume;
				}
			}
		}

		string128 sndName;

		if (IsZoomed())
		{
			strconcat(sizeof(sndName), sndName, m_sSndShotCurrent.c_str(), "Actor", (iAmmoElapsed == 1) ? "Last" : "", bIndoor ? "Indoor" : "");

		//	Msg("AIM SHOOT");
		}
		else
		{
			strconcat(sizeof(sndName), sndName, m_sSndShotCurrent.c_str(), "Actorh", (iAmmoElapsed == 1) ? "Lasth" : "", bIndoor ? "Indoorh" : "");
		//	Msg("NO AIM SHOOT");
		}

		if (m_sounds.FindSoundItem(sndName, false))
		{
			m_sounds.PlaySound(sndName, get_LastFP(), H_Root(), !!GetHUDmode(), false, (u8)-1);
			return;
		}

		if (char* indoorPos = strstr(sndName, "Indoor"))
		{
			string128 noIndoor{};
			strncpy(noIndoor, sndName, indoorPos - sndName);
			strcat(noIndoor, indoorPos + strlen("Indoor"));

			if (m_sounds.FindSoundItem(noIndoor, false))
			{
				m_sounds.PlaySound(noIndoor, get_LastFP(), H_Root(), !!GetHUDmode(), false, (u8)-1);
				return;
			}
		}

		if (char* lastPos = strstr(sndName, "Last"))
		{
			string128 noLast{};
			strncpy(noLast, sndName, lastPos - sndName);
			strcat(noLast, lastPos + strlen("Last"));

			if (m_sounds.FindSoundItem(noLast, false))
			{
				m_sounds.PlaySound(noLast, get_LastFP(), H_Root(), !!GetHUDmode(), false, (u8)-1);
				return;
			}
		}

		if (char* indoorPosHip = strstr(sndName, "Indoorh"))
		{
			string128 noIndoor{};
			strncpy(noIndoor, sndName, indoorPosHip - sndName);
			strcat(noIndoor, indoorPosHip + strlen("Indoorh"));

			if (m_sounds.FindSoundItem(noIndoor, false))
			{
				m_sounds.PlaySound(noIndoor, get_LastFP(), H_Root(), !!GetHUDmode(), false, (u8)-1);
				return;
			}
		}

		if (char* lastPosHip = strstr(sndName, "Lasth"))
		{
			string128 noLast{};
			strncpy(noLast, sndName, lastPosHip - sndName);
			strcat(noLast, lastPosHip + strlen("Lasth"));

			if (m_sounds.FindSoundItem(noLast, false))
			{
				m_sounds.PlaySound(noLast, get_LastFP(), H_Root(), !!GetHUDmode(), false, (u8)-1);
				return;
			}
		}

	}

	string128 sndName;
	strconcat(sizeof(sndName), sndName, m_sSndShotCurrent.c_str(), (iAmmoElapsed == 1) ? "Last" : "", bIndoor ? "Indoor" : "");

	if (m_sounds.FindSoundItem(sndName, false))
	{
		m_sounds.PlaySound(sndName, get_LastFP(), H_Root(), !!GetHUDmode(), false, (u8)-1);
		return;
	}

	if (char* indoorPos = strstr(sndName, "Indoor"))
	{
		string128 noIndoor{};
		strncpy(noIndoor, sndName, indoorPos - sndName);
		strcat(noIndoor, indoorPos + strlen("Indoor"));

		if (m_sounds.FindSoundItem(noIndoor, false))
		{
			m_sounds.PlaySound(noIndoor, get_LastFP(), H_Root(), !!GetHUDmode(), false, (u8)-1);
			return;
		}
	}

	if (char* lastPos = strstr(sndName, "Last"))
	{
		string128 noLast{};
		strncpy(noLast, sndName, lastPos - sndName);
		strcat(noLast, lastPos + strlen("Last"));

		if (m_sounds.FindSoundItem(noLast, false))
		{
			m_sounds.PlaySound(noLast, get_LastFP(), H_Root(), !!GetHUDmode(), false, (u8)-1);
			return;
		}
	}

	m_sounds.PlaySound(m_sSndShotCurrent.c_str(), get_LastFP(), H_Root(), !!GetHUDmode(), false, (u8)-1);

	// Ýõî âûñòðåëà
	if (IsSilencerAttached() == false)
	{
		if (bIndoor && m_sounds.FindSoundItem("sndReflect", false))
		{
			if (IsHudModeNow())
			{
				HUD_SOUND_ITEM::SetHudSndGlobalVolumeFactor(WEAPON_SND_REFLECTION_HUD_FACTOR);
			}
			PlaySound("sndReflect", get_LastFP());
			HUD_SOUND_ITEM::SetHudSndGlobalVolumeFactor(1.0f);
		}
	}
}


void CWeaponMagazined::OnEmptyClick	()
{
	if (PlayAnimFakeShoot())
	{
		PlaySound("sndEmptyClick", get_LastFP());
		return;
	}
	else
	{
		if (m_BlendFakeShootCam.name.size())
		{
			if (ParentIsActor())
				g_player_hud->PlayBlendAnm(m_BlendFakeShootCam.name.c_str(), 2, m_BlendFakeShootCam.speed, m_BlendFakeShootCam.power, false, false);
		}
	}

	PlaySound("sndEmptyClick", get_LastFP());
}

void CWeaponMagazined::OnAnimationEnd(u32 state) 
{
	switch(state) 
	{
		case eReload:
		{
		CheckMagazine(); // Îñíîâàíî íà ìåõàíèçìå èç Lost Alpha: New Project
						 // Àâòîðû: rafa & Kondr48

			CCartridge FirstBulletInGun;

			bool bNeedputBullet = iAmmoElapsed > 0;

			if (!m_bIsRevolver && m_bNeedBulletInGun && bNeedputBullet)
			{
				FirstBulletInGun = m_magazine.back();
				m_magazine.pop_back();
				iAmmoElapsed--;
			}

			ReloadMagazine();

			if (!m_bIsRevolver && m_bNeedBulletInGun && bNeedputBullet)
			{
				m_magazine.push_back(FirstBulletInGun);
				iAmmoElapsed++;
			}

			if (ParentIsActor())
			{
				luabind::functor<void> funct;
				if (ai().script_engine().functor("lfo_weapons.on_actor_reloading", funct))
					funct();
			}

			SwitchState(eIdle);

		}break;// End of reload animation
		case eHiding:
		{
			SwitchState(eHidden);
			ResetShootingEffect();
			break;	// End of Hide
		}
		case eShowing:	SwitchState(eIdle);		break;	// End of Show
		case eIdle:
		{
			switch2_Idle();
		} break;  // Keep showing idle
		case eMagEmpty:
		{
			if (ParentIsActor())
				switch2_Idle();
		} break;  // Keep showing idle
		case eUnMisfire:
		{
			bMisfire = false;
			
			if (!m_bIsRevolver && m_bMisfireBulletRemove && iAmmoElapsed)
			{
				m_magazine.pop_back();
				iAmmoElapsed--;
			}

			SwitchState(eIdle);
		}break; // End of UnMisfire animation
		case eFiremodePrev:
		{
			SwitchState(eIdle);
			break;
		}
		case eFiremodeNext:
		{
			SwitchState(eIdle);
			break;
		}
		case eLaserSwitch:
		{
			SwitchState(eIdle);
			break;
		}
		case eFlashlightSwitch:
		{
			SwitchState(eIdle);
			break;
		}
		case eDeviceSwitch:
		{
			SwitchState(eIdle);
			break;
		}
	}
	inherited::OnAnimationEnd(state);
}

void CWeaponMagazined::switch2_Idle	()
{
	m_iShotNum = 0;
	if(m_fOldBulletSpeed != 0.f)
		SetBulletSpeed(m_fOldBulletSpeed);

	if (IsPending())
		SetPending		(FALSE);

	PlayAnimIdle		();
}

void CWeaponMagazined::switch2_ChangeFireMode()
{
	if (GetState() != eFiremodeNext && GetState() != eFiremodePrev)
		return;

	FireEnd();

	if (IsGripAttached())
	{
		PlayAnimFireModeGrip();
	}
	else if (IsGripvAttached())
	{
		PlayAnimFireModeGripV();
	}
	else
	{
		PlayAnimFireMode();
	}

	SetPending(TRUE);
}

void CWeaponMagazined::PlayAnimFireMode()
{
	auto det = smart_cast<CCustomDetector*>(g_actor->inventory().ItemFromSlot(DETECTOR_SLOT));

	if (auto det = smart_cast<CCustomDetector*>(g_actor->inventory().ItemFromSlot(DETECTOR_SLOT)); g_actor->IsDetectorActive())
		det->PlayDetectorAnimation(true, eDetAction, "anm_firemode");

	string_path guns_firemode_anm{};

	// Msg("ANIMS CHANGE FIRE MODE WITHOUD GRIP ATTACHED");

	if (isHUDAnimationExist("anm_changefiremode"))
	{
		strconcat(sizeof(guns_firemode_anm), guns_firemode_anm, "anm_changefiremode", (IsMisfire() ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : ""));

		PlayHUDMotionIfExists({ guns_firemode_anm, "anm_changefiremode" }, true, GetState());
		return;
	}

	strconcat(sizeof(guns_firemode_anm), guns_firemode_anm, "anm_changefiremode_from_", (m_iCurFireMode == 0) ? "a_to_1" : (m_iCurFireMode == 1) ? (m_aFireModes.size() == 3) ? "1_to_2" : "1_to_a" : (m_iCurFireMode == 2) ? "2_to_a" : "a_to_1");

	string64 guns_aim_anm_full;
	strconcat(sizeof(guns_aim_anm_full), guns_aim_anm_full, guns_firemode_anm, (IsMisfire() ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : ""));

	if (isHUDAnimationExist(guns_aim_anm_full))
	{
		PlayHUDMotionNew(guns_aim_anm_full, true, GetState());
		return;
	}
	else if (guns_aim_anm_full && strstr(guns_aim_anm_full, "_jammed"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_aim_anm_full);
		new_guns_aim_anm[strlen(guns_aim_anm_full) - strlen("_jammed")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
	}
	else if (guns_aim_anm_full && strstr(guns_aim_anm_full, "_empty"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_aim_anm_full);
		new_guns_aim_anm[strlen(guns_aim_anm_full) - strlen("_empty")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
	}
}

void CWeaponMagazined::PlayAnimFireModeGrip()
{
	auto det = smart_cast<CCustomDetector*>(g_actor->inventory().ItemFromSlot(DETECTOR_SLOT));

	if (auto det = smart_cast<CCustomDetector*>(g_actor->inventory().ItemFromSlot(DETECTOR_SLOT)); g_actor->IsDetectorActive())
		det->PlayDetectorAnimation(true, eDetAction, "anm_firemode");

	string_path guns_firemode_anm{};

	//Msg("ANIMS CHANGE FIRE MODE WITH GRIP ATTACHED");

	if (isHUDAnimationExist("anm_changefiremode_grip"))
	{
		strconcat(sizeof(guns_firemode_anm), guns_firemode_anm, "anm_changefiremode_grip", (IsMisfire() ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : ""));

		PlayHUDMotionIfExists({ guns_firemode_anm, "anm_changefiremode_grip" }, true, GetState());
		return;
	}

	strconcat(sizeof(guns_firemode_anm), guns_firemode_anm, "anm_changefiremode_grip_from_", (m_iCurFireMode == 0) ? "a_to_1" : (m_iCurFireMode == 1) ? (m_aFireModes.size() == 3) ? "1_to_2" : "1_to_a" : (m_iCurFireMode == 2) ? "2_to_a" : "a_to_1");

	string64 guns_aim_anm_full;
	strconcat(sizeof(guns_aim_anm_full), guns_aim_anm_full, guns_firemode_anm, (IsMisfire() ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : ""));

	if (isHUDAnimationExist(guns_aim_anm_full))
	{
		PlayHUDMotionNew(guns_aim_anm_full, true, GetState());
		return;
	}
	else if (guns_aim_anm_full && strstr(guns_aim_anm_full, "_jammed"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_aim_anm_full);
		new_guns_aim_anm[strlen(guns_aim_anm_full) - strlen("_jammed")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
	}
	else if (guns_aim_anm_full && strstr(guns_aim_anm_full, "_empty"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_aim_anm_full);
		new_guns_aim_anm[strlen(guns_aim_anm_full) - strlen("_empty")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
	}
}

void CWeaponMagazined::PlayAnimFireModeGripV()
{
	auto det = smart_cast<CCustomDetector*>(g_actor->inventory().ItemFromSlot(DETECTOR_SLOT));

	if (auto det = smart_cast<CCustomDetector*>(g_actor->inventory().ItemFromSlot(DETECTOR_SLOT)); g_actor->IsDetectorActive())
		det->PlayDetectorAnimation(true, eDetAction, "anm_firemode");

	string_path guns_firemode_anm{};

	//Msg("ANIMS CHANGE FIRE MODE WITH GRIPv ATTACHED");

	if (isHUDAnimationExist("anm_changefiremode_grip_v"))
	{
		strconcat(sizeof(guns_firemode_anm), guns_firemode_anm, "anm_changefiremode_grip_v", (IsMisfire() ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : ""));

		PlayHUDMotionIfExists({ guns_firemode_anm, "anm_changefiremode_grip_v" }, true, GetState());
		return;
	}

	strconcat(sizeof(guns_firemode_anm), guns_firemode_anm, "anm_changefiremode_grip_v_from_", (m_iCurFireMode == 0) ? "a_to_1" : (m_iCurFireMode == 1) ? (m_aFireModes.size() == 3) ? "1_to_2" : "1_to_a" : (m_iCurFireMode == 2) ? "2_to_a" : "a_to_1");

	string64 guns_aim_anm_full;
	strconcat(sizeof(guns_aim_anm_full), guns_aim_anm_full, guns_firemode_anm, (IsMisfire() ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : ""));

	if (isHUDAnimationExist(guns_aim_anm_full))
	{
		PlayHUDMotionNew(guns_aim_anm_full, true, GetState());
		return;
	}
	else if (guns_aim_anm_full && strstr(guns_aim_anm_full, "_jammed"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_aim_anm_full);
		new_guns_aim_anm[strlen(guns_aim_anm_full) - strlen("_jammed")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
	}
	else if (guns_aim_anm_full && strstr(guns_aim_anm_full, "_empty"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_aim_anm_full);
		new_guns_aim_anm[strlen(guns_aim_anm_full) - strlen("_empty")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
	}
}

void CWeaponMagazined::switch2_LaserSwitch()
{
	if (GetState() != eLaserSwitch)
		return;

	LaserSwitchAction = true;

	FireEnd();

	if (IsGripAttached())
	{
		PlayAnimLaserSwitchGrip();
	}
	else if (IsGripvAttached())
	{
		PlayAnimLaserSwitchGripV();
	}
	else 
	{
		PlayAnimLaserSwitch();
	}

	SetPending(TRUE);
}

void CWeaponMagazined::switch2_FlashlightSwitch()
{
	if (GetState() != eFlashlightSwitch)
		return;

	FlashlightSwitchAction = true;

	FireEnd();

	if (IsGripAttached())
	{
		PlayAnimFlashlightSwitchGrip();
	}
	else if (IsGripvAttached())
	{
		PlayAnimFlashlightSwitchGripV();
	}
	else
	{
		PlayAnimLaserSwitch();
	}

	SetPending(TRUE);
}

void CWeaponMagazined::PlayAnimLaserSwitch()
{
	string_path guns_device_switch_anm{};
	strconcat(sizeof(guns_device_switch_anm), guns_device_switch_anm, "anm_laser", !IsLaserOn() ? "_on" : "_off", (IsMisfire() ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : ""));

	if (isHUDAnimationExist(guns_device_switch_anm))
	{
		PlayHUDMotionNew(guns_device_switch_anm, true, GetState());
		return;
	}
	else if (guns_device_switch_anm && strstr(guns_device_switch_anm, "_jammed"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_device_switch_anm);
		new_guns_aim_anm[strlen(guns_device_switch_anm) - strlen("_jammed")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
	}
	else if (guns_device_switch_anm && strstr(guns_device_switch_anm, "_empty"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_device_switch_anm);
		new_guns_aim_anm[strlen(guns_device_switch_anm) - strlen("_empty")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
	}
}

void CWeaponMagazined::PlayAnimLaserSwitchGrip()
{
	string_path guns_device_switch_anm{};
	strconcat(sizeof(guns_device_switch_anm), guns_device_switch_anm, "anm_laser_grip_h", !IsLaserOn() ? "_on" : "_off", (IsMisfire() ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : ""));

	if (isHUDAnimationExist(guns_device_switch_anm))
	{
		PlayHUDMotionNew(guns_device_switch_anm, true, GetState());
		return;
	}
	else if (guns_device_switch_anm && strstr(guns_device_switch_anm, "_jammed"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_device_switch_anm);
		new_guns_aim_anm[strlen(guns_device_switch_anm) - strlen("_jammed")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
	}
	else if (guns_device_switch_anm && strstr(guns_device_switch_anm, "_empty"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_device_switch_anm);
		new_guns_aim_anm[strlen(guns_device_switch_anm) - strlen("_empty")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
	}
}

void CWeaponMagazined::PlayAnimLaserSwitchGripV()
{
	string_path guns_device_switch_anm{};
	strconcat(sizeof(guns_device_switch_anm), guns_device_switch_anm, "anm_laser_grip_v", !IsLaserOn() ? "_on" : "_off", (IsMisfire() ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : ""));

	if (isHUDAnimationExist(guns_device_switch_anm))
	{
		PlayHUDMotionNew(guns_device_switch_anm, true, GetState());
		return;
	}
	else if (guns_device_switch_anm && strstr(guns_device_switch_anm, "_jammed"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_device_switch_anm);
		new_guns_aim_anm[strlen(guns_device_switch_anm) - strlen("_jammed")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
	}
	else if (guns_device_switch_anm && strstr(guns_device_switch_anm, "_empty"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_device_switch_anm);
		new_guns_aim_anm[strlen(guns_device_switch_anm) - strlen("_empty")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
	}
}

void CWeaponMagazined::PlayAnimFlashlightSwitch()
{
	string_path guns_device_switch_anm{};
	strconcat(sizeof(guns_device_switch_anm), guns_device_switch_anm, "anm_torch", !IsFlashlightOn() ? "_on" : "_off", (IsMisfire() ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : ""));

	if (isHUDAnimationExist(guns_device_switch_anm))
	{
		PlayHUDMotionNew(guns_device_switch_anm, true, GetState());
		return;
	}
	else if (guns_device_switch_anm && strstr(guns_device_switch_anm, "_jammed"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_device_switch_anm);
		new_guns_aim_anm[strlen(guns_device_switch_anm) - strlen("_jammed")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
	}
	else if (guns_device_switch_anm && strstr(guns_device_switch_anm, "_empty"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_device_switch_anm);
		new_guns_aim_anm[strlen(guns_device_switch_anm) - strlen("_empty")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
	}
}

void CWeaponMagazined::PlayAnimFlashlightSwitchGrip()
{
	string_path guns_device_switch_anm{};
	strconcat(sizeof(guns_device_switch_anm), guns_device_switch_anm, "anm_torch_grip_h", !IsFlashlightOn() ? "_on" : "_off", (IsMisfire() ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : ""));

	if (isHUDAnimationExist(guns_device_switch_anm))
	{
		PlayHUDMotionNew(guns_device_switch_anm, true, GetState());
		return;
	}
	else if (guns_device_switch_anm && strstr(guns_device_switch_anm, "_jammed"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_device_switch_anm);
		new_guns_aim_anm[strlen(guns_device_switch_anm) - strlen("_jammed")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
	}
	else if (guns_device_switch_anm && strstr(guns_device_switch_anm, "_empty"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_device_switch_anm);
		new_guns_aim_anm[strlen(guns_device_switch_anm) - strlen("_empty")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
	}
}

void CWeaponMagazined::PlayAnimFlashlightSwitchGripV()
{
	string_path guns_device_switch_anm{};
	strconcat(sizeof(guns_device_switch_anm), guns_device_switch_anm, "anm_torch_grip_v", !IsFlashlightOn() ? "_on" : "_off", (IsMisfire() ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : ""));

	if (isHUDAnimationExist(guns_device_switch_anm))
	{
		PlayHUDMotionNew(guns_device_switch_anm, true, GetState());
		return;
	}
	else if (guns_device_switch_anm && strstr(guns_device_switch_anm, "_jammed"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_device_switch_anm);
		new_guns_aim_anm[strlen(guns_device_switch_anm) - strlen("_jammed")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
	}
	else if (guns_device_switch_anm && strstr(guns_device_switch_anm, "_empty"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_device_switch_anm);
		new_guns_aim_anm[strlen(guns_device_switch_anm) - strlen("_empty")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
	}
}

#ifdef DEBUG
#include "ai\stalker\ai_stalker.h"
#include "object_handler_planner.h"
#endif
void CWeaponMagazined::switch2_Fire	()
{
	CInventoryOwner* io		= smart_cast<CInventoryOwner*>(H_Parent());
	CInventoryItem* ii		= smart_cast<CInventoryItem*>(this);
#ifdef DEBUG
	if (!io)
		return;
	//VERIFY2					(io,make_string("no inventory owner, item %s",*cName()));

	if (ii != io->inventory().ActiveItem())
		Msg					("! not an active item, item %s, owner %s, active item %s",*cName(),*H_Parent()->cName(),io->inventory().ActiveItem() ? *io->inventory().ActiveItem()->object().cName() : "no_active_item");

	if ( !(io && (ii == io->inventory().ActiveItem())) ) 
	{
		CAI_Stalker			*stalker = smart_cast<CAI_Stalker*>(H_Parent());
		if (stalker) {
			stalker->planner().show						();
			stalker->planner().show_current_world_state	();
			stalker->planner().show_target_world_state	();
		}
	}
#else
	if (!io)
		return;
#endif // DEBUG

//
//	VERIFY2(
//		io && (ii == io->inventory().ActiveItem()),
//		make_string(
//			"item[%s], parent[%s]",
//			*cName(),
//			H_Parent() ? *H_Parent()->cName() : "no_parent"
//		)
//	);

	m_bStopedAfterQueueFired = false;
	m_bFireSingleShot = true;
	m_iShotNum = 0;

    if((OnClient() || Level().IsDemoPlay())&& !IsWorking())
		FireStart();

}

void CWeaponMagazined::switch2_Empty()
{
	OnZoomOut();

	if (psActorFlags2.test(AF_WPN_RELOAD_TYPE))
	{
		if (!TryReload())
		{
			OnEmptyClick();
		}
		else
		{
			inherited::FireEnd();
		}
	}
	else
	{ 
		inherited::FireEnd();
	}
}
void CWeaponMagazined::PlayReloadSound()
{
	if (m_sounds_enabled)
	{
		if (m_bIsRevolver)
		{
			u8 bullets_to_load = iMagazineSize - GetAvailableCartridgesToLoad(!iAmmoElapsed);

			string128 sndReloadName;
			strconcat(sizeof(sndReloadName), sndReloadName, "sndReload", (bullets_to_load > 0) ? std::to_string(bullets_to_load).c_str() : "Empty");

			if (m_sounds.FindSoundItem(sndReloadName, false))
				m_sounds.PlaySound(sndReloadName, get_LastFP(), H_Root(), !!GetHUDmode(), false, (u8)-1);
		}
		else
		{
			if (IsGripAttached())
			{
				if (iAmmoElapsed == 0)
					if (m_sounds.FindSoundItem("sndReloadEmptyGripH", false) && psWpnAnimsFlag.test(ANM_RELOAD_EMPTY))
						PlaySound("sndReloadEmptyGripH", get_LastFP());
					else
						PlaySound("sndReloadGripH", get_LastFP());
				else
					PlaySound("sndReloadGripH", get_LastFP());
			}
			else if (IsGripvAttached())
			{
				if (iAmmoElapsed == 0)
					if (m_sounds.FindSoundItem("sndReloadEmptyGripV", false) && psWpnAnimsFlag.test(ANM_RELOAD_EMPTY))
						PlaySound("sndReloadEmptyGripV", get_LastFP());
					else
						PlaySound("sndReloadGripV", get_LastFP());
				else
					PlaySound("sndReloadGripV", get_LastFP());
			}
			else
			{
				if (iAmmoElapsed == 0)
					if (m_sounds.FindSoundItem("sndReloadEmpty", false) && psWpnAnimsFlag.test(ANM_RELOAD_EMPTY))
						PlaySound("sndReloadEmpty", get_LastFP());
					else
						PlaySound("sndReload", get_LastFP());
				else
				PlaySound("sndReload", get_LastFP());
			}
		}
	}
}

void CWeaponMagazined::switch2_Reload()
{
	CWeapon::FireEnd	();

	PlayReloadSound		();
	PlayAnimReload		();
	SetPending			(TRUE);
}
void CWeaponMagazined::switch2_Hiding()
{
	OnZoomOut();
	CWeapon::FireEnd();

	if (m_sounds_enabled)
	{
		if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_HIDE_EMPTY) && WeaponSoundExist(m_section_id.c_str(), "snd_close"))
			PlaySound("sndClose", get_LastFP());
		else
			PlaySound("sndHide", get_LastFP());
	}

	PlayAnimHide		();
	SetPending			(TRUE);
}

void CWeaponMagazined::switch2_Unmis()
{
	VERIFY(GetState() == eUnMisfire);

	if (m_sounds_enabled)
	{
		if (IsGripAttached())
		{
			if ((iAmmoElapsed == 1) && m_sounds.FindSoundItem("sndReloadMisfireEmptyGripH", false) && isHUDAnimationExist("anm_reload_misfire_empty_grip_h"))
				PlaySound("sndReloadMisfireEmptyGripH", get_LastFP());
			else if ((iAmmoElapsed == 1) && m_sounds.FindSoundItem("sndReloadJammedEmptyGripH", false) && isHUDAnimationExist("anm_reload_jammed_empty_grip_h"))
				PlaySound("sndReloadJammedEmptyGripH", get_LastFP());
			else if (m_sounds.FindSoundItem("sndReloadMisfireGripH", false) && isHUDAnimationExist("anm_reload_misfire_grip_h"))
				PlaySound("sndReloadMisfireGripH", get_LastFP());
			else if (m_sounds.FindSoundItem("sndReloadJammedGripH", false) && isHUDAnimationExist("anm_reload_jammed_grip_h"))
				PlaySound("sndReloadJammedGripH", get_LastFP());
			else
				PlayReloadSound();
		}
		else if (IsGripvAttached()) 
		{
			if ((iAmmoElapsed == 1) && m_sounds.FindSoundItem("sndReloadMisfireEmptyGripV", false) && isHUDAnimationExist("anm_reload_misfire_empty_grip_v"))
				PlaySound("sndReloadMisfireEmptyGripV", get_LastFP());
			else if ((iAmmoElapsed == 1) && m_sounds.FindSoundItem("sndReloadJammedEmptyGripV", false) && isHUDAnimationExist("anm_reload_jammed_empty_grip_v"))
				PlaySound("sndReloadJammedEmptyGripV", get_LastFP());
			else if (m_sounds.FindSoundItem("sndReloadMisfire", false) && isHUDAnimationExist("anm_reload_misfire_grip_v"))
				PlaySound("sndReloadMisfireGripV", get_LastFP());
			else if (m_sounds.FindSoundItem("sndReloadJammedGripV", false) && isHUDAnimationExist("anm_reload_jammed_grip_v"))
				PlaySound("sndReloadJammedGripV", get_LastFP());
			else
				PlayReloadSound();
		}
		else
		{
			if ((iAmmoElapsed == 1) && m_sounds.FindSoundItem("sndReloadMisfireEmpty", false) && isHUDAnimationExist("anm_reload_misfire_empty"))
				PlaySound("sndReloadMisfireEmpty", get_LastFP());
			else if ((iAmmoElapsed == 1) && m_sounds.FindSoundItem("sndReloadJammedEmpty", false) && isHUDAnimationExist("anm_reload_jammed_empty"))
				PlaySound("sndReloadJammedEmpty", get_LastFP());
			else if (m_sounds.FindSoundItem("sndReloadMisfire", false) && isHUDAnimationExist("anm_reload_misfire"))
				PlaySound("sndReloadMisfire", get_LastFP());
			else if (m_sounds.FindSoundItem("sndReloadJammed", false) && isHUDAnimationExist("anm_reload_jammed"))
				PlaySound("sndReloadJammed", get_LastFP());
			else
				PlayReloadSound();
		}
	}

	string128 anmUnmisName{};

	if (IsGripAttached())
	{
	//	Msg("RELOAD ANIMATION: GRIP");

	strconcat(sizeof(anmUnmisName), anmUnmisName, "anm_reload_grip_h", isHUDAnimationExist("anm_reload_grip_h_misfire") ? "_misfire" : "_jammed", (iAmmoElapsed == 1) ? "_empty" : "");

	if (isHUDAnimationExist(anmUnmisName))
		PlayHUDMotionIfExists({ anmUnmisName, "anm_reload_grip_h" }, true, GetState());
	else
		PlayAnimReload();
	}
	else if (IsGripvAttached())
	{
		//	Msg("RELOAD ANIMATION: GRIP V");

		strconcat(sizeof(anmUnmisName), anmUnmisName, "anm_reload_grip_v", isHUDAnimationExist("anm_reload_grip_v_misfire") ? "_misfire" : "_jammed", (iAmmoElapsed == 1) ? "_empty" : "");

		if (isHUDAnimationExist(anmUnmisName))
			PlayHUDMotionIfExists({ anmUnmisName, "anm_reload_grip_v" }, true, GetState());
		else
			PlayAnimReload();
	}
	else
	{
	//	Msg("RELOAD ANIMATION: NO GRIP");

		strconcat(sizeof(anmUnmisName), anmUnmisName, "anm_reload", isHUDAnimationExist("anm_reload_misfire") ? "_misfire" : "_jammed", (iAmmoElapsed == 1) ? "_empty" : "");

		if (isHUDAnimationExist(anmUnmisName))
			PlayHUDMotionIfExists({ anmUnmisName, "anm_reload" }, true, GetState());
		else
			PlayAnimReload();
	}
}

void CWeaponMagazined::switch2_Hidden()
{
	CWeapon::FireEnd();

	StopCurrentAnimWithoutCallback();

	signal_HideComplete		();
	RemoveShotEffector		();
}
void CWeaponMagazined::switch2_Showing()
{
	if(m_sounds_enabled)
		PlaySound			("sndShow",get_LastFP());

	SetPending			(TRUE);
	PlayAnimShow		();
}

#include "CustomDetector.h"

bool CWeaponMagazined::Action(s32 cmd, u32 flags) 
{
	if(inherited::Action(cmd, flags)) return true;
	
	//åñëè îðóæèå ÷åì-òî çàíÿòî, òî íè÷åãî íå äåëàòü
	if(IsPending()) return false;
	
	switch(cmd) 
	{
	case kWPN_RELOAD:
		{
			if (Actor()->mstate_real & (mcSprint) && !GameConstants::GetReloadIfSprint())
				break;
			else if (flags & CMD_START)
				if (iAmmoElapsed < iMagazineSize || IsMisfire())
				{
					if (GetState() == eUnMisfire) // Rietmon: Çàïðåùàåì ïåðåçàðÿäêó, åñëè èãðàåò àíèìà ïåðåäåðãèâàíèÿ çàòâîðà
						return false;

					PIItem Det = Actor()->inventory().ItemFromSlot(DETECTOR_SLOT);
					if (!Det)
						Reload(); // Rietmon: Åñëè â ñëîòå íåòó äåòåêòîðà, òî îí íå ìîæåò áûòü àêòèâåí

					if (Det)
					{
						CCustomDetector* pDet = smart_cast<CCustomDetector*>(Det);
						if (!pDet->IsWorking())
							Reload();
					}
				}
		} 
		return true;
	case kWPN_INSPECT:
	{
		if (flags & CMD_START)
		{

		//	UpdateWeaponDoFInspect();

			if (isHUDAnimationExist("anm_inspect_weapon") && (!IsMisfire()))
			{
				if (iAmmoElapsed == 0)
				{
					if (IsGrenadeLauncherAttached() && isHUDAnimationExist("anm_inspect_weapon_empty_gl"))
					{
						PlayHUDMotion("anm_inspect_weapon_empty_gl", FALSE, this, GetState());
						PlaySound("sndInspectWeaponEmptyGl", get_LastFP());
						//Msg("Weapon Inspect Empty GL");
					}
					else
					{
						PlayHUDMotion("anm_inspect_weapon_empty", FALSE, this, GetState());
						PlaySound("sndInspectWeaponEmpty", get_LastFP());
						//Msg("Weapon Inspect Empty");
					}
				}
				else
				{
					if (IsGrenadeLauncherAttached() && isHUDAnimationExist("anm_inspect_weapon_gl"))
					{
						PlayHUDMotion("anm_inspect_weapon_gl", FALSE, this, GetState());
						PlaySound("sndInspectWeaponGl", get_LastFP());
						//Msg("Weapon Inspect GL");
					}
					else
					{
						PlayHUDMotion("anm_inspect_weapon", FALSE, this, GetState());
						PlaySound("sndInspectWeapon", get_LastFP());
						//Msg("Weapon Inspect");
					}
				}
			}
		};
	}
	return true;
	case kWPN_FIREMODE_PREV:
		{
			if(flags&CMD_START) 
			{
				OnPrevFireMode();
				return true;
			};
		}break;
	case kWPN_FIREMODE_NEXT:
		{
			if(flags&CMD_START) 
			{
				OnNextFireMode();
				return true;
			};
		}break;
	}
	return false;
}

bool CWeaponMagazined::AttachIsBlockedByAnotherAddon(shared_str addon_name)
{
	bool atch_result = false;
	attach_blocker_addons.clear();

	if (pSettings->line_exist(*addon_name, "attach_blocked_by_addons"))
	{
		const char* S = pSettings->r_string(*addon_name, "attach_blocked_by_addons");
		if (S && strlen(S))
		{
			const int count = _GetItemCount(S);
			string128 _attach_blocker_addon_name{};
			for (int it = 0; it < count; ++it)
			{
				_GetItem(S, it, _attach_blocker_addon_name);
				attach_blocker_addons.push_back(_attach_blocker_addon_name);
			}

			for (const shared_str& _attach_blocker_addon_name : attach_blocker_addons)
			{
				if (IsAddonAttachedBySection(_attach_blocker_addon_name))
				{
					atch_result = true;

					//Msg("[%s]: addon attach blocked by [%s]", *addon_name, *_attach_blocker_addon_name);
				}
			}

			attach_blocker_addons.clear();
			return atch_result;
		}
		else
			return false;
	}
	else
		return false;
}

bool CWeaponMagazined::CanAttach(PIItem pIItem)
{
	bool result = false;
	bool check_correct_name = false;

	auto CanAttachThisAddon = [&](bool cast, bool attachable, shared_str addon_name, int addon_state)
		{
			if (cast && attachable && addon_name.size())
			{
				if (addon_name == m_sScopeName)
				{
					for (const auto& scopes_sect : m_scopes)
					{
						check_correct_name = bUseAltScope ? (scopes_sect == pIItem->object().cNameSect()) :
							(pSettings->r_string(scopes_sect, "scope_name") == pIItem->object().cNameSect());
					}
				}
				else if (addon_name == m_sLaserName)
				{
					for (const auto& laser_sect : m_availableLasers)
					{
						check_correct_name == (pSettings->r_string(laser_sect, "laser_designator_name") == pIItem->object().cNameSect());
					}
				}
				else if (addon_name == m_sTacticalTorchName)
				{
					for (const auto& flashlight_sect : m_availableFlashlights)
					{
						check_correct_name == (pSettings->r_string(flashlight_sect, "tactical_torch_name") == pIItem->object().cNameSect());
					}
				}
				else if (addon_name == m_sStockName)
				{
					for (const auto& stock_sect : m_availableStocks)
					{
						check_correct_name == (pSettings->r_string(stock_sect, "stock_name") == pIItem->object().cNameSect());
					}
				}
				else if (addon_name == m_sHandguardName)
				{
					for (const auto& handguard_sect : m_availableHandguards)
					{
						check_correct_name == (pSettings->r_string(handguard_sect, "handguard_name") == pIItem->object().cNameSect());
					}
				}
				else if (addon_name == m_sPistolgripName)
				{
					for (const auto& pistolgrip_sect : m_availablePistolgrips)
					{
						check_correct_name == (pSettings->r_string(pistolgrip_sect, "pistolgrip_name") == pIItem->object().cNameSect());
					}
				}
				else
				{
					check_correct_name = (addon_name == pIItem->object().cNameSect());
				}

				if (check_correct_name && (m_flagsAddOnState & addon_state) == 0 && (!AttachIsBlockedByAnotherAddon(addon_name)))
				{
					result = true;
				}
			}
		};

	CanAttachThisAddon(smart_cast<CScope*>(pIItem), ScopeAttachable(), m_sScopeName, CSE_ALifeItemWeapon::eWeaponAddonScope);
	CanAttachThisAddon(smart_cast<CSilencer*>(pIItem), SilencerAttachable(), m_sSilencerName, CSE_ALifeItemWeapon::eWeaponAddonSilencer);
	CanAttachThisAddon(smart_cast<CGrenadeLauncher*>(pIItem), GrenadeLauncherAttachable(), m_sGrenadeLauncherName, CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher);
	CanAttachThisAddon(smart_cast<CLaserDesignator*>(pIItem), LaserAttachable(), m_sLaserName, CSE_ALifeItemWeapon::eWeaponAddonLaserDesignator);
	CanAttachThisAddon(smart_cast<CTacticalTorch*>(pIItem), TacticalTorchAttachable(), m_sTacticalTorchName, CSE_ALifeItemWeapon::eWeaponAddonTacticalTorch);

	CanAttachThisAddon(smart_cast<CStock*>(pIItem), StockAttachable(), m_sStockName, CSE_ALifeItemWeapon::eWeaponAddonStock);
	CanAttachThisAddon(smart_cast<CHorGrip*>(pIItem), GripAttachable(), m_sGripName, CSE_ALifeItemWeapon::eWeaponAddonGrip);
	CanAttachThisAddon(smart_cast<CVerGrip*>(pIItem), GripvAttachable(), m_sGripvName, CSE_ALifeItemWeapon::eWeaponAddonGripv);
	CanAttachThisAddon(smart_cast<CHandguard*>(pIItem), HandguardAttachable(), m_sHandguardName, CSE_ALifeItemWeapon::eWeaponAddonHandguard);
	CanAttachThisAddon(smart_cast<CPistolGrip*>(pIItem), PistolgripAttachable(), m_sPistolgripName, CSE_ALifeItemWeapon::eWeaponAddonPistolgrip);

	if (result)
		return true;
	else
		return inherited::CanAttach(pIItem);
}

bool CWeaponMagazined::CanDetach(const char* item_section_name)
{
	bool result = false;
	bool check_correct_name = false;

	auto CanDetachThisAddon = [&](bool attachable, shared_str addon_name, int addon_state)
		{
			if (attachable && addon_name.size())
			{
				if (addon_name == m_sScopeName)
				{
					for (const auto& scopes_sect : m_scopes)
					{
						check_correct_name = bUseAltScope ? (scopes_sect == item_section_name) :
							(pSettings->r_string(scopes_sect, "scope_name") == item_section_name);
					}
				}
				else if (addon_name == m_sLaserName)
				{
					for (const auto& laser_sect : m_availableLasers)
					{
						check_correct_name == (pSettings->r_string(laser_sect, "laser_designator_name") == item_section_name);
					}
				}
				else if (addon_name == m_sTacticalTorchName)
				{
					for (const auto& flashlight_sect : m_availableFlashlights)
					{
						check_correct_name == (pSettings->r_string(flashlight_sect, "tactical_torch_name") == item_section_name);
					}
				}
				else if (addon_name == m_sStockName)
				{
					for (const auto& stock_sect : m_availableStocks)
					{
						check_correct_name == (pSettings->r_string(stock_sect, "stock_name") == item_section_name);
					}
				}
				else if (addon_name == m_sPistolgripName)
				{
					for (const auto& pistolgrip_sect : m_availablePistolgrips)
					{
						check_correct_name == (pSettings->r_string(pistolgrip_sect, "pistolgrip_name") == item_section_name);
					}
				}
				else if (addon_name == m_sHandguardName)
				{
					for (const auto& handguard_sect : m_availableHandguards)
					{
						check_correct_name == (pSettings->r_string(handguard_sect, "handguard_name") == item_section_name);
					}
				}
				else
				{
					check_correct_name = (addon_name == item_section_name);
				}

				if (check_correct_name && (m_flagsAddOnState & addon_state) != 0)
				{
					result = true;
				}
			}
		};

	CanDetachThisAddon(ScopeAttachable(), m_sScopeName, CSE_ALifeItemWeapon::eWeaponAddonScope);
	CanDetachThisAddon(SilencerAttachable(), m_sSilencerName, CSE_ALifeItemWeapon::eWeaponAddonSilencer);
	CanDetachThisAddon(GrenadeLauncherAttachable(), m_sGrenadeLauncherName, CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher);
	CanDetachThisAddon(LaserAttachable(), m_sLaserName, CSE_ALifeItemWeapon::eWeaponAddonLaserDesignator);
	CanDetachThisAddon(TacticalTorchAttachable(), m_sTacticalTorchName, CSE_ALifeItemWeapon::eWeaponAddonTacticalTorch);

	CanDetachThisAddon(StockAttachable(), m_sStockName, CSE_ALifeItemWeapon::eWeaponAddonStock);
	CanDetachThisAddon(GripAttachable(), m_sGripName, CSE_ALifeItemWeapon::eWeaponAddonGrip);
	CanDetachThisAddon(GripvAttachable(), m_sGripvName, CSE_ALifeItemWeapon::eWeaponAddonGripv);
	CanDetachThisAddon(HandguardAttachable(), m_sHandguardName, CSE_ALifeItemWeapon::eWeaponAddonHandguard);
	CanDetachThisAddon(PistolgripAttachable(), m_sPistolgripName, CSE_ALifeItemWeapon::eWeaponAddonPistolgrip);

	if (result)
		return true;
	else
		return inherited::CanDetach(item_section_name);
}

bool CWeaponMagazined::Attach(PIItem pIItem, bool b_send_event)
{
	bool result = false;
	
	auto AttachThisAddon = [&](bool cast, bool attachable, shared_str addon_name, int addon_state)
	{
		if (cast && attachable && addon_name.size())
		{
			if (addon_name == m_sScopeName)
			{
				SCOPES_VECTOR_IT it = m_scopes.begin();
				for (; it != m_scopes.end(); it++)
				{
					if (bUseAltScope)
					{
						if (*it == pIItem->object().cNameSect())
							m_cur_scope = u8(it - m_scopes.begin());
					}
					else
					{
						if (pSettings->r_string((*it), "scope_name") == pIItem->object().cNameSect())
							m_cur_scope = u8(it - m_scopes.begin());
					}
				}
			}
			
			if ((addon_name == m_sScopeName ? true : addon_name == pIItem->object().cNameSect()) && (m_flagsAddOnState & addon_state) == 0 && (!AttachIsBlockedByAnotherAddon(addon_name)))
			{
				m_flagsAddOnState |= addon_state;

				if (addon_name == m_sScopeName && bUseAltScope)
				{
					bNVsecondVPstatus = !!pSettings->line_exist(pIItem->object().cNameSect(), "scope_nightvision");
				}

				if (b_send_event && OnServer())
					pIItem->object().DestroyObject();

				UpdateAltScope();
				InitAddons();
				UpdateAddonsVisibility();

				if (GetState() == eIdle)
					PlayAnimIdle();

				result = true;
			}
		}
	};

	auto AttachCompatibleAddon = [&](bool cast, bool attachable, shared_str addon_name, int addon_state)
	{
		bool bCompatibleAddon = false;

		if (cast && attachable && (m_flagsAddOnState & addon_state) == 0 && (!AttachIsBlockedByAnotherAddon(addon_name)))
		{
			if (addon_name.size() || addon_name == pIItem->object().cNameSect())
			{
				bCompatibleAddon = true;
			}
			else
			{

				auto SetAvailableAddon = [&](RStringVec vec, shared_str target_sect, shared_str target_addon_name,
					const char* config_name, const char* config_x, const char* config_y)
					{
						for (const auto& common_sect : vec)
						{
							shared_str common_addon_name = pSettings->r_string(common_sect, config_name);
							if (common_addon_name == pIItem->object().cNameSect())
							{
								bCompatibleAddon = true;
								target_sect = common_sect;
								target_addon_name = common_addon_name;

								int icon_x = GetAddonXYNameBySection(target_addon_name);
								int icon_y = GetAddonXYNameBySection(target_addon_name, true);
								icon_x = pSettings->r_s32(common_sect, config_x);
								icon_y = pSettings->r_s32(common_sect, config_y);
								break;
							}
						}
					};

				if (addon_name == m_sLaserName)
					SetAvailableAddon(m_availableLasers, m_sLaserAttachSection, addon_name, "laser_designator_name", "laser_designator_x", "laser_designator_y");
				else if (addon_name == m_sTacticalTorchName)
					SetAvailableAddon(m_availableFlashlights, m_sTacticalTorchAttachSection, addon_name, "tactical_torch_name", "tactical_torch_x", "tactical_torch_y");
				else if (addon_name == m_sStockName)
					SetAvailableAddon(m_availableStocks, m_sStockAttachSection, addon_name, "stock_name", "stock_x", "stock_y");
				else if (addon_name == m_sPistolgripName)
					SetAvailableAddon(m_availablePistolgrips, m_sPistolgripAttachSection, addon_name, "pistolgrip_name", "pistolgrip_x", "pistolgrip_y");
				else if (addon_name == m_sHandguardName)
					SetAvailableAddon(m_availableHandguards, m_sHandguardAttachSection, addon_name, "handguard_name", "handguard_x", "handguard_y");
	
			}

			if (bCompatibleAddon)
			{
				if (b_send_event && OnServer())
					pIItem->object().DestroyObject();

				UpdateAltScope();
				InitAddons();
				UpdateAddonsVisibility();

				if (GetState() == eIdle)
					PlayAnimIdle();

				result = true;
			}
		}
	};

	AttachThisAddon(smart_cast<CScope*>(pIItem), ScopeAttachable(), m_sScopeName, CSE_ALifeItemWeapon::eWeaponAddonScope);
	AttachThisAddon(smart_cast<CSilencer*>(pIItem), SilencerAttachable(), m_sSilencerName, CSE_ALifeItemWeapon::eWeaponAddonSilencer);
	AttachThisAddon(smart_cast<CGrenadeLauncher*>(pIItem), GrenadeLauncherAttachable(), m_sGrenadeLauncherName, CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher);
	AttachCompatibleAddon(smart_cast<CLaserDesignator*>(pIItem), LaserAttachable(), m_sLaserName, CSE_ALifeItemWeapon::eWeaponAddonLaserDesignator);
	AttachCompatibleAddon(smart_cast<CTacticalTorch*>(pIItem), TacticalTorchAttachable(), m_sTacticalTorchName, CSE_ALifeItemWeapon::eWeaponAddonTacticalTorch);

	AttachCompatibleAddon(smart_cast<CStock*>(pIItem), StockAttachable(), m_sStockName, CSE_ALifeItemWeapon::eWeaponAddonStock);
	AttachThisAddon(smart_cast<CHorGrip*>(pIItem), GripAttachable(), m_sGripName, CSE_ALifeItemWeapon::eWeaponAddonGrip);
	AttachThisAddon(smart_cast<CVerGrip*>(pIItem), GripvAttachable(), m_sGripvName, CSE_ALifeItemWeapon::eWeaponAddonGripv);
	AttachCompatibleAddon(smart_cast<CHandguard*>(pIItem), HandguardAttachable(), m_sHandguardName, CSE_ALifeItemWeapon::eWeaponAddonHandguard);
	AttachCompatibleAddon(smart_cast<CPistolGrip*>(pIItem), PistolgripAttachable(), m_sPistolgripName, CSE_ALifeItemWeapon::eWeaponAddonPistolgrip);

	if (result)
		return true;
	else
		return inherited::Attach(pIItem, b_send_event);
}

bool CWeaponMagazined::DetachScope(const char* item_section_name, bool b_spawn_item)
{
	bool detached = false;
	SCOPES_VECTOR_IT it = m_scopes.begin();
	shared_str iter_scope_name = "none";
	for(; it!=m_scopes.end(); it++)
	{
		if (bUseAltScope)
		{
			iter_scope_name = (*it);
		}
		else
		{
			iter_scope_name = pSettings->r_string((*it), "scope_name");
		}
		if(!xr_strcmp(iter_scope_name, item_section_name))
		{
			m_cur_scope = NULL;
			m_cur_scope_bone = NULL;
			m_cur_scope_bone_aim = NULL;
			m_cur_scope_bone_lens = NULL;
			m_cur_scope_bone_part = NULL;
			m_cur_scope_bone_aim_part = NULL;
			m_cur_scope_show_bone_rail = NULL;
			m_cur_scope_hide_bone_rail = NULL;
			m_cur_aim_hide_laser_dot_bone = NULL;
			m_cur_scope_bone_reticle = NULL;
			m_bAltZoomActive = false;
			detached = true;
		}
	}
	return detached;
}

bool CWeaponMagazined::Detach(const char* item_section_name, bool b_spawn_item)
{
	bool result = false;

	auto DetachThisAddon = [&](bool attachable, shared_str addon_name, int addon_state)
	{
		if (attachable && addon_name.size() &&
			(addon_name == m_sScopeName ? DetachScope(item_section_name, b_spawn_item) : (addon_name == item_section_name)) &&
			(m_flagsAddOnState & addon_state) != 0)
		{
			m_flagsAddOnState &= ~addon_state;

			if (addon_name == m_sScopeName)
			{
				UpdateAltScope();
				UpdateHudAltAimHud();
				UpdateHudAltAimHudMode2();
			}
			
			InitAddons();
			UpdateAddonsVisibility();

			if (GetState() == eIdle)
				PlayAnimIdle();

			result = true;
		}
	};

	DetachThisAddon(ScopeAttachable(), m_sScopeName, CSE_ALifeItemWeapon::eWeaponAddonScope);
	DetachThisAddon(SilencerAttachable(), m_sSilencerName, CSE_ALifeItemWeapon::eWeaponAddonSilencer);
	DetachThisAddon(GrenadeLauncherAttachable(), m_sGrenadeLauncherName, CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher);
	DetachThisAddon(LaserAttachable(), m_sLaserName, CSE_ALifeItemWeapon::eWeaponAddonLaserDesignator);
	DetachThisAddon(TacticalTorchAttachable(), m_sTacticalTorchName, CSE_ALifeItemWeapon::eWeaponAddonTacticalTorch);

	DetachThisAddon(StockAttachable(), m_sStockName, CSE_ALifeItemWeapon::eWeaponAddonStock);
	DetachThisAddon(GripAttachable(), m_sGripName, CSE_ALifeItemWeapon::eWeaponAddonGrip);
	DetachThisAddon(GripvAttachable(), m_sGripvName, CSE_ALifeItemWeapon::eWeaponAddonGripv);
	DetachThisAddon(HandguardAttachable(), m_sHandguardName, CSE_ALifeItemWeapon::eWeaponAddonHandguard);
	DetachThisAddon(PistolgripAttachable(), m_sPistolgripName, CSE_ALifeItemWeapon::eWeaponAddonPistolgrip);

	if (result)
		return CInventoryItemObject::Detach(item_section_name, b_spawn_item);
	else
		return inherited::Detach(item_section_name, b_spawn_item);
}

/*
void CWeaponMagazined::LoadAddons()
{
	m_zoom_params.m_fIronSightZoomFactor = READ_IF_EXISTS( pSettings, r_float, cNameSect(), "ironsight_zoom_factor", 50.0f );

}
*/
void CWeaponMagazined::InitAddons()
{
	m_zoom_params.m_fIronSightZoomFactor = READ_IF_EXISTS( pSettings, r_float, cNameSect(), "ironsight_zoom_factor", 50.0f );

	SetAnimFlag(ANM_SHOT_AIM,		"anm_shots_when_aim");
	SetAnimFlag(ANM_SHOT_AIM_GL,	"anm_shots_w_gl_when_aim");

	m_weapon_attaches.clear();

	if (IsScopeAttached())
	{
		if ( m_eScopeStatus == ALife::eAddonAttachable )
		{
			LoadCurrentScopeParams(GetScopeName().c_str());

			if (pSettings->line_exist(m_scopes[m_cur_scope], "bones"))
			{
				// BONE FOR SCOPE IDLE MODEL
				pcstr ScopeBone = pSettings->r_string(m_scopes[m_cur_scope], "scope_bones");
				m_cur_scope_bone = ScopeBone;

				// BONE FOR SCOPE AIM MODEL LENS
				pcstr ScopeBoneLens = pSettings->r_string(m_scopes[m_cur_scope], "scope_bones_lens");
				m_cur_scope_bone_lens = ScopeBoneLens;

				// BONE FOR SCOPE AIM MODEL
				pcstr ScopeBoneAim = pSettings->r_string(m_scopes[m_cur_scope], "scope_bones_aim");
				m_cur_scope_bone_aim = ScopeBoneAim;

				// BONE FOR SCOPE IDLE MODEL
				pcstr ScopeBonePart = pSettings->r_string(m_scopes[m_cur_scope], "scope_bones_part");
				m_cur_scope_bone_part = ScopeBonePart;

				// BONE FOR SCOPE AIM MODEL
				pcstr ScopeBoneAimPart = pSettings->r_string(m_scopes[m_cur_scope], "scope_bones_part_aim");
				m_cur_scope_bone_aim_part = ScopeBoneAimPart;

				// BONE FOR Picatinny RAILS
				pcstr ScopeShowBoneRail = pSettings->r_string(m_scopes[m_cur_scope], "scope_show_bones_rail");
				m_cur_scope_show_bone_rail = ScopeShowBoneRail;

				// BONE FOR Picatinny RAILS
				pcstr ScopeHideBoneRail = pSettings->r_string(m_scopes[m_cur_scope], "scope_hide_bones_rail");
				m_cur_scope_hide_bone_rail = ScopeHideBoneRail;

				// BONE HIDE LASER DOT
				pcstr LaserDotBones = pSettings->r_string(m_scopes[m_cur_scope], "aim_remove_laser_dot");
				m_cur_aim_hide_laser_dot_bone = LaserDotBones;

				// BONE SCOPES RETICLES
				pcstr ScopeReticleBones = pSettings->r_string(m_scopes[m_cur_scope], "scope_bone_reticles");
				m_cur_scope_bone_reticle = ScopeReticleBones;
			}

			if (m_sScopeAttachSection.size() && pSettings->line_exist(m_sScopeAttachSection, "attach_hud_visual"))
				WeaponAttach().CreateAttach(m_sScopeAttachSection, m_weapon_attaches);
		}
		else if (m_eScopeStatus == ALife::eAddonPermanent)
		{
			if (m_sScopeAttachSection.size() && pSettings->line_exist(m_sScopeAttachSection, "attach_hud_visual"))
				WeaponAttach().CreateAttach(m_sScopeAttachSection, m_weapon_attaches);
		}
	}
	else
	{
		if (m_sScopeAttachSection.size() && pSettings->line_exist(m_sScopeAttachSection, "attach_hud_visual"))
			WeaponAttach().RemoveAttach(m_sScopeAttachSection, m_weapon_attaches);

		if (m_UIScope)
			xr_delete(m_UIScope);

		if (bIsSecondVPZoomPresent())
			m_zoom_params.m_fSecondVPFovFactor = 0.0f;

		if (IsZoomEnabled())
			m_zoom_params.m_fIronSightZoomFactor = pSettings->r_float(cNameSect(), "scope_zoom_factor");
	}

	if (IsSilencerAttached() && SilencerAttachable())
	{
		m_sFlameParticlesCurrent = m_sSilencerFlameParticles;
		m_sSmokeParticlesCurrent = m_sSilencerSmokeParticles;
		m_sSndShotCurrent = "sndSilencerShot";

		//ïîäñâåòêà îò âûñòðåëà
		LoadLights(*cNameSect(), "silencer_");
		ApplySilencerKoeffs();

		if (m_sSilencerAttachSection.size() && pSettings->line_exist(m_sSilencerAttachSection, "attach_hud_visual"))
			WeaponAttach().CreateAttach(m_sSilencerAttachSection, m_weapon_attaches);
	}
	else
	{
		if (m_sSilencerAttachSection.size() && pSettings->line_exist(m_sSilencerAttachSection, "attach_hud_visual"))
			WeaponAttach().RemoveAttach(m_sSilencerAttachSection, m_weapon_attaches);

		m_sFlameParticlesCurrent = m_sFlameParticles;
		m_sSmokeParticlesCurrent = m_sSmokeParticles;
		m_sSndShotCurrent = "sndShot";

		//ïîäñâåòêà îò âûñòðåëà
		LoadLights		(*cNameSect(), "");
		ResetSilencerKoeffs();
	}

	if (m_sGrenadeLauncherAttachSection.size() && pSettings->line_exist(m_sGrenadeLauncherAttachSection, "attach_hud_visual"))
	{
		if (IsGrenadeLauncherAttached())
			WeaponAttach().CreateAttach(m_sGrenadeLauncherAttachSection, m_weapon_attaches);
		else
			WeaponAttach().RemoveAttach(m_sGrenadeLauncherAttachSection, m_weapon_attaches);
	}

	if (m_sGripAttachSection.size() && pSettings->line_exist(m_sGripAttachSection, "attach_hud_visual"))
	{
		if (IsGripAttached())
			WeaponAttach().CreateAttach(m_sGripAttachSection, m_weapon_attaches);
		else
			WeaponAttach().RemoveAttach(m_sGripAttachSection, m_weapon_attaches);
	}

	if (m_sGripvAttachSection.size() && pSettings->line_exist(m_sGripvAttachSection, "attach_hud_visual"))
	{
		if (IsGripvAttached())
			WeaponAttach().CreateAttach(m_sGripvAttachSection, m_weapon_attaches);
		else
			WeaponAttach().RemoveAttach(m_sGripvAttachSection, m_weapon_attaches);
	}

	if (m_sStockAttachSection.size() && pSettings->line_exist(m_sStockAttachSection, "attach_hud_visual"))
	{
		if (IsStockAttached())
			WeaponAttach().CreateAttach(m_sStockAttachSection, m_weapon_attaches);
		else
			WeaponAttach().RemoveAttach(m_sStockAttachSection, m_weapon_attaches);
	}

	if (m_sPistolgripAttachSection.size() && pSettings->line_exist(m_sPistolgripAttachSection, "attach_hud_visual"))
	{
		if (IsPistolgripAttached())
			WeaponAttach().CreateAttach(m_sPistolgripAttachSection, m_weapon_attaches);
		else
			WeaponAttach().RemoveAttach(m_sPistolgripAttachSection, m_weapon_attaches);
	}

	if (m_sHandguardAttachSection.size() && pSettings->line_exist(m_sHandguardAttachSection, "attach_hud_visual"))
	{
		if (IsHandguardAttached())
			WeaponAttach().CreateAttach(m_sHandguardAttachSection, m_weapon_attaches);
		else
			WeaponAttach().RemoveAttach(m_sHandguardAttachSection, m_weapon_attaches);
	}

	if (m_sLaserAttachSection.size() && pSettings->line_exist(m_sLaserAttachSection, "attach_hud_visual"))
	{
		if (IsLaserAttached())
			WeaponAttach().CreateAttach(m_sLaserAttachSection, m_weapon_attaches);
		else
			WeaponAttach().RemoveAttach(m_sLaserAttachSection, m_weapon_attaches);
	}

	if (m_sTacticalTorchAttachSection.size() && pSettings->line_exist(m_sTacticalTorchAttachSection, "attach_hud_visual"))
	{
		if (IsTacticalTorchAttached())
			WeaponAttach().CreateAttach(m_sTacticalTorchAttachSection, m_weapon_attaches);
		else
			WeaponAttach().RemoveAttach(m_sTacticalTorchAttachSection, m_weapon_attaches);
	}

	inherited::InitAddons();
}

void CWeaponMagazined::LoadSilencerKoeffs()
{
	if ( m_eSilencerStatus == ALife::eAddonAttachable )
	{
		LPCSTR sect = m_sSilencerName.c_str();
		m_silencer_koef.hit_power		= READ_IF_EXISTS( pSettings, r_float, sect, "bullet_hit_power_k", 1.0f );
		m_silencer_koef.hit_impulse		= READ_IF_EXISTS( pSettings, r_float, sect, "bullet_hit_impulse_k", 1.0f );
		m_silencer_koef.bullet_speed	= READ_IF_EXISTS( pSettings, r_float, sect, "bullet_speed_k", 1.0f );
		m_silencer_koef.fire_dispersion	= READ_IF_EXISTS( pSettings, r_float, sect, "fire_dispersion_base_k", 1.0f );
		m_silencer_koef.cam_dispersion	= READ_IF_EXISTS( pSettings, r_float, sect, "cam_dispersion_k", 1.0f );
		m_silencer_koef.cam_disper_inc	= READ_IF_EXISTS( pSettings, r_float, sect, "cam_dispersion_inc_k", 1.0f );
	}

	clamp( m_silencer_koef.hit_power,		0.0f, 1.0f );
	clamp( m_silencer_koef.hit_impulse,		0.0f, 1.0f );
	clamp( m_silencer_koef.bullet_speed,	0.0f, 1.0f );
	clamp( m_silencer_koef.fire_dispersion,	0.0f, 3.0f );
	clamp( m_silencer_koef.cam_dispersion,	0.0f, 1.0f );
	clamp( m_silencer_koef.cam_disper_inc,	0.0f, 1.0f );
}

void CWeaponMagazined::ApplySilencerKoeffs()
{
	cur_silencer_koef = m_silencer_koef;
}

void CWeaponMagazined::ResetSilencerKoeffs()
{
	cur_silencer_koef.Reset();
}

void CWeaponMagazined::PlayAnimShow()
{
	VERIFY(GetState()==eShowing);

	if (iAmmoElapsed >= 1)
		m_opened = false;
	else
		m_opened = true;

	HUD_VisualBulletUpdate();
	UpdateCheckWeaponHaveLaser();
	UpdateHudAltAimHud();
	UpdateHudAltAimHudMode2();

	if (IsGripAttached())
	{
		if (IsMisfire() && isHUDAnimationExist("anm_show_grip_h_jammed"))
			PlayHUDMotion("anm_show_grip_h_jammed", false, this, GetState());
		else if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_SHOW_EMPTY))
			PlayHUDMotion("anm_show_grip_h_empty", FALSE, this, GetState());
		else
			PlayHUDMotion("anm_show_grip_h", FALSE, this, GetState());
	}
	else if (IsGripvAttached())
	{
		if (IsMisfire() && isHUDAnimationExist("anm_show_grip_v_jammed"))
			PlayHUDMotion("anm_show_grip_v_jammed", false, this, GetState());
		else if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_SHOW_EMPTY))
			PlayHUDMotion("anm_show_grip_v_empty", FALSE, this, GetState());
		else
			PlayHUDMotion("anm_show_grip_v", FALSE, this, GetState());
	}
	else
	{
		if (IsMisfire() && isHUDAnimationExist("anm_show_jammed"))
			PlayHUDMotion("anm_show_jammed", false, this, GetState());
		else if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_SHOW_EMPTY))
			PlayHUDMotion("anm_show_empty", FALSE, this, GetState());
		else
			PlayHUDMotion("anm_show", FALSE, this, GetState());
	}
}

void CWeaponMagazined::PlayAnimHide()
{
	VERIFY(GetState()==eHiding);
	UpdateCheckWeaponHaveLaser();
	UpdateHudAltAimHud();
	UpdateHudAltAimHudMode2();

	if (IsGripAttached())
	{
		if (IsMisfire() && isHUDAnimationExist("anm_hide_grip_h_jammed"))
			PlayHUDMotion("anm_hide_grip_h_jammed", true, this, GetState());
		else if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_HIDE_EMPTY))
			PlayHUDMotion("anm_hide_grip_h_empty", TRUE, this, GetState());
		else
			PlayHUDMotion("anm_hide_grip_h", TRUE, this, GetState());
	}
	else if (IsGripvAttached())
	{
		if (IsMisfire() && isHUDAnimationExist("anm_hide_grip_v_jammed"))
			PlayHUDMotion("anm_hide_grip_v_jammed", true, this, GetState());
		else if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_HIDE_EMPTY))
			PlayHUDMotion("anm_hide_grip_v_empty", TRUE, this, GetState());
		else
			PlayHUDMotion("anm_hide_grip_v", TRUE, this, GetState());
	}
	else
	{
		if (IsMisfire() && isHUDAnimationExist("anm_hide_jammed"))
			PlayHUDMotion("anm_hide_jammed", true, this, GetState());
		else if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_HIDE_EMPTY))
			PlayHUDMotion("anm_hide_empty", TRUE, this, GetState());
		else
			PlayHUDMotion("anm_hide", TRUE, this, GetState());
	}

}

void CWeaponMagazined::PlayAnimBore()
{
	inherited::PlayAnimBore();
}

void CWeaponMagazined::PlayAnimIdleSprint()
{
	if (g_actor->IsDetectorActive())
	{
		if (IsMisfireNow())
			PlayHUDMotionIfExists({ "anm_idle_sprint_jammed", "anm_idle_sprint", "anm_idle" }, true, GetState());
		else
			PlayHUDMotionIfExists({ "anm_idle_sprint", "anm_idle" }, true, GetState());
	}
	else
	{
		if (IsGripAttached())
		{
			if (IsMisfire() && isHUDAnimationExist("anm_idle_sprint_grip_h_jammed"))
				PlayHUDMotion("anm_idle_sprint_grip_h_jammed", true, nullptr, GetState());
			else if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_SPRINT_EMPTY))
				PlayHUDMotion("anm_idle_sprint_grip_h_empty", TRUE, NULL, GetState());
			else
				inherited::PlayAnimIdleSprint();
		}
		else if (IsGripvAttached())
		{
			if (IsMisfire() && isHUDAnimationExist("anm_idle_sprint_grip_v_jammed"))
				PlayHUDMotion("anm_idle_sprint_grip_v_jammed", true, nullptr, GetState());
			else if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_SPRINT_EMPTY))
				PlayHUDMotion("anm_idle_sprint_grip_v_empty", TRUE, NULL, GetState());
			else
				inherited::PlayAnimIdleSprint();
		}
		else
		{
			if (IsMisfire() && isHUDAnimationExist("anm_idle_sprint_jammed"))
				PlayHUDMotion("anm_idle_sprint_jammed", true, nullptr, GetState());
			else if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_SPRINT_EMPTY))
				PlayHUDMotion("anm_idle_sprint_empty", TRUE, NULL, GetState());
			else
				inherited::PlayAnimIdleSprint();
		}
	}
}

void CWeaponMagazined::PlayAnimIdleMoving()
{
	if (g_actor->IsDetectorActive())
	{
			if (IsMisfireNow())
				PlayHUDMotionIfExists({ "anm_idle_moving_jammed", "anm_idle_moving", "anm_idle" }, true, GetState());
			else
				PlayHUDMotionIfExists({ "anm_idle_moving", "anm_idle" }, true, GetState());
	}
	else 
	{
		if (IsGripAttached())
		{
			if (IsMisfire() && isHUDAnimationExist("anm_idle_moving_grip_h_jammed"))
				PlayHUDMotion("anm_idle_moving_grip_h_jammed", true, nullptr, GetState());
			else if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_MOVING_EMPTY))
				PlayHUDMotion("anm_idle_moving_grip_h_empty", TRUE, NULL, GetState());
			else
				inherited::PlayAnimIdleMoving();
		}
		else if (IsGripvAttached())
		{
			if (IsMisfire() && isHUDAnimationExist("anm_idle_moving_grip_v_jammed"))
				PlayHUDMotion("anm_idle_moving_grip_v_jammed", true, nullptr, GetState());
			else if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_MOVING_EMPTY))
				PlayHUDMotion("anm_idle_moving_grip_v_empty", TRUE, NULL, GetState());
			else
				inherited::PlayAnimIdleMoving();
		}
		else
		{
			if (IsMisfire() && isHUDAnimationExist("anm_idle_moving_jammed"))
				PlayHUDMotion("anm_idle_moving_jammed", true, nullptr, GetState());
			else if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_MOVING_EMPTY))
				PlayHUDMotion("anm_idle_moving_empty", TRUE, NULL, GetState());
			else
				inherited::PlayAnimIdleMoving();
		}
	}
}

void CWeaponMagazined::PlayAnimReload()
{
	VERIFY(GetState()==eReload);

	if (m_bIsRevolver)
	{
		u8 bullets_to_load = iMagazineSize - GetAvailableCartridgesToLoad(!iAmmoElapsed);

		string128 anmReloadName;
		strconcat(sizeof(anmReloadName), anmReloadName, "anm_reload_", (bullets_to_load > 0) ? std::to_string(bullets_to_load).c_str() : "empty");

		if (isHUDAnimationExist(anmReloadName))
			PlayHUDMotionIfExists({ anmReloadName, "anm_reload" }, true, GetState());
	}
	else
	{
		if (IsGripAttached())
		{
			//Msg("RELOAD ANIMATION: GRIP");
			if (iAmmoElapsed == 0)
				PlayHUDMotionIfExists({ "anm_reload_grip_h_empty", "anm_reload_grip_h" }, true, GetState());
			else
				PlayHUDMotion("anm_reload_grip_h", TRUE, this, GetState());
		}
		else if (IsGripvAttached())
		{
			//Msg("RELOAD ANIMATION: GRIP");
			if (iAmmoElapsed == 0)
				PlayHUDMotionIfExists({ "anm_reload_grip_v_empty", "anm_reload_grip_v" }, true, GetState());
			else
				PlayHUDMotion("anm_reload_grip_v", TRUE, this, GetState());
		}
		else
		{
			//Msg("RELOAD ANIMATION: NO GRIP");
			if (iAmmoElapsed == 0)
				PlayHUDMotionIfExists({ "anm_reload_empty", "anm_reload" }, true, GetState());
			else
				PlayHUDMotion("anm_reload", TRUE, this, GetState());
		}
	}
}

void CWeaponMagazined::PlayAnimAim()
{
	if (m_sounds.FindSoundItem("sndSprintStart", false))
		m_sounds.StopSound("sndSprintIdle");

	auto det = smart_cast<CCustomDetector*>(g_actor->inventory().ItemFromSlot(DETECTOR_SLOT));

	if (IsRotatingToZoom())
	{
		if (g_actor->IsDetectorActive())
			det->PlayDetectorAnimation(true, eDetAction, "anm_idle_aim_start");

		string32 guns_aim_start_anm;
		strconcat(sizeof(guns_aim_start_anm), guns_aim_start_anm, "anm_idle_aim_start", (IsMisfire() ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : ""));

		if (isHUDAnimationExist(guns_aim_start_anm))
		{
			PlayHUDMotionNew(guns_aim_start_anm, true, GetState());
			return;
		}
		else if (guns_aim_start_anm && strstr(guns_aim_start_anm, "_jammed"))
		{
			char new_guns_aim_start_anm[256];
			strcpy(new_guns_aim_start_anm, guns_aim_start_anm);
			new_guns_aim_start_anm[strlen(guns_aim_start_anm) - strlen("_jammed")] = '\0';

			if (isHUDAnimationExist(new_guns_aim_start_anm))
			{
				PlayHUDMotionNew(new_guns_aim_start_anm, true, GetState());
				return;
			}
		}
		else if (guns_aim_start_anm && strstr(guns_aim_start_anm, "_empty"))
		{
			char new_guns_aim_start_anm[256];
			strcpy(new_guns_aim_start_anm, guns_aim_start_anm);
			new_guns_aim_start_anm[strlen(guns_aim_start_anm) - strlen("_empty")] = '\0';

			if (isHUDAnimationExist(new_guns_aim_start_anm))
			{
				PlayHUDMotionNew(new_guns_aim_start_anm, true, GetState());
				return;
			}
		}
	}

	if (const char* guns_aim_anm = GetAnimAimName())
	{
		if (isHUDAnimationExist(guns_aim_anm))
		{
			PlayHUDMotionNew(guns_aim_anm, true, GetState());

			if (g_actor->IsDetectorActive())
				det->PlayDetectorAnimation(true, eDetAction, GenerateAimAnimName("anm_idle_aim_moving"));

			return;
		}
		else if (guns_aim_anm && strstr(guns_aim_anm, "_jammed"))
		{
			char new_guns_aim_anm[256];
			strcpy(new_guns_aim_anm, guns_aim_anm);
			new_guns_aim_anm[strlen(guns_aim_anm) - strlen("_jammed")] = '\0';

			if (isHUDAnimationExist(new_guns_aim_anm))
			{
				PlayHUDMotionNew(new_guns_aim_anm, true, GetState());

				if (g_actor->IsDetectorActive())
					det->PlayDetectorAnimation(true, eDetAction, GenerateAimAnimName("anm_idle_aim_moving"));

				return;
			}
		}
		else if (guns_aim_anm && strstr(guns_aim_anm, "_empty"))
		{
			char new_guns_aim_anm[256];
			strcpy(new_guns_aim_anm, guns_aim_anm);
			new_guns_aim_anm[strlen(guns_aim_anm) - strlen("_empty")] = '\0';

			if (isHUDAnimationExist(new_guns_aim_anm))
			{
				PlayHUDMotionNew(new_guns_aim_anm, true, GetState());

				if (g_actor->IsDetectorActive())
					det->PlayDetectorAnimation(true, eDetAction, GenerateAimAnimName("anm_idle_aim_moving"));

				return;
			}
		}
	}

	if (IsMisfire() && isHUDAnimationExist("anm_idle_aim_jammed"))
		PlayHUDMotion("anm_idle_aim_jammed", true, nullptr, GetState());
	else if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_AIM_EMPTY))
		PlayHUDMotion("anm_idle_aim_empty", TRUE, NULL, GetState());
	else
		PlayHUDMotion("anm_idle_aim", TRUE, NULL, GetState());

	if (g_actor->IsDetectorActive())
		det->PlayDetectorAnimation(true, eDetAction, GenerateAimAnimName("anm_idle_aim"));
}

bool CWeaponMagazined::PlayAnimAimEnd()
{
	if (auto det = smart_cast<CCustomDetector*>(g_actor->inventory().ItemFromSlot(DETECTOR_SLOT)); g_actor->IsDetectorActive())
		det->PlayDetectorAnimation(true, eDetAction, "anm_idle_aim_end");

	string32 guns_aim_end_anm;
	strconcat(sizeof(guns_aim_end_anm), guns_aim_end_anm, "anm_idle_aim_end", (IsMisfire() ? "_jammed" : (IsMagazineEmpty()) ? "_empty" : ""));

	if (isHUDAnimationExist(guns_aim_end_anm))
	{
		PlayHUDMotionNew(guns_aim_end_anm, true, GetState());
		return true;
	}
	else if (guns_aim_end_anm && strstr(guns_aim_end_anm, "_jammed"))
	{
		char new_guns_aim_end_anm[256];
		strcpy(new_guns_aim_end_anm, guns_aim_end_anm);
		new_guns_aim_end_anm[strlen(guns_aim_end_anm) - strlen("_jammed")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_end_anm))
		{
			PlayHUDMotionNew(new_guns_aim_end_anm, true, GetState());
			return true;
		}

		return false;
	}
	else if (guns_aim_end_anm && strstr(guns_aim_end_anm, "_empty"))
	{
		char new_guns_aim_end_anm[256];
		strcpy(new_guns_aim_end_anm, guns_aim_end_anm);
		new_guns_aim_end_anm[strlen(guns_aim_end_anm) - strlen("_empty")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_end_anm))
		{
			PlayHUDMotionNew(new_guns_aim_end_anm, true, GetState());
			return true;
		}

		return false;
	}

	return false;
}

void CWeaponMagazined::PlayAnimIdle()
{
	if (GetState() != eIdle)
		return;

	if (IsZoomed())
	{
		UpdateWeaponDoF();

		if (IsGripAttached())
		{
			//Msg("TEST AIM GRIP H");
			if (IsMisfire() && isHUDAnimationExist("anm_idle_aim_jammed_grip_h"))
				PlayHUDMotion("anm_idle_aim_jammed_grip_h", true, nullptr, GetState());
			else if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_AIM_EMPTY))
				PlayHUDMotion("anm_idle_aim_empty_grip_h", TRUE, NULL, GetState());
			else
				PlayHUDMotion("anm_idle_aim_grip_h", TRUE, NULL, GetState());
		}
		else if(IsGripvAttached())
		{
			//Msg("TEST AIM GRIP V");
			if (IsMisfire() && isHUDAnimationExist("anm_idle_aim_jammed_grip_v"))
				PlayHUDMotion("anm_idle_aim_jammed_grip_v", true, nullptr, GetState());
			else if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_AIM_EMPTY))
				PlayHUDMotion("anm_idle_aim_empty_grip_v", TRUE, NULL, GetState());
			else
				PlayHUDMotion("anm_idle_aim_grip_v", TRUE, NULL, GetState());
		}
		else if (IsGrenadeLauncherAttached())
		{
			//Msg("TEST AIM GL");
			if (IsMisfire() && isHUDAnimationExist("anm_idle_w_gl_aim_jammed"))
				PlayHUDMotion("anm_idle_w_gl_aim_jammed", true, nullptr, GetState());
			else if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_AIM_EMPTY))
				PlayHUDMotion("anm_idle_w_gl_aim_empty", TRUE, NULL, GetState());
			else
				PlayHUDMotion("anm_idle_w_gl_aim", TRUE, NULL, GetState());
		}
		else
		{
		//	Msg("TEST AIM NO GRIP");
			PlayAnimAim();
		}
	}
	else if (IsMisfire() && isHUDAnimationExist("anm_idle_jammed") && !TryPlayAnimIdle() && !IsRotatingFromZoom())
	{
		if (IsGripAttached())
		{
			PlayHUDMotion("anm_idle_jammed_grip_h", true, nullptr, GetState());
		}
		else if (IsGripvAttached())
		{
			PlayHUDMotion("anm_idle_jammed_grip_v", true, nullptr, GetState());
		}
		else
		{
			PlayHUDMotion("anm_idle_jammed", true, nullptr, GetState());
		}
	}
	else if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_IDLE_EMPTY) && !TryPlayAnimIdle() && !IsRotatingFromZoom())
	{
		if (IsGripAttached())
		{
			PlayHUDMotion("anm_idle_empty_grip_h", TRUE, NULL, GetState());
		}
		else if (IsGripvAttached())
		{
			PlayHUDMotion("anm_idle_empty_grip_v", TRUE, NULL, GetState());
		}
		else
		{
			PlayHUDMotion("anm_idle_empty", TRUE, NULL, GetState());
		}
	}
	else
	{
		if (IsRotatingFromZoom())
		{
			if (PlayAnimAimEnd())
				return;
		}
		inherited::PlayAnimIdle();
	}
}

void CWeaponMagazined::PlayAnimShoot()
{
	VERIFY(GetState()==eFire);

	if ((IsRotatingToZoom() && m_zoom_params.m_fZoomRotationFactor != 0.0f) || (IsRotatingFromZoom() && m_zoom_params.m_fZoomRotationFactor != 1.0f))
		return;

	string256 guns_det_shoot_anm{};
	strconcat(sizeof(guns_det_shoot_anm), guns_det_shoot_anm, "anm_shots", (IsZoomed() && !IsRotatingToZoom()) ? "_aim" : "");

	if (auto det = smart_cast<CCustomDetector*>(g_actor->inventory().ItemFromSlot(DETECTOR_SLOT)); g_actor->IsDetectorActive())
		det->PlayDetectorAnimation(true, eDetAction, guns_det_shoot_anm);

	string_path guns_shoot_anm{};
	strconcat(sizeof(guns_shoot_anm), guns_shoot_anm, (isHUDAnimationExist("anm_shots") ? "anm_shots" : ""), (IsZoomed() && !IsRotatingToZoom()) ? (IsScopeAttached() && m_bUseAimScopeAnims ? "_aim_scope" : "_aim") : "", (iAmmoElapsed == 1) ? "_last" : "", (IsMisfire() ? "_jammed" : IsMagazineEmpty() ? "_empty" : ""), (IsSilencerAttached() && m_bUseSilShotAnim) ? "_sil" : "");

	//HUD_VisualBulletUpdate();

	//Msg("SHOOTING WITHout ATTACHED GRIP");
	if (iAmmoElapsed == 1)
		PlayHUDMotionIfExists({ guns_shoot_anm, "anm_shots_last", "anm_shots" }, false, GetState());
	else
		PlayHUDMotionIfExists({ guns_shoot_anm, "anm_shots" }, false, GetState());
}


void CWeaponMagazined::PlayAnimShootHorGrip()
{
	VERIFY(GetState() == eFire);

	if ((IsRotatingToZoom() && m_zoom_params.m_fZoomRotationFactor != 0.0f) || (IsRotatingFromZoom() && m_zoom_params.m_fZoomRotationFactor != 1.0f))
		return;

	string256 guns_det_shoot_anm{};
	strconcat(sizeof(guns_det_shoot_anm), guns_det_shoot_anm, "anm_shots_grip_h", (IsZoomed() && !IsRotatingToZoom()) ? "_aim" : "");

	if (auto det = smart_cast<CCustomDetector*>(g_actor->inventory().ItemFromSlot(DETECTOR_SLOT)); g_actor->IsDetectorActive())
		det->PlayDetectorAnimation(true, eDetAction, guns_det_shoot_anm);

	string_path guns_shoot_anm{};
	strconcat(sizeof(guns_shoot_anm), guns_shoot_anm, (isHUDAnimationExist("anm_shots_grip_h") ? "anm_shots_grip_h" : ""), (IsZoomed() && !IsRotatingToZoom()) ? (IsScopeAttached() && m_bUseAimScopeAnims ? "_aim_scope" : "_aim") : "", (iAmmoElapsed == 1) ? "_last" : "", (IsMisfire() ? "_jammed" : IsMagazineEmpty() ? "_empty" : ""), (IsSilencerAttached() && m_bUseSilShotAnim) ? "_sil" : "");

	//HUD_VisualBulletUpdate();

	//Msg("SHOOTING WITH ATTACHED GRIP");
	if (iAmmoElapsed == 1)
		PlayHUDMotionIfExists({ guns_shoot_anm, "anm_shots_grip_h_last", "anm_shots_grip_h" }, false, GetState());
	else
		PlayHUDMotionIfExists({ guns_shoot_anm, "anm_shots_grip_h" }, false, GetState());
}


void CWeaponMagazined::PlayAnimShootVerGrip()
{
	VERIFY(GetState() == eFire);

	if ((IsRotatingToZoom() && m_zoom_params.m_fZoomRotationFactor != 0.0f) || (IsRotatingFromZoom() && m_zoom_params.m_fZoomRotationFactor != 1.0f))
		return;

	string256 guns_det_shoot_anm{};
	strconcat(sizeof(guns_det_shoot_anm), guns_det_shoot_anm, "anm_shots_grip_v", (IsZoomed() && !IsRotatingToZoom()) ? "_aim" : "");

	if (auto det = smart_cast<CCustomDetector*>(g_actor->inventory().ItemFromSlot(DETECTOR_SLOT)); g_actor->IsDetectorActive())
		det->PlayDetectorAnimation(true, eDetAction, guns_det_shoot_anm);

	string_path guns_shoot_anm{};
	strconcat(sizeof(guns_shoot_anm), guns_shoot_anm, (isHUDAnimationExist("anm_shots_grip_v") ? "anm_shots_grip_v" : ""), (IsZoomed() && !IsRotatingToZoom()) ? (IsScopeAttached() && m_bUseAimScopeAnims ? "_aim_scope" : "_aim") : "", (iAmmoElapsed == 1) ? "_last" : "", (IsMisfire() ? "_jammed" : IsMagazineEmpty() ? "_empty" : ""), (IsSilencerAttached() && m_bUseSilShotAnim) ? "_sil" : "");

	//HUD_VisualBulletUpdate();

	//Msg("SHOOTING WITH ATTACHED GRIP v");
	if (iAmmoElapsed == 1)
		PlayHUDMotionIfExists({ guns_shoot_anm, "anm_shots_grip_v_last", "anm_shots_grip_v" }, false, GetState());
	else
		PlayHUDMotionIfExists({ guns_shoot_anm, "anm_shots_grip_v" }, false, GetState());

}

bool CWeaponMagazined::PlayAnimFakeShoot()
{
	if ((IsRotatingToZoom() && m_zoom_params.m_fZoomRotationFactor != 0.0f) || (IsRotatingFromZoom() && m_zoom_params.m_fZoomRotationFactor != 1.0f))
		return false;

	string256 guns_det_shoot_anm{};
	strconcat(sizeof(guns_det_shoot_anm), guns_det_shoot_anm, "anm_fakeshoot", (IsZoomed() && !IsRotatingToZoom()) ? "_aim" : "");

	if (auto det = smart_cast<CCustomDetector*>(g_actor->inventory().ItemFromSlot(DETECTOR_SLOT)); g_actor->IsDetectorActive())
		det->PlayDetectorAnimation(true, eDetAction, guns_det_shoot_anm);

	string128 guns_fakeshoot_anm{};
	strconcat(sizeof(guns_fakeshoot_anm), guns_fakeshoot_anm, ("anm_fakeshoot"), (IsZoomed() && !IsRotatingToZoom()) ? "_aim" : "", IsMisfire() ? "_jammed" : IsEmptyMagazine() ? "_empty" : "", IsGrenadeLauncherAttached() ? (!IsGrenadeMode() ? "_w_gl" : "_g") : "");

	if (isHUDAnimationExist(guns_fakeshoot_anm))
	{
		SetPending(TRUE);
		PlayHUDMotionNew(guns_fakeshoot_anm, true, GetState());
		return true;
	}
	else if (guns_fakeshoot_anm && strstr(guns_fakeshoot_anm, "_jammed"))
	{
		char new_guns_fakeshoot_anm[256];
		strcpy(new_guns_fakeshoot_anm, guns_fakeshoot_anm);
		new_guns_fakeshoot_anm[strlen(guns_fakeshoot_anm) - strlen("_jammed")] = '\0';

		if (isHUDAnimationExist(new_guns_fakeshoot_anm))
		{
			SetPending(TRUE);
			PlayHUDMotionNew(new_guns_fakeshoot_anm, true, GetState());
			return true;
		}
	}
	else if (guns_fakeshoot_anm && strstr(guns_fakeshoot_anm, "_empty"))
	{
		char new_guns_fakeshoot_anm[256];
		strcpy(new_guns_fakeshoot_anm, guns_fakeshoot_anm);
		new_guns_fakeshoot_anm[strlen(guns_fakeshoot_anm) - strlen("_empty")] = '\0';

		if (isHUDAnimationExist(new_guns_fakeshoot_anm))
		{
			SetPending(TRUE);
			PlayHUDMotionNew(new_guns_fakeshoot_anm, true, GetState());
			return true;
		}
	}

	return false;
}

void CWeaponMagazined::PlayAnimDeviceSwitch()
{
	CActor* actor = Actor();
	CTorch* torch = smart_cast<CTorch*>(Actor()->inventory().ItemFromSlot(TORCH_SLOT));
	CCustomDetector* det = smart_cast<CCustomDetector*>(g_actor->inventory().ItemFromSlot(DETECTOR_SLOT));

	if (!actor->GetNightVision())
		actor->SetNightVision(xr_new<CNightVisionEffector>(actor->cNameSect()));

	CNightVisionEffector* nvg = Actor()->GetNightVision();

	PlaySound(HeadLampSwitch && torch ? (!torch->IsSwitchedOn() ? "sndHeadlampOn" : "sndHeadlampOff") : NightVisionSwitch && nvg ? (!nvg->IsActive() ? "sndNvOn" : "sndNvOff") : CleanMaskAction ? "sndCleanMask" : "", get_LastFP());

	string128 guns_device_switch_anm{}, guns_device_switch_det_anm{};
	strconcat(sizeof(guns_device_switch_anm), guns_device_switch_anm, HeadLampSwitch && torch ? (!torch->IsSwitchedOn() ? "anm_headlamp_on" : "anm_headlamp_off") : NightVisionSwitch && nvg ? (!nvg->IsActive() ? "anm_nv_on" : "anm_nv_off") : CleanMaskAction ? "anm_clean_mask" : "", IsMisfire() ? "_jammed" : IsEmptyMagazine() ? "_empty" : "", IsGrenadeLauncherAttached() ? (!IsGrenadeMode() ? "_w_gl" : "_g") : "");
	strconcat(sizeof(guns_device_switch_det_anm), guns_device_switch_det_anm, HeadLampSwitch && torch ? (!torch->IsSwitchedOn() ? "anm_headlamp_on" : "anm_headlamp_off") : NightVisionSwitch && nvg ? (!nvg->IsActive() ? "anm_nv_on" : "anm_nv_off") : CleanMaskAction ? "anm_clean_mask" : "", "");

	if (isHUDAnimationExist(guns_device_switch_anm))
	{
		if (CleanMaskAction)
		{
			actor->SetMaskAnimLength(Device.dwTimeGlobal + PlayHUDMotionNew(guns_device_switch_anm, true, GetState()));
			actor->SetMaskAnimActive(true);
			actor->SetActionAnimInProcess(true);
		}
		else
			PlayHUDMotionNew(guns_device_switch_anm, true, GetState());
	}
	else if (guns_device_switch_anm && strstr(guns_device_switch_anm, "_jammed"))
	{
		char new_guns_device_switch_anm[256];
		strcpy(new_guns_device_switch_anm, guns_device_switch_anm);
		new_guns_device_switch_anm[strlen(guns_device_switch_anm) - strlen("_jammed")] = '\0';

		if (isHUDAnimationExist(new_guns_device_switch_anm))
		{
			if (CleanMaskAction)
			{
				actor->SetMaskAnimLength(Device.dwTimeGlobal + PlayHUDMotionNew(new_guns_device_switch_anm, true, GetState()));
				actor->SetMaskAnimActive(true);
				actor->SetActionAnimInProcess(true);
			}
			else
				PlayHUDMotionNew(new_guns_device_switch_anm, true, GetState());
		}
	}
	else if (guns_device_switch_anm && strstr(guns_device_switch_anm, "_empty"))
	{
		char new_guns_device_switch_anm[256];
		strcpy(new_guns_device_switch_anm, guns_device_switch_anm);
		new_guns_device_switch_anm[strlen(guns_device_switch_anm) - strlen("_empty")] = '\0';

		if (isHUDAnimationExist(new_guns_device_switch_anm))
		{
			if (CleanMaskAction)
			{
				actor->SetMaskAnimLength(Device.dwTimeGlobal + PlayHUDMotionNew(new_guns_device_switch_anm, true, GetState()));
				actor->SetMaskAnimActive(true);
				actor->SetActionAnimInProcess(true);
			}
			else
				PlayHUDMotionNew(new_guns_device_switch_anm, true, GetState());
		}
	}
	else
	{
		DeviceUpdate();
		SwitchState(eIdle);
		return;
	}

	if (g_actor->IsDetectorActive())
		det->PlayDetectorAnimation(true, eDetAction, guns_device_switch_det_anm);
}

void CWeaponMagazined::OnZoomIn			()
{
	inherited::OnZoomIn();

	shared_str cur_scope_sect = (m_sScopeAttachSection.size() ? m_sScopeAttachSection : (m_eScopeStatus == ALife::eAddonAttachable) ? m_scopes[m_cur_scope].c_str() : "scope");

	//UpdateAltAimZoomFactor2();

	bool UseScopeAimScript = false;
	if (pSettings->line_exist(cur_scope_sect, "scope_use_aim_effect"))
		UseScopeAimScript = READ_IF_EXISTS(pSettings, r_bool, cur_scope_sect, "scope_use_aim_effect", false);

	bool UseScopeNVGScript = false;
	if (pSettings->line_exist(cur_scope_sect, "scope_use_nightvision"))
		UseScopeNVGScript = READ_IF_EXISTS(pSettings, r_bool, cur_scope_sect, "scope_use_nightvision", false);


	if(GetState() == eIdle && !IsPending())
		PlayAnimIdle();

	luabind::functor<void> funct;

	if (psActorFlags2.test(AF_LFO_WEAPON_AIM_CAM_EFF))
	{
		if (ai().script_engine().functor("lfo_weapons.on_actor_aiming", funct))
			funct();

		if (UseScopeAimScript)
		{
			if (ai().script_engine().functor("lfo_weapons.on_actor_aiming_sniper", funct))
				funct();
		}
	}

	if (lfo_scope_type != 3)
	{
		if (UseScopeNVGScript)
		{
			if (ai().script_engine().functor("lfo_weapons.on_actor_scope_nvg_on", funct))
				funct();
		}
	}

	//Alundaio: callback not sure why vs2013 gives error, it's fine
	CGameObject* object = smart_cast<CGameObject*>(H_Parent());

	if (object)
		object->callback(GameObject::eOnWeaponZoomIn)(object->lua_game_object(), this->lua_game_object());
	//-Alundaio

	CActor* pActor = smart_cast<CActor*>(H_Parent());
	if(pActor)
	{
		if (m_sounds.FindSoundItem("sndZoomIn", false))
			m_sounds.PlaySound("sndZoomIn", get_LastFP(), H_Root(), !!GetHUDmode(), false, (u8)-1);

		CEffectorZoomInertion* S = smart_cast<CEffectorZoomInertion*>	(pActor->Cameras().GetCamEffector(eCEZoom));
		if (!S)	
		{
			S = (CEffectorZoomInertion*)pActor->Cameras().AddCamEffector(xr_new<CEffectorZoomInertion> ());
			S->Init(this);
		};
		S->SetRndSeed			(pActor->GetZoomRndSeed());
		R_ASSERT				(S);
	}
}
void CWeaponMagazined::OnZoomOut		()
{
	if(!IsZoomed())
		return;

	inherited::OnZoomOut	();

	bool UseScopeAimScript = true;
	bool UseScopeNVGScript = true;

	if (GetState() == eFire && !IsPending())
		PlayAnimAimEnd		();

	if(GetState() == eIdle && !IsPending())
		PlayAnimIdle		();

	luabind::functor<void> funct;

	if (psActorFlags2.test(AF_LFO_WEAPON_AIM_CAM_EFF))
	{
		if (ai().script_engine().functor("lfo_weapons.on_actor_not_aiming", funct))
			funct();

		if (UseScopeAimScript)
		{
			if (ai().script_engine().functor("lfo_weapons.on_actor_not_aiming_sniper", funct))
				funct();
		}
	}

	if (lfo_scope_type != 3)
	{
		if (UseScopeNVGScript)
		{
			if (ai().script_engine().functor("lfo_weapons.on_actor_scope_nvg_off", funct))
				funct();
		}
	}


	//Alundaio
	CGameObject* object = smart_cast<CGameObject*>(H_Parent());
	if (object)
		object->callback(GameObject::eOnWeaponZoomOut)(object->lua_game_object(), this->lua_game_object());
	//-Alundaio

	CActor* pActor			= smart_cast<CActor*>(H_Parent());

	if (pActor)
	{
		if (m_sounds.FindSoundItem("sndZoomOut", false))
			m_sounds.PlaySound("sndZoomOut", get_LastFP(), H_Root(), !!GetHUDmode(), false, (u8)-1);

		pActor->Cameras().RemoveCamEffector(eCEZoom);
	}

}

//ïåðåêëþ÷åíèå ðåæèìîâ ñòðåëüáû îäèíî÷íûìè è î÷åðåäÿìè
bool CWeaponMagazined::SwitchMode			()
{
	if(eIdle != GetState() || IsPending()) return false;

	if(SingleShotMode())
		m_iQueueSize = WEAPON_ININITE_QUEUE;
	else
		m_iQueueSize = 1;
	
	PlaySound	("sndEmptyClick", get_LastFP());

	return true;
}
 
void	CWeaponMagazined::OnNextFireMode		()
{
	if (!m_bHasDifferentFireModes) return;

	if (IsGripAttached())
	{
		if (isHUDAnimationExist("anm_changefiremode_grip_h_from_1_to_a") || isHUDAnimationExist("anm_changefiremode_grip_h"))
			SwitchState(eFiremodeNext);
	}
	else if (IsGripvAttached())
	{
		if (isHUDAnimationExist("anm_changefiremode_grip_v_from_1_to_a") || isHUDAnimationExist("anm_changefiremode_grip_v"))
			SwitchState(eFiremodeNext);
	}
	else
	{
		if (isHUDAnimationExist("anm_changefiremode_from_1_to_a") || isHUDAnimationExist("anm_changefiremode"))
			SwitchState(eFiremodeNext);
	}

	if (GetState() != eIdle) return;
	m_iCurFireMode = (m_iCurFireMode+1+m_aFireModes.size()) % m_aFireModes.size();
	SetQueueSize(GetCurrentFireMode());
};

void	CWeaponMagazined::OnPrevFireMode		()
{
	if (!m_bHasDifferentFireModes) return;

	if (IsGripAttached())
	{
		if (isHUDAnimationExist("anm_changefiremode_grip_h_from_1_to_a") || isHUDAnimationExist("anm_changefiremode_grip_h"))
			SwitchState(eFiremodePrev);
	}
	else if (IsGripvAttached())
	{
		if (isHUDAnimationExist("anm_changefiremode_grip_v_from_1_to_a") || isHUDAnimationExist("anm_changefiremode_grip_v"))
			SwitchState(eFiremodePrev);
	}
	else
	{
		if (isHUDAnimationExist("anm_changefiremode_from_1_to_a") || isHUDAnimationExist("anm_changefiremode"))
			SwitchState(eFiremodePrev);
	}

	if (GetState() != eIdle) return;
	m_iCurFireMode = (m_iCurFireMode-1+m_aFireModes.size()) % m_aFireModes.size();
	SetQueueSize(GetCurrentFireMode());	
};

void	CWeaponMagazined::OnH_A_Chield		()
{
	if (m_bHasDifferentFireModes)
	{
		CActor	*actor = smart_cast<CActor*>(H_Parent());
		if (!actor) SetQueueSize(-1);
		else SetQueueSize(GetCurrentFireMode());
	};	
	inherited::OnH_A_Chield();
};

void	CWeaponMagazined::SetQueueSize			(int size)  
{
	m_iQueueSize = size; 
};

float	CWeaponMagazined::GetWeaponDeterioration	()
{
// modified by Peacemaker [17.10.08]
//	if (!m_bHasDifferentFireModes || m_iPrefferedFireMode == -1 || u32(GetCurrentFireMode()) <= u32(m_iPrefferedFireMode)) 
//		return inherited::GetWeaponDeterioration();
//	return m_iShotNum*conditionDecreasePerShot;
	return (m_iShotNum==1) ? conditionDecreasePerShot : conditionDecreasePerQueueShot;
};

void CWeaponMagazined::save(NET_Packet &output_packet)
{
	inherited::save	(output_packet);
	save_data		(m_iQueueSize, output_packet);
	save_data		(m_iShotNum, output_packet);
	save_data		(m_iCurFireMode, output_packet);
}

void CWeaponMagazined::load(IReader &input_packet)
{
	inherited::load	(input_packet);
	load_data		(m_iQueueSize, input_packet);SetQueueSize(m_iQueueSize);
	load_data		(m_iShotNum, input_packet);
	load_data		(m_iCurFireMode, input_packet);
}

void CWeaponMagazined::net_Export	(NET_Packet& P)
{
	inherited::net_Export (P);

	P.w_u8(u8(m_iCurFireMode&0x00ff));
}

void CWeaponMagazined::net_Import	(NET_Packet& P)
{
	inherited::net_Import (P);

	m_iCurFireMode = P.r_u8();
	SetQueueSize(GetCurrentFireMode());
}

#include "string_table.h"
bool CWeaponMagazined::GetBriefInfo(II_BriefInfo& info)
{
	VERIFY(m_pInventory);
	string32	int_str, fire_mode, ammo = "";

	int	ae = GetAmmoElapsed();
	xr_sprintf(int_str, "%d", ae);
	info.cur_ammo = int_str;
	info.fire_mode._set("");

	if (bHasBulletsToHide)
	{
		last_hide_bullet = ae >= bullet_cnt ? bullet_cnt : bullet_cnt - ae - 1;

		if (ae == 0) last_hide_bullet = -1;

		//HUD_VisualBulletUpdate();
	}

	if (HasFireModes())
	{
		if (m_iQueueSize == WEAPON_ININITE_QUEUE)
		{
			info.fire_mode = "A";
		}
		else
		{
			xr_sprintf(fire_mode, "%d", m_iQueueSize);
			info.fire_mode = fire_mode;
		}
	}
	else
		info.fire_mode = "";

	if (m_pInventory->ModifyFrame() <= m_dwAmmoCurrentCalcFrame)
	{
		return false;
	}
	GetSuitableAmmoTotal();//update m_BriefInfo_CalcFrame
	info.grenade = "";

	u32 at_size = m_ammoTypes.size();
	if (unlimited_ammo() || at_size == 0)
	{
		info.fmj_ammo._set("--");
		info.ap_ammo._set("--");
	}
	else
	{
		//GetSuitableAmmoTotal(); //mp = all type

		// Lex Addon (correct by Suhar_) 28.03.2017		(begin)
		/*int add_ammo_count = 0;

		for (int i = 0; i < at_size; i++)
		{
			if (m_ammoType == i)
			{
				xr_sprintf(int_str, "%d", GetAmmoCount(i));
				info.fmj_ammo = int_str;
			}
			else
			{
				add_ammo_count += GetAmmoCount(i);
			}
		}
		if (at_size > 1)
			xr_sprintf(int_str, "%d", add_ammo_count);
		else
			xr_sprintf(int_str, "%s", "");

		info.ap_ammo = int_str;*/


		info.fmj_ammo._set("");
		info.ap_ammo._set("");

		if (at_size >= 1 && at_size < 3)
		{
			xr_sprintf(ammo, "%d", GetAmmoCount(0));
			info.fmj_ammo._set(ammo);
		}
		if (at_size == 2)
		{
			xr_sprintf(ammo, "%d", GetAmmoCount(1));
			info.ap_ammo._set(ammo);
		}
		if (at_size >= 3)
		{
			xr_sprintf(ammo, "%d", GetAmmoCount(m_ammoType));
			info.fmj_ammo._set(ammo);
			u8 m = 0;
			u64 ap = 0;
			while (m < at_size)
			{
				if (m != m_ammoType)
					ap += GetAmmoCount(m);
				m++;
			}
			xr_sprintf(ammo, "%d", ap);
			info.ap_ammo._set(ammo);
		}

		// Lex Addon (correct by Suhar_) 28.07.2017		(end)
	}

	if (ae != 0 && m_magazine.size() != 0)
	{
		LPCSTR ammo_type = m_ammoTypes[m_magazine.back().m_LocalAmmoType].c_str();
		info.name = CStringTable().translate(pSettings->r_string(ammo_type, "inv_name_short"));
		info.icon = ammo_type;
	}
	else
	{
		LPCSTR ammo_type = m_ammoTypes[m_ammoType].c_str();
		info.name = CStringTable().translate(pSettings->r_string(ammo_type, "inv_name_short"));
		info.icon = ammo_type;
	}
	return true;
}

bool CWeaponMagazined::install_upgrade_impl( LPCSTR section, bool test )
{
	bool result = inherited::install_upgrade_impl( section, test );
	
	LPCSTR str;
	// fire_modes = 1, 2, -1
	bool result2 = process_if_exists_set( section, "fire_modes", &CInifile::r_string, str, test );
	if ( result2 && !test )
	{
		int ModesCount = _GetItemCount( str );
		m_aFireModes.clear();
		for ( int i = 0; i < ModesCount; ++i )
		{
			string16 sItem;
			_GetItem( str, i, sItem );
			m_aFireModes.push_back( (s8)atoi(sItem) );
		}
		m_iCurFireMode = ModesCount - 1;
	}
	result |= result2;

	result |= process_if_exists_set(section, "base_dispersioned_bullets_count", &CInifile::r_s32, m_iBaseDispersionedBulletsCount, test);
	result |= process_if_exists_set(section, "base_dispersioned_bullets_speed", &CInifile::r_float, m_fBaseDispersionedBulletsSpeed, test);

	// sounds (name of the sound, volume (0.0 - 1.0), delay (sec))
	result2 = process_if_exists_set( section, "snd_draw", &CInifile::r_string, str, test );
	if ( result2 && !test ) { m_sounds.LoadSound( section, "snd_draw"	    , "sndShow"		, false, m_eSoundShow		);	}
	result |= result2;

	result2 = process_if_exists_set( section, "snd_holster", &CInifile::r_string, str, test );
	if ( result2 && !test ) { m_sounds.LoadSound( section, "snd_holster"	, "sndHide"		, false, m_eSoundHide		);	}
	result |= result2;

	result2 = process_if_exists_set( section, "snd_shoot", &CInifile::r_string, str, test );
	if ( result2 && !test ) { m_sounds.LoadSound( section, "snd_shoot"	, "sndShot"		, false, m_eSoundShot		);	}
	result |= result2;

	result2 = process_if_exists_set( section, "snd_empty", &CInifile::r_string, str, test );
	if ( result2 && !test ) { m_sounds.LoadSound( section, "snd_empty"	, "sndEmptyClick"	, false, m_eSoundEmptyClick);	}
	result |= result2;

	result2 = process_if_exists_set( section, "snd_reload", &CInifile::r_string, str, test );
	if ( result2 && !test ) { m_sounds.LoadSound( section, "snd_reload"	, "sndReload"		, true, m_eSoundReload	);	}
	result |= result2;

	result2 = process_if_exists_set(section, "snd_reflect", &CInifile::r_string, str, test);
	if (result2 && !test) { m_sounds.LoadSound(section, "snd_reflect", "sndReflect", false, m_eSoundReflect); }
	result |= result2;

	//snd_shoot1     = weapons\ak74u_shot_1 ??
	//snd_shoot2     = weapons\ak74u_shot_2 ??
	//snd_shoot3     = weapons\ak74u_shot_3 ??

	if ( m_eSilencerStatus == ALife::eAddonAttachable || m_eSilencerStatus == ALife::eAddonPermanent )
	{
		result |= process_if_exists_set( section, "silencer_flame_particles", &CInifile::r_string, m_sSilencerFlameParticles, test );
		result |= process_if_exists_set( section, "silencer_smoke_particles", &CInifile::r_string, m_sSilencerSmokeParticles, test );
		
		result2 = process_if_exists_set( section, "snd_silncer_shot", &CInifile::r_string, str, test );
		if ( result2 && !test ) { m_sounds.LoadSound( section, "snd_silncer_shot"	, "sndSilencerShot", false, m_eSoundShot	);	}
		result |= result2;
	}

	// fov for zoom mode
	result |= process_if_exists( section, "ironsight_zoom_factor", &CInifile::r_float, m_zoom_params.m_fIronSightZoomFactor, test );

	if( IsScopeAttached() )
	{
		//if ( m_eScopeStatus == ALife::eAddonAttachable )
		{
			result |= process_if_exists( section, "scope_zoom_factor", &CInifile::r_float, m_zoom_params.m_fScopeZoomFactor, test );
		}
	}
	else
	{
		if( IsZoomEnabled() )
		{
			result |= process_if_exists( section, "scope_zoom_factor", &CInifile::r_float, m_zoom_params.m_fIronSightZoomFactor, test );
		}
	}

	return result;
}
//òåêóùàÿ äèñïåðñèÿ (â ðàäèàíàõ) îðóæèÿ ñ ó÷åòîì èñïîëüçóåìîãî ïàòðîíà è íåäèñïåðñèîííûõ ïóëü
float CWeaponMagazined::GetFireDispersion(float cartridge_k, bool for_crosshair) 
{
	float fire_disp = GetBaseDispersion(cartridge_k);
	if(for_crosshair || !m_iBaseDispersionedBulletsCount || !m_iShotNum || m_iShotNum > m_iBaseDispersionedBulletsCount)
	{
		fire_disp = inherited::GetFireDispersion(cartridge_k);
	}
	return fire_disp;
}
void CWeaponMagazined::FireBullet(	const Fvector& pos, 
									const Fvector& shot_dir, 
									float fire_disp,
									const CCartridge& cartridge,
									u16 parent_id,
									u16 weapon_id,
									bool send_hit)
{
	if(m_iBaseDispersionedBulletsCount)
	{
		if(m_iShotNum <= 1)
		{
			m_fOldBulletSpeed = GetBulletSpeed();
			SetBulletSpeed(m_fBaseDispersionedBulletsSpeed);
		}
		else if(m_iShotNum > m_iBaseDispersionedBulletsCount)
		{
			SetBulletSpeed(m_fOldBulletSpeed);
		}
	}
	inherited::FireBullet(pos, shot_dir, fire_disp, cartridge, parent_id, weapon_id, send_hit);
}

void CWeaponMagazined::CheckMagazine()
{
	if (!ParentIsActor())
	{
		m_bNeedBulletInGun = false;
		return;
	}

	if (psWpnAnimsFlag.test(ANM_RELOAD_EMPTY) && iAmmoElapsed >= 1 && m_bNeedBulletInGun == false)
	{
		m_bNeedBulletInGun = !m_bIsRevolver ? true : false;
	}
	else if (psWpnAnimsFlag.test(ANM_RELOAD_EMPTY) && iAmmoElapsed == 0 && m_bNeedBulletInGun == true)
	{
		m_bNeedBulletInGun = false;
	}
}

bool CWeaponMagazined::HaveCartridgeInInventory(u8 cnt)
{
	if (unlimited_ammo())
		return true;

	if (!m_pInventory)
		return false;

	u32 ac = GetAmmoCount(m_ammoType);

	if (ac < cnt)
	{
		for (u8 i = 0; i < u8(m_ammoTypes.size()); ++i)
		{
			if (m_ammoType == i) continue;
			ac += GetAmmoCount(i);

			if (ac >= cnt)
			{
				m_ammoType = i;
				break;
			}
		}
	}
	return ac >= cnt;
}

u8 CWeaponMagazined::GetAvailableCartridgesToLoad(bool full_reload)
{
	if (full_reload)
	{
		if (HaveCartridgeInInventory(iMagazineSize))
			return iMagazineSize;

		for (u8 try_load = (iMagazineSize - 1); try_load > 0; try_load--)
		{
			if (HaveCartridgeInInventory(try_load))
				return try_load;
		}

		return 0;
	}
	else
	{
		u8 needed = iMagazineSize - iAmmoElapsed;

		if (HaveCartridgeInInventory(needed))
			return needed;

		for (u8 try_load = needed - 1; try_load > 0; try_load--)
		{
			if (HaveCartridgeInInventory(try_load))
				return try_load;
		}

		return 0;
	}
}