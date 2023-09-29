#include "stdafx.h"
#include "ActorHelmet.h"
#include "Actor.h"
#include "Inventory.h"
#include "BoneProtections.h"
#include "../Include/xrRender/Kinematics.h"
#include "DynamicHudGlass.h"
#include "AdvancedXrayGameConstants.h"
#include "AntigasFilter.h"
#include "ui/UIMainIngameWnd.h"
#include "ui/UIHudStatesWnd.h"
#include "UIGameCustom.h"
//#include "CustomOutfit.h"

CHelmet::CHelmet()
{
	m_flags.set(FUsingCondition, TRUE);
	for(int i=0; i<ALife::eHitTypeMax; i++)
		m_HitTypeProtection[(ALife::EHitType)i] = 1.0f;

	m_boneProtection = xr_new<SBoneProtections>();

	m_b_HasGlass = false;
	m_bUseFilter = false;
	m_NightVisionType = 0;
	m_fFilterDegradation = 0.0f;
	m_fFilterCondition = 1.0f;
}

CHelmet::~CHelmet()
{
	xr_delete(m_boneProtection);
}

void CHelmet::Load(LPCSTR section) 
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
	m_HitTypeProtection[ALife::eHitTypeChemicalBurn]= m_ConstHitTypeProtection[ALife::eHitTypeChemicalBurn];
	m_HitTypeProtection[ALife::eHitTypeExplosion]	= pSettings->r_float(section,"explosion_protection");
	m_HitTypeProtection[ALife::eHitTypeFireWound]	= pSettings->r_float(section,"fire_wound_protection");
//	m_HitTypeProtection[ALife::eHitTypePhysicStrike]= pSettings->r_float(section,"physic_strike_protection");
	m_HitTypeProtection[ALife::eHitTypeLightBurn]	= m_HitTypeProtection[ALife::eHitTypeBurn];
	m_boneProtection->m_fHitFracActor				= pSettings->r_float(section, "hit_fraction_actor");

	if (pSettings->line_exist(section, "nightvision_sect"))
		m_NightVisionSect = pSettings->r_string(section, "nightvision_sect");
	else
		m_NightVisionSect = "";

	m_fHealthRestoreSpeed			= READ_IF_EXISTS(pSettings, r_float, section, "health_restore_speed",    0.0f );
	m_fRadiationRestoreSpeed		= READ_IF_EXISTS(pSettings, r_float, section, "radiation_restore_speed", 0.0f );
	m_fSatietyRestoreSpeed			= READ_IF_EXISTS(pSettings, r_float, section, "satiety_restore_speed",   0.0f );
	m_fPowerRestoreSpeed			= READ_IF_EXISTS(pSettings, r_float, section, "power_restore_speed",     0.0f );
	m_fBleedingRestoreSpeed			= READ_IF_EXISTS(pSettings, r_float, section, "bleeding_restore_speed",  0.0f );
	m_fPowerLoss					= READ_IF_EXISTS(pSettings, r_float, section, "power_loss",    1.0f );
	m_fFilterDegradation			= READ_IF_EXISTS(pSettings, r_float, section, "filter_degradation_speed", 0.0f);
	m_fMaxFilterCondition			= READ_IF_EXISTS(pSettings, r_float, section, "max_filter_condition", 1.0f);

	m_bSecondHelmetEnabled			= READ_IF_EXISTS(pSettings, r_bool, section, "second_helmet_enabled", true);

	clamp							( m_fPowerLoss, 0.0f, 1.0f );

	m_BonesProtectionSect			= READ_IF_EXISTS(pSettings, r_string, section, "bones_koeff_protection",  "" );
	m_fShowNearestEnemiesDistance	= READ_IF_EXISTS(pSettings, r_float, section, "nearest_enemies_show_dist",  0.0f );

	m_b_HasGlass					= !!READ_IF_EXISTS(pSettings, r_bool, section, "has_glass", FALSE);
	m_bUseFilter					= READ_IF_EXISTS(pSettings, r_bool, section, "use_filter", false);
	m_NightVisionType				= READ_IF_EXISTS(pSettings, r_u32, section, "night_vision_type", 0);

	m_SuitableFilters.clear();
	m_SuitableRepairKits.clear();
	m_ItemsForRepair.clear();

	LPCSTR filters = READ_IF_EXISTS(pSettings, r_string, section, "suitable_filters", "antigas_filter");
	LPCSTR repair_kits = READ_IF_EXISTS(pSettings, r_string, section, "suitable_repair_kits", "repair_kit");
	LPCSTR items_for_repair = READ_IF_EXISTS(pSettings, r_string, section, "items_for_repair", "");

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

	if (items_for_repair && items_for_repair[0])
	{
		string128 items_for_repair_sect;
		int count = _GetItemCount(items_for_repair);

		for (int it = 0; it < count; ++it)
		{
			_GetItem(items_for_repair, it, items_for_repair_sect);

			if ((it % 2 != 0 && it != 0) || it == 1)
				m_ItemsForRepair[it / 2].second = std::stoi(items_for_repair_sect);
			else
				m_ItemsForRepair.push_back(std::make_pair(items_for_repair_sect, 0));
		}
	}

	if (GameConstants::GetOutfitUseFilters())
	{
		float rnd_cond = ::Random.randF(0.0f, m_fMaxFilterCondition);
		m_fFilterCondition = rnd_cond;
	}
}

