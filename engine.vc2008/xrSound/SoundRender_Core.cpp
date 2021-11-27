#include "stdafx.h"
#pragma hdrstop

#include "../xrEngine/xrLevel.h"
#include "SoundRender_Core.h"
#include "SoundRender_Source.h"
#include "SoundRender_Emitter.h"

int psSoundTargets				= 256;
int psSoundCacheSizeMB			= 512;
Flags32	psSoundFlags			= { ss_Hardware | ss_EFX };
float	psSoundOcclusionScale	= 0.5f;
float	psSoundCull				= 0.01f;
float	psSoundRolloff			= 0.75f;
float	psSoundVEffects			= 1.0f;
float	psSoundVFactor			= 1.0f;
float	psSoundVMusic			= 1.f;
u32		psSoundModel			= 0;
int		psUseDistDelay			= 1;
float	psSoundSpeed			= 250.f;
u32		psSoundPrecacheAll		= 0;

XRSOUND_API CSoundRender_Core* SoundRender = nullptr;
CSound_manager_interface* Sound = nullptr;

#include <openal/efx.h>
#define LOAD_PROC(x, type)  ((x) = (type)alGetProcAddress(#x))
static LPALEFFECTF alEffectf;
static LPALEFFECTI alEffecti;
static LPALDELETEEFFECTS alDeleteEffects;
static LPALISEFFECT alIsEffect;
static LPALGENEFFECTS alGenEffects;
LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots;
LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti;

CSoundRender_Core::CSoundRender_Core	()
{
	efx_reverb					= EFX_REVERB_PRESET_GENERIC;
	bPresent					= FALSE;
	bEFX						= false;
	bUserEnvironment			= FALSE;
	geom_MODEL					= NULL;
	geom_ENV					= NULL;
	geom_SOM					= NULL;
	s_environment				= NULL;
	Handler						= NULL;
	s_targets_pu				= 0;
	s_emitters_u				= 0;
    e_current.set_identity		();
    e_target.set_identity		();
    bListenerMoved				= FALSE;
    bReady						= FALSE;
    bLocked						= FALSE;
	fTimer_Value				= Timer.GetElapsed_sec();
	fTimer_Delta				= 0.0f;
	m_iPauseCounter				= 1;
	effect						= 0;
	slot						= 0;
}

void CSoundRender_Core::InitAlEFXAPI()
{
	LOAD_PROC(alDeleteAuxiliaryEffectSlots, LPALDELETEAUXILIARYEFFECTSLOTS);
	LOAD_PROC(alGenEffects, LPALGENEFFECTS);
	LOAD_PROC(alDeleteEffects, LPALDELETEEFFECTS);
	LOAD_PROC(alIsEffect, LPALISEFFECT);
	LOAD_PROC(alEffecti, LPALEFFECTI);
	LOAD_PROC(alAuxiliaryEffectSloti, LPALAUXILIARYEFFECTSLOTI);
	LOAD_PROC(alGenAuxiliaryEffectSlots, LPALGENAUXILIARYEFFECTSLOTS);
	LOAD_PROC(alEffectf, LPALEFFECTF);
}

CSoundRender_Core::~CSoundRender_Core()
{
	if (bEFX)
	{
		if (effect > 1) alDeleteEffects(1, &effect);
		if (slot > 1)   alDeleteAuxiliaryEffectSlots(1, &slot);
	}

	xr_delete					(geom_ENV);
	xr_delete					(geom_SOM);
}

void CSoundRender_Core::_initialize(int stage)
{
	Timer.Start					( );

    // load environment
	env_load					();

	bPresent					= TRUE;

	// Cache
	cache_bytes_per_line		= (sdef_target_block/8)*276400/1000;
    cache.initialize			(psSoundCacheSizeMB*1024,cache_bytes_per_line);

    bReady						= TRUE;

	if (psSoundPrecacheAll == 1)
	{
		i_create_all_sources();
	}
}

extern xr_vector<u8> g_target_temp_data;
void CSoundRender_Core::_clear	()
{
    bReady						= FALSE;
	cache.destroy				();
	env_unload					();

    // remove sources
	for (u32 sit=0; sit<s_sources.size(); sit++)
    	xr_delete				(s_sources[sit]);
    s_sources.clear				();
    
    // remove emmiters
	for (u32 eit=0; eit<s_emitters.size(); eit++)
    	xr_delete				(s_emitters[eit]);
    s_emitters.clear			();

	g_target_temp_data.clear	();
}

