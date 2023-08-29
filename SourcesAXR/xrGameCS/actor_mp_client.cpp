#include "stdafx.h"
#include "actor_mp_client.h"
#include "actorcondition.h"
#include "../xrEngine/CameraBase.h"
#include "../xrEngine/CameraManager.h"

//if we are not current control entity we use this value
const float	CActorMP::cam_inert_value = 0.7f;

CActorMP::CActorMP			()
{
	//m_i_am_dead				= false;
}

void CActorMP::OnEvent		( NET_Packet &P, u16 type)
{
	inherited::OnEvent		(P,type);
#ifdef DEBUG
	if (type == GE_ACTOR_MAX_HEALTH)
	{
		Msg("--- CActorMP after GE_ACTOR_MAX_HEALTH health is: %2.04f", m_state_holder.state().health);
	}
#endif // #ifdef DEBUG
}

void CActorMP::Die			(CObject *killer)
{
	//m_i_am_dead				= true;
	//conditions().health()	= 0.f;
	conditions().SetHealth( 0.f );
	inherited::Die			(killer);
}

void CActorMP::cam_Set		(EActorCameras style)
{
#ifndef	DEBUG
	if (style != eacFirstEye)
		return;
#endif
	CCameraBase* old_cam = cam_Active();
	cam_active = style;
	old_cam->OnDeactivate();
	cam_Active()->OnActivate(old_cam);
}

void CActorMP::On_SetEntity()
{
	prev_cam_inert_value = psCamInert;
	if (this != Level().CurrentControlEntity())
	{
		psCamInert = cam_inert_value;
	}
	inherited::On_SetEntity();
}

void CActorMP::On_LostEntity()
{
	psCamInert = prev_cam_inert_value;
}
