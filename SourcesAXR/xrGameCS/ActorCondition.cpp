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

#include "AdvancedXrayGameConstants.h"
#include "CustomOutfit.h"

#define MAX_SATIETY					1.0f
#define START_SATIETY				0.5f
#define MAX_THIRST					1.0f
#define START_THIRST				0.5f

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
	m_fDrugs					= 0.f;

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

//. ???	m_fSatietyCritical			= pSettings->r_float(section,"satiety_critical");
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
	m_fV_Thirst					= pSettings->r_float(section, "thirst_v");
	m_fV_ThirstPower			= pSettings->r_float(section, "thirst_power_v");
	m_fV_ThirstHealth			= pSettings->r_float(section, "thirst_health_v");


	// M.F.S. Team Intoxication
	m_fIntoxicationCritical = pSettings->r_float(section, "intoxication_critical");
	clamp(m_fIntoxicationCritical, 0.0f, 1.0f);
	m_fV_Intoxication = pSettings->r_float(section, "intoxication_v");
	m_fV_IntoxicationHealth = pSettings->r_float(section, "intoxication_health_v");

	// M.F.S. Team Sleepeness
	m_fSleepenessCritical = pSettings->r_float(section, "sleepeness_critical");
	clamp(m_fSleepenessCritical, 0.0f, 1.0f);
	m_fV_Sleepeness = pSettings->r_float(section, "sleepeness_v");
	m_fV_SleepenessPower = pSettings->r_float(section, "sleepeness_power_v");
	m_fSleepeness_V_Sleep = pSettings->r_float(section, "sleepeness_v_sleep");

	// M.F.S. Team Alcoholism (History Of Puhtinskyi)
	m_fV_Alcoholism = pSettings->r_float(section, "alcoholism_v");
	m_fHangoverCritical = pSettings->r_float(section, "hangover_critical");
	clamp(m_fHangoverCritical, 0.0f, 1.0f);
	m_fV_Hangover = pSettings->r_float(section, "hangover_v");
	m_fV_HangoverPower = pSettings->r_float(section, "hangover_power_v");

	// M.F.S. Team Narcotism (History Of Puhtinskyi)
	m_fV_Narcotism = pSettings->r_float(section, "narcotism_v");
	m_fWithdrawalCritical = pSettings->r_float(section, "withdrawal_critical");
	clamp(m_fWithdrawalCritical, 0.0f, 1.0f);
	m_fV_Withdrawal = pSettings->r_float(section, "withdrawal_v");
	m_fV_WithdrawalPower = pSettings->r_float(section, "withdrawal_power_v");
	m_fV_WithdrawalHealth = pSettings->r_float(section, "withdrawal_health_v");
	m_fV_Drugs = pSettings->r_float(section, "drugs_v");

	// M.F.S. Team Skills System
	m_fV_SatietySkill = READ_IF_EXISTS(pSettings, r_float, "skills_influence", "skills_satiety_restore", 0.0f);
	m_fV_HealthSkill = READ_IF_EXISTS(pSettings, r_float, "skills_influence", "skills_health_restore", 0.0f);
	m_fV_BleedingSkill = READ_IF_EXISTS(pSettings, r_float, "skills_influence", "skills_bleeding_restore", 0.0f);
	m_fV_RadiationSkill = READ_IF_EXISTS(pSettings, r_float, "skills_influence", "skills_radiation_restore", 0.0f);
	m_fV_PowerSkill = READ_IF_EXISTS(pSettings, r_float, "skills_influence", "skills_power_restore", 0.0f);
	m_fV_ThirstSkill = READ_IF_EXISTS(pSettings, r_float, "skills_influence", "skills_thirst_restore", 0.0f);
	m_fV_IntoxicationSkill = READ_IF_EXISTS(pSettings, r_float, "skills_influence", "skills_intoxication_restore", 0.0f);
	m_fV_SleepenessSkill = READ_IF_EXISTS(pSettings, r_float, "skills_influence", "skills_sleepeness_restore", 0.0f);

	m_fMaxWeightSkill = READ_IF_EXISTS(pSettings, r_float, "skills_influence", "skills_max_weight", 0.0f);
	m_fJumpSpeedSkill = READ_IF_EXISTS(pSettings, r_float, "skills_influence", "skills_jump_speed", 0.0f);
	m_fWalkAccelSkill = READ_IF_EXISTS(pSettings, r_float, "skills_influence", "skills_walk_accel", 0.0f);
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
		NODEFAULT;
	}
	
	return GetZoneMaxPower( iz_type );
}


