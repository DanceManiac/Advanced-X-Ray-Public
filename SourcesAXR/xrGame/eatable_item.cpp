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
#include "game_object_space.h"
#include "ai_object_location.h"
#include "Weapon.h"
#include "actorEffector.h"
#include "HudManager.h"
#include "UIGameCustom.h"
#include "ui\UIActorMenu.h"
#include "player_hud.h"
#include "../xrPhysics/ElevatorState.h"
#include "CustomDetector.h"
#include "CustomOutfit.h"
#include "CustomBackpack.h"
#include "../xrEngine/x_ray.h"
#include "AdvancedXrayGameConstants.h"

extern bool g_block_all_except_movement;
ENGINE_API extern float psHUD_FOV_def;
static float last_hud_fov{};

CEatableItem::CEatableItem()
{
	m_iPortionsNum			= 1;
	use_cam_effector		= nullptr;
	anim_sect				= nullptr;
	anim_sect_exo			= nullptr;
	m_bHasAnimation			= false;
	m_bUnlimited			= false;
	m_physic_item			= 0;
	m_fEffectorIntensity	= 1.0f;
	m_iAnimHandsCnt			= 1;
	m_iAnimLength			= 0;
	m_bActivated			= false;
	m_bItmStartAnim			= false;
	m_bNeedDestroyNotUseful = true;

	m_fRadioactivity		= 0.0f;
	m_fIrradiationCoef		= 0.0005f;
	m_fIrradiationZonePower = 0.0f;
	m_fSpoliage				= 0.0f;
	m_fFoodRottingCoef		= 0.0f;
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

	m_iConstPortions			= READ_IF_EXISTS(pSettings, r_u32, section, "eat_portions_num", 1);
	m_iPortionsNum				= m_iConstPortions;
	m_bHasAnimation				= READ_IF_EXISTS(pSettings, r_bool, section, "has_anim", false);
	m_bUnlimited				= READ_IF_EXISTS(pSettings, r_bool, section, "unlimited_usage", false);
	anim_sect					= READ_IF_EXISTS(pSettings, r_string, section, "hud_section", nullptr);
	anim_sect_exo				= READ_IF_EXISTS(pSettings, r_string, section, "hud_section_exo", nullptr);
	m_fEffectorIntensity		= READ_IF_EXISTS(pSettings, r_float, section, "cam_effector_intensity", 1.0f);
	use_cam_effector			= READ_IF_EXISTS(pSettings, r_string, section, "use_cam_effector", nullptr);

	m_bNeedDestroyNotUseful		= READ_IF_EXISTS(pSettings, r_bool, section, "need_destroy_if_not_useful", true);

	m_fIrradiationCoef			= READ_IF_EXISTS(pSettings, r_float, section, "irradiation_coef", 0.0005f);
	m_fIrradiationZonePower		= READ_IF_EXISTS(pSettings, r_float, section, "irradiation_zone_power", 0.0f);
	m_fFoodRottingCoef			= READ_IF_EXISTS(pSettings, r_float, section, "rotting_factor", 0.0f);
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

	if (!Useful() && this->m_bCanUse)
	{
		if (object().Local() && OnServer())
			object().DestroyObject();
	}
}

void CEatableItem::OnH_B_Independent(bool just_before_destroy)
{
	if (!Useful())
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
	save_data(m_fRadioactivity, packet);
	save_data(m_fSpoliage, packet);
}

void CEatableItem::load(IReader &packet)
{
	inherited::load(packet);
	load_data(m_iPortionsNum, packet);
	load_data(m_fRadioactivity, packet);
	load_data(m_fSpoliage, packet);

	if (g_block_all_except_movement)
		g_block_all_except_movement = false;

	if (!g_actor_allow_ladder)
		g_actor_allow_ladder = true;
}

void CEatableItem::UpdateInRuck(CActor* actor)
{
	UpdateUseAnim(actor);

	if (GameConstants::GetFoodRotting() && GameConstants::GetActorIntoxication())
	{
		float rotten_coef = (m_fFoodRottingCoef / 128) * Device.fTimeDelta;
		static float spoliage = m_fSpoliage;

		if (spoliage < 1.0f)
			spoliage += rotten_coef;

		if (spoliage > 0.0f)
			m_fSpoliage = smoothstep(0.75f, 1.0f, spoliage);

		clamp(m_fFoodRottingCoef, 0.0f, 1.0f);
	}
}

