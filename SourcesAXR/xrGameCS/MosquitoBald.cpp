#include "stdafx.h"
#include "mosquitobald.h"
#include "hudmanager.h"
#include "ParticlesObject.h"
#include "level.h"
#include "physicsshellholder.h"
#include "..\XrEngine\xr_collide_form.h"
#include "../../xrCore/_detail_collision_point.h"

ENGINE_API extern xr_vector<DetailCollisionPoint> level_detailcoll_points;
ENGINE_API extern int ps_detail_enable_collision;
ENGINE_API extern Fvector actor_position;
ENGINE_API extern float ps_detail_collision_radius;

CMosquitoBald::CMosquitoBald(void) 
{
	m_fHitImpulseScale		= 1.f;
	m_bLastBlowoutUpdate	= false;
}

CMosquitoBald::~CMosquitoBald(void) 
{
}

void CMosquitoBald::Load(LPCSTR section) 
{
	inherited::Load(section);
}


bool CMosquitoBald::BlowoutState()
{
	bool result = inherited::BlowoutState();
	if(!result)
	{
		m_bLastBlowoutUpdate = false;
		UpdateBlowout();
	}
	else if(!m_bLastBlowoutUpdate)
	{
		m_bLastBlowoutUpdate = true;
		UpdateBlowout();
	}

	return result;
}

void CMosquitoBald::Affect(SZoneObjectInfo* O) 
{
	if (ps_detail_enable_collision)
	{
		if (actor_position.distance_to(Position()) <= ps_detail_collision_radius)
		{
			//-- это специфично для трамплина, т.к. он может бить очень часто и коллизия ломается
			EraseDetailCollPointIfExists(ID());
			level_detailcoll_points.push_back(DetailCollisionPoint(Position(), ID(), 2.5f, 0.3f, 1.f, true));
		}
	}

	CPhysicsShellHolder *pGameObject = smart_cast<CPhysicsShellHolder*>(O->object);
	if(!pGameObject) return;

	if(O->zone_ignore) return;

	Fvector P; 
	XFORM().transform_tiny(P,CFORM()->getSphere().P);

	Fvector hit_dir; 
	hit_dir.set(	::Random.randF(-.5f,.5f), 
					::Random.randF(.0f,1.f), 
					::Random.randF(-.5f,.5f)); 
	hit_dir.normalize();

	Fvector position_in_bone_space;

	VERIFY(!pGameObject->getDestroy());

	float dist = pGameObject->Position().distance_to(P) - pGameObject->Radius();
	float power = Power(dist>0.f?dist:0.f);
	float power_critical = 0.0f;
	float impulse = m_fHitImpulseScale*power*pGameObject->GetMass();

	if(power > 0.01f) 
	{
		position_in_bone_space.set(0.f,0.f,0.f);

		CreateHit(pGameObject->ID(),ID(),hit_dir,power,power_critical,0,position_in_bone_space,impulse,m_eHitTypeBlowout);

		PlayHitParticles(pGameObject);
	}
}
