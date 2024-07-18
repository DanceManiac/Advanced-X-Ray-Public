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
	if (m_magazine.empty())
		return;

	string_path guns_shoot_anm{};
	strconcat(sizeof(guns_shoot_anm), guns_shoot_anm, pSettings->line_exist(cNameSect().c_str(), "anm_shoot") ? "anm_shoot" : "anm_shots", (this->IsZoomed() && !this->IsRotatingToZoom()) ? "_aim_" : "_", std::to_string(m_magazine.size()).c_str());
	if (isHUDAnimationExist(guns_shoot_anm))
	{
		PlayHUDMotionNew(guns_shoot_anm, false, GetState());
		return;
	}

	switch (m_magazine.size())
	{
	case 1:
		PlayHUDMotionIfExists({ "anim_shoot_1", "anm_shot_1" }, FALSE, GetState());
		break;
	case 2:
		PlayHUDMotionIfExists({ "anm_shoot_2", "anm_shot_2" }, FALSE, GetState());
		break;
	default: 
		PlayHUDMotionIfExists({ "anim_shoot", "anm_shots" }, FALSE, GetState());
		break;
	}
}

void CWeaponBM16::PlayAnimBore()
{
	switch( m_magazine.size() )
	{
	case 0:
		PlayHUDMotion("anm_bore_0",TRUE,this,GetState());
		break;
	case 1:
		PlayHUDMotion("anm_bore_1",TRUE,this,GetState());
		break;
	case 2:
		PlayHUDMotion("anm_bore_2",TRUE,this,GetState());
		break;
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
		if (IsRotatingToZoom())
		{
			string32 guns_aim_anm;
			strconcat(sizeof(guns_aim_anm), guns_aim_anm, "anm_idle_aim_start_", std::to_string(m_magazine.size()).c_str());
			if (isHUDAnimationExist(guns_aim_anm))
			{
				PlayHUDMotionNew(guns_aim_anm, true, GetState());
				return;
			}
		}
		if (const char* guns_aim_anm = GetAnimAimName())
		{
			string64 guns_aim_anm_full;
			strconcat(sizeof(guns_aim_anm_full), guns_aim_anm_full, guns_aim_anm, "_", std::to_string(m_magazine.size()).c_str());

			if (isHUDAnimationExist(guns_aim_anm_full))
			{
				PlayHUDMotionNew(guns_aim_anm_full, true, GetState());
				return;
			}
			else if (strstr(guns_aim_anm_full, "_jammed"))
			{
				char* jammed_position = strstr(guns_aim_anm_full, "_jammed");
				int jammed_length = strlen("_jammed");

				char new_guns_aim_anm[100];
				strncpy(new_guns_aim_anm, guns_aim_anm_full, jammed_position - guns_aim_anm_full);
				strcpy(new_guns_aim_anm + (jammed_position - guns_aim_anm_full), guns_aim_anm_full + (jammed_position - guns_aim_anm_full) + jammed_length);

				if (isHUDAnimationExist(new_guns_aim_anm))
				{
					PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
					return;
				}
			}
			else if (strstr(guns_aim_anm_full, "_empty"))
			{
				char* empty_position = strstr(guns_aim_anm_full, "_empty");
				int empty_length = strlen("_empty");

				char new_guns_aim_anm[100];
				strncpy(new_guns_aim_anm, guns_aim_anm_full, empty_position - guns_aim_anm_full);
				strcpy(new_guns_aim_anm + (empty_position - guns_aim_anm_full), guns_aim_anm_full + (empty_position - guns_aim_anm_full) + empty_length);

				if (isHUDAnimationExist(new_guns_aim_anm))
				{
					PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
					return;
				}
			}
		}

		switch (m_magazine.size())
		{
		case 0:{
			PlayHUDMotionIfExists({"anm_idle_aim_0", "anim_idle_aim"}, TRUE, GetState());
		}break;
		case 1:{
			PlayHUDMotionIfExists({"anim_zoomed_idle_1", "anm_idle_aim_1", "anim_idle_aim" }, TRUE, GetState());
		}break;
		case 2:{
			PlayHUDMotionIfExists({"anim_zoomed_idle_2", "anim_zoomedidle_2", "anm_idle_aim_2", "anim_idle_aim" }, TRUE, GetState());
		}break;
		};
	}
	else
	{
		if (IsRotatingFromZoom())
		{
			string32 guns_aim_anm;
			strconcat(sizeof(guns_aim_anm), guns_aim_anm, "anm_idle_aim_end_", std::to_string(m_magazine.size()).c_str());
			if (isHUDAnimationExist(guns_aim_anm))
			{
				PlayHUDMotionNew(guns_aim_anm, true, GetState());
				return;
			}
		}

		switch (m_magazine.size())
		{
		case 0:
		{
			PlayHUDMotionIfExists({"anim_idle", "anm_idle_0"}, TRUE, GetState());
		}
		break;
		case 1:
		{
			PlayHUDMotionIfExists({"anim_idle_1", "anm_idle_1", "anim_idle"}, TRUE, GetState());
		}
		break;
		case 2:
		{
			PlayHUDMotionIfExists({ "anim_idle_2", "anm_idle_2", "anim_idle" }, TRUE, GetState());
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

void CWeaponBM16::PlayAnimIdleMovingSlow()
{
	switch (m_magazine.size())
	{
	case 0:
		PlayHUDMotionIfExists({ "anm_idle_moving_slow_0", "anm_idle_moving_0" }, true, GetState());
		break;
	case 1:
		PlayHUDMotionIfExists({ "anm_idle_moving_slow_1", "anm_idle_moving_1" }, true, GetState());
		break;
	case 2:
		PlayHUDMotionIfExists({ "anm_idle_moving_slow_2", "anm_idle_moving_2" }, true, GetState());
		break;
	}
}

void CWeaponBM16::PlayAnimIdleMovingCrouch()
{
	switch (m_magazine.size())
	{
	case 0:
		PlayHUDMotionIfExists({ "anm_idle_moving_crouch_0", "anm_idle_moving_0" }, true, GetState());
		break;
	case 1:
		PlayHUDMotionIfExists({ "anm_idle_moving_crouch_1", "anm_idle_moving_1" }, true, GetState());
		break;
	case 2:
		PlayHUDMotionIfExists({ "anm_idle_moving_crouch_2", "anm_idle_moving_2" }, true, GetState());
		break;
	}
}

void CWeaponBM16::PlayAnimIdleMovingCrouchSlow()
{
	switch (m_magazine.size())
	{
	case 0:
		PlayHUDMotionIfExists({ "anm_idle_moving_crouch_slow_0", "anm_idle_moving_crouch_0", "anm_idle_moving_0" }, true, GetState());
		break;
	case 1:
		PlayHUDMotionIfExists({ "anm_idle_moving_crouch_slow_1", "anm_idle_moving_crouch_1", "anm_idle_moving_1" }, true, GetState());
		break;
	case 2:
		PlayHUDMotionIfExists({ "anm_idle_moving_crouch_slow_2", "anm_idle_moving_crouch_2", "anm_idle_moving_2" }, true, GetState());
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

void CWeaponBM16::PlayAnimSprintStart()
{
	string_path guns_sprint_start_anm{};
	strconcat(sizeof(guns_sprint_start_anm), guns_sprint_start_anm, "anm_idle_sprint_start_", std::to_string(m_magazine.size()).c_str(), IsMisfire() ? "_jammed" : "");

	if (isHUDAnimationExist(guns_sprint_start_anm))
		PlayHUDMotionNew(guns_sprint_start_anm, true, GetState());
	else if (strstr(guns_sprint_start_anm, "_jammed"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_sprint_start_anm);
		new_guns_aim_anm[strlen(guns_sprint_start_anm) - strlen("_jammed")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
	}
	else
	{
		m_bSprintType = true;
		SwitchState(eIdle);
	}
}

void CWeaponBM16::PlayAnimSprintEnd()
{
	string_path guns_sprint_end_anm{};
	strconcat(sizeof(guns_sprint_end_anm), guns_sprint_end_anm, "anm_idle_sprint_end_", std::to_string(m_magazine.size()).c_str(), IsMisfire() ? "_jammed" : "");

	if (isHUDAnimationExist(guns_sprint_end_anm))
		PlayHUDMotionNew(guns_sprint_end_anm, true, GetState());
	else if (strstr(guns_sprint_end_anm, "_jammed"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_sprint_end_anm);
		new_guns_aim_anm[strlen(guns_sprint_end_anm) - strlen("_jammed")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
	}
	else
	{
		m_bSprintType = false;
		SwitchState(eIdle);
	}
}