#include "stdafx.h"
#include "torch.h"
#include "entity.h"
#include "actor.h"
#include "../xrEngine/LightAnimLibrary.h"
#include "PhysicsShell.h"
#include "xrserver_objects_alife_items.h"
#include "ai_sounds.h"

#include "HUDManager.h"
#include "level.h"
#include "../Include/xrRender/Kinematics.h"
#include "../xrEngine/camerabase.h"
#include "inventory.h"
#include "game_base_space.h"

#include "UIGameCustom.h"
#include "actorEffector.h"
#include "CustomOutfit.h"
#include "..\XrEngine\xr_collide_form.h"

#include "AdvancedXrayGameConstants.h"
#include "Battery.h"

static const float		TIME_2_HIDE					= 5.f;
static const float		TORCH_INERTION_CLAMP		= PI_DIV_6;
static const float		TORCH_INERTION_SPEED_MAX	= 7.5f;
static const float		TORCH_INERTION_SPEED_MIN	= 0.5f;
static const Fvector	TORCH_OFFSET				= {-0.2f,+0.1f,-0.3f};
static const Fvector	OMNI_OFFSET					= {-0.2f,+0.1f,-0.1f};
static const float		OPTIMIZATION_DISTANCE		= 100.f;

static bool stalker_use_dynamic_lights	= false;

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
	time2hide					= 0;
	fBrightness					= 1.f;

	/*m_NightVisionRechargeTime	= 6.f;
	m_NightVisionRechargeTimeMin= 2.f;
	m_NightVisionDischargeTime	= 10.f;
	m_NightVisionChargeTime		= 0.f;*/

	m_prev_hp.set				(0,0);
	m_delta_h					= 0;

	m_fMaxChargeLevel			= 0.0f;
	m_fCurrentChargeLevel		= 1.0f;
	m_fUnchargeSpeed			= 0.0f;
	m_fMaxRange					= 20.f;
	m_fCurveRange				= 20.f;
}

CTorch::~CTorch(void) 
{
	light_render.destroy	();
	light_omni.destroy		();
	glow_render.destroy		();
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
		m_sounds.LoadSound(section, "snd_turn_on", "sndTurnOn", SOUND_TYPE_ITEM_USING);
	if (pSettings->line_exist(section, "snd_turn_off"))
		m_sounds.LoadSound(section, "snd_turn_off", "sndTurnOff", SOUND_TYPE_ITEM_USING);

	m_bNightVisionEnabled = !!pSettings->r_bool(section,"night_vision");

	if(m_bNightVisionEnabled)
	{
		m_sounds.LoadSound(section,"snd_night_vision_on", "NightVisionOnSnd", SOUND_TYPE_ITEM_USING);
		m_sounds.LoadSound(section,"snd_night_vision_off", "NightVisionOffSnd", SOUND_TYPE_ITEM_USING);
		m_sounds.LoadSound(section,"snd_night_vision_idle", "NightVisionIdleSnd", SOUND_TYPE_ITEM_USING);
		m_sounds.LoadSound(section,"snd_night_vision_broken", "NightVisionBrokenSnd", SOUND_TYPE_ITEM_USING);
	}

	//Случайный начальный заряд батареек в фонарике, если включена опция ограниченного заряда батареек у фонарика
	if (GameConstants::GetTorchHasBattery())
	{
		float rnd_charge = ::Random.randF(0.0f, m_fMaxChargeLevel);
		m_fCurrentChargeLevel = rnd_charge;
	}
}

void CTorch::SwitchNightVision()
{
	if (OnClient()) return;
	SwitchNightVision(!m_bNightVisionOn);	
}