void CHelmet::ReloadBonesProtection()
{
	CObject* parent = H_Parent();
	if(IsGameTypeSingle())
		parent = smart_cast<CObject*>(Level().CurrentViewEntity());

	if(parent && parent->Visual() && m_BonesProtectionSect.size())
		m_boneProtection->reload( m_BonesProtectionSect, smart_cast<IKinematics*>(parent->Visual()));
}

BOOL CHelmet::net_Spawn(CSE_Abstract* DC)
{
	if(IsGameTypeSingle())
		ReloadBonesProtection();

	BOOL res = inherited::net_Spawn(DC);
	return					(res);
}

void CHelmet::net_Export(NET_Packet& P)
{
	inherited::net_Export(P);
	P.w_float_q8(GetCondition(),0.0f,1.0f);
}

void CHelmet::net_Import(NET_Packet& P)
{
	inherited::net_Import(P);
	float _cond;
	P.r_float_q8(_cond,0.0f,1.0f);
	SetCondition(_cond);
}

void CHelmet::save(NET_Packet& output_packet)
{
	inherited::save(output_packet);
	save_data(m_fFilterCondition, output_packet);

}

void CHelmet::load(IReader& input_packet)
{
	inherited::load(input_packet);
	load_data(m_fFilterCondition, input_packet);
}

void CHelmet::OnH_A_Chield()
{
	inherited::OnH_A_Chield();
//	ReloadBonesProtection();
}

