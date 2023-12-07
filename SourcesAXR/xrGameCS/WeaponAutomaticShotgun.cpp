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
}

CWeaponAutomaticShotgun::~CWeaponAutomaticShotgun()
{
}

void CWeaponAutomaticShotgun::Load(LPCSTR section)
{
	inherited::Load(section);

	if(pSettings->line_exist(section, "tri_state_reload")){
		m_bTriStateReload = !!pSettings->r_bool(section, "tri_state_reload");
	};
	if(m_bTriStateReload){
		m_sounds.LoadSound(section, "snd_open_weapon", "sndOpen", false, m_eSoundOpen);

		m_sounds.LoadSound(section, "snd_add_cartridge", "sndAddCartridge", false, m_eSoundAddCartridge);

		if (WeaponSoundExist(section, "snd_add_cartridge_empty", true))
			m_sounds.LoadSound(section, "snd_add_cartridge_empty", "sndAddCartridgeEmpty", false, m_eSoundAddCartridge);

		if (WeaponSoundExist(section, "snd_reload_misfire", true))
			m_sounds.LoadSound(section, "snd_reload_misfire", "sndReloadMisfire", false, m_eSoundOpen);

		m_sounds.LoadSound(section, "snd_close_weapon", "sndClose_2", false, m_eSoundClose_2);
	};

}

bool CWeaponAutomaticShotgun::Action(s32 cmd, u32 flags)
{
	if(inherited::Action(cmd, flags)) return true;

	if(	m_bTriStateReload && GetState()==eReload && !IsMisfire() &&
		cmd==kWPN_FIRE && flags&CMD_START &&
		m_sub_state==eSubstateReloadInProcess		)//���������� ������������
	{
		AddCartridge(1);
		m_sub_state = eSubstateReloadEnd;
		return true;
	}
	return false;
}

void CWeaponAutomaticShotgun::OnAnimationEnd(u32 state) 
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

void CWeaponAutomaticShotgun::Reload() 
{
	if(m_bTriStateReload){
		TriStateReload();
	}else
		inherited::Reload();
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
	if(!m_bTriStateReload || S != eReload){
		inherited::OnStateSwitch(S);
		return;
	}

	CWeapon::OnStateSwitch(S);

	if ((m_magazine.size() == (u32)iMagazineSize) && !IsMisfire() || !HaveCartridgeInInventory(1) && !IsMisfire())
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

void CWeaponAutomaticShotgun::switch2_StartReload()
{
	if (!IsMisfire())
		PlaySound("sndOpen", get_LastFP());
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
	SetPending			(FALSE);
	
	if (!IsMisfire())
	{
		PlaySound("sndClose_2", get_LastFP());
		PlayAnimCloseWeapon();
	}
	else
	{
		bMisfire = false;

		if (GetAmmoElapsed() > 0) //xrKrodin: ��, ���� �� ������� ����������� ������ � ������ ������. ���� �������� ��� ����.
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
		PlayHUDMotion("anm_open",FALSE,this,GetState());
}
void CWeaponAutomaticShotgun::PlayAnimAddOneCartridgeWeapon()
{
	VERIFY(GetState()==eReload);

	if (iAmmoElapsed == 0)
		PlayHUDMotionIfExists({ "anm_add_cartridge_empty", "anm_add_cartridge" }, true, GetState());
	else
		PlayHUDMotion("anm_add_cartridge",FALSE,this,GetState());
}
void CWeaponAutomaticShotgun::PlayAnimCloseWeapon()
{
	VERIFY(GetState()==eReload);
	PlayHUDMotion("anm_close",FALSE,this,GetState());
}

bool CWeaponAutomaticShotgun::HaveCartridgeInInventory(u8 cnt)
{
	if (unlimited_ammo())	return true;
	m_pCurrentAmmo = NULL;
	if(m_pInventory) 
	{
		//���������� ����� � ��������� ������� �������� ���� 
		m_pCurrentAmmo = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(*m_ammoTypes[m_ammoType]));
		
		if(!m_pCurrentAmmo )
		{
			for(u32 i = 0; i < m_ammoTypes.size(); ++i) 
			{
				//��������� ������� ���� ���������� �����
				m_pCurrentAmmo = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(*m_ammoTypes[i]));
				if(m_pCurrentAmmo) 
				{ 
					m_ammoType = i; 
					break; 
				}
			}
		}
	}
	return (m_pCurrentAmmo!=NULL)&&(m_pCurrentAmmo->m_boxCurr>=cnt) ;
}

u8 CWeaponAutomaticShotgun::AddCartridge		(u8 cnt)
{
	if (IsMisfire())	bMisfire = false;

	if (m_set_next_ammoType_on_reload != u32(-1))
	{
		m_ammoType						= m_set_next_ammoType_on_reload;
		m_set_next_ammoType_on_reload	= u32(-1);
	}

	if( !HaveCartridgeInInventory(1) )
		return 0;

	VERIFY((u32)iAmmoElapsed == m_magazine.size());


	if (m_DefaultCartridge.m_LocalAmmoType != m_ammoType)
		m_DefaultCartridge.Load(*m_ammoTypes[m_ammoType], u8(m_ammoType));
	CCartridge l_cartridge = m_DefaultCartridge;
	while(cnt)
	{
		if (!unlimited_ammo())
		{
			if (!m_pCurrentAmmo->Get(l_cartridge)) break;
		}
		--cnt;
		++iAmmoElapsed;
		l_cartridge.m_LocalAmmoType = u8(m_ammoType);
		m_magazine.push_back(l_cartridge);
//		m_fCurrentCartirdgeDisp = l_cartridge.m_kDisp;
	}
	m_ammoName = (m_pCurrentAmmo) ? m_pCurrentAmmo->m_nameShort : NULL;

	VERIFY((u32)iAmmoElapsed == m_magazine.size());

	//�������� ������� ��������, ���� ��� ������
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
		Msg("! %s reload to %s", *l_cartridge.m_ammoSect, *(m_ammoTypes[LocalAmmoType]));
#endif
		l_cartridge.Load(*(m_ammoTypes[LocalAmmoType]), LocalAmmoType);
	}
}
