#include "stdafx.h"
#include "artefact.h"
#include "../xrphysics/PhysicsShell.h"
#include "PhysicsShellHolder.h"
#include "game_cl_base.h"

#include "../Include/xrRender/Kinematics.h"
#include "../Include/xrRender/KinematicsAnimated.h"

#include "inventory.h"
#include "level.h"
#include "ai_object_location.h"
#include "xrServer_Objects_ALife_Monsters.h"
#include "../xrphysics/iphworld.h"
#include "restriction_space.h"
#include "../xrEngine/IGame_Persistent.h"

#include "artefact_activation.h"

#include "ai_space.h"
#include "patrol_path.h"
#include "patrol_path_storage.h"
#include "Actor.h"
#include "ArtefactContainer.h"
#include "AdvancedXrayGameConstants.h"

#define	FASTMODE_DISTANCE (50.f)	//distance to camera from sphere, when zone switches to fast update sequence

#define CHOOSE_MAX(x,inst_x,y,inst_y,z,inst_z)\
	if(x>y)\
		if(x>z){inst_x;}\
		else{inst_z;}\
	else\
		if(y>z){inst_y;}\
		else{inst_z;}

extern float af_from_container_charge_level;
extern int af_from_container_rank;
extern CArtefactContainer* m_LastAfContainer;

CArtefact::CArtefact() 
{
	shedule.t_min				= 20;
	shedule.t_max				= 50;
	m_sParticlesName			= NULL;
	m_pTrailLight				= NULL;
	m_activationObj				= NULL;
	m_detectorObj				= NULL;
	m_additional_weight			= 0.0f;

	m_bVolumetricLights			= false;
	m_fVolumetricQuality		= 1.0f;
	m_fVolumetricDistance		= 0.3f;
	m_fVolumetricIntensity		= 0.5f;

	m_fChargeLevel				= 1.0f;
	m_fDegradationSpeed			= 1.0f;
	m_iAfRank					= 1;

	//For Degradation
	m_fConstAdditionalWeight	= 0.0f;

	m_bInContainer				= false;
}


CArtefact::~CArtefact() 
{}

