#include "stdafx.h"
#include "pch_script.h"
#include "HudItem.h"
#include "physic_item.h"
#include "actor.h"
#include "actoreffector.h"
#include "Missile.h"
#include "xrmessages.h"
#include "level.h"
#include "inventory.h"
#include "../xrEngine/CameraBase.h"
#include "player_hud.h"
#include "../xrEngine/SkeletonMotions.h"
#include "../xrEngine/GameMtlLib.h"
#include "ui_base.h"
#include "HUDManager.h"
#include "Weapon.h"
#include "WeaponMagazinedWGrenade.h"
#include "PDA.h"
#include <array>

#include "script_callback_ex.h"
#include "script_game_object.h"
#include "../xrCore/_vector3d_ext.h"
#include "../xrCore/vector.h"

ENGINE_API extern float psHUD_FOV_def;
BOOL	b_hud_collision = FALSE;

CHudItem::CHudItem()
{
	RenderHud					(TRUE);
	EnableHudInertion			(TRUE);
	AllowHudInertion			(TRUE);
	m_bStopAtEndAnimIsRunning	= false;
	m_bSprintType				= false;
	m_current_motion_def		= NULL;
	m_started_rnd_anim_idx		= u8(-1);
	m_hud_sect					= nullptr;
	m_bBoreEnabled				= true;
}

DLL_Pure *CHudItem::_construct	()
{
	m_object			= smart_cast<CPhysicItem*>(this);
	VERIFY				(m_object);

	m_item				= smart_cast<CInventoryItem*>(this);
	VERIFY				(m_item);

	return				(m_object);
}

CHudItem::~CHudItem()
{
}

