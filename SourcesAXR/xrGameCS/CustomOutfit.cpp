#include "stdafx.h"

#include "customoutfit.h"
#include "PhysicsShell.h"
#include "inventory_space.h"
#include "Inventory.h"
#include "Actor.h"
#include "game_cl_base.h"
#include "Level.h"
#include "BoneProtections.h"
#include "../Include/xrRender/Kinematics.h"
#include "player_hud.h"
#include "DynamicHudGlass.h"
#include "AdvancedXrayGameConstants.h"
#include "AntigasFilter.h"
#include "ui/UIMainIngameWnd.h"
#include "ui/UIHudStatesWnd.h"
#include "hudmanager.h"
#include "UIGameCustom.h"

CCustomOutfit::CCustomOutfit()
{
	m_flags.set(FUsingCondition, TRUE);

	for(int i=0; i<ALife::eHitTypeMax; i++)
		m_HitTypeProtection[(ALife::EHitType)i] = 1.0f;

	m_boneProtection = xr_new<SBoneProtections>();
	m_artefact_count = 0;
	m_BonesProtectionSect = nullptr;
	m_bUseFilter = false;
	m_b_HasGlass = false;
	m_bHasLSS = false;
	m_NightVisionType = 0;
	m_fFilterDegradation = 0.0f;
	m_fFilterCondition = 1.0f;
}

CCustomOutfit::~CCustomOutfit() 
{
	xr_delete(m_boneProtection);
}

BOOL CCustomOutfit::net_Spawn(CSE_Abstract* DC)
{
	BOOL res = inherited::net_Spawn(DC);
	return					(res);
}

void CCustomOutfit::net_Export(NET_Packet& P)
{
	inherited::net_Export	(P);
	P.w_float_q8			(GetCondition(),0.0f,1.0f);
}

void CCustomOutfit::net_Import(NET_Packet& P)
{
	inherited::net_Import	(P);
	float _cond;
	P.r_float_q8			(_cond,0.0f,1.0f);
	SetCondition			(_cond);
}

void CCustomOutfit::save(NET_Packet& output_packet)
{
	inherited::save(output_packet);
	save_data(m_fFilterCondition, output_packet);

}

void CCustomOutfit::load(IReader& input_packet)
{
	inherited::load(input_packet);
	load_data(m_fFilterCondition, input_packet);
}

