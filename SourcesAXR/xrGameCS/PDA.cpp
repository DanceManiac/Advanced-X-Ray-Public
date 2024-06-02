#include "stdafx.h"
#include "pch_script.h"
#include "pda.h"
//#include "../xrphysics/PhysicsShell.h"
#include "Entity.h"
#include "actor.h"

#include "xrserver.h"
#include "xrServer_Objects_ALife_Items.h"
#include "level.h"

#include "specific_character.h"
#include "alife_registry_wrappers.h"
#include "../xrServerEntitiesCS/script_engine.h"

#include "player_hud.h"
#include "UIGameCustom.h"
#include "ui\UIPdaWnd.h"
#include "ai_sounds.h"
#include "Inventory.h"
#include "HUDManager.h"
#include "../xrEngine/x_ray.h"
#include "AdvancedXrayGameConstants.h"
#include "../xrEngine/LightAnimLibrary.h"

bool SSFX_PDA_DoF_active = false;

CPda::CPda(void)
{
	m_idOriginalOwner = u16(-1);
	m_SpecificChracterOwner = nullptr;
	TurnOff();
	m_bZoomed = false;
	joystick = BI_NONE;
	target_screen_switch = 0.f;
	m_fLR_CameraFactor = 0.f;
	m_fLR_MovingFactor = 0.f;
	m_fLR_InertiaFactor = 0.f;
	m_fUD_InertiaFactor = 0.f;
	m_bNoticedEmptyBattery = false;
	m_psy_factor = 0.0f;
}

CPda::~CPda()
{
	pda_light.destroy();
	pda_glow.destroy();
}

BOOL CPda::net_Spawn(CSE_Abstract* DC)
{
	inherited::net_Spawn(DC);
	CSE_Abstract* abstract = (CSE_Abstract*)(DC);
	CSE_ALifeItemPDA* pda = smart_cast<CSE_ALifeItemPDA*>(abstract);
	R_ASSERT(pda);
	m_idOriginalOwner = pda->m_original_owner;
	m_SpecificChracterOwner = pda->m_specific_character;

	return true;
}

void CPda::net_Destroy()
{
	inherited::net_Destroy();
	TurnOff();
	feel_touch.clear();
	UpdateActiveContacts();
}

