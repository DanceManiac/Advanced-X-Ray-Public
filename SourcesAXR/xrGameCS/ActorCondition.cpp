#include "pch_script.h"
#include "actorcondition.h"
#include "actor.h"
#include "actorEffector.h"
#include "inventory.h"
#include "level.h"
#include "sleepeffector.h"
#include "game_base_space.h"
#include "autosave_manager.h"
#include "xrserver.h"
#include "ai_space.h"
#include "script_callback_ex.h"
#include "script_game_object.h"
#include "game_object_space.h"
#include "ui\UIVideoPlayerWnd.h"
#include "script_callback_ex.h"
#include "object_broker.h"
#include "weapon.h"

#include "PDA.h"
#include "ai/monsters/BaseMonster/base_monster.h"
#include "ui/UIMainIngameWnd.h"
#include "AdvancedXrayGameConstants.h"
#include "CustomOutfit.h"

constexpr auto MAX_SATIETY = 1.0f;
constexpr auto START_SATIETY = 0.5f;
constexpr auto MAX_THIRST = 1.0f;
constexpr auto START_THIRST = 0.5f;

BOOL	GodMode	()	
{ 
	if (GameID() == eGameIDSingle) 
		return psActorFlags.test(AF_GODMODE|AF_GODMODE_RT); 
	return FALSE;	
}

CActorCondition::CActorCondition(CActor *object) :
	inherited	(object)
{
	m_fJumpPower				= 0.f;
	m_fStandPower				= 0.f;
	m_fWalkPower				= 0.f;
	m_fJumpWeightPower			= 0.f;
	m_fWalkWeightPower			= 0.f;
	m_fOverweightWalkK			= 0.f;
	m_fOverweightJumpK			= 0.f;
	m_fAccelK					= 0.f;
	m_fSprintK					= 0.f;
	m_fAlcohol					= 0.f;
	m_fSatiety					= 1.0f;
	m_fThirst					= 1.0f;
	m_fIntoxication				= 0.0f;
	m_fSleepeness				= 0.0f;
	m_fAlcoholism				= 0.0f;
	m_fHangover					= 0.0f;
	m_fNarcotism				= 0.0f;
	m_fWithdrawal				= 0.0f;
	m_fDrugs					= 0.0f;
	m_fFrostbite				= 0.0f;
	m_fV_PsyHealth_Health		= 0.0f;

	m_bPsyHealthKillActor		= false;

	VERIFY						(object);
	m_object					= object;
	m_condition_flags.zero		();
	m_death_effector			= NULL;

	m_zone_max_power[ALife::infl_rad]	= 1.0f;
	m_zone_max_power[ALife::infl_fire]	= 1.0f;
	m_zone_max_power[ALife::infl_acid]	= 1.0f;
	m_zone_max_power[ALife::infl_psi]	= 1.0f;
	m_zone_max_power[ALife::infl_electra]= 1.0f;

	m_zone_danger[ALife::infl_rad]	= 0.0f;
	m_zone_danger[ALife::infl_fire]	= 0.0f;
	m_zone_danger[ALife::infl_acid]	= 0.0f;
	m_zone_danger[ALife::infl_psi]	= 0.0f;
	m_zone_danger[ALife::infl_electra]= 0.0f;
	m_f_time_affected = Device.fTimeGlobal;

	m_max_power_restore_speed	= 0.0f;
	m_max_wound_protection		= 0.0f;
	m_max_fire_wound_protection = 0.0f;
}

CActorCondition::~CActorCondition()
{
	xr_delete( m_death_effector );
}