void CCustomOutfit::Load(LPCSTR section) 
{
	inherited::Load(section);

	m_ConstHitTypeProtection[ALife::eHitTypeRadiation] = pSettings->r_float(section, "radiation_protection");
	m_ConstHitTypeProtection[ALife::eHitTypeChemicalBurn] = pSettings->r_float(section, "chemical_burn_protection");

	m_HitTypeProtection[ALife::eHitTypeBurn]		= pSettings->r_float(section,"burn_protection");
	m_HitTypeProtection[ALife::eHitTypeStrike]		= pSettings->r_float(section,"strike_protection");
	m_HitTypeProtection[ALife::eHitTypeShock]		= pSettings->r_float(section,"shock_protection");
	m_HitTypeProtection[ALife::eHitTypeWound]		= pSettings->r_float(section,"wound_protection");
	m_HitTypeProtection[ALife::eHitTypeRadiation]	= m_ConstHitTypeProtection[ALife::eHitTypeRadiation];
	m_HitTypeProtection[ALife::eHitTypeTelepatic]	= pSettings->r_float(section,"telepatic_protection");
	m_HitTypeProtection[ALife::eHitTypeChemicalBurn] = m_ConstHitTypeProtection[ALife::eHitTypeChemicalBurn];
	m_HitTypeProtection[ALife::eHitTypeExplosion]	= pSettings->r_float(section,"explosion_protection");
	m_HitTypeProtection[ALife::eHitTypeFireWound]	= pSettings->r_float(section,"fire_wound_protection");
	m_HitTypeProtection[ALife::eHitTypePhysicStrike]= READ_IF_EXISTS(pSettings, r_float, section, "physic_strike_protection", 0.0f);

	m_boneProtection->m_fHitFracActor = pSettings->r_float(section, "hit_fraction_actor");

	if (pSettings->line_exist(section, "actor_visual"))
		m_ActorVisual = pSettings->r_string(section, "actor_visual");
	else
		m_ActorVisual = NULL;

	m_ef_equipment_type		= pSettings->r_u32(section,"ef_equipment_type");
	if ( pSettings->line_exist(section, "power_loss") )
	{
		m_fPowerLoss = pSettings->r_float(section, "power_loss");
		clamp( m_fPowerLoss, 0.0f, 1.0f );
	}
	else
	{
		m_fPowerLoss = 1.0f;
	}

	m_additional_weight				= pSettings->r_float(section,"additional_inventory_weight");
	m_additional_weight2			= pSettings->r_float(section,"additional_inventory_weight2");

	m_fHealthRestoreSpeed		= READ_IF_EXISTS(pSettings, r_float, section, "health_restore_speed",    0.0f );
	m_fRadiationRestoreSpeed	= READ_IF_EXISTS(pSettings, r_float, section, "radiation_restore_speed", 0.0f );
	m_fSatietyRestoreSpeed		= READ_IF_EXISTS(pSettings, r_float, section, "satiety_restore_speed",   0.0f );
	m_fPowerRestoreSpeed		= READ_IF_EXISTS(pSettings, r_float, section, "power_restore_speed",     0.0f );
	m_fBleedingRestoreSpeed		= READ_IF_EXISTS(pSettings, r_float, section, "bleeding_restore_speed",  0.0f );
	m_fThirstRestoreSpeed		= READ_IF_EXISTS(pSettings, r_float, section, "thirst_restore_speed",	 0.0f );
	m_fIntoxicationRestoreSpeed = READ_IF_EXISTS(pSettings, r_float, section, "intoxication_restore_speed", 0.0f);
	m_fSleepenessRestoreSpeed	= READ_IF_EXISTS(pSettings, r_float, section, "sleepeness_restore_speed", 0.0f);
	m_fAlcoholismRestoreSpeed	= READ_IF_EXISTS(pSettings, r_float, section, "alcoholism_restore_speed", 0.0f);
	m_fNarcotismRestoreSpeed	= READ_IF_EXISTS(pSettings, r_float, section, "narcotism_restore_speed", 0.0f);
	m_fPsyHealthRestoreSpeed	= READ_IF_EXISTS(pSettings, r_float, section, "psy_health_restore_speed", 0.0f);

	m_fJumpSpeed				= READ_IF_EXISTS(pSettings, r_float, section, "jump_speed", 1.f);
	m_fWalkAccel				= READ_IF_EXISTS(pSettings, r_float, section, "walk_accel", 1.f);
	m_fOverweightWalkK			= READ_IF_EXISTS(pSettings, r_float, section, "overweight_walk_k", 1.f);

	m_fFilterDegradation		= READ_IF_EXISTS(pSettings, r_float, section, "filter_degradation_speed", 0.0f);
	m_fMaxFilterCondition		= READ_IF_EXISTS(pSettings, r_float, section, "max_filter_condition", 1.0f);

	m_b_HasGlass				= !!READ_IF_EXISTS(pSettings, r_bool, section, "has_glass", FALSE);
	m_bUseFilter				= READ_IF_EXISTS(pSettings, r_bool, section, "use_filter", false);
	m_bHasLSS					= READ_IF_EXISTS(pSettings, r_bool, section, "has_ls_system", false);
	m_NightVisionType			= READ_IF_EXISTS(pSettings, r_u32, section, "night_vision_type", 0);

	if (pSettings->line_exist(section, "nightvision_sect"))
		m_NightVisionSect = pSettings->r_string(section, "nightvision_sect");
	else
		m_NightVisionSect = NULL;

	m_SuitableFilters.clear();
	m_SuitableRepairKits.clear();
	LPCSTR filters = READ_IF_EXISTS(pSettings, r_string, section, "suitable_filters", "antigas_filter");
	LPCSTR repair_kits = READ_IF_EXISTS(pSettings, r_string, section, "suitable_repair_kits", "repair_kit");

	m_PlayerHudSection = READ_IF_EXISTS(pSettings, r_string, section, "player_hud_section", "actor_hud");

	// Added by Axel, to enable optional condition use on any item
	m_flags.set(FUsingCondition, READ_IF_EXISTS(pSettings, r_bool, section, "use_condition", true));

	if (filters && filters[0])
	{
		string128 filter_sect;
		int count = _GetItemCount(filters);
		for (int it = 0; it < count; ++it)
		{
			_GetItem(filters, it, filter_sect);
			m_SuitableFilters.push_back(filter_sect);
		}
	}

	if (repair_kits && repair_kits[0])
	{
		string128 repair_kits_sect;
		int count = _GetItemCount(repair_kits);
		for (int it = 0; it < count; ++it)
		{
			_GetItem(repair_kits, it, repair_kits_sect);
			m_SuitableRepairKits.push_back(repair_kits_sect);
		}
	}

	m_full_icon_name	= pSettings->r_string( section, "full_icon_name" );
	m_artefact_count 	= READ_IF_EXISTS( pSettings, r_u32, section, "artefact_count", 0 );
	clamp( m_artefact_count, (u32)0, (u32)GameConstants::GetArtefactsCount());

	if ( pSettings->line_exist( cNameSect(), "bones_koeff_protection") )
	{
		m_BonesProtectionSect._set( pSettings->r_string( cNameSect(), "bones_koeff_protection" ) );
	}
	CActor* pActor = smart_cast<CActor*>( Level().CurrentViewEntity() );
	ReloadBonesProtection( pActor );

	if (GameConstants::GetOutfitUseFilters())
	{
		float rnd_cond = ::Random.randF(0.0f, m_fMaxFilterCondition);
		m_fFilterCondition = rnd_cond;
	}
}