void CTorch::SwitchNightVision(bool vision_on)
{
	if(!m_bNightVisionEnabled) return;
	
	if(vision_on /*&& (m_NightVisionChargeTime > m_NightVisionRechargeTimeMin || OnClient())*/)
	{
		//m_NightVisionChargeTime = m_NightVisionDischargeTime*m_NightVisionChargeTime/m_NightVisionRechargeTime;
		m_bNightVisionOn = true;
	}
	else
	{
		m_bNightVisionOn = false;
	}

	CActor *pA = smart_cast<CActor *>(H_Parent());

	if(!pA)					return;
	bool bPlaySoundFirstPerson = (pA == Level().CurrentViewEntity());

	LPCSTR disabled_names	= pSettings->r_string(cNameSect(),"disabled_maps");
	LPCSTR curr_map			= *Level().name();
	u32 cnt					= _GetItemCount(disabled_names);
	bool b_allow			= true;
	string512				tmp;
	for(u32 i=0; i<cnt;++i){
		_GetItem(disabled_names, i, tmp);
		if(0==stricmp(tmp, curr_map)){
			b_allow = false;
			break;
		}
	}

	CCustomOutfit* pCO=pA->GetOutfit();
	if(pCO&&pCO->m_NightVisionSect.size()&&!b_allow){
		m_sounds.PlaySound("NightVisionBrokenSnd", pA->Position(), pA, bPlaySoundFirstPerson);
		return;
	}

	if(m_bNightVisionOn){
		CEffectorPP* pp = pA->Cameras().GetPPEffector((EEffectorPPType)effNightvision);
		if(!pp){
			if (pCO&&pCO->m_NightVisionSect.size())
			{
				AddEffector(pA,effNightvision, pCO->m_NightVisionSect);
				m_sounds.PlaySound("NightVisionOnSnd", pA->Position(), pA, bPlaySoundFirstPerson);
				m_sounds.PlaySound("NightVisionIdleSnd", pA->Position(), pA, bPlaySoundFirstPerson, true);
			}
		}
	}else{
 		CEffectorPP* pp = pA->Cameras().GetPPEffector((EEffectorPPType)effNightvision);
		if(pp){
			pp->Stop			(1.0f);
			m_sounds.PlaySound("NightVisionOffSnd", pA->Position(), pA, bPlaySoundFirstPerson);
			m_sounds.StopSound("NightVisionIdleSnd");
		}
	}
}


void CTorch::UpdateSwitchNightVision   ()
{
	if(!m_bNightVisionEnabled) return;
	if (OnClient()) return;


	/*if(m_bNightVisionOn)
	{
		m_NightVisionChargeTime			-= Device.fTimeDelta;

		if(m_NightVisionChargeTime<0.f)
			SwitchNightVision(false);
	}
	else
	{
		m_NightVisionChargeTime			+= Device.fTimeDelta;
		clamp(m_NightVisionChargeTime, 0.f, m_NightVisionRechargeTime);
	}*/
}


void CTorch::Switch()
{
	if (OnClient()) return;
	bool bActive			= !m_switched_on;
	Switch					(bActive);
}