void CHudItem::Load(LPCSTR section)
{
	m_item_sect				= section;

	//загрузить hud, если он нужен
	if (pSettings->line_exist(section, "hud"))
		m_hud_sect			= pSettings->r_string		(section,"hud");

	m_animation_slot		= pSettings->r_u32			(section,"animation_slot");

	// HUD FOV
	m_nearwall_enabled			= READ_IF_EXISTS(pSettings, r_bool,	 section, "nearwall_on", true);

	if (m_nearwall_enabled)
	{
		m_nearwall_dist_min		= READ_IF_EXISTS(pSettings, r_float, section, "nearwall_dist_min", 0.5f);
		m_nearwall_dist_max		= READ_IF_EXISTS(pSettings, r_float, section, "nearwall_dist_max", 1.f);
		m_nearwall_target_hud_fov= READ_IF_EXISTS(pSettings, r_float, section, "nearwall_target_hud_fov", 0.27f);
		m_nearwall_target_aim_hud_fov = READ_IF_EXISTS(pSettings, r_float, section, "nearwall_target_aim_hud_fov", m_nearwall_target_hud_fov);
		m_nearwall_speed_mod	= READ_IF_EXISTS(pSettings, r_float, section, "nearwall_speed_mod", 10.f);
	}
	
	if (pSettings->line_exist(m_hud_sect, "hud_fov"))
		m_base_fov = READ_IF_EXISTS(pSettings, r_float, m_hud_sect, "hud_fov", 0.0f);
	else
		m_base_fov = READ_IF_EXISTS(pSettings, r_float, section, "hud_fov", 0.0f);

	m_nearwall_last_hud_fov		= m_base_fov > 0.0f ? m_base_fov : psHUD_FOV_def;

	////////////////////////////////////////////
	m_strafe_offset[0][0] = READ_IF_EXISTS(pSettings, r_fvector3, section, "strafe_hud_offset_pos", (Fvector{0.025f, 0.f, 0.f}));
	m_strafe_offset[1][0] = READ_IF_EXISTS(pSettings, r_fvector3, section, "strafe_hud_offset_rot", (Fvector{0.f, 0.f, 5.5f}));

	m_strafe_offset[0][1] = READ_IF_EXISTS(pSettings, r_fvector3, section, "strafe_aim_hud_offset_pos", (Fvector{0.f, 0.f, 0.f}));
	m_strafe_offset[1][1] = READ_IF_EXISTS(pSettings, r_fvector3, section, "strafe_aim_hud_offset_rot", (Fvector{0.f, 0.f, 3.5f}));

	m_strafe_offset[2][0].set(READ_IF_EXISTS(pSettings, r_bool, section, "strafe_enabled", true), READ_IF_EXISTS(pSettings, r_float, section, "strafe_transition_time", 0.25f), 0.f);
	m_strafe_offset[2][1].set(READ_IF_EXISTS(pSettings, r_bool, section, "strafe_aim_enabled", true), READ_IF_EXISTS(pSettings, r_float, section, "strafe_aim_transition_time", 0.15f), 0.f);

	////////////////////////////////////////////
	////////////////////////////////////////////
	m_lookout_offset[0][0] = READ_IF_EXISTS(pSettings, r_fvector3, section, "lookout_hud_offset_pos", (Fvector{0.045f, 0.f, 0.f}));
	m_lookout_offset[1][0] = READ_IF_EXISTS(pSettings, r_fvector3, section, "lookout_hud_offset_rot", (Fvector{0.f, 0.f, 10.f}));

	m_lookout_offset[0][1] = READ_IF_EXISTS(pSettings, r_fvector3, section, "lookout_aim_hud_offset_pos", (Fvector{0.f, 0.f, 0.f}));
	m_lookout_offset[1][1] = READ_IF_EXISTS(pSettings, r_fvector3, section, "lookout_aim_hud_offset_rot", (Fvector{0.f, 0.f, 15.f}));

	m_lookout_offset[2][0].set(READ_IF_EXISTS(pSettings, r_bool, section, "lookout_enabled", true), READ_IF_EXISTS(pSettings, r_float, section, "lookout_transition_time", 0.25f), 0.f);
	m_lookout_offset[2][1].set(READ_IF_EXISTS(pSettings, r_bool, section, "lookout_aim_enabled", true), READ_IF_EXISTS(pSettings, r_float, section, "lookout_aim_transition_time", 0.15f), 0.f);
	////////////////////////////////////////////
	////////////////////////////////////////////
	m_jump_offset[0][0] = READ_IF_EXISTS(pSettings, r_fvector3, section, "jump_hud_offset_pos", (Fvector{0.f, 0.05f, 0.03f}));
	m_jump_offset[1][0] = READ_IF_EXISTS(pSettings, r_fvector3, section, "jump_hud_offset_rot", (Fvector{0.f, -10.f, -10.f}));

	m_jump_offset[0][1] = READ_IF_EXISTS(pSettings, r_fvector3, section, "jump_aim_hud_offset_pos", (Fvector{0.f, 0.03f, 0.01f}));
	m_jump_offset[1][1] = READ_IF_EXISTS(pSettings, r_fvector3, section, "jump_aim_hud_offset_rot", (Fvector{0.f, 2.5f, -3.f}));

	m_jump_offset[2][0].set(READ_IF_EXISTS(pSettings, r_bool, section, "jump_enabled", true), READ_IF_EXISTS(pSettings, r_float, section, "jump_transition_time", 0.35f), 0.f);
	m_jump_offset[2][1].set(READ_IF_EXISTS(pSettings, r_bool, section, "jump_aim_enabled", true), READ_IF_EXISTS(pSettings, r_float, section, "jump_aim_transition_time", 0.4f), 0.f);
	////////////////////////////////////////////
	////////////////////////////////////////////
	m_fall_offset[0][0] = READ_IF_EXISTS(pSettings, r_fvector3, section, "fall_hud_offset_pos", (Fvector{0.f, -0.05f, 0.06f}));
	m_fall_offset[1][0] = READ_IF_EXISTS(pSettings, r_fvector3, section, "fall_hud_offset_rot", (Fvector{0.f, 5.f, 0.f}));

	m_fall_offset[0][1] = READ_IF_EXISTS(pSettings, r_fvector3, section, "fall_aim_hud_offset_pos", (Fvector{0.f, 0.03f, -0.01f}));
	m_fall_offset[1][1] = READ_IF_EXISTS(pSettings, r_fvector3, section, "fall_aim_hud_offset_rot", (Fvector{0.f, -2.5f, 3.f}));
	////////////////////////////////////////////
	////////////////////////////////////////////
	m_landing_offset[0][0] = READ_IF_EXISTS(pSettings, r_fvector3, section, "landing_hud_offset_pos", (Fvector{0.f, -0.2f, 0.03f}));
	m_landing_offset[1][0] = READ_IF_EXISTS(pSettings, r_fvector3, section, "landing_hud_offset_rot", (Fvector{0.f, -5.f, 10.f}));

	m_landing_offset[0][1] = READ_IF_EXISTS(pSettings, r_fvector3, section, "landing_aim_hud_offset_pos", (Fvector{0.f, -0.1f, 0.02f}));
	m_landing_offset[1][1] = READ_IF_EXISTS(pSettings, r_fvector3, section, "landing_aim_hud_offset_rot", (Fvector{0.f, -2.5f, 5.f}));
	////////////////////////////////////////////
	////////////////////////////////////////////
	m_move_offset[0] = READ_IF_EXISTS(pSettings, r_fvector3, section, "stay_hud_offset_pos", (Fvector{0.f, -0.03f, 0.f}));
	m_move_offset[1] = READ_IF_EXISTS(pSettings, r_fvector3, section, "stay_hud_offset_rot", (Fvector{0.f, 0.5f, -3.f}));
	m_move_offset[2].set(READ_IF_EXISTS(pSettings, r_bool, section, "move_enabled", true), READ_IF_EXISTS(pSettings, r_float, section, "move_transition_time", 0.25f), 0.f);
	////////////////////////////////////////////
	////////////////////////////////////////////
	m_walk_offset[0] = READ_IF_EXISTS(pSettings, r_fvector3, section, "walk_hud_offset_pos", (Fvector{-0.02f, -0.02f, -0.03f}));
	m_walk_offset[1] = READ_IF_EXISTS(pSettings, r_fvector3, section, "walk_hud_offset_rot", (Fvector{0.f, 0.05f, -1.f}));
	m_walk_offset[2].set(READ_IF_EXISTS(pSettings, r_bool, section, "walk_enabled", true), READ_IF_EXISTS(pSettings, r_float, section, "walk_transition_time", 0.25f), 0.f);

	//Загрузка параметров инерции --#SM+# Begin--
	constexpr float PITCH_OFFSET_R = 0.0f; // Насколько сильно ствол смещается вбок (влево) при вертикальных поворотах камеры
	constexpr float PITCH_OFFSET_N = 0.0f; // Насколько сильно ствол поднимается\опускается при вертикальных поворотах камеры
	constexpr float PITCH_OFFSET_D = 0.02f; // Насколько сильно ствол приближается\отдаляется при вертикальных поворотах камеры
	float PITCH_LOW_LIMIT = -PI; // Минимальное значение pitch при использовании совместно с PITCH_OFFSET_N
	constexpr float ORIGIN_OFFSET = -0.05f; // Фактор влияния инерции на положение ствола (чем меньше, тем масштабней инерция)
	constexpr float ORIGIN_OFFSET_AIM = -0.02f; // (Для прицеливания)
	constexpr float TENDTO_SPEED = 5.f; // Скорость нормализации положения ствола
	constexpr float TENDTO_SPEED_AIM = 10.f; // (Для прицеливания)

	inertion_data.m_pitch_offset_r = READ_IF_EXISTS(pSettings, r_float, m_hud_sect, "pitch_offset_right", PITCH_OFFSET_R);
	inertion_data.m_pitch_offset_n = READ_IF_EXISTS(pSettings, r_float, m_hud_sect, "pitch_offset_up", PITCH_OFFSET_N);
	inertion_data.m_pitch_offset_d = READ_IF_EXISTS(pSettings, r_float, m_hud_sect, "pitch_offset_forward", PITCH_OFFSET_D);
	inertion_data.m_pitch_low_limit = READ_IF_EXISTS(pSettings, r_float, m_hud_sect, "pitch_offset_up_low_limit", PITCH_LOW_LIMIT);

	inertion_data.m_origin_offset = READ_IF_EXISTS(pSettings, r_float, m_hud_sect, "inertion_origin_offset", ORIGIN_OFFSET);
	inertion_data.m_origin_offset_aim = READ_IF_EXISTS(pSettings, r_float, m_hud_sect, "inertion_zoom_origin_offset", ORIGIN_OFFSET_AIM);
	inertion_data.m_tendto_speed = READ_IF_EXISTS(pSettings, r_float, m_hud_sect, "inertion_tendto_speed", TENDTO_SPEED);
	inertion_data.m_tendto_speed_aim = READ_IF_EXISTS(pSettings, r_float, m_hud_sect, "inertion_zoom_tendto_speed", TENDTO_SPEED_AIM);

	// Загрузка параметров смещения при стрельбе
	m_shooting_params.m_shot_max_offset_LRUD = READ_IF_EXISTS(pSettings, r_fvector4, m_hud_sect, "shooting_max_LRUD", Fvector4().set(0, 0, 0, 0));
	m_shooting_params.m_shot_max_offset_LRUD_aim = READ_IF_EXISTS(pSettings, r_fvector4, m_hud_sect, "shooting_max_LRUD_aim", Fvector4().set(0, 0, 0, 0));
	m_shooting_params.m_shot_max_rot_UD = READ_IF_EXISTS(pSettings, r_fvector2, m_hud_sect, "shooting_max_UD_rot", Fvector2().set(0, 0));
	m_shooting_params.m_shot_max_rot_UD_aim = READ_IF_EXISTS(pSettings, r_fvector2, m_hud_sect, "shooting_max_UD_rot_aim", Fvector2().set(0, 0));
	m_shooting_params.m_shot_offset_BACKW = READ_IF_EXISTS(pSettings, r_float, m_hud_sect, "shooting_backward_offset", 0.0f);
	m_shooting_params.m_shot_offset_BACKW_aim = READ_IF_EXISTS(pSettings, r_float, m_hud_sect, "shooting_backward_offset_aim", 0.0f);
	m_shooting_params.m_shot_offsets_strafe = READ_IF_EXISTS(pSettings, r_fvector2, m_hud_sect, "shooting_strafe_offsets", Fvector2().set(0, 0));
	m_shooting_params.m_shot_offsets_strafe_aim = READ_IF_EXISTS(pSettings, r_fvector2, m_hud_sect, "shooting_strafe_offsets_aim", Fvector2().set(0, 0));
	m_shooting_params.m_shot_diff_per_shot = READ_IF_EXISTS(pSettings, r_fvector2, m_hud_sect, "shooting_diff_per_shot", Fvector2().set(0, 0));
	m_shooting_params.m_shot_power_per_shot = READ_IF_EXISTS(pSettings, r_fvector2, m_hud_sect, "shooting_power_per_shot", Fvector2().set(0, 0));
	m_shooting_params.m_ret_time = READ_IF_EXISTS(pSettings, r_fvector2, m_hud_sect, "shooting_ret_time", Fvector2().set(1.0f, 1.0f));
	m_shooting_params.m_ret_time_fire = READ_IF_EXISTS(pSettings, r_fvector2, m_hud_sect, "shooting_ret_time_fire", Fvector2().set(1000.0f, 1000.0f));
	m_shooting_params.m_ret_time_backw_koef = READ_IF_EXISTS(pSettings, r_float, m_hud_sect, "shooting_ret_time_backw_k", 1.0f);

	bReloadShooting = READ_IF_EXISTS(pSettings, r_bool, m_hud_sect, "use_new_shooting_params", false);
	//--#SM+# End--

	m_bBoreEnabled				= READ_IF_EXISTS(pSettings, r_bool, section, "enable_bore_state", true);

	if (m_bBoreEnabled)
		m_sounds.LoadSound		(section, "snd_bore", "sndBore", true);

	if (pSettings->line_exist(section, "snd_headlamp_on"))
		m_sounds.LoadSound(section, "snd_headlamp_on", "sndHeadlampOn", false);
	if (pSettings->line_exist(section, "snd_headlamp_off"))
		m_sounds.LoadSound(section, "snd_headlamp_off", "sndHeadlampOff", false);
	if (pSettings->line_exist(section, "snd_nv_on"))
		m_sounds.LoadSound(section, "snd_nv_on", "sndNvOn", false);
	if (pSettings->line_exist(section, "snd_nv_off"))
		m_sounds.LoadSound(section, "snd_nv_off", "sndNvOff", false);
	if (pSettings->line_exist(section, "snd_clean_mask"))
		m_sounds.LoadSound(section, "snd_clean_mask", "sndCleanMask", false);
}


void CHudItem::PlaySound(LPCSTR alias, const Fvector& position)
{
	m_sounds.PlaySound	(alias, position, object().H_Root(), !!GetHUDmode());
}

void CHudItem::renderable_Render()
{
	UpdateXForm					();
	BOOL _hud_render			= ::Render->get_HUD() && GetHUDmode();
	
	if (!(_hud_render && !IsHidden()))
	{
		if (!object().H_Parent() || (!_hud_render && !IsHidden()))
		{
			on_renderable_Render		();
			debug_draw_firedeps			();
		}else
		if (object().H_Parent()) 
		{
			CInventoryOwner	*owner = smart_cast<CInventoryOwner*>(object().H_Parent());
			VERIFY			(owner);
			CInventoryItem	*self = smart_cast<CInventoryItem*>(this);
			if (item().GetSlot() != RIFLE_SLOT)
			{
				if (owner->attached(self))
					on_renderable_Render();
			}
			else
				on_renderable_Render();
		}
	}
}

void CHudItem::SwitchState(u32 S)
{
	if (OnClient()) 
		return;

	SetNextState( S );

	if (object().Local() && !object().getDestroy())	
	{
		// !!! Just single entry for given state !!!
		NET_Packet				P;
		object().u_EventGen		(P,GE_WPN_STATE_CHANGE,object().ID());
		P.w_u8					(u8(S));
		object().u_EventSend	(P);
	}
}