void CArtefact::Load(LPCSTR section) 
{
	inherited::Load			(section);


	if (pSettings->line_exist(section, "particles"))
		m_sParticlesName	= pSettings->r_string(section, "particles");

	m_bLightsEnabled		= !!pSettings->r_bool(section, "lights_enabled");
	if(m_bLightsEnabled){
		sscanf(pSettings->r_string(section,"trail_light_color"), "%f,%f,%f", 
			&m_TrailLightColor.r, &m_TrailLightColor.g, &m_TrailLightColor.b);
		m_fConstTrailLightRange = pSettings->r_float(section,"trail_light_range");
	}

	//Случайный начальный ранг артефакта
	if (GameConstants::GetAfRanks())
	{
		int rnd_rank = ::Random.randI(1, 100);

		if (rnd_rank <= 50) //50%
			m_iAfRank = 1;
		else if (rnd_rank > 50 && rnd_rank <= 75) //25%
			m_iAfRank = 2;
		else if (rnd_rank > 75 && rnd_rank <= 90) //15%
			m_iAfRank = 3;
		else if (rnd_rank > 90 && rnd_rank <= 98) //7%
			m_iAfRank = 4;
		else if (rnd_rank > 98) //2%
			m_iAfRank = 5;
	}

	m_fConstInfectionRestoreSpeed		= READ_IF_EXISTS(pSettings, r_float, section, "infection_restore_speed",		0.f) * m_iAfRank;
	m_fConstHealthRestoreSpeed			= READ_IF_EXISTS(pSettings, r_float, section, "health_restore_speed",			0.f) * m_iAfRank;
	m_fConstRadiationRestoreSpeed		= READ_IF_EXISTS(pSettings, r_float, section, "radiation_restore_speed",		0.f);
	m_fConstSatietyRestoreSpeed			= READ_IF_EXISTS(pSettings, r_float, section, "satiety_restore_speed",			0.f) * m_iAfRank;
	m_fConstPowerRestoreSpeed			= READ_IF_EXISTS(pSettings, r_float, section, "power_restore_speed",			0.f) * m_iAfRank;
	m_fConstBleedingRestoreSpeed		= READ_IF_EXISTS(pSettings, r_float, section, "bleeding_restore_speed",			0.f) * m_iAfRank;
	m_fConstThirstRestoreSpeed			= READ_IF_EXISTS(pSettings, r_float, section, "thirst_restore_speed",			0.f) * m_iAfRank;
	m_fConstIntoxicationRestoreSpeed	= READ_IF_EXISTS(pSettings, r_float, section, "intoxication_restore_speed",		0.f) * m_iAfRank;
	m_fConstSleepenessRestoreSpeed		= READ_IF_EXISTS(pSettings, r_float, section, "sleepeness_restore_speed",		0.f) * m_iAfRank;
	m_fConstAlcoholismRestoreSpeed		= READ_IF_EXISTS(pSettings, r_float, section, "alcoholism_restore_speed",		0.f) * m_iAfRank;
	m_fConstNarcotismRestoreSpeed		= READ_IF_EXISTS(pSettings, r_float, section, "narcotism_restore_speed",		0.f) * m_iAfRank;
	m_fConstPsyHealthRestoreSpeed		= READ_IF_EXISTS(pSettings, r_float, section, "psy_health_restore_speed",		0.f) * m_iAfRank;
	m_fConstFrostbiteRestoreSpeed		= READ_IF_EXISTS(pSettings, r_float, section, "frostbite_restore_speed",		0.f) * m_iAfRank;
	m_fConstAdditionalWeight			= READ_IF_EXISTS(pSettings, r_float, section, "additional_inventory_weight",	0.f) * m_iAfRank;

	m_fChargeLevel = READ_IF_EXISTS(pSettings, r_float, section, "artefact_charge_level", 1.0f);

	m_bVolumetricLights = READ_IF_EXISTS(pSettings, r_bool, section, "volumetric_lights", false);
	m_fVolumetricQuality = READ_IF_EXISTS(pSettings, r_float, section, "volumetric_quality", 1.0f);
	m_fConstVolumetricDistance = READ_IF_EXISTS(pSettings, r_float, section, "volumetric_distance", 0.3f);
	m_fConstVolumetricIntensity = READ_IF_EXISTS(pSettings, r_float, section, "volumetric_intensity", 0.5f);

	m_fConstJumpSpeed = READ_IF_EXISTS(pSettings, r_float, section, "jump_speed", 1.f);
	m_fConstWalkAccel = READ_IF_EXISTS(pSettings, r_float, section, "walk_accel", 1.f);

	m_fTrailLightRange = m_fConstTrailLightRange;

	m_fVolumetricDistance = m_fConstVolumetricDistance;
	m_fVolumetricIntensity = m_fConstVolumetricIntensity;

	m_fHealthRestoreSpeed = m_fConstHealthRestoreSpeed;
	m_fRadiationRestoreSpeed = m_fConstRadiationRestoreSpeed;
	m_fSatietyRestoreSpeed = m_fConstSatietyRestoreSpeed;
	m_fPowerRestoreSpeed = m_fConstPowerRestoreSpeed;
	m_fBleedingRestoreSpeed = m_fConstBleedingRestoreSpeed;
	m_fThirstRestoreSpeed = m_fConstThirstRestoreSpeed;
	m_fIntoxicationRestoreSpeed = m_fConstIntoxicationRestoreSpeed;
	m_fSleepenessRestoreSpeed = m_fConstSleepenessRestoreSpeed;
	m_fAlcoholismRestoreSpeed = m_fConstAlcoholismRestoreSpeed;
	m_fNarcotismRestoreSpeed = m_fConstNarcotismRestoreSpeed;
	m_fPsyHealthRestoreSpeed = m_fConstPsyHealthRestoreSpeed;
	m_fFrostbiteRestoreSpeed = m_fConstFrostbiteRestoreSpeed;
	m_fInfectionRestoreSpeed = m_fConstInfectionRestoreSpeed;
	m_additional_weight = m_fConstAdditionalWeight;
	m_fJumpSpeed = m_fConstJumpSpeed;
	m_fWalkAccel = m_fConstWalkAccel;
	
	if(pSettings->section_exist(pSettings->r_string(section,"hit_absorbation_sect")))
	{
		m_ArtefactHitImmunities.LoadImmunities(pSettings->r_string(section,"hit_absorbation_sect"),pSettings);
	}

	m_ConstHitTypeProtection[ALife::eHitTypeBurn]			= m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeBurn) * m_iAfRank;
	m_ConstHitTypeProtection[ALife::eHitTypeStrike]			= m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeStrike) * m_iAfRank;
	m_ConstHitTypeProtection[ALife::eHitTypeShock]			= m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeShock) * m_iAfRank;
	m_ConstHitTypeProtection[ALife::eHitTypeWound]			= m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeWound) * m_iAfRank;
	m_ConstHitTypeProtection[ALife::eHitTypeRadiation]		= m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeRadiation) * m_iAfRank;
	m_ConstHitTypeProtection[ALife::eHitTypeTelepatic]		= m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeTelepatic) * m_iAfRank;
	m_ConstHitTypeProtection[ALife::eHitTypeChemicalBurn]	= m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeChemicalBurn) * m_iAfRank;
	m_ConstHitTypeProtection[ALife::eHitTypeExplosion]		= m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeExplosion) * m_iAfRank;
	m_ConstHitTypeProtection[ALife::eHitTypeFireWound]		= m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeFireWound) * m_iAfRank;

	m_HitTypeProtection[ALife::eHitTypeBurn]			= m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeBurn);
	m_HitTypeProtection[ALife::eHitTypeStrike]			= m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeStrike);
	m_HitTypeProtection[ALife::eHitTypeShock]			= m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeShock);
	m_HitTypeProtection[ALife::eHitTypeWound]			= m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeWound);
	m_HitTypeProtection[ALife::eHitTypeRadiation]		= m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeRadiation);
	m_HitTypeProtection[ALife::eHitTypeTelepatic]		= m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeTelepatic);
	m_HitTypeProtection[ALife::eHitTypeChemicalBurn]	= m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeChemicalBurn);
	m_HitTypeProtection[ALife::eHitTypeExplosion]		= m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeExplosion);
	m_HitTypeProtection[ALife::eHitTypeFireWound]		= m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeFireWound);

	m_bCanSpawnZone			= !!pSettings->line_exist("artefact_spawn_zones", section);
	m_af_rank				= pSettings->r_u8(section, "af_rank");

	m_fDegradationSpeed = READ_IF_EXISTS(pSettings, r_float, section, "degradation_speed", 0.0f);
}

