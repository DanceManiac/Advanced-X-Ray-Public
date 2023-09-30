#include "stdafx.h"
#include "AnomalyDetector.h"
#include "CustomZone.h"
#include "HudManager.h"
//#include "Artifact.h"
#include "Inventory.h"
#include "Level.h"
#include "map_manager.h"
#include "CameraEffector.h"
#include "Actor.h"
#include "ai_sounds.h"
#include "player_hud.h"
#include "weapon.h"
#include "AdvancedXrayGameConstants.h"
#include "Battery.h"

ZONE_INFO::ZONE_INFO	()
{
	pParticle=NULL;
}

ZONE_INFO::~ZONE_INFO	()
{
	if(pParticle)
		CParticlesObject::Destroy(pParticle);
}

CDetectorAnomaly::CDetectorAnomaly(void)
{
	m_bWorking					= false;
	m_fUnchargeSpeed			= 0.0f;
}

CDetectorAnomaly::~CDetectorAnomaly(void)
{
	ZONE_TYPE_MAP_IT it;
	for(it = m_ZoneTypeMap.begin(); m_ZoneTypeMap.end() != it; ++it)
		HUD_SOUND_ITEM::DestroySound(it->second.detect_snds);
//		it->second.detect_snd.destroy();

	m_ZoneInfoMap.clear();
}

BOOL CDetectorAnomaly::net_Spawn(CSE_Abstract* DC)
{
	m_pCurrentActor		 = NULL;
	m_pCurrentInvOwner	 = NULL;

	return		(inherited::net_Spawn(DC));
}

void CDetectorAnomaly::Load(LPCSTR section)
{
	inherited::Load			(section);

	m_fRadius				= pSettings->r_float(section,"radius");
	
	if( pSettings->line_exist(section,"night_vision_particle") )
		m_nightvision_particle	= pSettings->r_string(section,"night_vision_particle");

	u32 i = 1;
	string256 temp;

	//загрузить звуки для обозначения различных типов зон
	do 
	{
		sprintf_s			(temp, "zone_class_%d", i);
		if(pSettings->line_exist(section,temp))
		{
			LPCSTR z_Class			= pSettings->r_string(section,temp);
			CLASS_ID zone_cls		= TEXT2CLSID(pSettings->r_string(z_Class,"class"));

			m_ZoneTypeMap.insert	(std::make_pair(zone_cls,ZONE_TYPE()));
			ZONE_TYPE& zone_type	= m_ZoneTypeMap[zone_cls];
			sprintf_s					(temp, "zone_min_freq_%d", i);
			zone_type.min_freq		= pSettings->r_float(section,temp);
			sprintf_s					(temp, "zone_max_freq_%d", i);
			zone_type.max_freq		= pSettings->r_float(section,temp);
			R_ASSERT				(zone_type.min_freq<zone_type.max_freq);
			sprintf_s					(temp, "zone_sound_%d_", i);

			HUD_SOUND_ITEM::LoadSound(section, temp	,zone_type.detect_snds		, SOUND_TYPE_ITEM);

			sprintf_s					(temp, "zone_map_location_%d", i);
			
			if( pSettings->line_exist(section,temp) )
				zone_type.zone_map_location = pSettings->r_string(section,temp);

			++i;
		}
		else break;
	} while(true);

	m_ef_detector_type	= pSettings->r_u32(section,"ef_detector_type");

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

	if (GameConstants::GetAnoDetectorUseBattery())
	{
		float rnd_charge = ::Random.randF(0.0f, m_fMaxChargeLevel);
		m_fCurrentChargeLevel = rnd_charge;
	}
}


void CDetectorAnomaly::shedule_Update(u32 dt)
{
	inherited::shedule_Update	(dt);
	
	if( !IsWorking() ) return;
	if( !H_Parent()  ) return;

	Position().set(H_Parent()->Position());

	if (H_Parent() && H_Parent() == Level().CurrentViewEntity())
	{
		Fvector					P; 
		P.set					(H_Parent()->Position());
		feel_touch_update		(P,m_fRadius);
	}
}

void CDetectorAnomaly::StopAllSounds()
{
	ZONE_TYPE_MAP_IT it;
	for(it = m_ZoneTypeMap.begin(); m_ZoneTypeMap.end() != it; ++it) 
	{
		ZONE_TYPE& zone_type = (*it).second;
		HUD_SOUND_ITEM::StopSound(zone_type.detect_snds);
//		zone_type.detect_snd.stop();
	}
}

void CDetectorAnomaly::UpdateCL()
{
	inherited::UpdateCL();

	if (GameConstants::GetAnoDetectorUseBattery())
		UpdateChargeLevel();

	if( !IsWorking() ) return;
	if( !H_Parent()  ) return;
	if (m_fCurrentChargeLevel <= 0.0) return;

	if(!m_pCurrentActor) return;

	ZONE_INFO_MAP_IT it;
	for(it = m_ZoneInfoMap.begin(); m_ZoneInfoMap.end() != it; ++it) 
	{
		CCustomZone *pZone = it->first;
		ZONE_INFO& zone_info = it->second;

		
		/*//такой тип зон не обнаруживается
		if(m_ZoneTypeMap.find(pZone->CLS_ID) == m_ZoneTypeMap.end() ||
			!pZone->VisibleByDetector())
			continue;*/

		ZONE_TYPE& zone_type = m_ZoneTypeMap[pZone->CLS_ID];

		float dist_to_zone = H_Parent()->Position().distance_to(pZone->Position()) - 0.8f*pZone->Radius();
		if(dist_to_zone<0) dist_to_zone = 0;
		
		float fRelPow = 1.f - dist_to_zone / m_fRadius;
		clamp(fRelPow, 0.f, 1.f);

		//определить текущую частоту срабатывания сигнала
		zone_info.cur_freq = zone_type.min_freq + 
			(zone_type.max_freq - zone_type.min_freq) * fRelPow* fRelPow* fRelPow* fRelPow;

		float current_snd_time = 1000.f*1.f/zone_info.cur_freq;
			
		if((float)zone_info.snd_time > current_snd_time)
		{
			zone_info.snd_time	= 0;
			HUD_SOUND_ITEM::PlaySound(zone_type.detect_snds, Fvector().set(0, 0, 0), NULL, true, false);

		} 
		else 
			zone_info.snd_time += Device.dwTimeDelta;
	}
}

