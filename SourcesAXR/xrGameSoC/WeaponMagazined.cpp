#include "stdafx.h"
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
#include "inventory.h"
#include "xrserver_objects_alife_items.h"
#include "ActorEffector.h"
#include "EffectorZoomInertion.h"
#include "xr_level_controller.h"
#include "level.h"
#include "object_broker.h"
#include "string_table.h"
#include "script_callback_ex.h"
#include "script_game_object.h"
#include "player_hud.h"
#include "Torch.h"
#include "ActorNightVision.h"
#include "AdvancedXrayGameConstants.h"

ENGINE_API  extern float psHUD_FOV;
ENGINE_API  extern float psHUD_FOV_def;

//CUIXml*				pWpnScopeXml = NULL;

CWeaponMagazined::CWeaponMagazined(LPCSTR name, ESoundTypes eSoundType) : CWeapon(name)
{
	m_eSoundShow		= ESoundTypes(SOUND_TYPE_ITEM_TAKING | eSoundType);
	m_eSoundHide		= ESoundTypes(SOUND_TYPE_ITEM_HIDING | eSoundType);
	m_eSoundShot		= ESoundTypes(SOUND_TYPE_WEAPON_SHOOTING | eSoundType);
	m_eSoundEmptyClick	= ESoundTypes(SOUND_TYPE_WEAPON_EMPTY_CLICKING | eSoundType);
	m_eSoundReload		= ESoundTypes(SOUND_TYPE_WEAPON_RECHARGING | eSoundType);
	m_eSoundClose		= ESoundTypes(SOUND_TYPE_WEAPON_RECHARGING);

	psWpnAnimsFlag = { 0 };
	
	m_sSndShotCurrent = nullptr;
	m_sSilencerFlameParticles = m_sSilencerSmokeParticles = NULL;

	m_bFireSingleShot		= false;
	m_iShotNum				= 0;
	bullet_cnt				= 0;
	m_iQueueSize			= WEAPON_ININITE_QUEUE;
	m_bLockType				= false;
	m_bAutoreloadEnabled	= READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "autoreload_enabled", true);
	m_bNeedBulletInGun		= false;
	m_bHasDifferentFireModes = false;
	m_opened				= false;
	m_bUseFiremodeChangeAnim = true;
	bHasBulletsToHide		= false;
	m_bIsRevolver			= false;
	m_bIsMosin				= false;
}

CWeaponMagazined::~CWeaponMagazined()
{
	// sounds
}


void CWeaponMagazined::StopHUDSounds		()
{
	//inherited::StopHUDSounds();
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
		
	SetAnimFlag(ANM_SHOW_EMPTY,		"anm_show_empty");
	SetAnimFlag(ANM_HIDE_EMPTY,		"anm_hide_empty");
	SetAnimFlag(ANM_IDLE_EMPTY,		"anm_idle_empty");
	SetAnimFlag(ANM_AIM_EMPTY,		"anm_idle_aim_empty");
	SetAnimFlag(ANM_BORE_EMPTY,		"anm_bore_empty");
	SetAnimFlag(ANM_SHOT_EMPTY,		"anm_shot_l");
	SetAnimFlag(ANM_SPRINT_EMPTY,	"anm_idle_sprint_empty");
	SetAnimFlag(ANM_MOVING_EMPTY,	"anm_idle_moving_empty");
	SetAnimFlag(ANM_RELOAD_EMPTY,	"anm_reload_empty");
	SetAnimFlag(ANM_MISFIRE,		"anm_reload_misfire");
	SetAnimFlag(ANM_SHOT_AIM,		"anm_shots_when_aim");

	// Sounds
	m_sounds.LoadSound	(section, "snd_draw", "sndShow", false, m_eSoundShow);
	m_sounds.LoadSound	(section, "snd_holster", "sndHide", false, m_eSoundHide);
	
	//Alundaio: LAYERED_SND_SHOOT
	m_sounds.LoadSound(section, "snd_shoot", "sndShot", m_eSoundShot);
	m_sounds.LoadSound(section, "snd_mag_shot", "sndMagShot", false, m_eSoundShot);
	
	if (WeaponSoundExist(section, "snd_shoot_actor", true))
	{
		m_sounds.LoadSound(section, "snd_shoot_actor", "sndShotActor", false, m_eSoundShot);

		if (WeaponSoundExist(section, "snd_shoot_last_actor", true))
			m_sounds.LoadSound(section, "snd_shoot_last_actor", "sndShotActorLast", false, m_eSoundShot);

		if (WeaponSoundExist(section, "snd_silncer_shoot_last_actor", true))
			m_sounds.LoadSound(section, "snd_silncer_shoot_last_actor", "sndSilencerShotActorLast", false, m_eSoundShot);
	}
	//-Alundaio

	if (WeaponSoundExist(section, "snd_shoot_last", true))
		m_sounds.LoadSound(section, "snd_shoot_last", "sndShotLast", false, m_eSoundShot);

	if (WeaponSoundExist(section, "snd_silncer_shoot_last", true))
		m_sounds.LoadSound(section, "snd_silncer_shoot_last", "sndSilencerShotLast", false, m_eSoundShot);

	m_sSndShotCurrent = IsSilencerAttached() ? "sndSilencerShot" : "sndShot";

	m_sounds.LoadSound	(section, "snd_empty", "sndEmptyClick", false, m_eSoundEmptyClick);
	m_sounds.LoadSound	(section, "snd_reload", "sndReload", false, m_eSoundReload);
	m_sounds.LoadSound	(section, "snd_reflect", "sndReflect", true, m_eSoundReflect);

	if (WeaponSoundExist(section, "snd_changefiremode", true))
		m_sounds.LoadSound(section, "snd_changefiremode", "sndFireModes", m_eSoundEmptyClick);

	if (WeaponSoundExist(section, "snd_laser_on", true))
		m_sounds.LoadSound(section, "snd_laser_on", "sndLaserOn", m_eSoundEmptyClick);
	if (WeaponSoundExist(section, "snd_laser_off", true))
		m_sounds.LoadSound(section, "snd_laser_off", "sndLaserOff", m_eSoundEmptyClick);
	if (WeaponSoundExist(section, "snd_torch_on", true))
		m_sounds.LoadSound(section, "snd_torch_on", "sndFlashlightOn", m_eSoundEmptyClick);
	if (WeaponSoundExist(section, "snd_torch_off", true))
		m_sounds.LoadSound(section, "snd_torch_off", "sndFlashlightOff", m_eSoundEmptyClick);

	if (WeaponSoundExist(section, "snd_change_zoom", true))
		m_sounds.LoadSound(section, "snd_change_zoom", "sndChangeZoom", m_eSoundEmptyClick);

	if (WeaponSoundExist(section, "snd_close", true))
		m_sounds.LoadSound(section, "snd_close", "sndClose", m_eSoundClose);

	if (WeaponSoundExist(section, "snd_reload_empty", true))
		m_sounds.LoadSound(section, "snd_reload_empty", "sndReloadEmpty", m_eSoundReload);
	if (WeaponSoundExist(section, "snd_reload_misfire", true))
		m_sounds.LoadSound(section, "snd_reload_misfire", "sndReloadMisfire", m_eSoundReload);
	if (WeaponSoundExist(section, "snd_reload_jammed", true))
		m_sounds.LoadSound(section, "snd_reload_jammed", "sndReloadJammed", m_eSoundReload);
	if (WeaponSoundExist(section, "snd_pump_gun", true))
		m_sounds.LoadSound(section, "snd_pump_gun", "sndPumpGun", m_eSoundReload);

	// Revolvers reload
	m_bIsRevolver = READ_IF_EXISTS(pSettings, r_bool, section, "is_revolver", false);

	if (m_bIsRevolver)
	{
		if (WeaponSoundExist(section, "snd_reload_6", true))
			m_sounds.LoadSound(section, "snd_reload_6", "sndReload6", m_eSoundReload);
		if (WeaponSoundExist(section, "snd_reload_5", true))
			m_sounds.LoadSound(section, "snd_reload_5", "sndReload5", m_eSoundReload);
		if (WeaponSoundExist(section, "snd_reload_4", true))
			m_sounds.LoadSound(section, "snd_reload_4", "sndReload4", m_eSoundReload);
		if (WeaponSoundExist(section, "snd_reload_3", true))
			m_sounds.LoadSound(section, "snd_reload_3", "sndReload3", m_eSoundReload);
		if (WeaponSoundExist(section, "snd_reload_2", true))
			m_sounds.LoadSound(section, "snd_reload_2", "sndReload2", m_eSoundReload);
		if (WeaponSoundExist(section, "snd_reload_1", true))
			m_sounds.LoadSound(section, "snd_reload_1", "sndReload1", m_eSoundReload);
	}

	//звуки и партиклы глушителя, еслит такой есть
	if (m_eSilencerStatus == ALife::eAddonAttachable)
	{
		if (pSettings->line_exist(section, "silencer_flame_particles"))
			m_sSilencerFlameParticles = pSettings->r_string(section, "silencer_flame_particles");
		if (pSettings->line_exist(section, "silencer_smoke_particles"))
			m_sSilencerSmokeParticles = pSettings->r_string(section, "silencer_smoke_particles");
		m_sounds.LoadSound(section,"snd_silncer_shot", "sndSilencerShot", false, m_eSoundShot);
	}

	if (WeaponSoundExist(section, "snd_zoom_in", true))
		m_sounds.LoadSound(section, "snd_zoom_in", "sndZoomIn", true, m_eSoundEmptyClick);
	if (WeaponSoundExist(section, "snd_zoom_out", true))
		m_sounds.LoadSound(section, "snd_zoom_out", "sndZoomOut", true, m_eSoundEmptyClick);
	if (WeaponSoundExist(section, "snd_sprint_start", true))
		m_sounds.LoadSound(section, "snd_sprint_start", "sndSprintStart", true, m_eSoundEmptyClick);
	if (WeaponSoundExist(section, "snd_sprint_end", true))
		m_sounds.LoadSound(section, "snd_sprint_end", "sndSprintEnd", true, m_eSoundEmptyClick);
	if (WeaponSoundExist(section, "snd_sprint_idle", true))
		m_sounds.LoadSound(section, "snd_sprint_idle", "sndSprintIdle", true, m_eSoundEmptyClick);

	//  [7/20/2005]
	if (pSettings->line_exist(section, "dispersion_start"))
		m_iShootEffectorStart = pSettings->r_u8(section, "dispersion_start");
	else
		m_iShootEffectorStart = 0;
	//  [7/20/2005]
	//  [7/21/2005]
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
			int FireMode = atoi(sItem);
			m_aFireModes.push_back(FireMode);			
		}
		m_iCurFireMode = ModesCount - 1;
		m_iPrefferedFireMode = READ_IF_EXISTS(pSettings, r_s16,section,"preffered_fire_mode",-1);
	}
	else
		m_bHasDifferentFireModes = false;
	//  [7/21/2005]

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
	if (IsValid() && !IsMisfire()) 
	{
		if (!IsWorking() || AllowFireWhileWorking())
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
			else
				SwitchState(eFire);
		}
	} 
	else 
	{
		if (eReload!=GetState() && eMisfire!=GetState()) OnMagazineEmpty();
	}
}

