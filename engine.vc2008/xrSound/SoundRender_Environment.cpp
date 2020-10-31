#include "stdafx.h"
#pragma hdrstop

#include "SoundRender.h"
#include "SoundRender_Environment.h"
#pragma warning(push)
#pragma warning(disable:4995)
#include <openal\efx.h>
#pragma warning(pop)

CSoundRender_Environment::CSoundRender_Environment(void)
{
	version			= sdef_env_version;
	set_default		();
}

CSoundRender_Environment::~CSoundRender_Environment(void)
{
}

void CSoundRender_Environment::set_default	()
{
	Environment			= 0u;
	Room				= AL_REVERB_DEFAULT_GAIN;
	RoomHF				= AL_REVERB_DEFAULT_GAINHF;
	RoomRolloffFactor	= AL_REVERB_DEFAULT_ROOM_ROLLOFF_FACTOR;
	DecayTime			= AL_REVERB_DEFAULT_DECAY_TIME;
	DecayHFRatio		= AL_REVERB_DEFAULT_DECAY_HFRATIO;
	Reflections			= AL_REVERB_DEFAULT_REFLECTIONS_GAIN;
	ReflectionsDelay	= AL_REVERB_DEFAULT_REFLECTIONS_DELAY;
	Reverb				= AL_REVERB_DEFAULT_LATE_REVERB_GAIN;
	ReverbDelay			= AL_REVERB_DEFAULT_LATE_REVERB_DELAY;
	EnvironmentSize		= 7.5f;
	EnvironmentDiffusion= AL_REVERB_DEFAULT_DIFFUSION;
	AirAbsorptionHF		= AL_REVERB_DEFAULT_AIR_ABSORPTION_GAINHF;

}

void CSoundRender_Environment::set_identity	()
{
	set_default				();
	Room				= AL_REVERB_MIN_GAIN;
	clamp				  	();
}

void CSoundRender_Environment::lerp			(CSoundRender_Environment& A, CSoundRender_Environment& B, float f)
{
	float	fi				= 1.f-f;

    Room                    = fi*A.Room                	+ f*B.Room;                
    RoomHF                  = fi*A.RoomHF              	+ f*B.RoomHF;              
    RoomRolloffFactor       = fi*A.RoomRolloffFactor   	+ f*B.RoomRolloffFactor;
    DecayTime               = fi*A.DecayTime           	+ f*B.DecayTime;           
    DecayHFRatio            = fi*A.DecayHFRatio        	+ f*B.DecayHFRatio;        
    Reflections             = fi*A.Reflections         	+ f*B.Reflections;         
    ReflectionsDelay        = fi*A.ReflectionsDelay    	+ f*B.ReflectionsDelay;    
    Reverb                  = fi*A.Reverb              	+ f*B.Reverb;              
    ReverbDelay             = fi*A.ReverbDelay         	+ f*B.ReverbDelay;         
    EnvironmentSize         = fi*A.EnvironmentSize     	+ f*B.EnvironmentSize;     
    EnvironmentDiffusion    = fi*A.EnvironmentDiffusion	+ f*B.EnvironmentDiffusion;
    AirAbsorptionHF         = fi*A.AirAbsorptionHF     	+ f*B.AirAbsorptionHF;     

	clamp					();
}

/*
void CSoundRender_Environment::get			(EAXLISTENERPROPERTIES& ep)
{
    ep.lRoom					= iFloor(Room)					;	// room effect level at low frequencies
    ep.lRoomHF					= iFloor(RoomHF)				;   // room effect high-frequency level re. low frequency level
    ep.flRoomRolloffFactor		= RoomRolloffFactor				;   // like DS3D flRolloffFactor but for room effect
    ep.flDecayTime				= DecayTime						;   // reverberation decay time at low frequencies
    ep.flDecayHFRatio			= DecayHFRatio					;   // high-frequency to low-frequency decay time ratio
    ep.lReflections				= iFloor(Reflections)			;   // early reflections level relative to room effect
    ep.flReflectionsDelay		= ReflectionsDelay				;   // initial reflection delay time
    ep.lReverb					= iFloor(Reverb)	 			;   // late reverberation level relative to room effect
    ep.flReverbDelay			= ReverbDelay					;   // late reverberation delay time relative to initial reflection
    ep.dwEnvironment			= EAXLISTENER_DEFAULTENVIRONMENT;  	// sets all listener properties
    ep.flEnvironmentSize		= EnvironmentSize				;  	// environment size in meters
    ep.flEnvironmentDiffusion	= EnvironmentDiffusion			; 	// environment diffusion
    ep.flAirAbsorptionHF		= AirAbsorptionHF				;	// change in level per meter at 5 kHz
    ep.dwFlags					= EAXLISTENER_DEFAULTFLAGS		;	// modifies the behavior of properties
}
*/
void CSoundRender_Environment::clamp		()
{
	::clamp(Room, AL_REVERB_MIN_GAIN, AL_REVERB_MAX_GAIN);
	::clamp(RoomHF, AL_REVERB_MIN_GAINHF, AL_REVERB_MAX_GAINHF);
	::clamp(RoomRolloffFactor, AL_REVERB_MIN_ROOM_ROLLOFF_FACTOR, AL_REVERB_MAX_ROOM_ROLLOFF_FACTOR);
	::clamp(DecayTime, AL_REVERB_MIN_DECAY_TIME, AL_REVERB_MAX_DECAY_TIME);
	::clamp(DecayHFRatio, AL_REVERB_MIN_DECAY_HFRATIO, AL_REVERB_MAX_DECAY_HFRATIO);
	::clamp(Reflections, AL_REVERB_MIN_REFLECTIONS_GAIN, AL_REVERB_MAX_REFLECTIONS_GAIN);
	::clamp(ReflectionsDelay, AL_REVERB_MIN_REFLECTIONS_DELAY, AL_REVERB_MAX_REFLECTIONS_DELAY);
	::clamp(Reverb, AL_REVERB_MIN_LATE_REVERB_GAIN, AL_REVERB_MAX_LATE_REVERB_GAIN);
	::clamp(ReverbDelay, AL_REVERB_MIN_LATE_REVERB_DELAY, AL_REVERB_MAX_LATE_REVERB_DELAY);
	::clamp(AirAbsorptionHF, AL_REVERB_MIN_AIR_ABSORPTION_GAINHF, AL_REVERB_MAX_AIR_ABSORPTION_GAINHF);

	::clamp(EnvironmentDiffusion, 0.f, 1.f);
	::clamp(EnvironmentSize, 1.f, 100.f);
}