void CTorch::Switch	(bool light_on)
{
	CActor* pActor = smart_cast<CActor*>(H_Parent());
	if (pActor)
	{
		if (light_on && !m_switched_on)
		{
			if (m_sounds.FindSoundItem("SndTurnOn", false))
				m_sounds.PlaySound("SndTurnOn", pActor->Position(), NULL, !!pActor->HUDview());
		}
		else if (!light_on && m_switched_on)
		{
			if (m_sounds.FindSoundItem("SndTurnOff", false))
				m_sounds.PlaySound("SndTurnOff", pActor->Position(), NULL, !!pActor->HUDview());
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
		
		CActor *pA = smart_cast<CActor *>(H_Parent());
		if(!pA)light_omni->set_active(light_on);
	}
	glow_render->set_active					(light_on);

	if (*light_trace_bone) 
	{
		IKinematics* pVisual				= smart_cast<IKinematics*>(Visual()); VERIFY(pVisual);
		u16 bi								= pVisual->LL_BoneID(light_trace_bone);

		pVisual->LL_SetBoneVisible			(bi,	light_on,	TRUE);
		pVisual->CalculateBones				(TRUE);
//.		pVisual->LL_SetBoneVisible			(bi,	light_on,	TRUE); //hack
	}
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
	b_r2					|= !!psDeviceFlags.test(rsR3);
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

	float range = pUserData->r_float(m_light_section, (b_r2) ? "range_r2" : "range");
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

	CActor* pActor = smart_cast<CActor*>(H_Parent());
	if (pActor)
		light_render->set_volumetric(!!READ_IF_EXISTS(pUserData, r_bool, m_light_section, "volumetric_for_actor", 0));
	else
	light_render->set_volumetric(!!READ_IF_EXISTS(pUserData, r_bool, m_light_section, "volumetric", 0));
	light_render->set_volumetric_quality(READ_IF_EXISTS(pUserData, r_float, m_light_section, "volumetric_quality", 1.f));
	light_render->set_volumetric_intensity(READ_IF_EXISTS(pUserData, r_float, m_light_section, "volumetric_intensity", 1.f));
	light_render->set_volumetric_distance(READ_IF_EXISTS(pUserData, r_float, m_light_section, "volumetric_distance", 1.f));

	//включить/выключить фонарик
	Switch					(torch->m_active);
	VERIFY					(!torch->m_active || (torch->ID_Parent != 0xffff));
	
	SwitchNightVision		(false);

	m_delta_h				= PI_DIV_2-atan((m_fMaxRange*0.5f)/_abs(TORCH_OFFSET.x));

	return					(TRUE);
}

void CTorch::net_Destroy() 
{
	Switch					(false);
	SwitchNightVision		(false);

	inherited::net_Destroy	();
}

void CTorch::OnH_A_Chield() 
{
	inherited::OnH_A_Chield			();
	m_focus.set						(Position());
}

void CTorch::OnH_B_Independent	(bool just_before_destroy) 
{
	inherited::OnH_B_Independent	(just_before_destroy);
	time2hide						= TIME_2_HIDE;

	Switch						(false);
	SwitchNightVision			(false);

	m_sounds.StopAllSounds		();
}

void CTorch::UpdateChargeLevel(void)
{
	if (m_switched_on)
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

		//Msg("Torch Charge is: %f", m_fCurrentChargeLevel); //Для тестов

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

	if (GameConstants::GetTorchHasBattery)
		UpdateChargeLevel();
	
	UpdateSwitchNightVision		();

	if (!m_switched_on)			return;

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
				m_prev_hp.x = angle_inertion_var(m_prev_hp.x, -actor->cam_Active()->yaw, TORCH_INERTION_SPEED_MIN, TORCH_INERTION_SPEED_MAX, TORCH_INERTION_CLAMP, Device.fTimeDelta);
				m_prev_hp.y = angle_inertion_var(m_prev_hp.y, -actor->cam_Active()->pitch, TORCH_INERTION_SPEED_MIN, TORCH_INERTION_SPEED_MAX, TORCH_INERTION_CLAMP, Device.fTimeDelta);
			}
			else {
				m_prev_hp.x = angle_inertion_var(m_prev_hp.x, -actor->cam_FirstEye()->yaw, TORCH_INERTION_SPEED_MIN, TORCH_INERTION_SPEED_MAX, TORCH_INERTION_CLAMP, Device.fTimeDelta);
				m_prev_hp.y = angle_inertion_var(m_prev_hp.y, -actor->cam_FirstEye()->pitch, TORCH_INERTION_SPEED_MIN, TORCH_INERTION_SPEED_MAX, TORCH_INERTION_CLAMP, Device.fTimeDelta);
			}

			Fvector			dir,right,up;	
			dir.setHP		(m_prev_hp.x+m_delta_h,m_prev_hp.y);
			Fvector::generate_orthonormal_basis_normalized(dir,up,right);


			if (true)
			{
				Fvector offset				= M.c; 
				offset.mad					(M.i,TORCH_OFFSET.x);
				offset.mad					(M.j,TORCH_OFFSET.y);
				offset.mad					(M.k,TORCH_OFFSET.z);
				light_render->set_position	(offset);

				if(false)
				{
					offset						= M.c; 
					offset.mad					(M.i,OMNI_OFFSET.x);
					offset.mad					(M.j,OMNI_OFFSET.y);
					offset.mad					(M.k,OMNI_OFFSET.z);
					light_omni->set_position	(offset);
				}
			}//if (true)
			glow_render->set_position	(M.c);

			if (true)
			{
				light_render->set_rotation	(dir, right);
				
				if(false)
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

			//. what should we do in case when 
			// light_render is not active at this moment,
			// but m_switched_on is true?
//			light_render->set_rotation	(M.k,M.i);
//			light_render->set_position	(M.c);
//			glow_render->set_position	(M.c);
//			glow_render->set_direction	(M.k);
//
//			time2hide					-= Device.fTimeDelta;
//			if (time2hide<0)
			{
				m_switched_on			= false;
				light_render->set_active(false);
				light_omni->set_active(false);
				glow_render->set_active	(false);
			}
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
	F |= (m_bNightVisionOn ? eNightVisionActive : 0);
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
	bool new_m_bNightVisionOn			= !!(F & eNightVisionActive);

	if (new_m_switched_on != m_switched_on)			Switch						(new_m_switched_on);
	if (new_m_bNightVisionOn != m_bNightVisionOn)	
	{
//		Msg("CTorch::net_Import - NV[%d]", new_m_bNightVisionOn);

		SwitchNightVision			(new_m_bNightVisionOn);
	}
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
		//return pA->inventory().InSlot(this);
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
	float condition = 1.f * m_fCurrentChargeLevel / m_fUnchargeSpeed;
	SetCondition(condition);
}

void CTorch::Recharge(float val)
{
	m_fCurrentChargeLevel = m_fCurrentChargeLevel + val;

	SetCondition(m_fCurrentChargeLevel);

	//Msg("Battery Charge in Recharge is: %f", val); //Для тестов

	if (m_fCurrentChargeLevel > m_fMaxChargeLevel)
		m_fCurrentChargeLevel = m_fMaxChargeLevel;
}