void CWeaponMagazined::FireEnd() 
{
	inherited::FireEnd();

	if (m_bAutoreloadEnabled)
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
}

void CWeaponMagazined::OnMotionMark(u32 state, const motion_marks& M)
{
	inherited::OnMotionMark(state, M);
	if (state == eReload)
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
	if (m_pInventory) 
	{
		m_pAmmo = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(*m_ammoTypes[m_ammoType] ));

		
		if (IsMisfire() && iAmmoElapsed)
		{
			SetPending(TRUE);
			SwitchState(eUnMisfire);
			return				true;
		}

		if (m_pAmmo || unlimited_ammo())
		{
			SetPending(TRUE);
			SwitchState(eReload); 
			return true;
		}
		else for(u32 i = 0; i < m_ammoTypes.size(); ++i) 
		{
			for (u32 i = 0; i < m_ammoTypes.size(); ++i)
			{
				m_pAmmo = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(*m_ammoTypes[i]));
				if (m_pAmmo)
				{
					m_set_next_ammoType_on_reload = i;
					SetPending(TRUE);
					SwitchState(eReload);
					return true;
				}
			}
		}
	}
	
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

	//попытка стрелять когда нет патронов
	if (GetState() == eIdle) 
	{
		OnEmptyClick			();
		return;
	}

	if ( GetNextState() != eMagEmpty && GetNextState() != eReload)
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
			if (!xr_strcmp(*l_cartridge.m_ammoSect, l_it->first)) 
			{ 
				 ++(l_it->second); 
				 break; 
			}
		}

		if (l_it == l_ammo.end()) l_ammo[*l_cartridge.m_ammoSect] = 1;
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
		if (m_pInventory)
		{
			CWeaponAmmo* l_pA = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(l_it->first));
			if (l_pA)
			{
				u16 l_free = l_pA->m_boxSize - l_pA->m_boxCurr;
				l_pA->m_boxCurr = l_pA->m_boxCurr + (l_free < l_it->second ? l_free : l_it->second);
				l_it->second = l_it->second - (l_free < l_it->second ? l_free : l_it->second);
			}
		}
		if (l_it->second && !unlimited_ammo()) SpawnAmmo(l_it->second, l_it->first);
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

	//устранить осечку при перезарядке
	if (IsMisfire())	bMisfire = false;
	
	//переменная блокирует использование
	//только разных типов патронов
//	static bool l_lockType = false;
	if (!m_bLockType) {
		m_ammoName	= NULL;
		m_pAmmo		= NULL;
	}
	
	if (!m_pInventory) return;

	if (m_set_next_ammoType_on_reload != u32(-1)){		
		m_ammoType						= m_set_next_ammoType_on_reload;
		m_set_next_ammoType_on_reload	= u32(-1);
	}
	
	if (!unlimited_ammo()) 
	{
		//попытаться найти в инвентаре патроны текущего типа 
		m_pAmmo = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(*m_ammoTypes[m_ammoType]));
		
		if (!m_pAmmo && !m_bLockType) 
		{
			for(u32 i = 0; i < m_ammoTypes.size(); ++i) 
			{
				//проверить патроны всех подходящих типов
				m_pAmmo = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(*m_ammoTypes[i]));
				if (m_pAmmo) 
				{ 
					m_ammoType = i; 
					break; 
				}
			}
		}
	}
	else
		m_ammoType = m_ammoType;


	//нет патронов для перезарядки
	if (!m_pAmmo && !unlimited_ammo() ) return;

	//разрядить магазин, если загружаем патронами другого типа
	if (!m_bLockType && !m_magazine.empty() && (!m_pAmmo || xr_strcmp(m_pAmmo->cNameSect(), *m_magazine.front().m_ammoSect)))
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
			if (!m_pAmmo->Get(l_cartridge)) break;
		}
		++iAmmoElapsed;
		l_cartridge.m_LocalAmmoType = u8(m_ammoType);
		m_magazine.push_back(l_cartridge);
	}
	m_ammoName = (m_pAmmo) ? m_pAmmo->m_nameShort : NULL;

	VERIFY((u32)iAmmoElapsed == m_magazine.size());

	//выкинуть коробку патронов, если она пустая
	if (m_pAmmo && !m_pAmmo->m_boxCurr && OnServer()) 
		m_pAmmo->SetDropManual(TRUE);

	if (iMagazineSize > iAmmoElapsed) 
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

