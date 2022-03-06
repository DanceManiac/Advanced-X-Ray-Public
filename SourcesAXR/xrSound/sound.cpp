#include "stdafx.h"
#pragma hdrstop

#include "SoundRender_CoreA.h"

XRSOUND_API xr_token*		snd_devices_token	= NULL;
XRSOUND_API u32				snd_device_id		= u32(-1);
void CSound_manager_interface::_create(int stage)
{
	if(stage==0)
	{
		SoundRenderA	= xr_new<CSoundRender_CoreA>();
		SoundRender		= SoundRenderA;
		Sound			= SoundRender;
		SoundRender->bPresent = !(strstr(Core.Params, "-nosound"));
	}

	if (SoundRender->bPresent)
	{
		Sound->_initialize(stage);
	}
}

void CSound_manager_interface::_destroy	()
{
	Sound->_clear		();
    xr_delete			(SoundRender);
	Sound				= nullptr;
}