void CPda::Load(LPCSTR section)
{
	inherited::Load(section);

	m_fRadius = pSettings->r_float(section, "radius");
	m_functor_str = READ_IF_EXISTS(pSettings, r_string, section, "play_function", "");
	m_fDisplayBrightnessPowerSaving = READ_IF_EXISTS(pSettings, r_float, section, "power_saving_brightness", .6f);
	m_fPowerSavingCharge = READ_IF_EXISTS(pSettings, r_float, section, "power_saving_charge", .15f);
	m_joystick_bone = READ_IF_EXISTS(pSettings, r_string, section, "joystick_bone", nullptr);
	m_sounds.LoadSound(section, "snd_draw", "sndShow", true);
	m_sounds.LoadSound(section, "snd_holster", "sndHide", true);
	m_sounds.LoadSound(section, "snd_draw_empty", "sndShowEmpty", true);
	m_sounds.LoadSound(section, "snd_holster_empty", "sndHideEmpty", true);
	m_sounds.LoadSound(section, "snd_btn_press", "sndButtonPress");
	m_sounds.LoadSound(section, "snd_btn_release", "sndButtonRelease"); 
	m_sounds.LoadSound(section, "snd_empty", "sndEmptyBattery", true);
	m_screen_on_delay = READ_IF_EXISTS(pSettings, r_float, section, "screen_on_delay", 0.f);
	m_screen_off_delay = READ_IF_EXISTS(pSettings, r_float, section, "screen_off_delay", 0.f);
	m_thumb_rot[0] = READ_IF_EXISTS(pSettings, r_float, section, "thumb_rot_x", 0.f);
	m_thumb_rot[1] = READ_IF_EXISTS(pSettings, r_float, section, "thumb_rot_y", 0.f);

	m_bLightsEnabled = READ_IF_EXISTS(pSettings, r_string, section, "light_enabled", false);

	if (!pda_light && m_bLightsEnabled && psActorFlags.test(AF_3D_PDA))
	{
		pda_light = ::Render->light_create();
		pda_light->set_shadow(READ_IF_EXISTS(pSettings, r_string, section, "light_shadow", false));

		m_bVolumetricLights = READ_IF_EXISTS(pSettings, r_bool, section, "volumetric_lights", false);
		m_fVolumetricQuality = READ_IF_EXISTS(pSettings, r_float, section, "volumetric_quality", 1.0f);
		m_fVolumetricDistance = READ_IF_EXISTS(pSettings, r_float, section, "volumetric_distance", 0.3f);
		m_fVolumetricIntensity = READ_IF_EXISTS(pSettings, r_float, section, "volumetric_intensity", 0.5f);

		m_iLightType = READ_IF_EXISTS(pSettings, r_u8, section, "light_type", 1);
		light_lanim = LALib.FindItem(READ_IF_EXISTS(pSettings, r_string, section, "color_animator", ""));

		const Fcolor clr = READ_IF_EXISTS(pSettings, r_fcolor, section, "light_color", (Fcolor{ 1.0f, 0.0f, 0.0f, 1.0f }));

		fBrightness = clr.intensity();
		pda_light->set_color(clr);

		const float range = READ_IF_EXISTS(pSettings, r_float, section, "light_range", 1.f);

		pda_light->set_range(range);
		pda_light->set_hud_mode(true);
		pda_light->set_type((IRender_Light::LT)m_iLightType);
		pda_light->set_cone(deg2rad(READ_IF_EXISTS(pSettings, r_float, section, "light_spot_angle", 1.f)));
		pda_light->set_texture(READ_IF_EXISTS(pSettings, r_string, section, "spot_texture", nullptr));

		pda_light->set_volumetric(m_bVolumetricLights);
		pda_light->set_volumetric_quality(m_fVolumetricQuality);
		pda_light->set_volumetric_distance(m_fVolumetricDistance);
		pda_light->set_volumetric_intensity(m_fVolumetricIntensity);

		//Glow
		m_bGlowEnabled = READ_IF_EXISTS(pSettings, r_string, section, "glow_enabled", false);

		if (!pda_glow && m_bGlowEnabled)
		{
			pda_glow = ::Render->glow_create();
			pda_glow->set_texture(READ_IF_EXISTS(pSettings, r_string, section, "glow_texture", nullptr));
			pda_glow->set_color(clr);
			pda_glow->set_radius(READ_IF_EXISTS(pSettings, r_float, section, "glow_radius", 0.3f));
		}
	}
}

