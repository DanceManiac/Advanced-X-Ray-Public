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
	m_bFilterProtectionDropsInstantly = false;
	m_bUseAttach = false;
	m_NightVisionType = 0;
	m_fNightVisionLumFactor = 0.0f;
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
	m_HitTypeProtection[ALife::eHitTypePhysicStrike] = READ_IF_EXISTS(pSettings, r_float, section, "physic_strike_protection", 0.0f);

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
	m_bFilterProtectionDropsInstantly = READ_IF_EXISTS(pSettings, r_bool, section, "filter_protection_drops_instantly", false);
	m_bUseAttach					= READ_IF_EXISTS(pSettings, r_bool, section, "use_attaching", false);

	m_sShaderNightVisionSect		= READ_IF_EXISTS(pSettings, r_string, section, "shader_nightvision_sect", "shader_nightvision_default");
	m_NightVisionType				= READ_IF_EXISTS(pSettings, r_u32, m_sShaderNightVisionSect, "shader_nightvision_type", 0);
	m_fNightVisionLumFactor			= READ_IF_EXISTS(pSettings, r_float, m_sShaderNightVisionSect, "shader_nightvision_lum_factor", 0.0f);

	m_SuitableFilters.clear();
	m_SuitableRepairKits.clear();
	m_ItemsForRepair.clear();
	m_ItemsForRepairNames.clear();

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
			{
				m_ItemsForRepair.push_back(std::make_pair(items_for_repair_sect, 0));
				m_ItemsForRepairNames.push_back(pSettings->r_string(items_for_repair_sect, "inv_name"));
			}
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

void CHelmet::net_Export(NET_Packet& P)
{
	inherited::net_Export(P);
	P.w_float_q8(GetCondition(),0.0f,1.0f);
}

void CHelmet::net_Import(NET_Packet& P)
{
	inherited::net_Import(P);
	P.r_float_q8(m_fCondition, 0.0f, 1.0f);
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

void CHelmet::UpdateFilterCondition(void)
{
	if (!ParentIsActor() || ParentIsActor() && m_eItemPlace != eItemPlaceSlot)
		return;

	if (m_bUseFilter)
	{
		float uncharge_coef = (m_fFilterDegradation / 16) * Device.fTimeDelta;

		m_fFilterCondition -= uncharge_coef;
		clamp(m_fFilterCondition, 0.0f, m_fMaxFilterCondition);

		float condition = 1.f * m_fFilterCondition;
		float percent = m_fFilterCondition * 100;

		if (m_bFilterProtectionDropsInstantly)
		{
			if (fis_zero(percent))
			{
				if (!fis_zero(m_HitTypeProtection[ALife::eHitTypeRadiation]) && !fis_zero(m_ConstHitTypeProtection[ALife::eHitTypeRadiation]))
					m_HitTypeProtection[ALife::eHitTypeRadiation] = (m_ConstHitTypeProtection[ALife::eHitTypeRadiation] / 100) * 20.0f;
				if (!fis_zero(m_HitTypeProtection[ALife::eHitTypeChemicalBurn]) && !fis_zero(m_ConstHitTypeProtection[ALife::eHitTypeChemicalBurn]))
					m_HitTypeProtection[ALife::eHitTypeChemicalBurn] = (m_ConstHitTypeProtection[ALife::eHitTypeChemicalBurn] / 100) * 20.0f;
			}
		}
		else
		{
			if (percent > 20.0f)
			{
				if (!fis_zero(m_HitTypeProtection[ALife::eHitTypeRadiation]) && !fis_zero(m_ConstHitTypeProtection[ALife::eHitTypeRadiation]))
					m_HitTypeProtection[ALife::eHitTypeRadiation] = (m_ConstHitTypeProtection[ALife::eHitTypeRadiation] / 100) * percent;
				if (!fis_zero(m_HitTypeProtection[ALife::eHitTypeChemicalBurn]) && !fis_zero(m_ConstHitTypeProtection[ALife::eHitTypeChemicalBurn]))
					m_HitTypeProtection[ALife::eHitTypeChemicalBurn] = (m_ConstHitTypeProtection[ALife::eHitTypeChemicalBurn] / 100) * percent;
			}
			else
			{
				if (!fis_zero(m_HitTypeProtection[ALife::eHitTypeRadiation]) && !fis_zero(m_ConstHitTypeProtection[ALife::eHitTypeRadiation]))
					m_HitTypeProtection[ALife::eHitTypeRadiation] = (m_ConstHitTypeProtection[ALife::eHitTypeRadiation] / 100) * 20.0f;
				if (!fis_zero(m_HitTypeProtection[ALife::eHitTypeChemicalBurn]) && !fis_zero(m_ConstHitTypeProtection[ALife::eHitTypeChemicalBurn]))
					m_HitTypeProtection[ALife::eHitTypeChemicalBurn] = (m_ConstHitTypeProtection[ALife::eHitTypeChemicalBurn] / 100) * 20.0f;
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

void CHelmet::OnMoveToSlot(EItemPlace previous_place)
{
	inherited::OnMoveToSlot		(previous_place);
	if (m_pInventory && (previous_place == eItemPlaceSlot))
	{
		CActor* pActor = smart_cast<CActor*> (H_Parent());

		if (pActor)
		{
			if (pActor->GetNightVisionStatus())
				pActor->SwitchNightVision(true, false);
		}
	}
}

void CHelmet::OnMoveToRuck(EItemPlace previous_place)
{
	inherited::OnMoveToRuck		(previous_place);
	if (m_pInventory && (previous_place == eItemPlaceSlot))
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
	float hit_power_not_k = hit_power;
	hit_power *= m_HitTypeK[hit_type];
	ChangeCondition(-hit_power);

	if (!GameConstants::GetOutfitUseFilters() || !Actor()->inventory().InSlot(this) || !m_bUseFilter)
		return;

	switch (hit_type)
	{
	case ALife::eHitTypeChemicalBurn:
	case ALife::eHitTypeRadiation:
	{
		m_fFilterCondition -= hit_power_not_k / 50;
		clamp(m_fFilterCondition, 0.0f, m_fMaxFilterCondition);
	} break;
	default:
		break;
	}
}

float CHelmet::GetDefHitTypeProtection(ALife::EHitType hit_type)
{
	return 1.0f - m_HitTypeProtection[hit_type] * GetCondition();
}

float CHelmet::GetHitTypeProtection(ALife::EHitType hit_type, s16 element)
{
	float fBase = m_HitTypeProtection[hit_type] * GetCondition();
	float bone = m_boneProtection->getBoneProtection(element);
	return 1.0f - fBase * bone;
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