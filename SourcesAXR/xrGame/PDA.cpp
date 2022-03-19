﻿#include "pch_script.h"
#include "pda.h"
#include "../xrphysics/PhysicsShell.h"
#include "Entity.h"
#include "actor.h"

#include "xrserver.h"
#include "xrServer_Objects_ALife_Items.h"
#include "level.h"

#include "specific_character.h"
#include "alife_registry_wrappers.h"
#include "../xrServerEntities/script_engine.h"

#include "player_hud.h"
#include "UIGameCustom.h"
#include "ui\UIPdaWnd.h"
#include "ai_sounds.h"
#include "Inventory.h"

CPda::CPda(void)
{
	m_idOriginalOwner = u16(-1);
	m_SpecificChracterOwner = NULL;
	TurnOff();
	m_bZoomed = false;
	m_eDeferredEnable = eDefault;
	joystick = BI_NONE;
	target_screen_switch = 0.f;
	m_fLR_CameraFactor = 0.f;
	m_fLR_MovingFactor = 0.f;
	m_fLR_InertiaFactor = 0.f;
	m_fUD_InertiaFactor = 0.f;
	m_bNoticedEmptyBattery = false;
}

CPda::~CPda() {}

BOOL CPda::net_Spawn(CSE_Abstract* DC)
{
	BOOL res = inherited::net_Spawn(DC);
	CSE_Abstract* abstract = (CSE_Abstract*)(DC);
	CSE_ALifeItemPDA* pda = smart_cast<CSE_ALifeItemPDA*>(abstract);
	R_ASSERT(pda);
	m_idOriginalOwner = pda->m_original_owner;
	m_SpecificChracterOwner = pda->m_specific_character;

	return (res);
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
		g_pGamePersistent->pda_shader_data.pda_display_factor = 0.f;

		m_sounds.PlaySound(hasEnoughBatteryPower() ? "sndShow" : "sndShowEmpty", Position(), H_Root(), !!GetHUDmode(), false);
		PlayHUDMotion(!m_bNoticedEmptyBattery ? "anm_show" : "anm_show_empty", FALSE, this, GetState());
		
		SetPending(TRUE);
		target_screen_switch = Device.fTimeGlobal + m_screen_on_delay;
	}
	break;
	case eHiding:
	{
		m_sounds.PlaySound(hasEnoughBatteryPower() ? "sndHide" : "sndHideEmpty", Position(), H_Root(), !!GetHUDmode(), false);
		PlayHUDMotion(!m_bNoticedEmptyBattery ? "anm_hide" : "anm_hide_empty", TRUE, this, GetState());
		SetPending(TRUE);
		m_bZoomed = false;
		CurrentGameUI()->GetPdaMenu().Enable(false);
		g_player_hud->reset_thumb(false);
		CurrentGameUI()->GetPdaMenu().ResetJoystick(false);
		if (joystick != BI_NONE && HudItemData())
			HudItemData()->m_model->LL_GetBoneInstance(joystick).reset_callback();
		target_screen_switch = Device.fTimeGlobal + m_screen_off_delay;
	}
	break;
	case eHidden:
	{
		m_bZoomed = false;
		m_fZoomfactor = 0.f;
		CUIPdaWnd* pda = &CurrentGameUI()->GetPdaMenu();

		if (pda->IsShown())
		{
			if (psActorFlags.test(AF_3D_PDA))
				pda->Enable(true);
			else
				pda->HideDialog();
		}

		g_player_hud->reset_thumb(true);
		pda->ResetJoystick(true);
	SetPending(FALSE);
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
		SetPending(TRUE);
		m_sounds.PlaySound("sndEmptyBattery", Position(), H_Root(), !!GetHUDmode(), false);
		PlayHUDMotion("anm_empty", TRUE, this, GetState());
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
		SetPending(FALSE);
		SwitchState(eIdle);
	}
	break;
	case eHiding:
	{
		SetPending(FALSE);
		SwitchState(eHidden);
		g_player_hud->detach_item(this);
	}
	break;
	case eEmptyBattery:
	{
		SetPending(FALSE);
		SwitchState(eIdle);
	}
	break;
	}
}

