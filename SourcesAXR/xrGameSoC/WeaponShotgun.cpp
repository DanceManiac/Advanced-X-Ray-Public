#include "stdafx.h"
#include "weaponshotgun.h"
#include "entity.h"
#include "ParticlesObject.h"
#include "xr_level_controller.h"
#include "inventory.h"
#include "level.h"
#include "actor.h"

CWeaponShotgun::CWeaponShotgun(void) : CWeaponCustomPistol("TOZ34")
{
	m_eSoundShotBoth		= ESoundTypes(SOUND_TYPE_WEAPON_SHOOTING);
	m_eSoundClose_2			= ESoundTypes(SOUND_TYPE_WEAPON_SHOOTING);
	m_eSoundAddCartridge	= ESoundTypes(SOUND_TYPE_WEAPON_SHOOTING);

	m_bOnlyTriStateWithScope = false;
	m_bIsCancelReloadNow	= false;
	m_bIsShotgun			= !m_bIsBoltRiffle;
}

CWeaponShotgun::~CWeaponShotgun(void)
{
	// sounds
}

void CWeaponShotgun::net_Destroy()
{
	inherited::net_Destroy();
}

void CWeaponShotgun::Load	(LPCSTR section)
{
	inherited::Load		(section);

	// Звук и анимация для выстрела дуплетом
	m_sounds.LoadSound(section, "snd_shoot_duplet", "sndShotBoth", false, m_eSoundShotBoth);
	
	if (pSettings->line_exist(section, "tri_state_reload"))
	{
		m_bTriStateReload = !!pSettings->r_bool(section, "tri_state_reload");
	};

	if (m_bTriStateReload)
	{
		m_sounds.LoadSound(section, "snd_open_weapon", "m_sndOpen", false, m_eSoundOpen);

		if (WeaponSoundExist(section, "snd_open_weapon_empty", true))
			m_sounds.LoadSound(section, "snd_open_weapon_empty", "sndOpenEmpty", false, m_eSoundAddCartridge);

		m_sounds.LoadSound(section, "snd_add_cartridge", "sndAddCartridge", false, m_eSoundAddCartridge);

		if (WeaponSoundExist(section, "snd_add_cartridge_empty", true))
			m_sounds.LoadSound(section, "snd_add_cartridge_empty", "sndAddCartridgeEmpty", false, m_eSoundAddCartridge);

		if (WeaponSoundExist(section, "snd_reload_misfire", true))
			m_sounds.LoadSound(section, "snd_reload_misfire", "sndReloadMisfire", false, m_eSoundOpen);

		if (WeaponSoundExist(section, "snd_reload_misfire_empty", true))
			m_sounds.LoadSound(section, "snd_reload_misfire_empty", "sndReloadMisfireEmpty", false, m_eSoundOpen);

		m_sounds.LoadSound(section, "snd_close_weapon", "m_sndClose_2", false, m_eSoundClose_2);

		if (WeaponSoundExist(section, "snd_close_weapon_empty,", true))
			m_sounds.LoadSound(section, "snd_close_weapon_empty,", "sndClose_2_Empty", false, m_eSoundClose_2);
	};

	m_bIsBoltRiffle = READ_IF_EXISTS(pSettings, r_bool, section, "is_bolt_rifle", false);
	m_bOnlyTriStateWithScope = READ_IF_EXISTS(pSettings, r_bool, section, "only_tri_state_with_scope", false);
}

void CWeaponShotgun::OnShot () 
{
//.?std::swap(m_pHUD->FirePoint(), m_pHUD->FirePoint2());
//.	std::swap(vLoadedFirePoint, vLoadedFirePoint2);
	inherited::OnShot();
}

void CWeaponShotgun::Fire2Start () 
{
	if (IsPending())
		return;

	inherited::Fire2Start();

	if (IsValid())
	{
		if (!IsWorking())
		{
			if (GetState()==eReload)		return;
			if (GetState()==eShowing)		return;
			if (GetState()==eHiding)		return;

			if (!iAmmoElapsed)	
			{
				CWeapon::FireStart			();
				SwitchState					(eMagEmpty);
			}
			else					
			{
				CWeapon::FireStart			();
				SwitchState					((iAmmoElapsed < iMagazineSize)?eFire:eFire2);
			}
		}
	}else{
		if (!iAmmoElapsed)	
			SwitchState						(eMagEmpty);
	}
}

void CWeaponShotgun::Fire2End () 
{
	inherited::Fire2End();
	FireEnd();
}


