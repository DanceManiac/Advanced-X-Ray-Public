#pragma once

#include "SoundRender_Core.h"            
#include "OpenALDeviceList.h"


#ifdef DEBUG
#	define A_CHK(expr)		{ alGetError(); 		expr; ALenum error=alGetError(); 			VERIFY2(error==AL_NO_ERROR, (LPCSTR)alGetString(error)); }
#	define AC_CHK(expr)		{ alcGetError(pDevice); expr; ALCenum error=alcGetError(pDevice); 	VERIFY2(error==ALC_NO_ERROR,(LPCSTR)alcGetString(pDevice,error)); }
#else
#	define A_CHK(expr)		{ expr; }
#	define AC_CHK(expr)		{ expr; }
#endif

class CSoundRender_CoreA: public CSoundRender_Core
{
	typedef CSoundRender_Core inherited;
	ALCdevice* 				pDevice;
    ALCcontext*				pContext;
	ALDeviceList*			pDeviceList;

	struct SListener{
		Fvector				position;
		Fvector				orientation[2];
	};
	SListener				Listener;

protected:
	virtual void			update_listener			( const Fvector& P, const Fvector& D, const Fvector& N, float dt );
public:	
						    CSoundRender_CoreA		();
    virtual					~CSoundRender_CoreA		();

	virtual void			_initialize				(int stage);
	virtual void			_clear					( );
	virtual void			_restart				( );
    
	virtual void			set_master_volume		( float f		);

	virtual const Fvector&	listener_position		( ){return Listener.position;}
};
extern CSoundRender_CoreA* SoundRenderA;