void CEatableItem::HideWeapon()
{
	if (Actor()->m_bActionAnimInProcess || m_bActivated || m_bItmStartAnim)
		return;

	CEffectorCam* effector = Actor()->Cameras().GetCamEffector((ECamEffectorType)effUseItem);
	CCustomDetector* pDet = smart_cast<CCustomDetector*>(Actor()->inventory().ItemFromSlot(DETECTOR_SLOT));
	CWeapon* pWpn = smart_cast<CWeapon*>(Actor()->inventory().ActiveItem());
	
	if (pWpn && !(pWpn->GetState() == CWeapon::eIdle))
		return;

	Actor()->SetWeaponHideState(INV_STATE_BLOCK_ALL, true);

	if (pDet)
		pDet->HideDetector(true);

	m_bItmStartAnim = true;
}

void CEatableItem::StartAnimation()
{
	m_bActivated = true;

	CEffectorCam* effector = Actor()->Cameras().GetCamEffector((ECamEffectorType)effUseItem);
	CCustomOutfit* cur_outfit = Actor()->GetOutfit();

	bool has_lss = (cur_outfit && cur_outfit->m_bHasLSS);
	LPCSTR cur_section = (anim_sect_exo && pSettings->section_exist(anim_sect_exo) && has_lss) ? anim_sect_exo : anim_sect;

	bool m_bSingleHanded = READ_IF_EXISTS(pSettings, r_bool, cur_section, "single_handed_anim", false);
	int m_iAnimHandsCnt = m_bSingleHanded ? 1 : 2;

	m_bItmStartAnim = false;
	g_block_all_except_movement = true;
	g_actor_allow_ladder = false;
	Actor()->m_bActionAnimInProcess = true;

	if (pSettings->line_exist(cur_section, "anm_use"))
	{
		string128 anim_name{};
		strconcat(sizeof(anim_name), anim_name, "anm_use", (m_iPortionsNum == 1) ? "_last" : "");

		LPCSTR attach_visual = READ_IF_EXISTS(pSettings, r_string, cur_section, "item_visual", nullptr);
		
		// - cari0us - ВАЖНО !!! Названия аргументов и прочих штук в данном случае не должны начинаться с "anim_", 
		// иначе движок может воспринимать их как анимацию по какой-то причине; Потому переименовал в "use_anm_speed_k"
		float anim_speed = 1.0f;
		if (pSettings->line_exist(cur_section, "anim_speed"))
			anim_speed = pSettings->r_float(cur_section, "anim_speed");	// желательно избавиться в будущем
		else
			anim_speed = READ_IF_EXISTS(pSettings, r_float, cur_section, "use_anm_speed_k", 1.0f);

		if (pSettings->line_exist(cur_section, anim_name))
		{
			g_player_hud->script_anim_play(m_iAnimHandsCnt, cur_section, anim_name, false, anim_speed, attach_visual);
			m_iAnimLength = Device.dwTimeGlobal + g_player_hud->motion_length_script(cur_section, anim_name, anim_speed);
		}
		else
		{
			g_player_hud->script_anim_play(m_iAnimHandsCnt, cur_section, "anm_use", false, anim_speed, attach_visual);
			m_iAnimLength = Device.dwTimeGlobal + g_player_hud->motion_length_script(cur_section, "anm_use", anim_speed);
		}

		if (pSettings->line_exist(cur_section, "hud_fov"))
		{
			last_hud_fov = psHUD_FOV_def;
			psHUD_FOV_def = pSettings->r_float(cur_section, "hud_fov");
		}

		ps_ssfx_wpn_dof_1 = GameConstants::GetSSFX_FocusDoF();
		ps_ssfx_wpn_dof_2 = GameConstants::GetSSFX_FocusDoF().z;
	}

	if (!effector && use_cam_effector != nullptr)
		AddEffector(Actor(), effUseItem, use_cam_effector, m_fEffectorIntensity);

	if (pSettings->line_exist(cur_section, "snd_using"))
	{
		if (m_using_sound._feedback())
			m_using_sound.stop();

		string128 snd_var_name{};
		shared_str snd_name{};

		strconcat(sizeof(snd_var_name), snd_var_name, "snd_using", (m_iPortionsNum == 1) ? "_last" : "");

		if (pSettings->line_exist(cur_section, snd_var_name))
			snd_name = pSettings->r_string(cur_section, snd_var_name);
		else
			snd_name = pSettings->r_string(cur_section, "snd_using");

		m_using_sound.create(snd_name.c_str(), st_Effect, sg_SourceType);
		m_using_sound.play(NULL, sm_2D);
	}
}