void CHudItem::OnEvent(NET_Packet& P, u16 type)
{
	switch (type)
	{
	case GE_WPN_STATE_CHANGE:
		{
			u8				S;
			P.r_u8			(S);
			OnStateSwitch	(u32(S));
		}
		break;
	}
}

void CHudItem::OnStateSwitch(u32 S)
{
	SetState			(S);
	
	if(object().Remote()) 
		SetNextState	(S);

	if (S == eHidden)
	{
		m_nearwall_last_hud_fov = m_base_fov > 0.0f ? m_base_fov : psHUD_FOV_def;
		m_bSprintType = false;
	}
	if (S != eIdle)
		m_bSprintType = false;

	switch (S)
	{
	case eBore:
		{
			SetPending		(FALSE);

			PlayAnimBore	();
			if (HudItemData())
			{
				Fvector P		= HudItemData()->m_item_transform.c;
				m_sounds.PlaySound("sndBore", P, object().H_Root(), !!GetHUDmode(), false, m_started_rnd_anim_idx);
			}

		} break;
	case eSprintStart:
		PlayAnimSprintStart();
		break;
	case eSprintEnd:
		{
			PlayAnimSprintEnd();
		} break;
	}
	if (psActorFlags3.test(AF_LFO_WPN_MOVEMENT_LAYER))
	{
		g_player_hud->updateMovementLayerState();
	}
}

void CHudItem::OnAnimationEnd(u32 state)
{
	CActor* actor = smart_cast<CActor*>(object().H_Parent());
	if (actor)
	{
		actor->callback(GameObject::eActorHudAnimationEnd)(smart_cast<CGameObject*>(this)->lua_game_object(),
			this->m_hud_sect.c_str(), this->m_current_motion.c_str(), state,
			this->animation_slot());
	}

	switch(state)
	{
	case eBore:
		{
			SwitchState	(eIdle);
		} break;
	case eSprintStart:
		{
			m_bSprintType = true;
			SwitchState(eIdle);

			if (m_sounds.FindSoundItem("sndSprintIdle", false))
				m_sounds.PlaySound("sndSprintIdle", HudItemData()->m_item_transform.c, object().H_Root(), !!GetHUDmode(), true, (u8)-1);
		} break;
	case eSprintEnd:
		{
			m_bSprintType = false;
			SwitchState(eIdle);
		} break;
	}
}

void CHudItem::OnMotionMark(u32 state, const motion_marks& M)
{
	luabind::functor<bool> funct;
	if (ai().script_engine().functor("mfs_functions.on_motion_mark", funct))
		funct(*M.name);
}

void CHudItem::PlayAnimBore()
{
	if (IsMisfireNow())
		PlayHUDMotionIfExists({ "anm_bore_jammed", "anm_bore" }, true, GetState());
	else if (IsMagazineEmpty())
		PlayHUDMotionIfExists({ "anm_bore_empty", "anm_bore" }, true, GetState());
	else
		PlayHUDMotion("anm_bore", TRUE, this, GetState());
}

bool CHudItem::ActivateItem() 
{
	OnActiveItem	();
	return			true;
}

void CHudItem::DeactivateItem() 
{
	OnHiddenItem	();
}
void CHudItem::OnMoveToRuck(EItemPlace prev)
{
	SwitchState(eHidden);
}

void CHudItem::SendDeactivateItem	()
{
	SendHiddenItem	();
}
void CHudItem::SendHiddenItem()
{
	if (!object().getDestroy())
	{
		NET_Packet				P;
		object().u_EventGen		(P,GE_WPN_STATE_CHANGE,object().ID());
		P.w_u8					(u8(eHiding));
		object().u_EventSend	(P, net_flags(TRUE, TRUE, FALSE, TRUE));
	}
}

void CHudItem::UpdateCL()
{
	if(m_current_motion_def)
	{
		if(m_bStopAtEndAnimIsRunning)
		{
			const xr_vector<motion_marks>&	marks = m_current_motion_def->marks;
			if(!marks.empty())
			{
				float motion_prev_time = ((float)m_dwMotionCurrTm - (float)m_dwMotionStartTm)/1000.0f;
				float motion_curr_time = ((float)Device.dwTimeGlobal - (float)m_dwMotionStartTm)/1000.0f;
				
				xr_vector<motion_marks>::const_iterator it = marks.begin();
				xr_vector<motion_marks>::const_iterator it_e = marks.end();
				for(;it!=it_e;++it)
				{
					const motion_marks&	M = (*it);
					if(M.is_empty())
						continue;
	
					const motion_marks::interval* Iprev = M.pick_mark(motion_prev_time);
					const motion_marks::interval* Icurr = M.pick_mark(motion_curr_time);
					if(Iprev==nullptr && Icurr!=nullptr /* || M.is_mark_between(motion_prev_time, motion_curr_time)*/)
					{
						OnMotionMark				(m_startedMotionState, M);
					}
				}
			
			}

			m_dwMotionCurrTm					= Device.dwTimeGlobal;
			if(m_dwMotionCurrTm > m_dwMotionEndTm)
			{
				m_current_motion_def				= nullptr;
				m_dwMotionStartTm					= 0;
				m_dwMotionEndTm						= 0;
				m_dwMotionCurrTm					= 0;
				m_bStopAtEndAnimIsRunning			= false;
				OnAnimationEnd						(m_startedMotionState);
			}
		}
	}
}

void CHudItem::OnH_A_Chield		()
{}

void CHudItem::OnH_B_Chield		()
{
	StopCurrentAnimWithoutCallback();

	m_nearwall_last_hud_fov = m_base_fov > 0.0f ? m_base_fov : psHUD_FOV_def;
}

void CHudItem::OnH_B_Independent	(bool just_before_destroy)
{
	m_sounds.StopAllSounds	();
	UpdateXForm				();

	m_nearwall_last_hud_fov = m_base_fov > 0.0f ? m_base_fov : psHUD_FOV_def;

	// next code was commented 
	/*
	if(HudItemData() && !just_before_destroy)
	{
		object().XFORM().set( HudItemData()->m_item_transform );
	}
	
	if (HudItemData())
	{
		g_player_hud->detach_item(this);
		Msg("---Detaching hud item [%s][%d]", this->HudSection().c_str(), this->object().ID());
	}*/
	//SetHudItemData			(NULL);
}

void CHudItem::OnH_A_Independent	()
{
	if(HudItemData())
		g_player_hud->detach_item(this);
	StopCurrentAnimWithoutCallback();
}

void CHudItem::on_b_hud_detach()
{
	m_sounds.StopAllSounds	();
}

void CHudItem::on_a_hud_attach()
{
	if(m_current_motion_def && m_current_motion.size())
	{
		PlayHUDMotion_noCB(m_current_motion, false);
#ifdef DEBUG
//		Msg("continue playing [%s][%d]",m_current_motion.c_str(), Device.dwFrame);
#endif // #ifdef DEBUG
	}
}

u32 CHudItem::PlayHUDMotion(const shared_str& M, BOOL bMixIn, CHudItem*  W, u32 state, float speed)
{
	u32 anim_time = PlayHUDMotion_noCB(M, bMixIn, speed);
	if (anim_time > 0)
	{
		m_bStopAtEndAnimIsRunning = true;
		m_dwMotionStartTm = Device.dwTimeGlobal;
		m_dwMotionCurrTm = m_dwMotionStartTm;
		m_dwMotionEndTm = m_dwMotionStartTm + anim_time;
		m_startedMotionState = state;
	}
	else
		m_bStopAtEndAnimIsRunning = false;

	return anim_time;
}

u32 CHudItem::PlayHUDMotionNew(const shared_str& M, const bool bMixIn, const u32 state, const bool randomAnim, float speed)
{
	//Msg("~~[%s] Playing motion [%s] for [%s]", __FUNCTION__, M.c_str(), HudSection().c_str());
	u32 anim_time					= PlayHUDMotion_noCB(M, bMixIn, speed);
	if (anim_time>0)
	{
		m_bStopAtEndAnimIsRunning	= true;
		m_dwMotionStartTm			= Device.dwTimeGlobal;
		m_dwMotionCurrTm			= m_dwMotionStartTm;
		m_dwMotionEndTm				= m_dwMotionStartTm + anim_time;
		m_startedMotionState		= state;
	}
	else
		m_bStopAtEndAnimIsRunning	= false;

	return anim_time;
}

