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
	bool GetArtefactsDegradation();
	bool GetShowWpnInfo();
	bool GetJumpSpeedWeightCalc();
	float GetDistantSndDistance();
	float GetDistantSndDistanceFar();
};
