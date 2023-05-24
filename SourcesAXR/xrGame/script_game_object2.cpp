////////////////////////////////////////////////////////////////////////////
//	Module 		: script_game_object_script2.cpp
//	Created 	: 17.11.2004
//  Modified 	: 17.11.2004
//	Author		: Dmitriy Iassenev
//	Description : Script game object class script export
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "script_game_object.h"
#include "script_game_object_impl.h"
#include "ai_space.h"
#include "script_engine.h"
#include "explosive.h"
#include "script_zone.h"
#include "object_handler.h"
#include "script_hit.h"
#include "../Include/xrRender/Kinematics.h"
#include "pda.h"
#include "InfoPortion.h"
#include "memory_manager.h"
#include "ai_phrasedialogmanager.h"
#include "xrMessages.h"
#include "custommonster.h"
#include "memory_manager.h"
#include "visual_memory_manager.h"
#include "sound_memory_manager.h"
#include "hit_memory_manager.h"
#include "enemy_manager.h"
#include "item_manager.h"
#include "danger_manager.h"
#include "memory_space.h"
#include "actor.h"
#include "../Include/xrRender/Kinematics.h"
#include "../xrEngine/CameraBase.h"
#include "ai/stalker/ai_stalker.h"
#include "car.h"
#include "movement_manager.h"
#include "detail_path_manager.h"

void CScriptGameObject::explode	(u32 level_time)
{
	if (object().H_Parent())
	{
		Msg("! CExplosive: cannot explode object wiht parent");
		return;
	}
	
	MakeObj(CExplosive,explosive);
	Fvector normal;
	explosive->FindNormal(normal);
	explosive->SetInitiator(object().ID());
	explosive->GenExplodeEvent(object().Position(), normal);
}

bool CScriptGameObject::active_zone_contact		(u16 id)
{
	RMakeObj(CScriptZone,script_zone,false);
	return			(script_zone->active_contact(id));
}

CScriptGameObject *CScriptGameObject::best_weapon()
{
	CObjectHandler	*object_handler = smart_cast<CAI_Stalker*>(&object());
	if (!object_handler) {
		RCAST_ERR(CObjectHandler,object_handler,nullptr);
		return			(nullptr);
	}
	else {
		CGameObject		*game_object = object_handler->best_weapon() ? &object_handler->best_weapon()->object() : nullptr;
		return			(game_object ? game_object->lua_game_object() : nullptr);
	}
}

void CScriptGameObject::set_item		(MonsterSpace::EObjectAction object_action)
{
	CObjectHandler			*object_handler = smart_cast<CAI_Stalker*>(&object());
	if (!object_handler)
		CAST_ERR(CObjectHandler,object_handler);
	else
		object_handler->set_goal(object_action);
}

void CScriptGameObject::set_item		(MonsterSpace::EObjectAction object_action, CScriptGameObject *lua_game_object)
{
	CObjectHandler			*object_handler = smart_cast<CAI_Stalker*>(&object());
	if (!object_handler)
		CAST_ERR(CObjectHandler,object_handler);
	else
		object_handler->set_goal(object_action,lua_game_object ? &lua_game_object->object() : nullptr);
}

void CScriptGameObject::set_item(MonsterSpace::EObjectAction object_action, CScriptGameObject *lua_game_object, u32 queue_size)
{
	CObjectHandler			*object_handler = smart_cast<CAI_Stalker*>(&object());
	if (!object_handler)
		CAST_ERR(CObjectHandler,object_handler);
	else
		object_handler->set_goal(object_action,lua_game_object ? &lua_game_object->object() : nullptr, queue_size, queue_size);
}

void CScriptGameObject::set_item(MonsterSpace::EObjectAction object_action, CScriptGameObject *lua_game_object, u32 queue_size, u32 queue_interval)
{
	CObjectHandler			*object_handler = smart_cast<CAI_Stalker*>(&object());
	if (!object_handler)
		CAST_ERR(CObjectHandler,object_handler);
	else
		object_handler->set_goal(object_action,lua_game_object ? &lua_game_object->object() : nullptr, queue_size, queue_size, queue_interval, queue_interval);
}

void CScriptGameObject::play_cycle(LPCSTR anim, bool mix_in)
{
	MakeObj2(IKinematicsAnimated,sa,object().Visual());
	MotionID m	= sa->ID_Cycle(anim);
	if (m)
		sa->PlayCycle(m,(BOOL)mix_in);
	else 
		Msg("! CGameObject: has not cycle %s",anim);
}

void CScriptGameObject::play_cycle(LPCSTR anim)
{
	play_cycle	(anim,true);
}