void CActorCondition::LoadCondition(LPCSTR entity_section)
{
	inherited::LoadCondition(entity_section);

	LPCSTR						section = READ_IF_EXISTS(pSettings,r_string,entity_section,"condition_sect",entity_section);

	m_fJumpPower				= pSettings->r_float(section,"jump_power");
	m_fStandPower				= pSettings->r_float(section,"stand_power");
	m_fWalkPower				= pSettings->r_float(section,"walk_power");
	m_fJumpWeightPower			= pSettings->r_float(section,"jump_weight_power");
	m_fWalkWeightPower			= pSettings->r_float(section,"walk_weight_power");
	m_fOverweightWalkK			= pSettings->r_float(section,"overweight_walk_k");
	m_fOverweightJumpK			= pSettings->r_float(section,"overweight_jump_k");
	m_fAccelK					= pSettings->r_float(section,"accel_k");
	m_fSprintK					= pSettings->r_float(section,"sprint_k");

	//порог силы и здоровья меньше которого актер начинает хромать
	m_fLimpingHealthBegin		= pSettings->r_float(section,	"limping_health_begin");
	m_fLimpingHealthEnd			= pSettings->r_float(section,	"limping_health_end");
	R_ASSERT					(m_fLimpingHealthBegin<=m_fLimpingHealthEnd);

	m_fLimpingPowerBegin		= pSettings->r_float(section,	"limping_power_begin");
	m_fLimpingPowerEnd			= pSettings->r_float(section,	"limping_power_end");
	R_ASSERT					(m_fLimpingPowerBegin<=m_fLimpingPowerEnd);

	m_fCantWalkPowerBegin		= pSettings->r_float(section,	"cant_walk_power_begin");
	m_fCantWalkPowerEnd			= pSettings->r_float(section,	"cant_walk_power_end");
	R_ASSERT					(m_fCantWalkPowerBegin<=m_fCantWalkPowerEnd);

	m_fCantSprintPowerBegin		= pSettings->r_float(section,	"cant_sprint_power_begin");
	m_fCantSprintPowerEnd		= pSettings->r_float(section,	"cant_sprint_power_end");
	R_ASSERT					(m_fCantSprintPowerBegin<=m_fCantSprintPowerEnd);

	m_fPowerLeakSpeed			= pSettings->r_float(section,"max_power_leak_speed");
	
	m_fV_Alcohol				= pSettings->r_float(section,"alcohol_v");

	m_fSatietyCritical			= pSettings->r_float(section, "satiety_critical");
	clamp						(m_fSatietyCritical, 0.0f, 1.0f);
	m_fV_Satiety				= pSettings->r_float(section,"satiety_v");		
	m_fV_SatietyPower			= pSettings->r_float(section,"satiety_power_v");
	m_fV_SatietyHealth			= pSettings->r_float(section,"satiety_health_v");
	
	m_MaxWalkWeight				= pSettings->r_float(section,"max_walk_weight");

	m_zone_max_power[ALife::infl_rad]	= pSettings->r_float(section, "radio_zone_max_power" );
	m_zone_max_power[ALife::infl_fire]	= pSettings->r_float(section, "fire_zone_max_power" );
	m_zone_max_power[ALife::infl_acid]	= pSettings->r_float(section, "acid_zone_max_power" );
	m_zone_max_power[ALife::infl_psi]	= pSettings->r_float(section, "psi_zone_max_power" );
	m_zone_max_power[ALife::infl_electra]= pSettings->r_float(section, "electra_zone_max_power" );

	VERIFY( !fis_zero(m_zone_max_power[ALife::infl_rad]) );
	VERIFY( !fis_zero(m_zone_max_power[ALife::infl_fire]) );
	VERIFY( !fis_zero(m_zone_max_power[ALife::infl_acid]) );
	VERIFY( !fis_zero(m_zone_max_power[ALife::infl_psi]) );
	VERIFY( !fis_zero(m_zone_max_power[ALife::infl_electra]) );

	// M.F.S. Team Thirst
	m_fThirstCritical = READ_IF_EXISTS(pSettings,r_float,section,"thirst_critical", 1.0f);
	clamp(m_fThirstCritical, 0.0f, 1.0f);
	m_fV_Thirst					= READ_IF_EXISTS(pSettings,r_float,section,"thirst_v", 0.0f);
	m_fV_ThirstPower			= READ_IF_EXISTS(pSettings,r_float,section,"thirst_power_v", 0.0f);
	m_fV_ThirstHealth			= READ_IF_EXISTS(pSettings,r_float,section,"thirst_health_v", 0.0f);
	m_fThirstAccelTemp			= READ_IF_EXISTS(pSettings,r_float,section,"thirst_accel_temp", 0.0f);

	// M.F.S. Team Intoxication
	m_fIntoxicationCritical		= READ_IF_EXISTS(pSettings,r_float,section,"intoxication_critical", 0.0f);
	clamp(m_fIntoxicationCritical, 0.0f, 1.0f);
	m_fV_Intoxication			= READ_IF_EXISTS(pSettings,r_float,section,"intoxication_v", 0.0f);
	m_fV_IntoxicationHealth		= READ_IF_EXISTS(pSettings,r_float,section,"intoxication_health_v", 0.0f);

	// M.F.S. Team Sleepeness
	m_fSleepenessCritical		= READ_IF_EXISTS(pSettings,r_float,section,"sleepeness_critical", 0.0f);
	clamp(m_fSleepenessCritical, 0.0f, 1.0f);
	m_fV_Sleepeness				= READ_IF_EXISTS(pSettings,r_float,section,"sleepeness_v", 0.0f);
	m_fV_SleepenessPower		= READ_IF_EXISTS(pSettings,r_float,section,"sleepeness_power_v", 0.0f);
	m_fV_SleepenessPsyHealth	= READ_IF_EXISTS(pSettings,r_float,section,"sleepeness_psy_health_v", 0.0f);
	m_fSleepeness_V_Sleep		= READ_IF_EXISTS(pSettings,r_float,section,"sleepeness_v_sleep", 0.0f);

	// M.F.S. Team Alcoholism (History Of Puhtinskyi)
	m_fV_Alcoholism				= READ_IF_EXISTS(pSettings,r_float,section,"alcoholism_v", 0.0f);
	m_fHangoverCritical			= READ_IF_EXISTS(pSettings,r_float,section,"hangover_critical", 0.0f);
	clamp(m_fHangoverCritical, 0.0f, 1.0f);
	m_fV_Hangover				= READ_IF_EXISTS(pSettings,r_float,section,"hangover_v", 0.0f);
	m_fV_HangoverPower			= READ_IF_EXISTS(pSettings,r_float,section,"hangover_power_v", 0.0f);

	// M.F.S. Team Narcotism (History Of Puhtinskyi)
	m_fV_Narcotism				= READ_IF_EXISTS(pSettings,r_float,section,"narcotism_v", 0.0f);
	m_fWithdrawalCritical		= READ_IF_EXISTS(pSettings,r_float,section,"withdrawal_critical", 0.0f);
	clamp(m_fWithdrawalCritical, 0.0f, 1.0f);
	m_fV_Withdrawal				= READ_IF_EXISTS(pSettings,r_float,section,"withdrawal_v", 0.0f);
	m_fV_WithdrawalPower		= READ_IF_EXISTS(pSettings,r_float,section,"withdrawal_power_v", 0.0f);
	m_fV_WithdrawalHealth		= READ_IF_EXISTS(pSettings,r_float,section,"withdrawal_health_v", 0.0f);
	m_fV_Drugs					= READ_IF_EXISTS(pSettings,r_float,section,"drugs_v", 0.0f);

	// M.F.S. Team Frostbite
	m_fFrostbiteCritical		= READ_IF_EXISTS(pSettings,r_float,section,"frostbite_critical", 0.0f);
	clamp(m_fFrostbiteCritical, 0.0f, 1.0f);
	m_fV_Frostbite				= READ_IF_EXISTS(pSettings,r_float,section,"frostbite_v", 0.0f);
	m_fV_FrostbiteAdd			= READ_IF_EXISTS(pSettings,r_float,section,"frostbite_v_add", 0.0f);
	m_fFrostbiteIncTemp			= READ_IF_EXISTS(pSettings,r_float,section,"frostbite_inc_temp", 0.0f);;
	m_fFrostbiteDecTemp			= READ_IF_EXISTS(pSettings,r_float,section,"frostbite_dec_temp", 0.0f);
	m_fV_FrostbiteHealth		= READ_IF_EXISTS(pSettings,r_float,section,"frostbite_health_v", 0.0f);

	// Psy Health Stuff
	m_bPsyHealthKillActor		= READ_IF_EXISTS(pSettings, r_bool, section, "psy_health_kill_actor", false);
	m_fV_PsyHealth_Health		= READ_IF_EXISTS(pSettings, r_float, section, "psy_health_health_v", 0.0f);

	// M.F.S. Team Skills System
	m_fV_SatietySkill			= READ_IF_EXISTS(pSettings, r_float, "skills_influence", "skills_satiety_restore", 0.0f);
	m_fV_HealthSkill			= READ_IF_EXISTS(pSettings, r_float, "skills_influence", "skills_health_restore", 0.0f);
	m_fV_BleedingSkill			= READ_IF_EXISTS(pSettings, r_float, "skills_influence", "skills_bleeding_restore", 0.0f);
	m_fV_RadiationSkill			= READ_IF_EXISTS(pSettings, r_float, "skills_influence", "skills_radiation_restore", 0.0f);
	m_fV_PowerSkill				= READ_IF_EXISTS(pSettings, r_float, "skills_influence", "skills_power_restore", 0.0f);
	m_fV_ThirstSkill			= READ_IF_EXISTS(pSettings, r_float, "skills_influence", "skills_thirst_restore", 0.0f);
	m_fV_IntoxicationSkill		= READ_IF_EXISTS(pSettings, r_float, "skills_influence", "skills_intoxication_restore", 0.0f);
	m_fV_SleepenessSkill		= READ_IF_EXISTS(pSettings, r_float, "skills_influence", "skills_sleepeness_restore", 0.0f);
	m_fV_FrostbiteSkill			= READ_IF_EXISTS(pSettings, r_float, "skills_influence", "skills_frostbite_restore", 0.0f);
	m_fV_FrostbiteAddSkill		= READ_IF_EXISTS(pSettings, r_float, "skills_influence", "skills_frostbite_v_add", 0.0f);

	m_fMaxWeightSkill			= READ_IF_EXISTS(pSettings, r_float, "skills_influence", "skills_max_weight", 0.0f);
	m_fJumpSpeedSkill			= READ_IF_EXISTS(pSettings, r_float, "skills_influence", "skills_jump_speed", 0.0f);
	m_fWalkAccelSkill			= READ_IF_EXISTS(pSettings, r_float, "skills_influence", "skills_walk_accel", 0.0f);

	m_fPackingSkill				= READ_IF_EXISTS(pSettings, r_float, "skills_influence", "skills_packing", 0.0f);
}

float CActorCondition::GetZoneMaxPower( ALife::EInfluenceType type) const
{
	if ( type < ALife::infl_rad || ALife::infl_electra < type )
	{
		return 1.0f;
	}
	return m_zone_max_power[type];
}

float CActorCondition::GetZoneMaxPower( ALife::EHitType hit_type ) const
{
	ALife::EInfluenceType iz_type = ALife::infl_max_count;
	switch( hit_type )
	{
	case ALife::eHitTypeRadiation:		iz_type = ALife::infl_rad;		break;
	case ALife::eHitTypeBurn:			iz_type = ALife::infl_fire;		break;
	case ALife::eHitTypeChemicalBurn:	iz_type = ALife::infl_acid;		break;
	case ALife::eHitTypeTelepatic:		iz_type = ALife::infl_psi;		break;
	case ALife::eHitTypeShock:			iz_type = ALife::infl_electra;	break;

	case ALife::eHitTypeStrike:
	case ALife::eHitTypeWound:
	case ALife::eHitTypeExplosion:
	case ALife::eHitTypeFireWound:
	case ALife::eHitTypeWound_2:
	case ALife::eHitTypePhysicStrike:
		return 1.0f;
	default:
		break;
	}
	
	return GetZoneMaxPower( iz_type );
}


//вычисление параметров с ходом времени
#include "UI.h"
#include "HUDManager.h"

