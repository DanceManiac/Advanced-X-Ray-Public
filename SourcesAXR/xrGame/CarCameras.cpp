#include "stdafx.h"
#pragma hdrstop
#ifdef DEBUG

#include "PHDebug.h"
#include "../xrphysics/iphworld.h"
#endif
#include "alife_space.h"
#include "hit.h"
#include "PHDestroyable.h"
#include "car.h"
#include "actor.h"
#include "cameralook.h"
#include "camerafirsteye.h"
#include "level.h"
#include "../xrEngine/cameramanager.h"
#include "../../Include/xrRender/Kinematics.h"

bool CCar::HUDView() const		
{
	return active_camera->tag==ectFirst;
}

void	CCar::cam_Update			(float dt, float fov)
{
	VERIFY(!physics_world()->Processing());
	Fvector							P,Da;
	Da.set							(0,0,0);
	//bool							owner = !!Owner();

	switch(active_camera->tag) {
	case ectFirst:
		XFORM().transform_tiny(P, m_camera_position_firsteye);

		// rotate head
		if(OwnerActor()) OwnerActor()->Orientation().yaw	= -active_camera->yaw;
		if(OwnerActor()) OwnerActor()->Orientation().pitch	= -active_camera->pitch;
		break;
	case ectChase:
		XFORM().transform_tiny(P, m_camera_position_lookat);

		break;
	case ectFree:
		XFORM().transform_tiny(P, m_camera_position_free);

		break;
	}
	active_camera->f_fov				= fov;
	active_camera->Update				(P,Da);
	Level().Cameras().UpdateFromCamera	(active_camera);
}

void	CCar::OnCameraChange		(int type)
{
	if(Owner())
	{
		if (type == ectFirst) //-> TODO: ��������� ������ � ��������
			Owner()->setVisible(TRUE);
		else
			Owner()->setVisible(FALSE);

		IKinematics* pKinematics = smart_cast<IKinematics*>(Owner()->Visual());
		u16 		head_bone = pKinematics->LL_BoneID("bip01_head");

		if (type == ectFirst)
			pKinematics->LL_SetBoneVisible(head_bone, false, false);
		else if (active_camera->tag == ectFirst)
			pKinematics->LL_SetBoneVisible(head_bone, true, true);
	}
	
	if (!active_camera||active_camera->tag!=type){
		active_camera	= camera[type];
		if (ectFree==type){
			Fvector xyz;
			XFORM().getXYZi(xyz);
			active_camera->yaw		= xyz.y;
		}
	}

}