void CSoundRender_Core::stop_emitters()
{
	for (u32 eit=0; eit<s_emitters.size(); eit++)
		s_emitters[eit]->stop	(FALSE);
}

int CSoundRender_Core::pause_emitters(bool val)
{
	m_iPauseCounter				+= val?+1:-1;
	VERIFY						(m_iPauseCounter>=0);

	for (u32 it=0; it<s_emitters.size(); it++)
		((CSoundRender_Emitter*)s_emitters[it])->pause	(val,val?m_iPauseCounter:m_iPauseCounter+1);

	return m_iPauseCounter;
}

bool CSoundRender_Core::EFXTestSupport()
{
	alGenEffects(1, &effect);

	alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
	alEffectf(effect, AL_REVERB_DENSITY, efx_reverb.flDensity);
	alEffectf(effect, AL_REVERB_DIFFUSION, efx_reverb.flDiffusion);
	alEffectf(effect, AL_REVERB_GAIN, efx_reverb.flGain);
	alEffectf(effect, AL_REVERB_GAINHF, efx_reverb.flGainHF);
	alEffectf(effect, AL_REVERB_DECAY_TIME, efx_reverb.flDecayTime);
	alEffectf(effect, AL_REVERB_DECAY_HFRATIO, efx_reverb.flDecayHFRatio);
	alEffectf(effect, AL_REVERB_REFLECTIONS_GAIN, efx_reverb.flReflectionsGain);
	alEffectf(effect, AL_REVERB_REFLECTIONS_DELAY, efx_reverb.flReflectionsDelay);
	alEffectf(effect, AL_REVERB_LATE_REVERB_GAIN, efx_reverb.flLateReverbGain);
	alEffectf(effect, AL_REVERB_LATE_REVERB_DELAY, efx_reverb.flLateReverbDelay);
	alEffectf(effect, AL_REVERB_AIR_ABSORPTION_GAINHF, efx_reverb.flAirAbsorptionGainHF);
	alEffectf(effect, AL_REVERB_ROOM_ROLLOFF_FACTOR, efx_reverb.flRoomRolloffFactor);
	alEffecti(effect, AL_REVERB_DECAY_HFLIMIT, efx_reverb.iDecayHFLimit);

	/* Check if an error occurred, and clean up if so. */
	ALenum err = alGetError();
	if (err != AL_NO_ERROR) {
		Msg("OpenAL error: %s", alGetString(err));
		if (alIsEffect(effect))
			alDeleteEffects(1, &effect);
		return false;
	}

	alGenAuxiliaryEffectSlots(1, &slot);
	err = alGetError();
	if (err != AL_NO_ERROR)
	{
		Msg("[OpenAL] EFX error: %s", alGetString(err));
		FATAL("Error during EFX initialization");
	}

	return true;
}


void CSoundRender_Core::env_load	()
{
	// Load environment
	string_path	fn;
	if (FS.exist(fn,"$game_data$",SNDENV_FILENAME))
	{
		s_environment				= xr_new<SoundEnvironment_LIB>();
		s_environment->Load			(fn);
	}

	// Load geometry

	// Assosiate geometry
}

void CSoundRender_Core::env_unload	()
{
	// Unload 
	if (s_environment)
		s_environment->Unload	();
	xr_delete					(s_environment);
}

void CSoundRender_Core::_restart		()
{
	cache.destroy				();
	cache.initialize			(psSoundCacheSizeMB*1024,cache_bytes_per_line);
	env_apply					();
}

void CSoundRender_Core::set_handler(sound_event* E)
{
	Handler			= E;
}

void CSoundRender_Core::set_geometry_occ(CDB::MODEL* M)
{
	geom_MODEL		= M;
}