void CScriptGameObject::Hit(CScriptHit *tpLuaHit)
{
	CScriptHit		&tLuaHit = *tpLuaHit;
	NET_Packet		P;
	SHit			HS;
	HS.GenHeader(GE_HIT,object().ID());										//	object().u_EventGen(P,GE_HIT,object().ID());
	THROW2			(tLuaHit.m_tpDraftsman,"Where is hit initiator??!");	//	THROW2			(tLuaHit.m_tpDraftsman,"Where is hit initiator??!");
	HS.whoID  = u16(tLuaHit.m_tpDraftsman->ID());							//	P.w_u16			(u16(tLuaHit.m_tpDraftsman->ID()));
	HS.weaponID = 0;														//	P.w_u16			(0);
	HS.dir = tLuaHit.m_tDirection;											//	P.w_dir			(tLuaHit.m_tDirection);
	HS.power = tLuaHit.m_fPower;											//	P.w_float		(tLuaHit.m_fPower);
	if (xr_strlen	(tLuaHit.m_caBoneName))									//	if (xr_strlen	(tLuaHit.m_caBoneName))
	{
		IKinematics *V = smart_cast<IKinematics*>(object().Visual());		//	IKinematics		*V = smart_cast<IKinematics*>(object().Visual());
		VERIFY(V);
		HS.boneID = 		(V->LL_BoneID(tLuaHit.m_caBoneName));			//		P.w_s16		(V->LL_BoneID(tLuaHit.m_caBoneName));
	}
	else																	//	else
		HS.boneID = 		(s16(0));										//		P.w_s16		(s16(0));
	HS.p_in_bone_space = Fvector().set(0,0,0);								//	P.w_vec3		(Fvector().set(0,0,0));
	HS.impulse = tLuaHit.m_fImpulse;										//	P.w_float		(tLuaHit.m_fImpulse);
	HS.hit_type = (ALife::EHitType)(tLuaHit.m_tHitType);					//	P.w_u16			(u16(tLuaHit.m_tHitType));
	HS.Write_Packet(P);						

	object().u_EventSend(P);
}


#pragma todo("Dima to Dima : find out why user defined conversion operators work incorrect")

CScriptGameObject::operator CObject*()
{
	return			(&object());
}

CScriptGameObject *CScriptGameObject::GetBestEnemy()
{
	RMakeObj(CCustomMonster,monster,nullptr);
	if (monster->memory().enemy().selected())
		return				(monster->memory().enemy().selected()->lua_game_object());
	return nullptr;
}

const CDangerObject *CScriptGameObject::GetBestDanger()
{
	RMakeObj(CCustomMonster,monster,nullptr);
	if (!monster->memory().danger().selected())
		return				nullptr;

	return					(monster->memory().danger().selected());
}

CScriptGameObject *CScriptGameObject::GetBestItem()
{
	RMakeObj(CCustomMonster,monster,nullptr);
	if (monster->memory().item().selected())
		return				(monster->memory().item().selected()->lua_game_object());
	return nullptr;
}

u32 CScriptGameObject::memory_time(const CScriptGameObject &lua_game_object)
{
	RMakeObj(CCustomMonster,monster,0);
	return				(monster->memory().memory_time(&lua_game_object.object()));
}

Fvector CScriptGameObject::memory_position(const CScriptGameObject &lua_game_object)
{
	RMakeObj(CCustomMonster,monster,Fvector().set(0.f,0.f,0.f));
	return				(monster->memory().memory_position(&lua_game_object.object()));
}

void CScriptGameObject::enable_memory_object	(CScriptGameObject *game_object, bool enable)
{
	MakeObj(CCustomMonster,monster);
	monster->memory().enable			(&game_object->object(),enable);
}

const xr_vector<CNotYetVisibleObject> &CScriptGameObject::not_yet_visible_objects() const
{
	CCustomMonster			*monster = smart_cast<CCustomMonster*>(&object());
	if (!monster) {
		CAST_ERR(CCustomMonster,monster);
		NODEFAULT;
	}
	return					(monster->memory().visual().not_yet_visible_objects());
}

float CScriptGameObject::visibility_threshold	() const
{
	CCustomMonster			*monster = smart_cast<CCustomMonster*>(&object());
	if (!monster) {
		CAST_ERR(CCustomMonster,monster);
		NODEFAULT;
	}
	return					(monster->memory().visual().visibility_threshold());
}

void CScriptGameObject::enable_vision			(bool value)
{
	MakeObj(CCustomMonster,monster);
	monster->memory().visual().enable		(value);
}

bool CScriptGameObject::vision_enabled			() const
{
	RMakeObj(CCustomMonster,monster,false);
	return									(monster->memory().visual().enabled());
}

void CScriptGameObject::set_sound_threshold		(float value)
{
	MakeObj(CCustomMonster,monster);
	monster->memory().sound().set_threshold		(value);
}

void CScriptGameObject::restore_sound_threshold	()
{
	MakeObj(CCustomMonster,monster);
	monster->memory().sound().restore_threshold	();
}

void CScriptGameObject::SetStartDialog(LPCSTR dialog_id)
{
	MakeObj(CAI_PhraseDialogManager,pDialogManager);
	pDialogManager->SetStartDialog(dialog_id);
}

