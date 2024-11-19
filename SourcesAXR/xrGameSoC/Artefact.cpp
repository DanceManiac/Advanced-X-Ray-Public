#include "stdafx.h"
#include "Artefact.h"
#include "PhysicsShell.h"
#include "PhysicsShellHolder.h"
#include "game_cl_base.h"
#include "../Include/xrRender/KinematicsAnimated.h"
#include "../Include/xrRender/Kinematics.h"
#include "inventory.h"
#include "level.h"
#include "ai_object_location.h"
#include "xrServer_Objects_ALife_Monsters.h"
#include "phworld.h"
#include "restriction_space.h"
#include "../xrEngine/IGame_Persistent.h"
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

struct SArtefactActivation{
	enum EActivationStates		{eNone=0, eStarting, eFlying, eBeforeSpawn, eSpawnZone, eMax};
	struct SStateDef{
		float		m_time;
		shared_str	m_snd;
		Fcolor		m_light_color;
		float		m_light_range;
		shared_str	m_particle;
		shared_str	m_animation;
		
					SStateDef	():m_time(0.0f){};
		void		Load		(LPCSTR section, LPCSTR name);
	};

	SArtefactActivation			(CArtefact* af, u32 owner_id);
	~SArtefactActivation		();
	CArtefact*					m_af;
	svector<SStateDef,eMax>		m_activation_states;
	EActivationStates			m_cur_activation_state;
	float						m_cur_state_time;

	ref_light					m_light;
	ref_sound					m_snd;
	
	u32							m_owner_id;

	void						UpdateActivation				();
	void						Load							();
	void						Start							();
	void						ChangeEffects					();
	void						UpdateEffects					();
	void						SpawnAnomaly					();
	void						PhDataUpdate					(dReal step);
};

extern float af_from_container_charge_level;
extern int af_from_container_rank;
extern CArtefactContainer* m_LastAfContainer;

CArtefact::CArtefact(void) 
{
	shedule.t_min				= 20;
	shedule.t_max				= 50;
	m_sParticlesName			= nullptr;
	m_pTrailLight				= nullptr;
	m_activationObj				= nullptr;
	m_additional_weight			= 0.0f;

	m_bVolumetricLights			= false;
	m_fVolumetricQuality		= 1.0f;
	m_fVolumetricDistance		= 0.3f;
	m_fVolumetricIntensity		= 0.5f;

	m_fChargeLevel				= 1.0f;
	m_fDegradationSpeed			= 1.0f;

	//For Degradation
	m_fConstAdditionalWeight	= 0.0f;
	m_bInContainer				= false;

	m_iAfRank					= 1;
}


CArtefact::~CArtefact(void) 
{}