void CWeaponMagazined::OnStateSwitch(u32 S, u32 oldState)
{
	HUD_VisualBulletUpdate();

	inherited::OnStateSwitch(S, oldState);
	switch (S)
	{
	case eFiremodeNext:
		{
			PlaySound("sndFireModes", get_LastFP());
			switch2_ChangeFireMode();
		} break;
	case eFiremodePrev:
		{
			PlaySound("sndFireModes", get_LastFP());
			switch2_ChangeFireMode();
		} break;
	case eLaserSwitch:
		{
			if (IsLaserOn())
				PlaySound("sndLaserOn", get_LastFP());
			else
				PlaySound("sndLaserOff", get_LastFP());

			switch2_LaserSwitch();
		} break;
	case eFlashlightSwitch:
		{
			if (IsFlashlightOn())
				PlaySound("sndFlashlightOn", get_LastFP());
			else
				PlaySound("sndFlashlightOff", get_LastFP());

			switch2_FlashlightSwitch();
		} break;
	case eIdle:
		switch2_Idle	();
		break;
	case eFire:
		switch2_Fire	();
		break;
	case eFire2:
		switch2_Fire2	();
		break;
	case eUnMisfire:
		/*if (owner)
			m_sounds_enabled = owner->CanPlayShHdRldSounds();*/
		switch2_Unmis();
		break;
	case eMisfire:
	{
		//misfire
		CGameObject* object = smart_cast<CGameObject*>(H_Parent());
		if (object)
			object->callback(GameObject::eOnWeaponJammed)(object->lua_game_object(), this->lua_game_object());

		if (smart_cast<CActor*>(this->H_Parent()) && (Level().CurrentViewEntity() == H_Parent()))
			HUD().GetUI()->AddInfoMessage("gun_jammed");
	} break;
	case eMagEmpty:
		switch2_Empty	();
		break;
	case eReload:
		switch2_Reload	();
		break;
	case eShowing:
		switch2_Showing	();
		break;
	case eHiding:
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

	//когда происходит апдейт состояния оружия
	//ничего другого не делать
	if (GetNextState() == GetState())
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
			fTime			-=	dt;
			if (fTime<0)	
				fTime = 0;
			break;
		case eFire:			
			if (iAmmoElapsed>0)
				state_Fire		(dt);
			
			if (fTime<=0)
			{
				if (iAmmoElapsed == 0)
					OnMagazineEmpty();
				StopShooting();
			}
			else
			{
				fTime			-=	dt;
			}

			break;
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

	// ref_sound positions
	m_sounds.SetPosition("sndShow",		get_LastFP());
	m_sounds.SetPosition("sndHide",		get_LastFP());
	m_sounds.SetPosition("sndReload",	get_LastFP());
	
	if (psWpnAnimsFlag.test(ANM_HIDE_EMPTY) && WeaponSoundExist(m_section_id.c_str(), "snd_close"))
		m_sounds.SetPosition("sndClose", get_LastFP());

	if (WeaponSoundExist(m_section_id.c_str(), "snd_changefiremode"))
		m_sounds.SetPosition("sndFireModes", get_LastFP());
	if (WeaponSoundExist(m_section_id.c_str(), "snd_laser_on"))
		m_sounds.SetPosition("sndLaserOn", get_LastFP());
	if (WeaponSoundExist(m_section_id.c_str(), "snd_laser_off"))
		m_sounds.SetPosition("sndLaserOff", get_LastFP());
	if (WeaponSoundExist(m_section_id.c_str(), "snd_torch_on"))
		m_sounds.SetPosition("sndFlashlightOn", get_LastFP());
	if (WeaponSoundExist(m_section_id.c_str(), "snd_torch_off"))
		m_sounds.SetPosition("sndFlashlightOff", get_LastFP());
	if (WeaponSoundExist(m_section_id.c_str(), "snd_change_zoom"))
		m_sounds.SetPosition("sndChangeZoom", get_LastFP());
	if (WeaponSoundExist(m_section_id.c_str(), "snd_zoom_in"))
		m_sounds.SetPosition("sndZoomIn", get_LastFP());
	if (WeaponSoundExist(m_section_id.c_str(), "snd_zoom_out"))
		m_sounds.SetPosition("sndZoomOut", get_LastFP());
	if (WeaponSoundExist(m_section_id.c_str(), "snd_sprint_start"))
		m_sounds.SetPosition("sndSprintStart", get_LastFP());
	if (WeaponSoundExist(m_section_id.c_str(), "snd_sprint_end"))
		m_sounds.SetPosition("sndSprintEnd", get_LastFP());
	if (WeaponSoundExist(m_section_id.c_str(), "snd_sprint_idle"))
		m_sounds.SetPosition("sndSprintIdle", get_LastFP());
}

void CWeaponMagazined::state_Fire	(float dt)
{
	VERIFY(fTimeToFire>0.f);

	Fvector					p1, d; 
	p1.set(get_LastFP());
	d.set(get_LastFD());

	if (!H_Parent()) return;

	/*CInventoryOwner* io		= smart_cast<CInventoryOwner*>(H_Parent());
	if (NULL == io->inventory().ActiveItem())
	{
			Log("current_state", GetState() );
			Log("next_state", GetNextState());
			Log("state_time", m_dwStateTime);
			Log("item_sect", cNameSect().c_str());
			Log("H_Parent", H_Parent()->cNameSect().c_str());
	}  */


	smart_cast<CEntity*>	(H_Parent())->g_fireParams	(this, p1,d);
	if (m_iShotNum == 0)
	{
		m_vStartPos = p1;
		m_vStartDir = d;
	};
		
	VERIFY(!m_magazine.empty());
//	Msg("%d && %d && (%d || %d) && (%d || %d)", !m_magazine.empty(), fTime<=0, IsWorking(), m_bFireSingleShot, m_iQueueSize < 0, m_iShotNum < m_iQueueSize);
	while (!m_magazine.empty() && fTime <= 0 && (IsWorking() || m_bFireSingleShot) && (m_iQueueSize < 0 || m_iShotNum < m_iQueueSize))
	{
		m_bFireSingleShot = false;

		VERIFY(fTimeToFire>0.f);
		fTime			+=	fTimeToFire;
		fTime			+= (fTimeToFire * (m_fOverheatingSubRpm / 100.f)) * m_fWeaponOverheating;

		++m_iShotNum;
		
		OnShot			();
		static int i = 0;
		if (i||m_iShotNum>m_iShootEffectorStart)
			FireTrace		(p1,d);
		else
			FireTrace		(m_vStartPos, m_vStartDir);
	}
	
	if (m_iShotNum == m_iQueueSize)
		m_bStopedAfterQueueFired = true;

	UpdateSounds			();

	//if (m_fFactor > 0)
	//	StopShooting();
}

void CWeaponMagazined::state_Misfire	(float /**dt/**/)
{
	OnEmptyClick			();
	SwitchState				(eIdle);
	
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

void CWeaponMagazined::OnShot		()
{
	if (ParentIsActor() && GameConstants::GetStopActorIfShoot())
		Actor()->set_state_wishful(Actor()->get_state_wishful() & (~mcSprint));

	// Camera	
	AddShotEffector		();

	// Animation
	PlayAnimShoot		();

	HUD_VisualBulletUpdate();
	
	// Shell Drop
	Fvector vel; 
	PHGetLinearVell(vel);
	OnShellDrop					(get_LastSP(), vel);
	
	// Огонь из ствола
	StartFlameParticles	();

	//дым из ствола
	ForceUpdateFireParticles	();
	StartSmokeParticles			(get_LastFP(), vel);

	// Проиграем звук помпы отдельно, если не будет работать то будем думать что делать и как быть
	if (m_sounds.FindSoundItem("sndPumpGun", false))
		PlaySound("sndPumpGun", get_LastFP());

	if (ParentIsActor())
	{
		luabind::functor<void> funct;
		if (ai().script_engine().functor("mfs_functions.on_actor_shoot", funct))
			funct();

		if (auto mag_shot_snd = m_sounds.FindSoundItem("sndMagShot", false))
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

		string128 sndName;
		strconcat(sizeof(sndName), sndName, m_sSndShotCurrent.c_str(), "Actor", (iAmmoElapsed == 1) ? "Last" : "");
		if (m_sounds.FindSoundItem(sndName, false))
		{
			m_sounds.PlaySound(sndName, get_LastFP(), H_Root(), !!GetHUDmode(), false, (u8)-1);
			return;
		}
		else if (strstr(sndName, "Last"))
		{
			char newSndName[256];
			strcpy(newSndName, sndName);
			newSndName[strlen(sndName) - strlen("Last")] = '\0';

			if (m_sounds.FindSoundItem(newSndName, false))
			{
				m_sounds.PlaySound(newSndName, get_LastFP(), H_Root(), !!GetHUDmode(), false, (u8)-1);
				return;
			}
		}
	}

	string128 sndName;
	strconcat(sizeof(sndName), sndName, m_sSndShotCurrent.c_str(), (iAmmoElapsed == 1) ? "Last" : "");

	if (m_sounds.FindSoundItem(sndName, false))
		m_sounds.PlaySound(sndName, get_LastFP(), H_Root(), !!GetHUDmode(), false, (u8)-1);
	else
		m_sounds.PlaySound(m_sSndShotCurrent.c_str(), get_LastFP(), H_Root(), !!GetHUDmode(), false, (u8)-1);

	// Эхо выстрела
	if (IsSilencerAttached() == false)
	{
		bool bIndoor = false;
		if (H_Parent() != nullptr)
		{
			bIndoor = H_Parent()->renderable_ROS()->get_luminocity_hemi() < WEAPON_INDOOR_HEMI_FACTOR;
		}

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

	CGameObject* object = smart_cast<CGameObject*>(H_Parent());
	if (object)
		object->callback(GameObject::eOnWeaponFired)(object->lua_game_object(), this->lua_game_object(), iAmmoElapsed);

	// Эффект сдвига (отдача)
	AddHUDShootingEffect();
}


void CWeaponMagazined::OnEmptyClick	()
{
	if (m_BlendFakeShootCam.name.size())
		g_player_hud->PlayBlendAnm(m_BlendFakeShootCam.name.c_str(), 2, m_BlendFakeShootCam.speed, m_BlendFakeShootCam.power, false, false);

	PlaySound	("sndEmptyClick", get_LastFP());
	PlayAnimFakeShoot();
}

void CWeaponMagazined::OnAnimationEnd(u32 state) 
{
	switch(state) 
	{
		case eReload:
		{
			CheckMagazine();

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

			SwitchState(eIdle);

		}break;// End of reload animation
		case eHiding:
		{
			SwitchState(eHidden);
			ResetShootingEffect();
			break;	// End of Hide
		}
		case eShowing:	SwitchState(eIdle);		break;	// End of Show
		case eIdle:		switch2_Idle();			break;  // Keep showing idle
		case eUnMisfire:
		{
			bMisfire = false;
			
			if (!m_bIsRevolver)
			{
				m_magazine.pop_back();
				iAmmoElapsed--;
			}

			SwitchState(eIdle);
		} break; // End of UnMisfire animation
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
}

void CWeaponMagazined::switch2_Idle	()
{
	SetPending(FALSE);
	PlayAnimIdle();
}

void CWeaponMagazined::switch2_ChangeFireMode()
{
	if (GetState() != eFiremodeNext && GetState() != eFiremodePrev)
		return;

	FireEnd();
	PlayAnimFireMode();
	SetPending(TRUE);
}

void CWeaponMagazined::PlayAnimFireMode()
{
	string_path guns_firemode_anm{};

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

void CWeaponMagazined::switch2_LaserSwitch()
{
	if (GetState() != eLaserSwitch)
		return;

	LaserSwitchAction = true;

	FireEnd();
	PlayAnimLaserSwitch();
	SetPending(TRUE);
}

void CWeaponMagazined::switch2_FlashlightSwitch()
{
	if (GetState() != eFlashlightSwitch)
		return;

	FlashlightSwitchAction = true;

	FireEnd();
	PlayAnimFlashlightSwitch();
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

#ifdef DEBUG
#include "ai\stalker\ai_stalker.h"
#include "object_handler_planner.h"
#endif
void CWeaponMagazined::switch2_Fire	()
{
	CInventoryOwner* io		= smart_cast<CInventoryOwner*>(H_Parent());
	CInventoryItem* ii		= smart_cast<CInventoryItem*>(this);
#ifdef DEBUG
	VERIFY2					(io,make_string("no inventory owner, item %s",*cName()));

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

	if ((OnClient() || Level().IsDemoPlay())&& !IsWorking())
		FireStart();

/*	if (SingleShotMode())
	{
		m_bFireSingleShot = true;
		bWorking = false;
	}*/
}
void CWeaponMagazined::switch2_Empty()
{
	OnZoomOut();
	
	if (m_bAutoreloadEnabled)
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
		OnEmptyClick();
	}
}
void CWeaponMagazined::PlayReloadSound()
{
	if (m_bIsRevolver)
	{
		string128 sndReloadName;
		strconcat(sizeof(sndReloadName), sndReloadName, "sndReload", (iAmmoElapsed > 0) ? std::to_string(iAmmoElapsed).c_str() : "Empty");

		if (m_sounds.FindSoundItem(sndReloadName, false))
			m_sounds.PlaySound(sndReloadName, get_LastFP(), H_Root(), !!GetHUDmode(), false, (u8)-1);
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

void CWeaponMagazined::switch2_Reload()
{
	CWeapon::FireEnd();

	PlayReloadSound	();
	PlayAnimReload	();
	SetPending(TRUE);
}
void CWeaponMagazined::switch2_Hiding()
{
	OnZoomOut();
	CWeapon::FireEnd();
	
	if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_HIDE_EMPTY) && WeaponSoundExist(m_section_id.c_str(), "snd_close"))
		PlaySound("sndClose", get_LastFP());
	else
		PlaySound("sndHide", get_LastFP());

	PlayAnimHide();
	SetPending(TRUE);
}

void CWeaponMagazined::switch2_Unmis()
{
	VERIFY(GetState() == eUnMisfire);

	if (m_sounds.FindSoundItem("sndReloadMisfire", false) && isHUDAnimationExist("anm_reload_misfire"))
		PlaySound("sndReloadMisfire", get_LastFP());
	else if (m_sounds.FindSoundItem("sndReloadJammed", false) && isHUDAnimationExist("anm_reload_jammed"))
		PlaySound("sndReloadJammed", get_LastFP());
	else
		PlayReloadSound();

	if (isHUDAnimationExist("anm_reload_misfire"))
		PlayHUDMotionIfExists({ "anm_reload_misfire", "anm_reload", "anim_reload" }, true, GetState());
	else if (isHUDAnimationExist("anm_reload_jammed"))
		PlayHUDMotionIfExists({ "anm_reload_jammed", "anm_reload", "anim_reload" }, true, GetState());
	else
		PlayAnimReload();
}

void CWeaponMagazined::switch2_Hidden()
{
	CWeapon::FireEnd();

	StopCurrentAnimWithoutCallback();

	signal_HideComplete		();
	RemoveShotEffector		();

	if (pSettings->line_exist(m_item_sect, "hud_fov"))
		m_nearwall_last_hud_fov = m_base_fov;
	else
		m_nearwall_last_hud_fov = psHUD_FOV_def;
}
void CWeaponMagazined::switch2_Showing()
{
	PlaySound	("sndShow", get_LastFP());

	SetPending(TRUE);
	PlayAnimShow();
}

bool CWeaponMagazined::Action(s32 cmd, u32 flags) 
{
	if (inherited::Action(cmd, flags)) return true;
	
	//если оружие чем-то занято, то ничего не делать
	if (IsPending()) return false;
	
	switch(cmd) 
	{
	case kWPN_RELOAD:
		{
			if (Actor()->mstate_real & (mcSprint) && !GameConstants::GetReloadIfSprint())
				break;
			else if (flags&CMD_START)  
				if (iAmmoElapsed < iMagazineSize || IsMisfire())
				{
					if (GetState() == eUnMisfire) // Rietmon: Запрещаем перезарядку, если играет анима передергивания затвора
						return false;

					Reload();
				}
		} 
		return true;
	case kWPN_FIREMODE_PREV:
		{
			if (flags&CMD_START) 
			{
				OnPrevFireMode();
				return true;
			};
		}break;
	case kWPN_FIREMODE_NEXT:
		{
			if (flags&CMD_START) 
			{
				OnNextFireMode();
				return true;
			};
		}break;
	}
	return false;
}

bool CWeaponMagazined::CanAttach(PIItem pIItem)
{
	CScope*				pScope = smart_cast<CScope*>(pIItem);
	CSilencer*			pSilencer = smart_cast<CSilencer*>(pIItem);
	CGrenadeLauncher*	pGrenadeLauncher = smart_cast<CGrenadeLauncher*>(pIItem);
	CLaserDesignator*	pLaser = smart_cast<CLaserDesignator*>(pIItem);
	CTacticalTorch*		pTacticalTorch = smart_cast<CTacticalTorch*>(pIItem);

	if (pScope &&
		m_eScopeStatus == ALife::eAddonAttachable &&
		(m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonScope) == 0 /*&&
		(m_scopes[cur_scope]->m_sScopeName == pIItem->object().cNameSect())*/)
	{
		SCOPES_VECTOR_IT it = m_scopes.begin();
		for (; it != m_scopes.end(); it++)
		{
			if (bUseAltScope)
			{
				if (*it == pIItem->object().cNameSect())
					return true;
			}
			else
			{
				if (pSettings->r_string((*it), "scope_name") == pIItem->object().cNameSect())
					return true;
			}
		}
		return false;
	}
	else if (pSilencer &&
			m_eSilencerStatus == ALife::eAddonAttachable &&
			(m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonSilencer) == 0 &&
			(m_sSilencerName == pIItem->object().cNameSect()))
			{
				if (m_bBlockSilencerWithGL && IsGrenadeLauncherAttached())
					return false;

				return true;
			}
	else if (pGrenadeLauncher &&
			m_eGrenadeLauncherStatus == ALife::eAddonAttachable &&
			(m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher) == 0 &&
			(m_sGrenadeLauncherName == pIItem->object().cNameSect()))
			{
				if (m_bBlockSilencerWithGL && IsSilencerAttached())
					return false;

				return true;
			}
	else if (pLaser &&
		m_eLaserDesignatorStatus == ALife::eAddonAttachable &&
		(m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonLaserDesignator) == 0 &&
		(m_sLaserName == pIItem->object().cNameSect()))
		return true;
	else if (pTacticalTorch &&
		m_eTacticalTorchStatus == ALife::eAddonAttachable &&
		(m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonTacticalTorch) == 0 &&
		(m_sTacticalTorchName == pIItem->object().cNameSect()))
		return true;
	else
		return inherited::CanAttach(pIItem);
}

bool CWeaponMagazined::CanDetach(const char* item_section_name)
{
	if (m_eScopeStatus == ALife::eAddonAttachable &&
		0 != (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonScope))/* &&
		(m_scopes[cur_scope]->m_sScopeName	== item_section_name))*/
	{
		SCOPES_VECTOR_IT it = m_scopes.begin();
		for (; it != m_scopes.end(); it++)
		{
			if (bUseAltScope)
			{
				if (*it == item_section_name)
					return true;
			}
			else
			{
				if (pSettings->r_string((*it), "scope_name") == item_section_name)
					return true;
			}
		}
		return false;
	}
	//	   return true;
	else if (m_eSilencerStatus == ALife::eAddonAttachable &&
		0 != (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonSilencer) &&
		(m_sSilencerName == item_section_name))
		return true;
	else if (m_eGrenadeLauncherStatus == ALife::eAddonAttachable &&
		0 != (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher) &&
		(m_sGrenadeLauncherName == item_section_name))
		return true;
	else if (m_eLaserDesignatorStatus == ALife::eAddonAttachable &&
		0 != (m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonLaserDesignator) &&
		(m_sLaserName == item_section_name))
		return true;
	else if (m_eTacticalTorchStatus == ALife::eAddonAttachable &&
		0 != (m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonTacticalTorch) &&
		(m_sTacticalTorchName == item_section_name))
		return true;
	else
		return inherited::CanDetach(item_section_name);
}

bool CWeaponMagazined::Attach(PIItem pIItem, bool b_send_event)
{
	bool result = false;

	CScope*				pScope = smart_cast<CScope*>(pIItem);
	CSilencer*			pSilencer = smart_cast<CSilencer*>(pIItem);
	CGrenadeLauncher*	pGrenadeLauncher = smart_cast<CGrenadeLauncher*>(pIItem);
	CLaserDesignator*	pLaser = smart_cast<CLaserDesignator*>(pIItem);
	CTacticalTorch*		pTacticalTorch = smart_cast<CTacticalTorch*>(pIItem);

	if (pScope &&
		m_eScopeStatus == ALife::eAddonAttachable &&
		(m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonScope) == 0 /*&&
		(m_scopes[cur_scope]->m_sScopeName == pIItem->object().cNameSect())*/)
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
		m_flagsAddOnState |= CSE_ALifeItemWeapon::eWeaponAddonScope;
		result = true;
	}
	else if (pSilencer &&
		m_eSilencerStatus == ALife::eAddonAttachable &&
		(m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonSilencer) == 0 &&
		(m_sSilencerName == pIItem->object().cNameSect()))
	{
		m_flagsAddOnState |= CSE_ALifeItemWeapon::eWeaponAddonSilencer;
		result = true;
	}
	else if (pGrenadeLauncher &&
		m_eGrenadeLauncherStatus == ALife::eAddonAttachable &&
		(m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher) == 0 &&
		(m_sGrenadeLauncherName == pIItem->object().cNameSect()))
	{
		m_flagsAddOnState |= CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher;
		result = true;
	}
	else if (pLaser &&
		m_eLaserDesignatorStatus == ALife::eAddonAttachable &&
		(m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonLaserDesignator) == 0 &&
		(m_sLaserName == pIItem->object().cNameSect()))
	{
		m_flagsAddOnState |= CSE_ALifeItemWeapon::eWeaponAddonLaserDesignator;
		result = true;
	}
	else if (pTacticalTorch &&
		m_eTacticalTorchStatus == ALife::eAddonAttachable &&
		(m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonTacticalTorch) == 0 &&
		(m_sTacticalTorchName == pIItem->object().cNameSect()))
	{
		m_flagsAddOnState |= CSE_ALifeItemWeapon::eWeaponAddonTacticalTorch;
		result = true;
	}

	if (result)
	{
		if (pScope && bUseAltScope)
		{
			bNVsecondVPstatus = !!pSettings->line_exist(pIItem->object().cNameSect(), "scope_nightvision");
		}

		if (b_send_event && OnServer())
		{
			//уничтожить подсоединенную вещь из инвентаря
//.			pIItem->Drop					();
			pIItem->object().DestroyObject	();
		};

		UpdateAltScope();
		UpdateAddonsVisibility();
		InitAddons();

		return true;
	}
	else
		return inherited::Attach(pIItem, b_send_event);
}


bool CWeaponMagazined::Detach(const char* item_section_name, bool b_spawn_item)
{
	if (m_eScopeStatus == CSE_ALifeItemWeapon::eAddonAttachable && DetachScope(item_section_name, b_spawn_item))
	{
		if ((m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonScope) == 0)
		{
			Msg("ERROR: scope addon already detached.");
			return true;
		}

		m_flagsAddOnState &= ~CSE_ALifeItemWeapon::eWeaponAddonScope;
		
		UpdateAltScope();
		UpdateAddonsVisibility();
		InitAddons();

		return CInventoryItemObject::Detach(item_section_name, b_spawn_item);
	}
	else if (m_eSilencerStatus == CSE_ALifeItemWeapon::eAddonAttachable && (m_sSilencerName == item_section_name))
	{
		if ((m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonSilencer) == 0)
		{
			Msg("ERROR: silencer addon already detached.");
			return true;
		}

		m_flagsAddOnState &= ~CSE_ALifeItemWeapon::eWeaponAddonSilencer;

		UpdateAddonsVisibility();
		InitAddons();
		return CInventoryItemObject::Detach(item_section_name, b_spawn_item);
	}
	else if (m_eGrenadeLauncherStatus == CSE_ALifeItemWeapon::eAddonAttachable && (m_sGrenadeLauncherName == item_section_name))
	{
		if ((m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher) == 0)
		{
			Msg("ERROR: grenade launcher addon already detached.");
			return true;
		}

		m_flagsAddOnState &= ~CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher;

		UpdateAddonsVisibility();
		InitAddons();
		return CInventoryItemObject::Detach(item_section_name, b_spawn_item);
	}
	else if (m_eLaserDesignatorStatus == ALife::eAddonAttachable &&
		(m_sLaserName == item_section_name))
	{
		if ((m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonLaserDesignator) == 0)
		{
			Msg("ERROR: laser designator addon already detached.");
			return true;
		}
		m_flagsAddOnState &= ~CSE_ALifeItemWeapon::eWeaponAddonLaserDesignator;

		UpdateAddonsVisibility();
		InitAddons();
		return CInventoryItemObject::Detach(item_section_name, b_spawn_item);
	}
	else if (m_eTacticalTorchStatus == ALife::eAddonAttachable &&
		(m_sTacticalTorchName == item_section_name))
	{
		if ((m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonTacticalTorch) == 0)
		{
			Msg("ERROR: tactical torch addon already detached.");
			return true;
		}
		m_flagsAddOnState &= ~CSE_ALifeItemWeapon::eWeaponAddonTacticalTorch;

		UpdateAddonsVisibility();
		InitAddons();
		return CInventoryItemObject::Detach(item_section_name, b_spawn_item);
	}
	else
		return inherited::Detach(item_section_name, b_spawn_item);;
}

bool CWeaponMagazined::DetachScope(const char* item_section_name, bool b_spawn_item)
{
	bool detached = false;
	SCOPES_VECTOR_IT it = m_scopes.begin();
	shared_str iter_scope_name = "none";
	for (; it != m_scopes.end(); it++)
	{
		if (bUseAltScope)
		{
			iter_scope_name = (*it);
		}
		else
		{
			iter_scope_name = pSettings->r_string((*it), "scope_name");
		}
		if (!xr_strcmp(iter_scope_name, item_section_name))
		{
			m_cur_scope = NULL;
			m_cur_scope_bone = NULL;
			m_bAltZoomActive = false;
			detached = true;
		}
	}
	return detached;
}

void CWeaponMagazined::InitAddons()
{
	m_zoom_params.m_fIronSightZoomFactor = READ_IF_EXISTS( pSettings, r_float, cNameSect(), "ironsight_zoom_factor", 50.0f );

	SetAnimFlag(ANM_SHOT_AIM,		"anm_shots_when_aim");
	SetAnimFlag(ANM_SHOT_AIM_GL,	"anm_shots_w_gl_when_aim");

	m_weapon_attaches.clear();

	if (IsScopeAttached())
	{
		if (m_eScopeStatus == ALife::eAddonAttachable)
		{
			LoadCurrentScopeParams(GetScopeName().c_str());

			if (pSettings->line_exist(m_scopes[m_cur_scope], "bones"))
			{
				pcstr ScopeBone = pSettings->r_string(m_scopes[m_cur_scope], "bones");
				m_cur_scope_bone = ScopeBone;
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

		LoadLights(*cNameSect(), "");
		//ResetSilencerKoeffs();
	}

	if (m_sGrenadeLauncherAttachSection.size() && pSettings->line_exist(m_sGrenadeLauncherAttachSection, "attach_hud_visual"))
	{
		if (IsGrenadeLauncherAttached())
			WeaponAttach().CreateAttach(m_sGrenadeLauncherAttachSection, m_weapon_attaches);
		else
			WeaponAttach().RemoveAttach(m_sGrenadeLauncherAttachSection, m_weapon_attaches);
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

void CWeaponMagazined::ApplySilencerKoeffs	()
{
	float BHPk = 1.0f, BSk = 1.0f;
	float FDB_k = 1.0f, CD_k = 1.0f;
	
	if (pSettings->line_exist(m_sSilencerName, "bullet_hit_power_k"))
	{
		BHPk = pSettings->r_float(m_sSilencerName, "bullet_hit_power_k");
		clamp(BHPk, 0.0f, 1.0f);
	};
	if (pSettings->line_exist(m_sSilencerName, "bullet_speed_k"))
	{
		BSk = pSettings->r_float(m_sSilencerName, "bullet_speed_k");
		clamp(BSk, 0.0f, 1.0f);
	};
	if (pSettings->line_exist(m_sSilencerName, "fire_dispersion_base_k"))
	{
		FDB_k = pSettings->r_float(m_sSilencerName, "fire_dispersion_base_k");
//		clamp(FDB_k, 0.0f, 1.0f);
	};
	if (pSettings->line_exist(m_sSilencerName, "cam_dispersion_k"))
	{
		CD_k = pSettings->r_float(m_sSilencerName, "cam_dispersion_k");
		clamp(CD_k, 0.0f, 1.0f);
	};

	//fHitPower			= fHitPower*BHPk;
	fvHitPower			.mul(BHPk);
	fHitImpulse			*= BSk;
	m_fStartBulletSpeed *= BSk;
	fireDispersionBase	*= FDB_k;
	camDispersion		*= CD_k;
	camDispersionInc	*= CD_k;
}

//виртуальные функции для проигрывания анимации HUD
void CWeaponMagazined::PlayAnimShow()
{
	VERIFY(GetState() == eShowing);

	if (iAmmoElapsed >= 1)
		m_opened = false;
	else
		m_opened = true;

	HUD_VisualBulletUpdate();

	if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_SHOW_EMPTY))
		PlayHUDMotion("anm_show_empty", FALSE, this, GetState());
	else if (IsMisfire() && isHUDAnimationExist("anm_show_jammed"))
		PlayHUDMotion("anm_show_jammed", false, this, GetState());
	else
		PlayHUDMotionIfExists({"anim_draw", "anm_show"}, false, GetState());
}

void CWeaponMagazined::PlayAnimHide()
{
	VERIFY(GetState()==eHiding);

	if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_HIDE_EMPTY))
		PlayHUDMotion("anm_hide_empty", TRUE, this, GetState());
	else if (IsMisfire() && isHUDAnimationExist("anm_hide_jammed"))
		PlayHUDMotion("anm_hide_jammed", true, this, GetState());
	else
		PlayHUDMotionIfExists({ "anim_holster", "anm_hide" }, true, GetState());
}

void CWeaponMagazined::PlayAnimBore()
{
	inherited::PlayAnimBore();
}

void CWeaponMagazined::PlayAnimIdleSprint()
{
	if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_SPRINT_EMPTY))
		PlayHUDMotion("anm_idle_sprint_empty", TRUE, NULL, GetState());
	else if (IsMisfire() && isHUDAnimationExist("anm_idle_sprint_jammed"))
		PlayHUDMotion("anm_idle_sprint_jammed", true, nullptr, GetState());
	else
		inherited::PlayAnimIdleSprint();
}

void CWeaponMagazined::PlayAnimIdleMoving()
{
	if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_MOVING_EMPTY))
		PlayHUDMotion("anm_idle_moving_empty", TRUE, NULL, GetState());
	else if (IsMisfire() && isHUDAnimationExist("anm_idle_moving_jammed"))
		PlayHUDMotion("anm_idle_moving_jammed", true, nullptr, GetState());
	else
		inherited::PlayAnimIdleMoving();
}

