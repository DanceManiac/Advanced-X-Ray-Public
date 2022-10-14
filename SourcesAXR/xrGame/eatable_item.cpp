////////////////////////////////////////////////////////////////////////////
//	Module 		: eatable_item.cpp
//	Created 	: 24.03.2003
//  Modified 	: 29.01.2004
//	Author		: Yuri Dobronravin
//	Description : Eatable item
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "eatable_item.h"
#include "xrmessages.h"
#include "physic_item.h"
#include "Level.h"
#include "entity_alive.h"
#include "EntityCondition.h"
#include "InventoryOwner.h"
#include "Actor.h"
#include "Inventory.h"
#include "Level.h"
#include "game_object_space.h"
#include "ai_object_location.h"
#include "Weapon.h"
#include "actorEffector.h"
#include "HudManager.h"
#include "UIGameCustom.h"
#include "player_hud.h"
#include "../xrPhysics/ElevatorState.h"

extern bool g_block_all_except_movement;

CEatableItem::CEatableItem()
{
	m_iPortionsNum = 1;
	use_cam_effector = nullptr;
	anim_sect = nullptr;
	m_bHasAnimation = false;
	m_bUnlimited = false;
	m_physic_item	= 0;
	m_fEffectorIntensity = 1.0f;
	m_iAnimHandsCnt = 1;
	m_iAnimLength = 0;
	m_bActivated = false;
	m_bItmStartAnim = false;
}

CEatableItem::~CEatableItem()
{
}

DLL_Pure *CEatableItem::_construct	()
{
	m_physic_item	= smart_cast<CPhysicItem*>(this);
	return			(inherited::_construct());
}

void CEatableItem::Load(LPCSTR section)
{
	inherited::Load(section);

	m_iPortionsNum = READ_IF_EXISTS(pSettings, r_u32, section, "eat_portions_num", 1);
	m_bHasAnimation = READ_IF_EXISTS(pSettings, r_bool, section, "has_anim", false);
	m_bUnlimited = READ_IF_EXISTS(pSettings, r_bool, section, "unlimited_usage", false);
	anim_sect = READ_IF_EXISTS(pSettings, r_string, section, "hud_section", nullptr);
	m_fEffectorIntensity = READ_IF_EXISTS(pSettings, r_float, section, "cam_effector_intensity", 1.0f);
	use_cam_effector = READ_IF_EXISTS(pSettings, r_string, section, "use_cam_effector", nullptr);
	VERIFY						(m_iPortionsNum<10000);
}

BOOL CEatableItem::net_Spawn				(CSE_Abstract* DC)
{
	if (!inherited::net_Spawn(DC)) return FALSE;

	return TRUE;
};

bool CEatableItem::Useful() const
{
	if(!inherited::Useful()) return false;

	//проверить не все ли еще съедено
	if(m_iPortionsNum == 0 && !m_bUnlimited) return false;

	return true;
}

void CEatableItem::OnH_A_Independent() 
{
	inherited::OnH_A_Independent();
	if(!Useful()) {
		if (object().Local() && OnServer())	object().DestroyObject	();
	}	
}

void CEatableItem::OnH_B_Independent(bool just_before_destroy)
{
	if(!Useful()) 
	{
		object().setVisible(FALSE);
		object().setEnabled(FALSE);
		if (m_physic_item)
			m_physic_item->m_ready_to_destroy	= true;
	}
	inherited::OnH_B_Independent(just_before_destroy);
}

void CEatableItem::save(NET_Packet &packet)
{
	inherited::save(packet);
	save_data(m_iPortionsNum, packet);
}

void CEatableItem::load(IReader &packet)
{
	inherited::load(packet);
	load_data(m_iPortionsNum, packet);
}

void CEatableItem::UpdateInRuck(void)
{
	UpdateUseAnim();
}

