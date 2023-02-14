#include "stdafx.h"
#include "customdetector.h"
#include "ui/ArtefactDetectorUI.h"
#include "hudmanager.h"
#include "inventory.h"
#include "level.h"
#include "map_manager.h"
#include "ActorEffector.h"
#include "actor.h"
#include "ui/UIWindow.h"
#include "player_hud.h"
#include "weapon.h"
#include "AdvancedXrayGameConstants.h"
#include "../xrEngine/LightAnimLibrary.h"
#include "Battery.h"

ITEM_INFO::ITEM_INFO()
{
	pParticle	= NULL;
	curr_ref	= NULL;
}

ITEM_INFO::~ITEM_INFO()
{
	if(pParticle)
		CParticlesObject::Destroy(pParticle);
}

bool  CCustomDetector::CheckCompatibilityInt(CHudItem* itm)
{
	if(itm==NULL)
		return true;

	CInventoryItem iitm				= itm->item();
	u32 slot						= iitm.GetSlot();
	bool bres = (slot==PISTOL_SLOT || slot==KNIFE_SLOT || slot==BOLT_SLOT);

	if(itm->GetState()!=CHUDState::eShowing)
		bres = bres && !itm->IsPending();

	if(bres)
	{
		CWeapon* W = smart_cast<CWeapon*>(itm);
		if(W)
			bres = bres && (W->GetState()!=CHUDState::eBore) && !W->IsZoomed();
	}
	return bres;
}

bool  CCustomDetector::CheckCompatibility(CHudItem* itm)
{
	if(!inherited::CheckCompatibility(itm) )	
		return false;

	if(!CheckCompatibilityInt(itm))
	{
		HideDetector	(true);
		return			false;
	}
	return true;
}

void CCustomDetector::HideDetector(bool bFastMode)
{
	if(GetState()==eIdle)
		ToggleDetector(bFastMode);
}

void CCustomDetector::ShowDetector(bool bFastMode)
{
	if(GetState()==eHidden)
		ToggleDetector(bFastMode);
}

void CCustomDetector::ToggleDetector(bool bFastMode)
{
	m_bFastAnimMode = bFastMode;
	if(GetState()==eHidden)
	{
		PIItem iitem = m_pInventory->ActiveItem();
		CHudItem* itm = (iitem)?iitem->cast_hud_item():NULL;
		if(CheckCompatibilityInt(itm))
		{
			SwitchState				(eShowing);
			TurnDetectorInternal	(true);
		}
	}else
	if(GetState()==eIdle)
		SwitchState					(eHiding);

	m_bNeedActivation = false;
}

void CCustomDetector::OnStateSwitch(u32 S)
{
	inherited::OnStateSwitch(S);

	switch(S)
	{
	case eShowing:
		{
			g_player_hud->attach_item	(this);
			m_sounds.PlaySound			("sndShow", Fvector().set(0,0,0), this, true, false);
			PlayHUDMotion				(m_bFastAnimMode?"anm_show_fast":"anm_show", TRUE, this, GetState());
			SetPending					(TRUE);
		}break;
	case eHiding:
		{
			m_sounds.PlaySound			("sndHide", Fvector().set(0,0,0), this, true, false);
			PlayHUDMotion				(m_bFastAnimMode?"anm_hide_fast":"anm_hide", TRUE, this, GetState());
			SetPending					(TRUE);
		}break;
	case eIdle:
		{
			PlayAnimIdle				();
			SetPending					(FALSE);
		}break;
}
}

void CCustomDetector::OnAnimationEnd(u32 state)
{
	inherited::OnAnimationEnd	(state);
	switch(state)
	{
	case eShowing:
		{
			SwitchState					(eIdle);
		} break;
	case eHiding:
		{
			SwitchState					(eHidden);
			TurnDetectorInternal		(false);
			g_player_hud->detach_item	(this);
		} break;
	}
}

void CCustomDetector::UpdateXForm()
{
	CInventoryItem::UpdateXForm();
}