void CWeaponMagazined::PlayAnimReload()
{
	VERIFY(GetState() == eReload);

	if (m_bIsRevolver)
	{
		string128 anmReloadName;
		strconcat(sizeof(anmReloadName), anmReloadName, "anm_reload_", (iAmmoElapsed > 0) ? std::to_string(iAmmoElapsed).c_str() : "empty");

		if (isHUDAnimationExist(anmReloadName))
			PlayHUDMotionIfExists({ anmReloadName, "anm_reload" }, true, GetState());
	}
	else
	{
		if (iAmmoElapsed == 0)
			PlayHUDMotionIfExists({ "anm_reload_empty", "anm_reload", "anim_reload" }, true, GetState());
		else
			PlayHUDMotionIfExists({ "anim_reload", "anm_reload" }, TRUE, GetState());
	}
}

void CWeaponMagazined::PlayAnimAim()
{
	if (IsRotatingToZoom())
	{
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
				return;
			}
		}
	}

	if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_AIM_EMPTY))
		PlayHUDMotion("anm_idle_aim_empty", TRUE, NULL, GetState());
	else if (IsMisfire() && isHUDAnimationExist("anm_idle_aim_jammed"))
		PlayHUDMotion("anm_idle_aim_jammed", true, nullptr, GetState());
	else
		PlayHUDMotionIfExists({ "anim_idle_aim", "anm_idle_aim" }, true, GetState());
}

