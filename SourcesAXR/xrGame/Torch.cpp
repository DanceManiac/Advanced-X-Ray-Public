#include "stdafx.h"
#include "torch.h"
#include "entity.h"
#include "actor.h"
#include "../xrEngine/LightAnimLibrary.h"
#include "../xrphysics/PhysicsShell.h"
#include "xrserver_objects_alife_items.h"
#include "ai_sounds.h"

#include "level.h"
#include "../Include/xrRender/Kinematics.h"
#include "../xrEngine/camerabase.h"
#include "../xrengine/xr_collide_form.h"
#include "inventory.h"
#include "game_base_space.h"

#include "UIGameCustom.h"
#include "CustomOutfit.h"
#include "ActorHelmet.h"
#include "../xrPhysics/ElevatorState.h"
#include "player_hud.h"
#include "Inventory.h"
#include "Weapon.h"
#include "ActorEffector.h"
#include "AdvancedXrayGameConstants.h"
#include "Battery.h"
#include "CustomDetector.h"

std::atomic<bool> isHidingInProgressTorch(false);
std::atomic<bool> processSwitchNeeded(false);

static const float		TORCH_INERTION_CLAMP		= PI_DIV_6;
static const float		TORCH_INERTION_SPEED_MAX	= 7.5f;
static const float		TORCH_INERTION_SPEED_MIN	= 0.5f;
static		 Fvector	TORCH_OFFSET				= {-0.2f,+0.1f,-0.3f};
static const Fvector	OMNI_OFFSET					= {-0.2f,+0.1f,-0.1f};
static const float		OPTIMIZATION_DISTANCE		= 100.f;

static bool stalker_use_dynamic_lights	= false;

ENGINE_API int g_current_renderer;

extern bool g_block_all_except_movement;

CTorch::CTorch(void) 
{
	light_render				= ::Render->light_create();
	light_render->set_type		(IRender_Light::SPOT);
	light_render->set_shadow	(true);
	light_omni					= ::Render->light_create();
	light_omni->set_type		(IRender_Light::POINT);
	light_omni->set_shadow		(false);

	m_switched_on				= false;
	glow_render					= ::Render->glow_create();
	lanim						= 0;
	fBrightness					= 1.f;

	m_prev_hp.set				(0,0);
	m_delta_h					= 0;

	m_fMaxChargeLevel			= 0.0f;
	m_fCurrentChargeLevel		= 1.0f;
	m_fUnchargeSpeed			= 0.0f;
	m_fMaxRange					= 20.f;
	m_fCurveRange				= 20.f;

	m_torch_offset				= TORCH_OFFSET;
	m_omni_offset				= OMNI_OFFSET;
	m_torch_inertion_speed_max	= TORCH_INERTION_SPEED_MAX;
	m_torch_inertion_speed_min	= TORCH_INERTION_SPEED_MIN;

	m_iAnimLength				= 0;
	m_iActionTiming				= 0;
	m_bActivated				= false;
	m_bSwitched					= false;
}

CTorch::~CTorch() 
{
	light_render.destroy	();
	light_omni.destroy		();
	glow_render.destroy		();
}

void CTorch::OnMoveToSlot(const SInvItemPlace& prev)
{
	CInventoryOwner* owner = smart_cast<CInventoryOwner*>(H_Parent());
	if (owner && !owner->attached(this))
	{
		owner->attach(this->cast_inventory_item());
	}
}

void CTorch::OnMoveToRuck(const SInvItemPlace& prev)
{
	if (prev.type == eItemPlaceSlot)
	{
		Switch(false);
	}
}

inline bool CTorch::can_use_dynamic_lights	()
{
	if (!H_Parent())
		return				(true);

	CInventoryOwner			*owner = smart_cast<CInventoryOwner*>(H_Parent());
	if (!owner)
		return				(true);

	return					(owner->can_use_dynamic_lights());
}