void CSoundRender_Core::set_geometry_som(IReader* I)
{
#ifdef _EDITOR
	ETOOLS::destroy_model	(geom_SOM);
#else
	xr_delete				(geom_SOM);
#endif
	if (0==I)		return;

	// check version
	R_ASSERT		(I->find_chunk(0));
	u32 version		= I->r_u32(); 
	VERIFY2			(version==0,"Invalid SOM version");
	// load geometry	
	IReader* geom	= I->open_chunk(1); 
	VERIFY2			(geom,"Corrupted SOM file");
	// Load tris and merge them
	struct SOM_poly{
		Fvector3	v1;
		Fvector3	v2;
		Fvector3	v3;
		u32			b2sided;
		float		occ;
	};
	// Create AABB-tree

	CDB::Collector				CL;			
	while (!geom->eof()){
		SOM_poly				P;
		geom->r					(&P,sizeof(P));
		CL.add_face_packed_D	(P.v1,P.v2,P.v3,*(u32*)&P.occ,0.01f);
		if (P.b2sided)
			CL.add_face_packed_D(P.v3,P.v2,P.v1,*(u32*)&P.occ,0.01f);
	}
	geom_SOM			= xr_new<CDB::MODEL> ();
	geom_SOM->build		(CL.getV(),int(CL.getVS()),CL.getT(),int(CL.getTS()));

	geom->close();
}

void CSoundRender_Core::set_geometry_env(IReader* I)
{
#ifdef _EDITOR
	ETOOLS::destroy_model	(geom_ENV);
#else
	xr_delete				(geom_ENV);
#endif
	if (0==I)				return;
	if (0==s_environment)	return;

	// Assosiate names
	xr_vector<u16>			ids;
	IReader*				names	= I->open_chunk(0);
	while (!names->eof())
	{
		string256			n;
		names->r_stringZ	(n,sizeof(n));
		int id				= s_environment->GetID(n);
		R_ASSERT			(id>=0);
		ids.push_back		(u16(id));
	}
	names->close		();

	// Load geometry
	IReader*			geom_ch	= I->open_chunk(1);
	
	u8*	_data			= (u8*)xr_malloc(geom_ch->length());
	
	Memory.mem_copy		(_data, geom_ch->pointer(), geom_ch->length() );

	IReader* geom		= xr_new<IReader>(_data, geom_ch->length(), 0);
	
	hdrCFORM			H;
	geom->r				(&H,sizeof(hdrCFORM));
	Fvector*	verts	= (Fvector*)geom->pointer();
	CDB::TRI*	tris	= (CDB::TRI*)(verts+H.vertcount);
	for (u32 it=0; it<H.facecount; it++)
	{
		CDB::TRI*	T	= tris+it;
		u16		id_front= (u16)((T->dummy&0x0000ffff)>>0);		//	front face
		u16		id_back	= (u16)((T->dummy&0xffff0000)>>16);		//	back face
		R_ASSERT		(id_front<(u16)ids.size());
		R_ASSERT		(id_back<(u16)ids.size());
		T->dummy		= u32(ids[id_back]<<16) | u32(ids[id_front]);
	}
#ifdef _EDITOR    
	geom_ENV			= ETOOLS::create_model(verts, H.vertcount, tris, H.facecount);
	env_apply			();
#else
	geom_ENV			= xr_new<CDB::MODEL> ();
	geom_ENV->build		(verts, H.vertcount, tris, H.facecount);
#endif
	geom_ch->close			();
	geom->close				();
	xr_free					(_data);
}

void	CSoundRender_Core::create				( ref_sound& S, const char* fName, esound_type sound_type, int game_type )
{
	if (!bPresent)		return;
    S._p				= xr_new<ref_sound_data>(fName,sound_type,game_type);
}

void	CSoundRender_Core::attach_tail				( ref_sound& S, const char* fName)
{
	if (!bPresent)		return;
	string_path			fn;
	xr_strcpy			(fn,fName);
    if (strext(fn))		*strext(fn)	= 0;
	if (S._p->fn_attached[0].size() && S._p->fn_attached[1].size()) return;

	u32 idx = S._p->fn_attached[0].size()?1:0;
	
	S._p->fn_attached[idx]			= fn;

	CSoundRender_Source* s			= SoundRender->i_create_source(fn);
	S._p->dwBytesTotal				+= s->bytes_total();
	S._p->fTimeTotal				+= s->length_sec();
    if(S._feedback())
    	((CSoundRender_Emitter*)S._feedback())->fTimeToStop		+= s->length_sec();
}