void CWeaponShotgun::OnShotBoth()
{
	//если патронов меньше, чем 2 
	if (iAmmoElapsed < iMagazineSize) 
	{ 
		OnShot(); 
		return; 
	}

	//звук выстрела дуплетом
	PlaySound			("sndShotBoth", get_LastFP());
	
	// Camera
	AddShotEffector		();
	
	// анимация дуплета
	PlayHUDMotionIfExists({"anim_shoot_both", "anm_shots_both"}, false, GetState());
	
	// Shell Drop
	Fvector vel; 
	PHGetLinearVell		(vel);
	OnShellDrop			(get_LastSP(), vel);

	//огонь из 2х стволов
	StartFlameParticles			();
	StartFlameParticles2		();

	//дым из 2х стволов
	CParticlesObject* pSmokeParticles = NULL;
	CShootingObject::StartParticles(pSmokeParticles, *m_sSmokeParticlesCurrent, get_LastFP(), m_zero_vel, true);
	pSmokeParticles = NULL;
	CShootingObject::StartParticles(pSmokeParticles, *m_sSmokeParticlesCurrent, get_LastFP2(), m_zero_vel, true);

}

void CWeaponShotgun::switch2_Fire	()
{
	inherited::switch2_Fire	();
	bWorking = false;
}

void CWeaponShotgun::switch2_Fire2	()
{
	VERIFY(fTimeToFire>0.f);

	if (fTime<=0)
	{
		// Fire
		Fvector						p1, d; 
		p1.set	(get_LastFP()); 
		d.set	(get_LastFD());

		CEntity*					E = smart_cast<CEntity*>(H_Parent());
		if (E){
		/*CInventoryOwner* io		= smart_cast<CInventoryOwner*>(H_Parent());
			if (NULL == io->inventory().ActiveItem())
			{
			Log("current_state", GetState() );
			Log("next_state", GetNextState());
			Log("state_time", m_dwStateTime);
			Log("item_sect", cNameSect().c_str());
			Log("H_Parent", H_Parent()->cNameSect().c_str());
			}	*/
			E->g_fireParams		(this, p1,d);
		}
		
		OnShotBoth						();

		//выстрел из обоих стволов
		FireTrace					(p1,d);
		FireTrace					(p1,d);
		fTime						+= fTimeToFire*2.f;

		// Patch for "previous frame position" :)))
		m_dwFP_Frame					= 0xffffffff;
		m_dwXF_Frame					= 0xffffffff;
	}
}

void CWeaponShotgun::UpdateSounds	()
{
	inherited::UpdateSounds();

	m_sounds.SetPosition("sndShotBoth", get_LastFP());
}

bool CWeaponShotgun::Action			(s32 cmd, u32 flags) 
{
	if (inherited::Action(cmd, flags)) return true;

	if (	m_bTriStateReload && GetState()==eReload && !IsMisfire() &&
		cmd==kWPN_FIRE && flags&CMD_START &&
		m_sub_state==eSubstateReloadInProcess		)//остановить перезагрузку
	{
		AddCartridge(1);
		m_sub_state = eSubstateReloadEnd;
		m_bIsCancelReloadNow = true;
		return true;
	}
	//если оружие чем-то занято, то ничего не делать
	if (IsPending()) return false;

	switch(cmd) 
	{
		case kWPN_ZOOM : 
			{
				if (flags&CMD_START) Fire2Start();
				else Fire2End();
			}
			return true;
	}
	return false;
}

void CWeaponShotgun::OnAnimationEnd(u32 state) 
{
	switch (state)
	{
	case eFire:
		{
			if (IsMisfire())
				SwitchState(eIdle);
		} break;
	}

	if (!m_bTriStateReload || (m_bIsBoltRiffle && !(IsScopeAttached() && m_bOnlyTriStateWithScope) && !iAmmoElapsed && HaveCartridgeInInventory(iMagazineSize)) || state != eReload)
		return inherited::OnAnimationEnd(state);

	switch(m_sub_state)
	{
		case eSubstateReloadBegin:
		{
			m_sub_state = IsMisfire() ? eSubstateReloadEnd : eSubstateReloadInProcess;
			SwitchState(eReload);
		}break;

		case eSubstateReloadInProcess:
		{
			if ( 0 != AddCartridge(1) )
			{
				m_sub_state = eSubstateReloadEnd;
			}
			SwitchState(eReload);
		}break;

		case eSubstateReloadEnd:
		{
			m_sub_state = eSubstateReloadBegin;
			SwitchState(eIdle);
		}break;
		
	};
}

void CWeaponShotgun::Reload() 
{
	if (!m_bTriStateReload || (m_bIsBoltRiffle && !(IsScopeAttached() && m_bOnlyTriStateWithScope) && !iAmmoElapsed && HaveCartridgeInInventory(iMagazineSize)))
	{
		inherited::Reload();
	}
	else
		TriStateReload();
}