//вычисление параметров с ходом времени
#include "UI.h"
#include "HUDManager.h"

void CActorCondition::UpdateCondition()
{
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
		
		string512			pp_sect_name;
		shared_str ln		= Level().name();
		if(ln.size())
		{
			CEffectorPP* ppe	= object().Cameras().GetPPEffector((EEffectorPPType)effPsyHealth);
			

			strconcat			(sizeof(pp_sect_name),pp_sect_name, "effector_psy_health", "_", *ln);
			if(!pSettings->section_exist(pp_sect_name))
				strcpy_s			(pp_sect_name, "effector_psy_health");

			if	( !fsimilar(GetPsyHealth(), 1.0f, 0.05f) )
			{
				if(!ppe)
				{
					AddEffector(m_object,effPsyHealth, pp_sect_name, GET_KOEFF_FUNC(this, &CActorCondition::GetPsy));
				}
			}else
			{
				if(ppe)
					RemoveEffector(m_object,effPsyHealth);
			}
		}
//-		if(fis_zero(GetPsyHealth()))
//-			SetHealth( 0.0f );
	};

	UpdateSatiety				();

	if (GameConstants::GetActorThirst())
	{
		UpdateThirst();
	}

	if (GameConstants::GetActorIntoxication())
	{
		UpdateIntoxication();
	}

	if (GameConstants::GetActorSleepeness())
	{
		UpdateSleepeness();
	}

	if (GameConstants::GetActorAlcoholism())
	{
		UpdateAlcoholism();
	}

	if (GameConstants::GetActorNarcotism())
	{
		UpdateNarcotism();
	}

	inherited::UpdateCondition	();

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

	AffectDamage_InjuriousMaterial();

	/*if(m_fDeltaTime > 0.0f)
	{
		float inj_material_damage = GetInjuriousMaterialDamage();
		if(inj_material_damage>0)
		{
			inj_material_damage	*= (m_fDeltaTime / Level().GetGameTimeFactor());
			SHit	HDS = SHit(inj_material_damage,0.0f,Fvector().set(0,1,0),NULL,BI_NONE,Fvector().set(0,0,0), 0.f, ALife::eHitTypeRadiation);

			HDS.GenHeader(GE_HIT, m_object->ID());

			NET_Packet			np;
			HDS.Write_Packet	(np);
			
			CGameObject::u_EventSend(np);
			m_object->Hit(&HDS);
		}
	}*/
}

void CActorCondition::AffectDamage_InjuriousMaterial()
{
	float one = 0.1f;
	float tg  = Device.fTimeGlobal;

	float damage = GetInjuriousMaterialDamage();
	if ( damage < EPS )
	{
		m_f_time_affected = tg;
		return;
	}

	if ( m_f_time_affected + one > tg )
	{
		return;
	}
	clamp( m_f_time_affected, tg - (one * 3), tg );
	
	damage *= one;

	NET_Packet	np;

	while ( m_f_time_affected + one < tg )
	{
		m_f_time_affected += one;

		SHit HDS = SHit( damage, 0.0f, Fvector().set(0,1,0), NULL, BI_NONE, Fvector().set(0,0,0), 0.0f, ALife::eHitTypeRadiation );
///		Msg( "_____ damage = %.4f     frame=%d", damage, Device.dwFrame );

		HDS.GenHeader(GE_HIT, m_object->ID());

		HDS.Write_Packet( np );
		CGameObject::u_EventSend( np );

		m_object->Hit(&HDS);
	
	}//while
}

#include "characterphysicssupport.h"
float CActorCondition::GetInjuriousMaterialDamage()
{
	u16 mat_injurios = m_object->character_physics_support()->movement()->injurious_material_idx();

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
	if (!IsGameTypeSingle()) return;

	float k = 1.0f;
	if(m_fSatiety>0)
	{
		m_fSatiety -=	m_fV_Satiety*
						k*
						m_fDeltaTime;
	
		clamp			(m_fSatiety,		0.0f,		1.0f);

	}
		
	//сытость увеличивает здоровье только если нет открытых ран
	if(!m_bIsBleeding)
	{
		m_fDeltaHealth += CanBeHarmed() ? 
					(m_fV_SatietyHealth*(m_fSatiety>0.0f?1.f:-1.f)*m_fDeltaTime)
					: 0;
	}

	//коэффициенты уменьшения восстановления силы от сытоти и радиации
	float radiation_power_k		= 1.f;
	float satiety_power_k		= 1.f;
			
	m_fDeltaPower += m_fV_SatietyPower*
				radiation_power_k*
				satiety_power_k*
				m_fDeltaTime;
}