void CActorCondition::UpdateCondition()
{
	if(psActorFlags.test(AF_GODMODE_RT))
	{
		UpdateSatiety();
		UpdateBoosters();

		if (GameConstants::GetActorThirst())
			UpdateThirst();

		if (GameConstants::GetActorIntoxication())
			UpdateIntoxication();

		if (GameConstants::GetActorSleepeness())
			UpdateSleepeness();
		
		if (GameConstants::GetActorAlcoholism())
			UpdateAlcoholism();

		if (GameConstants::GetActorNarcotism())
			UpdateNarcotism();

		if (GameConstants::GetActorFrostbite())
			UpdateFrostbite();

		UpdatePsyHealth();

		m_fAlcohol		+= m_fV_Alcohol*m_fDeltaTime;
		clamp			(m_fAlcohol,			0.0f,		1.0f);

		m_fDrugs += m_fV_Drugs * m_fDeltaTime;
		clamp(m_fDrugs, 0.0f, 1.0f);

		if(IsGameTypeSingle())
		{
			CEffectorCam* ce = Actor()->Cameras().GetCamEffector((ECamEffectorType)effAlcohol);
			if(ce)
				RemoveEffector(m_object,effAlcohol);
		}
	}

	if (GodMode())				return;
	if (!object().g_Alive())	return;
	if (!object().Local() && m_object != Level().CurrentViewEntity())		return;	
	
	float base_weight			= object().MaxCarryWeight();
	float cur_weight			= object().inventory().TotalWeight();

	if ((object().mstate_real&mcAnyMove))
	{
		ConditionWalk( cur_weight / base_weight,
			isActorAccelerated( object().mstate_real,object().IsZoomAimingMode() ),
			(object().mstate_real&mcSprint) != 0 );
	}
	else
	{
		ConditionStand( cur_weight / base_weight );
	}
	
	if ( IsGameTypeSingle() )
	{
		float k_max_power = 1.0f;
		if( true )
		{
			k_max_power = 1.0f + _min(cur_weight, base_weight) / base_weight
				+ _max(0.0f, (cur_weight - base_weight) / 10.0f);
		}
		else
		{
			k_max_power = 1.0f;
		}
		SetMaxPower		(GetMaxPower() - m_fPowerLeakSpeed * m_fDeltaTime * k_max_power);
	}


	m_fAlcohol		+= m_fV_Alcohol*m_fDeltaTime;
	clamp			(m_fAlcohol,			0.0f,		1.0f);

	m_fDrugs		+= m_fV_Drugs * m_fDeltaTime;
	clamp			(m_fDrugs,				0.0f,		1.0f);

	if ( IsGameTypeSingle() )
	{	
		CEffectorCam* ce = Actor()->Cameras().GetCamEffector((ECamEffectorType)effAlcohol);
		if	((m_fAlcohol>0.0001f) ){
			if(!ce){
				AddEffector(m_object,effAlcohol, "effector_alcohol", GET_KOEFF_FUNC(this, &CActorCondition::GetAlcohol));
			}
		}else{
			if(ce)
				RemoveEffector(m_object,effAlcohol);
		}

		CEffectorCam* ceDrugs = Actor()->Cameras().GetCamEffector((ECamEffectorType)effDrugs);
		if ((m_fDrugs > 0.0001f))
		{
			if (!ceDrugs)
			{
				AddEffector(m_object, effDrugs, "effector_drugs", GET_KOEFF_FUNC(this, &CActorCondition::GetDrugs));
			}
		}
		else
		{
			if (ceDrugs)
				RemoveEffector(m_object, effDrugs);
		}
	};

	UpdateSatiety();
	UpdateBoosters();
	UpdatePsyHealth();

	if (GameConstants::GetActorThirst())
		UpdateThirst();

	if (GameConstants::GetActorIntoxication())
		UpdateIntoxication();

	if (GameConstants::GetActorSleepeness())
		UpdateSleepeness();

	if (GameConstants::GetActorAlcoholism())
		UpdateAlcoholism();

	if (GameConstants::GetActorNarcotism())
		UpdateNarcotism();

	if (GameConstants::GetActorFrostbite())
		UpdateFrostbite();

	inherited::UpdateCondition();

	if( IsGameTypeSingle() )
		UpdateTutorialThresholds();

	if(GetHealth()<0.05f && m_death_effector==NULL && IsGameTypeSingle())
	{
		if(pSettings->section_exist("actor_death_effector"))
			m_death_effector = xr_new<CActorDeathEffector>(this, "actor_death_effector");
	}
	if(m_death_effector && m_death_effector->IsActual())
	{
		m_death_effector->UpdateCL	();

		if(!m_death_effector->IsActual())
			m_death_effector->Stop();
	}

	AffectDamage_InjuriousMaterialAndMonstersInfluence();
}

void CActorCondition::UpdateBoosters()
{
	for (u8 i = 0; i < eBoostMaxCount; i++)
	{
		BOOSTER_MAP::iterator it = m_booster_influences.find((EBoostParams)i);
		if (it != m_booster_influences.end())
		{
			it->second.fBoostTime -= m_fDeltaTime / (IsGameTypeSingle() ? Level().GetGameTimeFactor() : 1.0f);
			if (it->second.fBoostTime <= 0.0f)
			{
				DisableBoostParameters(it->second);
				m_booster_influences.erase(it);
			}
		}
	}

	if (m_object == Level().CurrentViewEntity())
		HUD().GetUI()->UIMainIngameWnd->UpdateBoosterIndicators(m_booster_influences);
}

void CActorCondition::AffectDamage_InjuriousMaterialAndMonstersInfluence()
{
	float one = 0.1f;
	float tg = Device.fTimeGlobal;
	if (m_f_time_affected + one > tg)
		return;

	clamp( m_f_time_affected, tg - (one * 3), tg );

	float psy_influence					=	0;
	float fire_influence				=	0;
	float acid_influence				=	0;
	float radiation_influence			=	GetInjuriousMaterialDamage(); // Get Radiation from Material

	// Add Radiation and Psy Level from Monsters
	CPda* const pda						=	m_object->GetPDA();

	if ( pda )
	{
		typedef xr_vector<CObject*>				monsters;

		for ( monsters::const_iterator	it	=	pda->feel_touch.begin();
										it	!=	pda->feel_touch.end();
										++it )
		{
			CBaseMonster* const	monster		=	smart_cast<CBaseMonster*>(*it);
			if (!monster) continue;

			if (monster->g_Alive())
			{
				psy_influence		+= monster->get_psy_influence();
				radiation_influence += monster->get_radiation_influence();
				fire_influence		+= monster->get_fire_influence();
				acid_influence		+= monster->get_acid_influence();
			}
			else
			{
				if (monster->get_enable_psy_aura_after_die())
					psy_influence += monster->get_psy_influence();

				if (monster->get_enable_rad_aura_after_die())
					radiation_influence += monster->get_radiation_influence();

				if (monster->get_enable_fire_aura_after_die())
					fire_influence += monster->get_fire_influence();

				if (monster->get_enable_acid_aura_after_die())
					acid_influence += monster->get_acid_influence();
			}
		}
	}

	struct 
	{
		ALife::EHitType	type;
		float			value;

	} hits[]		=	{	{ ALife::eHitTypeRadiation,		radiation_influence	*	one },
							{ ALife::eHitTypeTelepatic,		psy_influence		*	one }, 
							{ ALife::eHitTypeBurn,			fire_influence		*	one },
							{ ALife::eHitTypeChemicalBurn,	acid_influence		*	one } };

 	NET_Packet	np;

	while ( m_f_time_affected + one < tg )
	{
		m_f_time_affected			+=	one;

		for ( int i=0; i<sizeof(hits)/sizeof(hits[0]); ++i )
		{
			float			damage	=	hits[i].value;
			ALife::EHitType	type	=	hits[i].type;

			if ( damage > EPS )
			{
				SHit HDS = SHit(damage, 
//.								0.0f, 
								Fvector().set(0,1,0), 
								NULL, 
								BI_NONE, 
								Fvector().set(0,0,0), 
								0.0f, 
								type, 
								0.0f, 
								false);

				HDS.GenHeader(GE_HIT, m_object->ID());
				HDS.Write_Packet( np );
				CGameObject::u_EventSend( np );
			}

		} // for

	}//while
}

#include "characterphysicssupport.h"
float CActorCondition::GetInjuriousMaterialDamage()
{
	u16 mat_injurios = m_object->character_physics_support()->get_movement()->injurious_material_idx();

	if(mat_injurios!=GAMEMTL_NONE_IDX)
	{
		const SGameMtl* mtl		= GMLib.GetMaterialByIdx(mat_injurios);
		return					mtl->fInjuriousSpeed;
	}else
		return 0.0f;
}

void CActorCondition::SetZoneDanger( float danger, ALife::EInfluenceType type )
{
	VERIFY( type != ALife::infl_max_count );
	m_zone_danger[type] = danger;
	clamp( m_zone_danger[type], 0.0f, 1.0f );
}

float CActorCondition::GetZoneDanger() const
{
	float sum = 0.0f;
	for ( u8 i = 0; i < ALife::infl_strike; ++i )
	{
		sum += m_zone_danger[i];
	}

	clamp( sum, 0.0f, 1.5f );
	return sum;
}

void CActorCondition::UpdateRadiation()
{
	inherited::UpdateRadiation();
}

void CActorCondition::UpdateSatiety()
{
 	if (!IsGameTypeSingle()) 
	{
		m_fDeltaPower += m_fV_SatietyPower * m_fDeltaTime;
 		return;
	}

	if(m_fSatiety>0)
	{
		m_fSatiety -= m_fV_Satiety*m_fDeltaTime;
		clamp(m_fSatiety, 0.0f, 1.0f);
	}
		
	float satiety_health_koef = (m_fSatiety-m_fSatietyCritical)/(m_fSatiety>=m_fSatietyCritical?1-m_fSatietyCritical:m_fSatietyCritical);
	if(CanBeHarmed() && !psActorFlags.test(AF_GODMODE_RT) )
	{
		m_fDeltaHealth += m_fV_SatietyHealth*satiety_health_koef*m_fDeltaTime;
		m_fDeltaPower += m_fV_SatietyPower*m_fSatiety*m_fDeltaTime;
	}
}