BOOL CArtefact::net_Spawn(CSE_Abstract* DC) 
{
	if(pSettings->r_bool(cNameSect(),"can_be_controlled") )
		m_detectorObj				= xr_new<SArtefactDetectorsSupport>(this);

	BOOL result						= inherited::net_Spawn(DC);
	SwitchAfParticles				(true);

	StartLights();
	m_CarringBoneID					= u16(-1);
	IKinematicsAnimated	*K			= smart_cast<IKinematicsAnimated*>(Visual());
	if(K)
		K->PlayCycle("idle");
	
	o_fastmode						= FALSE;		// start initially with fast-mode enabled
	o_render_frame					= 0;
	SetState						(eHidden);

	return							result;	
}

void CArtefact::net_Destroy() 
{
	inherited::net_Destroy			();

	StopLights						();
	if(m_pTrailLight)
		m_pTrailLight.destroy		();

	CPHUpdateObject::Deactivate		();
	xr_delete						(m_activationObj);
	xr_delete						(m_detectorObj);
}

void CArtefact::OnH_A_Chield() 
{
	inherited::OnH_A_Chield		();

	StopLights();
	if (IsGameTypeSingle())
	{
		SwitchAfParticles(false);
	}
	else
	{
		IKinematics* K	= smart_cast<IKinematics*>(H_Parent()->Visual());
		if (K)
			m_CarringBoneID			= K->LL_BoneID("bip01_head");
		else
			m_CarringBoneID = u16(-1);
	}
	if(m_detectorObj)
	{
		m_detectorObj->m_currPatrolPath = NULL;
		m_detectorObj->m_currPatrolVertex = NULL;
	}

	if (m_LastAfContainer) //Костыль для контейнеров, потом надо нормально как-то сделать
	{
		SetChargeLevel(af_from_container_charge_level);
		SetRank(af_from_container_rank);

		m_LastAfContainer = nullptr;
	}
}

void CArtefact::OnH_B_Independent(bool just_before_destroy) 
{
	VERIFY(!physics_world()->Processing());
	inherited::OnH_B_Independent(just_before_destroy);

	StartLights();
	SwitchAfParticles	(true);
}

void CArtefact::save(NET_Packet &packet)
{
	inherited::save(packet);

	save_data(m_fChargeLevel, packet);
	save_data(m_fVolumetricIntensity, packet);
	save_data(m_fVolumetricDistance, packet);
	save_data(m_additional_weight, packet);
	save_data(m_fHealthRestoreSpeed, packet);
	save_data(m_fRadiationRestoreSpeed, packet);
	save_data(m_fSatietyRestoreSpeed, packet);
	save_data(m_fPowerRestoreSpeed, packet);
	save_data(m_fBleedingRestoreSpeed, packet);
	save_data(m_fThirstRestoreSpeed, packet);
	save_data(m_fIntoxicationRestoreSpeed, packet);
	save_data(m_fSleepenessRestoreSpeed, packet);
	save_data(m_fAlcoholismRestoreSpeed, packet);
	save_data(m_fNarcotismRestoreSpeed, packet);
	save_data(m_fPsyHealthRestoreSpeed, packet);
	save_data(m_fFrostbiteRestoreSpeed, packet);
	save_data(m_fInfectionRestoreSpeed, packet);
	save_data(m_additional_weight, packet);
	save_data(m_fJumpSpeed, packet);
	save_data(m_fWalkAccel, packet);
	save_data(m_iAfRank, packet);
	save_data(m_bInContainer, packet);
}

