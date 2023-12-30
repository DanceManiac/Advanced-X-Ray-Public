// Rain.h: interface for the CRain class.
//
//////////////////////////////////////////////////////////////////////

#ifndef RainH
#define RainH
#pragma once

#include "../xrcdb/xr_collide_defs.h"
#include "x_ray.h"

static const int	max_desired_items = rain_max_desired_items;
static const float	source_radius = rain_source_radius;
static const float	source_offset = rain_source_offset;
static const float	max_distance = source_offset * rain_max_distance_koef;
static const float	sink_offset = -(max_distance - source_offset);

const int	max_particles = rain_max_particles;
const int	particles_cache = rain_particles_cache;
const float particles_time = rain_particles_time;

//refs
class ENGINE_API IRender_DetailModel;

#include "../Include/xrRender/FactoryPtr.h"
#include "../Include/xrRender/RainRender.h"

class ENGINE_API CEffect_Rain
{
	friend class dxRainRender;
private:
	struct	Item
	{
		Fvector			P;
		Fvector			Phit;
		Fvector			D;
		float			fSpeed;
		u32				dwTime_Life;
		u32				dwTime_Hit;
		u32				uv_set;
		void			invalidate	()
		{
			dwTime_Life	= 0;
		}
	};
	struct	Particle
	{
		Particle		*next,*prev;
		Fmatrix			mXForm;
		Fsphere			bounds;
		float			time;
	};
	enum	States
	{
		stIdle		= 0,
		stWorking
	};
private:
	// Visualization	(rain) and (drops)
	FactoryPtr<IRainRender>	m_pRender;
	
	// Data and logic
	xr_vector<Item>					items;
	States							state;

	// Particles
	xr_vector<Particle>				particle_pool;
	Particle*						particle_active;
	Particle*						particle_idle;

	//Rain params
	float							drop_speed_min;
	float							drop_speed_max;
	float							drop_length;
	float							drop_width;
	float							drop_angle;
	float							drop_max_wind_vel;
	float							drop_max_angle;

	// Sounds
	ref_sound						snd_Ambient;
	ref_sound						snd_Wind;
	ref_sound						snd_RainOnMask;

	float							rain_hemi = 0.0f;
	
	bool							m_bWindWorking;
	float							m_fWindVolumeKoef;

	// Utilities
	void							p_create		();
	void							p_destroy		();

	void							p_remove		(Particle* P, Particle* &LST);
	void							p_insert		(Particle* P, Particle* &LST);
	int								p_size			(Particle* LST);
	Particle*						p_allocate		();
	void							p_free			(Particle* P);

	// Some methods
	void							Born			(Item& dest, const float radius, const float speed);
	void							Hit				(Fvector& pos);
	BOOL							RayPick			(const Fvector& s, const Fvector& d, float& range, collide::rq_target tgt);
	void							RenewItem		(Item& dest, float height, BOOL bHit);
	void							Prepare			(Fvector2& offset, Fvector3& axis, float Wind_Vel, float Wind_Dir);
public:
									CEffect_Rain	();
									~CEffect_Rain	();

	void							Render			();
	void							OnFrame			();

	float							GetRainHemi		() { return rain_hemi; }
};

#endif //RainH