//M.F.S. Team Thirst
void CActorCondition::UpdateThirst()
{
	if (!IsGameTypeSingle()) return;
	if (fis_zero(m_fV_Thirst)) return;

	if (m_fThirst > 0)
	{
		float cur_temperature = g_pGamePersistent->Environment().CurrentEnv->m_fAirTemperature;

		if (cur_temperature >= m_fThirstAccelTemp)
			m_fThirst -= (m_fV_Thirst + (cur_temperature / 1000000.f)) * m_fDeltaTime;
		else
			m_fThirst -= m_fV_Thirst * m_fDeltaTime;

		clamp(m_fThirst, 0.0f, 1.0f);
	}

	float thirst_health_koef = (m_fThirst - m_fThirstCritical) / (m_fThirst >= m_fThirstCritical ? 1 - m_fThirstCritical : m_fThirstCritical);
	if (CanBeHarmed() && !psActorFlags.test(AF_GODMODE_RT))
	{
		m_fDeltaHealth += m_fV_ThirstHealth * thirst_health_koef*m_fDeltaTime;
		m_fDeltaPower += m_fV_ThirstPower * m_fThirst*m_fDeltaTime;
	}
}

//M.F.S. Team Intoxication
void CActorCondition::UpdateIntoxication()
{
	if (fis_zero(m_fV_Intoxication)) return;
	
	CEffectorCam* ce = Actor()->Cameras().GetCamEffector((ECamEffectorType)effIntoxication);
	if ((m_fIntoxication >= m_fIntoxicationCritical))
	{
		if (!ce)
			AddEffector(m_object, effIntoxication, "effector_intoxication", GET_KOEFF_FUNC(this, &CActorCondition::GetIntoxication));
	}
	else
	{
		if (ce)
			RemoveEffector(m_object, effIntoxication);
	}

	if (m_fIntoxication > 0)
	{
		m_fIntoxication -= m_fV_Intoxication * m_fDeltaTime;
		clamp(m_fIntoxication, 0.0f, 1.0f);
	}

	if (CanBeHarmed() && !psActorFlags.test(AF_GODMODE_RT))
	{
		if (m_fIntoxication >= m_fIntoxicationCritical && GetHealth() >= 0.25)
			m_fDeltaHealth -= m_fV_IntoxicationHealth * m_fIntoxication * m_fDeltaTime;
		else if (m_fIntoxication >= 0.9f && GetHealth() <= 0.25)
			m_fDeltaHealth -= m_fV_IntoxicationHealth * m_fIntoxication * m_fDeltaTime;
	}
}

//M.F.S. Team Sleepeness
void CActorCondition::UpdateSleepeness()
{
	if (fis_zero(m_fV_Sleepeness)) return;
	
	if (GetSleepeness() >= 0.85f && !GameConstants::GetSleepInfluenceOnPsyHealth())
	{
		luabind::functor<void> funct;
		if (ai().script_engine().functor("mfs_functions.generate_phantoms", funct))
			funct();
	}

	if (GetSleepeness() >= 1.0f)
	{
		luabind::functor<void> funct;
		if (ai().script_engine().functor("mfs_functions.put_the_actor_to_sleep", funct))
			funct();
	}

	CEffectorCam* ce = Actor()->Cameras().GetCamEffector((ECamEffectorType)effSleepeness);
	if (m_fSleepeness <= m_fSleepenessCritical)
	{
		if (!ce && pSettings->section_exist("effector_sleepeness"))
			AddEffector(m_object, effSleepeness, "effector_sleepeness", GET_KOEFF_FUNC(this, &CActorCondition::GetSleepeness));
	}
	else
	{
		if (ce)
			RemoveEffector(m_object, effSleepeness);
	}

	if (m_fSleepeness < 1.0f)
	{
		if (Actor()->HasInfo("actor_is_sleeping"))
			m_fSleepeness -= m_fSleepeness_V_Sleep * m_fDeltaTime;
		else
			m_fSleepeness += m_fV_Sleepeness * m_fDeltaTime;

		clamp(m_fSleepeness, 0.0f, 1.0f);
	}

	if (CanBeHarmed() && !psActorFlags.test(AF_GODMODE_RT))
	{
		if (m_fSleepeness >= m_fSleepenessCritical)
		{
			m_fDeltaPower -= m_fV_SleepenessPower * m_fSleepeness * m_fDeltaTime;

			if (GameConstants::GetSleepInfluenceOnPsyHealth())
				m_fDeltaPsyHealth -= m_fV_SleepenessPsyHealth * m_fSleepeness * m_fDeltaTime;
		}
	}
}

//M.F.S. Team Alcoholism
void CActorCondition::UpdateAlcoholism()
{
	if (fis_zero(m_fV_Alcoholism)) return;
	
	if (m_fAlcoholism > 0.0f)
	{
		if (m_fAlcohol <= 0.0f)
		{
			m_fAlcoholism -= m_fV_Alcoholism * m_fDeltaTime;
			clamp(m_fAlcoholism, 0.0f, 4.0f);
		}
	}

	if (m_fAlcoholism >= 1.0f && m_fAlcohol <= 0.0f)
	{
		if (CanBeHarmed() && !psActorFlags.test(AF_GODMODE_RT))
		{
			if (m_fHangover >= m_fHangoverCritical)
			{
				m_fDeltaPower -= m_fV_HangoverPower * m_fHangover * m_fDeltaTime;

				luabind::functor<void> funct;
				if (ai().script_engine().functor("mfs_functions.on_actor_hangover", funct))
					funct();
			}
		}

		m_fHangover += m_fV_Hangover * m_fDeltaTime;
		clamp(m_fHangover, 0.0f, 3.0f);
	}
	else
	{
		m_fHangover -= m_fV_Hangover * m_fDeltaTime;
		clamp(m_fHangover, 0.0f, 3.0f);
	}
}

//M.F.S. Team Narcotism
void CActorCondition::UpdateNarcotism()
{
	if (fis_zero(m_fV_Narcotism)) return;
	
	if (m_fNarcotism > 0.0f)
	{
		if (m_fDrugs <= 0.0f)
		{
			m_fNarcotism -= m_fV_Narcotism * m_fDeltaTime;
			clamp(m_fNarcotism, 0.0f, 10.0f);
		}
	}

	if (m_fDrugs >= 0.5f)
	{
		luabind::functor<void> funct;
		if (ai().script_engine().functor("mfs_functions.on_actor_drugs", funct))
			funct();
	}

	if (m_fNarcotism >= 1.0f && m_fDrugs <= 0.0f)
	{
		if (CanBeHarmed() && !psActorFlags.test(AF_GODMODE_RT))
		{
			if (m_fWithdrawal >= m_fWithdrawalCritical)
			{
				m_fDeltaPower -= m_fV_WithdrawalPower * m_fWithdrawal * m_fDeltaTime;

				if (GetHealth() >= 0.5)
					m_fDeltaHealth -= m_fV_WithdrawalHealth * m_fWithdrawal * m_fDeltaTime;

				luabind::functor<void> funct2;
				if (ai().script_engine().functor("mfs_functions.on_actor_withdrawal", funct2))
					funct2();
			}
		}

		m_fWithdrawal += m_fV_Withdrawal * m_fDeltaTime;
		clamp(m_fWithdrawal, 0.0f, 3.0f);
	}
	else
	{
		m_fWithdrawal -= m_fV_Withdrawal * m_fDeltaTime;
		clamp(m_fWithdrawal, 0.0f, 3.0f);
	}
}

//M.F.S. Team Psy Health
void CActorCondition::UpdatePsyHealth()
{
	if (GetPsy() > 0.85f)
	{
		luabind::functor<void> funct;
		if (ai().script_engine().functor("mfs_functions.generate_phantoms", funct))
			funct();

		if (m_bPsyHealthKillActor)
			m_fDeltaHealth -= m_fV_PsyHealth_Health * GetPsy() * m_fDeltaTime;
	}

	CEffectorPP* ppePsyHealth = object().Cameras().GetPPEffector((EEffectorPPType)effPsyHealth);

	if (!fsimilar(GetPsyHealth(), 1.0f, 0.05f))
	{
		if (!ppePsyHealth && pSettings->section_exist("effector_psy_health"))
			AddEffector(m_object, effPsyHealth, "effector_psy_health", GET_KOEFF_FUNC(this, &CActorCondition::GetPsy));
	}
	else
	{
		if (ppePsyHealth)
			RemoveEffector(m_object, effPsyHealth);
	}
}

