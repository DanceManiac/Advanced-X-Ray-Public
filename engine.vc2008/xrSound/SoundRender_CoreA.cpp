#include "stdafx.h"
#pragma hdrstop

#include "SoundRender_CoreA.h"
#include "SoundRender_TargetA.h"

CSoundRender_CoreA*	SoundRenderA= 0; 

CSoundRender_CoreA::CSoundRender_CoreA	():CSoundRender_Core()
{
	pDevice						= 0;
	pDeviceList					= 0;
	pContext					= 0;
}

CSoundRender_CoreA::~CSoundRender_CoreA	()
{
}

void  CSoundRender_CoreA::_restart()
{
	inherited::_restart();
}

void CSoundRender_CoreA::_initialize(int stage)
{
	if(stage==0)
	{
		pDeviceList					= xr_new<ALDeviceList>();

		if (0==pDeviceList->GetNumDevices())
		{ 
			CHECK_OR_EXIT			(0, "[OpenAL] Can't create sound device.");
			xr_delete				(pDeviceList);
		}
		return;
	}
	
	pDeviceList->SelectBestDevice	();
	R_ASSERT						(snd_device_id>=0 && snd_device_id<pDeviceList->GetNumDevices());
	const ALDeviceDesc& deviceDesc	= pDeviceList->GetDeviceDesc(snd_device_id);
    // OpenAL device
	// alcOpenDevice can fail without any visible reason. Just try several times
	for (DWORD i = 0; i < 100; ++i)
	{
		pDevice = alcOpenDevice(deviceDesc.name);
		if (pDevice != nullptr) break;
	}

	if (pDevice == NULL)
	{
		CHECK_OR_EXIT			(0,"[OpenAL] Failed to create context.");
		bPresent				= FALSE;
		return;
	}

    // Get the device specifier.
    const ALCchar*		        deviceSpecifier;
    deviceSpecifier         	= alcGetString		(pDevice, ALC_DEVICE_SPECIFIER);

    // Create context
    pContext					= alcCreateContext	(pDevice,NULL);
	if (0==pContext){
		CHECK_OR_EXIT			(0,"SOUND: OpenAL: Failed to create context.");
		bPresent				= FALSE;
		alcCloseDevice			(pDevice); pDevice = 0;
		return;
	}
    
    // clear errors
	alGetError					();
	alcGetError					(pDevice);
    
    // Set active context
    AC_CHK				        (alcMakeContextCurrent(pContext));

    // initialize listener
    A_CHK				        (alListener3f		(AL_POSITION,0.f,0.f,0.f));
    A_CHK				        (alListener3f		(AL_VELOCITY,0.f,0.f,0.f));
    Fvector	orient[2]	        = {{0.f,0.f,1.f},{0.f,1.f,0.f}};
    A_CHK				        (alListenerfv		(AL_ORIENTATION,&orient[0].x));
    A_CHK				        (alListenerf		(AL_GAIN,1.f));

	// Check for EFX extension
	if (deviceDesc.props.efx)
	{
		InitAlEFXAPI();
		bEFX = EFXTestSupport();
		Msg("[OpenAL] EFX: %s", bEFX ? "present" : "absent");
    }

    inherited::_initialize		(stage);

	if(stage==1)//first initialize
	{
		// Pre-create targets
		CSoundRender_Target*	T	= 0;
		for (u32 tit=0; tit<u32(psSoundTargets); tit++)
		{
			T						=	xr_new<CSoundRender_TargetA>();
			if (T->_initialize())
			{
				if (bEFX)T->alAuxInit(slot);
				s_targets.push_back(T);
			}
			else
			{
        		Log					("[OpenAL] ! SOUND: OpenAL: Max targets - ",tit);
				T->_destroy			();
        		xr_delete			(T);
        		break;
			}
		}
	}
}

void CSoundRender_CoreA::set_master_volume(float f )
{
	if (bPresent)				{
		A_CHK				    (alListenerf	(AL_GAIN,f));
	}
}

void CSoundRender_CoreA::_clear	()
{
	inherited::_clear			();
    // remove targets
	CSoundRender_Target*	T	= 0;
	for (u32 tit=0; tit<s_targets.size(); tit++)
	{
		T						= s_targets[tit];
		T->_destroy				();
        xr_delete				(T);
	}
    // Reset the current context to NULL.
    alcMakeContextCurrent		(NULL);         
    // Release the context and the device.
    alcDestroyContext			(pContext);		pContext	= 0;
    alcCloseDevice				(pDevice);		pDevice		= 0;
	xr_delete					(pDeviceList);
}

void CSoundRender_CoreA::update_listener		( const Fvector& P, const Fvector& D, const Fvector& N, float dt )
{
	inherited::update_listener(P,D,N,dt);

	if (!Listener.position.similar(P)){
		Listener.position.set	(P);
		bListenerMoved			= TRUE;
	}
	Listener.orientation[0].set	(D.x,D.y,-D.z);
	Listener.orientation[1].set	(N.x,N.y,-N.z);

	A_CHK						(alListener3f	(AL_POSITION,Listener.position.x,Listener.position.y,-Listener.position.z));
	A_CHK						(alListener3f	(AL_VELOCITY,0.f,0.f,0.f));
	A_CHK						(alListenerfv	(AL_ORIENTATION,&Listener.orientation[0].x));
}