void CHelmet::UpdateFilterCondition(void)
{
	CHelmet* helmet = smart_cast<CHelmet*>(Actor()->inventory().ItemFromSlot(HELMET_SLOT));
	CHelmet* helmet2 = smart_cast<CHelmet*>(Actor()->inventory().ItemFromSlot(SECOND_HELMET_SLOT));

	if ((helmet && helmet->m_bUseFilter) || (helmet2 && helmet2->m_bUseFilter))
	{
		float m_radia_hit = CurrentGameUI()->get_zone_cur_power(ALife::eHitTypeRadiation) * 4;
		float m_chemical_hit = CurrentGameUI()->get_zone_cur_power(ALife::eHitTypeChemicalBurn);

		if (helmet)
		{
			float uncharge_coef = ((helmet->m_fFilterDegradation + m_radia_hit + m_chemical_hit) / 16) * Device.fTimeDelta;

			helmet->m_fFilterCondition -= uncharge_coef;
			clamp(helmet->m_fFilterCondition, 0.0f, m_fMaxFilterCondition);

			float condition = 1.f * helmet->m_fFilterCondition;
			float percent = helmet->m_fFilterCondition * 100;

			if (percent > 20.0f)
			{
				if (!fis_zero(helmet->m_HitTypeProtection[ALife::eHitTypeRadiation]) && !fis_zero(m_ConstHitTypeProtection[ALife::eHitTypeRadiation]))
					helmet->m_HitTypeProtection[ALife::eHitTypeRadiation] = (m_ConstHitTypeProtection[ALife::eHitTypeRadiation] / 100) * percent;
				if (!fis_zero(helmet->m_HitTypeProtection[ALife::eHitTypeChemicalBurn]) && !fis_zero(m_ConstHitTypeProtection[ALife::eHitTypeChemicalBurn]))
					helmet->m_HitTypeProtection[ALife::eHitTypeChemicalBurn] = (m_ConstHitTypeProtection[ALife::eHitTypeChemicalBurn] / 100) * percent;
			}
			else
			{
				if (!fis_zero(helmet->m_HitTypeProtection[ALife::eHitTypeRadiation]) && !fis_zero(m_ConstHitTypeProtection[ALife::eHitTypeRadiation]))
					helmet->m_HitTypeProtection[ALife::eHitTypeRadiation] = (m_ConstHitTypeProtection[ALife::eHitTypeRadiation] / 100) * 20.0f;
				if (!fis_zero(helmet->m_HitTypeProtection[ALife::eHitTypeChemicalBurn]) && !fis_zero(m_ConstHitTypeProtection[ALife::eHitTypeChemicalBurn]))
					helmet->m_HitTypeProtection[ALife::eHitTypeChemicalBurn] = (m_ConstHitTypeProtection[ALife::eHitTypeChemicalBurn] / 100) * 20.0f;
			}
		}

		if (helmet2)
		{
			float uncharge_coef = ((helmet2->m_fFilterDegradation + m_radia_hit + m_chemical_hit) / 16) * Device.fTimeDelta;

			helmet2->m_fFilterCondition -= uncharge_coef;
			clamp(helmet2->m_fFilterCondition, 0.0f, m_fMaxFilterCondition);

			float condition = 1.f * helmet2->m_fFilterCondition;
			float percent = helmet2->m_fFilterCondition * 100;

			if (percent > 20.0f)
			{
				if (!fis_zero(helmet2->m_HitTypeProtection[ALife::eHitTypeRadiation]) && !fis_zero(m_ConstHitTypeProtection[ALife::eHitTypeRadiation]))
					helmet2->m_HitTypeProtection[ALife::eHitTypeRadiation] = (m_ConstHitTypeProtection[ALife::eHitTypeRadiation] / 100) * percent;
				if (!fis_zero(helmet2->m_HitTypeProtection[ALife::eHitTypeChemicalBurn]) && !fis_zero(m_ConstHitTypeProtection[ALife::eHitTypeChemicalBurn]))
					helmet2->m_HitTypeProtection[ALife::eHitTypeChemicalBurn] = (m_ConstHitTypeProtection[ALife::eHitTypeChemicalBurn] / 100) * percent;
			}
			else
			{
				if (!fis_zero(helmet2->m_HitTypeProtection[ALife::eHitTypeRadiation]) && !fis_zero(m_ConstHitTypeProtection[ALife::eHitTypeRadiation]))
					helmet2->m_HitTypeProtection[ALife::eHitTypeRadiation] = (m_ConstHitTypeProtection[ALife::eHitTypeRadiation] / 100) * 20.0f;
				if (!fis_zero(helmet2->m_HitTypeProtection[ALife::eHitTypeChemicalBurn]) && !fis_zero(m_ConstHitTypeProtection[ALife::eHitTypeChemicalBurn]))
					helmet2->m_HitTypeProtection[ALife::eHitTypeChemicalBurn] = (m_ConstHitTypeProtection[ALife::eHitTypeChemicalBurn] / 100) * 20.0f;
			}
		}
	}
}

void CHelmet::UpdateCL()
{
	inherited::UpdateCL();

	if (GameConstants::GetOutfitUseFilters())
		UpdateFilterCondition();
}