//M.F.S. Team Frostbite
void CActorCondition::UpdateFrostbite()
{
	if (fis_zero(m_fV_Frostbite)) return;
	
	CEffectorCam* ce = Actor()->Cameras().GetCamEffector((ECamEffectorType)effFrostbite);
	if ((m_fFrostbite >= m_fFrostbiteCritical))
	{
		if (!ce)
			AddEffector(m_object, effFrostbite, "effector_frostbite", GetFrostbite() / 4);
	}
	else
	{
		if (ce)
			RemoveEffector(m_object, effFrostbite);
	}

	if (Actor()->GetCurrentHeating() <= 0.0f)
		Actor()->SetHeatingStatus(false);

	float cur_temperature = g_pGamePersistent->Environment().CurrentEnv->m_fAirTemperature;
	bool IsHeat = Actor()->GetHeatingStatus() || cur_temperature > m_fFrostbiteDecTemp; // Сейчас ГГ около источника тепла

	if (IsHeat)
		m_fFrostbite -= (m_fV_Frostbite * Actor()->GetCurrentHeating() + (cur_temperature / 1000000.f)) * m_fDeltaTime;
	else
	{
		if (!g_pGamePersistent->IsActorInHideout() && cur_temperature < m_fFrostbiteIncTemp)
			m_fFrostbite += ((m_fV_FrostbiteAdd - m_fV_FrostbiteAddSkill + abs(cur_temperature / 1000000.f))) * m_fDeltaTime;
	}

	clamp(m_fFrostbite, 0.0f, 1.0f);

	if (CanBeHarmed() && !psActorFlags.test(AF_GODMODE_RT))
	{
		if (m_fFrostbite >= m_fFrostbiteCritical && GetHealth() >= 0.25)
			m_fDeltaHealth -= m_fV_FrostbiteHealth * m_fFrostbite * m_fDeltaTime;
		else if (m_fFrostbite >= 0.75f && GetHealth() <= 0.25)
			m_fDeltaHealth -= m_fV_FrostbiteHealth * m_fFrostbite * m_fDeltaTime;
	}
}

CWound* CActorCondition::ConditionHit(SHit* pHDS)
{
	if (GodMode()) return NULL;
	return inherited::ConditionHit(pHDS);
}

void CActorCondition::PowerHit(float power, bool apply_outfit)
{
	m_fPower			-=	apply_outfit ? HitPowerEffect(power) : power;
	clamp					(m_fPower, 0.f, 1.f);
}

//weight - "удельный" вес от 0..1
void CActorCondition::ConditionJump(float weight)
{
	if (GodMode()) return;

	float power			=	m_fJumpPower;
	power				+=	m_fJumpWeightPower*weight*(weight>1.f?m_fOverweightJumpK:1.f);
	m_fPower			-=	HitPowerEffect(power);
	clamp				(m_fPower, 0.f, 1.f);
}
void CActorCondition::ConditionWalk(float weight, bool accel, bool sprint)
{	
	float overweight_k = m_fOverweightWalkK;

	CCustomOutfit* outfit = object().GetOutfit();
	if (outfit)
		overweight_k += outfit->m_fOverweightWalkK;

	float power = m_fWalkPower;
	power += m_fWalkWeightPower * weight*(weight > 1.f ? overweight_k : 1.f);
	power *= m_fDeltaTime * (accel ? (sprint ? m_fSprintK : m_fAccelK) : 1.f);
	m_fPower			-= HitPowerEffect(power);
	clamp				(m_fPower, 0.f, 1.f);
}

void CActorCondition::ConditionStand(float weight)
{	
	float power			= m_fStandPower;
	power				*= m_fDeltaTime;
	m_fPower			-= power;
	clamp				(m_fPower, 0.f, 1.f);
}


bool CActorCondition::IsCantWalk() const
{
	if(m_fPower< m_fCantWalkPowerBegin)
		m_bCantWalk		= true;
	else if(m_fPower > m_fCantWalkPowerEnd)
		m_bCantWalk		= false;
	return				m_bCantWalk;
}

bool CActorCondition::IsCantWalkWeight()
{
	if(IsGameTypeSingle() && !GodMode())
	{
		float max_w	= m_object->MaxWalkWeight();

		if( object().inventory().TotalWeight() > max_w )
		{
			m_condition_flags.set			(eCantWalkWeight, TRUE);
			return true;
		}
	}
	m_condition_flags.set					(eCantWalkWeight, FALSE);
	return false;
}

bool CActorCondition::IsCantSprint() const
{
	if(m_fPower< m_fCantSprintPowerBegin)
		m_bCantSprint	= true;
	else if(m_fPower > m_fCantSprintPowerEnd)
		m_bCantSprint	= false;
	return				m_bCantSprint;
}

bool CActorCondition::IsLimping() const
{
	if(m_fPower< m_fLimpingPowerBegin || GetHealth() < m_fLimpingHealthBegin)
		m_bLimping = true;
	else if(m_fPower > m_fLimpingPowerEnd && GetHealth() > m_fLimpingHealthEnd)
		m_bLimping = false;
	return m_bLimping;
}
extern bool g_bShowHudInfo;

void CActorCondition::save(NET_Packet &output_packet)
{
	inherited::save		(output_packet);
	save_data			(m_fAlcohol, output_packet);
	save_data			(m_condition_flags, output_packet);
	save_data			(m_fSatiety, output_packet);
	save_data			(m_fThirst,  output_packet);
	save_data			(m_fIntoxication, output_packet);
	save_data			(m_fSleepeness, output_packet);
	save_data			(m_fAlcoholism, output_packet);
	save_data			(m_fHangover, output_packet);
	save_data			(m_fNarcotism, output_packet);
	save_data			(m_fWithdrawal, output_packet);
	save_data			(m_fDrugs, output_packet);
	save_data			(m_fFrostbite, output_packet);

	save_data			(m_curr_medicine_influence.fHealth, output_packet);
	save_data			(m_curr_medicine_influence.fPower, output_packet);
	save_data			(m_curr_medicine_influence.fSatiety, output_packet);
	save_data			(m_curr_medicine_influence.fRadiation, output_packet);
	save_data			(m_curr_medicine_influence.fWoundsHeal, output_packet);
	save_data			(m_curr_medicine_influence.fMaxPowerUp, output_packet);
	save_data			(m_curr_medicine_influence.fAlcohol, output_packet);
	save_data			(m_curr_medicine_influence.fTimeTotal, output_packet);
	save_data			(m_curr_medicine_influence.fTimeCurrent, output_packet);
	save_data			(m_curr_medicine_influence.fThirst, output_packet);
	save_data			(m_curr_medicine_influence.fIntoxication, output_packet);
	save_data			(m_curr_medicine_influence.fSleepeness, output_packet);
	save_data			(m_curr_medicine_influence.fAlcoholism, output_packet);
	save_data			(m_curr_medicine_influence.fHangover, output_packet);
	save_data			(m_curr_medicine_influence.fNarcotism, output_packet);
	save_data			(m_curr_medicine_influence.fWithdrawal, output_packet);
	save_data			(m_curr_medicine_influence.fDrugs, output_packet);
	save_data			(m_curr_medicine_influence.fFrostbite, output_packet);

	output_packet.w_u8((u8)m_booster_influences.size());
	BOOSTER_MAP::iterator b = m_booster_influences.begin(), e = m_booster_influences.end();
	for (; b != e; b++)
	{
		output_packet.w_u8((u8)b->second.m_type);
		output_packet.w_float(b->second.fBoostValue);
		output_packet.w_float(b->second.fBoostTime);
	}
}

void CActorCondition::load(IReader &input_packet)
{
	inherited::load		(input_packet);
	load_data			(m_fAlcohol, input_packet);
	load_data			(m_condition_flags, input_packet);
	load_data			(m_fSatiety, input_packet);
	load_data			(m_fThirst,  input_packet);
	load_data			(m_fIntoxication, input_packet);
	load_data			(m_fSleepeness, input_packet);
	load_data			(m_fAlcoholism, input_packet);
	load_data			(m_fHangover, input_packet);
	load_data			(m_fNarcotism, input_packet);
	load_data			(m_fWithdrawal, input_packet);
	load_data			(m_fDrugs, input_packet);
	load_data			(m_fFrostbite, input_packet);

	load_data			(m_curr_medicine_influence.fHealth, input_packet);
	load_data			(m_curr_medicine_influence.fPower, input_packet);
	load_data			(m_curr_medicine_influence.fSatiety, input_packet);
	load_data			(m_curr_medicine_influence.fRadiation, input_packet);
	load_data			(m_curr_medicine_influence.fWoundsHeal, input_packet);
	load_data			(m_curr_medicine_influence.fMaxPowerUp, input_packet);
	load_data			(m_curr_medicine_influence.fAlcohol, input_packet);
	load_data			(m_curr_medicine_influence.fTimeTotal, input_packet);
	load_data			(m_curr_medicine_influence.fTimeCurrent, input_packet);
	load_data			(m_curr_medicine_influence.fThirst, input_packet);
	load_data			(m_curr_medicine_influence.fIntoxication, input_packet);
	load_data			(m_curr_medicine_influence.fSleepeness, input_packet);
	load_data			(m_curr_medicine_influence.fAlcoholism, input_packet);
	load_data			(m_curr_medicine_influence.fHangover, input_packet);
	load_data			(m_curr_medicine_influence.fNarcotism, input_packet);
	load_data			(m_curr_medicine_influence.fWithdrawal, input_packet);
	load_data			(m_curr_medicine_influence.fDrugs, input_packet);
	load_data			(m_curr_medicine_influence.fFrostbite, input_packet);

	u8 cntr = input_packet.r_u8();
	for (; cntr > 0; cntr--)
	{
		SBooster B;
		B.m_type = (EBoostParams)input_packet.r_u8();
		B.fBoostValue = input_packet.r_float();
		B.fBoostTime = input_packet.r_float();
		m_booster_influences[B.m_type] = B;
		BoostParameters(B, false);
	}
}