//AVO: check if animation exists
bool CHudItem::isHUDAnimationExist(LPCSTR anim_name, bool withSuffix)
{
	if (HudItemData()) // First person
	{
		string256	anim_name_r, anim_name_def;
		bool is_16x9 = UI().is_widescreen();
		u16 attach_place_idx = pSettings->r_u16(HudItemData()->m_sect_name, "attach_place_idx");
		xr_sprintf(anim_name_r, "%s%s", anim_name, (attach_place_idx == 1 && is_16x9) ? "_16x9" : "");
		xr_sprintf(anim_name_def, "%s", anim_name);
		player_hud_motion* anm = HudItemData()->m_hand_motions.find_motion(anim_name_r, withSuffix);
		player_hud_motion* anm_def = HudItemData()->m_hand_motions.find_motion(anim_name_def, withSuffix);
		
		if (anm || anm_def)
			return true;
	}
	else // Third person
	{
		if (g_player_hud->motion_length(anim_name, HudSection(), m_current_motion_def) > 100)
			return true;
	}

#ifdef DEBUG
	Msg("~ [WARNING] [%s]: Animation [%s] does not exist in [%s]", __FUNCTION__, anim_name, HudSection().c_str());
#endif

	return false;
}

u32 CHudItem::PlayHUDMotionIfExists(std::initializer_list<const char*> Ms, const bool bMixIn, const u32 state, const bool randomAnim, float speed)
{
	for (const auto* M : Ms)
		if (isHUDAnimationExist(M))
			return PlayHUDMotionNew(M, bMixIn, state, randomAnim, speed);

	std::string dbg_anim_name;
	for (const auto* M : Ms)
	{
		dbg_anim_name += M;
		dbg_anim_name += ", ";
	}

#ifdef DEBUG
	Msg("~ [WARNING] [%s]: Motions [%s] not found for [%s]", __FUNCTION__, dbg_anim_name.c_str(), HudSection().c_str());
#endif

	return 0;
}

u32 CHudItem::PlayHUDMotion_noCB(const shared_str& motion_name, const bool bMixIn, const bool randomAnim, float speed)
{
	m_current_motion					= motion_name;

	if(bDebug && item().m_pInventory)
	{
		Msg("-[%s] as[%d] [%d]anim_play [%s][%d]",
			HudItemData()?"HUD":"Simulating", 
			item().m_pInventory->GetActiveSlot(), 
			item().object_id(),
			motion_name.c_str(), 
			Device.dwFrame);
	}
	if( HudItemData() )
	{
		return HudItemData()->anim_play		(motion_name, bMixIn, m_current_motion_def, m_started_rnd_anim_idx, speed);
	}
	else
	{
		m_started_rnd_anim_idx				= 0;
		return g_player_hud->motion_length	(motion_name, HudSection(), m_current_motion_def, speed );
	}
}

void CHudItem::StopCurrentAnimWithoutCallback()
{
	m_dwMotionStartTm			= 0;
	m_dwMotionEndTm				= 0;
	m_dwMotionCurrTm			= 0;
	m_bStopAtEndAnimIsRunning	= false;
	m_current_motion_def		= nullptr;
}

BOOL CHudItem::GetHUDmode()
{
	if(object().H_Parent())
	{
		CActor* A = smart_cast<CActor*>(object().H_Parent());
		return (A && A->HUDview() && HudItemData());
	}
	else
		return FALSE;
}

void CHudItem::PlayAnimIdle()
{
	if (TryPlayAnimIdle()) return;

	if (IsMisfireNow())
		PlayHUDMotionIfExists({ "anm_idle_jammed", "anm_idle" }, true, GetState());
	else if (IsMagazineEmpty())
		PlayHUDMotionIfExists({ "anm_idle_empty", "anm_idle" }, true, GetState());
	else
		PlayHUDMotion("anm_idle", TRUE, NULL, GetState());
}

bool CHudItem::TryPlayAnimIdle()
{
	if (MovingAnimAllowedNow() || !smart_cast<CWeapon*>(this))
	{
		if (auto pActor = smart_cast<CActor*>(object().H_Parent()))
		{
			const u32 State = pActor->get_state();
			if (State & mcSprint)
			{
				if (!m_bSprintType)
				{
					SwitchState(eSprintStart);
					return true;
				}

				PlayAnimIdleSprint();
				return true;
			}
			else if (m_sounds.FindSoundItem("sndSprintStart", false))
				m_sounds.StopSound("sndSprintIdle");

			if (m_bSprintType)
			{
				if ((State & mcClimb))
					return false;
				
				SwitchState(eSprintEnd);
				return true;
			}
			else if (State & mcAnyMove)
			{
				if (!(State & mcCrouch))
				{
					if (State & mcAccel) //Ходьба медленная (SHIFT)
						PlayAnimIdleMovingSlow();
					else
						PlayAnimIdleMoving();
					return true;
				}
				else if (State & mcAccel) //Ходьба в присяде (CTRL+SHIFT)
				{
					PlayAnimIdleMovingCrouchSlow();
					return true;
				}
				else
				{
					PlayAnimIdleMovingCrouch();
					return true;
				}
			}
		}
	}
	return false;
}

bool CHudItem::NeedBlendAnm()
{
	u32 state = GetState();
	return (state != eIdle && state != eHidden);
}

void CHudItem::PlayAnimIdleMoving()
{
	if (IsMisfireNow())
		PlayHUDMotionIfExists({ "anm_idle_moving_jammed", "anm_idle_moving", "anm_idle" }, true, GetState());
	else
		PlayHUDMotionIfExists({ "anm_idle_moving", "anm_idle"}, true, GetState());
}

void CHudItem::PlayAnimIdleMovingSlow()
{
	if (IsMisfireNow())
		PlayHUDMotionIfExists({ "anm_idle_moving_slow_jammed", "anm_idle_moving_slow", "anm_idle_moving", "anm_idle" }, true, GetState());
	else if (IsMagazineEmpty())
		PlayHUDMotionIfExists({ "anm_idle_moving_slow_empty", "anm_idle_moving_slow", "anm_idle_moving", "anm_idle" }, true, GetState());
	else
		PlayHUDMotionIfExists({ "anm_idle_moving_slow", "anm_idle_moving", "anm_idle"}, true, GetState());
}

void CHudItem::PlayAnimIdleMovingCrouch()
{
	if (IsMisfireNow())
		PlayHUDMotionIfExists({ "anm_idle_moving_crouch_jammed", "anm_idle_moving_crouch", "anm_idle_moving", "anm_idle" }, true, GetState());
	else if (IsMagazineEmpty())
		PlayHUDMotionIfExists({ "anm_idle_moving_crouch_empty", "anm_idle_moving_crouch", "anm_idle_moving", "anm_idle" }, true, GetState());
	else
		PlayHUDMotionIfExists({ "anm_idle_moving_crouch", "anm_idle_moving", "anm_idle"}, true, GetState());
}

void CHudItem::PlayAnimIdleMovingCrouchSlow()
{
	if (IsMisfireNow())
		PlayHUDMotionIfExists({ "anm_idle_moving_crouch_slow_jammed", "anm_idle_moving_crouch_slow", "anm_idle_moving_crouch", "anm_idle_moving", "anm_idle" }, true, GetState());
	else if (IsMagazineEmpty())
		PlayHUDMotionIfExists({ "anm_idle_moving_crouch_slow_empty", "anm_idle_moving_crouch_slow", "anm_idle_moving_crouch", "anm_idle_moving", "anm_idle" }, true, GetState());
	else
		PlayHUDMotionIfExists({ "anm_idle_moving_crouch_slow", "anm_idle_moving_crouch", "anm_idle_moving", "anm_idle"}, true, GetState());
}

void CHudItem::PlayAnimIdleSprint()
{
	if (IsMisfireNow())
		PlayHUDMotionIfExists({ "anm_idle_sprint_jammed", "anm_idle_sprint", "anm_idle" }, true, GetState());
	else
		PlayHUDMotionIfExists({ "anm_idle_sprint", "anm_idle"}, true, GetState());
}