void CArtefact::Load(LPCSTR section) 
{
	inherited::Load			(section);


	if (pSettings->line_exist(section, "particles"))
		m_sParticlesName	= pSettings->r_string(section, "particles");

	m_bLightsEnabled		= !!pSettings->r_bool(section, "lights_enabled");
	
	if(m_bLightsEnabled)
	{
		sscanf(pSettings->r_string(section,"trail_light_color"), "%f,%f,%f", 
			&m_TrailLightColor.r, &m_TrailLightColor.g, &m_TrailLightColor.b);

		m_fConstTrailLightRange		= pSettings->r_float(section, "trail_light_range");

		m_bVolumetricLights			= READ_IF_EXISTS(pSettings, r_bool, section, "volumetric_lights", false);

		if (m_bVolumetricLights)
		{
			m_fVolumetricQuality		= READ_IF_EXISTS(pSettings, r_float, section, "volumetric_quality", 1.0f);
			m_fConstVolumetricDistance	= READ_IF_EXISTS(pSettings, r_float, section, "volumetric_distance", 0.3f);
			m_fConstVolumetricIntensity = READ_IF_EXISTS(pSettings, r_float, section, "volumetric_intensity", 0.5f);
		}
	}
	
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
	m_fConstJumpSpeed					= READ_IF_EXISTS(pSettings, r_float, section, "jump_speed",						1.f);
	m_fConstWalkAccel					= READ_IF_EXISTS(pSettings, r_float, section, "walk_accel",						1.f);

	m_fTrailLightRange				= m_fConstTrailLightRange;

	m_fVolumetricDistance			= m_fConstVolumetricDistance;
	m_fVolumetricIntensity			= m_fConstVolumetricIntensity;

	m_fHealthRestoreSpeed			= m_fConstHealthRestoreSpeed;
	m_fRadiationRestoreSpeed		= m_fConstRadiationRestoreSpeed;
	m_fSatietyRestoreSpeed			= m_fConstSatietyRestoreSpeed;
	m_fPowerRestoreSpeed			= m_fConstPowerRestoreSpeed;
	m_fBleedingRestoreSpeed			= m_fConstBleedingRestoreSpeed;
	m_fThirstRestoreSpeed			= m_fConstThirstRestoreSpeed;
	m_fIntoxicationRestoreSpeed		= m_fConstIntoxicationRestoreSpeed;
	m_fSleepenessRestoreSpeed		= m_fConstSleepenessRestoreSpeed;
	m_fAlcoholismRestoreSpeed		= m_fConstAlcoholismRestoreSpeed;
	m_fNarcotismRestoreSpeed		= m_fConstNarcotismRestoreSpeed;
	m_fPsyHealthRestoreSpeed		= m_fConstPsyHealthRestoreSpeed;
	m_fFrostbiteRestoreSpeed		= m_fConstFrostbiteRestoreSpeed;
	m_additional_weight				= m_fConstAdditionalWeight;
	m_fJumpSpeed					= m_fConstJumpSpeed;
	m_fWalkAccel					= m_fConstWalkAccel;

	m_fChargeLevel					= READ_IF_EXISTS(pSettings, r_float, section, "artefact_charge_level", 1.0f);
	m_fDegradationSpeed				= READ_IF_EXISTS(pSettings, r_float, section, "degradation_speed", 0.0f);

	if(pSettings->section_exist(/**cNameSect(), */pSettings->r_string(section,"hit_absorbation_sect")))
		m_ArtefactHitImmunities.LoadImmunities(pSettings->r_string(section,"hit_absorbation_sect"),pSettings);

	m_ConstHitTypeProtection[ALife::eHitTypeBurn] = (m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeBurn) > 1.0) ?
		((m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeBurn) - 1.0f) / m_iAfRank) + 1.0f :
		((m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeBurn) - 1.0f) * m_iAfRank) + 1.0f;

	m_ConstHitTypeProtection[ALife::eHitTypeStrike]			= (m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeStrike) > 1.0) ? 
		((m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeStrike) - 1.0f) / m_iAfRank) + 1.0f :
		((m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeStrike) - 1.0f) * m_iAfRank) + 1.0f;

	m_ConstHitTypeProtection[ALife::eHitTypeShock]			= (m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeShock) > 1.0) ? 
		((m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeShock) - 1.0f) / m_iAfRank) + 1.0f :
		((m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeShock) - 1.0f) * m_iAfRank) + 1.0f;

	m_ConstHitTypeProtection[ALife::eHitTypeWound]			= (m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeWound) > 1.0) ?
		((m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeWound) - 1.0f) / m_iAfRank) + 1.0f :
		((m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeWound) - 1.0f) * m_iAfRank) + 1.0f;

	m_ConstHitTypeProtection[ALife::eHitTypeRadiation]			= (m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeRadiation) > 1.0) ?
		((m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeRadiation) - 1.0f) / m_iAfRank) + 1.0f :
		((m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeRadiation) - 1.0f) * m_iAfRank) + 1.0f;

	m_ConstHitTypeProtection[ALife::eHitTypeTelepatic]			= (m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeTelepatic) > 1.0) ?
		((m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeTelepatic) - 1.0f) / m_iAfRank) + 1.0f :
		((m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeTelepatic) - 1.0f) * m_iAfRank) + 1.0f;

	m_ConstHitTypeProtection[ALife::eHitTypeChemicalBurn]			= (m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeChemicalBurn) > 1.0) ?
		((m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeChemicalBurn) - 1.0f) / m_iAfRank) + 1.0f :
		((m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeChemicalBurn) - 1.0f) * m_iAfRank) + 1.0f;

	m_ConstHitTypeProtection[ALife::eHitTypeExplosion]			= (m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeExplosion) > 1.0) ?
		((m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeExplosion) - 1.0f) / m_iAfRank) + 1.0f :
		((m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeExplosion) - 1.0f) * m_iAfRank) + 1.0f;

	m_ConstHitTypeProtection[ALife::eHitTypeFireWound]			= (m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeFireWound) > 1.0) ?
		((m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeFireWound) - 1.0f) / m_iAfRank) + 1.0f :
		((m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeFireWound) - 1.0f) * m_iAfRank) + 1.0f;

	m_HitTypeProtection[ALife::eHitTypeBurn]			= m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeBurn);
	m_HitTypeProtection[ALife::eHitTypeStrike]			= m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeStrike);
	m_HitTypeProtection[ALife::eHitTypeShock]			= m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeShock);
	m_HitTypeProtection[ALife::eHitTypeWound]			= m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeWound);
	m_HitTypeProtection[ALife::eHitTypeRadiation]		= m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeRadiation);
	m_HitTypeProtection[ALife::eHitTypeTelepatic]		= m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeTelepatic);
	m_HitTypeProtection[ALife::eHitTypeChemicalBurn]	= m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeChemicalBurn);
	m_HitTypeProtection[ALife::eHitTypeExplosion]		= m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeExplosion);
	m_HitTypeProtection[ALife::eHitTypeFireWound]		= m_ArtefactHitImmunities.GetHitImmunity(ALife::eHitTypeFireWound);

	m_bCanSpawnZone = !!pSettings->line_exist("artefact_spawn_zones", section);
}

