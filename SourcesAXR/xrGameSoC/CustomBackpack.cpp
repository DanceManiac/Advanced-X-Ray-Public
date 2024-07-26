////////////////////////////////////////////////////////////////////////////
//	Module 		: CustomBackpack.cpp
//	Created 	: 21.08.2023
//  Modified 	: 21.08.2023
//	Author		: Dance Maniac
//	Description : Backpack class
//  MIT License
//	Copyright(c) 2020 M.F.S. Team
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CustomBackpack.h"

#include "Actor.h"
#include "Inventory.h"
#include "player_hud.h"
#include "UIGameCustom.h"
#include "HudManager.h"
#include "UIGameSP.h"
#include "ui\UIInventoryWnd.h"
#include "ui\UICarBodyWnd.h"
#include "ui\UIPdaWnd.h"
#include "ui\UITalkWnd.h"
#include "ElevatorState.h"
#include "../xrEngine/CameraBase.h"

extern bool g_block_actor_movement;
extern bool g_actor_allow_ladder;

CCustomBackpack::CCustomBackpack()
{
	SetState					(eHidden);
	SetNextState				(eHidden);
	m_fPowerLoss				= 1.0f;
	m_additional_weight			= 0.0f;
	m_additional_weight2		= 0.0f;

	m_fHealthRestoreSpeed		= 0.0f;
	m_fRadiationRestoreSpeed	= 0.0f;
	m_fSatietyRestoreSpeed		= 0.0f;
	m_fPowerRestoreSpeed		= 0.0f;
	m_fBleedingRestoreSpeed		= 0.0f;
	m_fThirstRestoreSpeed		= 0.0f;
	m_fIntoxicationRestoreSpeed = 0.0f;
	m_fSleepenessRestoreSpeed	= 0.0f;
	m_fAlcoholismRestoreSpeed	= 0.0f;
	m_fNarcotismRestoreSpeed	= 0.0f;
	m_fPsyHealthRestoreSpeed	= 0.0f;
	m_fFrostbiteRestoreSpeed	= 0.0f;

	m_fJumpSpeed				= 1.0f;
	m_fWalkAccel				= 1.0f;
	m_fOverweightWalkK			= 1.0f;

	m_fInventoryCapacity		= 0.0f;
}

CCustomBackpack::~CCustomBackpack()
{
}

BOOL CCustomBackpack::net_Spawn(CSE_Abstract* DC)
{
	return		(inherited::net_Spawn(DC));
}

void CCustomBackpack::Load(LPCSTR section)
{
	inherited::Load(section);

	m_sounds.LoadSound(section, "snd_draw", "sndShow");
	m_sounds.LoadSound(section, "snd_holster", "sndHide");

	m_fPowerLoss				= READ_IF_EXISTS(pSettings, r_float, section, "power_loss", 1.0f);
	clamp(m_fPowerLoss, 0.0f, 1.0f);

	m_additional_weight			= READ_IF_EXISTS(pSettings, r_float, section, "additional_inventory_weight", 0.0f);
	m_additional_weight2		= READ_IF_EXISTS(pSettings, r_float, section, "additional_inventory_weight2", 0.0f);

	m_fHealthRestoreSpeed		= READ_IF_EXISTS(pSettings, r_float, section, "health_restore_speed",    0.0f );
	m_fRadiationRestoreSpeed	= READ_IF_EXISTS(pSettings, r_float, section, "radiation_restore_speed", 0.0f );
	m_fSatietyRestoreSpeed		= READ_IF_EXISTS(pSettings, r_float, section, "satiety_restore_speed",   0.0f );
	m_fPowerRestoreSpeed		= READ_IF_EXISTS(pSettings, r_float, section, "power_restore_speed",     0.0f );
	m_fBleedingRestoreSpeed		= READ_IF_EXISTS(pSettings, r_float, section, "bleeding_restore_speed",  0.0f );
	m_fThirstRestoreSpeed		= READ_IF_EXISTS(pSettings, r_float, section, "thirst_restore_speed",	 0.0f );
	m_fIntoxicationRestoreSpeed = READ_IF_EXISTS(pSettings, r_float, section, "intoxication_restore_speed", 0.0f);
	m_fSleepenessRestoreSpeed	= READ_IF_EXISTS(pSettings, r_float, section, "sleepeness_restore_speed", 0.0f);
	m_fAlcoholismRestoreSpeed	= READ_IF_EXISTS(pSettings, r_float, section, "alcoholism_restore_speed", 0.0f);
	m_fNarcotismRestoreSpeed	= READ_IF_EXISTS(pSettings, r_float, section, "narcotism_restore_speed", 0.0f);
	m_fPsyHealthRestoreSpeed	= READ_IF_EXISTS(pSettings, r_float, section, "psy_health_restore_speed", 0.0f);
	m_fFrostbiteRestoreSpeed	= READ_IF_EXISTS(pSettings, r_float, section, "frostbite_restore_speed", 0.0f);

	m_fJumpSpeed				= READ_IF_EXISTS(pSettings, r_float, section, "jump_speed", 1.f);
	m_fWalkAccel				= READ_IF_EXISTS(pSettings, r_float, section, "walk_accel", 1.f);
	m_fOverweightWalkK			= READ_IF_EXISTS(pSettings, r_float, section, "overweight_walk_k", 1.f);

	m_fInventoryCapacity		= READ_IF_EXISTS(pSettings, r_float, section, "inventory_capacity", 0.0f);
}

