#include "pch_script.h"
#include "script_game_object.h"
#include "script_game_object_impl.h"
#include "UsableScriptObject.h"
#include "GameObject.h"
#include "script_storage_space.h"
#include "script_engine.h"
#include "stalker_planner.h"
#include "ai/stalker/ai_stalker.h"
#include "searchlight.h"
#include "script_callback_ex.h"
#include "game_object_space.h"
#include "memory_manager.h"
#include "enemy_manager.h"
#include "movement_manager.h"
#include "patrol_path_manager.h"
#include "PHCommander.h"
#include "PHScriptCall.h"
#include "PHSimpleCalls.h"
#include "phworld.h"
void CScriptGameObject::SetTipText (LPCSTR tip_text)
{
	MakeObj(CUsableScriptObject,usable_obj);
	usable_obj->set_tip_text(tip_text);
}

void CScriptGameObject::SetTipTextDefault ()
{
	MakeObj(CUsableScriptObject,usable_obj);
	usable_obj->set_tip_text_default();
}

void CScriptGameObject::SetNonscriptUsable(bool nonscript_usable)
{
	MakeObj(CUsableScriptObject,usable_obj);
	usable_obj->set_nonscript_usable(nonscript_usable);
}

Fvector CScriptGameObject::GetCurrentDirection()
{
	RMakeObj(CProjector,obj,Fvector().set(0.f,0.f,0.f));
	return obj->GetCurrentDirection();
}

CScriptGameObject::CScriptGameObject		(CGameObject *game_object)
{
	m_game_object	= game_object;
	R_ASSERT2		(m_game_object,"Null actual object passed!");
}

CScriptGameObject::~CScriptGameObject		()
{
}

CScriptGameObject *CScriptGameObject::Parent				() const
{
	RMakeObj2(CGameObject,GO,nullptr,object().H_Parent());
	return		(GO->lua_game_object());
}

int	CScriptGameObject::clsid				() const
{
	return			(object().clsid());
}

LPCSTR CScriptGameObject::Name				() const
{
	return			(*object().cName());
}

shared_str CScriptGameObject::cName				() const
{
	return			(object().cName());
}

LPCSTR CScriptGameObject::Section				() const
{
	return			(*object().cNameSect());
}

void CScriptGameObject::Kill					(CScriptGameObject* who)
{
	MakeObj(CEntity,entity);
	if (!entity->AlreadyDie())
		entity->KillEntity					(who ? who->object().ID() : object().ID());
	else
		Msg("~ attempt to kill dead object %s",*object().cName());
}

bool CScriptGameObject::Alive					() const
{
	RMakeObj(CEntity,entity,false);
	return				(!!entity->g_Alive());
}

ALife::ERelationType CScriptGameObject::GetRelationType	(CScriptGameObject* who)
{
	RMakeObj(CEntityAlive,entity1,ALife::eRelationTypeDummy);
	RMakeObj2(CEntityAlive,entity2,ALife::eRelationTypeDummy,&who->object());
	return entity1->tfGetRelationType(entity2);
}

template <typename T>
IC	T	*CScriptGameObject::action_planner()
{
	RMakeObj(CAI_Stalker,manager,0);
	return								(&manager->brain());
}

CScriptActionPlanner		*script_action_planner(CScriptGameObject *obj)
{
	return					(obj->action_planner<CScriptActionPlanner>());
}

void CScriptGameObject::set_enemy_callback	(const luabind::functor<bool> &functor)
{
	MakeObj(CCustomMonster,monster);
	monster->memory().enemy().useful_callback().set(functor);
}

void CScriptGameObject::set_enemy_callback	(const luabind::functor<bool> &functor, const luabind::object &object)
{
	MakeObj2(CCustomMonster,monster,&this->object());
	monster->memory().enemy().useful_callback().set(functor,object);
}

void CScriptGameObject::set_enemy_callback	()
{
	MakeObj(CCustomMonster,monster);
	monster->memory().enemy().useful_callback().clear();
}

void CScriptGameObject::SetCallback(GameObject::ECallbackType type, const luabind::functor<void> &functor)
{
	object().callback(type).set(functor);
}

void CScriptGameObject::SetCallback(GameObject::ECallbackType type, const luabind::functor<void> &functor, const luabind::object &object)
{
	this->object().callback(type).set(functor, object);
}

void CScriptGameObject::SetCallback(GameObject::ECallbackType type)
{
	object().callback(type).clear();
}

void CScriptGameObject::set_fastcall(const luabind::functor<bool> &functor, const luabind::object &object)
{
	CPHScriptGameObjectCondition* c=xr_new<CPHScriptGameObjectCondition>(object,functor,m_game_object);
	CPHDummiAction*				  a=xr_new<CPHDummiAction>();
	CPHSriptReqGObjComparer cmpr(m_game_object);
	Level().ph_commander_scripts().remove_calls(&cmpr);
	Level().ph_commander_scripts().add_call(c,a);
}
void CScriptGameObject::set_const_force(const Fvector &dir,float value,u32 time_interval)
{
	if(!ph_world)	{
		Msg("! set_const_force: ph_world do not exist");
		return;
	}
	CPhysicsShell	*shell=object().cast_physics_shell_holder()->PPhysicsShell();
	if(!shell){
		Msg("! set_const_force: object %s has no physics shell",*object().cName());
		return;
	}

	Fvector force;force.set(dir);force.mul(value);
	CPHConstForceAction *a=	xr_new<CPHConstForceAction>(shell,force);
	CPHExpireOnStepCondition *cn=xr_new<CPHExpireOnStepCondition>();
	cn->set_time_interval(time_interval);
	ph_world->AddCall(cn,a);
}