#include "StdAfx.h"
#include "AdvancedXrayGameConstants.h"
#include "GamePersistent.h"

bool	m_bDistantSoundsEnabled = true;
bool	m_bKnifeSlotEnabled = false;
bool	m_bBinocularSlotEnabled = false;
bool	m_bTorchSlotEnabled = false;
bool	m_bBackpackSlotEnabled = false;
bool	m_bSecondHelmetSlotEnabled = false;
bool	m_bDosimeterSlotEnabled = false;
bool	m_bPantsSlotEnabled = false;
bool	m_bPdaSlotEnabled = false;
bool	m_bTorchUseBattery = false;
bool	m_bArtefactDetectorUseBattery = false;
bool	m_bAnomalyDetectorUseBattery = false;
bool	m_bLimitedBolts = false;
bool	m_bActorThirst = false;
bool	m_bActorIntoxication = false;
bool	m_bActorSleepeness = false;
bool	m_bActorAlcoholism = false;
bool	m_bActorNarcotism = false;
bool	m_bArtefactsDegradation = false;
bool	m_bMultiItemPickup = true;
bool	m_bShowWpnInfo = true;
bool	m_bJumpSpeedWeightCalc = false;
bool	m_bHideWeaponInInventory = false;
bool	m_bStopActorIfShoot = false;
bool	m_bReloadIfSprint = true;
int		m_iArtefactsCount = 5;
float	m_fDistantSndDistance = 150.f;
float	m_fDistantSndDistanceFar = 250.f;

namespace GameConstants
{
	void LoadConstants()
	{
		m_bDistantSoundsEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "distant_sounds_enabled", true);
		m_bTorchUseBattery = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "torch_use_battery", false);
		m_bArtefactDetectorUseBattery = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "artefact_detector_use_battery", false);
		m_bAnomalyDetectorUseBattery = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "anomaly_detector_use_battery", false);
		m_bKnifeSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_knife_slot", false);
		m_bBinocularSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_binocular_slot", false);
		m_bTorchSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_torch_slot", false);
		m_bBackpackSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_backpack_slot", false);
		m_bSecondHelmetSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_second_helmet_slot", false);
		m_bDosimeterSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_dosimeter_slot", false);
		m_bPantsSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_pants_slot", false);
		m_bPdaSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_pda_slot", false);
		m_bLimitedBolts = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "limited_bolts", false);
		m_bActorThirst = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "actor_thirst_enabled", false);
		m_bActorIntoxication = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "actor_intoxication_enabled", false);
		m_bActorSleepeness = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "actor_sleepeness_enabled", false);
		m_bActorAlcoholism = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "actor_alcoholism_enabled", false);
		m_bActorNarcotism = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "actor_narcotism_enabled", false);
		m_bArtefactsDegradation = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "artefacts_degradation", false);
		m_bMultiItemPickup = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "multi_item_pickup", true);
		m_bShowWpnInfo = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "show_wpn_info", true);
		m_bJumpSpeedWeightCalc = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "jump_and_speed_weight_calc", false);
		m_bHideWeaponInInventory = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "hide_weapon_in_inventory", false);
		m_bStopActorIfShoot = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "stop_actor_sprint_if_shoot", false);
		m_bReloadIfSprint = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "enable_reload_if_sprint", true);
		m_iArtefactsCount = READ_IF_EXISTS(pAdvancedSettings, r_u32, "inventory", "artefacts_count", 5);
		m_fDistantSndDistance = READ_IF_EXISTS(pAdvancedSettings, r_float, "gameplay", "distant_snd_distance", 150.f);
		m_fDistantSndDistanceFar = READ_IF_EXISTS(pAdvancedSettings, r_float, "gameplay", "distant_snd_distance_far", 250.f);

		Msg("# Advanced X-Ray GameConstants are loaded");
	}
	
	bool GetDistantSoundsEnabled()
	{
		return m_bDistantSoundsEnabled;
	}

	bool GetKnifeSlotEnabled()
	{
		return m_bKnifeSlotEnabled;
	}

	bool GetBinocularSlotEnabled()
	{
		return m_bBinocularSlotEnabled;
	}

	bool GetTorchSlotEnabled()
	{
		return m_bTorchSlotEnabled;
	}

	bool GetBackpackSlotEnabled()
	{
		return m_bBackpackSlotEnabled;
	}

	bool GetSecondHelmetSlotEnabled()
	{
		return m_bSecondHelmetSlotEnabled;
	}

	bool GetDosimeterSlotEnabled()
	{
		return m_bDosimeterSlotEnabled;
	}

	bool GetPantsSlotEnabled()
	{
		return m_bPantsSlotEnabled;
	}

	bool GetPdaSlotEnabled()
	{
		return m_bPdaSlotEnabled;
	}

	bool GetTorchHasBattery()
	{
		return m_bTorchUseBattery;
	}

	bool GetArtDetectorUseBattery()
	{
		return m_bArtefactDetectorUseBattery;
	}

	bool GetAnoDetectorUseBattery()
	{
		return m_bAnomalyDetectorUseBattery;
	}

	bool GetLimitedBolts()
	{
		return m_bLimitedBolts;
	}

	bool GetActorThirst()
	{
		return m_bActorThirst;
	}

	bool GetActorIntoxication()
	{
		return m_bActorIntoxication;
	}

	bool GetActorSleepeness()
	{
		return m_bActorSleepeness;
	}

	bool GetActorAlcoholism()
	{
		return m_bActorAlcoholism;
	}

	bool GetActorNarcotism()
	{
		return m_bActorNarcotism;
	}

	bool GetArtefactsDegradation()
	{
		return m_bArtefactsDegradation;
	}

	bool GetMultiItemPickup()
	{
		return m_bMultiItemPickup;
	}

	bool GetShowWpnInfo()
	{
		return m_bShowWpnInfo;
	}

	bool GetJumpSpeedWeightCalc()
	{
		return m_bJumpSpeedWeightCalc;
	}

	bool GetHideWeaponInInventory()
	{
		return m_bHideWeaponInInventory;
	}

	bool GetStopActorIfShoot()
	{
		return m_bStopActorIfShoot;
	}

	bool GetReloadIfSprint()
	{
		return m_bReloadIfSprint;
	}

	int GetArtefactsCount()
	{
		return m_iArtefactsCount;
	}

	float GetDistantSndDistance()
	{
		return m_fDistantSndDistance;
	}

	float GetDistantSndDistanceFar()
	{
		return m_fDistantSndDistanceFar;
	}
}
