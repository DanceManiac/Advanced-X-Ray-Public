#include "stdafx.h"
#include "weaponBM16.h"

CWeaponBM16::~CWeaponBM16()
{
}

void CWeaponBM16::Load	(LPCSTR section)
{
	inherited::Load		(section);

	m_sounds.LoadSound(section, "snd_reload_1", "m_sndReload1", false, m_eSoundShot);
}

void CWeaponBM16::PlayReloadSound()
{
	bool b_both = HaveCartridgeInInventory(2);

	if (m_magazine.size() == 1 || !b_both)
		PlaySound("sndReload1", get_LastFP());
	else
		PlaySound("sndReload", get_LastFP());
}

void CWeaponBM16::PlayAnimShoot()
{
	switch (m_magazine.size())
	{
	case 1: PlayHUDMotionIfExists({"anim_shoot_1", "anm_shot_1"}, FALSE, GetState()); break;
	case 2: PlayHUDMotionIfExists({"anim_shoot", "anm_shot_2"}, FALSE, GetState()); break;
	default: PlayHUDMotionIfExists({"anim_shoot", "anm_shots"}, FALSE, GetState()); break;
	}
}

void CWeaponBM16::PlayAnimReload()
{
	VERIFY(GetState() == eReload);
	if (m_magazine.size() == 1 || !HaveCartridgeInInventory(2))
		PlayHUDMotionIfExists({"anim_reload_1", "anm_reload_1"}, TRUE, GetState());
	else
		PlayHUDMotionIfExists({"anim_reload", "anm_reload_2"}, TRUE, GetState());
}

void CWeaponBM16::PlayAnimIdle()
{
	if(TryPlayAnimIdle())	return;

	if(IsZoomed())
	{
		switch (m_magazine.size())
		{
		case 0:{
			PlayHUDMotionIfExists({"anim_idle", "anm_idle_aim_0"}, TRUE, GetState());
		}break;
		case 1:{
			PlayHUDMotionIfExists({"anim_zoomed_idle_1", "anm_idle_aim_1"}, TRUE, GetState());
		}break;
		case 2:{
			PlayHUDMotionIfExists({"anim_zoomed_idle_2", "anm_idle_aim_2"}, TRUE, GetState());
		}break;
		};
	}
	else
	{
		switch (m_magazine.size())
		{
		case 0:
		{
			PlayHUDMotionIfExists({"anim_idle", "anm_idle_0"}, TRUE, GetState());
		}
		break;
		case 1:
		{
			PlayHUDMotionIfExists({"anim_idle_1", "anm_idle_1"}, TRUE, GetState());
		}
		break;
		case 2:
		{
			PlayHUDMotionIfExists({ "anim_idle_2", "anm_idle_2" }, TRUE, GetState());
		}
		break;
		};
	}
}

void CWeaponBM16::PlayAnimShow()
{
	switch (m_magazine.size())
	{
	case 0:
		PlayHUDMotionIfExists({ "anm_show_0", "anim_draw" }, true, GetState());
		break;
	case 1:
		PlayHUDMotionIfExists({ "anm_show_1", "anim_draw" }, true, GetState());
		break;
	case 2:
		PlayHUDMotionIfExists({ "anm_show_2", "anim_draw" }, true, GetState());
		break;
	}
}

void CWeaponBM16::PlayAnimHide()
{
	switch (m_magazine.size())
	{
	case 0:
		PlayHUDMotionIfExists({ "anm_hide_0", "anim_holster" }, true, GetState());
		break;
	case 1:
		PlayHUDMotionIfExists({ "anm_hide_1", "anim_holster" }, true, GetState());
		break;
	case 2:
		PlayHUDMotionIfExists({ "anm_hide_2", "anim_holster" }, true, GetState());
		break;
	}
}

void  CWeaponBM16::PlayAnimIdleMoving()
{
	switch (m_magazine.size())
	{
	case 0:
		PlayHUDMotionIfExists({ "anm_idle_moving_0", "anim_idle" }, true, GetState());
		break;
	case 1:
		PlayHUDMotionIfExists({ "anm_idle_moving_1", "anim_idle" }, true, GetState());
		break;
	case 2:
		PlayHUDMotionIfExists({ "anm_idle_moving_2", "anim_idle" }, true, GetState());
		break;
	}
}

void  CWeaponBM16::PlayAnimIdleSprint()
{
	switch (m_magazine.size())
	{
	case 0:
		PlayHUDMotion("anm_idle_sprint_0", TRUE, this, GetState());
		PlayHUDMotionIfExists({ "anm_idle_sprint_0", "anim_idle_sprint", "anim_idle" }, true, GetState());
		break;
	case 1:
		PlayHUDMotionIfExists({ "anm_idle_sprint_1", "anim_idle_sprint", "anim_idle" }, true, GetState());
		break;
	case 2:
		PlayHUDMotionIfExists({ "anm_idle_sprint_2", "anim_idle_sprint", "anim_idle" }, true, GetState());
		break;
	}
}
