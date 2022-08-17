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

CEatableItem::CEatableItem()
{
	m_fHealthInfluence = 0;
	m_fPowerInfluence = 0;
	m_fSatietyInfluence = 0;
	m_fRadiationInfluence = 0;
	m_fThirstInfluence = 0;
	m_fIntoxicationInfluence = 0;
	m_fSleepenessInfluence = 0;
	m_fAlcoholismInfluence = 0;
	m_fHangoverInfluence = 0;
	m_fNarcotismInfluence = 0;
	m_iPortionsNum = 1;
	anim_sect = nullptr;
	use_cam_effector = nullptr;
	m_bActivated = false;
	m_bTimerEnd = false;
	m_bHasAnimation = false;
	ItmStartAnim = false;
	last_slot_id = NO_ACTIVE_SLOT;
	m_iTiming = 0;
	UseTimer = 0;
	m_physic_item	= 0;
	m_fEffectorIntensity = 1.0f;
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

	m_fHealthInfluence			= pSettings->r_float(section, "eat_health");
	m_fPowerInfluence			= pSettings->r_float(section, "eat_power");
	m_fSatietyInfluence			= pSettings->r_float(section, "eat_satiety");
	m_fRadiationInfluence		= pSettings->r_float(section, "eat_radiation");
	m_fThirstInfluence			= pSettings->r_float(section, "eat_thirst");
	m_fIntoxicationInfluence	= pSettings->r_float(section, "eat_intoxication");
	m_fSleepenessInfluence		= pSettings->r_float(section, "eat_sleepeness");
	m_fAlcoholismInfluence		= pSettings->r_float(section, "eat_alcoholism");
	m_fHangoverInfluence		= pSettings->r_float(section, "eat_hangover");
	m_fNarcotismInfluence		= pSettings->r_float(section, "eat_narcotism");
	m_fWoundsHealPerc			= pSettings->r_float(section, "wounds_heal_perc");
	clamp						(m_fWoundsHealPerc, 0.f, 1.f);
	
	m_iPortionsNum = READ_IF_EXISTS(pSettings, r_u32, section, "eat_portions_num", 1);
	m_fMaxPowerUpInfluence		= READ_IF_EXISTS	(pSettings,r_float,section,"eat_max_power",0.0f);
	VERIFY						(m_iPortionsNum<10000);

	m_bHasAnimation = READ_IF_EXISTS(pSettings, r_bool, section, "has_anim", false);
	m_iTiming = READ_IF_EXISTS(pSettings, r_u32, section, "timing", 0);
	m_fEffectorIntensity = READ_IF_EXISTS(pSettings, r_float, section, "cam_effector_intensity", 1.0f);
	anim_sect = READ_IF_EXISTS(pSettings, r_string, section, "animation_item", nullptr);
	use_cam_effector = READ_IF_EXISTS(pSettings, r_string, section, "use_cam_effector", nullptr);
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
	if(m_iPortionsNum == 0) return false;

	return true;
}

void CEatableItem::HideWeapon()
{
	CEffectorCam* effector = Actor()->Cameras().GetCamEffector((ECamEffectorType)effUseItem);
	CInventoryOwner* pInvOwner = smart_cast<CInventoryOwner*>(Level().CurrentEntity());
	CActor* pActor = smart_cast<CActor*>(pInvOwner);
	CGameObject* game_object = smart_cast<CGameObject*>(this);
	VERIFY(game_object);
	Level().spawn_item(anim_sect, Actor()->Position(), false, Actor()->ID());

	u16 cur_active_slot = Actor()->inventory().GetActiveSlot();
	bool active_item = Actor()->inventory().ActiveItem();

	if (cur_active_slot == KNIFE_SLOT)
	{
		last_slot_id = KNIFE_SLOT;
		Actor()->inventory().Activate(NO_ACTIVE_SLOT);
	}
	else if (cur_active_slot == PISTOL_SLOT)
	{
		last_slot_id = PISTOL_SLOT;
		Actor()->inventory().Activate(NO_ACTIVE_SLOT);
	}
	else if (cur_active_slot == RIFLE_SLOT)
	{
		last_slot_id = RIFLE_SLOT;
		Actor()->inventory().Activate(NO_ACTIVE_SLOT);
	}
	else if (cur_active_slot == GRENADE_SLOT)
	{
		last_slot_id = GRENADE_SLOT;
		Actor()->inventory().Activate(NO_ACTIVE_SLOT);
	}
	else if (cur_active_slot == APPARATUS_SLOT)
	{
		last_slot_id = APPARATUS_SLOT;
		Actor()->inventory().Activate(NO_ACTIVE_SLOT);
	}
	else if (cur_active_slot == BOLT_SLOT)
	{
		last_slot_id = BOLT_SLOT;
		Actor()->inventory().Activate(NO_ACTIVE_SLOT);
	}
	else if (cur_active_slot == PDA_SLOT)
	{
		last_slot_id = PDA_SLOT;
		Actor()->inventory().Activate(NO_ACTIVE_SLOT);
	}
	else if (cur_active_slot == DETECTOR_SLOT)
	{
		last_slot_id = DETECTOR_SLOT;
		Actor()->inventory().Activate(NO_ACTIVE_SLOT);
	}

	Actor()->block_action(kWPN_1);
	Actor()->block_action(kWPN_2);
	Actor()->block_action(kWPN_3);
	Actor()->block_action(kWPN_4);
	Actor()->block_action(kWPN_5);
	Actor()->block_action(kWPN_6);
	Actor()->block_action(kWPN_NEXT);
	Actor()->block_action(kDETECTOR);
	Actor()->block_action(kNEXT_SLOT);
	Actor()->block_action(kPREV_SLOT);
	Actor()->block_action(kACTIVE_JOBS);
	Actor()->block_action(kQUICK_SAVE);
	Actor()->block_action(kINVENTORY);

	Actor()->m_bEatAnimActive = true;
	ItmStartAnim = true;

	if (!pActor)
		HUD().GetUI()->UIGame()->HideActorMenu();
}

