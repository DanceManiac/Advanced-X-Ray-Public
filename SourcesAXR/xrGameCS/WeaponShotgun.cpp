#include "stdafx.h"
#include "weaponshotgun.h"
#include "entity.h"
#include "ParticlesObject.h"
#include "xr_level_controller.h"
#include "inventory.h"
#include "level.h"
#include "actor.h"

CWeaponShotgun::CWeaponShotgun()
{
	m_eSoundClose_2			= ESoundTypes(SOUND_TYPE_WEAPON_SHOOTING);
	m_eSoundAddCartridge	= ESoundTypes(SOUND_TYPE_WEAPON_SHOOTING);
}

CWeaponShotgun::~CWeaponShotgun()
{
}

void CWeaponShotgun::net_Destroy()
{
	inherited::net_Destroy();
}

void CWeaponShotgun::Load	(LPCSTR section)
{
	inherited::Load		(section);

	if(pSettings->line_exist(section, "tri_state_reload")){
		m_bTriStateReload = !!pSettings->r_bool(section, "tri_state_reload");
	};
	if(m_bTriStateReload){
		m_sounds.LoadSound(section, "snd_open_weapon", "sndOpen", m_eSoundOpen);

		m_sounds.LoadSound(section, "snd_add_cartridge", "sndAddCartridge", m_eSoundAddCartridge);

		if (WeaponSoundExist(section, "snd_add_cartridge_empty", true))
			m_sounds.LoadSound(section, "snd_add_cartridge_empty", "sndAddCartridgeEmpty", false, m_eSoundAddCartridge);

		if (WeaponSoundExist(section, "snd_reload_misfire", true))
			m_sounds.LoadSound(section, "snd_reload_misfire", "sndReloadMisfire", false, m_eSoundOpen);

		m_sounds.LoadSound(section, "snd_close_weapon", "sndClose_2", m_eSoundClose_2);
	};

}

void CWeaponShotgun::switch2_Fire	()
{
	inherited::switch2_Fire	();
	bWorking = false;
}


bool CWeaponShotgun::Action			(s32 cmd, u32 flags) 
{
	if(inherited::Action(cmd, flags)) return true;

	if(	m_bTriStateReload && GetState()==eReload && !IsMisfire() &&
		cmd==kWPN_FIRE && flags&CMD_START &&
		m_sub_state==eSubstateReloadInProcess		)//остановить перезагрузку
	{
		AddCartridge(1);
		m_sub_state = eSubstateReloadEnd;
		return true;
	}
	return false;
}

void CWeaponShotgun::OnAnimationEnd(u32 state) 
{
	if(!m_bTriStateReload || state != eReload)
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

void CWeaponShotgun::Reload() 
{
	if(m_bTriStateReload){
		TriStateReload();
	}else
		inherited::Reload();
}

void CWeaponShotgun::TriStateReload()
{
	if (HaveCartridgeInInventory(1) || IsMisfire())
	{
		m_sub_state = eSubstateReloadBegin;
		SwitchState(eReload);
	}
}

void CWeaponShotgun::OnStateSwitch	(u32 S)
{
	if(!m_bTriStateReload || S != eReload){
		inherited::OnStateSwitch(S);
		return;
	}

	CWeapon::OnStateSwitch(S);

	if( m_magazine.size() == (u32)iMagazineSize || (!HaveCartridgeInInventory(1) && !IsMisfire()))
	{
			switch2_EndReload		();
			m_sub_state = eSubstateReloadEnd;
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

void CWeaponShotgun::switch2_StartReload()
{
	if (!IsMisfire())
		PlaySound("sndOpen", get_LastFP());
	else
		PlaySound("sndReloadMisfire", get_LastFP());

	PlayAnimOpenWeapon	();
	SetPending			(TRUE);
}

void CWeaponShotgun::switch2_AddCartgidge	()
{
	if (WeaponSoundExist(m_section_id.c_str(), "snd_add_cartridge_empty") && iAmmoElapsed == 0)
		PlaySound("sndAddCartridgeEmpty", get_LastFP());
	else
		PlaySound("sndAddCartridge", get_LastFP());

	PlayAnimAddOneCartridgeWeapon();
	SetPending			(TRUE);
}

void CWeaponShotgun::switch2_EndReload	()
{
	SetPending			(FALSE);

	if (!IsMisfire())
	{
		PlaySound("sndClose_2", get_LastFP());
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

void CWeaponShotgun::PlayAnimOpenWeapon()
{
	VERIFY(GetState()==eReload);
	PlayHUDMotionIfExists({ "anm_open_weapon", "anm_open" }, false, GetState());
}
void CWeaponShotgun::PlayAnimAddOneCartridgeWeapon()
{
	VERIFY(GetState()==eReload);

	if (iAmmoElapsed == 0)
		PlayHUDMotionIfExists({ "anm_add_cartridge_empty", "anm_add_cartridge" }, true, GetState());
	else
		PlayHUDMotion("anm_add_cartridge", FALSE, this, GetState());
}
void CWeaponShotgun::PlayAnimCloseWeapon()
{
	VERIFY(GetState()==eReload);

	PlayHUDMotionIfExists({ "anm_close_weapon", "anm_close" }, false, GetState());
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

	if (isHUDAnimationExist("anm_idle_aim"))
		PlayHUDMotion("anm_idle_aim", TRUE, NULL, GetState());
}

bool CWeaponShotgun::HaveCartridgeInInventory		(u8 cnt)
{
	if (unlimited_ammo()) return true;
	m_pAmmo = NULL;
	if(m_pInventory) 
	{
		//попытатьс€ найти в инвентаре патроны текущего типа 
		m_pAmmo = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(*m_ammoTypes[m_ammoType]));
		
		if(!m_pAmmo )
		{
			for(u32 i = 0; i < m_ammoTypes.size(); ++i) 
			{
				//проверить патроны всех подход€щих типов
				m_pAmmo = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(*m_ammoTypes[i]));
				if(m_pAmmo) 
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
	if(IsMisfire())	bMisfire = false;

	if(m_set_next_ammoType_on_reload != u32(-1)){
		m_ammoType						= m_set_next_ammoType_on_reload;
		m_set_next_ammoType_on_reload	= u32(-1);

	}

	if( !HaveCartridgeInInventory(1) )
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

	//выкинуть коробку патронов, если она пуста€
	if(m_pAmmo && !m_pAmmo->m_boxCurr && OnServer()) 
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
	}
}
