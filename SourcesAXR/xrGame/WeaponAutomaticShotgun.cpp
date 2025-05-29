#include "stdafx.h"
#include "WeaponAutomaticShotgun.h"
#include "entity.h"
#include "ParticlesObject.h"
#include "xr_level_controller.h"
#include "inventory.h"
#include "level.h"
#include "actor.h"

CWeaponAutomaticShotgun::CWeaponAutomaticShotgun()
{
	m_eSoundClose_2			= ESoundTypes(SOUND_TYPE_WEAPON_SHOOTING);
	m_eSoundAddCartridge	= ESoundTypes(SOUND_TYPE_WEAPON_SHOOTING);

	m_bOnlyTriStateWithScope = false;
	m_bLastShotRPM			= true;
	m_bIsCancelReloadNow	= false;
}

CWeaponAutomaticShotgun::~CWeaponAutomaticShotgun()
{
}

void CWeaponAutomaticShotgun::Load(LPCSTR section)
{
	inherited::Load(section);

	if (pSettings->line_exist(section, "tri_state_reload"))
	{
		m_bTriStateReload = !!pSettings->r_bool(section, "tri_state_reload");
	};

	if (m_bTriStateReload)
	{
		m_sounds.LoadSound(section, "snd_open_weapon", "sndOpen", false, m_eSoundOpen);

		if (WeaponSoundExist(section, "snd_open_weapon_empty,", true))
			m_sounds.LoadSound(section, "snd_open_weapon_empty,", "sndOpenEmpty", false, m_eSoundAddCartridge);

		m_sounds.LoadSound(section, "snd_add_cartridge", "sndAddCartridge", false, m_eSoundAddCartridge);

		if (WeaponSoundExist(section, "snd_add_cartridge_empty", true))
			m_sounds.LoadSound(section, "snd_add_cartridge_empty", "sndAddCartridgeEmpty", false, m_eSoundAddCartridge);

		if (WeaponSoundExist(section, "snd_reload_misfire", true))
			m_sounds.LoadSound(section, "snd_reload_misfire", "sndReloadMisfire", false, m_eSoundOpen);

		m_sounds.LoadSound(section, "snd_close_weapon", "sndClose_2", false, m_eSoundClose_2);

		if (WeaponSoundExist(section, "snd_close_weapon_empty,", true))
			m_sounds.LoadSound(section, "snd_close_weapon_empty,", "sndClose_2_Empty", false, m_eSoundClose_2);
	};

	m_bIsBoltRiffle = READ_IF_EXISTS(pSettings, r_bool, section, "is_bolt_rifle", false);
	m_bOnlyTriStateWithScope = READ_IF_EXISTS(pSettings, r_bool, section, "only_tri_state_with_scope", false);
}

bool CWeaponAutomaticShotgun::Action(u16 cmd, u32 flags) 
{
	if(inherited::Action(cmd, flags)) return true;

	if(	m_bTriStateReload && GetState()==eReload && !IsMisfire() &&
		cmd==kWPN_FIRE && flags&CMD_START &&
		m_sub_state==eSubstateReloadInProcess		)//остановить перезагрузку
	{
		AddCartridge(1);
		m_sub_state = eSubstateReloadEnd;
		m_bIsCancelReloadNow = true;
		return true;
	}
	return false;
}