void CEatableItem::StartAnimation()
{
	m_bActivated = true;

	CEffectorCam* effector = Actor()->Cameras().GetCamEffector((ECamEffectorType)effUseItem);

	UseTimer = Device.dwTimeGlobal + m_iTiming;

	Actor()->inventory().Activate(ANIMATION_SLOT, true);
	ItmStartAnim = false;

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

void CEatableItem::UpdateUseAnim()
{
	if (!m_bHasAnimation) return;

	CEffectorCam* effector = Actor()->Cameras().GetCamEffector((ECamEffectorType)effUseItem);
	CWeapon* cur_anim_itm = smart_cast<CWeapon*>(Actor()->inventory().ItemFromSlot(ANIMATION_SLOT));

	bool cur_active_slot = Actor()->inventory().GetActiveSlot();
	bool active_item = Actor()->inventory().ActiveItem();

	if (ItmStartAnim)
		StartAnimation();

	if (m_bActivated)
	{
		if (UseTimer <= Device.dwTimeGlobal)
		{

			if (last_slot_id == 0)
				Actor()->inventory().Activate(NO_ACTIVE_SLOT);
			if (last_slot_id == 1)
				Actor()->inventory().Activate(KNIFE_SLOT);
			if (last_slot_id == 2)
				Actor()->inventory().Activate(PISTOL_SLOT);
			if (last_slot_id == 3)
				Actor()->inventory().Activate(RIFLE_SLOT);
			if (last_slot_id == 4)
				Actor()->inventory().Activate(GRENADE_SLOT);
			if (last_slot_id == 5)
				Actor()->inventory().Activate(APPARATUS_SLOT);
			if (last_slot_id == 6)
				Actor()->inventory().Activate(BOLT_SLOT);
			if (last_slot_id == 8)
				Actor()->inventory().Activate(PDA_SLOT);
			if (last_slot_id == 9)
				Actor()->inventory().Activate(DETECTOR_SLOT);

			Actor()->inventory().Eat(this);
			UseTimer = Device.dwTimeGlobal;
			m_bActivated = false;
			Actor()->m_bEatAnimActive = false;

			if (cur_anim_itm)
				cur_anim_itm->DestroyObject();

			if (effector)
				RemoveEffector(Actor(), effUseItem);

			Actor()->unblock_action(kWPN_1);
			Actor()->unblock_action(kWPN_2);
			Actor()->unblock_action(kWPN_3);
			Actor()->unblock_action(kWPN_4);
			Actor()->unblock_action(kWPN_5);
			Actor()->unblock_action(kWPN_6);
			Actor()->unblock_action(kWPN_NEXT);
			Actor()->unblock_action(kDETECTOR);
			Actor()->unblock_action(kNEXT_SLOT);
			Actor()->unblock_action(kPREV_SLOT);
			Actor()->unblock_action(kACTIVE_JOBS);
			Actor()->unblock_action(kQUICK_SAVE);
			Actor()->unblock_action(kINVENTORY);
		}
	}
}

void CEatableItem::UseBy (CEntityAlive* entity_alive)
{
	CInventoryOwner* IO	= smart_cast<CInventoryOwner*>(entity_alive);
	R_ASSERT		(IO);
	R_ASSERT		(m_pInventory==IO->m_inventory);
	R_ASSERT		(object().H_Parent()->ID()==entity_alive->ID());
	entity_alive->conditions().ChangeHealth		(m_fHealthInfluence);
	entity_alive->conditions().ChangePower		(m_fPowerInfluence);
	entity_alive->conditions().ChangeSatiety	(m_fSatietyInfluence);
	entity_alive->conditions().ChangeRadiation	(m_fRadiationInfluence);
	entity_alive->conditions().ChangeBleeding	(m_fWoundsHealPerc);
	entity_alive->conditions().ChangeThirst		(m_fThirstInfluence);
	entity_alive->conditions().ChangeIntoxication	(m_fIntoxicationInfluence);
	entity_alive->conditions().ChangeSleepeness	(m_fSleepenessInfluence);
	entity_alive->conditions().ChangeAlcoholism (m_fAlcoholismInfluence);
	entity_alive->conditions().ChangeHangover	(m_fHangoverInfluence);
	entity_alive->conditions().ChangeNarcotism	(m_fNarcotismInfluence);
	
	entity_alive->conditions().SetMaxPower( entity_alive->conditions().GetMaxPower()+m_fMaxPowerUpInfluence );
	
	//уменьшить количество порций
	if (m_iPortionsNum != -1)
	{
		if (m_iPortionsNum > 0)
			--(m_iPortionsNum);
		else
			m_iPortionsNum = 0;
	}
}