void CScriptGameObject::GetStartDialog		()
{
	MakeObj(CAI_PhraseDialogManager,pDialogManager);
	pDialogManager->GetStartDialog();
}
void CScriptGameObject::RestoreDefaultStartDialog()
{
	MakeObj(CAI_PhraseDialogManager,pDialogManager);
	pDialogManager->RestoreDefaultStartDialog();
}

void CScriptGameObject::SetActorPosition			(Fvector pos)
{
	MakeObj(CActor,actor);
	Fmatrix F = actor->XFORM();
	F.c = pos;
	actor->ForceTransform(F);
//	actor->XFORM().c = pos;
}

void CScriptGameObject::SetNpcPosition			(Fvector pos)
{
	MakeObj(CCustomMonster,obj);
	Fmatrix F = obj->XFORM();
	F.c = pos;
	obj->movement().detail().make_inactual();
	if (obj->animation_movement_controlled())
		obj->destroy_anim_mov_ctrl();
	obj->ForceTransform(F);
	//		actor->XFORM().c = pos;

}

void CScriptGameObject::SetActorDirection		(float dir)
{
	MakeObj(CActor,actor);
	actor->cam_Active()->Set(dir,0,0);
//	actor->XFORM().setXYZ(0,dir,0);
}

void CScriptGameObject::DisableHitMarks			(bool disable)
{
	MakeObj(CActor,actor);
	actor->DisableHitMarks(disable);
}

bool CScriptGameObject::DisableHitMarks			()	const
{
	RMakeObj(CActor,actor,false);
	return actor->DisableHitMarks();
}

Fvector CScriptGameObject::GetMovementSpeed		()	const
{
	CActor* actor = smart_cast<CActor*>(&object());
	if(!actor)
	{
		CAST_ERR(CCustomMonster,monster);
		NODEFAULT;
	}
	return actor->GetMovementSpeed();
}

CHolderCustom* CScriptGameObject::get_current_holder()
{
	RMakeObj(CActor,actor,nullptr);
	return actor->Holder();
}

void CScriptGameObject::set_ignore_monster_threshold	(float ignore_monster_threshold)
{
	MakeObj(CAI_Stalker,stalker);
	clamp				(ignore_monster_threshold,0.f,1.f);
	stalker->memory().enemy().ignore_monster_threshold	(ignore_monster_threshold);
}

void CScriptGameObject::restore_ignore_monster_threshold	()
{
	MakeObj(CAI_Stalker,stalker);
	stalker->memory().enemy().restore_ignore_monster_threshold	();
}

float CScriptGameObject::ignore_monster_threshold		() const
{
	RMakeObj(CAI_Stalker,stalker,0.f);
	return				(stalker->memory().enemy().ignore_monster_threshold());
}

void CScriptGameObject::set_max_ignore_monster_distance	(const float &max_ignore_monster_distance)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->memory().enemy().max_ignore_monster_distance	(max_ignore_monster_distance);
}

void CScriptGameObject::restore_max_ignore_monster_distance	()
{
	MakeObj(CAI_Stalker,stalker);
	stalker->memory().enemy().restore_max_ignore_monster_distance	();
}

float CScriptGameObject::max_ignore_monster_distance	() const
{
	RMakeObj(CAI_Stalker,stalker,0.f);
	return				(stalker->memory().enemy().max_ignore_monster_distance());
}

CCar* CScriptGameObject::get_car	()
{
	CCar		*car = smart_cast<CCar*>(&object());
	if (!car) {
		CAST_ERR(CCar,car);
		NODEFAULT;
	}
	return car;
}

#ifdef DEBUG
void CScriptGameObject::debug_planner				(const script_planner *planner)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->debug_planner	(planner);
}
#endif

u32 CScriptGameObject::location_on_path				(float distance, Fvector *location)
{
	if (!location) {
		Msg("! CAI_Stalker: location_on_path -> specify destination location");
		return								(u32(-1));
	}

	RMakeObj(CCustomMonster,monster,u32(-1));
	VERIFY									(location);
	return									(monster->movement().detail().location_on_path(monster,distance,*location));
}

bool CScriptGameObject::is_there_items_to_pickup	() const
{
	RMakeObj(CAI_Stalker,stalker,false);
	return (!!stalker->memory().item().selected());
}

void CScriptGameObject::RemoveMemorySoundObject(const MemorySpace::CSoundObject& memory_object)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->memory().sound().remove(&memory_object);
}

void CScriptGameObject::RemoveMemoryHitObject(const MemorySpace::CHitObject& memory_object)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->memory().hit().remove(&memory_object);
}

void CScriptGameObject::RemoveMemoryVisibleObject(const MemorySpace::CVisibleObject& memory_object)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->memory().visual().remove(&memory_object);
}

void CScriptGameObject::ResetBoneProtections(pcstr imm_sect, pcstr bone_sect)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->ResetBoneProtections(imm_sect, bone_sect);
}