void CActorCondition::reinit	()
{
	inherited::reinit	();
	m_bLimping					= false;
	m_fSatiety					= 1.f;
}

void CActorCondition::ChangeAlcohol	(float value)
{
	m_fAlcohol += value;
}

void CActorCondition::ChangeSatiety(float value)
{
	m_fSatiety += value;
	clamp		(m_fSatiety, 0.0f, 1.0f);
}

//M.F.S. Team Thirst
void CActorCondition::ChangeThirst(float value)
{
	m_fThirst += value;
	clamp(m_fThirst, 0.0f, 1.0f);
}

//M.F.S. Team Intoxication
void CActorCondition::ChangeIntoxication(float value)
{
	m_fIntoxication += value;
	clamp(m_fIntoxication, 0.0f, 1.0f);
}

//M.F.S. Team Sleepeness
void CActorCondition::ChangeSleepeness(float value)
{
	m_fSleepeness += value;
	clamp(m_fSleepeness, 0.0f, 1.0f);
}

//M.F.S. Team Alcoholism (HoP)
void CActorCondition::ChangeAlcoholism(float value)
{
	m_fAlcoholism += value;
	clamp(m_fAlcoholism, 0.0f, 4.0f);
}

void CActorCondition::ChangeHangover(float value)
{
	m_fHangover += value;
	clamp(m_fHangover, 0.0f, 3.0f);
}

//M.F.S. Team Narcotism (HoP)
void CActorCondition::ChangeNarcotism(float value)
{
	m_fNarcotism += value;
	clamp(m_fNarcotism, 0.0f, 10.0f);
}

void CActorCondition::ChangeWithdrawal(float value)
{
	m_fWithdrawal += value;
	clamp(m_fWithdrawal, 0.0f, 3.0f);
}

void CActorCondition::ChangeDrugs(float value)
{
	m_fDrugs += value;
}

//M.F.S. Team Psy Health
void CActorCondition::ChangePsyHealth(float value)
{
	m_fPsyHealth += value;
	clamp(m_fPsyHealth, 0.0f, 1.0f);
}

//M.F.S. Team Frostbite
void CActorCondition::ChangeFrostbite(float value)
{
	m_fFrostbite += value;
	clamp(m_fFrostbite, 0.0f, 1.0f);
}

void CActorCondition::BoostParameters(const SBooster& B, bool need_change_tf)
{
	if(OnServer())
	{
		switch(B.m_type)
		{
			case eBoostHpRestore: BoostHpRestore(B.fBoostValue); break;
			case eBoostPowerRestore: BoostPowerRestore(B.fBoostValue); break;
			case eBoostRadiationRestore: BoostRadiationRestore(B.fBoostValue); break;
			case eBoostBleedingRestore: BoostBleedingRestore(B.fBoostValue); break;
			case eBoostMaxWeight: BoostMaxWeight(B.fBoostValue); break;
			case eBoostBurnImmunity: BoostBurnImmunity(B.fBoostValue); break;
			case eBoostShockImmunity: BoostShockImmunity(B.fBoostValue); break;
			case eBoostRadiationImmunity: BoostRadiationImmunity(B.fBoostValue); break;
			case eBoostTelepaticImmunity: BoostTelepaticImmunity(B.fBoostValue); break;
			case eBoostChemicalBurnImmunity: BoostChemicalBurnImmunity(B.fBoostValue); break;
			case eBoostExplImmunity: BoostExplImmunity(B.fBoostValue); break;
			case eBoostStrikeImmunity: BoostStrikeImmunity(B.fBoostValue); break;
			case eBoostFireWoundImmunity: BoostFireWoundImmunity(B.fBoostValue); break;
			case eBoostWoundImmunity: BoostWoundImmunity(B.fBoostValue); break;
			case eBoostRadiationProtection: BoostRadiationProtection(B.fBoostValue); break;
			case eBoostTelepaticProtection: BoostTelepaticProtection(B.fBoostValue); break;
			case eBoostChemicalBurnProtection: BoostChemicalBurnProtection(B.fBoostValue); break;
			case eBoostSatietyRestore: BoostSatietyRestore(B.fBoostValue); break;
			case eBoostThirstRestore: BoostThirstRestore(B.fBoostValue); break;
			case eBoostPsyHealthRestore: BoostPsyHealthRestore(B.fBoostValue); break;
			case eBoostIntoxicationRestore: BoostIntoxicationRestore(B.fBoostValue); break;
			case eBoostSleepenessRestore: BoostSleepenessRestore(B.fBoostValue); break;
			case eBoostAlcoholRestore: BoostAlcoholRestore(B.fBoostValue); break;
			case eBoostAlcoholismRestore: BoostAlcoholismRestore(B.fBoostValue); break;
			case eBoostHangoverRestore: BoostHangoverRestore(B.fBoostValue); break;
			case eBoostDrugsRestore: BoostDrugsRestore(B.fBoostValue); break;
			case eBoostNarcotismRestore: BoostNarcotismRestore(B.fBoostValue); break;
			case eBoostWithdrawalRestore: BoostWithdrawalRestore(B.fBoostValue); break;
			case eBoostFrostbiteRestore: BoostFrostbiteRestore(B.fBoostValue); break;
			case eBoostTimeFactor: need_change_tf ? BoostTimeFactor(B.fBoostValue) : BoostTimeFactor(0.0f); break;
			default: NODEFAULT;	
		}
	}
}
void CActorCondition::DisableBoostParameters(const SBooster& B)
{
	if(!OnServer())
		return;

	switch(B.m_type)
	{
		case eBoostHpRestore: BoostHpRestore(-B.fBoostValue); break;
		case eBoostPowerRestore: BoostPowerRestore(-B.fBoostValue); break;
		case eBoostRadiationRestore: BoostRadiationRestore(-B.fBoostValue); break;
		case eBoostBleedingRestore: BoostBleedingRestore(-B.fBoostValue); break;
		case eBoostMaxWeight: BoostMaxWeight(-B.fBoostValue); break;
		case eBoostBurnImmunity: BoostBurnImmunity(-B.fBoostValue); break;
		case eBoostShockImmunity: BoostShockImmunity(-B.fBoostValue); break;
		case eBoostRadiationImmunity: BoostRadiationImmunity(-B.fBoostValue); break;
		case eBoostTelepaticImmunity: BoostTelepaticImmunity(-B.fBoostValue); break;
		case eBoostChemicalBurnImmunity: BoostChemicalBurnImmunity(-B.fBoostValue); break;
		case eBoostExplImmunity: BoostExplImmunity(-B.fBoostValue); break;
		case eBoostStrikeImmunity: BoostStrikeImmunity(-B.fBoostValue); break;
		case eBoostFireWoundImmunity: BoostFireWoundImmunity(-B.fBoostValue); break;
		case eBoostWoundImmunity: BoostWoundImmunity(-B.fBoostValue); break;
		case eBoostRadiationProtection: BoostRadiationProtection(-B.fBoostValue); break;
		case eBoostTelepaticProtection: BoostTelepaticProtection(-B.fBoostValue); break;
		case eBoostChemicalBurnProtection: BoostChemicalBurnProtection(-B.fBoostValue); break;
		case eBoostSatietyRestore: BoostSatietyRestore(-B.fBoostValue); break;
		case eBoostThirstRestore: BoostThirstRestore(-B.fBoostValue); break;
		case eBoostPsyHealthRestore: BoostPsyHealthRestore(-B.fBoostValue); break;
		case eBoostIntoxicationRestore: BoostIntoxicationRestore(-B.fBoostValue); break;
		case eBoostSleepenessRestore: BoostSleepenessRestore(-B.fBoostValue); break;
		case eBoostAlcoholRestore: BoostAlcoholRestore(-B.fBoostValue); break;
		case eBoostAlcoholismRestore: BoostAlcoholismRestore(-B.fBoostValue); break;
		case eBoostHangoverRestore: BoostHangoverRestore(-B.fBoostValue); break;
		case eBoostDrugsRestore: BoostDrugsRestore(-B.fBoostValue); break;
		case eBoostNarcotismRestore: BoostNarcotismRestore(-B.fBoostValue); break;
		case eBoostWithdrawalRestore: BoostWithdrawalRestore(-B.fBoostValue); break;
		case eBoostFrostbiteRestore: BoostFrostbiteRestore(-B.fBoostValue); break;
		case eBoostTimeFactor: BoostTimeFactor(-B.fBoostValue); break;
		default: NODEFAULT;	
	}
}