void CWeaponAutomaticShotgun::OnAnimationEnd(u32 state) 
{
	if(!m_bTriStateReload || (m_bIsBoltRiffle && !(IsScopeAttached() && m_bOnlyTriStateWithScope) && !iAmmoElapsed) || state != eReload)
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
			if( 0 != AddCartridge(1) )
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

void CWeaponAutomaticShotgun::Reload() 
{
	if (!m_bTriStateReload || (m_bIsBoltRiffle && !(IsScopeAttached() && m_bOnlyTriStateWithScope) && !iAmmoElapsed))
	{
		inherited::Reload();
	}
	else
		TriStateReload();
}

void CWeaponAutomaticShotgun::TriStateReload()
{
	if ((m_magazine.size() == (u32)iMagazineSize) && !IsMisfire())
		return;

	if (HaveCartridgeInInventory(1) || IsMisfire())
	{
		m_sub_state = eSubstateReloadBegin;
		SwitchState(eReload);
	}
}

void CWeaponAutomaticShotgun::OnStateSwitch	(u32 S)
{
	if(!m_bIsCancelReloadNow && (!m_bTriStateReload || (m_bIsBoltRiffle && !(IsScopeAttached() && m_bOnlyTriStateWithScope) && !iAmmoElapsed) || S != eReload))
	{
		inherited::OnStateSwitch(S);
		return;
	}

	CWeapon::OnStateSwitch(S);

	if ((m_magazine.size() == (u32)iMagazineSize) && !IsMisfire() || !HaveCartridgeInInventory(1) && !IsMisfire() || m_bIsCancelReloadNow)
	{
			switch2_EndReload		();
			m_sub_state = eSubstateReloadEnd;
			m_bIsCancelReloadNow = false;
			return;
	};

	switch (m_sub_state)
	{
	case eSubstateReloadBegin:
		if (HaveCartridgeInInventory(1) || IsMisfire())
			switch2_StartReload	();
		break;
	case eSubstateReloadInProcess:
			if( HaveCartridgeInInventory(1) )
				switch2_AddCartgidge	();
		break;
	case eSubstateReloadEnd:
			switch2_EndReload		();
		break;
	};
}

void CWeaponAutomaticShotgun::switch2_StartReload()
{
	if (!IsMisfire())
		PlaySound((iAmmoElapsed == 0 && m_sounds.FindSoundItem("sndOpenEmpty", false)) ? "sndOpenEmpty" : "sndOpen", get_LastFP());
	else
		PlaySound("sndReloadMisfire", get_LastFP());

	PlayAnimOpenWeapon	();
	SetPending			(TRUE);
}

void CWeaponAutomaticShotgun::switch2_AddCartgidge	()
{
	if (WeaponSoundExist(m_section_id.c_str(), "snd_add_cartridge_empty") && iAmmoElapsed == 0)
		PlaySound	("sndAddCartridgeEmpty", get_LastFP());
	else
		PlaySound	("sndAddCartridge", get_LastFP());

	PlayAnimAddOneCartridgeWeapon();
	SetPending			(TRUE);
}

void CWeaponAutomaticShotgun::switch2_EndReload	()
{
	SetPending			(TRUE);
	
	if (!IsMisfire())
	{
		PlaySound((iAmmoElapsed == 0 && m_sounds.FindSoundItem("sndClose_2_Empty", false)) ? "sndClose_2_Empty" : "sndClose_2", get_LastFP());
		PlayAnimCloseWeapon();
	}
	else
	{
		bMisfire = false;

		if (GetAmmoElapsed() > 0) //xrKrodin: хз, надо ли удал€ть заклинивший патрон в данном случае. Ќадо подумать над этим.
			SetAmmoElapsed(GetAmmoElapsed() - 1);

		SwitchState(eIdle);
	}
}

void CWeaponAutomaticShotgun::PlayAnimOpenWeapon()
{
	VERIFY(GetState()==eReload);

	if (IsMisfire())
		PlayHUDMotionIfExists({ "anm_reload_misfire", "anm_close" }, true, GetState());
	else if (iAmmoElapsed == 0)
		PlayHUDMotionIfExists({ "anm_open_empty", "anm_open_weapon", "anm_open" }, false, GetState());
	else
		PlayHUDMotionIfExists({ "anm_open_weapon", "anm_open" }, true, GetState());
}
void CWeaponAutomaticShotgun::PlayAnimAddOneCartridgeWeapon()
{
	VERIFY(GetState()==eReload);

	if (iAmmoElapsed == 0)
		PlayHUDMotionIfExists({ "anm_add_cartridge_empty", "anm_add_cartridge" }, true, GetState());
	else
		PlayHUDMotion("anm_add_cartridge",FALSE,this,GetState());

	if (m_bBulletsVisualization)
		HUD_VisualBulletUpdate();
}
void CWeaponAutomaticShotgun::PlayAnimCloseWeapon()
{
	VERIFY(GetState()==eReload);

	if (iAmmoElapsed == 0)
		PlayHUDMotionIfExists({ "anm_close_weapon_empty", "anm_close_empty", "anm_close_weapon", "anm_close" }, true, GetState());
	else
		PlayHUDMotionIfExists({ "anm_close_weapon", "anm_close" }, true, GetState());

	if (m_bBulletsVisualization)
		HUD_VisualBulletUpdate();
}

bool CWeaponAutomaticShotgun::HaveCartridgeInInventory(u8 cnt)
{
	if (unlimited_ammo())	return true;
	if(!m_pInventory)		return false;

	u32 ac = GetAmmoCount(m_ammoType);
	if(ac<cnt)
	{
		for(u8 i = 0; i < u8(m_ammoTypes.size()); ++i) 
		{
			if(m_ammoType==i) continue;
			ac	+= GetAmmoCount(i);
			if(ac >= cnt)
			{
				m_ammoType = i;
				break; 
			}
		}
	}
	return ac>=cnt;
}


u8 CWeaponAutomaticShotgun::AddCartridge		(u8 cnt)
{
	if (IsMisfire())	bMisfire = false;

	if ( m_set_next_ammoType_on_reload != undefined_ammo_type )
	{
		m_ammoType						= m_set_next_ammoType_on_reload;
		m_set_next_ammoType_on_reload	= undefined_ammo_type;
	}

	if( !HaveCartridgeInInventory(1) )
		return 0;

	m_pCurrentAmmo = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny( m_ammoTypes[m_ammoType].c_str() ));
	VERIFY((u32)iAmmoElapsed == m_magazine.size());


	if (m_DefaultCartridge.m_LocalAmmoType != m_ammoType)
		m_DefaultCartridge.Load(m_ammoTypes[m_ammoType].c_str(), m_ammoType);
	CCartridge l_cartridge = m_DefaultCartridge;
	while(cnt)
	{
		if (!unlimited_ammo())
		{
			if (!m_pCurrentAmmo->Get(l_cartridge)) break;
		}
		--cnt;
		++iAmmoElapsed;
		l_cartridge.m_LocalAmmoType = m_ammoType;
		m_magazine.push_back(l_cartridge);
//		m_fCurrentCartirdgeDisp = l_cartridge.m_kDisp;
	}

	VERIFY((u32)iAmmoElapsed == m_magazine.size());

	//выкинуть коробку патронов, если она пуста€
	if(m_pCurrentAmmo && !m_pCurrentAmmo->m_boxCurr && OnServer()) 
		m_pCurrentAmmo->SetDropManual(TRUE);

	return cnt;
}

void	CWeaponAutomaticShotgun::net_Export	(NET_Packet& P)
{
	inherited::net_Export(P);	
	P.w_u8(u8(m_magazine.size()));	
	for (u32 i=0; i<m_magazine.size(); i++)
	{
		CCartridge& l_cartridge = *(m_magazine.begin()+i);
		P.w_u8(l_cartridge.m_LocalAmmoType);
	}
}

void	CWeaponAutomaticShotgun::net_Import	(NET_Packet& P)
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
		Msg("! %s reload to %s", *l_cartridge.m_ammoSect, m_ammoTypes[LocalAmmoType].c_str());
#endif
		l_cartridge.Load( m_ammoTypes[LocalAmmoType].c_str(), LocalAmmoType );
	}
}