void CHudItem::PlayAnimSprintStart()
{
	CWeapon* wpn = smart_cast<CWeapon*>(this);
	CWeaponMagazinedWGrenade* wpn_wgrenade = smart_cast<CWeaponMagazinedWGrenade*>(this);

	bool magazine_empty = false;

	if (wpn_wgrenade && wpn_wgrenade->IsGrenadeMode())
		magazine_empty = wpn_wgrenade->IsMainMagazineEmpty();
	else
		magazine_empty = IsMagazineEmpty();

	string_path guns_sprint_start_anm{};
	strconcat(sizeof(guns_sprint_start_anm), guns_sprint_start_anm, "anm_idle_sprint_start", (wpn && wpn->IsGrenadeLauncherAttached()) ? (wpn && wpn->IsGrenadeMode() ? "_g" : "_w_gl") : "", (IsMisfireNow() ? "_jammed" : (magazine_empty) ? "_empty" : ""));

	if (m_sounds.FindSoundItem("sndSprintStart", false))
		m_sounds.PlaySound("sndSprintStart", HudItemData()->m_item_transform.c, object().H_Root(), !!GetHUDmode(), false, (u8)-1);

	if (isHUDAnimationExist(guns_sprint_start_anm))
		PlayHUDMotionNew(guns_sprint_start_anm, true, GetState());
	else if (guns_sprint_start_anm && strstr(guns_sprint_start_anm, "_jammed"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_sprint_start_anm);
		new_guns_aim_anm[strlen(guns_sprint_start_anm) - strlen("_jammed")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
		else
		{
			m_bSprintType = true;
			SwitchState(eIdle);
		}
	}
	else if (guns_sprint_start_anm && strstr(guns_sprint_start_anm, "_empty"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_sprint_start_anm);
		new_guns_aim_anm[strlen(guns_sprint_start_anm) - strlen("_empty")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
		else
		{
			m_bSprintType = true;
			SwitchState(eIdle);
		}
	}
	else
	{
		m_bSprintType = true;
		SwitchState(eIdle);
	}
}

void CHudItem::PlayAnimSprintEnd()
{
	CWeapon* wpn = smart_cast<CWeapon*>(this);
	CWeaponMagazinedWGrenade* wpn_wgrenade = smart_cast<CWeaponMagazinedWGrenade*>(this);

	if (wpn && wpn->IsRotatingFromZoom())
	{
		m_bSprintType = false;
		SwitchState(eIdle);

		return;
	}

	bool magazine_empty = false;

	if (wpn_wgrenade && wpn_wgrenade->IsGrenadeMode())
		magazine_empty = wpn_wgrenade->IsMainMagazineEmpty();
	else
		magazine_empty = IsMagazineEmpty();

	string_path guns_sprint_end_anm{};
	strconcat(sizeof(guns_sprint_end_anm), guns_sprint_end_anm, "anm_idle_sprint_end", (wpn && wpn->IsGrenadeLauncherAttached()) ? (wpn && wpn->IsGrenadeMode() ? "_g" : "_w_gl") : "", (IsMisfireNow() ? "_jammed" : (magazine_empty) ? "_empty" : ""));

	if (m_sounds.FindSoundItem("sndSprintEnd", false))
		m_sounds.PlaySound("sndSprintEnd", HudItemData()->m_item_transform.c, object().H_Root(), !!GetHUDmode(), false, (u8)-1);

	if (isHUDAnimationExist(guns_sprint_end_anm))
		PlayHUDMotionNew(guns_sprint_end_anm, true, GetState());
	else if (guns_sprint_end_anm && strstr(guns_sprint_end_anm, "_jammed"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_sprint_end_anm);
		new_guns_aim_anm[strlen(guns_sprint_end_anm) - strlen("_jammed")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
		else
		{
			m_bSprintType = false;
			SwitchState(eIdle);
		}
	}
	else if (guns_sprint_end_anm && strstr(guns_sprint_end_anm, "_empty"))
	{
		char new_guns_aim_anm[256];
		strcpy(new_guns_aim_anm, guns_sprint_end_anm);
		new_guns_aim_anm[strlen(guns_sprint_end_anm) - strlen("_empty")] = '\0';

		if (isHUDAnimationExist(new_guns_aim_anm))
		{
			PlayHUDMotionNew(new_guns_aim_anm, true, GetState());
			return;
		}
		else
		{
			m_bSprintType = false;
			SwitchState(eIdle);
		}
	}
	else
	{
		m_bSprintType = false;
		SwitchState(eIdle);
	}
}

void CHudItem::OnMovementChanged(ACTOR_DEFS::EMoveCommand cmd)
{
	if(GetState()==eIdle && !m_bStopAtEndAnimIsRunning)
	{
		PlayAnimIdle();
		ResetSubStateTime();
	}
}

attachable_hud_item* CHudItem::HudItemData()
{
	attachable_hud_item* hi = nullptr;
	if(!g_player_hud)		
		return				hi;

	hi = g_player_hud->attached_item(0);
	if (hi && hi->m_parent_hud_item == this)
		return hi;

	hi = g_player_hud->attached_item(1);
	if (hi && hi->m_parent_hud_item == this)
		return hi;

	return NULL;
}

bool CHudItem::ParentIsActor()
{
	CObject* O = object().H_Parent();
	if (!O)
		return false;

	CEntityAlive* EA = smart_cast<CEntityAlive*>(O);
	if (!EA)
		return false;

	return EA->cast_actor() != nullptr;
}

void CHudItem::ReplaceHudSection(LPCSTR hud_section)
{
	if (hud_section != m_hud_sect)
		m_hud_sect = hud_section;
}

static BOOL pick_trace_callback(collide::rq_result& result, LPVOID params)
{
	collide::rq_result* RQ = (collide::rq_result*)params;
	if (!result.O)
	{
		// получить треугольник и узнать его материал
		CDB::TRI* T = Level().ObjectSpace.GetStaticTris() + result.element;
		if (T->material < GMLib.CountMaterial())
		{
			if (GMLib.GetMaterialByIdx(T->material)->Flags.is(SGameMtl::flPassable) || GMLib.GetMaterialByIdx(T->material)->Flags.is(SGameMtl::flActorObstacle))
				return TRUE;
		}
	}
	*RQ = result;
	return FALSE;
}

static float GetRayQueryDist()
{
	collide::rq_result RQ;
	g_pGameLevel->ObjectSpace.RayPick(Device.vCameraPosition, Device.vCameraDirection, 3.0f, collide::rqtStatic, RQ, Actor());
	if (!RQ.O)
	{
		CDB::TRI* T = Level().ObjectSpace.GetStaticTris() + RQ.element;
		if (T->material < GMLib.CountMaterial())
		{
			collide::rq_result  RQ2;
			collide::rq_results RQR;
			RQ2.range = 3.0f;
			collide::ray_defs RD(Device.vCameraPosition, Device.vCameraDirection, RQ2.range, CDB::OPT_CULL, collide::rqtStatic);
			if (Level().ObjectSpace.RayQuery(RQR, RD, pick_trace_callback, &RQ2, NULL, Level().CurrentEntity()))
			{
				clamp(RQ2.range, RQ.range, RQ2.range);
				return RQ2.range;
			}
		}
	}
	return RQ.range;
}

void CHudItem::UpdateInertion(Fmatrix& trans)
{
    if (HudInertionEnabled())
    {
        Fmatrix xform;
        Fvector& origin = trans.c;
        xform = trans;

        // calc difference
        Fvector diff_dir;
        diff_dir.sub(xform.k, inert_st_last_dir);

        // clamp by PI_DIV_2
        Fvector last;
        last.normalize_safe(inert_st_last_dir);
        const float dot = last.dotproduct(xform.k);
        if (dot < EPS)
        {
            Fvector v0;
            v0.crossproduct(inert_st_last_dir, xform.k);
            inert_st_last_dir.crossproduct(xform.k, v0);
            diff_dir.sub(xform.k, inert_st_last_dir);
        }

        // tend to forward
        float tendto_speed = GetCurrentHudOffsetIdx() > 0 ? inertion_data.m_tendto_speed_aim : inertion_data.m_tendto_speed;
        float origin_offset = GetCurrentHudOffsetIdx() > 0 ? inertion_data.m_origin_offset_aim : inertion_data.m_origin_offset;

        // Фактор силы инерции
        const float power_factor = GetInertionPowerFactor();
        tendto_speed *= power_factor;
        origin_offset *= power_factor;

        inert_st_last_dir.mad(diff_dir, tendto_speed * Device.fTimeDelta);
        origin.mad(diff_dir, origin_offset);

        // pitch compensation
        float pitch = angle_normalize_signed(xform.k.getP());
        pitch *= GetInertionFactor();

        Fvector current_offset{inertion_data.m_pitch_offset_d, inertion_data.m_pitch_offset_r, inertion_data.m_pitch_offset_n};
        current_offset.mul(-pitch);
        current_offset.mul(HudInertionAllowed());

        if (!current_offset.similar(current_pitch_offset, EPS))
            current_pitch_offset.lerp(current_pitch_offset, current_offset, tendto_speed * Device.fTimeDelta);

        // Отдаление\приближение
        origin.mad(xform.k, current_pitch_offset.x);

        // Сдвиг в противоположную часть экрана
        origin.mad(xform.i, current_pitch_offset.y);

        // Подьём\опускание
        clamp(pitch, inertion_data.m_pitch_low_limit, PI);
        origin.mad(xform.j, current_pitch_offset.z);
    }
}

// Обновление координат текущего худа
void CHudItem::UpdateHudAdditional(Fmatrix& trans)
{
    UpdateInertion(trans);

	CWeapon* wpn = smart_cast<CWeapon*>(this);
	CPda* pda = smart_cast<CPda*>(this);

    Fvector summary_offset{}, summary_rotate{};

    attachable_hud_item* hi = HudItemData();
    u8 idx = (pda && pda->Is3DPDA()) ? (pda->m_bZoomed ? 1 : 0) : GetCurrentHudOffsetIdx();
    const bool b_aiming = idx != 0;
    Fvector zr_offs = hi->m_measures.m_hands_offset[0][idx];
    Fvector zr_rot = hi->m_measures.m_hands_offset[1][idx];

    //============= Поворот ствола во время аима =============//
    if (wpn)
	{
        const float factor = Device.fTimeDelta / wpn->m_zoom_params.m_fZoomRotateTime;

        // I AM DEAD - Dirty hack from delayed exit from aim
        if (IsZoomed())
			wpn->m_zoom_params.m_fZoomRotationFactor += factor * 2.f;
        else
			wpn->m_zoom_params.m_fZoomRotationFactor -= factor * 2.f;

        clamp(wpn->m_zoom_params.m_fZoomRotationFactor, 0.f, 1.f);

        if (!zr_offs.similar(current_difference[0], EPS))
            current_difference[0].lerp(current_difference[0], zr_offs, factor * 2.f);

        if (!zr_rot.similar(current_difference[1], EPS))
            current_difference[1].lerp(current_difference[1], zr_rot, factor * 2.f);

        summary_offset.add(current_difference[0]);
    }
	else if (pda && pda->Is3DPDA())
	{
		const float factor = Device.fTimeDelta / .25f;

		// I AM DEAD - Dirty hack from delayed exit from aim
		if (IsZoomed())
			pda->m_fZoomfactor += factor;
		else
			pda->m_fZoomfactor -= factor;

		clamp(pda->m_fZoomfactor, 0.f, 1.f);

		if (!zr_offs.similar(current_difference[0], EPS))
			current_difference[0].lerp(current_difference[0], zr_offs, factor);

		if (!zr_rot.similar(current_difference[1], EPS))
			current_difference[1].lerp(current_difference[1], zr_rot, factor);

		summary_offset.add(current_difference[0]);

	}

	//============= Коллизия оружия =============//
	if (b_hud_collision)
	{
		float dist = GetRayQueryDist();

		Fvector curr_offs, curr_rot;
		curr_offs = hi->m_measures.m_collision_offset[0];//pos,aim
		curr_rot = hi->m_measures.m_collision_offset[1];//rot,aim
		curr_offs.mul(m_fHudCollisionFactor);
		curr_rot.mul(m_fHudCollisionFactor);

		float m_fColPosition;
		float m_fColRotation;

		if (dist <= 0.8 && !IsZoomed())
		{
			m_fColPosition = curr_offs.y + ((1 - dist - 0.2) * 5.0f);
			m_fColRotation = curr_rot.x + ((1 - dist - 0.2) * 5.0f);
		}
		else
		{
			m_fColPosition = curr_offs.y;
			m_fColRotation = curr_rot.x;
		}

		if (m_fHudCollisionFactor < m_fColPosition)
		{
			m_fHudCollisionFactor += Device.fTimeDelta / 0.3;
			if (m_fHudCollisionFactor > m_fColPosition)
				m_fHudCollisionFactor = m_fColPosition;
		}
		else if (m_fHudCollisionFactor > m_fColPosition)
		{
			m_fHudCollisionFactor -= Device.fTimeDelta / 0.3;
			if (m_fHudCollisionFactor < m_fColPosition)
				m_fHudCollisionFactor = m_fColPosition;
		}

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

		clamp(m_fHudCollisionFactor, 0.f, 1.f);
	}
	else
	{
		m_fHudCollisionFactor = 0.0;
	}

    //====================================================//

    auto pActor = smart_cast<const CActor*>(object().H_Parent());
    const u32 iMovingState = pActor->MovingState();
    idx = b_aiming ? 1ui8 : 0ui8;

    //============= Боковой стрейф с оружием =============//
    {
        const bool bEnabled = m_strafe_offset[2][idx].x;
        if (bEnabled)
        {
            float fStrafeMaxTime = m_strafe_offset[2][idx].y; // Макс. время в секундах, за которое произойдет смещение худа при стрейфах

            if (fStrafeMaxTime <= EPS)
                fStrafeMaxTime = 0.01f;

            const float fStepPerUpd = Device.fTimeDelta / fStrafeMaxTime; // Величина изменение фактора смещения худа при стрейфах

            Fvector current_moving_offs{}, current_moving_rot{};

            if (iMovingState & mcLStrafe) // Двигаемся влево
            {
                current_moving_offs.set(-m_strafe_offset[0][idx]);
                current_moving_rot.set(-m_strafe_offset[1][idx]);
            }
            else if (iMovingState & mcRStrafe) // Двигаемся вправо
            {
                current_moving_offs.set(m_strafe_offset[0][idx]);
                current_moving_rot.set(m_strafe_offset[1][idx]);
            }
            else // Двигаемся в любом другом направлении
            {
                current_moving_offs.set(Fvector{});
                current_moving_rot.set(Fvector{});
            }

            current_moving_rot.mul(-PI / 180.f); // Преобразуем углы в радианы

            if (!current_moving_offs.similar(current_strafe[0], EPS))
                current_strafe[0].lerp(current_strafe[0], current_moving_offs, fStepPerUpd);

            if (!current_moving_rot.similar(current_strafe[1], EPS))
                current_strafe[1].lerp(current_strafe[1], current_moving_rot, fStepPerUpd);

            summary_offset.add(current_strafe[0]);
            summary_rotate.add(current_strafe[1]);
        }
    }

    //=============== Эффекты прыжка ===============//
    {
        const bool bEnabled = m_jump_offset[2][idx].x;
        if (bEnabled)
        {
            float fJumpMaxTime = m_jump_offset[2][idx].y; // Макс. время в секундах, за которое произойдет смещение худа при прыжке

            if (fJumpMaxTime <= EPS)
                fJumpMaxTime = 0.01f;

            float fStepPerUpd = Device.fTimeDelta / fJumpMaxTime; // Величина изменение фактора смещения худа при прыжке

            Fvector current_jump_offs{}, current_jump_rot{};

            if (iMovingState & mcJump) // Прыжок
            {
                current_jump_offs.set(m_jump_offset[0][idx]);
                current_jump_rot.set(m_jump_offset[1][idx]);
            }
            else if (iMovingState & mcFall) // Полет
            {
                current_jump_offs.set(m_fall_offset[0][idx]);
                current_jump_rot.set(m_fall_offset[1][idx]);
            }
            else if (iMovingState & mcLanding || iMovingState & mcLanding2) // Полет
            {
                current_jump_offs.set(m_landing_offset[0][idx]);
                current_jump_rot.set(m_landing_offset[1][idx]);
            }
            else
            {
                current_jump_offs.set(Fvector{});
                current_jump_rot.set(Fvector{});
                fStepPerUpd = Device.fTimeDelta / (fJumpMaxTime * 0.5);
            }

			float koef = iMovingState & mcLanding2 ? 1.3 : 1.0;
            current_jump_offs.mul(koef);
            current_jump_rot.mul(koef);
            current_jump_rot.mul(-PI / 180.f); // Преобразуем углы в радианы

            if (!current_jump_offs.similar(current_jump[0], EPS))
                current_jump[0].lerp(current_jump[0], current_jump_offs, fStepPerUpd);

            if (!current_jump_rot.similar(current_jump[1], EPS))
                current_jump[1].lerp(current_jump[1], current_jump_rot, fStepPerUpd);

            summary_offset.add(current_jump[0]);
            summary_rotate.add(current_jump[1]);
        }
    }

    //=============== Эффекты наклонов ===================//
    {
        const bool bEnabled = m_lookout_offset[2][idx].x;
        if (bEnabled)
        {
            float fLookoutMaxTime = m_lookout_offset[2][idx].y; // Макс. время в секундах, за которое мы наклонимся из центрального положения
            if (fLookoutMaxTime <= EPS)
                fLookoutMaxTime = 0.01f;

            const float fStepPerUpd = Device.fTimeDelta / fLookoutMaxTime; // Величина изменение фактора поворота

            float koef{1.f};
            if ((iMovingState & mcCrouch) && (iMovingState & mcAccel))
                koef = 0.5; // во сколько раз менять амплитуду при полном присяде
            else if (iMovingState & mcCrouch)
                koef = 0.75; // во сколько раз менять амплитуду при присяде

            Fvector current_lookout_offs{}, current_lookout_rot{};

            if ((iMovingState & mcLLookout) && !(iMovingState & mcRLookout)) // Выглядываем влево
            {
                current_lookout_offs.set(-m_lookout_offset[0][idx]);
                current_lookout_rot.set(-m_lookout_offset[1][idx]);
            }
            else if ((iMovingState & mcRLookout) && !(iMovingState & mcLLookout)) // Выглядываем вправо
            {
                current_lookout_offs.set(m_lookout_offset[0][idx]);
                current_lookout_rot.set(m_lookout_offset[1][idx]);
            }
            else
            {
                current_lookout_offs.set(Fvector{});
                current_lookout_rot.set(Fvector{});
            }

            current_lookout_offs.mul(koef);
            current_lookout_rot.mul(koef);
            current_lookout_rot.mul(-PI / 180.f); // Преобразуем углы в радианы

            if (!current_lookout_offs.similar(current_lookout[0], EPS))
                current_lookout[0].lerp(current_lookout[0], current_lookout_offs, fStepPerUpd);

            if (!current_lookout_rot.similar(current_lookout[1], EPS))
                current_lookout[1].lerp(current_lookout[1], current_lookout_rot, fStepPerUpd);

            summary_offset.add(current_lookout[0]);
            summary_rotate.add(current_lookout[1]);
        }
    }

    //=============== Эффекты стойки ===================//
    {
        const bool bEnabled = m_move_offset[2].x;
        if (bEnabled)
        {
            float fMoveMaxTime = m_move_offset[2].y; // Макс. время в секундах, за которое мы наклонимся из центрального положения
            if (fMoveMaxTime <= EPS)
                fMoveMaxTime = 0.01f;

            const float fStepPerUpd = Device.fTimeDelta / fMoveMaxTime; // Величина изменение фактора поворота

            float koef{};
            if ((iMovingState & mcCrouch) && (iMovingState & mcAccel))
                koef = 1.0; // во сколько раз менять амплитуду при полном присяде
            else if (iMovingState & mcCrouch)
                koef = 0.5; // во сколько раз менять амплитуду при присяде

            Fvector current_move_offs{}, current_move_rot{};

            if (iMovingState & mcCrouch) // Выглядываем влево
            {
                current_move_offs.set(m_move_offset[0]);
                current_move_rot.set(m_move_offset[1]);
            }
            else
            {
                current_move_offs.set(Fvector{});
                current_move_rot.set(Fvector{});
            }

            auto missile = smart_cast<CMissile*>(this);

            current_move_offs.mul(koef);
            current_move_rot.mul(koef);
            current_move_offs.mul(!IsZoomed());
            current_move_rot.mul(!IsZoomed());
            current_move_offs.mul(!pda && !missile);
            current_move_rot.mul(!pda && !missile);
            current_move_rot.mul(-PI / 180.f); // Преобразуем углы в радианы

            if (!current_move_offs.similar(current_move[0], EPS))
                current_move[0].lerp(current_move[0], current_move_offs, fStepPerUpd);

            if (!current_move_rot.similar(current_move[1], EPS))
                current_move[1].lerp(current_move[1], current_move_rot, fStepPerUpd);

            summary_offset.add(current_move[0]);
            summary_rotate.add(current_move[1]);
        }
    }

    //=============== Эффекты ходьбы ===================//
    {
        const bool bEnabled = m_walk_offset[2].x;
        if (bEnabled)
        {
            float fWalkMaxTime = m_walk_offset[2].y; // Макс. время в секундах, за которое мы наклонимся из центрального положения
            if (fWalkMaxTime <= EPS)
                fWalkMaxTime = 0.01f;

            const float fStepPerUpd = Device.fTimeDelta / fWalkMaxTime; // Величина изменение фактора поворота

            float koef{1.f};
            if ((iMovingState & mcCrouch) && (iMovingState & mcAccel))
                koef = 0.5; // во сколько раз менять амплитуду при полном присяде
            else if (iMovingState & mcCrouch)
                koef = 0.7; // во сколько раз менять амплитуду при присяде

            Fvector current_walk_offs{}, current_walk_rot{};

            if (iMovingState & mcFwd)
            {
                current_walk_offs.set(m_walk_offset[0]);
                current_walk_rot.set(m_walk_offset[1]);
            }
            else if (iMovingState & mcBack)
            {
                current_walk_offs.set(m_walk_offset[0].x, m_walk_offset[0].y, -m_walk_offset[0].z);
                current_walk_rot.set(m_walk_offset[1]);
            }
            else
            {
                current_walk_offs.set(Fvector{});
                current_walk_rot.set(Fvector{});
            }

            auto pda = smart_cast<CPda*>(this);
            auto missile = smart_cast<CMissile*>(this);

            current_walk_offs.mul(!IsZoomed());
            current_walk_rot.mul(!IsZoomed());
            current_walk_offs.mul(!pda && !missile);
            current_walk_rot.mul(!pda && !missile);
            current_walk_offs.mul(koef);
            current_walk_rot.mul(koef);
            current_walk_rot.mul(-PI / 180.f); // Преобразуем углы в радианы

            if (!current_walk_offs.similar(current_walk[0], EPS))
                current_walk[0].lerp(current_walk[0], current_walk_offs, fStepPerUpd);

            if (!current_walk_rot.similar(current_walk[1], EPS))
                current_walk[1].lerp(current_walk[1], current_walk_rot, fStepPerUpd);

            summary_offset.add(current_walk[0]);
            summary_rotate.add(current_walk[1]);
        }
    }

    //=============== Эффекты тряски от стрельбы ===================//
	if (bReloadShooting && wpn)
	{
		// Параметры сдвига
		//--> Длительность (== скорость) затухания эффекта ...
		float fShootingStabilizeTime = (GetState() == wpn->eFire ?
			//--> ... во время анимации стрельбы
			lerp(m_shooting_params.m_ret_time_fire[0], //--> от бедра
				m_shooting_params.m_ret_time_fire[1], //--> в зуме
				wpn->m_zoom_params.m_fZoomRotationFactor) :
			//--> ... после анимации стрельбы
			lerp(m_shooting_params.m_ret_time[0], //--> от бедра
				m_shooting_params.m_ret_time[1], //--> в зуме
				wpn->m_zoom_params.m_fZoomRotationFactor));

		if (fShootingStabilizeTime <= EPS)
			fShootingStabilizeTime = 0.01f;

		const float fStepPerUpd = Device.fTimeDelta / (fShootingStabilizeTime * 0.25); // Величина изменение фактора поворота

		//--> Сдвиг по Z
		float fShootingBackwOffset = lerp(
			m_shooting_params.m_shot_offset_BACKW, //--> от бедра
			m_shooting_params.m_shot_offset_BACKW_aim, //--> в зуме
			wpn->m_zoom_params.m_fZoomRotationFactor);

		//--> Сдвиг по X, Y
		Fvector4 vShOffsets; // 0 = -X, 1 = +X, 2 = +Y, 3 = -Y
		vShOffsets[0] = lerp(
			m_shooting_params.m_shot_max_offset_LRUD[0], //--> от бедра
			m_shooting_params.m_shot_max_offset_LRUD_aim[0], //--> в зуме
			wpn->m_zoom_params.m_fZoomRotationFactor);
		vShOffsets[1] = lerp(
			m_shooting_params.m_shot_max_offset_LRUD[1], //--> от бедра
			m_shooting_params.m_shot_max_offset_LRUD_aim[1], //--> в зуме
			wpn->m_zoom_params.m_fZoomRotationFactor);
		vShOffsets[2] = lerp(
			m_shooting_params.m_shot_max_offset_LRUD[2], //--> от бедра
			m_shooting_params.m_shot_max_offset_LRUD_aim[2], //--> в зуме
			wpn->m_zoom_params.m_fZoomRotationFactor);
		vShOffsets[3] = lerp(
			m_shooting_params.m_shot_max_offset_LRUD[3], //--> от бедра
			m_shooting_params.m_shot_max_offset_LRUD_aim[3], //--> в зуме
			wpn->m_zoom_params.m_fZoomRotationFactor);

		//--> Поворот по оси X
		Fvector4 vShRotX; // 0 = При смещении вверх, 1 = при смещении вниз
		vShRotX[0] = lerp(
			m_shooting_params.m_shot_max_rot_UD[0], //--> от бедра
			m_shooting_params.m_shot_max_rot_UD_aim[0], //--> в зуме
			wpn->m_zoom_params.m_fZoomRotationFactor);
		vShRotX[1] = lerp(
			m_shooting_params.m_shot_max_rot_UD[1], //--> от бедра
			m_shooting_params.m_shot_max_rot_UD_aim[1], //--> в зуме
			wpn->m_zoom_params.m_fZoomRotationFactor);

		// Применяем сдвиг от стрельбы к HUD-у
		{
			Fvector shoot_offs, shoot_rot;

			//--> Рассчитываем основу сдвига
			shoot_offs = {
				//--> Горизонтальный сдвиг
				lerp(vShOffsets[0], vShOffsets[1], m_fShootingFactorLR) * m_fShootingCurPowerLRUD,
				//--> Вертикальный сдвиг
				lerp(vShOffsets[2], vShOffsets[3], m_fShootingFactorUD) * m_fShootingCurPowerLRUD,
				//--> Глубинный сдвиг
				-1.f * fShootingBackwOffset * m_fShootingCurPowerBACKW
			};
			shoot_rot = {
				//--> Поворот по вертикали
				lerp(vShRotX[0], vShRotX[1], m_fShootingFactorUD) * m_fShootingCurPowerLRUD,
				0.0f,
				0.0f
			};
			shoot_rot.mul(-PI / 180.f);

			//--> Модифицируем её коэфицентом силы сдвига
			float fShootingKoef = GetShootingEffectKoef();
			shoot_offs.mul(fShootingKoef);
			shoot_rot.mul(fShootingKoef);

			if (!shoot_offs.similar(current_shooting[0], EPS))
				current_shooting[0].lerp(current_shooting[0], shoot_offs, fStepPerUpd);

			if (!shoot_rot.similar(current_shooting[1], EPS))
				current_shooting[1].lerp(current_shooting[1], shoot_rot, fStepPerUpd);

			//--> Применяем к HUD-у
			//summary_offset.add(shoot_offs);
			//summary_rotate.add(shoot_rot);
			summary_offset.add(current_shooting[0]);
			summary_rotate.add(current_shooting[1]);
		}

		// Плавное затухание сдвига от стрельбы
		//--> Глубинный сдвиг
		float fBackwStabilizeTimeKoef = m_shooting_params.m_ret_time_backw_koef;
		m_fShootingCurPowerBACKW -= Device.fTimeDelta / (fShootingStabilizeTime * fBackwStabilizeTimeKoef * 0.25);
		clamp(m_fShootingCurPowerBACKW, 0.0f, 1.0f);

		//--> Боковой сдвиг
		m_fShootingCurPowerLRUD -= Device.fTimeDelta / fShootingStabilizeTime * 0.25;
		if (m_fShootingCurPowerLRUD <= 0.0f)
		{
			ResetShootingEffect(true);
		}
	}

    //================ Применение эффектов ===============//
    {
        // поворот с сохранением смещения by Zander
        Fvector _angle{}, _pos{trans.c};
        trans.getHPB(_angle);
        _angle.add(-summary_rotate);

        // Msg("##[%s] summary_rotate: [%f,%f,%f]", __FUNCTION__, summary_rotate.x, summary_rotate.y, summary_rotate.z);
        trans.setHPB(_angle.x, _angle.y, _angle.z);
        trans.c = _pos;

        Fmatrix hud_rotation;
        hud_rotation.identity();

        //================ Прицеливание ===============//
        hud_rotation.rotateX(current_difference[1].x);

        Fmatrix hud_rotation_part;
        hud_rotation_part.identity();
        hud_rotation_part.rotateY(current_difference[1].y);
        hud_rotation.mulA_43(hud_rotation_part);

        hud_rotation_part.identity();
        hud_rotation_part.rotateZ(current_difference[1].z);
        hud_rotation.mulA_43(hud_rotation_part);

        //Msg("--[%s] summary_offset: [%f,%f,%f]", __FUNCTION__, summary_offset.x, summary_offset.y, summary_offset.z);

        hud_rotation.translate_over(summary_offset);
        trans.mulB_43(hud_rotation);
    }
    //====================================================//
}
// Добавить HUD-эффект сдвига оружия от выстрела
void CHudItem::AddHUDShootingEffect()
{
	CWeapon* wpn = smart_cast<CWeapon*>(this);
	if (!wpn || IsHidden() || ParentIsActor() == false)
		return;

	attachable_hud_item* hi = HudItemData();
	if (hi == nullptr)
		return;

	// Отдача назад всегда максимальная на каждом выстреле
	m_fShootingCurPowerBACKW = 1.0f;

	// Отдача в бока становится сильнее при длительной стрельбе
	//--> Регулируем плавность перемещения оружия по экрану, ограничивая макс. сдвиг на каждый выстрел
	float fLRUDDiffPerShot = lerp(m_shooting_params.m_shot_diff_per_shot[0],
		m_shooting_params.m_shot_diff_per_shot[1], wpn->m_zoom_params.m_fZoomRotationFactor);
	clamp(fLRUDDiffPerShot, 0.0f, 1.0f);

	m_fShootingFactorLR += ::Random.randF(-fLRUDDiffPerShot, fLRUDDiffPerShot); //--> m_fShootingFactorLR будет 0.5f на момент начала стрельбы
	clamp(m_fShootingFactorLR, 0.0f, 1.0f);

	m_fShootingFactorUD += ::Random.randF(-fLRUDDiffPerShot, fLRUDDiffPerShot); //--> m_fShootingFactorUD будет 0.5f на момент начала стрельбы
	clamp(m_fShootingFactorUD, 0.0f, 1.0f);

	//--> С каждым выстрелом разрешаем оружию всё ближе приближаться к текущим границам сдвига
	m_fShootingCurPowerLRUD += lerp(m_shooting_params.m_shot_power_per_shot[0],
		m_shooting_params.m_shot_power_per_shot[1], wpn->m_zoom_params.m_fZoomRotationFactor);
	clamp(m_fShootingCurPowerLRUD, 0.0f, 1.0f);

	// Наклон ствола от стрельбы (стрейф)
	float fShotStrafeMin = lerp(m_shooting_params.m_shot_offsets_strafe[0],
		m_shooting_params.m_shot_offsets_strafe_aim[0], wpn->m_zoom_params.m_fZoomRotationFactor);
	float fShotStrafeMax = lerp(m_shooting_params.m_shot_offsets_strafe[1],
		m_shooting_params.m_shot_offsets_strafe_aim[1], wpn->m_zoom_params.m_fZoomRotationFactor);

	float fLRShotStrafeDir = (::Random.randF(-1.0f, 1.0f) >= 0.0f ? 1.0f : -1.0f);
	float fStrafeVal = (::Random.randF(fShotStrafeMin, fShotStrafeMax) * fLRShotStrafeDir);
	float fStrafePwr = clampr(m_fShootingCurPowerLRUD * 2.0f, 0.0f, 1.0f);

	current_shooting[0].mul(fStrafeVal * GetShootingEffectKoef() * fStrafePwr * 4.0f);
	current_shooting[1].mul(fStrafeVal * GetShootingEffectKoef() * fStrafePwr * 4.0f);
}

// Получить коэфицент силы тряски HUD-a при стрельбе
float CHudItem::GetShootingEffectKoef()
{
	float fShakeKoef = 1.0f;

	//--> Глушитель
	//fShakeKoef *= cur_silencer_koef.shooting_shake;

	return fShakeKoef;
}

void CHudItem::merge_measures_params()
{
	// Смещение от стрельбы
	if (bReloadShooting) //-> хз зачем	
	{
		m_shooting_params.m_shot_max_offset_LRUD = m_shooting_params.m_shot_max_offset_LRUD;
		m_shooting_params.m_shot_max_offset_LRUD_aim = m_shooting_params.m_shot_max_offset_LRUD_aim;
		m_shooting_params.m_shot_max_rot_UD = m_shooting_params.m_shot_max_rot_UD;
		m_shooting_params.m_shot_max_rot_UD_aim = m_shooting_params.m_shot_max_rot_UD_aim;
		m_shooting_params.m_shot_offset_BACKW = m_shooting_params.m_shot_offset_BACKW;
		m_shooting_params.m_shot_offset_BACKW_aim = m_shooting_params.m_shot_offset_BACKW_aim;
		m_shooting_params.m_shot_offsets_strafe = m_shooting_params.m_shot_offsets_strafe;
		m_shooting_params.m_shot_offsets_strafe_aim = m_shooting_params.m_shot_offsets_strafe_aim;
		m_shooting_params.m_shot_diff_per_shot = m_shooting_params.m_shot_diff_per_shot;
		m_shooting_params.m_shot_power_per_shot = m_shooting_params.m_shot_power_per_shot;
		m_shooting_params.m_ret_time = m_shooting_params.m_ret_time;
		m_shooting_params.m_ret_time_fire = m_shooting_params.m_ret_time_fire;
		m_shooting_params.m_ret_time_backw_koef = m_shooting_params.m_ret_time_backw_koef;
	}
}

float CHudItem::GetHudFov()
{
	if (m_nearwall_enabled && ParentIsActor() && Level().CurrentViewEntity() == object().H_Parent())
	{
		collide::rq_result& RQ = HUD().GetCurrentRayQuery();
		float dist = RQ.range;

		clamp(dist, m_nearwall_dist_min, m_nearwall_dist_max);
		const float fDistanceMod = ((dist - m_nearwall_dist_min) / (m_nearwall_dist_max - m_nearwall_dist_min)); // 0.f ... 1.f

		float fBaseFov{ psHUD_FOV_def };

		fBaseFov = (m_base_fov > 0.0f ? m_base_fov : psHUD_FOV_def);
		clamp(fBaseFov, 0.0f, FLT_MAX);

		float src = m_nearwall_speed_mod * Device.fTimeDelta;
		clamp(src, 0.f, 1.f);

		const float fTrgFov = (IsZoomed() ? m_nearwall_target_aim_hud_fov : m_nearwall_target_hud_fov) + 
			fDistanceMod * (fBaseFov - (IsZoomed() ? m_nearwall_target_aim_hud_fov : m_nearwall_target_hud_fov));
		m_nearwall_last_hud_fov = m_nearwall_last_hud_fov * (1 - src) + fTrgFov * src;
	}

	return m_nearwall_last_hud_fov;
}

void CHudItem::TimeLockAnimation()
{
	if (GetState() != eDeviceSwitch && GetState() != eLaserSwitch && GetState() != eFlashlightSwitch)
		return;

	string128 anm_time_param;
	xr_strconcat(anm_time_param, "lock_time_", m_current_motion.c_str(), "_end");
	const float time = READ_IF_EXISTS(pSettings, r_float, HudSection(), anm_time_param, 0) * 1000.f; // Читаем с конфига время анимации (например, lock_time_end_anm_reload)
	const float current_time = Device.dwTimeGlobal - m_dwMotionStartTm;

	if (time && current_time >= time)
		DeviceUpdate();
}