void CEatableItem::HideWeapon()
{
	CEffectorCam* effector = Actor()->Cameras().GetCamEffector((ECamEffectorType)effUseItem);

	Actor()->SetWeaponHideState(INV_STATE_BLOCK_ALL, true);
	m_bItmStartAnim = true;

	if (!Actor()->inventory_disabled())
		CurrentGameUI()->HideActorMenu();
}

void CEatableItem::StartAnimation()
{
	m_bActivated = true;

	CEffectorCam* effector = Actor()->Cameras().GetCamEffector((ECamEffectorType)effUseItem);

	if (pSettings->line_exist(anim_sect, "single_handed_anim"))
		m_iAnimHandsCnt = pSettings->r_u32(anim_sect, "single_handed_anim");

	m_bItmStartAnim = false;
	g_block_all_except_movement = true;
	g_actor_allow_ladder = false;

	if (pSettings->line_exist(anim_sect, "anm_use"))
	{
		g_player_hud->script_anim_play(m_iAnimHandsCnt, anim_sect, "anm_use", true, 1.0f);
		m_iAnimLength = Device.dwTimeGlobal + g_player_hud->motion_length_script(anim_sect, "anm_use", 1.0f);
	}

	if (!effector && use_cam_effector != nullptr)
		AddEffector(Actor(), effUseItem, use_cam_effector, m_fEffectorIntensity);

	if (pSettings->line_exist(anim_sect, "snd_using"))
	{
		if (m_using_sound._feedback())
			m_using_sound.stop();

		shared_str snd_name = pSettings->r_string(anim_sect, "snd_using");
		m_using_sound.create(snd_name.c_str(), st_Effect, sg_SourceType);
		m_using_sound.play(NULL, sm_2D);
	}
}

void CEatableItem::UpdateUseAnim()
{
	if (!m_bHasAnimation) return;

	CEffectorCam* effector = Actor()->Cameras().GetCamEffector((ECamEffectorType)effUseItem);

	if (m_bItmStartAnim && Actor()->inventory().GetActiveSlot() == NO_ACTIVE_SLOT)
		StartAnimation();

	if (m_bActivated)
	{
		if (m_iAnimLength <= Device.dwTimeGlobal)
		{
			Actor()->SetWeaponHideState(INV_STATE_BLOCK_ALL, false);

			m_iAnimLength = Device.dwTimeGlobal;
			m_bActivated = false;
			g_block_all_except_movement = false;
			g_actor_allow_ladder = true;

			if (effector)
				RemoveEffector(Actor(), effUseItem);

			Actor()->inventory().Eat(this);
		}
	}
}

bool CEatableItem::UseBy (CEntityAlive* entity_alive)
{
	SMedicineInfluenceValues	V;
	V.Load(m_physic_item->cNameSect());

	CInventoryOwner* IO = smart_cast<CInventoryOwner*>(entity_alive);
	R_ASSERT(IO);
	R_ASSERT(m_pInventory == IO->m_inventory);
	R_ASSERT(object().H_Parent()->ID() == entity_alive->ID());

	entity_alive->conditions().ApplyInfluence(V, m_physic_item->cNameSect());

	for (u8 i = 0; i < (u8)eBoostMaxCount; i++)
	{
		if (pSettings->line_exist(m_physic_item->cNameSect().c_str(), ef_boosters_section_names[i]))
		{
			SBooster B;
			B.Load(m_physic_item->cNameSect(), (EBoostParams)i);
			entity_alive->conditions().ApplyBooster(B, m_physic_item->cNameSect());
		}
	}

	if (!IsGameTypeSingle() && OnServer())
	{
		NET_Packet				tmp_packet;
		CGameObject::u_EventGen(tmp_packet, GEG_PLAYER_USE_BOOSTER, entity_alive->ID());
		tmp_packet.w_u16(object_id());
		Level().Send(tmp_packet);
	}

	if (m_iPortionsNum != -1 && !m_bUnlimited)
	{
		if (m_iPortionsNum > 0)
			--(m_iPortionsNum);
		else
			m_iPortionsNum = 0;
	}

	return true;
}