void CPda::OnStateSwitch(u32 S)
{
	inherited::OnStateSwitch(S);

	if (!ParentIsActor())
		return;

	switch (S)
	{
	case eShowing:
	{
		g_player_hud->attach_item(this);
		g_pGamePersistent->devices_shader_data.pda_display_factor = 0.f;

		m_sounds.PlaySound((hasEnoughBatteryPower() && m_psy_factor < 1.0f) ? "sndShow" : "sndShowEmpty", Position(), H_Root(), !!GetHUDmode(), false);
		PlayHUDMotion((!m_bNoticedEmptyBattery && m_psy_factor < 1.0f) ? "anm_show" : "anm_show_empty", false, this, GetState());

		if (auto pda = HUD().GetUI() && &HUD().GetUI()->UIGame()->PdaMenu() ? &HUD().GetUI()->UIGame()->PdaMenu() : nullptr)
			pda->ResetJoystick(true);

		SetPending(true);
		target_screen_switch = Device.fTimeGlobal + m_screen_on_delay;
	}
	break;
	case eHiding:
	{
		m_sounds.PlaySound((hasEnoughBatteryPower() && m_psy_factor < 1.0f) ? "sndHide" : "sndHideEmpty", Position(), H_Root(), !!GetHUDmode(), false);
		PlayHUDMotion((!m_bNoticedEmptyBattery && m_psy_factor < 1.0f) ? "anm_hide" : "anm_hide_empty", true, this, GetState());
		SetPending(true);
		m_bZoomed = false;
		HUD().GetUI()->UIGame()->PdaMenu().Enable(false);
		g_player_hud->reset_thumb(false);
		HUD().GetUI()->UIGame()->PdaMenu().ResetJoystick(false);
		if (joystick != BI_NONE && HudItemData())
			HudItemData()->m_model->LL_GetBoneInstance(joystick).reset_callback();
		target_screen_switch = Device.fTimeGlobal + m_screen_off_delay;
	}
	break;
	case eHidden:
	{
		m_bZoomed = false;
		m_fZoomfactor = 0.f;
		CUIPdaWnd* pda = &HUD().GetUI()->UIGame()->PdaMenu();

		if (HUD().GetUI() && HUD().GetUI()->MainInputReceiver() == pda)
			HUD().GetUI()->SetMainInputReceiver(nullptr, false);

		if (pda->IsShown())
		{
			if (psActorFlags.test(AF_3D_PDA))
				pda->Enable(true);
			else
				HUD().GetUI()->StartStopMenu(pda, false);
		}

		g_player_hud->reset_thumb(true);
		SetPending(false);
	}
	break;
	case eIdle:
	{
		PlayAnimIdle();

		if (m_joystick_bone && joystick == BI_NONE && HudItemData())
			joystick = HudItemData()->m_model->LL_BoneID(m_joystick_bone);

		if (joystick != BI_NONE && HudItemData())
		{
			CBoneInstance* bi = &HudItemData()->m_model->LL_GetBoneInstance(joystick);
			if (bi)
				bi->set_callback(bctCustom, JoystickCallback, this);
		}
	}
	break;
	case eEmptyBattery:
	{
		SetPending(true);
		m_sounds.PlaySound("sndEmptyBattery", Position(), H_Root(), !!GetHUDmode(), false);
		PlayHUDMotion("anm_empty", true, this, GetState());
		m_bNoticedEmptyBattery = true;
	}
	}
}

void CPda::OnAnimationEnd(u32 state)
{
	inherited::OnAnimationEnd(state);
	switch (state)
	{
	case eShowing:
	{
		if (!hasEnoughBatteryPower() && !m_bNoticedEmptyBattery)
		{
			SwitchState(eEmptyBattery);
			return;
		}
		SetPending(false);
		SwitchState(eIdle);
	}
	break;
	case eHiding:
	{
		SetPending(false);
		SwitchState(eHidden);
		g_player_hud->detach_item(this);
	}
	break;
	case eEmptyBattery:
	{
		SetPending(false);
		SwitchState(eIdle);
	}
	break;
	}
}

void CPda::JoystickCallback(CBoneInstance* B)
{
	CPda* Pda = static_cast<CPda*>(B->callback_param());
	CUIPdaWnd* pda = &HUD().GetUI()->UIGame()->PdaMenu();

	static float fAvgTimeDelta = Device.fTimeDelta;
	fAvgTimeDelta = inertion(fAvgTimeDelta, Device.fTimeDelta, 0.8f);

	Fvector& target = pda->target_joystickrot;
	Fvector& current = pda->joystickrot;
	float& target_press = pda->target_buttonpress;
	float& press = pda->buttonpress;

	if (!target.similar(current, .0001f))
	{
		Fvector diff;
		diff = target;
		diff.sub(current);
		diff.mul(fAvgTimeDelta / .1f);
		current.add(diff);
	}
	else
		current.set(target);

	if (!fsimilar(target_press, press, .0001f))
	{
		float prev_press = press;

		float diff = target_press;
		diff -= press;
		diff *= (fAvgTimeDelta / .1f);
		press += diff;

		if (prev_press == 0.f && press < 0.f)
			Pda->m_sounds.PlaySound("sndButtonPress", B->mTransform.c, Pda->H_Root(), !!Pda->GetHUDmode());
		else if (prev_press < -.001f && press >= -.001f)
			Pda->m_sounds.PlaySound("sndButtonRelease", B->mTransform.c, Pda->H_Root(), !!Pda->GetHUDmode());
	}
	else
		press = target_press;

	Fmatrix rotation;
	rotation.identity();
	rotation.rotateX(current.x);

	Fmatrix rotation_y;
	rotation_y.identity();
	rotation_y.rotateY(current.y);
	rotation.mulA_43(rotation_y);

	rotation_y.identity();
	rotation_y.rotateZ(current.z);
	rotation.mulA_43(rotation_y);

	rotation.translate_over(0.f, press, 0.f);

	B->mTransform.mulB_43(rotation);
}

