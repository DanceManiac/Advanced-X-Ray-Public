#include "stdafx.h"
#include "weaponpistol.h"
#include "ParticlesObject.h"
#include "actor.h"

CWeaponPistol::CWeaponPistol(LPCSTR name) : CWeaponCustomPistol(name)
{
	m_eSoundClose		= ESoundTypes(SOUND_TYPE_WEAPON_RECHARGING /*| eSoundType*/);
	m_opened = false;
	SetPending(FALSE);
}

CWeaponPistol::~CWeaponPistol(void)
{
}

void CWeaponPistol::net_Destroy()
{
	inherited::net_Destroy();
}


void CWeaponPistol::Load	(LPCSTR section)
{
	inherited::Load		(section);

	m_sounds.LoadSound	(section, "snd_close", "sndClose", false, m_eSoundClose);
}

void CWeaponPistol::OnH_B_Chield		()
{
	inherited::OnH_B_Chield		();
	m_opened = false;
}

void CWeaponPistol::PlayAnimShow	()
{
	VERIFY(GetState()==eShowing);
	if(iAmmoElapsed >= 1)
		m_opened = false;
	else
		m_opened = true;
		
	if (m_opened)
		PlayHUDMotionIfExists({"anim_draw_empty", "anm_show_empty"}, FALSE, GetState());
	else
		inherited::PlayAnimShow();
}

/*void CWeaponPistol::PlayAnimBore()
{
	if (m_opened)
		PlayHUDMotion("anim_empty", "anm_bore_empty", TRUE, this, GetState());
	else
		inherited::PlayAnimBore();
}*/

void CWeaponPistol::PlayAnimIdleSprint()
{
	if (m_opened)
	{
		PlayHUDMotion("anim_empty", TRUE, nullptr, GetState());
	}
	else
		inherited::PlayAnimIdleSprint();
}

void CWeaponPistol::PlayAnimIdleMoving()
{
	if (m_opened)
		PlayHUDMotionIfExists({"anim_empty", "anm_idle_moving_empty"}, TRUE, GetState());
	else
		inherited::PlayAnimIdleMoving();
}

void CWeaponPistol::PlayAnimIdle()
{
	VERIFY(GetState() == eIdle);

	if (TryPlayAnimIdle())
		return;

	if (m_opened)
		PlayHUDMotionIfExists({"anim_empty", "anm_idle_empty"}, TRUE, GetState());
	else
		inherited::PlayAnimIdle();
}

void CWeaponPistol::PlayAnimAim()
{
	if (m_opened)
		PlayHUDMotionIfExists({"anim_empty", "anm_idle_aim_empty"}, TRUE, GetState());
	else
		inherited::PlayAnimAim();
}

void CWeaponPistol::PlayAnimReload()
{
	VERIFY(GetState() == eReload);
	if (m_opened)
	{
		PlayHUDMotionIfExists({"anim_reload_empty", "anm_reload_empty"}, TRUE, GetState());
	}
	else 
	{
		inherited::PlayAnimReload();
	}
}

void CWeaponPistol::PlayAnimHide()
{
	VERIFY(GetState() == eHiding);
	if (m_opened)
	{
		PlaySound("sndClose", get_LastFP());
		PlayHUDMotionIfExists({"anim_close", "anm_hide_empty"}, TRUE, GetState());
	}
	else
		inherited::PlayAnimHide();
}

void CWeaponPistol::PlayAnimShoot()
{
	VERIFY(GetState() == eFire || GetState() == eFire2);
	if (iAmmoElapsed > 1)
	{
		PlayHUDMotionIfExists({"anim_shoot", "anm_shots"}, FALSE, GetState());
		m_opened = false;
	}
	else
	{
		PlayHUDMotionIfExists({"anim_shoot_last", "anm_shot_l"}, FALSE, GetState());
		m_opened = true;
	}
}

void CWeaponPistol::switch2_Reload()
{
//.	if(GetState()==eReload) return;
	inherited::switch2_Reload();
}

void CWeaponPistol::OnAnimationEnd(u32 state)
{
	if(state == eHiding && m_opened) 
	{
		m_opened = false;
//		switch2_Hiding();
	} 
	inherited::OnAnimationEnd(state);
}

void CWeaponPistol::OnShot		()
{
	// Sound
	PlaySound		(m_pSndShotCurrent.c_str(), get_LastFP());

	AddShotEffector	();
	
	PlayAnimShoot	();

	// Shell Drop
	Fvector vel; 
	PHGetLinearVell(vel);
	OnShellDrop					(get_LastSP(),  vel);

	// ќгонь из ствола
	
	StartFlameParticles	();
	R_ASSERT2(!m_pFlameParticles || !m_pFlameParticles->IsLooped(),
			  "can't set looped particles system for shoting with pistol");
	
	//дым из ствола
	StartSmokeParticles	(get_LastFP(), vel);
}

void CWeaponPistol::UpdateSounds()
{
	inherited::UpdateSounds();

	m_sounds.SetPosition("sndClose", get_LastFP());
}