BOOL CArtefact::net_Spawn(CSE_Abstract* DC) 
{
	BOOL result = inherited::net_Spawn(DC);
	if (*m_sParticlesName) 
	{Fvector dir;
		dir.set(0,1,0);
		CParticlesPlayer::StartParticles(m_sParticlesName,dir,ID(),-1, false);
	}

	VERIFY(m_pTrailLight == NULL);
	m_pTrailLight = ::Render->light_create();
	m_pTrailLight->set_shadow(true);
	m_pTrailLight->set_moveable(true);

	StartLights();
	/////////////////////////////////////////
	m_CarringBoneID = u16(-1);
	/////////////////////////////////////////
	IKinematicsAnimated	*K=smart_cast<IKinematicsAnimated*>(Visual());
	if(K)K->PlayCycle("idle");
	
	o_fastmode					= FALSE	;		// start initially with fast-mode enabled
	o_render_frame				= 0		;
	SetState					(eHidden);

	return result;	
}

void CArtefact::net_Destroy() 
{
/*
	if (*m_sParticlesName) 
		CParticlesPlayer::StopParticles(m_sParticlesName, BI_NONE, true);
*/
	inherited::net_Destroy		();

	StopLights					();
	m_pTrailLight.destroy		();
	CPHUpdateObject::Deactivate	();
	xr_delete					(m_activationObj);
}