//extern bool IsMainMenuActive();

void CPda::UpdateCL()
{
	inherited::UpdateCL();

	if (!ParentIsActor())
		return;

	UpdateLights();

	const u32 state = GetState();
	const bool enoughBatteryPower = hasEnoughBatteryPower();
	const bool b_main_menu_is_active = (g_pGamePersistent->m_pMainMenu && g_pGamePersistent->m_pMainMenu->IsActive());

	// For battery icon
	const float condition = GetCondition();
	const auto pda = &HUD().GetUI()->UIGame()->PdaMenu();
	pda->m_power = condition;

	if (!psActorFlags.test(AF_3D_PDA))
	{
		if (state != eHidden)
			Actor()->inventory().Activate(NO_ACTIVE_SLOT);
		return;
	}

	if (pda->IsShown())
	{
		// Hide PDA UI on low condition (battery) or when the item is hidden.
		if (!enoughBatteryPower || state == eHidden)
		{
			HUD().GetUI()->SetMainInputReceiver(nullptr, false);
			HUD().GetUI()->StartStopMenu(pda, false);
			m_bZoomed = false;

			if (state == eIdle)
				SwitchState(eEmptyBattery);
		}
		else
		{
			// Force update PDA UI if it's disabled (no input) and check for deferred enable or zoom in.
			if (!pda->IsEnabled())
			{
				pda->Update();
				if (m_bZoomed)
					pda->Enable(true);
			}

			// Turn on "power saving" on low battery charge (dims the screen).
			if (IsUsingCondition() && condition < m_fPowerSavingCharge)
			{
				/*if (!m_bPowerSaving)
				{
					luabind::functor<void> funct;
					if (ai().script_engine().functor("pda.on_low_battery", funct))
						funct();
					m_bPowerSaving = true;
				}*/
			}

			// Turn off "power saving" if battery has sufficient charge.
			else if (m_bPowerSaving)
				m_bPowerSaving = false;
		}
	}
	else
	{
		// Show PDA UI if possible
		if (!b_main_menu_is_active && state != eHiding && state != eHidden && enoughBatteryPower)
		{
			HUD().GetUI()->StartStopMenu(pda, false);
			HUD().GetUI()->SetMainInputReceiver(nullptr, false);
			m_bNoticedEmptyBattery = false;

			if (!m_bZoomed)
				pda->Enable(false);
		}
	}

	if (state != eHidden)
	{
		// Adjust screen brightness (smooth)
		if (m_bPowerSaving)
		{
			if (g_pGamePersistent->devices_shader_data.pda_displaybrightness > m_fDisplayBrightnessPowerSaving)
				g_pGamePersistent->devices_shader_data.pda_displaybrightness -= Device.fTimeDelta / .25f;
		}
		else
			g_pGamePersistent->devices_shader_data.pda_displaybrightness = 1.f;

		clamp(g_pGamePersistent->devices_shader_data.pda_displaybrightness, m_fDisplayBrightnessPowerSaving, 1.f);

		// Screen "Glitch" factor
		g_pGamePersistent->devices_shader_data.pda_psy_influence = m_psy_factor;

		// Update Display Visibility (turn on/off)
		if (target_screen_switch < Device.fTimeGlobal)
		{
			if (!enoughBatteryPower || state == eHiding)
				// Change screen transparency (towards 0 = not visible).
				g_pGamePersistent->devices_shader_data.pda_display_factor -= Device.fTimeDelta / .25f;
			else
				// Change screen transparency (towards 1 = fully visible).
				g_pGamePersistent->devices_shader_data.pda_display_factor += Device.fTimeDelta / .75f;
		}

		clamp(g_pGamePersistent->devices_shader_data.pda_display_factor, 0.f, 1.f);
	}

	if (m_bZoomed)
	{
		ps_ssfx_wpn_dof_1 = GameConstants::GetSSFX_FocusDoF();
		ps_ssfx_wpn_dof_2 = GameConstants::GetSSFX_FocusDoF().z;
		SSFX_PDA_DoF_active = true;
	}
	else
	{
		if (SSFX_PDA_DoF_active)
		{
			ps_ssfx_wpn_dof_1 = GameConstants::GetSSFX_DefaultDoF();
			ps_ssfx_wpn_dof_2 = GameConstants::GetSSFX_DefaultDoF().z;
			SSFX_PDA_DoF_active = false;
		}
	}

	luabind::functor<bool> m_functor;

	if (ai().script_engine().functor("pda.check_surge", m_functor))
		m_functor();
}