//M.F.S. Team Thirst
void CActorCondition::UpdateThirst()
{
	if (!IsGameTypeSingle()) return;

	float k = 1.0f;
	if (m_fThirst > 0)
	{
		m_fThirst -= m_fV_Thirst *
			k*
			m_fDeltaTime;

		clamp(m_fThirst, 0.0f, 1.0f);

	}

	//жажда увеличивает здоровье только если нет открытых ран
	if (!m_bIsBleeding)
	{
		m_fDeltaHealth += CanBeHarmed() ?
			(m_fV_ThirstHealth*(m_fThirst > 0.0f ? 1.f : -1.f)*m_fDeltaTime)
			: 0;
	}

	//коэффициенты уменьшения восстановления силы от жажды
	float thirst_power_k = 1.f;

	m_fDeltaPower += m_fV_ThirstPower *
		thirst_power_k*
		m_fDeltaTime;
}

//M.F.S. Team Intoxication
void CActorCondition::UpdateIntoxication()
{
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
	if (GetSleepeness() >= 0.85f)
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
		if (!ce)
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
			m_fDeltaPower -= m_fV_SleepenessPower * m_fSleepeness * m_fDeltaTime;
	}
}

//M.F.S. Team Alcoholism
void CActorCondition::UpdateAlcoholism()
{
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

CWound* CActorCondition::ConditionHit(SHit* pHDS)
{
	if (GodMode()) return NULL;
	return inherited::ConditionHit(pHDS);
}

//weight - "удельный" вес от 0..1
void CActorCondition::ConditionJump(float weight)
{
	float power			=	m_fJumpPower;
	power				+=	m_fJumpWeightPower*weight*(weight>1.f?m_fOverweightJumpK:1.f);
	m_fPower			-=	HitPowerEffect(power);
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
	m_fPower -= HitPowerEffect(power);
}

void CActorCondition::ConditionStand(float weight)
{	
	float power			= m_fStandPower;
	power				*= m_fDeltaTime;
	m_fPower			-= power;
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
	m_fHangover += value;
	clamp(m_fWithdrawal, 0.0f, 3.0f);
}

void CActorCondition::ChangeDrugs(float value)
{
	m_fDrugs += value;
}

void CActorCondition::UpdateTutorialThresholds()
{
	string256						cb_name;
	static float _cPowerThr			= pSettings->r_float("tutorial_conditions_thresholds","power");
	static float _cPowerMaxThr		= pSettings->r_float("tutorial_conditions_thresholds","max_power");
	static float _cBleeding			= pSettings->r_float("tutorial_conditions_thresholds","bleeding");
	static float _cSatiety			= pSettings->r_float("tutorial_conditions_thresholds","satiety");
	static float _cThirst			= pSettings->r_float("tutorial_conditions_thresholds","thirst");
	static float _cRadiation		= pSettings->r_float("tutorial_conditions_thresholds","radiation");
	static float _cWpnCondition		= pSettings->r_float("tutorial_conditions_thresholds","weapon_jammed");
	static float _cPsyHealthThr		= pSettings->r_float("tutorial_conditions_thresholds","psy_health");
	static float _cIntoxication		= pSettings->r_float("tutorial_conditions_thresholds", "intoxication");
	static float _cSleepeness		= pSettings->r_float("tutorial_conditions_thresholds", "sleepeness");
	static float _cAlcoholism		= pSettings->r_float("tutorial_conditions_thresholds", "alcoholism");
	static float _cHangover			= pSettings->r_float("tutorial_conditions_thresholds", "hangover");
	static float _cNarcotism		= pSettings->r_float("tutorial_conditions_thresholds", "narcotism");
	static float _cWithdrawal		= pSettings->r_float("tutorial_conditions_thresholds", "withdrawal");

	bool b = true;
	if(b && !m_condition_flags.test(eCriticalPowerReached) && GetPower()<_cPowerThr){
		m_condition_flags.set			(eCriticalPowerReached, TRUE);
		b=false;
		strcpy_s(cb_name,"_G.on_actor_critical_power");
	}

	if(b && !m_condition_flags.test(eCriticalMaxPowerReached) && GetMaxPower()<_cPowerMaxThr){
		m_condition_flags.set			(eCriticalMaxPowerReached, TRUE);
		b=false;
		strcpy_s(cb_name,"_G.on_actor_critical_max_power");
	}

	if(b && !m_condition_flags.test(eCriticalBleedingSpeed) && BleedingSpeed()>_cBleeding){
		m_condition_flags.set			(eCriticalBleedingSpeed, TRUE);
		b=false;
		strcpy_s(cb_name,"_G.on_actor_bleeding");
	}

	if(b && !m_condition_flags.test(eCriticalSatietyReached) && GetSatiety()<_cSatiety){
		m_condition_flags.set			(eCriticalSatietyReached, TRUE);
		b=false;
		strcpy_s(cb_name,"_G.on_actor_satiety");
	}

	if (b && !m_condition_flags.test(eCriticalThirstReached) && GetThirst()<_cThirst)
	{
		m_condition_flags.set(eCriticalThirstReached, TRUE);
		b = false;
		xr_strcpy(cb_name, "_G.on_actor_thirst");
	}

	if (b && !m_condition_flags.test(eCriticalIntoxicationReached) && GetIntoxication() > _cIntoxication) {
		m_condition_flags.set(eCriticalIntoxicationReached, TRUE);
		b = false;
		xr_strcpy(cb_name, "_G.on_actor_intoxication");
	}

	if (b && !m_condition_flags.test(eCriticalSleepenessReached) && GetSleepeness() >= _cSleepeness) {
		m_condition_flags.set(eCriticalSleepenessReached, TRUE);
		b = false;
		xr_strcpy(cb_name, "_G.on_actor_sleepeness");
	}

	if (b && !m_condition_flags.test(eCriticalAlcoholismReached) && GetAlcoholism() > _cAlcoholism) {
		m_condition_flags.set(eCriticalAlcoholismReached, TRUE);
		b = false;
		xr_strcpy(cb_name, "_G.on_actor_alcoholism");
	}

	if (b && !m_condition_flags.test(eCriticalNarcotismReached) && GetNarcotism() > _cNarcotism) {
		m_condition_flags.set(eCriticalNarcotismReached, TRUE);
		b = false;
		xr_strcpy(cb_name, "_G.on_actor_narcotism");
	}

	if (b && !m_condition_flags.test(eCriticalWithdrawalReached) && GetWithdrawal() > _cWithdrawal) {
		m_condition_flags.set(eCriticalWithdrawalReached, TRUE);
		b = false;
		xr_strcpy(cb_name, "_G.on_actor_withdrawal");
	}

	if (b && !m_condition_flags.test(eCriticalHangoverReached) && GetHangover() > _cHangover) {
		m_condition_flags.set(eCriticalHangoverReached, TRUE);
		b = false;
		xr_strcpy(cb_name, "_G.on_actor_hangover");
	}

	if(b && !m_condition_flags.test(eCriticalRadiationReached) && GetRadiation()>_cRadiation){
		m_condition_flags.set			(eCriticalRadiationReached, TRUE);
		b=false;
		strcpy_s(cb_name,"_G.on_actor_radiation");
	}

	if(b && !m_condition_flags.test(ePhyHealthMinReached) && GetPsyHealth()>_cPsyHealthThr){
//.		m_condition_flags.set			(ePhyHealthMinReached, TRUE);
		b=false;
		strcpy_s(cb_name,"_G.on_actor_psy");
	}

	if(b && !m_condition_flags.test(eCantWalkWeight)){
//.		m_condition_flags.set			(eCantWalkWeight, TRUE);
		b=false;
		strcpy_s(cb_name,"_G.on_actor_cant_walk_weight");
	}

	if(b && !m_condition_flags.test(eWeaponJammedReached)&&m_object->inventory().GetActiveSlot()!=NO_ACTIVE_SLOT){
		PIItem item							= m_object->inventory().ItemFromSlot(m_object->inventory().GetActiveSlot());
		CWeapon* pWeapon					= smart_cast<CWeapon*>(item); 
		if(pWeapon&&pWeapon->GetCondition()<_cWpnCondition){
			m_condition_flags.set			(eWeaponJammedReached, TRUE);b=false;
			strcpy_s(cb_name,"_G.on_actor_weapon_jammed");
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