void	CSoundRender_Core::clone				( ref_sound& S, const ref_sound& from, esound_type sound_type, int	game_type )
{
	if (!bPresent)		return;
	S._p				= xr_new<ref_sound_data>();
	S._p->handle		= from._p->handle;
	S._p->dwBytesTotal	= from._p->dwBytesTotal;
	S._p->fTimeTotal	= from._p->fTimeTotal;
	S._p->fn_attached[0]	= from._p->fn_attached[0];
	S._p->fn_attached[1]	= from._p->fn_attached[1];
	S._p->g_type		= (game_type==sg_SourceType)?S._p->handle->game_type():game_type;
	S._p->s_type		= sound_type;
}


void	CSoundRender_Core::play					( ref_sound& S, CObject* O, u32 flags, float delay)
{
	if (!bPresent || 0==S._handle())return;
	S._p->g_object		= O;
	if (S._feedback())	((CSoundRender_Emitter*)S._feedback())->rewind ();
	else				i_play					(&S,flags&sm_Looped,delay);

	if (flags&sm_2D || S._handle()->channels_num()==2)	
		S._feedback()->switch_to_2D();
}

void	CSoundRender_Core::play_no_feedback		( ref_sound& S, CObject* O, u32 flags, float delay, Fvector* pos, float* vol, float* freq, Fvector2* range)
{
	if (!bPresent || 0==S._handle())return;
	ref_sound_data_ptr	orig = S._p;
	S._p				= xr_new<ref_sound_data>();
	S._p->handle		= orig->handle;
	S._p->g_type		= orig->g_type;
	S._p->g_object		= O;
	S._p->dwBytesTotal	= orig->dwBytesTotal;
	S._p->fTimeTotal	= orig->fTimeTotal;
	S._p->fn_attached[0]	= orig->fn_attached[0];
	S._p->fn_attached[1]	= orig->fn_attached[1];

	i_play				(&S,flags&sm_Looped,delay);
	
	if (flags&sm_2D || S._handle()->channels_num()==2)	
		S._feedback()->switch_to_2D();

	if (pos)			S._feedback()->set_position	(*pos);
	if (freq)			S._feedback()->set_frequency(*freq);
	if (range)			S._feedback()->set_range   	((*range)[0],(*range)[1]);
	if (vol)			S._feedback()->set_volume   (*vol);
	S._p				= orig;
}

void	CSoundRender_Core::play_at_pos			( ref_sound& S, CObject* O, const Fvector &pos, u32 flags, float delay)
{
	if (!bPresent || 0 == S._handle())
		return;

	S._p->g_object = O;

	// Delay the sound starting, depending on distance from camer(sound speed feature)
	if (psUseDistDelay && !(flags&sm_2D)) // dont apply on 2d sounds
	{
		float dist_to_listener = SoundRender->listener_position().distance_to(pos);

		delay = dist_to_listener / psSoundSpeed;
	}

	if (S._feedback())
		((CSoundRender_Emitter*)S._feedback())->rewind();
	else
		i_play(&S, flags&sm_Looped, delay);

	S._feedback()->set_position(pos);

	if (flags&sm_2D || S._handle()->channels_num() == 2)
		S._feedback()->switch_to_2D();
}
void	CSoundRender_Core::destroy	(ref_sound& S )
{
	if (S._feedback()){                   
		CSoundRender_Emitter* E		= (CSoundRender_Emitter*)S._feedback();
		E->stop						(FALSE);
	}
	S._p				= 0;
}                                                    

void CSoundRender_Core::_create_data( ref_sound_data& S, LPCSTR fName, esound_type sound_type, int game_type)
{
	string_path			fn;
	xr_strcpy			(fn,fName);
    if (strext(fn))		*strext(fn)	= 0;
	S.handle			= (CSound_source*)SoundRender->i_create_source(fn);
	S.g_type			= (game_type==sg_SourceType)?S.handle->game_type():game_type;
	S.s_type			= sound_type;
	S.feedback			= 0; 
    S.g_object			= 0; 
    S.g_userdata		= 0;
	S.dwBytesTotal		= S.handle->bytes_total();
	S.fTimeTotal		= S.handle->length_sec();
}
void CSoundRender_Core::_destroy_data( ref_sound_data& S)
{
	if (S.feedback){                   
		CSoundRender_Emitter* E		= (CSoundRender_Emitter*)S.feedback;
		E->stop						(FALSE);
	}
	R_ASSERT						(0==S.feedback);
	
	S.handle						= NULL;
}

