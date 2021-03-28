#include "StdAfx.h"
#include "AdvancedXrayGameConstants.h"
#include "GamePersistent.h"

bool	m_bDistantSoundsEnabled = true;
bool	m_bKnifeSlotEnabled = false;
bool	m_bBinocularSlotEnabled = false;
float	m_fDistantSndDistance = 150.f;
float	m_fDistantSndDistanceFar = 250.f;

namespace GameConstants
{
	void LoadConstants()
	{
		m_bDistantSoundsEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "distant_sounds_enabled", true);
		m_bKnifeSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_knife_slot", false);
		m_bBinocularSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_binocular_slot", false);
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

	float GetDistantSndDistance()
	{
		return m_fDistantSndDistance;
	}

	float GetDistantSndDistanceFar()
	{
		return m_fDistantSndDistanceFar;
	}
}
