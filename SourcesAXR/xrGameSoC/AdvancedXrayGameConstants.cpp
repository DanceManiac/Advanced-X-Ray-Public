#include "StdAfx.h"
#include "AdvancedXrayGameConstants.h"
#include "GamePersistent.h"
#include "game_cl_single.h"
#include "Actor.h"
#include "Inventory.h"
#include "CustomBackpack.h"

bool	m_bKnifeSlotEnabled = false;
bool	m_bBinocularSlotEnabled = false;
bool	m_bTorchSlotEnabled = false;
bool	m_bBackpackSlotEnabled = false;
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
bool	m_bActorFrostbite = false;
bool	m_bArtefactsDegradation = false;
bool	m_bShowWpnInfo = true;
bool	m_bJumpSpeedWeightCalc = false;
bool	m_bHideWeaponInInventory = false;
bool	m_bStopActorIfShoot = false;
bool	m_bReloadIfSprint = true;
bool	m_bColorizeValues = false;
bool	m_bArtefactsRanks = false;
bool	m_bUseFilters = false;
bool	m_bHideHudOnMaster = false;
bool	m_bActorSkills = false;
bool	m_bSleepInfluenceOnPsyHealth = false;
bool	m_bUseHQ_Icons = false;
bool	m_bAfPanelEnabled = false;
bool	m_bHUD_UsedItemText = true;
bool	m_bLimitedInventory = false;
bool	m_bInventoryItemsAutoVolume = false;
bool	m_bFoodIrradiation = false;
bool	m_bFoodRotting = false;
bool	m_bDisableStopping = true;
bool	m_bDisableStoppingBolt = true;
bool	m_bDisableStoppingGrenade = true;
bool	m_bMergeAmmoLineWithFiremode = false;
bool	m_bUseAutoAmmoInfo = false;
bool	m_bUseAutoAmmoInfoDesc = false;
bool	m_bOGSE_WpnZoomSystem = false;
bool	m_bQuickThrowGrenadesEnabled = true;
bool	m_bPDA_FlashingIconsEnabled = false;
bool	m_bPDA_FlashingIconsQuestsEnabled = false;
bool	m_bFogInfluenceVolumetricLight = false;
bool	m_bEnableBoreDoF = true;
bool	m_bBackpackAnimsEnabled = false;
bool	m_bShowSaveName = false;
bool	m_bLimitedInvBoxes = false;
BOOL	m_b_animated_backpack = 0;
int		m_iArtefactsCount = 5;
int		m_i_CMD_Count = 1;
int		m_B_CMD_Count = 1;
float	m_fDistantSndDistance = 150.f;
Ivector4 m_IV4RedColor = Ivector4().set(255, 0, 0, 255);
Ivector4 m_IV4GreenColor = Ivector4().set(0, 255, 0, 255);
Ivector4 m_IV4NeutralColor = Ivector4().set(170, 170, 170, 255);
LPCSTR	m_sAfInfluenceMode = "from_belt";
LPCSTR	m_sArtefactsDegradationMode = "from_belt";
shared_str	m_sMoonPhasesMode = "off";
//SSFX DoF
Fvector4 m_FV4DefaultDoF = Fvector4().set(0.1f, 0.25f, 0.0f, 0.0f);
Fvector4 m_FV4FocusDoF = Fvector4().set(0.1f, 0.25f, 0.0f, 0.0f);