void CArtefact::load(IReader &packet)
{
	inherited::load(packet);
	load_data(m_fChargeLevel, packet);
	load_data(m_fVolumetricIntensity, packet);
	load_data(m_fVolumetricDistance, packet);
	load_data(m_additional_weight, packet);
	load_data(m_fHealthRestoreSpeed, packet);
	load_data(m_fRadiationRestoreSpeed, packet);
	load_data(m_fSatietyRestoreSpeed, packet);
	load_data(m_fPowerRestoreSpeed, packet);
	load_data(m_fBleedingRestoreSpeed, packet);
	load_data(m_fThirstRestoreSpeed, packet);
	load_data(m_fIntoxicationRestoreSpeed, packet);
	load_data(m_fSleepenessRestoreSpeed, packet);
	load_data(m_fAlcoholismRestoreSpeed, packet);
	load_data(m_fNarcotismRestoreSpeed, packet);
	load_data(m_fPsyHealthRestoreSpeed, packet);
	load_data(m_fFrostbiteRestoreSpeed, packet);
	load_data(m_fInfectionRestoreSpeed, packet);
	load_data(m_additional_weight, packet);
	load_data(m_fJumpSpeed, packet);
	load_data(m_fWalkAccel, packet);
	load_data(m_iAfRank, packet);
	load_data(m_bInContainer, packet);
}

void CArtefact::SwitchAfParticles(bool bOn)
{
	if(m_sParticlesName.size()==0)
		return;

	if(bOn)
	{
			Fvector dir;
			dir.set(0,1,0);
			CParticlesPlayer::StartParticles(m_sParticlesName,dir,ID(),-1, false);
	}else
	{
			CParticlesPlayer::StopParticles(m_sParticlesName, BI_NONE, true);
	}
}

// called only in "fast-mode"
void CArtefact::UpdateCL		() 
{
	inherited::UpdateCL			();
	
	if (o_fastmode || m_activationObj)
		UpdateWorkload			(Device.dwTimeDelta);	

	if (m_fChargeLevel <= 0.5f)
	{
		m_bCanSpawnZone = false;
	}
}

void CArtefact::Interpolate()
{
	if (OnServer())
		return;

	net_updateInvData* p = NetSync();
	while (p->NET_IItem.size() > 1)	//in real no interpolation, just get latest state
	{
		p->NET_IItem.pop_front();
	}
	inherited::Interpolate();
	
	if (p->NET_IItem.size())	
	{
		p->NET_IItem.clear(); //same as p->NET_IItem.pop_front();
	}
}

void CArtefact::UpdateDegradation()
{
	float uncharge_coef = (m_fDegradationSpeed / 16) * Device.fTimeDelta;

	m_fChargeLevel -= uncharge_coef;
	clamp(m_fChargeLevel, 0.f, 1.f);

	float percent = m_fChargeLevel * 100;

	if (m_fHealthRestoreSpeed > 0.0f && m_fConstHealthRestoreSpeed > 0.0f)
		m_fHealthRestoreSpeed = (m_fConstHealthRestoreSpeed / 100) * percent;
	if (m_fRadiationRestoreSpeed < 0.0f && m_fConstRadiationRestoreSpeed < 0.0f)
		m_fRadiationRestoreSpeed = (m_fConstRadiationRestoreSpeed / 100) * percent;
	if (m_fSatietyRestoreSpeed > 0.0f && m_fConstSatietyRestoreSpeed > 0.0f)
		m_fSatietyRestoreSpeed = (m_fConstSatietyRestoreSpeed / 100) * percent;
	if (m_fPowerRestoreSpeed > 0.0f && m_fConstPowerRestoreSpeed > 0.0f)
		m_fPowerRestoreSpeed = (m_fConstPowerRestoreSpeed / 100) * percent;
	if (m_fBleedingRestoreSpeed > 0.0f && m_fConstBleedingRestoreSpeed > 0.0f)
		m_fBleedingRestoreSpeed = (m_fConstBleedingRestoreSpeed / 100) * percent;
	if (m_fThirstRestoreSpeed > 0.0f && m_fConstThirstRestoreSpeed > 0.0f)
		m_fThirstRestoreSpeed = (m_fConstThirstRestoreSpeed / 100) * percent;
	if (m_fIntoxicationRestoreSpeed > 0.0f && m_fConstIntoxicationRestoreSpeed > 0.0f)
		m_fIntoxicationRestoreSpeed = (m_fConstIntoxicationRestoreSpeed / 100) * percent;
	if (m_fSleepenessRestoreSpeed > 0.0f && m_fConstSleepenessRestoreSpeed > 0.0f)
		m_fSleepenessRestoreSpeed = (m_fConstSleepenessRestoreSpeed / 100) * percent;
	if (m_fAlcoholismRestoreSpeed > 0.0f && m_fConstAlcoholismRestoreSpeed > 0.0f)
		m_fAlcoholismRestoreSpeed = (m_fConstAlcoholismRestoreSpeed / 100) * percent;
	if (m_fNarcotismRestoreSpeed > 0.0f && m_fConstNarcotismRestoreSpeed > 0.0f)
		m_fNarcotismRestoreSpeed = (m_fConstNarcotismRestoreSpeed / 100) * percent;
	if (m_fPsyHealthRestoreSpeed > 0.0f && m_fConstPsyHealthRestoreSpeed > 0.0f)
		m_fPsyHealthRestoreSpeed = (m_fConstPsyHealthRestoreSpeed / 100) * percent;
	if (m_fFrostbiteRestoreSpeed > 0.0f && m_fConstFrostbiteRestoreSpeed > 0.0f)
		m_fFrostbiteRestoreSpeed = (m_fConstFrostbiteRestoreSpeed / 100) * percent;
	if (m_fInfectionRestoreSpeed > 0.0f && m_fConstInfectionRestoreSpeed > 0.0f)
		m_fInfectionRestoreSpeed = (m_fConstInfectionRestoreSpeed / 100) * percent;
	if (m_additional_weight > 0.0f && m_fConstAdditionalWeight > 0.0f)
		m_additional_weight = (m_fConstAdditionalWeight / 100) * percent;
	if (m_fJumpSpeed > 1.f && m_fConstJumpSpeed > 1.f)
		m_fJumpSpeed = (m_fConstJumpSpeed / 100) * percent;
	if (m_fWalkAccel > 1.f && m_fConstWalkAccel > 1.f)
		m_fWalkAccel = (m_fConstWalkAccel / 100) * percent;
	if (m_fChargeLevel <= 0.0f)
		m_iAfRank = 0;

	for (size_t i = 0; i < ALife::infl_max_count; ++i)
	{
		if (!fis_zero(m_HitTypeProtection[(ALife::EHitType)i]) && !fis_zero(m_ConstHitTypeProtection[(ALife::EHitType)i]))
			m_HitTypeProtection[(ALife::EHitType)i] = (m_ConstHitTypeProtection[(ALife::EHitType)i] / 100) * percent;
	}

	//Lights
	if (m_bLightsEnabled)
	{
		if (m_fTrailLightRange >= 0.0f)
			m_fTrailLightRange = (m_fConstTrailLightRange / 100)*percent;
		else
			m_bLightsEnabled = false;
	}

	//Volumetric Lights
	if (m_bVolumetricLights)
	{
		if (m_fVolumetricDistance >= 0.0f)
			m_fVolumetricDistance = (m_fConstVolumetricDistance / 100)*percent;
		else if (m_fVolumetricIntensity >= 0.0f)
			m_fVolumetricIntensity = (m_fConstVolumetricIntensity / 100)*percent;
		else
			m_bVolumetricLights = false;
	}
}