void CTorch::Load(LPCSTR section) 
{
	inherited::Load			(section);
	light_trace_bone		= pSettings->r_string(section,"light_trace_bone");

	m_light_section = READ_IF_EXISTS(pSettings, r_string, section, "light_section", "torch_definition");
	m_fMaxChargeLevel = READ_IF_EXISTS(pSettings, r_float, section, "max_charge_level", 1.0f);
	m_fUnchargeSpeed = READ_IF_EXISTS(pSettings, r_float, section, "uncharge_speed", 0.0f);

	if (pSettings->line_exist(section, "snd_turn_on"))
		m_sounds.LoadSound(section, "snd_turn_on", "sndTurnOn", false, SOUND_TYPE_ITEM_USING);
	if (pSettings->line_exist(section, "snd_turn_off"))
		m_sounds.LoadSound(section, "snd_turn_off", "sndTurnOff", false, SOUND_TYPE_ITEM_USING);

	m_SuitableBatteries.clear();
	LPCSTR batteries = READ_IF_EXISTS(pSettings, r_string, section, "suitable_batteries", "torch_battery");

	if (batteries && batteries[0])
	{
		string128 battery_sect;
		int count = _GetItemCount(batteries);
		for (int it = 0; it < count; ++it)
		{
			_GetItem(batteries, it, battery_sect);
			m_SuitableBatteries.push_back(battery_sect);
		}
	}

	//Случайный начальный заряд батареек в фонарике, если включена опция ограниченного заряда батареек у фонарика
	if (GameConstants::GetTorchHasBattery())
	{
		float rnd_charge = ::Random.randF(0.0f, m_fMaxChargeLevel);
		m_fCurrentChargeLevel = rnd_charge;
	}

	m_torch_offset = READ_IF_EXISTS(pSettings, r_fvector3, section, "torch_offset", TORCH_OFFSET);
	m_omni_offset = READ_IF_EXISTS(pSettings, r_fvector3, section, "omni_offset", OMNI_OFFSET);
	m_torch_inertion_speed_max = READ_IF_EXISTS(pSettings, r_float, section, "torch_inertion_speed_max", TORCH_INERTION_SPEED_MAX);
	m_torch_inertion_speed_min = READ_IF_EXISTS(pSettings, r_float, section, "torch_inertion_speed_min", TORCH_INERTION_SPEED_MIN);

	// Disabling shift by x and z axes for 1st render, 
	// because we don't have dynamic lighting in it. 
	if (g_current_renderer == 1)
	{
		m_torch_offset.x = 0;
		m_torch_offset.z = 0;
	}
}

void CTorch::Switch()
{
	if (OnClient())
		return;

	if (isHidingInProgressTorch.load())
		return;

	CCustomDetector* pDet = smart_cast<CCustomDetector*>(Actor()->inventory().ItemFromSlot(DETECTOR_SLOT));

	if (!pDet || pDet->IsHidden())
	{
		ProcessSwitch();
		return;
	}

	isHidingInProgressTorch.store(true);

	std::thread hidingThread([&, pDet]
	{
		while (pDet && !pDet->IsHidden())
			pDet->HideDetector(true);

		isHidingInProgressTorch.store(false);
		processSwitchNeeded.store(true);
	});

	hidingThread.detach();
}

void CTorch::ProcessSwitch()
{
	if (OnClient())
		return;

	bool bActive			= !m_switched_on;

	LPCSTR anim_sect = READ_IF_EXISTS(pAdvancedSettings, r_string, "actions_animations", "switch_torch_section", nullptr);

	if (!anim_sect)
	{
		Switch(bActive);
		return;
	}

	CWeapon* Wpn = smart_cast<CWeapon*>(Actor()->inventory().ActiveItem());

	if (Wpn && !(Wpn->GetState() == CWeapon::eIdle))
		return;

	m_bActivated = true;

	int anim_timer = READ_IF_EXISTS(pSettings, r_u32, anim_sect, "anim_timing", 0);

	g_block_all_except_movement = true;
	g_actor_allow_ladder = false;

	LPCSTR use_cam_effector = READ_IF_EXISTS(pSettings, r_string, anim_sect, !Wpn ? "anim_camera_effector" : "anim_camera_effector_weapon", nullptr);
	float effector_intensity = READ_IF_EXISTS(pSettings, r_float, anim_sect, "cam_effector_intensity", 1.0f);
	float anim_speed = READ_IF_EXISTS(pSettings, r_float, anim_sect, "anim_speed", 1.0f);

	if (pSettings->line_exist(anim_sect, "anm_use"))
	{
		g_player_hud->script_anim_play(!Actor()->inventory().GetActiveSlot() ? 2 : 1, anim_sect, !Wpn ? "anm_use" : "anm_use_weapon", true, anim_speed);
		
		if (use_cam_effector)
			g_player_hud->PlayBlendAnm(use_cam_effector, 0, anim_speed, effector_intensity, false);
		
		m_iAnimLength = Device.dwTimeGlobal + g_player_hud->motion_length_script(anim_sect, !Wpn ? "anm_use" : "anm_use_weapon", anim_speed);
	}

	if (pSettings->line_exist(anim_sect, "snd_using"))
	{
		if (m_action_anim_sound._feedback())
			m_action_anim_sound.stop();

		shared_str snd_name = pSettings->r_string(anim_sect, "snd_using");
		m_action_anim_sound.create(snd_name.c_str(), st_Effect, sg_SourceType);
		m_action_anim_sound.play(NULL, sm_2D);
	}

	m_iActionTiming = Device.dwTimeGlobal + anim_timer;

	m_bSwitched = false;
	Actor()->m_bActionAnimInProcess = true;
}