void CPda::JoystickCallback(CBoneInstance* B)
{
	CPda* Pda = static_cast<CPda*>(B->callback_param());
	CUIPdaWnd* pda = &CurrentGameUI()->GetPdaMenu();

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

	bool b_main_menu_is_active = (g_pGamePersistent->m_pMainMenu && g_pGamePersistent->m_pMainMenu->IsActive());

	// For battery icon
	float condition = GetCondition();
	CUIPdaWnd* pda = &CurrentGameUI()->GetPdaMenu();
	pda->m_power = condition;

	if (!psActorFlags.test(AF_3D_PDA))
	{
		if (GetState() != eHidden)
			Actor()->inventory().Activate(NO_ACTIVE_SLOT);
		return;
	}

	u32 state = GetState();
	bool enoughBatteryPower = hasEnoughBatteryPower();

	if (pda->IsShown())
	{
		// Hide PDA UI on low condition (battery) or when the item is hidden.
		if (!enoughBatteryPower || state == eHidden)
		{
			pda->HideDialog();
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
				if (m_eDeferredEnable == eEnable || m_eDeferredEnable == eEnableZoomed)
				{
					if (Actor()->cam_freelook == eflDisabled && g_player_hud->script_anim_part == u8(-1))
					{
						pda->Enable(true);
						m_bZoomed = m_eDeferredEnable == eEnableZoomed;
						m_eDeferredEnable = eDefault;
					}
				}
			}

			// Disable PDA UI input if player is sprinting and no deferred input enable is expected.
			else
			{
				CEntity::SEntityState st;
				Actor()->g_State(st);
				if (st.bSprint && !st.bCrouch && !m_eDeferredEnable)
				{
					pda->Enable(false);
					m_bZoomed = false;
				}
			}

			// Turn on "power saving" on low battery charge (dims the screen).
			if (IsUsingCondition() && condition < m_fPowerSavingCharge)
			{
				if (!m_bPowerSaving)
				{
					luabind::functor<void> funct;
					if (ai().script_engine().functor("pda.on_low_battery", funct))
						funct();
					m_bPowerSaving = true;
				}
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
			pda->ShowDialog(false); // Don't hide indicators
			m_bNoticedEmptyBattery = false;
			if (m_eDeferredEnable == eEnable) // Don't disable input if it was enabled before opening the Main Menu.
				m_eDeferredEnable = eDefault;
			else
				pda->Enable(false);
		}
	}

	if (GetState() != eHidden)
	{
		// Adjust screen brightness (smooth)
		if (m_bPowerSaving)
		{
			if (g_pGamePersistent->pda_shader_data.pda_displaybrightness > m_fDisplayBrightnessPowerSaving)
				g_pGamePersistent->pda_shader_data.pda_displaybrightness -= Device.fTimeDelta / .25f;
		}
		else
			g_pGamePersistent->pda_shader_data.pda_displaybrightness = 1.f;

		clamp(g_pGamePersistent->pda_shader_data.pda_displaybrightness, m_fDisplayBrightnessPowerSaving, 1.f);

		// Screen "Glitch" factor
		g_pGamePersistent->pda_shader_data.pda_psy_influence = m_psy_factor;

		// Update Display Visibility (turn on/off)
		if (target_screen_switch < Device.fTimeGlobal)
		{
			if (!enoughBatteryPower || state == eHiding)
				// Change screen transparency (towards 0 = not visible).
				g_pGamePersistent->pda_shader_data.pda_display_factor -= Device.fTimeDelta / .25f;
			else
				// Change screen transparency (towards 1 = fully visible).
				g_pGamePersistent->pda_shader_data.pda_display_factor += Device.fTimeDelta / .75f;
		}

		clamp(g_pGamePersistent->pda_shader_data.pda_display_factor, 0.f, 1.f);
	}
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

void CPda::OnMoveToRuck(const SInvItemPlace& prev)
{
	inherited::OnMoveToRuck(prev);

	if (!ParentIsActor())
		return;

	if (prev.type == eItemPlaceSlot)
	{
		SwitchState(eHidden);
		if (joystick != BI_NONE && HudItemData())
			HudItemData()->m_model->LL_GetBoneInstance(joystick).reset_callback();
		g_player_hud->detach_item(this);
	}
	CUIPdaWnd* pda = &CurrentGameUI()->GetPdaMenu();
	if (pda->IsShown()) pda->HideDialog();
	StopCurrentAnimWithoutCallback();
	SetPending(FALSE);
}

void CPda::UpdateHudAdditional(Fmatrix& trans)
{
	CActor* pActor = smart_cast<CActor*>(H_Parent());
	if (!pActor)
		return;

	attachable_hud_item* hi = HudItemData();
	R_ASSERT(hi);

	//u8 idx = GetCurrentHudOffsetIdx();

	if (pActor->cam_freelook == eflEnabled || pActor->cam_freelook == eflEnabling || g_player_hud->script_anim_part != u8(-1))
	{
		CUIPdaWnd* pda = &CurrentGameUI()->GetPdaMenu();

		if (pda->IsEnabled())
		{
			if (m_bZoomed)
			{
				m_bZoomed = false;
				m_eDeferredEnable = eEnableZoomed;
			}
			else
			{
				m_eDeferredEnable = eEnable;
			}

			pda->Enable(false);
		}
	}

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
	SetPending(FALSE);
	m_bZoomed = false;
	m_fZoomfactor = 0.f;

	CUIPdaWnd* pda = &CurrentGameUI()->GetPdaMenu();
	if (pda->IsShown()) pda->HideDialog();
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