void CCustomDetector::OnActiveItem()
{
	return;
}

void CCustomDetector::OnHiddenItem()
{
}

CCustomDetector::CCustomDetector() 
{
	m_ui				= NULL;
	m_bFastAnimMode		= false;
	m_bNeedActivation	= false;

	m_fMaxChargeLevel	= 0.0f;
	m_fCurrentChargeLevel = 1.0f;
	m_fUnchargeSpeed	= 0.0f;

	flash_light_bone	= "light_bone_1";
	m_flash_bone_id		= BI_NONE;
	Flash(false, 0.0f);
}

CCustomDetector::~CCustomDetector() 
{
	m_artefacts.destroy		();
	TurnDetectorInternal	(false);
	xr_delete				(m_ui);
	detector_light.destroy	();
	detector_glow.destroy	();
}

BOOL CCustomDetector::net_Spawn(CSE_Abstract* DC) 
{
	TurnDetectorInternal(false);
	return		(inherited::net_Spawn(DC));
}

void CCustomDetector::Load(LPCSTR section) 
{
	inherited::Load			(section);

	m_fAfDetectRadius		= pSettings->r_float(section,"af_radius");
	m_fAfVisRadius			= pSettings->r_float(section,"af_vis_radius");
	m_artefacts.load		(section, "af");

	m_sounds.LoadSound( section, "snd_draw", "sndShow");
	m_sounds.LoadSound( section, "snd_holster", "sndHide");

	m_fMaxChargeLevel = READ_IF_EXISTS(pSettings, r_float, section, "max_charge_level", 1.0f);
	m_fUnchargeSpeed = READ_IF_EXISTS(pSettings, r_float, section, "uncharge_speed", 0.0f);

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

	if (GameConstants::GetArtDetectorUseBattery())
	{
		float rnd_charge = ::Random.randF(0.0f, m_fMaxChargeLevel);
		m_fCurrentChargeLevel = rnd_charge;
	}

	m_bLightsEnabled = READ_IF_EXISTS(pSettings, r_string, section, "light_enabled", false);

	if (!detector_light && m_bLightsEnabled)
	{
		m_bLightsAlways = READ_IF_EXISTS(pSettings, r_string, section, "light_always", false);
		m_bUseFlashLight = READ_IF_EXISTS(pSettings, r_bool, section, "light_flash_mode", false);

		flash_light_bone = READ_IF_EXISTS(pSettings, r_string, section, "flash_bone", "light_bone_2");

		detector_light = ::Render->light_create();
		detector_light->set_shadow(READ_IF_EXISTS(pSettings, r_string, section, "light_shadow", false));

		m_bVolumetricLights = READ_IF_EXISTS(pSettings, r_bool, section, "volumetric_lights", false);
		m_fVolumetricQuality = READ_IF_EXISTS(pSettings, r_float, section, "volumetric_quality", 1.0f);
		m_fConstVolumetricDistance = READ_IF_EXISTS(pSettings, r_float, section, "volumetric_distance", 0.3f);
		m_fConstVolumetricIntensity = READ_IF_EXISTS(pSettings, r_float, section, "volumetric_intensity", 0.5f);

		m_fVolumetricDistance = m_fConstVolumetricDistance;
		m_fVolumetricIntensity = m_fConstVolumetricIntensity;

		m_iLightType = READ_IF_EXISTS(pSettings, r_u8, section, "light_type", 1);
		light_lanim = LALib.FindItem(READ_IF_EXISTS(pSettings, r_string, section, "color_animator", ""));

		const Fcolor clr = READ_IF_EXISTS(pSettings, r_fcolor, section, "light_color", (Fcolor{ 1.0f, 0.0f, 0.0f, 1.0f }));

		fBrightness = clr.intensity();
		detector_light->set_color(clr);

		m_fConstLightRange = READ_IF_EXISTS(pSettings, r_float, section, "light_range", 1.f);
		m_fLightRange = m_fConstLightRange;

		detector_light->set_range(m_fLightRange);
		detector_light->set_hud_mode(true);
		detector_light->set_type((IRender_Light::LT)m_iLightType);
		detector_light->set_cone(deg2rad(READ_IF_EXISTS(pSettings, r_float, section, "light_spot_angle", 1.f)));
		detector_light->set_texture(READ_IF_EXISTS(pSettings, r_string, section, "spot_texture", nullptr));

		detector_light->set_volumetric(m_bVolumetricLights);
		detector_light->set_volumetric_quality(m_fVolumetricQuality);
		detector_light->set_volumetric_distance(m_fVolumetricDistance);
		detector_light->set_volumetric_intensity(m_fVolumetricIntensity);

		//Glow
		m_bGlowEnabled = READ_IF_EXISTS(pSettings, r_string, section, "glow_enabled", false);

		if (!detector_glow && m_bGlowEnabled)
		{
			detector_glow = ::Render->glow_create();
			detector_glow->set_texture(READ_IF_EXISTS(pSettings, r_string, section, "glow_texture", nullptr));
			detector_glow->set_color(clr);
			detector_glow->set_radius(READ_IF_EXISTS(pSettings, r_float, section, "glow_radius", 0.3f));
		}
	}
}