void CArtefact::UpdateWorkload		(u32 dt) 
{

	VERIFY(!physics_world()->Processing());
	// particles - velocity
	Fvector vel = {0, 0, 0};
	if (H_Parent()) 
	{
		CPhysicsShellHolder* pPhysicsShellHolder = smart_cast<CPhysicsShellHolder*>(H_Parent());
		if(pPhysicsShellHolder) pPhysicsShellHolder->PHGetLinearVell(vel);
	}
	CParticlesPlayer::SetParentVel	(vel);

	// 
	UpdateLights					();
	if(m_activationObj && m_activationObj->IsInProgress())	{
		CPHUpdateObject::Activate			();
		m_activationObj->UpdateActivation	();
		return;
	}

	// custom-logic
	if(!CAttachableItem::enabled())
		UpdateCLChild					();

}

void CArtefact::shedule_Update		(u32 dt) 
{
	inherited::shedule_Update		(dt);

	//////////////////////////////////////////////////////////////////////////
	// check "fast-mode" border
	if (H_Parent())			o_switch_2_slow	();
	else					{
		Fvector	center;			Center(center);
		BOOL	rendering		= (Device.dwFrame==o_render_frame);
		float	cam_distance	= Device.vCameraPosition.distance_to(center)-Radius();
		if (rendering || (cam_distance < FASTMODE_DISTANCE))	o_switch_2_fast	();
		else													o_switch_2_slow	();
	}
	if (!o_fastmode)		UpdateWorkload	(dt);

	if(!H_Parent() && m_detectorObj)
	{
		m_detectorObj->UpdateOnFrame();
	}
}


void CArtefact::create_physic_shell	()
{
	m_pPhysicsShell=P_build_Shell(this,false);
	m_pPhysicsShell->Deactivate();
}

void CArtefact::StartLights()
{
	VERIFY(!physics_world()->Processing());
	if(!m_bLightsEnabled)		return;

	VERIFY							(m_pTrailLight == NULL);
	m_pTrailLight					= ::Render->light_create();
	bool const b_light_shadow = !!pSettings->r_bool(cNameSect(), "idle_light_shadow");

	m_pTrailLight->set_shadow	(m_bLightsEnabled);
	m_pTrailLight->set_moveable	(true);

	m_pTrailLight->set_color	(m_TrailLightColor); 
	m_pTrailLight->set_range	(m_fTrailLightRange);
	m_pTrailLight->set_position	(Position()); 
	m_pTrailLight->set_active	(true);

	m_pTrailLight->set_flare	(true);

	m_pTrailLight->set_volumetric(m_bVolumetricLights);
	m_pTrailLight->set_volumetric_quality(m_fVolumetricQuality);
	m_pTrailLight->set_volumetric_distance(m_fVolumetricDistance);
	m_pTrailLight->set_volumetric_intensity(m_fVolumetricIntensity);
}