void CWeaponShotgun::TriStateReload()
{
	if ((m_magazine.size() == (u32)iMagazineSize) && !IsMisfire())
		return;

	if (HaveCartridgeInInventory(1) || IsMisfire())
	{
		m_sub_state = eSubstateReloadBegin;
		SwitchState(eReload);
	}
}

void CWeaponShotgun::OnStateSwitch(u32 S, u32 oldState)
{
	if (!m_bIsCancelReloadNow && (!m_bTriStateReload || (m_bIsBoltRiffle && !(IsScopeAttached() && m_bOnlyTriStateWithScope) && !iAmmoElapsed && HaveCartridgeInInventory(iMagazineSize)) || S != eReload))
	{
		inherited::OnStateSwitch(S, oldState);
		return;
	}

	CWeapon::OnStateSwitch(S, oldState);

	if ((m_magazine.size() == (u32)iMagazineSize) && !IsMisfire() || !HaveCartridgeInInventory(1) && !IsMisfire() || m_bIsCancelReloadNow)
	{
			switch2_EndReload		();
			m_sub_state = eSubstateReloadEnd;
			m_bIsCancelReloadNow = false;
	};

	switch (m_sub_state)
	{
	case eSubstateReloadBegin:
		if (HaveCartridgeInInventory(1) || IsMisfire())
			switch2_StartReload	();
		break;
	case eSubstateReloadInProcess:
			if ( HaveCartridgeInInventory(1) )
				switch2_AddCartgidge	();
		break;
	case eSubstateReloadEnd:
			switch2_EndReload		();
		break;
	};
}

void CWeaponShotgun::switch2_StartReload()
{
	if (!IsMisfire())
		PlaySound((iAmmoElapsed == 0 && m_sounds.FindSoundItem("sndOpenEmpty", false)) ? "sndOpenEmpty" : "sndOpen", get_LastFP());
	else
		PlaySound((iAmmoElapsed == 1 && m_sounds.FindSoundItem("sndReloadMisfireEmpty", false)) ? "sndReloadMisfireEmpty" : "sndReloadMisfire", get_LastFP());

	PlayAnimOpenWeapon	();
	SetPending(TRUE);
}

void CWeaponShotgun::switch2_AddCartgidge	()
{
	if (WeaponSoundExist(m_section_id.c_str(), "snd_add_cartridge_empty") && iAmmoElapsed == 0)
		PlaySound("sndAddCartridgeEmpty", get_LastFP());
	else
		PlaySound("sndAddCartridge", get_LastFP());

	PlayAnimAddOneCartridgeWeapon();
	SetPending(TRUE);
}

void CWeaponShotgun::switch2_EndReload	()
{
	SetPending(TRUE);

	if (!IsMisfire())
	{
		PlaySound((iAmmoElapsed == 0 && m_sounds.FindSoundItem("sndClose_2_Empty", false)) ? "sndClose_2_Empty" : "sndClose_2", get_LastFP());
		PlayAnimCloseWeapon();
	}
	else
	{
		bMisfire = false;

		if (GetAmmoElapsed() > 0) //xrKrodin: хз, надо ли удалять заклинивший патрон в данном случае. Надо подумать над этим.
			SetAmmoElapsed(GetAmmoElapsed() - 1);

		SwitchState(eIdle);
	}
}

void CWeaponShotgun::PlayAnimOpenWeapon()
{
	VERIFY(GetState()==eReload);

	if (IsMisfire())
	{
		if (iAmmoElapsed == 1)
			PlayHUDMotionIfExists({ "anm_reload_misfire_empty", "anm_reload_misfire", "anm_close", "anim_close_weapon" }, true, GetState());
		else
			PlayHUDMotionIfExists({ "anm_reload_misfire", "anm_close", "anim_close_weapon" }, true, GetState());
	}
	else if (iAmmoElapsed == 0)
		PlayHUDMotionIfExists({ "anm_open_empty", "anm_open_weapon", "anm_open", "anim_open_weapon" }, false, GetState());
	else
		PlayHUDMotionIfExists({"anim_open_weapon", "anm_open_weapon", "anm_open"}, FALSE, GetState());
}
void CWeaponShotgun::PlayAnimAddOneCartridgeWeapon()
{
	VERIFY(GetState()==eReload);

	if (iAmmoElapsed == 0)
		PlayHUDMotionIfExists({ "anm_add_cartridge_empty", "anm_add_cartridge", "anim_add_cartridge" }, true, GetState());
	else
		PlayHUDMotionIfExists({"anim_add_cartridge", "anm_add_cartridge"}, FALSE, GetState());

	if (m_bBulletsVisualization)
		HUD_VisualBulletUpdate();
}
void CWeaponShotgun::PlayAnimCloseWeapon()
{
	VERIFY(GetState()==eReload);

	if (iAmmoElapsed == 0)
		PlayHUDMotionIfExists({ "anm_close_weapon_empty", "anm_close_empty", "anm_close_weapon", "anm_close", "anim_close_weapon" }, true, GetState());
	else
		PlayHUDMotionIfExists({ "anim_close_weapon", "anm_close_weapon", "anm_close" }, true, GetState());

	if (m_bBulletsVisualization)
		HUD_VisualBulletUpdate();
}