void CArtefact::OnH_A_Chield() 
{
	inherited::OnH_A_Chield		();

	StopLights();
	if (GameID() == GAME_SINGLE)
	{
		if (*m_sParticlesName) 
		{	
			CParticlesPlayer::StopParticles(m_sParticlesName, BI_NONE, true);
		}
	}
	else
	{
		IKinematics* K	= smart_cast<IKinematics*>(H_Parent()->Visual());
		if (K)
			m_CarringBoneID			= K->LL_BoneID("bip01_head");
		else
			m_CarringBoneID = u16(-1);
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
	VERIFY(!ph_world->Processing());
	inherited::OnH_B_Independent(just_before_destroy);

	StartLights();
	if (*m_sParticlesName) 
	{
		Fvector dir;
		dir.set(0,1,0);
		CParticlesPlayer::StartParticles(m_sParticlesName,dir,ID(),-1, false);
	}
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
	save_data(m_additional_weight, packet);
	save_data(m_fJumpSpeed, packet);
	save_data(m_fWalkAccel, packet);
	save_data(m_iAfRank, packet);
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
	load_data(m_additional_weight, packet);
	load_data(m_fJumpSpeed, packet);
	load_data(m_fWalkAccel, packet);
	load_data(m_iAfRank, packet);
}


// called only in "fast-mode"
void CArtefact::UpdateCL		() 
{
	inherited::UpdateCL			();

	if (GameConstants::GetArtefactsDegradation() && ParentIsActor())
		UpdateDegradation();
	
	if (o_fastmode || m_activationObj)
		UpdateWorkload			(Device.dwTimeDelta);	

	if (m_fChargeLevel <= 0.5f)
	{
		m_bCanSpawnZone = false;
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
	if (m_additional_weight > 0.0f && m_fConstAdditionalWeight > 0.0f)
		m_additional_weight = (m_fConstAdditionalWeight / 100) * percent;
	if (m_fJumpSpeed > 1.f && m_fConstJumpSpeed > 1.f)
		m_fJumpSpeed = (m_fConstJumpSpeed / 100) * percent;
	if (m_fWalkAccel > 1.f && m_fConstWalkAccel > 1.f)
		m_fWalkAccel = (m_fConstWalkAccel / 100) * percent;
	if (m_fChargeLevel <= 0.0f)
		m_iAfRank = 0;

	for (size_t i = 0; i < ALife::eHitTypeMax; ++i)
	{
		if (m_HitTypeProtection[(ALife::EHitType)i] < 1.0f && m_ConstHitTypeProtection[(ALife::EHitType)i] < 1.0f)
			m_HitTypeProtection[(ALife::EHitType)i] = ((m_ConstHitTypeProtection[(ALife::EHitType)i]) * 100) / percent;
		else if (m_HitTypeProtection[(ALife::EHitType)i] > 1.0f && m_ConstHitTypeProtection[(ALife::EHitType)i] > 1.0f)
			m_HitTypeProtection[(ALife::EHitType)i] = m_ConstHitTypeProtection[(ALife::EHitType)i];
	}

	//Lights
	if (m_bLightsEnabled)
	{
		if (m_fTrailLightRange >= 0.0f)
			m_fTrailLightRange = (m_fConstTrailLightRange / 100) * percent;
		else
			m_bLightsEnabled = false;
	}

	//Volumetric Lights
	if (m_bVolumetricLights)
	{
		if (m_fVolumetricDistance >= 0.0f)
			m_fVolumetricDistance = (m_fConstVolumetricDistance / 100) * percent;
		else if (m_fVolumetricIntensity >= 0.0f)
			m_fVolumetricIntensity = (m_fConstVolumetricIntensity / 100) * percent;
		else
			m_bVolumetricLights = false;
	}
}

void CArtefact::UpdateWorkload		(u32 dt) 
{
	VERIFY(!ph_world->Processing());
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
	if(m_activationObj)	{
		CPHUpdateObject::Activate			();
		m_activationObj->UpdateActivation	();
		return	;
	}

	// custom-logic
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
}


void CArtefact::create_physic_shell	()
{
	///create_box2sphere_physic_shell	();
	m_pPhysicsShell=P_build_Shell(this,false);
	m_pPhysicsShell->Deactivate();
}

void CArtefact::StartLights()
{
	VERIFY(!ph_world->Processing());
	if(!m_bLightsEnabled) return;

	//включить световую подсветку от двигателя
	m_pTrailLight->set_color(m_TrailLightColor.r, 
		m_TrailLightColor.g, 
		m_TrailLightColor.b);

	m_pTrailLight->set_range(m_fTrailLightRange);
	m_pTrailLight->set_position(Position()); 
	m_pTrailLight->set_active(true);
	m_pTrailLight->set_flare(true);
	m_pTrailLight->set_volumetric(m_bVolumetricLights);
}

void CArtefact::StopLights()
{
	VERIFY(!ph_world->Processing());
	if(!m_bLightsEnabled) return;
	m_pTrailLight->set_active(false);
}

void CArtefact::UpdateLights()
{
	VERIFY(!ph_world->Processing());
	if(!m_bLightsEnabled || !m_pTrailLight->get_active()) return;
	m_pTrailLight->set_position(Position());
}

void CArtefact::ActivateArtefact	()
{
	VERIFY(m_bCanSpawnZone);
	VERIFY( H_Parent() );
	m_activationObj = xr_new<SArtefactActivation>(this,H_Parent()->ID());
	m_activationObj->Start();

}

void CArtefact::PhDataUpdate	(dReal step)
{
	if(m_activationObj)
		m_activationObj->PhDataUpdate			(step);
}

bool CArtefact::CanTake() const
{
	if(!inherited::CanTake())return false;
	return (m_activationObj==NULL);
}

void CArtefact::Hide()
{
	SwitchState(eHiding);
}

void CArtefact::Show()
{
	SwitchState(eShowing);
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

		// Get matrices
		int					boneL,boneR,boneR2;
		E->g_WeaponBones	(boneL,boneR,boneR2);

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

void CArtefact::OnStateSwitch(u32 S, u32 oldState)
{
    inherited::OnStateSwitch(S, oldState);
    switch (S)
    {
    case eShowing: {
		PlayHUDMotionIfExists({"anim_show", "anm_show"}, false, S);
    }
    break;
    case eHiding: {
        if (oldState != eHiding)
			PlayHUDMotionIfExists({"anim_hide", "anm_hide"}, true, S);
    }
    break;
    case eActivating: {
		PlayHUDMotionIfExists({"anim_activate", "anm_activate"}, true, S);
    }
    break;
    case eIdle: {
        PlayAnimIdle();
    }
    break;
    }
}

void CArtefact::PlayAnimIdle()
{
	PlayHUDMotionIfExists({"anim_idle", "anm_idle"}, FALSE, eIdle);
}

void CArtefact::OnAnimationEnd(u32 state)
{
	switch (state)
	{
	case eHiding:
		{
			SwitchState(eHidden);
//.			if(m_pCurrentInventory->GetNextActiveSlot()!=NO_ACTIVE_SLOT)
//.				m_pCurrentInventory->Activate(m_pCurrentInventory->GetPrevActiveSlot());
		}break;
	case eShowing:
		{
			SwitchState(eIdle);
		}break;
	case eActivating:
		{
			if(Local()){
				SwitchState		(eHiding);
				NET_Packet		P;
				u_EventGen		(P, GEG_PLAYER_ACTIVATEARTEFACT, H_Parent()->ID());
				P.w_u16			(ID());
				u_EventSend		(P);	
			}
		}break;
	};
}



u16	CArtefact::bone_count_to_synchronize	() const
{
	return CInventoryItem::object().PHGetSyncItemsNumber();
}



//---SArtefactActivation----
SArtefactActivation::SArtefactActivation(CArtefact* af,u32 owner_id)
{
	m_af			= af;
	Load			();
	m_light			= ::Render->light_create();
	m_light->set_shadow(true);
	m_owner_id		= owner_id;
}
SArtefactActivation::~SArtefactActivation()
{
	m_light.destroy();

}

void SArtefactActivation::Load()
{
	for(int i=0; i<(int)eMax; ++i)
		m_activation_states.push_back(SStateDef());

	LPCSTR activation_seq = pSettings->r_string(*m_af->cNameSect(),"artefact_activation_seq");


	m_activation_states[(int)eStarting].Load(activation_seq,	"starting");
	m_activation_states[(int)eFlying].Load(activation_seq,		"flying");
	m_activation_states[(int)eBeforeSpawn].Load(activation_seq,	"idle_before_spawning");
	m_activation_states[(int)eSpawnZone].Load(activation_seq,	"spawning");

}

void SArtefactActivation::Start()
{
	VERIFY(!ph_world->Processing());
	m_af->StopLights				();
	m_cur_activation_state			= eStarting;
	m_cur_state_time				= 0.0f;
	
	m_af->processing_activate();

	NET_Packet						P;
	CGameObject::u_EventGen			(P,GE_OWNERSHIP_REJECT, m_af->H_Parent()->ID());
	P.w_u16							(m_af->ID());
	if (OnServer())
		CGameObject::u_EventSend		(P);
	m_light->set_active				(true);
	ChangeEffects					();
}

void SArtefactActivation::UpdateActivation()
{
	VERIFY(!ph_world->Processing());
	m_cur_state_time				+=	Device.fTimeDelta;
	if(m_cur_state_time				>=	m_activation_states[int(m_cur_activation_state)].m_time){
		m_cur_activation_state		=	(EActivationStates)(int)(m_cur_activation_state+1);
		
		if(m_cur_activation_state == eMax){
			m_cur_activation_state = eNone;

			m_af->processing_deactivate			();
			m_af->CPHUpdateObject::Deactivate	();
			m_af->DestroyObject();
		}

		m_cur_state_time	= 0.0f;
		ChangeEffects				();


	if(m_cur_activation_state==eSpawnZone && OnServer())
		SpawnAnomaly	();

	}
	UpdateEffects				();

}

void SArtefactActivation::PhDataUpdate(dReal step)
{
	if (m_cur_activation_state==eFlying) {
		Fvector dir	= {0, -1.f, 0};
		if(Level().ObjectSpace.RayTest(m_af->Position(), dir, 1.0f, collide::rqtBoth,NULL,m_af) ){
			dir.y = ph_world->Gravity()*1.1f; 
			m_af->m_pPhysicsShell->applyGravityAccel(dir);
		}
	}

}
void SArtefactActivation::ChangeEffects()
{
	VERIFY(!ph_world->Processing());
	SStateDef& state_def = m_activation_states[(int)m_cur_activation_state];
	
	if(m_snd._feedback())
		m_snd.stop();
	
	if(state_def.m_snd.size()){
		m_snd.create			(*state_def.m_snd,st_Effect,sg_SourceType);
		m_snd.play_at_pos		(m_af,	m_af->Position());
	};

	m_light->set_range		(	state_def.m_light_range);
	m_light->set_color		(	state_def.m_light_color.r,
								state_def.m_light_color.g,
								state_def.m_light_color.b);
	
	if(state_def.m_particle.size()){
		Fvector dir;
		dir.set(0,1,0);

		m_af->CParticlesPlayer::StartParticles(	state_def.m_particle,
												dir,
												m_af->ID(),
												iFloor(state_def.m_time*1000) );
	};
	if(state_def.m_animation.size()){
		IKinematicsAnimated	*K=smart_cast<IKinematicsAnimated*>(m_af->Visual());
		if(K)K->PlayCycle(*state_def.m_animation);
	}

}

void SArtefactActivation::UpdateEffects()
{
	VERIFY(!ph_world->Processing());
	if(m_snd._feedback())
		m_snd.set_position( m_af->Position() );
	
	m_light->set_position(m_af->Position());
}

void SArtefactActivation::SpawnAnomaly()
{
	VERIFY(!ph_world->Processing());
	string128 tmp;
	LPCSTR str			= pSettings->r_string("artefact_spawn_zones",*m_af->cNameSect());
	VERIFY3(3==_GetItemCount(str),"Bad record format in artefact_spawn_zones",str);
	float zone_radius	= (float)atof(_GetItem(str,1,tmp));
	float zone_power	= (float)atof(_GetItem(str,2,tmp));
	LPCSTR zone_sect	= _GetItem(str,0,tmp); //must be last call of _GetItem... (LPCSTR !!!)

		Fvector pos;
		m_af->Center(pos);
		CSE_Abstract		*object = Level().spawn_item(	zone_sect,
															pos,
															(g_dedicated_server)?u32(-1):m_af->ai_location().level_vertex_id(),
															0xffff,
															true
		);
		CSE_ALifeAnomalousZone*		AlifeZone = smart_cast<CSE_ALifeAnomalousZone*>(object);
		VERIFY(AlifeZone);
		CShapeData::shape_def		_shape;
		_shape.data.sphere.P.set	(0.0f,0.0f,0.0f);
		_shape.data.sphere.R		= zone_radius;
		_shape.type					= CShapeData::cfSphere;
		AlifeZone->assign_shapes	(&_shape,1);
		AlifeZone->m_maxPower		= zone_power;
		AlifeZone->m_owner_id		= m_owner_id;
		AlifeZone->m_space_restrictor_type	= RestrictionSpace::eRestrictorTypeNone;

		NET_Packet					P;
		object->Spawn_Write			(P,TRUE);
		Level().Send				(P,net_flags(TRUE));
		F_entity_Destroy			(object);
//. #ifdef DEBUG
		Msg("artefact [%s] spawned a zone [%s] at [%f]", *m_af->cName(), zone_sect, Device.fTimeGlobal);
//. #endif
}

shared_str clear_brackets(LPCSTR src)
{
	if	(0==src)					return	shared_str(0);
	
	if( NULL == strchr(src,'"') )	return	shared_str(src);

	string512						_original;	
	strcpy_s						(_original,src);
	u32			_len				= xr_strlen(_original);
	if	(0==_len)					return	shared_str("");
	if	('"'==_original[_len-1])	_original[_len-1]=0;					// skip end
	if	('"'==_original[0])			return	shared_str(&_original[0] + 1);	// skip begin
	return									shared_str(_original);

}
void SArtefactActivation::SStateDef::Load(LPCSTR section, LPCSTR name)
{
	LPCSTR str = pSettings->r_string(section,name);
	VERIFY(_GetItemCount(str)==8);


	string128 tmp;

	m_time			= (float)atof(		_GetItem(str,0,tmp) );
	
	m_snd			= clear_brackets(	_GetItem(str,1,tmp) )	;

	m_light_color.r = (float)atof(		_GetItem(str,2,tmp) );
	m_light_color.g = (float)atof(		_GetItem(str,3,tmp) );
	m_light_color.b = (float)atof(		_GetItem(str,4,tmp) );

	m_light_range	= (float)atof(		_GetItem(str,5,tmp) );

	m_particle		= clear_brackets(	_GetItem(str,6,tmp) );
	m_animation		= clear_brackets(	_GetItem(str,7,tmp) );

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
	float percent = m_fChargeLevel*100;
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
#pragma todo("new params are temporary empty")
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
		default:
		{
			NODEFAULT;
		}break;
	}
	return res;
}

bool CArtefact::IsInContainer()
{
	return m_bInContainer;
}

bool CArtefact::ParentIsActor()
{
	CObject* O = H_Parent();
	if (!O)
		return false;

	CEntityAlive* EA = smart_cast<CEntityAlive*>(O);
	if (!EA)
		return false;

	return EA->cast_actor() != nullptr;
}