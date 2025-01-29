#include "stdafx.h"
#include "weaponBM16.h"
#include "Actor.h"
#include "Inventory.h"
#include "ActorNightVision.h"
#include "Torch.h"

CWeaponBM16::~CWeaponBM16()
{
}

void CWeaponBM16::Load	(LPCSTR section)
{
	inherited::Load		(section);
	m_sounds.LoadSound	(section, "snd_reload_1", "sndReload1", true, m_eSoundShot);
}

void CWeaponBM16::PlayReloadSound()
{
	bool b_both = HaveCartridgeInInventory(2);

	if (m_magazine.size() == 1 || !b_both)
		PlaySound	("sndReload1",get_LastFP());
	else						
		PlaySound	("sndReload",get_LastFP());
}

void CWeaponBM16::PlayAnimShoot()
{
	if (m_magazine.empty())
		return;

	if ((IsRotatingToZoom() && m_zoom_params.m_fZoomRotationFactor != 0.0f) || (IsRotatingFromZoom() && m_zoom_params.m_fZoomRotationFactor != 1.0f))
		return;

	string_path guns_shoot_anm{};
	strconcat(sizeof(guns_shoot_anm), guns_shoot_anm, pSettings->line_exist(cNameSect().c_str(), "anm_shoot") ? "anm_shoot" : "anm_shots", (this->IsZoomed() && !this->IsRotatingToZoom()) ? "_aim_" : "_", std::to_string(m_magazine.size()).c_str());
	if (isHUDAnimationExist(guns_shoot_anm)) 
	{
		PlayHUDMotionNew(guns_shoot_anm, false, GetState());
		return;
	}

	switch( m_magazine.size() )
	{
	case 1:
		PlayHUDMotionIfExists({ "anm_shoot_1", "anm_shot_1" }, false, GetState());
		break;
	case 2:
		PlayHUDMotionIfExists({ "anm_shoot_2", "anm_shot_2" }, false, GetState());
		break;
	}
}