void CPda::shedule_Update(u32 dt)
{
	inherited::shedule_Update(dt);

	if (!H_Parent()) return;
	Position().set(H_Parent()->Position());

	if (IsOn() && Level().CurrentEntity() && Level().CurrentEntity()->ID() == H_Parent()->ID())
	{
		CEntityAlive* EA = smart_cast<CEntityAlive*>(H_Parent());
		if (!EA || !EA->g_Alive())
		{
			TurnOff();
			return;
		}

		feel_touch_update(Position(), m_fRadius);
		UpdateActiveContacts();
	}
}

void CPda::UpdateLights()
{
	if (pda_light && psActorFlags.test(AF_3D_PDA))
	{
		const u32 state = GetState();

		if (!pda_light->get_active() && (state == eShowing || state == eIdle))
		{
			pda_light->set_active(true);

			if (pda_glow && !pda_glow->get_active() && m_bGlowEnabled)
				pda_glow->set_active(true);
		}
		else if (pda_light->get_active() && (state == eHiding || state == eHidden))
		{
			pda_light->set_active(false);

			if (pda_glow && pda_glow->get_active() && m_bGlowEnabled)
				pda_glow->set_active(false);
		}

		if (pda_light->get_active() && HudItemData())
		{
			if (GetHUDmode())
			{
				firedeps fd;
				HudItemData()->setup_firedeps(fd);
				pda_light->set_position(fd.vLastFP2);

				if (pda_glow && pda_glow->get_active())
					pda_glow->set_position(fd.vLastFP2);
			}

			// calc color animator
			if (light_lanim)
			{
				int frame{};
				u32 clr = light_lanim->CalculateRGB(Device.fTimeGlobal, frame);
				Fcolor fclr;
				fclr.set(clr);
				pda_light->set_color(fclr);
			}
		}
	}
}

void CPda::OnMoveToRuck(EItemPlace prev)
{
	inherited::OnMoveToRuck(prev);

	if (!ParentIsActor())
		return;

	if (prev == eItemPlaceSlot)
	{
		SwitchState(eHidden);
		if (joystick != BI_NONE && HudItemData())
			HudItemData()->m_model->LL_GetBoneInstance(joystick).reset_callback();
		g_player_hud->detach_item(this);
	}
	CUIPdaWnd* pda = &HUD().GetUI()->UIGame()->PdaMenu();
	HUD().GetUI()->StartStopMenu(pda, false);
	StopCurrentAnimWithoutCallback();
	SetPending(false);
}