void CCustomOutfit::UpdateCL()
{
	inherited::UpdateCL();

	if (Actor())
		DynamicHudGlass::UpdateDynamicHudGlass();

	if (GameConstants::GetOutfitUseFilters())
		UpdateFilterCondition();
}

void CCustomOutfit::UpdateFilterCondition(void)
{
	CCustomOutfit* outfit = smart_cast<CCustomOutfit*>(Actor()->inventory().ItemFromSlot(OUTFIT_SLOT));

	if (outfit && m_bUseFilter)
	{
		float m_radia_hit = HUD().GetUI()->UIGame()->get_zone_cur_power(ALife::eHitTypeRadiation) * 4;
		float m_chemical_hit = HUD().GetUI()->UIGame()->get_zone_cur_power(ALife::eHitTypeChemicalBurn);
		float uncharge_coef = ((m_fFilterDegradation + m_radia_hit + m_chemical_hit) / 16) * Device.fTimeDelta;

		m_fFilterCondition -= uncharge_coef;
		clamp(m_fFilterCondition, 0.0f, m_fMaxFilterCondition);

		float condition = 1.f * m_fFilterCondition;
		float percent = outfit->m_fFilterCondition * 100;

		if (percent > 20.0f)
		{
			if (!fis_zero(outfit->m_HitTypeProtection[ALife::eHitTypeRadiation]) && !fis_zero(m_ConstHitTypeProtection[ALife::eHitTypeRadiation]))
				outfit->m_HitTypeProtection[ALife::eHitTypeRadiation] = (m_ConstHitTypeProtection[ALife::eHitTypeRadiation] / 100) * percent;
			if (!fis_zero(outfit->m_HitTypeProtection[ALife::eHitTypeChemicalBurn]) && !fis_zero(m_ConstHitTypeProtection[ALife::eHitTypeChemicalBurn]))
				outfit->m_HitTypeProtection[ALife::eHitTypeChemicalBurn] = (m_ConstHitTypeProtection[ALife::eHitTypeChemicalBurn] / 100) * percent;
		}
		else
		{
			if (!fis_zero(outfit->m_HitTypeProtection[ALife::eHitTypeRadiation]) && !fis_zero(m_ConstHitTypeProtection[ALife::eHitTypeRadiation]))
				outfit->m_HitTypeProtection[ALife::eHitTypeRadiation] = (m_ConstHitTypeProtection[ALife::eHitTypeRadiation] / 100) * 20.0f;
			if (!fis_zero(outfit->m_HitTypeProtection[ALife::eHitTypeChemicalBurn]) && !fis_zero(m_ConstHitTypeProtection[ALife::eHitTypeChemicalBurn]))
				outfit->m_HitTypeProtection[ALife::eHitTypeChemicalBurn] = (m_ConstHitTypeProtection[ALife::eHitTypeChemicalBurn] / 100) * 20.0f;
		}
	}
}