void CHelmet::OnMoveToSlot(const SInvItemPlace& previous_place)
{
	inherited::OnMoveToSlot		(previous_place);
	if (m_pInventory)
	{
		CActor* pActor = smart_cast<CActor*> (H_Parent());
		if (!pActor)
			return;

		if (previous_place.type == eItemPlaceSlot)
		{
			if (pActor->GetNightVisionStatus())
				pActor->SwitchNightVision(true, false);
		}

		CHelmet* pHelmet1 = smart_cast<CHelmet*>(pActor->inventory().ItemFromSlot(HELMET_SLOT));
		CHelmet* pHelmet2 = smart_cast<CHelmet*>(pActor->inventory().ItemFromSlot(SECOND_HELMET_SLOT));

		if (this == pHelmet1 && !pHelmet1->m_bSecondHelmetEnabled)
		{
			if (pHelmet2)
			{
				pActor->inventory().Ruck(pHelmet2, false);
			}
		}
		else if (this == pHelmet2 && !pHelmet2->m_bSecondHelmetEnabled)
		{
			if (pHelmet1)
			{
				pActor->inventory().Ruck(pHelmet1, false);
			}
		}
	}
}

void CHelmet::OnMoveToRuck(const SInvItemPlace& previous_place)
{
	inherited::OnMoveToRuck		(previous_place);
	if (m_pInventory && (previous_place.type==eItemPlaceSlot))
	{
		CActor* pActor = smart_cast<CActor*> (H_Parent());
		if (pActor)
		{
			pActor->SwitchNightVision(false);
		}
	}
}

void CHelmet::Hit(float hit_power, ALife::EHitType hit_type)
{
	hit_power *= GetHitImmunity(hit_type);
	ChangeCondition(-hit_power);
}

float CHelmet::GetDefHitTypeProtection(ALife::EHitType hit_type)
{
	return m_HitTypeProtection[hit_type]*GetCondition();
}

float CHelmet::GetHitTypeProtection(ALife::EHitType hit_type, s16 element)
{
	float fBase = m_HitTypeProtection[hit_type]*GetCondition();
	float bone = m_boneProtection->getBoneProtection(element);
	return fBase*bone;
}

float CHelmet::GetBoneArmor(s16 element)
{
	return m_boneProtection->getBoneArmor(element);
}

bool CHelmet::install_upgrade_impl( LPCSTR section, bool test )
{
	bool result = inherited::install_upgrade_impl( section, test );

	result |= process_if_exists( section, "burn_protection",          &CInifile::r_float, m_HitTypeProtection[ALife::eHitTypeBurn]        , test );
	result |= process_if_exists( section, "shock_protection",         &CInifile::r_float, m_HitTypeProtection[ALife::eHitTypeShock]       , test );
	result |= process_if_exists( section, "strike_protection",        &CInifile::r_float, m_HitTypeProtection[ALife::eHitTypeStrike]      , test );
	result |= process_if_exists( section, "wound_protection",         &CInifile::r_float, m_HitTypeProtection[ALife::eHitTypeWound]       , test );
	result |= process_if_exists( section, "radiation_protection",     &CInifile::r_float, m_HitTypeProtection[ALife::eHitTypeRadiation]   , test );
	result |= process_if_exists( section, "telepatic_protection",     &CInifile::r_float, m_HitTypeProtection[ALife::eHitTypeTelepatic]   , test );
	result |= process_if_exists( section, "chemical_burn_protection", &CInifile::r_float, m_HitTypeProtection[ALife::eHitTypeChemicalBurn], test );
	result |= process_if_exists( section, "explosion_protection",     &CInifile::r_float, m_HitTypeProtection[ALife::eHitTypeExplosion]   , test );
	result |= process_if_exists( section, "fire_wound_protection",    &CInifile::r_float, m_HitTypeProtection[ALife::eHitTypeFireWound]   , test );

	LPCSTR str;
	bool result2 = process_if_exists_set( section, "nightvision_sect", &CInifile::r_string, str, test );
	if ( result2 && !test )
	{
		m_NightVisionSect._set( str );
	}
	result |= result2;

	result |= process_if_exists( section, "health_restore_speed",    &CInifile::r_float, m_fHealthRestoreSpeed,    test );
	result |= process_if_exists( section, "radiation_restore_speed", &CInifile::r_float, m_fRadiationRestoreSpeed, test );
	result |= process_if_exists( section, "satiety_restore_speed",   &CInifile::r_float, m_fSatietyRestoreSpeed,   test );
	result |= process_if_exists( section, "power_restore_speed",     &CInifile::r_float, m_fPowerRestoreSpeed,     test );
	result |= process_if_exists( section, "bleeding_restore_speed",  &CInifile::r_float, m_fBleedingRestoreSpeed,  test );

	result |= process_if_exists( section, "power_loss", &CInifile::r_float, m_fPowerLoss, test );
	clamp( m_fPowerLoss, 0.0f, 1.0f );

	result |= process_if_exists( section, "nearest_enemies_show_dist",  &CInifile::r_float, m_fShowNearestEnemiesDistance,  test );

	result |= process_if_exists_set(section, "night_vision_type", &CInifile::r_u32, m_NightVisionType, test);

	result2 = process_if_exists_set( section, "bones_koeff_protection", &CInifile::r_string, str, test );
	if ( result2 && !test )
	{
		m_BonesProtectionSect	= str;
		ReloadBonesProtection	();
	}
	result2 = process_if_exists_set( section, "bones_koeff_protection_add", &CInifile::r_string, str, test );
	if ( result2 && !test )
		AddBonesProtection	(str);

	return result;
}