void CCustomDetector::shedule_Update(u32 dt) 
{
	inherited::shedule_Update(dt);
	
	if( !IsWorking() )			return;

	Position().set(H_Parent()->Position());

	Fvector						P; 
	P.set						(H_Parent()->Position());
	m_artefacts.feel_touch_update(P,m_fAfDetectRadius);
}


bool CCustomDetector::IsWorking()
{
	return m_bWorking && H_Parent() && H_Parent()==Level().CurrentViewEntity();
}

void CCustomDetector::UpfateWork()
{
	if (m_fCurrentChargeLevel > 0)
	UpdateAf				();
	m_ui->update			();
}

void CCustomDetector::UpdateLights()
{
	if (detector_light)
	{
		if (!detector_light->get_active() && m_fCurrentChargeLevel > 0 && IsWorking() && (m_artefacts.m_ItemInfos.size() > 0 || m_bLightsAlways))
		{
			if (!m_bUseFlashLight)
				detector_light->set_active(true);

			if (detector_glow && !detector_glow->get_active() && m_bGlowEnabled)
				detector_glow->set_active(true);
		}
		else if (detector_light->get_active() && (m_fCurrentChargeLevel <= 0 || !IsWorking() || (m_artefacts.m_ItemInfos.size() == 0) && !m_bLightsAlways))
		{
			detector_light->set_active(false);

			if (detector_glow && detector_glow->get_active() && m_bGlowEnabled)
				detector_glow->set_active(false);
		}

		if (detector_light->get_active() && HudItemData())
		{
			if (GetHUDmode())
			{
				firedeps fd;
				HudItemData()->setup_firedeps(fd);
				detector_light->set_position(fd.vLastFP2);

				Fmatrix lightXForm;
				lightXForm.identity();
				lightXForm.k.set(fd.vLastFD);
				Fvector::generate_orthonormal_basis_normalized(lightXForm.k, lightXForm.j, lightXForm.i);

				detector_light->set_rotation(lightXForm.k, lightXForm.i);

				if (detector_glow && detector_glow->get_active())
					detector_glow->set_position(fd.vLastFP2);
			}

			// calc color animator
			if (light_lanim)
			{
				int frame{};
				u32 clr = light_lanim->CalculateRGB(Device.fTimeGlobal, frame);
				Fcolor fclr;
				fclr.set(clr);
				detector_light->set_color(fclr);
			}
		}

		CCustomDetector* detector = smart_cast<CCustomDetector*>(Actor()->inventory().ItemFromSlot(DETECTOR_SLOT));
		float percent = 100.f;

		if (detector)
			percent = detector->m_fCurrentChargeLevel * 100;

		//The effect of charge on light
		if (m_bLightsEnabled)
		{
			if (m_fLightRange >= 0.0f)
				m_fLightRange = (m_fConstLightRange / 100) * percent;
			else
				m_bLightsEnabled = false;

			detector_light->set_range(m_fLightRange);
		}

		//The effect of charge on volumetric lights
		if (m_bVolumetricLights)
		{
			if (m_fVolumetricDistance >= 0.0f)
				m_fVolumetricDistance = (m_fConstVolumetricDistance / 100) * percent;
			else if (m_fVolumetricIntensity >= 0.0f)
				m_fVolumetricIntensity = (m_fConstVolumetricIntensity / 100) * percent;
			else
				m_bVolumetricLights = false;

			detector_light->set_volumetric_distance(m_fVolumetricDistance);
			detector_light->set_volumetric_intensity(m_fVolumetricIntensity);
		}
	}
}