void CWeaponShotgun::PlayAnimAim()
{
	if (IsRotatingToZoom())
	{
		if (isHUDAnimationExist("anm_idle_aim_start"))
		{
			PlayHUDMotionNew("anm_idle_aim_start", true, GetState());
			return;
		}
	}

	if (const char* guns_aim_anm = GetAnimAimName())
	{
		if (isHUDAnimationExist(guns_aim_anm))
		{
			PlayHUDMotionNew(guns_aim_anm, true, GetState());
			return;
		}
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

	if (isHUDAnimationExist("anm_idle_aim"))
		PlayHUDMotion("anm_idle_aim", TRUE, NULL, GetState());
	else if (isHUDAnimationExist("anim_idle_aim"))
		PlayHUDMotion("anim_idle_aim", TRUE, NULL, GetState());
}

bool CWeaponShotgun::HaveCartridgeInInventory		(u8 cnt)
{
	if (unlimited_ammo()) return true;
	m_pAmmo = NULL;
	if (m_pInventory) 
	{
		//попытаться найти в инвентаре патроны текущего типа 
		m_pAmmo = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(*m_ammoTypes[m_ammoType]));
		
		if (!m_pAmmo)
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
	return (m_pAmmo!=NULL)&&(m_pAmmo->m_boxCurr>=cnt) ;
}

u8 CWeaponShotgun::AddCartridge		(u8 cnt)
{
	if (IsMisfire())	bMisfire = false;

	if (m_set_next_ammoType_on_reload != u32(-1)){
		m_ammoType						= m_set_next_ammoType_on_reload;
		m_set_next_ammoType_on_reload	= u32(-1);

	}

	if ( !HaveCartridgeInInventory(1) )
		return 0;

	VERIFY((u32)iAmmoElapsed == m_magazine.size());


	if (m_DefaultCartridge.m_LocalAmmoType != m_ammoType)
		m_DefaultCartridge.Load(*m_ammoTypes[m_ammoType], u8(m_ammoType));
	CCartridge l_cartridge = m_DefaultCartridge;
	while(cnt)// && m_pAmmo->Get(l_cartridge)) 
	{
		if (!unlimited_ammo())
		{
			if (!m_pAmmo->Get(l_cartridge)) break;
		}
		--cnt;
		++iAmmoElapsed;
		l_cartridge.m_LocalAmmoType = u8(m_ammoType);
		m_magazine.push_back(l_cartridge);
//		m_fCurrentCartirdgeDisp = l_cartridge.m_kDisp;
	}
	m_ammoName = (m_pAmmo) ? m_pAmmo->m_nameShort : NULL;

	VERIFY((u32)iAmmoElapsed == m_magazine.size());

	//выкинуть коробку патронов, если она пустая
	if (m_pAmmo && !m_pAmmo->m_boxCurr && OnServer()) 
		m_pAmmo->SetDropManual(TRUE);

	return cnt;
}

void	CWeaponShotgun::net_Export	(NET_Packet& P)
{
	inherited::net_Export(P);	
	P.w_u8(u8(m_magazine.size()));	
	for (u32 i=0; i<m_magazine.size(); i++)
	{
		CCartridge& l_cartridge = *(m_magazine.begin()+i);
		P.w_u8(l_cartridge.m_LocalAmmoType);
	}
}

void	CWeaponShotgun::net_Import	(NET_Packet& P)
{
	inherited::net_Import(P);	
	u8 AmmoCount = P.r_u8();
	for (u32 i=0; i<AmmoCount; i++)
	{
		u8 LocalAmmoType = P.r_u8();
		if (i>=m_magazine.size()) continue;
		CCartridge& l_cartridge = *(m_magazine.begin()+i);
		if (LocalAmmoType == l_cartridge.m_LocalAmmoType) continue;
#ifdef DEBUG
		Msg("! %s reload to %s", *l_cartridge.m_ammoSect, *(m_ammoTypes[LocalAmmoType]));
#endif
		l_cartridge.Load(*(m_ammoTypes[LocalAmmoType]), LocalAmmoType); 
//		m_fCurrentCartirdgeDisp = m_DefaultCartridge.m_kDisp;		
	}
}