void CCustomBackpack::shedule_Update(u32 dt)
{
	inherited::shedule_Update(dt);
}

void CCustomBackpack::UpdateCL()
{
	inherited::UpdateCL();

	if (!ParentIsActor())
		return;

	float cam_height = Actor()->GetCamHeightFactor();
	const float start_cam_height = pSettings->r_float("actor", "camera_height_factor");

	if (GetState() == eShowing)
		cam_height -= 0.01f;

	if (GetState() == eHiding)
		cam_height += 0.01f;

	clamp(cam_height, 0.55f, start_cam_height);
	Actor()->SetCamHeightFactor(cam_height);
}

void CCustomBackpack::OnH_A_Chield()
{
	inherited::OnH_A_Chield();
}

void CCustomBackpack::OnH_B_Independent(bool just_before_destroy)
{
	inherited::OnH_B_Independent(just_before_destroy);
}

void CCustomBackpack::OnMoveToRuck(EItemPlace prev)
{
	inherited::OnMoveToRuck(prev);

	if (!ParentIsActor())
		return;

	if (prev == eItemPlaceSlot)
	{
		g_actor_allow_ladder = true;
		g_block_actor_movement = false;

		float cam_height = Actor()->GetCamHeightFactor();
		const float start_cam_height = pSettings->r_float("actor", "camera_height_factor");

		while (cam_height != start_cam_height)
		{
			cam_height += 0.01f;
			clamp(cam_height, 0.55f, start_cam_height);
			Actor()->SetCamHeightFactor(cam_height);
		}

		SwitchState(eHidden);
		g_player_hud->detach_item(this);
	}

	StopCurrentAnimWithoutCallback();
}

void CCustomBackpack::OnMoveToSlot()
{
	inherited::OnMoveToSlot();
}

void CCustomBackpack::OnActiveItem()
{
	if (!ParentIsActor())
		return;

	SwitchState(eShowing);
}

void CCustomBackpack::OnHiddenItem()
{
	if (!ParentIsActor())
		return;

	SwitchState(eHiding);
}

void CCustomBackpack::HideBackpack()
{
	if (GetState() == eIdle)
		ToggleBackpack();
}

void CCustomBackpack::ShowBackpack()
{
	if (GetState() == eHidden)
		ToggleBackpack();
}

void CCustomBackpack::ToggleBackpack()
{
	if (GetState() == eHidden)
	{
		PIItem iitem = m_pCurrentInventory->ActiveItem();
		CHudItem* itm = (iitem) ? iitem->cast_hud_item() : NULL;
		u16 slot_to_activate = NO_ACTIVE_SLOT;
	}
	else
		if (GetState() == eIdle)
			SwitchState(eHiding);

}

void CCustomBackpack::OnStateSwitch(u32 S, u32 old_state)
{
	if (!ParentIsActor())
		return;

	inherited::OnStateSwitch(S, old_state);

	auto* pGameSP = smart_cast<CUIGameSP*>(HUD().GetUI()->UIGame());

	switch (S)
	{
	case eShowing:
		{
			g_actor_allow_ladder = false;
			g_block_actor_movement = true;

			g_player_hud->attach_item(this);
			m_sounds.PlaySound("sndShow", Fvector().set(0, 0, 0), this, true, false);
			PlayHUDMotion("anm_show", FALSE, this, GetState());
			SetPending(TRUE);
		}break;
	case eHiding:
		{
			g_actor_allow_ladder = true;
			g_block_actor_movement = false;

			m_sounds.PlaySound("sndHide", Fvector().set(0, 0, 0), this, true, false);
			PlayHUDMotion("anm_hide", FALSE, this, GetState());

			if (pGameSP && pGameSP->InventoryMenu && pGameSP->InventoryMenu->IsShown())
				pGameSP->InventoryMenu->HideDialog1();

			SetPending(TRUE);
		}break;
	case eIdle:
		{
			PlayAnimIdle();

			if (pGameSP && pGameSP->InventoryMenu && !pGameSP->InventoryMenu->IsShown())
				pGameSP->InventoryMenu->ShowDialog1(false);

			SetPending(FALSE);
		}break;
	case eHidden:
		{
			g_actor_allow_ladder = true;
			g_block_actor_movement = false;
		}break;
	}
}

void CCustomBackpack::OnAnimationEnd(u32 state)
{
	inherited::OnAnimationEnd(state);

	if (!ParentIsActor())
		return;

	switch (state)
	{
	case eShowing:
		{
			SwitchState(eIdle);
		} break;
	case eHiding:
		{
			SwitchState(eHidden);
			g_player_hud->detach_item(this);
		} break;
	}
}

void CCustomBackpack::UpdateXForm()
{
	CInventoryItem::UpdateXForm();
}

bool CCustomBackpack::ParentIsActor()
{
	CObject* O = H_Parent();
	if (!O)
		return false;

	CEntityAlive* EA = smart_cast<CEntityAlive*>(O);
	if (!EA)
		return false;

	return EA->cast_actor() != nullptr;
}