void CCustomDetector::Flash(bool bOn, float fRelPower)
{
	if (!HudItemData() || !m_bUseFlashLight || !m_bLightsEnabled) return;

	IKinematics* K = HudItemData()->m_model;

	if (m_bUseFlashLight)
	{
		if (m_flash_bone_id == BI_NONE)
		{
			R_ASSERT(K);

			R_ASSERT(m_flash_bone_id == BI_NONE);

			m_flash_bone_id = K->LL_BoneID(flash_light_bone);

			K->LL_SetBoneVisible(m_flash_bone_id, FALSE, TRUE);
		}
	}

	if (bOn)
	{
		K->LL_SetBoneVisible(m_flash_bone_id, TRUE, TRUE);
		m_turn_off_flash_time = Device.dwTimeGlobal + iFloor(fRelPower * 1000.0f);
	}
	else
	{
		K->LL_SetBoneVisible(m_flash_bone_id, FALSE, TRUE);
		m_turn_off_flash_time = 0;
	}
	if (bOn != detector_light->get_active())
		detector_light->set_active(bOn);
}

void CCustomDetector::UpdateVisibility()
{
	//check visibility
	attachable_hud_item* i0		= g_player_hud->attached_item(0);
	if(i0 && HudItemData())
	{
		CWeapon* wpn			= smart_cast<CWeapon*>(i0->m_parent_hud_item);
		if(wpn)
		{
			u32 state			= wpn->GetState();
			bool bClimb			= ( (Actor()->MovingState()&mcClimb) != 0 );
			if(bClimb || wpn->IsZoomed() || state==CWeapon::eReload || state==CWeapon::eSwitch)
			{
				HideDetector		(true);
				m_bNeedActivation	= true;
			}
		}
	}else
	if(m_bNeedActivation)
	{
		attachable_hud_item* i0		= g_player_hud->attached_item(0);
		bool bClimb					= ( (Actor()->MovingState()&mcClimb) != 0 );
		if(!bClimb)
		{
			CWeapon* wpn			= (i0)?smart_cast<CWeapon*>(i0->m_parent_hud_item) : NULL;
			if(	!wpn || 
				(	!wpn->IsZoomed() && 
					wpn->GetState()!=CWeapon::eReload && 
					wpn->GetState()!=CWeapon::eSwitch
				)
			)
			{
				ShowDetector		(true);
			}
		}
	}
}

void CCustomDetector::UpdateCL() 
{
	inherited::UpdateCL();
	UpdateLights();

	if (GameConstants::GetArtDetectorUseBattery())
		UpdateChargeLevel();

	UpdateVisibility		();

	if( !IsWorking() )		return;
	UpfateWork				();
}

void CCustomDetector::OnH_A_Chield() 
{
	inherited::OnH_A_Chield		();
}

void CCustomDetector::OnH_B_Independent(bool just_before_destroy) 
{
	inherited::OnH_B_Independent(just_before_destroy);
	
	m_artefacts.clear			();
}


void CCustomDetector::OnMoveToRuck(EItemPlace prev)
{
	inherited::OnMoveToRuck	(prev);
	if(GetState()==eIdle)
	{
		SwitchState					(eHidden);
		g_player_hud->detach_item	(this);
	}
	TurnDetectorInternal	(false);
}

