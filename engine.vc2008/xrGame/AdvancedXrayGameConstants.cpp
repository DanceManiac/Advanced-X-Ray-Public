#include "StdAfx.h"
#include "AdvancedXrayGameConstants.h"
#include "GamePersistent.h"

bool	m_bDistantSoundsEnabled = true;
float	m_fDistantSndDistance = 150.f;

namespace GameConstants
{
	void LoadConstants()
	{
		m_bDistantSoundsEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "distant_sounds_enabled", true);
		m_fDistantSndDistance = READ_IF_EXISTS(pAdvancedSettings, r_float, "gameplay", "distant_snd_distance", 150.f);

		Msg("# Advanced X-Ray GameConstants are loaded");
	}
	
	bool GetDistantSoundsEnabled()
	{
		return m_bDistantSoundsEnabled;
	}

	float GetDistantSndDistance()
	{
		return m_fDistantSndDistance;
	}
}