void CArtefact::StopLights()
{
	VERIFY(!physics_world()->Processing());
	if(!m_bLightsEnabled || !m_pTrailLight) 
		return;

	m_pTrailLight->set_active	(false);
	m_pTrailLight.destroy		();
}

void CArtefact::UpdateLights()
{
	VERIFY(!physics_world()->Processing());
	if(!m_bLightsEnabled || !m_pTrailLight ||!m_pTrailLight->get_active()) return;
	m_pTrailLight->set_position(Position());
}

void CArtefact::ActivateArtefact	()
{
	VERIFY(m_bCanSpawnZone);
	VERIFY( H_Parent() );
	CreateArtefactActivation();
	if (!m_activationObj)
		return;
	m_activationObj->Start();

}

void CArtefact::PhDataUpdate	(float step)
{
	if(m_activationObj && m_activationObj->IsInProgress())
		m_activationObj->PhDataUpdate			(step);
}

bool CArtefact::CanTake() const
{
	if (!inherited::CanTake())
		return false;
	
	if (m_activationObj && m_activationObj->IsInProgress())
		return false;

	return true;
}

void CArtefact::Hide()
{
	SwitchState(eHiding);
}

void CArtefact::Show()
{
	SwitchState(eShowing);
}

void CArtefact::MoveTo(Fvector const &  position)
{
	if (!PPhysicsShell())
		return;

	Fmatrix	M = XFORM();
	M.translate(position);
	ForceTransform(M);
	//m_bInInterpolation = false;	
}


#include "inventoryOwner.h"
#include "Entity_alive.h"
void CArtefact::UpdateXForm()
{
	if (Device.dwFrame!=m_dwXF_Frame)
	{
		m_dwXF_Frame			= Device.dwFrame;

		if (0==H_Parent())	return;

		// Get access to entity and its visual
		CEntityAlive*		E		= smart_cast<CEntityAlive*>(H_Parent());
        
		if(!E)				return	;

		const CInventoryOwner	*parent = smart_cast<const CInventoryOwner*>(E);
		if (parent && parent->use_simplified_visual())
			return;

		VERIFY				(E);
		IKinematics*		V		= smart_cast<IKinematics*>	(E->Visual());
		VERIFY				(V);
		if(CAttachableItem::enabled())
			return;

		// Get matrices
		int					boneL = -1, boneR = -1, boneR2 = -1;
		E->g_WeaponBones	(boneL,boneR,boneR2);
		if (boneR == -1)	return;

		boneL = boneR2;

		V->CalculateBones	();
		Fmatrix& mL			= V->LL_GetTransform(u16(boneL));
		Fmatrix& mR			= V->LL_GetTransform(u16(boneR));

		// Calculate
		Fmatrix				mRes;
		Fvector				R,D,N;
		D.sub				(mL.c,mR.c);	D.normalize_safe();
		R.crossproduct		(mR.j,D);		R.normalize_safe();
		N.crossproduct		(D,R);			N.normalize_safe();
		mRes.set			(R,N,D,mR.c);
		mRes.mulA_43		(E->XFORM());
//		UpdatePosition		(mRes);
		XFORM().mul			(mRes,offset());
	}
}
#include "xr_level_controller.h"
bool CArtefact::Action(s32 cmd, u32 flags) 
{
	switch (cmd)
	{
	case kWPN_FIRE:
		{
			if (flags&CMD_START && m_bCanSpawnZone){
				SwitchState(eActivating);
				return true;
			}
			if (flags&CMD_STOP && m_bCanSpawnZone && GetState()==eActivating)
			{
				SwitchState(eIdle);
				return true;
			}
		}break;
	default:
		break;
	}
	return inherited::Action(cmd,flags);
}

void CArtefact::OnStateSwitch(u32 S)
{
	inherited::OnStateSwitch	(S);
	switch(S)
	{
	case eShowing:
		{
			PlayHUDMotion("anm_show", FALSE, this, S);
		}break;
	case eHiding:
		{
			PlayHUDMotion("anm_hide", FALSE, this, S);
		}break;
	case eActivating:
		{
			PlayHUDMotion("anm_activate", FALSE, this, S);
		}break;
	case eIdle:
		{
			PlayAnimIdle();
		}break;
	}
}

void CArtefact::PlayAnimIdle()
{
	PlayHUDMotion("anm_idle", TRUE, NULL, eIdle);
}

void CArtefact::OnAnimationEnd(u32 state)
{
	switch (state)
	{
	case eHiding:
		{
			SwitchState(eHidden);
		}break;
	case eShowing:
		{
			SwitchState(eIdle);
		}break;
	case eActivating:
		{
			if(Local())
			{
				SwitchState		(eHiding);
				NET_Packet		P;
				u_EventGen		(P, GEG_PLAYER_ACTIVATEARTEFACT, H_Parent()->ID());
				P.w_u16			(ID());
				u_EventSend		(P);	
			}
		}break;
	};
}