namespace GameConstants
{
	void LoadConstants()
	{
		m_bTorchUseBattery = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "torch_use_battery", false);
		m_bArtefactDetectorUseBattery = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "detector_use_battery", false); // Пока просто detector (аномалий)
		m_bAnomalyDetectorUseBattery = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "anomaly_detector_use_battery", false);
		m_bKnifeSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_knife_slot", false);
		m_bBinocularSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_binocular_slot", false);
		m_bTorchSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_torch_slot", false);
		m_bBackpackSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_backpack_slot", false);
		m_bDosimeterSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_dosimeter_slot", false);
		m_bPantsSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_pants_slot", false);
		m_bPdaSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_pda_slot", false);
		m_bLimitedBolts = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "limited_bolts", false);
		m_bActorThirst = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "actor_thirst_enabled", false);
		m_bActorIntoxication = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "actor_intoxication_enabled", false);
		m_bActorSleepeness = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "actor_sleepeness_enabled", false);
		m_bActorAlcoholism = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "actor_alcoholism_enabled", false);
		m_bActorNarcotism = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "actor_narcotism_enabled", false);
		m_bActorFrostbite = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "actor_frostbite_enabled", false);
		m_bArtefactsDegradation = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "artefacts_degradation", false);
		m_bShowWpnInfo = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "show_wpn_info", true);
		m_bJumpSpeedWeightCalc = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "jump_and_speed_weight_calc", false);
		m_bHideWeaponInInventory = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "hide_weapon_in_inventory", false);
		m_bStopActorIfShoot = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "stop_actor_sprint_if_shoot", false);
		m_bReloadIfSprint = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "enable_reload_if_sprint", true);
		m_bArtefactsRanks = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "enable_artefacts_ranks", false);
		m_bUseFilters = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "enable_antigas_filters", false);
		m_bHideHudOnMaster = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "hide_hud_on_master", false);
		m_bActorSkills = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "actor_skills_enabled", false);
		m_bSleepInfluenceOnPsyHealth = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "sleepeness_infl_on_psy_health", false);
		m_bLimitedInventory = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "enable_limited_inventory", false);
		m_bInventoryItemsAutoVolume = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "items_auto_volume", false);
		m_bFoodIrradiation = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "enable_food_irradiation", false);
		m_bFoodRotting = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "enable_food_rotting", false);
		m_bDisableStopping = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "disable_stopping_empty", true);
		m_bDisableStoppingBolt = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "disable_stopping_bolt", true);
		m_bDisableStoppingGrenade = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "disable_stopping_grenade", true);
		m_bOGSE_WpnZoomSystem = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "ogse_weapons_zoom_system", false);
		m_bQuickThrowGrenadesEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "enable_quick_throw_grenades", true);
		m_bBackpackAnimsEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "actions_animations", "enable_backpack_animations", false);
		m_iArtefactsCount = READ_IF_EXISTS(pAdvancedSettings, r_u32, "inventory", "artefacts_count", 5);
		m_i_CMD_Count = READ_IF_EXISTS(pAdvancedSettings, r_u32, "custom_commands", "integer_cmd_count", 1);
		m_B_CMD_Count = READ_IF_EXISTS(pAdvancedSettings, r_u32, "custom_commands", "bool_cmd_count", 1);
		m_fDistantSndDistance = READ_IF_EXISTS(pAdvancedSettings, r_float, "gameplay", "distant_snd_distance", 150.f);
		m_bMergeAmmoLineWithFiremode = READ_IF_EXISTS(pAdvancedSettings, r_bool, "ui_settings", "merge_ammo_line_with_firemode", true);
		m_bUseAutoAmmoInfo = READ_IF_EXISTS(pAdvancedSettings, r_bool, "ui_settings", "use_auto_ammo_info", false);
		m_bUseAutoAmmoInfoDesc = READ_IF_EXISTS(pAdvancedSettings, r_bool, "ui_settings", "use_auto_ammo_info_desc", false);
		m_bColorizeValues = READ_IF_EXISTS(pAdvancedSettings, r_bool, "ui_settings", "colorize_values", true);
		m_IV4RedColor = READ_IF_EXISTS(pAdvancedSettings, r_ivector4, "ui_settings", "colorize_values_red", Ivector4().set(255, 0, 0, 255));
		m_IV4GreenColor = READ_IF_EXISTS(pAdvancedSettings, r_ivector4, "ui_settings", "colorize_values_green", Ivector4().set(0, 255, 0, 255));
		m_IV4NeutralColor = READ_IF_EXISTS(pAdvancedSettings, r_ivector4, "ui_settings", "colorize_values_neutral", Ivector4().set(170, 170, 170, 255));
		m_bUseHQ_Icons = READ_IF_EXISTS(pAdvancedSettings, r_bool, "ui_settings", "hq_icons", false);
		m_bAfPanelEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "ui_settings", "enable_artefact_panel", false);
		m_bHUD_UsedItemText = READ_IF_EXISTS(pAdvancedSettings, r_bool, "ui_settings", "enable_hud_used_item_text", true);
		m_bPDA_FlashingIconsEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "ui_settings", "enable_pda_info_icons", false);
		m_bPDA_FlashingIconsQuestsEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "ui_settings", "enable_new_task_icon", false);
		m_bShowSaveName = READ_IF_EXISTS(pAdvancedSettings, r_bool, "ui_settings", "show_saved_game_name", false);
		m_sAfInfluenceMode = READ_IF_EXISTS(pAdvancedSettings, r_string, "gameplay", "artefacts_infl_mode", "from_belt"); //from_belt|from_ruck|from_ruck_only_rad
		m_sArtefactsDegradationMode = READ_IF_EXISTS(pAdvancedSettings, r_string, "gameplay", "artefacts_degradation_mode", "from_belt"); //from_belt|from_ruck
		m_FV4DefaultDoF = READ_IF_EXISTS(pAdvancedSettings, r_fvector4, "ssfx_dof", "default_dof", Fvector4().set(0.1f, 0.25f, 0.0f, 0.0f));
		m_FV4FocusDoF = READ_IF_EXISTS(pAdvancedSettings, r_fvector4, "ssfx_dof", "focus_dof", Fvector4().set(0.1f, 0.25f, 0.0f, 0.0f));
		m_bEnableBoreDoF = READ_IF_EXISTS(pAdvancedSettings, r_bool, "ssfx_dof", "bore_dof_enabled", true);
		m_sMoonPhasesMode = READ_IF_EXISTS(pAdvancedSettings, r_string, "environment", "moon_phases_mode", "off"); //off|8days|28days
		m_bFogInfluenceVolumetricLight = READ_IF_EXISTS(pAdvancedSettings, r_bool, "environment", "fog_infl_volumetric_light", false);
		m_bLimitedInvBoxes = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "enable_limited_inv_boxes", false);

		Msg("# Advanced X-Ray GameConstants are loaded");
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

	bool GetActorFrostbite()
	{
		return m_bActorFrostbite;
	}

	bool GetArtefactsDegradation()
	{
		return m_bArtefactsDegradation;
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
		if (GetBackpackAnimsEnabled())
		{
			//if (smart_cast<CCustomBackpack*>(Actor()->inventory().ItemFromSlot(BACKPACK_SLOT)))
			return false;
			//else
			//	return m_bHideWeaponInInventory;
		}

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

	bool GetColorizeValues()
	{
		return m_bColorizeValues;
	}

	bool GetAfRanks()
	{
		return m_bArtefactsRanks;
	}

	bool GetOutfitUseFilters()
	{
		return m_bUseFilters;
	}

	bool GetHideHudOnMaster()
	{
		return m_bHideHudOnMaster && g_SingleGameDifficulty == egdMaster;
	}

	bool GetActorSkillsEnabled()
	{
		return m_bActorSkills;
	}

	bool GetSleepInfluenceOnPsyHealth()
	{
		return m_bSleepInfluenceOnPsyHealth;
	}

	bool GetArtefactPanelEnabled()
	{
		return m_bAfPanelEnabled;
	}

	bool GetHUD_UsedItemTextEnabled()
	{
		return m_bHUD_UsedItemText;
	}

	bool GetFoodIrradiation()
	{
		return m_bFoodIrradiation;
	}

	bool GetFoodRotting()
	{
		return m_bFoodRotting;
	}

	bool GetDisableStopping()
	{
		return m_bDisableStopping;
	}

	bool GetDisableStoppingBolt()
	{
		return m_bDisableStoppingBolt;
	}

	bool GetDisableStoppingGrenade()
	{
		return m_bDisableStoppingGrenade;
	}

	bool GetMergedAmmoLineWithFiremodes()
	{
		return m_bMergeAmmoLineWithFiremode;
	}

	bool GetAutoAmmoInfo()
	{
		return m_bUseAutoAmmoInfo;
	}

	bool GetAutoAmmoInfoDesc()
	{
		return m_bUseAutoAmmoInfoDesc;
	}

	bool GetOGSE_WpnZoomSystem()
	{
		return m_bOGSE_WpnZoomSystem;
	}

	bool GetQuickThrowGrenadesEnabled()
	{
		return m_bQuickThrowGrenadesEnabled;
	}

	bool GetLimitedInvBoxes()
	{
		return m_bLimitedInvBoxes;
	}

	int GetArtefactsCount()
	{
		return m_iArtefactsCount;
	}

	int GetIntScriptCMDCount()
	{
		return m_i_CMD_Count;
	}

	int GetBOOLScriptCMDCount()
	{
		return m_B_CMD_Count;
	}

	float GetDistantSndDistance()
	{
		return m_fDistantSndDistance;
	}

	bool GetUseHQ_Icons()
	{
		return m_bUseHQ_Icons;
	}

	bool GetLimitedInventory()
	{
		return m_bLimitedInventory;
	}

	bool GetInventoryItemsAutoVolume()
	{
		return m_bInventoryItemsAutoVolume;
	}

	bool GetBackpackAnimsEnabled()
	{
		return (m_bBackpackAnimsEnabled && m_b_animated_backpack);
	}

	bool GetPDA_FlashingIconsEnabled()
	{
		return m_bPDA_FlashingIconsEnabled;
	}

	bool GetPDA_FlashingIconsQuestsEnabled()
	{
		return m_bPDA_FlashingIconsQuestsEnabled;
	}

	bool GetFogInfluenceVolumetricLight()
	{
		return m_bFogInfluenceVolumetricLight;
	}

	bool GetShowSaveName()
	{
		return m_bShowSaveName;
	}

	Ivector4 GetRedColor()
	{
		return m_IV4RedColor;
	}

	Ivector4 GetGreenColor()
	{
		return m_IV4GreenColor;
	}

	Ivector4 GetNeutralColor()
	{
		return m_IV4NeutralColor;
	}

	Fvector4 GetSSFX_DefaultDoF()
	{
		return m_FV4DefaultDoF;
	}

	Fvector4 GetSSFX_FocusDoF()
	{
		return m_FV4FocusDoF;
	}

	bool GetSSFX_EnableBoreDoF()
	{
		return m_bEnableBoreDoF;
	}

	LPCSTR GetAfInfluenceMode()
	{
		return m_sAfInfluenceMode;
	}

	LPCSTR GetArtefactDegradationMode()
	{
		return m_sArtefactsDegradationMode;
	}

	shared_str GetMoonPhasesMode()
	{
		return m_sMoonPhasesMode;
	}
}