void CTorch::UpdateUseAnim()
{
	if (OnClient())
		return;

	bool IsActorAlive = g_pGamePersistent->GetActorAliveStatus();
	bool bActive = !m_switched_on;

	if ((m_iActionTiming <= Device.dwTimeGlobal && !m_bSwitched) && IsActorAlive)
	{
		m_iActionTiming = Device.dwTimeGlobal;
		Switch(bActive);
		m_bSwitched = true;
	}

	if (m_bActivated)
	{
		if ((m_iAnimLength <= Device.dwTimeGlobal) || !IsActorAlive)
		{
			m_iAnimLength = Device.dwTimeGlobal;
			m_iActionTiming = Device.dwTimeGlobal;
			m_action_anim_sound.stop();
			g_block_all_except_movement = false;
			g_actor_allow_ladder = true;
			Actor()->m_bActionAnimInProcess = false;
			m_bActivated = false;
		}
	}
}

void CTorch::Switch(bool light_on)
{
	CActor* pActor = smart_cast<CActor*>(H_Parent());
	if (pActor)
	{
		if (light_on && !m_switched_on)
		{
			if (m_sounds.FindSoundItem("sndTurnOn", false))
				m_sounds.PlaySound("sndTurnOn", pActor->Position(), NULL, !!pActor->HUDview());
		}
		else if (!light_on && m_switched_on)
		{
			if (m_sounds.FindSoundItem("sndTurnOff", false))
				m_sounds.PlaySound("sndTurnOff", pActor->Position(), NULL, !!pActor->HUDview());
		}
	}

	if (light_on && m_fCurrentChargeLevel <= 0.0f)
	{
		light_on = false;
	}

	m_switched_on			= light_on;
	if (can_use_dynamic_lights())
	{
		light_render->set_active(light_on);
		
		// CActor *pA = smart_cast<CActor *>(H_Parent());
		//if(!pA)
			light_omni->set_active(light_on);
	}
	glow_render->set_active					(light_on);

	if (*light_trace_bone) 
	{
		IKinematics* pVisual				= smart_cast<IKinematics*>(Visual()); VERIFY(pVisual);
		u16 bi								= pVisual->LL_BoneID(light_trace_bone);

		pVisual->LL_SetBoneVisible			(bi,	light_on,	TRUE);
		pVisual->CalculateBones				(TRUE);
	}
}
bool CTorch::torch_active					() const
{
	return (m_switched_on);
}