bool CWeaponMagazined::PlayAnimAimEnd()
{
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
		PlayAnimAim();
	else if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_IDLE_EMPTY) && !TryPlayAnimIdle() && !IsRotatingFromZoom())
		PlayHUDMotion("anm_idle_empty", TRUE, NULL, GetState());
	else if (IsMisfire() && isHUDAnimationExist("anm_idle_jammed") && !TryPlayAnimIdle() && !IsRotatingFromZoom())
		PlayHUDMotion("anm_idle_jammed", true, nullptr, GetState());
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
	VERIFY(GetState() == eFire);

	if ((IsRotatingToZoom() && m_zoom_params.m_fZoomRotationFactor != 0.0f) || (IsRotatingFromZoom() && m_zoom_params.m_fZoomRotationFactor != 1.0f))
		return;

	string_path guns_shoot_anm{};
	strconcat(sizeof(guns_shoot_anm), guns_shoot_anm, (isHUDAnimationExist("anm_shoot") ? "anm_shoot" : "anm_shots"), (iAmmoElapsed == 1) ? "_last" : "", (IsZoomed() && !IsRotatingToZoom()) ? (IsScopeAttached() ? "_aim_scope" : "_aim") : "", (IsSilencerAttached() && m_bUseAimSilShotAnim) ? "_sil" : "");

	//HUD_VisualBulletUpdate();

	if (iAmmoElapsed == 1)
		PlayHUDMotionIfExists({ guns_shoot_anm, "anm_shot_l", "anim_shoot", "anm_shots" }, false, GetState());
	else
		PlayHUDMotionIfExists({ guns_shoot_anm, "anim_shoot", "anm_shots" }, false, GetState());
}