void CPda::UpdateHudAdditional(Fmatrix& trans)
{
	CActor* pActor = smart_cast<CActor*>(H_Parent());
	if (!pActor)
		return;

	attachable_hud_item* hi = HudItemData();
	R_ASSERT(hi);

	Fvector curr_offs, curr_rot;

	//curr_offs = g_player_hud->m_adjust_mode ? g_player_hud->m_adjust_offset[0][1] : hi->m_measures.m_hands_offset[0][1];
	//curr_rot = g_player_hud->m_adjust_mode ? g_player_hud->m_adjust_offset[1][1] : hi->m_measures.m_hands_offset[1][1];

	curr_offs = hi->m_measures.m_hands_offset[0][1];//pos,aim
	curr_rot = hi->m_measures.m_hands_offset[1][1];//rot,aim

	curr_offs.mul(m_fZoomfactor);
	curr_rot.mul(m_fZoomfactor);

	Fmatrix hud_rotation;
	hud_rotation.identity();
	hud_rotation.rotateX(curr_rot.x);

	Fmatrix hud_rotation_y;
	hud_rotation_y.identity();
	hud_rotation_y.rotateY(curr_rot.y);
	hud_rotation.mulA_43(hud_rotation_y);

	hud_rotation_y.identity();
	hud_rotation_y.rotateZ(curr_rot.z);
	hud_rotation.mulA_43(hud_rotation_y);

	hud_rotation.translate_over(curr_offs);
	trans.mulB_43(hud_rotation);

	if (m_bZoomed)
		m_fZoomfactor += Device.fTimeDelta / .25f;
	else
		m_fZoomfactor -= Device.fTimeDelta / .25f;

	clamp(m_fZoomfactor, 0.f, 1.f);

	if (!g_player_hud->inertion_allowed())
		return;

	static float fAvgTimeDelta = Device.fTimeDelta;
	fAvgTimeDelta = inertion(fAvgTimeDelta, Device.fTimeDelta, 0.8f);

	u8 idx = m_bZoomed ? 1 : 0;

	float fYMag = pActor->fFPCamYawMagnitude;
	float fPMag = pActor->fFPCamPitchMagnitude;

	//============= Инерция оружия =============//
	// Параметры инерции
	float fInertiaSpeedMod = lerp(
		hi->m_measures.m_inertion_params.m_tendto_speed,
		hi->m_measures.m_inertion_params.m_tendto_speed_aim,
		m_fZoomfactor);

	float fInertiaReturnSpeedMod = lerp(
		hi->m_measures.m_inertion_params.m_tendto_ret_speed,
		hi->m_measures.m_inertion_params.m_tendto_ret_speed_aim,
		m_fZoomfactor);

	float fInertiaMinAngle = lerp(
		hi->m_measures.m_inertion_params.m_min_angle,
		hi->m_measures.m_inertion_params.m_min_angle_aim,
		m_fZoomfactor);

	Fvector4 vIOffsets; // x = L, y = R, z = U, w = D
	vIOffsets.x = lerp(
		hi->m_measures.m_inertion_params.m_offset_LRUD.x,
		hi->m_measures.m_inertion_params.m_offset_LRUD_aim.x,
		m_fZoomfactor);
	vIOffsets.y = lerp(
		hi->m_measures.m_inertion_params.m_offset_LRUD.y,
		hi->m_measures.m_inertion_params.m_offset_LRUD_aim.y,
		m_fZoomfactor);
	vIOffsets.z = lerp(
		hi->m_measures.m_inertion_params.m_offset_LRUD.z,
		hi->m_measures.m_inertion_params.m_offset_LRUD_aim.z,
		m_fZoomfactor);
	vIOffsets.w = lerp(
		hi->m_measures.m_inertion_params.m_offset_LRUD.w,
		hi->m_measures.m_inertion_params.m_offset_LRUD_aim.w,
		m_fZoomfactor);

	// Высчитываем инерцию из поворотов камеры
	bool bIsInertionPresent = m_fLR_InertiaFactor != 0.0f || m_fUD_InertiaFactor != 0.0f;
	if (abs(fYMag) > fInertiaMinAngle || bIsInertionPresent)
	{
		float fSpeed = fInertiaSpeedMod;
		if (fYMag > 0.0f && m_fLR_InertiaFactor > 0.0f ||
			fYMag < 0.0f && m_fLR_InertiaFactor < 0.0f)
		{
			fSpeed *= 2.f; //--> Ускоряем инерцию при движении в противоположную сторону
		}

		m_fLR_InertiaFactor -= (fYMag * fAvgTimeDelta * fSpeed); // Горизонталь (м.б. > |1.0|)
	}

	if (abs(fPMag) > fInertiaMinAngle || bIsInertionPresent)
	{
		float fSpeed = fInertiaSpeedMod;
		if (fPMag > 0.0f && m_fUD_InertiaFactor > 0.0f ||
			fPMag < 0.0f && m_fUD_InertiaFactor < 0.0f)
		{
			fSpeed *= 2.f; //--> Ускоряем инерцию при движении в противоположную сторону
		}

		m_fUD_InertiaFactor -= (fPMag * fAvgTimeDelta * fSpeed); // Вертикаль (м.б. > |1.0|)
	}

	clamp(m_fLR_InertiaFactor, -1.0f, 1.0f);
	clamp(m_fUD_InertiaFactor, -1.0f, 1.0f);

	// Плавное затухание инерции (основное, но без линейной никогда не опустит инерцию до полного 0.0f)
	m_fLR_InertiaFactor *= clampr(1.f - fAvgTimeDelta * fInertiaReturnSpeedMod, 0.0f, 1.0f);
	m_fUD_InertiaFactor *= clampr(1.f - fAvgTimeDelta * fInertiaReturnSpeedMod, 0.0f, 1.0f);

	// Минимальное линейное затухание инерции при покое (горизонталь)
	if (fYMag == 0.0f)
	{
		float fRetSpeedMod = (fYMag == 0.0f ? 1.0f : 0.75f) * (fInertiaReturnSpeedMod * 0.075f);
		if (m_fLR_InertiaFactor < 0.0f)
		{
			m_fLR_InertiaFactor += fAvgTimeDelta * fRetSpeedMod;
			clamp(m_fLR_InertiaFactor, -1.0f, 0.0f);
		}
		else
		{
			m_fLR_InertiaFactor -= fAvgTimeDelta * fRetSpeedMod;
			clamp(m_fLR_InertiaFactor, 0.0f, 1.0f);
		}
	}

	// Минимальное линейное затухание инерции при покое (вертикаль)
	if (fPMag == 0.0f)
	{
		float fRetSpeedMod = (fPMag == 0.0f ? 1.0f : 0.75f) * (fInertiaReturnSpeedMod * 0.075f);
		if (m_fUD_InertiaFactor < 0.0f)
		{
			m_fUD_InertiaFactor += fAvgTimeDelta * fRetSpeedMod;
			clamp(m_fUD_InertiaFactor, -1.0f, 0.0f);
		}
		else
		{
			m_fUD_InertiaFactor -= fAvgTimeDelta * fRetSpeedMod;
			clamp(m_fUD_InertiaFactor, 0.0f, 1.0f);
		}
	}

	// Применяем инерцию к худу
	float fLR_lim = (m_fLR_InertiaFactor < 0.0f ? vIOffsets.x : vIOffsets.y);
	float fUD_lim = (m_fUD_InertiaFactor < 0.0f ? vIOffsets.z : vIOffsets.w);

	curr_offs = { fLR_lim * -1.f * m_fLR_InertiaFactor, fUD_lim * m_fUD_InertiaFactor, 0.0f };

	hud_rotation.identity();
	hud_rotation.translate_over(curr_offs);
	trans.mulB_43(hud_rotation);
}