BOOL CTorch::net_Spawn(CSE_Abstract* DC) 
{
	CSE_Abstract			*e	= (CSE_Abstract*)(DC);
	CSE_ALifeItemTorch		*torch	= smart_cast<CSE_ALifeItemTorch*>(e);
	R_ASSERT				(torch);
	cNameVisual_set			(torch->get_visual());

	R_ASSERT				(!CFORM());
	R_ASSERT				(smart_cast<IKinematics*>(Visual()));
	collidable.model		= xr_new<CCF_Skeleton>	(this);

	if (!inherited::net_Spawn(DC))
		return				(FALSE);
	
	bool b_r2				= !!psDeviceFlags.test(rsR2);
	b_r2					|= !!psDeviceFlags.test(rsR4);

	IKinematics* K			= smart_cast<IKinematics*>(Visual());
	CInifile* pUserData		= K->LL_UserData(); 
	R_ASSERT3				(pUserData,"Empty Torch user data!",torch->get_visual());

	R_ASSERT2(pUserData->section_exist(m_light_section), "Section not found in torch user data! Check 'light_section' field in config");

	lanim = LALib.FindItem(pUserData->r_string(m_light_section, "color_animator"));
	guid_bone = K->LL_BoneID(pUserData->r_string(m_light_section, "guide_bone"));	VERIFY(guid_bone != BI_NONE);

	Fcolor clr = pUserData->r_fcolor(m_light_section, (b_r2) ? "color_r2" : "color");
	fBrightness				= clr.intensity();

	m_fMaxRange = pUserData->r_float(m_light_section, (b_r2) ? "max_range_r2" : "max_range");
	m_fCurveRange = pUserData->r_float(m_light_section, "curve_range");

	float range				= pUserData->r_float(m_light_section, (b_r2) ? "range_r2" : "range");
	light_render->set_color(clr);
	light_render->set_range(m_fMaxRange);

	Fcolor clr_o			= pUserData->r_fcolor(m_light_section, (b_r2) ? "omni_color_r2" : "omni_color");
	float range_o			= pUserData->r_float(m_light_section, (b_r2) ? "omni_range_r2" : "omni_range");
	light_omni->set_color	(clr_o);
	light_omni->set_range	(range_o);

	light_render->set_cone(deg2rad(pUserData->r_float(m_light_section, "spot_angle")));
	light_render->set_texture(READ_IF_EXISTS(pUserData, r_string, m_light_section, "spot_texture", (0)));

	glow_render->set_texture(pUserData->r_string(m_light_section, "glow_texture"));
	glow_render->set_color	(clr);
	glow_render->set_radius(pUserData->r_float(m_light_section, "glow_radius"));

	if (g_actor && e->ID_Parent == g_actor->ID())
		light_render->set_volumetric(!!READ_IF_EXISTS(pUserData, r_bool, m_light_section, "volumetric_for_actor", 0));
	else
		light_render->set_volumetric(!!READ_IF_EXISTS(pUserData, r_bool, m_light_section, "volumetric", 0));
	light_render->set_volumetric_quality(READ_IF_EXISTS(pUserData, r_float, m_light_section, "volumetric_quality", 1.f));
	light_render->set_volumetric_intensity(READ_IF_EXISTS(pUserData, r_float, m_light_section, "volumetric_intensity", 1.f));
	light_render->set_volumetric_distance(READ_IF_EXISTS(pUserData, r_float, m_light_section, "volumetric_distance", 1.f));

	light_render->set_type((IRender_Light::LT)(READ_IF_EXISTS(pUserData, r_u8, m_light_section, "type", 2)));
	light_omni->set_type((IRender_Light::LT)(READ_IF_EXISTS(pUserData, r_u8, m_light_section, "omni_type", 1)));

	//включить/выключить фонарик
	Switch					(torch->m_active);
	VERIFY					(!torch->m_active || (torch->ID_Parent != 0xffff));

	m_delta_h				= PI_DIV_2-atan((m_fMaxRange*0.5f)/_abs(m_torch_offset.x));

	return					(TRUE);
}

void CTorch::net_Destroy() 
{
	Switch					(false);

	inherited::net_Destroy	();
}

void CTorch::OnH_A_Chield() 
{
	inherited::OnH_A_Chield			();
	m_focus.set						(Position());
}

void CTorch::OnH_B_Independent(bool just_before_destroy) 
{
	inherited::OnH_B_Independent(just_before_destroy);

	Switch						(false);
}

void CTorch::UpdateChargeLevel(void)
{
	if (GameConstants::GetTorchHasBattery())
	{
		float uncharge_coef = (m_fUnchargeSpeed / 16) * Device.fTimeDelta;

		m_fCurrentChargeLevel -= uncharge_coef;

		float condition = 1.f * m_fCurrentChargeLevel;
		SetCondition(condition);

		float rangeCoef = atan(m_fCurveRange * m_fCurrentChargeLevel) / PI_DIV_2;
		clamp(rangeCoef, 0.f, 1.f);
		float range = m_fMaxRange * rangeCoef;

		light_render->set_range(range);
		m_delta_h = PI_DIV_2 - atan((range*0.5f) / _abs(TORCH_OFFSET.x));

		if (m_fCurrentChargeLevel < 0.0)
		{
			m_fCurrentChargeLevel = 0.0;
			Switch(false);
			return;
		}
	}
	else
		SetCondition(m_fCurrentChargeLevel);
}