void CActorCondition::WoundForEach(const luabind::functor<bool>& funct)
{
	auto const& cur_wounds = wounds();
	CEntityCondition::WOUND_VECTOR::const_iterator it = wounds().begin();
	CEntityCondition::WOUND_VECTOR::const_iterator it_e = wounds().end();
	for (; it != it_e; ++it)
	{
		if (funct(it) == true)
			break;
	}
}

void CActorCondition::BoosterForEach(const luabind::functor<bool>& funct)
{
	const auto& cur_booster_influences = GetCurBoosterInfluences();
	CEntityCondition::BOOSTER_MAP::const_iterator it = cur_booster_influences.begin();
	CEntityCondition::BOOSTER_MAP::const_iterator it_e = cur_booster_influences.end();
	for (; it != it_e; ++it)
	{
		if (funct((*it).first, (*it).second.fBoostTime, (*it).second.fBoostValue) == true)
			break;
	}
}

bool CActorCondition::ApplyBooster_script(const SBooster& B, LPCSTR sect)
{
	return ApplyBooster(B, sect);
}

void CActorCondition::ClearAllBoosters()
{
	const auto& cur_booster_influences = GetCurBoosterInfluences();
	CEntityCondition::BOOSTER_MAP::const_iterator it = cur_booster_influences.begin();
	CEntityCondition::BOOSTER_MAP::const_iterator it_e = cur_booster_influences.end();
	for (; it != it_e; ++it)
	{
		DisableBoostParameters((*it).second);
	}
}

void CActorCondition::BoostHpRestore(const float value)
{
	m_change_v.m_fV_HealthRestore += value;
}
void CActorCondition::BoostPowerRestore(const float value)
{
	m_fV_SatietyPower += value;
}
void CActorCondition::BoostRadiationRestore(const float value)
{
	m_change_v.m_fV_Radiation += value;
}
void CActorCondition::BoostBleedingRestore(const float value)
{
	m_change_v.m_fV_WoundIncarnation += value;
}
void CActorCondition::BoostMaxWeight(const float value)
{
	m_object->inventory().SetMaxWeight(object().inventory().GetMaxWeight()+value);
	m_MaxWalkWeight += value;
}
void CActorCondition::BoostBurnImmunity(const float value)
{
	m_fBoostBurnImmunity += value;
}
void CActorCondition::BoostShockImmunity(const float value)
{
	m_fBoostShockImmunity += value;
}
void CActorCondition::BoostRadiationImmunity(const float value)
{
	m_fBoostRadiationImmunity += value;
}
void CActorCondition::BoostTelepaticImmunity(const float value)
{
	m_fBoostTelepaticImmunity += value;
}
void CActorCondition::BoostChemicalBurnImmunity(const float value)
{
	m_fBoostChemicalBurnImmunity += value;
}
void CActorCondition::BoostExplImmunity(const float value)
{
	m_fBoostExplImmunity += value;
}
void CActorCondition::BoostStrikeImmunity(const float value)
{
	m_fBoostStrikeImmunity += value;
}
void CActorCondition::BoostFireWoundImmunity(const float value)
{
	m_fBoostFireWoundImmunity += value;
}
void CActorCondition::BoostWoundImmunity(const float value)
{
	m_fBoostWoundImmunity += value;
}
void CActorCondition::BoostRadiationProtection(const float value)
{
	m_fBoostRadiationProtection += value;
}
void CActorCondition::BoostTelepaticProtection(const float value)
{
	m_fBoostTelepaticProtection += value;
}
void CActorCondition::BoostChemicalBurnProtection(const float value)
{
	m_fBoostChemicalBurnProtection += value;
}

void CActorCondition::BoostTimeFactor(const float value)
{
	m_fBoostTimeFactor = Device.time_factor();

	m_fBoostTimeFactor += value;
	clamp(m_fBoostTimeFactor, 0.1f, 2.0f);

	Device.time_factor(m_fBoostTimeFactor);
	psSpeedOfSound = m_fBoostTimeFactor;
}

void CActorCondition::BoostSatietyRestore(const float value)
{
	m_fV_Satiety += value;
}
void CActorCondition::BoostThirstRestore(const float value)
{
	m_fV_Thirst += value;
}
void CActorCondition::BoostPsyHealthRestore(const float value)
{
	m_change_v.m_fV_PsyHealth += value;
}
void CActorCondition::BoostIntoxicationRestore(const float value)
{
	m_fV_Intoxication += value;
}
void CActorCondition::BoostSleepenessRestore(const float value)
{
	m_fV_Sleepeness += value;
}
void CActorCondition::BoostAlcoholRestore(const float value)
{
	m_fV_Alcohol += value;
}
void CActorCondition::BoostAlcoholismRestore(const float value)
{
	m_fV_Alcoholism += value;
}
void CActorCondition::BoostHangoverRestore(const float value)
{
	m_fV_Hangover += value;
}
void CActorCondition::BoostDrugsRestore(const float value)
{
	m_fV_Drugs += value;
}
void CActorCondition::BoostNarcotismRestore(const float value)
{
	m_fV_Narcotism += value;
}
void CActorCondition::BoostWithdrawalRestore(const float value)
{
	m_fV_Withdrawal += value;
}
void CActorCondition::BoostFrostbiteRestore(const float value)
{
	m_fV_Frostbite += value;
}