void CHelmet::AddBonesProtection(LPCSTR bones_section)
{
	CObject* parent = H_Parent();
	if(IsGameTypeSingle())
		parent = smart_cast<CObject*>(Level().CurrentViewEntity());

	if ( parent && parent->Visual() && m_BonesProtectionSect.size() )
		m_boneProtection->add(bones_section, smart_cast<IKinematics*>( parent->Visual() ) );
}

float CHelmet::HitThroughArmor(float hit_power, s16 element, float ap, bool& add_wound, ALife::EHitType hit_type)
{
	float NewHitPower = hit_power;
	if(hit_type == ALife::eHitTypeFireWound)
	{
		float ba = GetBoneArmor(element);
		if(ba<0.0f)
			return NewHitPower;

		float BoneArmor = ba*GetCondition();
		if(/*!fis_zero(ba, EPS) && */(ap > BoneArmor))
		{
			//пуля пробила бронь
			if(!IsGameTypeSingle())
			{
				float hit_fraction = (ap - BoneArmor) / ap;
				if(hit_fraction < m_boneProtection->m_fHitFracActor)
					hit_fraction = m_boneProtection->m_fHitFracActor;

				NewHitPower *= hit_fraction;
				NewHitPower *= m_boneProtection->getBoneProtection(element);
			}

			VERIFY(NewHitPower>=0.0f);
		}
		else
		{
			//пуля НЕ пробила бронь
			NewHitPower *= m_boneProtection->m_fHitFracActor;
			add_wound = false; 	//раны нет
		}
	}
	else
	{
		float one = 0.1f;
		if(hit_type == ALife::eHitTypeStrike || 
		   hit_type == ALife::eHitTypeWound || 
		   hit_type == ALife::eHitTypeWound_2 || 
		   hit_type == ALife::eHitTypeExplosion)
		{
			one = 1.0f;
		}
		float protect = GetDefHitTypeProtection(hit_type);
		NewHitPower -= protect * one;

		if(NewHitPower < 0.f)
			NewHitPower = 0.f;
	}
	//увеличить изношенность шлема
	Hit(hit_power, hit_type);

	return NewHitPower;
}

float CHelmet::GetDegradationSpeed() const
{
	return m_fFilterDegradation;
}

float CHelmet::GetFilterCondition() const
{
	return m_fFilterCondition;
}

void CHelmet::SetFilterCondition(float val)
{
	m_fFilterCondition = val;
}

void CHelmet::FilterReplace(float val)
{
	m_fFilterCondition += val;
	clamp(m_fFilterCondition, 0.0f, m_fMaxFilterCondition);
}

bool CHelmet::IsNecessaryItem(const shared_str& item_sect, xr_vector<shared_str> item)
{
	return (std::find(item.begin(), item.end(), item_sect) != item.end());
}