bool CSoundRender_Environment::load			(IReader* fs)
{
	version = fs->r_u32();

	auto mB_to_gain = [this](float mb)
	{
		return version >= 0x0005 ? mb : powf(10.0f, mb / 2000.0f);
	};

	if (version >= 0x0003)
	{
        fs->r_stringZ			    (name);

		Room						= mB_to_gain(fs->r_float());
		RoomHF						= mB_to_gain(fs->r_float());
        RoomRolloffFactor   		= fs->r_float();
        DecayTime           		= fs->r_float();
        DecayHFRatio        		= fs->r_float();
		Reflections					= mB_to_gain(fs->r_float());
        ReflectionsDelay    		= fs->r_float();
		Reverb						= mB_to_gain(fs->r_float());
        ReverbDelay         		= fs->r_float();
        EnvironmentSize     		= fs->r_float();
        EnvironmentDiffusion		= fs->r_float();
		AirAbsorptionHF				= mB_to_gain(fs->r_float());
		if (version >= 0x0004)
	        Environment     		= fs->r_u32();
        return true;
    }
    return false;
}

void CSoundRender_Environment::save	(IWriter* fs)
{
	fs->w_u32 	                    (sdef_env_version);
	fs->w_stringZ                   (name);

    fs->w_float	                    (Room                );
    fs->w_float	                    (RoomHF              );
    fs->w_float	                    (RoomRolloffFactor   );
    fs->w_float	                    (DecayTime           );
    fs->w_float	                    (DecayHFRatio        );
    fs->w_float	                    (Reflections         );
    fs->w_float	                    (ReflectionsDelay    );
    fs->w_float	                    (Reverb              );
    fs->w_float	                    (ReverbDelay         );
    fs->w_float	                    (EnvironmentSize     );
    fs->w_float	                    (EnvironmentDiffusion);
    fs->w_float	                    (AirAbsorptionHF     );

	fs->w_u32						(Environment		 );
}

//////////////////////////////////////////////////////////////////////////
void	SoundEnvironment_LIB::Load	(LPCSTR name)
{
	R_ASSERT			(library.empty());
	IReader* F			= FS.r_open(name);
	IReader* C;
	library.reserve		(256);
	for (u32 chunk=0; 0!=(C=F->open_chunk(chunk)); chunk++)
	{
		CSoundRender_Environment*	E	= xr_new<CSoundRender_Environment>	();
		if (E->load(C))	library.push_back(E);
        C->close		();
	}
	FS.r_close			(F);
}
bool	SoundEnvironment_LIB::Save	(LPCSTR name)
{
	IWriter* F			= FS.w_open(name);
    if (F){
        for (u32 chunk=0; chunk<library.size(); chunk++)
        {
            F->open_chunk		(chunk);
            library[chunk]->save(F);
            F->close_chunk		();
        }
        FS.w_close		(F);
        return 			true;
    }   
    return 				false;
}
void	SoundEnvironment_LIB::Unload	()
{
	for (u32 chunk=0; chunk<library.size(); chunk++)
		xr_delete(library[chunk]);
	library.clear		();
}
int		SoundEnvironment_LIB::GetID		(LPCSTR name)
{
	for (SE_IT it=library.begin(); it!=library.end(); it++)
		if (0==stricmp(name,*(*it)->name)) return int(it-library.begin());
	return -1;
}
CSoundRender_Environment*	SoundEnvironment_LIB::Get		(LPCSTR name)
{
	for (SE_IT it=library.begin(); it!=library.end(); it++)
		if (0==stricmp(name,*(*it)->name)) return *it;
	return NULL;
}
CSoundRender_Environment*	SoundEnvironment_LIB::Get		(int id)
{
	return library[id];
}
CSoundRender_Environment*	SoundEnvironment_LIB::Append	(CSoundRender_Environment* parent)
{
	library.push_back	(parent?xr_new<CSoundRender_Environment>(*parent):xr_new<CSoundRender_Environment>());
	return library.back	();
}
void						SoundEnvironment_LIB::Remove	(LPCSTR name)
{
	for (SE_IT it=library.begin(); it!=library.end(); it++)
		if (0==stricmp(name,*(*it)->name))
		{
			xr_delete		(*it);
			library.erase	(it);
			break;
		}
}
void						SoundEnvironment_LIB::Remove	(int id)
{
	xr_delete		(library[id]);
	library.erase	(library.begin()+id);
}
SoundEnvironment_LIB::SE_VEC& SoundEnvironment_LIB::Library	()	
{ 
	return library;
}