void CCustomOutfit::ReloadBonesProtection( CActor* pActor )
{
	if ( !pActor || !pActor->Visual() || m_BonesProtectionSect.size() == 0 )
	{
		return;
	}
	m_boneProtection->reload( m_BonesProtectionSect, smart_cast<IKinematics*>( pActor->Visual() ) );
}

void CCustomOutfit::Hit(float hit_power, ALife::EHitType hit_type)
{
	hit_power *= m_HitTypeK[hit_type];
	ChangeCondition(-hit_power);
}

float CCustomOutfit::GetDefHitTypeProtection(ALife::EHitType hit_type)
{
	return m_HitTypeProtection[hit_type]*GetCondition();
}

float CCustomOutfit::GetHitTypeProtection(ALife::EHitType hit_type, s16 element)
{
	float fBase = m_HitTypeProtection[hit_type]*GetCondition();
	float bone = m_boneProtection->getBoneProtection(element);
	return fBase*bone;
}

float CCustomOutfit::GetBoneArmor(s16 element)
{
	return m_boneProtection->getBoneArmor(element);
}

float CCustomOutfit::HitThroughArmor( float hit_power, s16 element, float ap, bool& add_wound )
{
	float NewHitPower = hit_power;
	float BoneArmor = m_boneProtection->getBoneArmor(element)*GetCondition();

	if( ap > EPS && ap > BoneArmor )
	{
		//пуля пробила бронь
		float d_ap = ap - BoneArmor;
		NewHitPower *= ( d_ap / ap );

		if ( NewHitPower < m_boneProtection->m_fHitFracActor )
		{
			NewHitPower = m_boneProtection->m_fHitFracActor;
		}
		if ( !IsGameTypeSingle() )
		{
			NewHitPower *= m_boneProtection->getBoneProtection(element);
		}
		
		if ( NewHitPower < 0.0f ) { NewHitPower = 0.0f; }

		//увеличить изношенность костюма
		Hit( NewHitPower, ALife::eHitTypeFireWound );
	}
	else
	{
		//пуля НЕ пробила бронь
		NewHitPower *= m_boneProtection->m_fHitFracActor;
		add_wound = false; 	//раны нет
		Hit( NewHitPower, ALife::eHitTypeFireWound );
	}// if >=

	return NewHitPower;
}

BOOL	CCustomOutfit::BonePassBullet					(int boneID)
{
	return m_boneProtection->getBonePassBullet(s16(boneID));
}

void	CCustomOutfit::OnMoveToSlot		()
{
	if ( m_pInventory )
	{
		CActor* pActor = smart_cast<CActor*>( m_pInventory->GetOwner() );
		if ( pActor )
		{
			ReloadBonesProtection( pActor );
			ApplySkinModel(pActor, true, false);
		}
	}
}