void CTorch::UpdateCL() 
{
	inherited::UpdateCL			();

	if (processSwitchNeeded.load())
	{
		ProcessSwitch();
		processSwitchNeeded.store(false);
	}

	if (Actor()->m_bActionAnimInProcess && m_bActivated)
		UpdateUseAnim();

	if (!m_switched_on)			return;

	UpdateChargeLevel();

	CBoneInstance			&BI = smart_cast<IKinematics*>(Visual())->LL_GetBoneInstance(guid_bone);
	Fmatrix					M;

	if (H_Parent()) 
	{
		CActor*			actor = smart_cast<CActor*>(H_Parent());
		if (actor)		smart_cast<IKinematics*>(H_Parent()->Visual())->CalculateBones_Invalidate	();

		if (H_Parent()->XFORM().c.distance_to_sqr(Device.vCameraPosition)<_sqr(OPTIMIZATION_DISTANCE) || GameID() != eGameIDSingle) {
			// near camera
			smart_cast<IKinematics*>(H_Parent()->Visual())->CalculateBones	();
			M.mul_43				(XFORM(),BI.mTransform);
		} else {
			// approximately the same
			M		= H_Parent()->XFORM		();
			H_Parent()->Center				(M.c);
			M.c.y	+= H_Parent()->Radius	()*2.f/3.f;
		}

		if (actor) 
		{
			if (actor->active_cam() == eacLookAt)
			{
				m_prev_hp.x = angle_inertion_var(m_prev_hp.x, -actor->cam_Active()->yaw, m_torch_inertion_speed_min, m_torch_inertion_speed_max, TORCH_INERTION_CLAMP, Device.fTimeDelta);
				m_prev_hp.y = angle_inertion_var(m_prev_hp.y, -actor->cam_Active()->pitch, m_torch_inertion_speed_min, m_torch_inertion_speed_max, TORCH_INERTION_CLAMP, Device.fTimeDelta);
			}
			else {
				m_prev_hp.x = angle_inertion_var(m_prev_hp.x, -actor->cam_FirstEye()->yaw, m_torch_inertion_speed_min, m_torch_inertion_speed_max, TORCH_INERTION_CLAMP, Device.fTimeDelta);
				m_prev_hp.y = angle_inertion_var(m_prev_hp.y, -actor->cam_FirstEye()->pitch, m_torch_inertion_speed_min, m_torch_inertion_speed_max, TORCH_INERTION_CLAMP, Device.fTimeDelta);
			}

			Fvector			dir,right,up;	
			dir.setHP		(m_prev_hp.x+m_delta_h,m_prev_hp.y);
			Fvector::generate_orthonormal_basis_normalized(dir,up,right);


			if (true)
			{
				Fvector offset				= M.c; 
				offset.mad					(M.i, m_torch_offset.x);
				offset.mad					(M.j, m_torch_offset.y);
				offset.mad					(M.k, m_torch_offset.z);
				light_render->set_position	(offset);

				if(true /*false*/)
				{
					offset						= M.c; 
					offset.mad					(M.i, m_omni_offset.x);
					offset.mad					(M.j, m_omni_offset.y);
					offset.mad					(M.k, m_omni_offset.z);
					light_omni->set_position	(offset);
				}
			}//if (true)
			glow_render->set_position	(M.c);

			if (true)
			{
				light_render->set_rotation	(dir, right);
				
				if(true /*false*/)
				{
					light_omni->set_rotation	(dir, right);
				}
			}//if (true)
			glow_render->set_direction	(dir);

		}// if(actor)
		else 
		{
			if (can_use_dynamic_lights()) 
			{
				light_render->set_position	(M.c);
				light_render->set_rotation	(M.k,M.i);

				Fvector offset				= M.c; 
				offset.mad					(M.i,OMNI_OFFSET.x);
				offset.mad					(M.j,OMNI_OFFSET.y);
				offset.mad					(M.k,OMNI_OFFSET.z);
				light_omni->set_position	(M.c);
				light_omni->set_rotation	(M.k,M.i);
			}//if (can_use_dynamic_lights()) 

			glow_render->set_position	(M.c);
			glow_render->set_direction	(M.k);
		}
	}//if(HParent())
	else 
	{
		if (getVisible() && m_pPhysicsShell) 
		{
			M.mul						(XFORM(),BI.mTransform);

			m_switched_on			= false;
			light_render->set_active(false);
			light_omni->set_active(false);
			glow_render->set_active	(false);
		}//if (getVisible() && m_pPhysicsShell)  
	}

	if (!m_switched_on)					return;

	// calc color animator
	if (!lanim)							return;

	int						frame;
	// возвращает в формате BGR
	u32 clr					= lanim->CalculateBGR(Device.fTimeGlobal,frame); 

	Fcolor					fclr;
	fclr.set				((float)color_get_B(clr),(float)color_get_G(clr),(float)color_get_R(clr),1.f);
	fclr.mul_rgb			(fBrightness/255.f);
	if (can_use_dynamic_lights())
	{
		light_render->set_color	(fclr);
		light_omni->set_color	(fclr);
	}
	glow_render->set_color		(fclr);
}


