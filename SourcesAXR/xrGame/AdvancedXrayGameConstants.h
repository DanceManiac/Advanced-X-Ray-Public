#pragma once

namespace GameConstants
{
	void LoadConstants();

	bool GetKnifeSlotEnabled();
	bool GetBinocularSlotEnabled();
	bool GetTorchSlotEnabled();
	bool GetBackpackSlotEnabled();
	bool GetSecondHelmetSlotEnabled();
	bool GetDosimeterSlotEnabled();
	bool GetPantsSlotEnabled();
	bool GetPdaSlotEnabled();
	bool GetTorchHasBattery();
	bool GetArtDetectorUseBattery();
	bool GetAnoDetectorUseBattery();
	bool GetLimitedBolts();
	bool GetActorThirst();
	bool GetActorIntoxication();
	bool GetActorSleepeness();
	bool GetActorAlcoholism();
	bool GetActorNarcotism();
	bool GetArtefactsDegradation();
	bool GetMultiItemPickup();
	bool GetShowWpnInfo();
	bool GetJumpSpeedWeightCalc();
	bool GetHideWeaponInInventory();
	bool GetStopActorIfShoot();
	bool GetReloadIfSprint();
	bool GetColorizeValues();
	bool GetAfRanks();
	bool GetOutfitUseFilters();
	bool GetHideHudOnMaster();
	bool GetActorSkillsEnabled();
	int  GetArtefactsCount();
	int  GetIntScriptCMDCount();
	int  GetBOOLScriptCMDCount();
	Fvector4 GetRedColor();
	Fvector4 GetGreenColor();
	Fvector4 GetNeutralColor();
	Fvector4 GetSSFX_DefaultDoF();
	Fvector4 GetSSFX_FocusDoF();
	LPCSTR GetAfInfluenceMode();
};