void CArtefact::FollowByPath(LPCSTR path_name, int start_idx, Fvector magic_force)
{
	if(m_detectorObj)
		m_detectorObj->FollowByPath(path_name, start_idx, magic_force);
}

bool CArtefact::CanBeInvisible()
{
	return (m_detectorObj!=NULL);
}

void CArtefact::SwitchVisibility(bool b)
{
	if(m_detectorObj)
		m_detectorObj->SetVisible(b);

}

void CArtefact::StopActivation()
{
	//VERIFY2(m_activationObj, "activation object not initialized");
	if (!m_activationObj)
		return;
	m_activationObj->Stop();
}

void CArtefact::ForceTransform(const Fmatrix& m)
{
	VERIFY( PPhysicsShell() );
	XFORM().set(m);
	PPhysicsShell()->SetGlTransformDynamic( m );// XFORM().set(m);
}

void CArtefact::CreateArtefactActivation()
{
	if (m_activationObj) {
		return;
	}
	m_activationObj = xr_new<SArtefactActivation>(this, H_Parent()->ID());
}

SArtefactDetectorsSupport::SArtefactDetectorsSupport(CArtefact* A)
:m_parent(A),m_currPatrolPath(NULL),m_currPatrolVertex(NULL),m_switchVisTime(0)
{	
}

SArtefactDetectorsSupport::~SArtefactDetectorsSupport()
{
	m_sound.destroy();
}

void SArtefactDetectorsSupport::SetVisible(bool b)
{
	m_switchVisTime			= Device.dwTimeGlobal; 
	if(b == !!m_parent->getVisible())	return;
	
	if(b)
		m_parent->StartLights	();
	else
		m_parent->StopLights	();

	if(b)
	{
		LPCSTR curr = nullptr;

		if (pSettings->line_exist(m_parent->cNameSect().c_str(), (b) ? "det_show_particles" : "det_hide_particles"))
			curr = pSettings->r_string(m_parent->cNameSect().c_str(), (b) ? "det_show_particles" : "det_hide_particles");
		
		if (curr)
		{
			IKinematics* K = smart_cast<IKinematics*>(m_parent->Visual());
			R_ASSERT2(K, m_parent->cNameSect().c_str());
			LPCSTR bone = pSettings->r_string(m_parent->cNameSect().c_str(), "particles_bone");
			u16 bone_id = K->LL_BoneID(bone);
			R_ASSERT2(bone_id != BI_NONE, bone);

			m_parent->CParticlesPlayer::StartParticles(curr, bone_id, Fvector().set(0, 1, 0), m_parent->ID());

			curr = pSettings->r_string(m_parent->cNameSect().c_str(), (b) ? "det_show_snd" : "det_hide_snd");
			m_sound.create(curr, st_Effect, sg_SourceType);
			m_sound.play_at_pos(0, m_parent->Position(), 0);
		}
	}
	
	m_parent->setVisible	(b);
	m_parent->SwitchAfParticles(b);
}

void SArtefactDetectorsSupport::Blink()
{
	LPCSTR curr = nullptr;

	if (pSettings->line_exist(m_parent->cNameSect().c_str(), "det_show_particles"))
		curr = pSettings->r_string(m_parent->cNameSect().c_str(), "det_show_particles");

	if (!curr)
		return;

	IKinematics* K			= smart_cast<IKinematics*>(m_parent->Visual());
	R_ASSERT2				(K, m_parent->cNameSect().c_str());
	LPCSTR bone				= pSettings->r_string(m_parent->cNameSect().c_str(), "particles_bone");
	u16 bone_id				= K->LL_BoneID(bone);
	R_ASSERT2				(bone_id!=BI_NONE, bone);

	m_parent->CParticlesPlayer::StartParticles(curr,bone_id,Fvector().set(0,1,0),m_parent->ID(), 1000, true);
}

void SArtefactDetectorsSupport::UpdateOnFrame()
{
	if(m_currPatrolPath && !m_parent->getVisible())
	{
		if(m_parent->Position().distance_to(m_destPoint) < 2.0f)
		{
			CPatrolPath::const_iterator b,e;
			m_currPatrolPath->begin(m_currPatrolVertex,b,e);
			if(b!=e)
			{
				std::advance(b, ::Random.randI(s32(e-b)));
				m_currPatrolVertex	= m_currPatrolPath->vertex((*b).vertex_id());
				m_destPoint			= m_currPatrolVertex->data().position();
			}	
		}
		float		cos_et	= _cos(deg2rad(45.f));
		Fvector		dir;
		dir.sub		(m_destPoint, m_parent->Position()).normalize_safe();

		Fvector v;
		m_parent->PHGetLinearVell(v);
		float	cosa		= v.dotproduct(dir);
		if(v.square_magnitude() < (0.7f*0.7f) || (cosa<cos_et) )
		{
			Fvector			power = dir;
			power.y			+= 1.0f;
			power.mul		(m_path_moving_force);
			m_parent->m_pPhysicsShell->applyGravityAccel(power);
		}
	}

	if(m_parent->getVisible() && m_parent->GetAfRank()!=0 && m_switchVisTime+5000 < Device.dwTimeGlobal)
		SetVisible(false);

	u32 dwDt = 2*3600*1000/10; //2 hour of game time
	if(!m_parent->getVisible() && m_switchVisTime+dwDt < Device.dwTimeGlobal)
	{
		m_switchVisTime		= Device.dwTimeGlobal;
		if(m_parent->Position().distance_to(Device.vCameraPosition)>40.0f)
			Blink			();
	}
}