void CCustomOutfit::ApplySkinModel(CActor* pActor, bool bDress, bool bHUDOnly)
{
	if(bDress)
	{
		if(!bHUDOnly && m_ActorVisual.size())
		{
			shared_str NewVisual = NULL;
			char* TeamSection = Game().getTeamSection(pActor->g_Team());
			if (TeamSection)
			{
				if (pSettings->line_exist(TeamSection, *cNameSect()))
				{
					NewVisual = pSettings->r_string(TeamSection, *cNameSect());
					string256 SkinName;

					strcpy_s(SkinName, pSettings->r_string("mp_skins_path", "skin_path"));
					strcat_s(SkinName, *NewVisual);
					strcat_s(SkinName, ".ogf");
					NewVisual._set(SkinName);
				}
			}
			if (!NewVisual.size())
				NewVisual = m_ActorVisual;

			pActor->ChangeVisual(NewVisual);
		}


		if (pActor == Level().CurrentViewEntity())	
			g_player_hud->load(m_PlayerHudSection);
	}else
	{
		if (!bHUDOnly && m_ActorVisual.size())
		{
			shared_str DefVisual	= pActor->GetDefaultVisualOutfit();
			if (DefVisual.size())
			{
				pActor->ChangeVisual(DefVisual);
			};
		}

		if (pActor == Level().CurrentViewEntity())	
			g_player_hud->load_default();
	}

}

void	CCustomOutfit::OnMoveToRuck		(EItemPlace prev)
{
	if(m_pInventory && prev == EItemPlaceSlot && !Level().is_removing_objects())
	{
		CActor* pActor = smart_cast<CActor*> (H_Parent());
		if (pActor)
		{
			ApplySkinModel(pActor, false, false);
				pActor->SwitchNightVision(false);
		}
	}
};

u32	CCustomOutfit::ef_equipment_type	() const
{
	return		(m_ef_equipment_type);
}

float CCustomOutfit::GetPowerLoss() 
{
	if (m_fPowerLoss<1 && GetCondition() <= 0)
	{
		return 1.0f;			
	};
	return m_fPowerLoss;
};