void CPda::UpdateXForm()
{
	CInventoryItem::UpdateXForm();
}

void CPda::OnActiveItem()
{
	if (!ParentIsActor())
		return;

	SwitchState(eShowing);
}

void CPda::OnHiddenItem()
{
	if (!ParentIsActor())
		return;

	SwitchState(eHiding);
}

void CPda::UpdateActiveContacts()
{
	m_active_contacts.clear_not_free();
	xr_vector<CObject*>::iterator it = feel_touch.begin();
	for (; it != feel_touch.end(); ++it)
	{
		CEntityAlive* pEA = smart_cast<CEntityAlive*>(*it);
		if (!!pEA->g_Alive() && !pEA->cast_base_monster())
		{
			m_active_contacts.push_back(*it);
		}
	}
}

void CPda::feel_touch_new(CObject* O)
{
	if (CInventoryOwner* pNewContactInvOwner = smart_cast<CInventoryOwner*>(O))
	{
		CInventoryOwner* pOwner = smart_cast<CInventoryOwner*>(H_Parent());
		VERIFY(pOwner);
		pOwner->NewPdaContact(pNewContactInvOwner);
	}
}

void CPda::feel_touch_delete(CObject* O)
{
	if (!H_Parent()) return;
	if (CInventoryOwner* pLostContactInvOwner = smart_cast<CInventoryOwner*>(O))
	{
		CInventoryOwner* pOwner = smart_cast<CInventoryOwner*>(H_Parent());
		VERIFY(pOwner);
		pOwner->LostPdaContact(pLostContactInvOwner);
	}
}