void SArtefactDetectorsSupport::FollowByPath(LPCSTR path_name, int start_idx, Fvector force)
{
	m_currPatrolPath		= ai().patrol_paths().path(path_name,true);
	if(m_currPatrolPath)
	{
		m_currPatrolVertex		= m_currPatrolPath->vertex(start_idx);
		m_destPoint				= m_currPatrolVertex->data().position();
		m_path_moving_force		= force;
	}
}

void CArtefact::OnActiveItem ()
{
	SwitchState					(eShowing);
	inherited::OnActiveItem		();
	SetState					(eIdle);
	SetNextState				(eIdle);
}

void CArtefact::OnHiddenItem ()
{
	if(IsGameTypeSingle())
		SwitchState(eHiding);
	else
		SwitchState(eHidden);

	inherited::OnHiddenItem		();
	SetState					(eHidden);
	SetNextState				(eHidden);
}

void CArtefact::SetChargeLevel(float charge_level)
{
	m_fChargeLevel = charge_level;
}

float CArtefact::GetCurrentChargeLevel() const
{
	return m_fChargeLevel;
}

void CArtefact::SetRank(int rank)
{
	m_iAfRank = rank;
}

int CArtefact::GetCurrentAfRank() const
{
	return m_iAfRank;
}

u32 CArtefact::Cost() const
{
	float percent = m_fChargeLevel * 100;
	u32 res = CInventoryItem::Cost() * m_iAfRank;

	if (GameConstants::GetArtefactsDegradation())
	{
		if (percent >= 10)
			res = (res / 100) * percent;
		else
			res = (res / 100) * 10;
	}

	return res;
}

bool CArtefact::IsInContainer()
{
	return m_bInContainer;
}

float CArtefact::GetRestoreByType(ALife::EConditionRestoreType type) const
{
	float res = 0.f;

	switch (type)
	{
		case ALife::eHealthRestoreSpeed:
		{
			res = m_fHealthRestoreSpeed;
		}break;
		case ALife::eSatietyRestoreSpeed:
		{
			res = m_fSatietyRestoreSpeed;
		}break;
		case ALife::eThirstRestoreSpeed:
		{
			res = m_fThirstRestoreSpeed;
		}break;
		case ALife::eRadiationRestoreSpeed:
		{
			res = m_fRadiationRestoreSpeed;
		}break;
		case ALife::ePowerRestoreSpeed:
		{
			res = m_fPowerRestoreSpeed;
		}break;
		case ALife::eBleedingRestoreSpeed:
		{
			res = m_fBleedingRestoreSpeed;
		}break;
		case ALife::ePsyHealthRestoreSpeed:
		{
			res = m_fPsyHealthRestoreSpeed;
		}break;
		case ALife::eSleepenessRestoreSpeed:
		{
			res = m_fSleepenessRestoreSpeed;
		}break;
		case ALife::eIntoxicationRestoreSpeed:
		{
			res = m_fIntoxicationRestoreSpeed;
		}break;
		case ALife::eAlcoholismRestoreSpeed:
		{
			res = m_fAlcoholismRestoreSpeed;
		}break;
#pragma todo("new params are temporary empty")
		case ALife::eHangoverRestoreSpeed:
		{
			res = 0.f; //m_fHangoverRestoreSpeed;
		}break;
		case ALife::eNarcotismRestoreSpeed:
		{
			res = m_fNarcotismRestoreSpeed;
		}break;
		case ALife::eWithDrawalRestoreSpeed:
		{
			res = 0.f; //m_fWithdrawalRestoreSpeed;
		}break;
		case ALife::eFrostbiteRestoreSpeed:
		{
			res = m_fFrostbiteRestoreSpeed;
		}break;
		case ALife::eInfectionRestoreSpeed:
		{
			res = m_fInfectionRestoreSpeed;
		}break;
		default:
		{
			NODEFAULT;
		}break;
	}
	return res;
}

bool CArtefact::GetBriefInfo(II_BriefInfo& info)
{
	info.name = Name();
	info.cur_ammo = "";
	info.icon = cNameSect();;

	return true;
}