void CTorch::create_physic_shell()
{
	CPhysicsShellHolder::create_physic_shell();
}

void CTorch::activate_physic_shell()
{
	CPhysicsShellHolder::activate_physic_shell();
}

void CTorch::setup_physic_shell	()
{
	CPhysicsShellHolder::setup_physic_shell();
}

void CTorch::net_Export			(NET_Packet& P)
{
	inherited::net_Export		(P);
//	P.w_u8						(m_switched_on ? 1 : 0);


	BYTE F = 0;
	F |= (m_switched_on ? eTorchActive : 0);
	const CActor *pA = smart_cast<const CActor *>(H_Parent());
	if (pA)
	{
		if (pA->attached(this))
			F |= eAttached;
	}
	P.w_u8(F);
//	Msg("CTorch::net_export - NV[%d]", m_bNightVisionOn);
}

void CTorch::net_Import			(NET_Packet& P)
{
	inherited::net_Import		(P);
	
	BYTE F = P.r_u8();
	bool new_m_switched_on				= !!(F & eTorchActive);

	if (new_m_switched_on != m_switched_on)			Switch						(new_m_switched_on);
}

void CTorch::save(NET_Packet &output_packet)
{
	inherited::save(output_packet);
	save_data(m_fCurrentChargeLevel, output_packet);

}

void CTorch::load(IReader &input_packet)
{
	inherited::load(input_packet);
	load_data(m_fCurrentChargeLevel, input_packet);
}

bool  CTorch::can_be_attached		() const
{
	const CActor *pA = smart_cast<const CActor *>(H_Parent());
	if (pA)
	{
		return (this == smart_cast<CTorch*>(pA->inventory().ItemFromSlot(TORCH_SLOT)));
	}
	return true;
}

void CTorch::afterDetach			()
{
	inherited::afterDetach	();
	Switch					(false);
}
void CTorch::renderable_Render()
{
	inherited::renderable_Render();
}

void CTorch::enable(bool value)
{
	inherited::enable(value);

	if(!enabled() && m_switched_on)
		Switch				(false);

}

float CTorch::GetUnchargeSpeed() const
{
	return m_fUnchargeSpeed;
}

float CTorch::GetCurrentChargeLevel() const
{
	return m_fCurrentChargeLevel;
}

bool CTorch::IsSwitchedOn() const
{
	return m_switched_on;
}

void CTorch::SetCurrentChargeLevel(float val)
{
	m_fCurrentChargeLevel = val;
	clamp(m_fCurrentChargeLevel, 0.f, m_fMaxChargeLevel);

	float condition = 1.f * m_fCurrentChargeLevel / m_fUnchargeSpeed;
	SetCondition(condition);
}

void CTorch::Recharge(float val)
{
	m_fCurrentChargeLevel += val;
	clamp(m_fCurrentChargeLevel, 0.f, m_fMaxChargeLevel);

	SetCondition(m_fCurrentChargeLevel);
}

float CTorch::get_range() const 
{
	return light_render->get_range();
}

bool CTorch::install_upgrade_impl(LPCSTR section, bool test)
{
	bool result = inherited::install_upgrade_impl(section, test);

	result |= process_if_exists(section, "uncharge_speed",	&CInifile::r_float, m_fUnchargeSpeed, test);
	result |= process_if_exists(section, "inv_weight", &CInifile::r_float, m_weight, test);

	return result;
}

bool CTorch::IsNecessaryItem(const shared_str& item_sect, xr_vector<shared_str> item)
{
	return (std::find(item.begin(), item.end(), item_sect) != item.end());
}