CSoundRender_Environment*	CSoundRender_Core::get_environment			( const Fvector& P )
{
	static CSoundRender_Environment	identity;

	if (!bUserEnvironment)
	{
		if (geom_ENV)
		{
			Fvector	dir				= {0,-1,0};
#ifdef _EDITOR
			ETOOLS::ray_options		(CDB::OPT_ONLYNEAREST);
			ETOOLS::ray_query		(geom_ENV,P,dir,1000.f);
			if (ETOOLS::r_count()){
				CDB::RESULT*		r	= ETOOLS::r_begin();
#else
			geom_DB.ray_options		(CDB::OPT_ONLYNEAREST);
			geom_DB.ray_query		(geom_ENV,P,dir,1000.f);
			if (geom_DB.r_count())
			{
				CDB::RESULT*		r	= geom_DB.r_begin();
#endif            
				CDB::TRI*		T = geom_ENV->get_tris() + r->id;
				Fvector*		V = geom_ENV->get_verts();
				Fvector tri_norm;
				tri_norm.mknormal		(V[T->verts[0]],V[T->verts[1]],V[T->verts[2]]);
				float	dot				= dir.dotproduct(tri_norm);
				if (dot < 0)
				{
					u16		id_front	= (u16)((T->dummy&0x0000ffff)>>0);		//	front face
					return	s_environment->Get(id_front);
				}
				else
				{
					u16		id_back		= (u16)((T->dummy&0xffff0000)>>16);	//	back face
					return	s_environment->Get(id_back);
				}
			}
		}
	}
	else return &s_user_environment;

	identity.set_identity();
	return &identity;
}

void CSoundRender_Core::env_apply		()
{
    bListenerMoved			= TRUE;
}

void CSoundRender_Core::update_listener( const Fvector& P, const Fvector& D, const Fvector& N, float dt )
{
}

bool CSoundRender_Core::i_efx_commit_setting()
{
	alGetError();
	/* Tell the effect slot to use the loaded effect object. Note that the this
	* effectively copies the effect properties. You can modify or delete the
	* effect object afterward without affecting the effect slot.
	*/
	alAuxiliaryEffectSloti(slot, AL_EFFECTSLOT_EFFECT, effect);
	ALenum err = alGetError();
	if (err != AL_NO_ERROR)
	{
		Msg("[OpenAL] EFX error: %s", alGetString(err));
		return false;
	}
	return true;
}

void CSoundRender_Core::object_relcase( CObject* obj )
{
	if (obj) for (u32 eit = 0; eit < s_emitters.size(); eit++)
	{
		if (s_emitters[eit] && s_emitters[eit]->owner_data)
			if (obj == s_emitters[eit]->owner_data->g_object)
				s_emitters[eit]->owner_data->g_object = 0;
    }
}

void CSoundRender_Core::i_efx_listener_set(CSound_environment* _E)
{
	const auto E = static_cast<CSoundRender_Environment*>(_E);

	// http://openal.org/pipermail/openal/2014-March/000083.html
	float density = powf(E->EnvironmentSize, 3.0f) / 16.0f;
	if (density > 1.0f)
		density = 1.0f;

	alEffectf(effect, AL_REVERB_DENSITY, density);
	alEffectf(effect, AL_REVERB_DIFFUSION, E->EnvironmentDiffusion);

	alEffectf(effect, AL_REVERB_GAIN, E->Room);
	alEffectf(effect, AL_REVERB_GAINHF, E->RoomHF);

	alEffectf(effect, AL_REVERB_DECAY_TIME, E->DecayTime);
	alEffectf(effect, AL_REVERB_DECAY_HFRATIO, E->DecayHFRatio);

	alEffectf(effect, AL_REVERB_REFLECTIONS_GAIN, E->Reflections);
	alEffectf(effect, AL_REVERB_REFLECTIONS_DELAY, E->ReflectionsDelay);

	alEffectf(effect, AL_REVERB_LATE_REVERB_GAIN, E->Reverb);
	alEffectf(effect, AL_REVERB_LATE_REVERB_DELAY, E->ReverbDelay);

	alEffectf(effect, AL_REVERB_AIR_ABSORPTION_GAINHF, E->AirAbsorptionHF);
	alEffectf(effect, AL_REVERB_ROOM_ROLLOFF_FACTOR, E->RoomRolloffFactor);
}