BOOL CPda::feel_touch_contact(CObject* O)
{
	CEntityAlive* entity_alive = smart_cast<CEntityAlive*>(O);

	if (entity_alive && entity_alive->cast_base_monster())
	{
		return true;
	}
	else if (CInventoryOwner* pInvOwner = smart_cast<CInventoryOwner*>(O))
	{
		if (this != pInvOwner->GetPDA())
		{
			CEntityAlive* pEntityAlive = smart_cast<CEntityAlive*>(O);
			if (pEntityAlive)
				return true;
		}
		else
			return false;
	}

	return false;
}

void CPda::OnH_A_Chield()
{
	VERIFY(IsOff());

	//Switch on PDA only in case of a new product with auxiliary control
	if (H_Parent()->ID() == m_idOriginalOwner)
	{
		TurnOn();
		if (m_sFullName.empty())
		{
			m_sFullName.assign(NameItem());
			m_sFullName += " ";
			m_sFullName += (smart_cast<CInventoryOwner*>(H_Parent()))->Name();
		}
	};
	inherited::OnH_A_Chield();
}

void CPda::OnH_B_Independent(bool just_before_destroy)
{
	inherited::OnH_B_Independent(just_before_destroy);
	TurnOff();

	if (!ParentIsActor())
		return;

	m_sounds.PlaySound(hasEnoughBatteryPower() ? "sndHide" : "sndHideEmpty", Position(), H_Root(), !!GetHUDmode(), false);

	SwitchState(eHidden);
	SetPending(false);
	m_bZoomed = false;
	m_fZoomfactor = 0.f;

	CUIPdaWnd* pda = &HUD().GetUI()->UIGame()->PdaMenu();

	if (pda->IsShown())
		HUD().GetUI()->StartStopMenu(pda, false);

	g_player_hud->reset_thumb(true);
	pda->ResetJoystick(true);

	if (joystick != BI_NONE && HudItemData())
		HudItemData()->m_model->LL_GetBoneInstance(joystick).reset_callback();

	g_player_hud->detach_item(this);
}

CInventoryOwner* CPda::GetOriginalOwner()
{
	CObject* pObject = Level().Objects.net_Find(GetOriginalOwnerID());
	CInventoryOwner* pInvOwner = smart_cast<CInventoryOwner*>(pObject);

	return pInvOwner;
}

void CPda::ActivePDAContacts(xr_vector<CPda*>& res)
{
	res.clear_not_free();
	xr_vector<CObject*>::iterator it = m_active_contacts.begin();
	xr_vector<CObject*>::iterator it_e = m_active_contacts.end();

	for (; it != it_e; ++it)
	{
		CPda* p = GetPdaFromOwner(*it);
		if (p)
			res.push_back(p);
	}
}

void CPda::save(NET_Packet& output_packet)
{
	inherited::save(output_packet);
	save_data(m_sFullName, output_packet);
}

void CPda::load(IReader& input_packet)
{
	inherited::load(input_packet);
	load_data(m_sFullName, input_packet);
}

CObject* CPda::GetOwnerObject()
{
	return Level().Objects.net_Find(GetOriginalOwnerID());
}

CPda* CPda::GetPdaFromOwner(CObject* owner)
{
	return smart_cast<CInventoryOwner*>(owner)->GetPDA();
}

void CPda::PlayScriptFunction()
{
	if (xr_strcmp(m_functor_str, ""))
	{
		luabind::functor<void> m_functor;
		R_ASSERT(ai().script_engine().functor(m_functor_str.c_str(), m_functor));
		m_functor();
	}
}
