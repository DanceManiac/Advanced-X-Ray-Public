#pragma once

namespace GameConstants
{
	void LoadConstants();

	bool GetDistantSoundsEnabled();
	bool GetKnifeSlotEnabled();
	bool GetBinocularSlotEnabled();
	bool GetTorchSlotEnabled();
	bool GetBackpackSlotEnabled();
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
	bool GetShowWpnInfo();
	bool GetJumpSpeedWeightCalc();
	bool GetHideWeaponInInventory();
	bool GetStopActorIfShoot();
	bool GetReloadIfSprint();
	bool GetColorizeValues();
	bool GetAfRanks();
	bool GetOutfitUseFilters();
	int  GetArtefactsCount();
	float GetDistantSndDistance();
	float GetDistantSndDistanceFar();
	Fvector4 GetRedColor();
	Fvector4 GetGreenColor();
	Fvector4 GetNeutralColor();
	LPCSTR GetAfInfluenceMode();
};