void CEatableItem::UpdateUseAnim(CActor* actor)
{
	if (!m_bHasAnimation) return;

	CCustomDetector* pDet = smart_cast<CCustomDetector*>(actor->inventory().ItemFromSlot(DETECTOR_SLOT));
	CEffectorCam* effector = actor->Cameras().GetCamEffector((ECamEffectorType)effUseItem);
	CCustomOutfit* cur_outfit = Actor()->GetOutfit();

	bool has_lss = (cur_outfit && cur_outfit->m_bHasLSS);
	LPCSTR cur_section = (anim_sect_exo && pSettings->section_exist(anim_sect_exo) && has_lss) ? anim_sect_exo : anim_sect;

	bool IsActorAlive = g_pGamePersistent->GetActorAliveStatus();

	if (!Actor()->inventory_disabled() && m_bItmStartAnim)
		CurrentGameUI()->HideActorMenu();

	if (m_bItmStartAnim && actor->inventory().GetActiveSlot() == NO_ACTIVE_SLOT && (!pDet || pDet->IsHidden()) && !m_bActivated)
		StartAnimation();

	if (!IsActorAlive)
	{
		m_using_sound.stop();

		if (pSettings->line_exist(cur_section, "hud_fov") && last_hud_fov > 0.0f)
			psHUD_FOV_def = last_hud_fov;
	}

	if (m_bActivated)
	{
		if (m_iAnimLength <= Device.dwTimeGlobal || !IsActorAlive)
		{
			if (actor->inventory().GetPrevActiveSlot() == BACKPACK_SLOT)
				actor->inventory().SetPrevActiveSlot(NO_ACTIVE_SLOT);

			actor->SetWeaponHideState(INV_STATE_BLOCK_ALL, false);

			m_iAnimLength = Device.dwTimeGlobal;
			m_bActivated = false;
			g_block_all_except_movement = false;
			g_actor_allow_ladder = true;
			actor->m_bActionAnimInProcess = false;

			if (pSettings->line_exist(cur_section, "hud_fov") && last_hud_fov > 0.0f)
				psHUD_FOV_def = last_hud_fov;

			if (effector)
				RemoveEffector(actor, effUseItem);

			ps_ssfx_wpn_dof_1 = GameConstants::GetSSFX_DefaultDoF();
			ps_ssfx_wpn_dof_2 = GameConstants::GetSSFX_DefaultDoF().z;

			if (IsActorAlive)
				actor->inventory().Eat(this);
		}
	}
}

void CEatableItem::HitFromActorHit(SHit* pHDS)
{
	float hit_power = pHDS->damage();

	if (pHDS->hit_type == ALife::eHitTypeRadiation && hit_power > m_fIrradiationZonePower)
	{
		if (CCustomBackpack* backpack = smart_cast<CCustomBackpack*>(Actor()->inventory().ItemFromSlot(BACKPACK_SLOT)))
			hit_power = std::max(hit_power - backpack->GetRadiationProtection(), 0.0f);

		m_fRadioactivity += (hit_power / 10) * m_fIrradiationCoef;
		clamp(m_fRadioactivity, 0.0f, 1.0f);
	}
}

bool CEatableItem::UseBy (CEntityAlive* entity_alive)
{
	SMedicineInfluenceValues	V;
	V.Load(m_physic_item->cNameSect());

	CInventoryOwner* IO		= smart_cast<CInventoryOwner*>(entity_alive);
	R_ASSERT				(IO);
	R_ASSERT				(m_pInventory==IO->m_inventory);
	R_ASSERT				(object().H_Parent()->ID()==entity_alive->ID());

	entity_alive->conditions().ApplyInfluence(V, m_physic_item->cNameSect(), this);

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

	if (m_iPortionsNum > 1 && CurrentGameUI()->ActorMenu().IsShown() && CurrentGameUI()->ActorMenu().GetMenuMode() != mmDeadBodySearch)
		CurrentGameUI()->ActorMenu().RefreshConsumableCells();

	return true;
}

u32 CEatableItem::Cost() const
{
	u32 res = inherited::Cost();
	int percent = (m_iPortionsNum * 100) / m_iConstPortions;

	res = (res * percent) / 100;

	return res;
}

float CEatableItem::Weight() const
{
	float res = inherited::Weight();
	int percent = (m_iPortionsNum * 100) / m_iConstPortions;

	res = (res * percent) / 100;

	return res;
}