void CWeaponMagazined::PlayAnimFakeShoot()
{
	if ((IsRotatingToZoom() && m_zoom_params.m_fZoomRotationFactor != 0.0f) || (IsRotatingFromZoom() && m_zoom_params.m_fZoomRotationFactor != 1.0f))
		return;

	string128 guns_fakeshoot_anm{};
	strconcat(sizeof(guns_fakeshoot_anm), guns_fakeshoot_anm, ("anm_fakeshoot"), (IsZoomed() && !IsRotatingToZoom()) ? "_aim" : "", IsMisfire() ? "_jammed" : IsEmptyMagazine() ? "_empty" : "", IsGrenadeLauncherAttached() ? (!IsGrenadeMode() ? "_w_gl" : "_g") : "");

	if (isHUDAnimationExist(guns_fakeshoot_anm))
	{
		PlayHUDMotionNew(guns_fakeshoot_anm, true, GetState());
	}
	else if (guns_fakeshoot_anm && strstr(guns_fakeshoot_anm, "_jammed"))
	{
		char new_guns_fakeshoot_anm[256];
		strcpy(new_guns_fakeshoot_anm, guns_fakeshoot_anm);
		new_guns_fakeshoot_anm[strlen(guns_fakeshoot_anm) - strlen("_jammed")] = '\0';

		if (isHUDAnimationExist(new_guns_fakeshoot_anm))
		{
			PlayHUDMotionNew(new_guns_fakeshoot_anm, true, GetState());
		}
	}
	else if (guns_fakeshoot_anm && strstr(guns_fakeshoot_anm, "_empty"))
	{
		char new_guns_fakeshoot_anm[256];
		strcpy(new_guns_fakeshoot_anm, guns_fakeshoot_anm);
		new_guns_fakeshoot_anm[strlen(guns_fakeshoot_anm) - strlen("_empty")] = '\0';

		if (isHUDAnimationExist(new_guns_fakeshoot_anm))
		{
			PlayHUDMotionNew(new_guns_fakeshoot_anm, true, GetState());
		}
	}
}