void CDetectorAnomaly::feel_touch_new(CObject* O)
{
	CCustomZone *pZone = smart_cast<CCustomZone*>(O);
	if(pZone && pZone->IsEnabled()) 
	{
		m_ZoneInfoMap[pZone].snd_time = 0;
	}
}

void CDetectorAnomaly::feel_touch_delete(CObject* O)
{
	CCustomZone *pZone = smart_cast<CCustomZone*>(O);
	if(pZone)
	{
		m_ZoneInfoMap.erase(pZone);
	}
}

BOOL CDetectorAnomaly::feel_touch_contact(CObject* O)
{
	return (NULL != smart_cast<CCustomZone*>(O));
}

void CDetectorAnomaly::OnH_A_Chield()
{
	m_pCurrentActor				= smart_cast<CActor*>(H_Parent());
	m_pCurrentInvOwner			= smart_cast<CInventoryOwner*>(H_Parent());
	inherited::OnH_A_Chield		();
}

void CDetectorAnomaly::OnH_B_Independent(bool just_before_destroy)
{
	inherited::OnH_B_Independent(just_before_destroy);
	
	m_pCurrentActor				= NULL;
	m_pCurrentInvOwner			= NULL;

	StopAllSounds				();

	m_ZoneInfoMap.clear			();
	Feel::Touch::feel_touch.clear();
}


u32	CDetectorAnomaly::ef_detector_type	() const
{
	return	(m_ef_detector_type);
}

void CDetectorAnomaly::OnMoveToRuck(EItemPlace prev)
{
	inherited::OnMoveToRuck(prev);
	TurnOff();
}

void CDetectorAnomaly::OnMoveToSlot()
{
	inherited::OnMoveToSlot();
	TurnOn();
}

void CDetectorAnomaly::TurnOn()
{
	m_bWorking				= true;
}

void CDetectorAnomaly::TurnOff()
{
	m_bWorking				= false;
}

void CDetectorAnomaly::save(NET_Packet &output_packet)
{
	inherited::save(output_packet);
}

void CDetectorAnomaly::load(IReader &input_packet)
{
	inherited::load(input_packet);
}

void CDetectorAnomaly::UpdateChargeLevel(void)
{
	if (IsWorking())
	{
		float uncharge_coef = (m_fUnchargeSpeed / 16) * Device.fTimeDelta;
		ChangeChargeLevel(-uncharge_coef);
	}
}

float CDetectorAnomaly::GetUnchargeSpeed() const
{
	return m_fUnchargeSpeed;
}

float CDetectorAnomaly::GetCurrentChargeLevel() const
{
	return m_fCurrentChargeLevel;
}

void CDetectorAnomaly::SetCurrentChargeLevel(float val)
{
	m_fCurrentChargeLevel = val;
	clamp(m_fCurrentChargeLevel, 0.f, m_fMaxChargeLevel);

	float condition = 1.f * m_fCurrentChargeLevel / m_fUnchargeSpeed;
	SetChargeLevel(condition);
}

void CDetectorAnomaly::Recharge(float val)
{
	m_fCurrentChargeLevel += val;
	clamp(m_fCurrentChargeLevel, 0.f, m_fMaxChargeLevel);

	SetChargeLevel(m_fCurrentChargeLevel);

	//Msg("Charge Level In Recharge: %f", val); //For Test
}

bool CDetectorAnomaly::IsNecessaryItem(const shared_str& item_sect, xr_vector<shared_str> item)
{
	return (std::find(item.begin(), item.end(), item_sect) != item.end());
}

/*void CCustomDetector::AddRemoveMapSpot(CCustomZone* pZone, bool bAdd)
{
	if(m_ZoneTypeMap.find(pZone->CLS_ID) == m_ZoneTypeMap.end() )return;
	
	if ( bAdd && !pZone->VisibleByDetector() ) return;
		

	ZONE_TYPE& zone_type = m_ZoneTypeMap[pZone->CLS_ID];
	if( xr_strlen(zone_type.zone_map_location) ){
		if( bAdd )
			Level().MapManager().AddMapLocation(*zone_type.zone_map_location,pZone->ID());
		else
			Level().MapManager().RemoveMapLocation(*zone_type.zone_map_location,pZone->ID());
	}
}

void CCustomDetector::UpdateMapLocations() // called on turn on/off only
{
	ZONE_INFO_MAP_IT it;
	for(it = m_ZoneInfoMap.begin(); it != m_ZoneInfoMap.end(); ++it)
		AddRemoveMapSpot(it->first,IsWorking());
}*/