void CWeaponBM16::PlayAnimFakeShoot()
{
	if ((IsRotatingToZoom() && m_zoom_params.m_fZoomRotationFactor != 0.0f) || (IsRotatingFromZoom() && m_zoom_params.m_fZoomRotationFactor != 1.0f))
		return;

	string128 guns_fakeshoot_anm{};
	strconcat(sizeof(guns_fakeshoot_anm), guns_fakeshoot_anm, ("anm_fakeshoot"), (IsZoomed() && !IsRotatingToZoom()) ? "_aim" : "", IsMisfire() ? "_jammed" : "", "_", std::to_string(m_magazine.size()).c_str());

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

void CWeaponBM16::PlayAnimShow()
{
	switch( m_magazine.size() )
	{
	case 0:
		PlayHUDMotion("anm_show_0",TRUE,this,GetState());
		break;
	case 1:
		PlayHUDMotion("anm_show_1",TRUE,this,GetState());
		break;
	case 2:
		PlayHUDMotion("anm_show_2",TRUE,this,GetState());
		break;
	}
}

void CWeaponBM16::PlayAnimHide()
{
	switch( m_magazine.size() )
	{
	case 0:
		PlayHUDMotion("anm_hide_0",TRUE,this,GetState());
		break;
	case 1:
		PlayHUDMotion("anm_hide_1",TRUE,this,GetState());
		break;
	case 2:
		PlayHUDMotion("anm_hide_2",TRUE,this,GetState());
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
	bool b_both = HaveCartridgeInInventory(2);

	VERIFY(GetState()==eReload);
	

	if((m_magazine.size()==1 || !b_both) && 
		(m_set_next_ammoType_on_reload == undefined_ammo_type || 
		 m_ammoType == m_set_next_ammoType_on_reload))
		PlayHUDMotion("anm_reload_1",TRUE,this,GetState());
	else
		PlayHUDMotion("anm_reload_2",TRUE,this,GetState());
}

void  CWeaponBM16::PlayAnimIdleMoving()
{
	switch( m_magazine.size() )
	{
	case 0:
		PlayHUDMotion("anm_idle_moving_0",TRUE,this,GetState());
		break;
	case 1:
		PlayHUDMotion("anm_idle_moving_1",TRUE,this,GetState());
		break;
	case 2:
		PlayHUDMotion("anm_idle_moving_2",TRUE,this,GetState());
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
	switch( m_magazine.size() )
	{
	case 0:
		PlayHUDMotion("anm_idle_sprint_0",TRUE,this,GetState());
		break;
	case 1:
		PlayHUDMotion("anm_idle_sprint_1",TRUE,this,GetState());
		break;
	case 2:
		PlayHUDMotion("anm_idle_sprint_2",TRUE,this,GetState());
		break;
	}
}

bool CWeaponBM16::PlayAnimAimEnd()
{
	string32 guns_aim_end_anm;
	strconcat(sizeof(guns_aim_end_anm), guns_aim_end_anm, "anm_idle_aim_end_", IsMisfire() ? "_jammed" : "", std::to_string(m_magazine.size()).c_str());
	
	if (isHUDAnimationExist(guns_aim_end_anm))
	{
		PlayHUDMotionNew(guns_aim_end_anm, true, GetState());
		return true;
	}
	else if (guns_aim_end_anm && strstr(guns_aim_end_anm, "_jammed"))
	{
		char* jammed_position = strstr(guns_aim_end_anm, "_jammed");
		int jammed_length = strlen("_jammed");

		char new_guns_aim_anm[100];
		strncpy(new_guns_aim_anm, guns_aim_end_anm, jammed_position - guns_aim_end_anm);
		strcpy(new_guns_aim_anm + (jammed_position - guns_aim_end_anm), guns_aim_end_anm + (jammed_position - guns_aim_end_anm) + jammed_length);

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return true;
		}

		return false;
	}

	return false;
}

void CWeaponBM16::PlayAnimIdle()
{
	if(IsZoomed())
	{
		if (IsRotatingToZoom())
		{
			string32 guns_aim_start_anm;
			strconcat(sizeof(guns_aim_start_anm), guns_aim_start_anm, "anm_idle_aim_start_", IsMisfire() ? "_jammed" : "", std::to_string(m_magazine.size()).c_str());
			
			if (isHUDAnimationExist(guns_aim_start_anm))
			{
				PlayHUDMotionNew(guns_aim_start_anm, true, GetState());
				return;
			}
			else if (guns_aim_start_anm && strstr(guns_aim_start_anm, "_jammed"))
			{
				char* jammed_position = strstr(guns_aim_start_anm, "_jammed");
				int jammed_length = strlen("_jammed");

				char new_guns_aim_anm[100];
				strncpy(new_guns_aim_anm, guns_aim_start_anm, jammed_position - guns_aim_start_anm);
				strcpy(new_guns_aim_anm + (jammed_position - guns_aim_start_anm), guns_aim_start_anm + (jammed_position - guns_aim_start_anm) + jammed_length);

				if (isHUDAnimationExist(new_guns_aim_anm))
				{
					PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
					return;
				}
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
			else if (guns_aim_anm_full && strstr(guns_aim_anm_full, "_jammed"))
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
			else if (guns_aim_anm_full && strstr(guns_aim_anm_full, "_empty"))
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

		if (!TryPlayAnimIdle())
		{
			switch (m_magazine.size())
			{
			case 0:
				{
					PlayHUDMotionNew({ "anm_idle_aim_0" }, true, GetState());
				} break;
			case 1:
				{
					PlayHUDMotionNew({ "anm_idle_aim_1" }, true, GetState());
				} break;
			case 2:
				{
					PlayHUDMotionIfExists({ "anim_zoomed_idle_2", "anim_zoomedidle_2", "anm_idle_aim_2" }, true, GetState());
				} break;
			}
		}
	}
	else
	{
		if (IsRotatingFromZoom())
		{
			if (PlayAnimAimEnd())
				return;
		}

		if (TryPlayAnimIdle())
			return;

		switch (m_magazine.size())
		{
		case 0:
			{
				PlayHUDMotion("anm_idle_0", TRUE, NULL, GetState());
			} break;
		case 1:
			{
				PlayHUDMotion("anm_idle_1", TRUE, NULL, GetState());
			} break;
		case 2:
			{
				PlayHUDMotion("anm_idle_2", TRUE, NULL, GetState());
			} break;
		};
	}
}

void CWeaponBM16::PlayAnimSprintStart()
{
	string_path guns_sprint_start_anm{};
	strconcat(sizeof(guns_sprint_start_anm), guns_sprint_start_anm, "anm_idle_sprint_start_", std::to_string(m_magazine.size()).c_str(), IsMisfire() ? "_jammed" : "");

	if (isHUDAnimationExist(guns_sprint_start_anm))
		PlayHUDMotionNew(guns_sprint_start_anm, true, GetState());
	else if (guns_sprint_start_anm && strstr(guns_sprint_start_anm, "_jammed"))
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
	else if (guns_sprint_end_anm && strstr(guns_sprint_end_anm, "_jammed"))
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

void CWeaponBM16::PlayAnimDeviceSwitch()
{
	CActor* actor = Actor();
	CTorch* torch = smart_cast<CTorch*>(Actor()->inventory().ItemFromSlot(TORCH_SLOT));
	CNightVisionEffector* nvg = Actor()->GetNightVision();

	PlaySound(HeadLampSwitch && torch ? (!torch->IsSwitchedOn() ? "sndHeadlampOn" : "sndHeadlampOff") : NightVisionSwitch && nvg ? (!nvg->IsActive() ? "sndNvOn" : "sndNvOff") : "sndHeadlampOn", get_LastFP());

	string128 guns_device_switch_anm{};
	strconcat(sizeof(guns_device_switch_anm), guns_device_switch_anm, HeadLampSwitch && torch ? (!torch->IsSwitchedOn() ? "anm_headlamp_on" : "anm_headlamp_off") : NightVisionSwitch && nvg ? (!nvg->IsActive() ? "anm_nv_on" : "anm_nv_off") : "anm_headlamp_on", IsMisfire() ? "_jammed_" : "_", std::to_string(m_magazine.size()).c_str());

	if (isHUDAnimationExist(guns_device_switch_anm))
	{
		PlayHUDMotionNew(guns_device_switch_anm, true, GetState());
	}
	else if (guns_device_switch_anm && strstr(guns_device_switch_anm, "_jammed"))
	{
		char new_guns_device_switch_anm[256];
		strcpy(new_guns_device_switch_anm, guns_device_switch_anm);
		new_guns_device_switch_anm[strlen(guns_device_switch_anm) - strlen("_jammed")] = '\0';

		if (isHUDAnimationExist(new_guns_device_switch_anm))
		{
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
			PlayHUDMotionNew(new_guns_device_switch_anm, true, GetState());
		}
	}
	else
	{
		DeviceUpdate();
		SwitchState(eIdle);
	}
}