void CWeaponMagazined::PlayAnimDeviceSwitch()
{
	CActor* actor = Actor();
	CTorch* torch = smart_cast<CTorch*>(Actor()->inventory().ItemFromSlot(TORCH_SLOT));

	if (!actor->GetNightVision())
		actor->SetNightVision(xr_new<CNightVisionEffector>(actor->cNameSect()));

	CNightVisionEffector* nvg = Actor()->GetNightVision();

	PlaySound(HeadLampSwitch && torch ? (!torch->IsSwitchedOn() ? "sndHeadlampOn" : "sndHeadlampOff") : NightVisionSwitch && nvg ? (!nvg->IsActive() ? "sndNvOn" : "sndNvOff") : CleanMaskAction ? "sndCleanMask" : "", get_LastFP());

	string128 guns_device_switch_anm{};
	strconcat(sizeof(guns_device_switch_anm), guns_device_switch_anm, HeadLampSwitch && torch ? (!torch->IsSwitchedOn() ? "anm_headlamp_on" : "anm_headlamp_off") : NightVisionSwitch && nvg ? (!nvg->IsActive() ? "anm_nv_on" : "anm_nv_off") : CleanMaskAction ? "anm_clean_mask" : "", IsMisfire() ? "_jammed" : IsEmptyMagazine() ? "_empty" : "", IsGrenadeLauncherAttached() ? (!IsGrenadeMode() ? "_w_gl" : "_g") : "");

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
	}
}