void CActorCondition::UpdateTutorialThresholds()
{
	string256						cb_name;
	static float _cPowerThr			= pSettings->r_float("tutorial_conditions_thresholds","power");
	static float _cPowerMaxThr		= pSettings->r_float("tutorial_conditions_thresholds","max_power");
	static float _cBleeding			= pSettings->r_float("tutorial_conditions_thresholds","bleeding");
	static float _cSatiety			= pSettings->r_float("tutorial_conditions_thresholds","satiety");
	static float _cThirst			= READ_IF_EXISTS(pSettings, r_float, "tutorial_conditions_thresholds", "thirst", 0.0f);
	static float _cIntoxication		= READ_IF_EXISTS(pSettings, r_float, "tutorial_conditions_thresholds", "intoxication", 0.0f);
	static float _cRadiation		= pSettings->r_float("tutorial_conditions_thresholds","radiation");
	static float _cWpnCondition		= pSettings->r_float("tutorial_conditions_thresholds","weapon_jammed");
	static float _cPsyHealthThr		= pSettings->r_float("tutorial_conditions_thresholds","psy_health");
	static float _cSleepeness		= READ_IF_EXISTS(pSettings, r_float, "tutorial_conditions_thresholds", "sleepeness", 0.0f);
	static float _cAlcoholism		= READ_IF_EXISTS(pSettings, r_float, "tutorial_conditions_thresholds", "alcoholism", 0.0f);
	static float _cHangover			= READ_IF_EXISTS(pSettings, r_float, "tutorial_conditions_thresholds", "hangover", 0.0f);
	static float _cNarcotism		= READ_IF_EXISTS(pSettings, r_float, "tutorial_conditions_thresholds", "narcotism", 0.0f);
	static float _cWithdrawal		= READ_IF_EXISTS(pSettings, r_float, "tutorial_conditions_thresholds", "withdrawal", 0.0f);
	static float _cFrostbite		= READ_IF_EXISTS(pSettings, r_float, "tutorial_conditions_thresholds", "frostbite", 0.0f);

	bool b = true;
	if(b && !m_condition_flags.test(eCriticalPowerReached) && GetPower()<_cPowerThr){
		m_condition_flags.set			(eCriticalPowerReached, TRUE);
		b=false;
		xr_strcpy(cb_name,"_G.on_actor_critical_power");
	}

	if(b && !m_condition_flags.test(eCriticalMaxPowerReached) && GetMaxPower()<_cPowerMaxThr){
		m_condition_flags.set			(eCriticalMaxPowerReached, TRUE);
		b=false;
		xr_strcpy(cb_name,"_G.on_actor_critical_max_power");
	}

	if(b && !m_condition_flags.test(eCriticalBleedingSpeed) && BleedingSpeed()>_cBleeding){
		m_condition_flags.set			(eCriticalBleedingSpeed, TRUE);
		b=false;
		xr_strcpy(cb_name,"_G.on_actor_bleeding");
	}

	if(b && !m_condition_flags.test(eCriticalSatietyReached) && GetSatiety()<_cSatiety){
		m_condition_flags.set			(eCriticalSatietyReached, TRUE);
		b=false;
		xr_strcpy(cb_name,"_G.on_actor_satiety");
	}

	if (b && !m_condition_flags.test(eCriticalThirstReached) && GetThirst()<_cThirst && (!fis_zero(m_fV_Thirst)))
	{
		m_condition_flags.set(eCriticalThirstReached, TRUE);
		b = false;
		xr_strcpy(cb_name, "_G.on_actor_thirst");
	}

	if (b && !m_condition_flags.test(eCriticalIntoxicationReached) && GetIntoxication() > _cIntoxication && (!fis_zero(m_fV_Intoxication))) {
		m_condition_flags.set(eCriticalIntoxicationReached, TRUE);
		b = false;
		xr_strcpy(cb_name, "_G.on_actor_intoxication");
	}

	if (b && !m_condition_flags.test(eCriticalSleepenessReached) && GetSleepeness() >= _cSleepeness && (!fis_zero(m_fV_Sleepeness))) {
		m_condition_flags.set(eCriticalSleepenessReached, TRUE);
		b = false;
		xr_strcpy(cb_name, "_G.on_actor_sleepeness");
	}

	if (b && !m_condition_flags.test(eCriticalAlcoholismReached) && GetAlcoholism() > _cAlcoholism && (!fis_zero(m_fV_Alcoholism))) {
		m_condition_flags.set(eCriticalAlcoholismReached, TRUE);
		b = false;
		xr_strcpy(cb_name, "_G.on_actor_alcoholism");
	}

	if (b && !m_condition_flags.test(eCriticalNarcotismReached) && GetNarcotism() > _cNarcotism && (!fis_zero(m_fV_Narcotism))) {
		m_condition_flags.set(eCriticalNarcotismReached, TRUE);
		b = false;
		xr_strcpy(cb_name, "_G.on_actor_narcotism");
	}

	if (b && !m_condition_flags.test(eCriticalWithdrawalReached) && GetWithdrawal() > _cWithdrawal && (!fis_zero(m_fV_Withdrawal))) {
		m_condition_flags.set(eCriticalWithdrawalReached, TRUE);
		b = false;
		xr_strcpy(cb_name, "_G.on_actor_withdrawal");
	}

	if (b && !m_condition_flags.test(eCriticalHangoverReached) && GetHangover() > _cHangover && (!fis_zero(m_fV_Hangover))) {
		m_condition_flags.set(eCriticalHangoverReached, TRUE);
		b = false;
		xr_strcpy(cb_name, "_G.on_actor_hangover");
	}

	if(b && !m_condition_flags.test(eCriticalRadiationReached) && GetRadiation()>_cRadiation){
		m_condition_flags.set			(eCriticalRadiationReached, TRUE);
		b=false;
		xr_strcpy(cb_name,"_G.on_actor_radiation");
	}
	if(b && !m_condition_flags.test(ePsyHealthMinReached) && GetPsyHealth()<_cPsyHealthThr){
		m_condition_flags.set			(ePsyHealthMinReached, TRUE);
		b=false;
		xr_strcpy(cb_name,"_G.on_actor_psy");
	}

	if (b && !m_condition_flags.test(eCriticalFrostbiteReached) && GetFrostbite() > _cFrostbite && (!fis_zero(m_fV_Frostbite))) {
		m_condition_flags.set			(eCriticalFrostbiteReached, TRUE);
		b = false;
		xr_strcpy(cb_name, "_G.on_actor_frostbite");
	}

	if(b && m_condition_flags.test(eCantWalkWeight) && !m_condition_flags.test(eCantWalkWeightReached)){
		m_condition_flags.set			(eCantWalkWeightReached, TRUE);
		b=false;
		xr_strcpy(cb_name,"_G.on_actor_cant_walk_weight");
	}

	if(b && !m_condition_flags.test(eWeaponJammedReached)&&m_object->inventory().GetActiveSlot()!=NO_ACTIVE_SLOT){
		PIItem item							= m_object->inventory().ItemFromSlot(m_object->inventory().GetActiveSlot());
		CWeapon* pWeapon					= smart_cast<CWeapon*>(item); 
		if(pWeapon&&pWeapon->GetCondition()<_cWpnCondition){
			m_condition_flags.set			(eWeaponJammedReached, TRUE);b=false;
			xr_strcpy(cb_name,"_G.on_actor_weapon_jammed");
		}
	}
	
	if(!b){
		luabind::functor<LPCSTR>			fl;
		R_ASSERT							(ai().script_engine().functor<LPCSTR>(cb_name,fl));
		fl									();
	}
}

bool CActorCondition::DisableSprint(SHit* pHDS)
{
	return	(pHDS->hit_type != ALife::eHitTypeTelepatic)	&& 
			(pHDS->hit_type != ALife::eHitTypeChemicalBurn)	&&
			(pHDS->hit_type != ALife::eHitTypeBurn)			&&
			(pHDS->hit_type != ALife::eHitTypeRadiation)	;
}

bool CActorCondition::PlayHitSound(SHit* pHDS)
{
	switch (pHDS->hit_type)
	{
		case ALife::eHitTypeTelepatic:
			return false;
			break;
		case ALife::eHitTypeShock:
		case ALife::eHitTypeStrike:
		case ALife::eHitTypeWound:
		case ALife::eHitTypeExplosion:
		case ALife::eHitTypeFireWound:
		case ALife::eHitTypeWound_2:
		case ALife::eHitTypePhysicStrike:
			return true;
			break;

		case ALife::eHitTypeRadiation:
		case ALife::eHitTypeBurn:
		case ALife::eHitTypeChemicalBurn:
			return (pHDS->damage()>0.017f); //field zone threshold
			break;
		default:
			return true;
	}
}

float CActorCondition::HitSlowmo(SHit* pHDS)
{
	float ret;
	if(pHDS->hit_type==ALife::eHitTypeWound || pHDS->hit_type==ALife::eHitTypeStrike )
	{
		ret						= pHDS->damage();
		clamp					(ret,0.0f,1.f);
	}else
		ret						= 0.0f;

	return ret;	
}

bool CActorCondition::ApplyInfluence(const SMedicineInfluenceValues& V, const shared_str& sect, CEatableItem* cur_eatable)
{
	if(m_curr_medicine_influence.InProcess())
		return false;

	if(V.fTimeTotal<0.0f)
		return inherited::ApplyInfluence	(V, sect, cur_eatable);

	m_curr_medicine_influence				= V;
	m_curr_medicine_influence.fTimeCurrent  = m_curr_medicine_influence.fTimeTotal;
	return true;
}
bool CActorCondition::ApplyBooster(const SBooster& B, const shared_str& sect)
{
	if(!fis_zero(B.fBoostValue))
	{
		if (m_object->Local() && m_object == Level().CurrentViewEntity())
		{
			if(pSettings->line_exist(sect, "use_sound"))
			{
				if(m_use_sound._feedback())
					m_use_sound.stop		();
				shared_str snd_name			= pSettings->r_string(sect, "use_sound");
				m_use_sound.create			(snd_name.c_str(), st_Effect, sg_SourceType);
				m_use_sound.play			(NULL, sm_2D);
			}
		}

		BOOSTER_MAP::iterator it = m_booster_influences.find(B.m_type);
		if(it!=m_booster_influences.end())
			DisableBoostParameters((*it).second);
		m_booster_influences[B.m_type] = B;
		BoostParameters(B);
	}
	return true;
}

void disable_input();
void enable_input();
void hide_indicators();
void show_indicators();

CActorDeathEffector::CActorDeathEffector	(CActorCondition* parent, LPCSTR sect)	// -((
:m_pParent(parent)
{
	Actor()->SetWeaponHideState(INV_STATE_BLOCK_ALL,true);
	hide_indicators			();

	AddEffector				(Actor(), effActorDeath, sect);
	disable_input			();
	LPCSTR snd				= pSettings->r_string(sect, "snd");
	m_death_sound.create	(snd,st_Effect,0);
	m_death_sound.play_at_pos(0,Fvector().set(0,0,0),sm_2D);


	SBaseEffector* pe		= Actor()->Cameras().GetPPEffector((EEffectorPPType)effActorDeath);
	pe->m_on_b_remove_callback = SBaseEffector::CB_ON_B_REMOVE(this, &CActorDeathEffector::OnPPEffectorReleased);
	m_b_actual				= true;	
	m_start_health			= m_pParent->health();
}

CActorDeathEffector::~CActorDeathEffector()
{}

void CActorDeathEffector::UpdateCL()
{
	m_pParent->SetHealth( m_start_health );
}

void CActorDeathEffector::OnPPEffectorReleased()
{
	m_b_actual				= false;	
	Msg("111");
	//m_pParent->health()		= -1.0f;
	m_pParent->SetHealth		(-1.0f);
}

void CActorDeathEffector::Stop()
{
	RemoveEffector			(Actor(),effActorDeath);
	m_death_sound.destroy	();
	enable_input			();
	show_indicators			();
}