void CCustomDetector::OnMoveToSlot()
{
	inherited::OnMoveToSlot	();
}

void CCustomDetector::TurnDetectorInternal(bool b)
{
	m_bWorking				= b;
	if(b && m_ui==NULL)
	{
		CreateUI			();
	}else
	{
//.		xr_delete			(m_ui);
	}

	UpdateNightVisionMode	(b);
}



#include "game_base_space.h"
void CCustomDetector::UpdateNightVisionMode(bool b_on)
{
}

void CCustomDetector::save(NET_Packet &output_packet)
{
	inherited::save(output_packet);
	save_data(m_fCurrentChargeLevel, output_packet);

}

void CCustomDetector::load(IReader &input_packet)
{
	inherited::load(input_packet);
	load_data(m_fCurrentChargeLevel, input_packet);
}

void CCustomDetector::UpdateChargeLevel(void)
{
	if (IsWorking())
	{
		float uncharge_coef = (m_fUnchargeSpeed / 16) * Device.fTimeDelta;

		m_fCurrentChargeLevel -= uncharge_coef;

		float condition = 1.f * m_fCurrentChargeLevel;
		SetCondition(condition);

		//Msg("Заряд детектора: %f", m_fCurrentChargeLevel); //Для тестов

		clamp(m_fCurrentChargeLevel, 0.f, m_fMaxChargeLevel);
		SetCondition(m_fCurrentChargeLevel);
	}
	/*else
		SetCondition(m_fCurrentChargeLevel);*/
}

float CCustomDetector::GetUnchargeSpeed() const
{
	return m_fUnchargeSpeed;
}

float CCustomDetector::GetCurrentChargeLevel() const
{
	return m_fCurrentChargeLevel;
}

void CCustomDetector::SetCurrentChargeLevel(float val)
{
	m_fCurrentChargeLevel = val;
	clamp(m_fCurrentChargeLevel, 0.f, m_fMaxChargeLevel);

	float condition = 1.f * m_fCurrentChargeLevel / m_fUnchargeSpeed;
	SetCondition(condition);
}

void CCustomDetector::Recharge(float val)
{
	m_fCurrentChargeLevel += val;
	clamp(m_fCurrentChargeLevel, 0.f, m_fMaxChargeLevel);

	SetCondition(m_fCurrentChargeLevel);

	//Msg("Переданый в детектор заряд: %f", val); //Для Тестов
}

BOOL CAfList::feel_touch_contact	(CObject* O)
{
	CLASS_ID	clsid			= O->CLS_ID;
	TypesMapIt it				= m_TypesMap.find(clsid);

	bool res					 = (it!=m_TypesMap.end());
	if(res)
	{
		CArtefact*	pAf				= smart_cast<CArtefact*>(O);
		
		if(pAf->GetAfRank()>m_af_rank)
			res = false;
	}
	return						res;
}

bool CCustomDetector::install_upgrade_impl(LPCSTR section, bool test)
{
	//Msg("Detector Upgrade");
	bool result = inherited::install_upgrade_impl(section, test);

	result |= process_if_exists(section, "af_radius", &CInifile::r_float, m_fAfDetectRadius, test);
	result |= process_if_exists(section, "af_vis_radius", &CInifile::r_float, m_fAfVisRadius, test);
	result |= process_if_exists(section, "max_charge_level", &CInifile::r_float, m_fMaxChargeLevel, test);
	result |= process_if_exists(section, "uncharge_speed", &CInifile::r_float, m_fUnchargeSpeed, test);

	LPCSTR str;

	// name of the ltx-section of hud
	bool result2 = process_if_exists_set(section, "hud", &CInifile::r_string, str, test);
	if (result2 && !test)
		this->ReplaceHudSection(str);

	return result;
}

bool CCustomDetector::IsNecessaryItem(const shared_str& item_sect, xr_vector<shared_str> item)
{
	return (std::find(item.begin(), item.end(), item_sect) != item.end());
}