void CWeaponMagazined::OnZoomIn			()
{
	inherited::OnZoomIn();

	if (GetState() == eIdle)
		PlayAnimIdle();

	//Alundaio: callback not sure why vs2013 gives error, it's fine
	CGameObject* object = smart_cast<CGameObject*>(H_Parent());

	if (object)
		object->callback(GameObject::eOnWeaponZoomIn)(object->lua_game_object(), this->lua_game_object());
	//-Alundaio

	CActor* pActor = smart_cast<CActor*>(H_Parent());
	if (pActor)
	{
		if (m_sounds.FindSoundItem("sndZoomIn", false))
			m_sounds.PlaySound("sndZoomIn", get_LastFP(), H_Root(), !!GetHUDmode(), false, (u8)-1);

		CEffectorZoomInertion* S = smart_cast<CEffectorZoomInertion*>	(pActor->Cameras().GetCamEffector(eCEZoom));
		if (!S)	
		{
			S = (CEffectorZoomInertion*)pActor->Cameras().AddCamEffector(xr_new<CEffectorZoomInertion> ());
			S->Init(this);
		};
		S->SetRndSeed(pActor->GetZoomRndSeed());
		R_ASSERT				(S);
	}
}
void CWeaponMagazined::OnZoomOut		()
{
	if (!m_zoom_params.m_bIsZoomModeNow) return;

	inherited::OnZoomOut();

	if (GetState() == eFire)
		PlayAnimAimEnd();

	if (GetState() == eIdle)
		PlayAnimIdle();

	//Alundaio
	CGameObject* object = smart_cast<CGameObject*>(H_Parent());
	if (object)
		object->callback(GameObject::eOnWeaponZoomOut)(object->lua_game_object(), this->lua_game_object());
	//-Alundaio

	CActor* pActor = smart_cast<CActor*>(H_Parent());
	
	if (pActor)
	{
		if (m_sounds.FindSoundItem("sndZoomOut", false))
			m_sounds.PlaySound("sndZoomOut", get_LastFP(), H_Root(), !!GetHUDmode(), false, (u8)-1);

		pActor->Cameras().RemoveCamEffector(eCEZoom);
	}

}

//переключение режимов стрельбы одиночными и очередями
bool CWeaponMagazined::SwitchMode			()
{
	if (eIdle != GetState() || IsPending())
		return false;

	if (SingleShotMode())
		m_iQueueSize = WEAPON_ININITE_QUEUE;
	else
		m_iQueueSize = 1;
	
	PlaySound	("sndEmptyClick", get_LastFP());

	return true;
}

void	CWeaponMagazined::OnNextFireMode		()
{
	if (!m_bHasDifferentFireModes) return;

	if (isHUDAnimationExist("anm_changefiremode_from_1_to_a") || isHUDAnimationExist("anm_changefiremode"))
		SwitchState(eFiremodeNext);

	if (GetState() != eIdle) return;
	m_iCurFireMode = (m_iCurFireMode+1+m_aFireModes.size()) % m_aFireModes.size();
	SetQueueSize(GetCurrentFireMode());
};

void	CWeaponMagazined::OnPrevFireMode		()
{
	if (!m_bHasDifferentFireModes) return;

	if (isHUDAnimationExist("anm_changefiremode_from_1_to_a") || isHUDAnimationExist("anm_changefiremode"))
		SwitchState(eFiremodePrev);

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
	if (m_iQueueSize == -1)
		strcpy_s(m_sCurFireMode, " (A)");
	else
		sprintf_s(m_sCurFireMode, " (%d)", m_iQueueSize);
};

float	CWeaponMagazined::GetWeaponDeterioration	()
{
	if (!m_bHasDifferentFireModes || m_iPrefferedFireMode == -1 || u32(GetCurrentFireMode()) <= u32(m_iPrefferedFireMode)) 
		return inherited::GetWeaponDeterioration();
	return m_iShotNum*conditionDecreasePerShot;
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
	//	if (Level().IsDemoPlay())
	//		Msg("CWeapon::net_Import [%d]", ID());

	inherited::net_Import (P);

	m_iCurFireMode = P.r_u8();
	SetQueueSize(GetCurrentFireMode());
}
#include "string_table.h"
bool CWeaponMagazined::GetBriefInfo(II_BriefInfo& info)
{
	string32	int_str;
	int	ae					= GetAmmoElapsed();
	int	ac					= GetAmmoCurrent();

	if (!unlimited_ammo())
		xr_sprintf(int_str, "%d/%d", ae, ac - ae);
	else
		xr_sprintf(int_str, "%d/--", ae);

	info.cur_ammo = int_str;

	if (HasFireModes())
	{
		if (m_iQueueSize == WEAPON_ININITE_QUEUE)
		{
			info.fire_mode = "A";
		}
		else
		{
			xr_sprintf(int_str, "%d", m_iQueueSize);
			info.fire_mode = int_str;
		}
	}
	else
		info.fire_mode = "";
	
	if (ae == 0 || 0 == m_magazine.size())
		info.icon		= *m_ammoTypes[m_ammoType];
	else
		info.icon		= *m_ammoTypes[m_magazine.back().m_LocalAmmoType];

	GetAmmoCurrent();//update m_BriefInfo_CalcFrame
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

		xr_sprintf(int_str, "%d", GetAmmoCount(0)); // !!!!!!!!!!! == 0 temp
		if (m_ammoType == 0)
			info.fmj_ammo = int_str;
		else
			info.ap_ammo = int_str;

		if (at_size == 2)
		{
			xr_sprintf(int_str, "%d", GetAmmoCount(1));
			if (m_ammoType == 0)
				info.ap_ammo = int_str;
			else
				info.fmj_ammo = int_str;
		}
		else
		{
			info.ap_ammo = "";
		}
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

	if (GameConstants::GetMergedAmmoLineWithFiremodes() && HasFireModes())
	{
		string128 out_str = "";
		xr_sprintf(out_str, "%s (%s)", info.name.c_str(), info.fire_mode.c_str());
		info.name = out_str;
	}
	return true;
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