bool CCustomOutfit::install_upgrade_impl( LPCSTR section, bool test )
{
	bool result = CInventoryItemObject::install_upgrade_impl( section, test );

	result |= process_if_exists( section, "burn_protection",          &CInifile::r_float, m_HitTypeProtection[ALife::eHitTypeBurn]        , test );
	result |= process_if_exists( section, "shock_protection",         &CInifile::r_float, m_HitTypeProtection[ALife::eHitTypeShock]       , test );
	result |= process_if_exists( section, "strike_protection",        &CInifile::r_float, m_HitTypeProtection[ALife::eHitTypeStrike]      , test );
	result |= process_if_exists( section, "wound_protection",         &CInifile::r_float, m_HitTypeProtection[ALife::eHitTypeWound]       , test );
	result |= process_if_exists( section, "radiation_protection",     &CInifile::r_float, m_HitTypeProtection[ALife::eHitTypeRadiation]   , test );
	result |= process_if_exists( section, "telepatic_protection",     &CInifile::r_float, m_HitTypeProtection[ALife::eHitTypeTelepatic]   , test );
	result |= process_if_exists( section, "chemical_burn_protection", &CInifile::r_float, m_HitTypeProtection[ALife::eHitTypeChemicalBurn], test );
	result |= process_if_exists( section, "explosion_protection",     &CInifile::r_float, m_HitTypeProtection[ALife::eHitTypeExplosion]   , test );
	result |= process_if_exists( section, "fire_wound_protection",    &CInifile::r_float, m_HitTypeProtection[ALife::eHitTypeFireWound]   , test );
	result |= process_if_exists( section, "physic_strike_protection", &CInifile::r_float, m_HitTypeProtection[ALife::eHitTypePhysicStrike], test );

	LPCSTR str;
	bool result2 = process_if_exists_set( section, "bones_koeff_protection", &CInifile::r_string, str, test );
	if ( result2 && !test )
	{
		m_BonesProtectionSect._set( str );
		CActor* pActor = smart_cast<CActor*>( Level().CurrentViewEntity() );
		ReloadBonesProtection( pActor );
	}
	result |= result2;
	result |= process_if_exists( section, "hit_fraction_actor", &CInifile::r_float, m_boneProtection->m_fHitFracActor, test );
	
	result2 = process_if_exists_set( section, "nightvision_sect", &CInifile::r_string, str, test );
	if ( result2 && !test )
	{
		m_NightVisionSect._set( str );
	}

	result2 = process_if_exists_set(section, "player_hud_section", &CInifile::r_string, str, test);
	if (result2 && !test)
	{
		m_PlayerHudSection = str;

		if (this == smart_cast<CCustomOutfit*>(Actor()->inventory().ItemFromSlot(OUTFIT_SLOT)))
			ApplySkinModel(Actor(), true, false);
	}

	result |= result2;
	
	result |= process_if_exists( section, "additional_inventory_weight",  &CInifile::r_float,  m_additional_weight,  test );
	result |= process_if_exists( section, "additional_inventory_weight2", &CInifile::r_float,  m_additional_weight2, test );

	result |= process_if_exists( section, "health_restore_speed",    &CInifile::r_float, m_fHealthRestoreSpeed,    test );
	result |= process_if_exists( section, "radiation_restore_speed", &CInifile::r_float, m_fRadiationRestoreSpeed, test );
	result |= process_if_exists( section, "satiety_restore_speed",   &CInifile::r_float, m_fSatietyRestoreSpeed,   test );
	result |= process_if_exists( section, "power_restore_speed",     &CInifile::r_float, m_fPowerRestoreSpeed,     test );
	result |= process_if_exists( section, "bleeding_restore_speed",  &CInifile::r_float, m_fBleedingRestoreSpeed,  test );
	result |= process_if_exists( section, "thirst_restore_speed",	 &CInifile::r_float, m_fThirstRestoreSpeed,	   test );
	result |= process_if_exists( section, "intoxication_restore_speed", &CInifile::r_float, m_fIntoxicationRestoreSpeed, test);
	result |= process_if_exists( section, "sleepeness_restore_speed",&CInifile::r_float, m_fSleepenessRestoreSpeed,test );
	result |= process_if_exists( section, "alcoholism_restore_speed",&CInifile::r_float, m_fAlcoholismRestoreSpeed,test );
	result |= process_if_exists( section, "narcotism_restore_speed", &CInifile::r_float, m_fNarcotismRestoreSpeed, test);

	result |= process_if_exists( section, "power_loss", &CInifile::r_float, m_fPowerLoss, test );
	clamp( m_fPowerLoss, 0.0f, 1.0f );

	result |= process_if_exists( section, "artefact_count", &CInifile::r_u32, m_artefact_count, test );
	clamp( m_artefact_count, (u32)0, (u32)GameConstants::GetArtefactsCount());

	result |= process_if_exists(section, "jump_speed", &CInifile::r_float, m_fJumpSpeed, test);
	result |= process_if_exists(section, "walk_accel", &CInifile::r_float, m_fWalkAccel, test);
	result |= process_if_exists(section, "overweight_walk_k", &CInifile::r_float, m_fOverweightWalkK, test);

	result |= process_if_exists_set(section, "night_vision_type", &CInifile::r_u32, m_NightVisionType, test);

	m_fJumpSpeed = READ_IF_EXISTS(pSettings, r_float, section, "jump_speed", 1.f);
	m_fWalkAccel = READ_IF_EXISTS(pSettings, r_float, section, "walk_accel", 1.f);
	m_fOverweightWalkK = READ_IF_EXISTS(pSettings, r_float, section, "overweight_walk_k", 1.f);

	return result;
}

float CCustomOutfit::GetDegradationSpeed() const
{
	return m_fFilterDegradation;
}

float CCustomOutfit::GetFilterCondition() const
{
	return m_fFilterCondition;
}

void CCustomOutfit::SetFilterCondition(float val)
{
	m_fFilterCondition = val;
}

void CCustomOutfit::FilterReplace(float val)
{
	m_fFilterCondition += val;
	clamp(m_fFilterCondition, 0.0f, m_fMaxFilterCondition);
}

bool CCustomOutfit::IsNecessaryItem(const shared_str& item_sect, xr_vector<shared_str> item)
{
	return (std::find(item.begin(), item.end(), item_sect) != item.end());
}