////////////////////////////////////////////////////////////////////////////
// script_game_object_trader.сpp :	функции для торговли и торговцев
//////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "script_game_object.h"
#include "script_game_object_impl.h"

#include "script_zone.h"
#include "ai/trader/ai_trader.h"

#include "ai_space.h"
#include "alife_simulator.h"

#include "ai/stalker/ai_stalker.h"
#include "stalker_movement_manager_smart_cover.h"

#include "sight_manager_space.h"
#include "sight_control_action.h"
#include "sight_manager.h"
#include "inventoryBox.h"
#include "ZoneCampfire.h"
#include "physicobject.h"
#include "artefact.h"
class CWeapon;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool CScriptGameObject::is_body_turning		() const
{
	RMakeObj(CCustomMonster,monster,false);

	CAI_Stalker			*stalker = smart_cast<CAI_Stalker*>(monster);
	if (!stalker)
		return			(!fsimilar(monster->movement().body_orientation().target.yaw,monster->movement().body_orientation().current.yaw));
	return			(!fsimilar(stalker->movement().head_orientation().target.yaw,stalker->movement().head_orientation().current.yaw) || !fsimilar(monster->movement().body_orientation().target.yaw,monster->movement().body_orientation().current.yaw));
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

u32	CScriptGameObject::add_sound		(LPCSTR prefix, u32 max_count, ESoundTypes type, u32 priority, u32 mask, u32 internal_type, LPCSTR bone_name)
{
	RMakeObj(CCustomMonster,monster,0);
	return					(monster->sound().add(prefix,max_count,type,priority,mask,internal_type,bone_name));
}

u32	CScriptGameObject::add_sound		(LPCSTR prefix, u32 max_count, ESoundTypes type, u32 priority, u32 mask, u32 internal_type)
{
	return						(add_sound(prefix,max_count,type,priority,mask,internal_type,"bip01_head"));
}

void CScriptGameObject::remove_sound	(u32 internal_type)
{
	MakeObj(CCustomMonster,monster);
	monster->sound().remove	(internal_type);
}

void CScriptGameObject::set_sound_mask	(u32 sound_mask)
{
	MakeObj(CCustomMonster,monster);
	CEntityAlive			*entity_alive = smart_cast<CEntityAlive*>(monster);
	if (entity_alive)
		VERIFY2				(entity_alive->g_Alive(),"Stalkers talk after death??? Say why??");
	monster->sound().set_sound_mask(sound_mask);
}

void CScriptGameObject::play_sound		(u32 internal_type)
{
	MakeObj(CCustomMonster,monster);
	monster->sound().play		(internal_type);
}

void CScriptGameObject::play_sound		(u32 internal_type, u32 max_start_time)
{
	MakeObj(CCustomMonster,monster);
	monster->sound().play		(internal_type,max_start_time);
}

void CScriptGameObject::play_sound		(u32 internal_type, u32 max_start_time, u32 min_start_time)
{
	MakeObj(CCustomMonster,monster);
	monster->sound().play		(internal_type,max_start_time,min_start_time);
}

void CScriptGameObject::play_sound		(u32 internal_type, u32 max_start_time, u32 min_start_time, u32 max_stop_time)
{
	MakeObj(CCustomMonster,monster);
	monster->sound().play		(internal_type,max_start_time,min_start_time,max_stop_time);
}

void CScriptGameObject::play_sound		(u32 internal_type, u32 max_start_time, u32 min_start_time, u32 max_stop_time, u32 min_stop_time)
{
	MakeObj(CCustomMonster,monster);
	monster->sound().play		(internal_type,max_start_time,min_start_time,max_stop_time,min_stop_time);
}

void CScriptGameObject::play_sound		(u32 internal_type, u32 max_start_time, u32 min_start_time, u32 max_stop_time, u32 min_stop_time, u32 id)
{
	MakeObj(CCustomMonster,monster);
	monster->sound().play		(internal_type,max_start_time,min_start_time,max_stop_time,min_stop_time,id);
}

int  CScriptGameObject::active_sound_count		(bool only_playing)
{
	RMakeObj(CCustomMonster,monster,-1);
	return								(monster->sound().active_sound_count(only_playing));
}

int CScriptGameObject::active_sound_count		()
{
	return									(active_sound_count(false));
}

bool CScriptGameObject::wounded					() const
{
	RMakeObj(CAI_Stalker,stalker,false);
	return						(stalker->wounded());
}

void CScriptGameObject::wounded					(bool value)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->wounded			(value);
}

CSightParams CScriptGameObject::sight_params	()
{
	CAI_Stalker						*stalker = smart_cast<CAI_Stalker*>(&object());
	if (!stalker) {
		CAST_ERR(CAI_Stalker,stalker);
		CSightParams				result;
		result.m_object				= 0;
		result.m_vector				= Fvector().set(flt_max,flt_max,flt_max);
		result.m_sight_type			= SightManager::eSightTypeDummy;
		return						(result);
	}

	const CSightControlAction		&action = stalker->sight().current_action();
	CSightParams					result;
	result.m_sight_type				= action.sight_type();
	result.m_object					= action.object_to_look() ? action.object_to_look()->lua_game_object() : 0;
	result.m_vector					= action.vector3d();
	return							(result);
}

bool CScriptGameObject::critically_wounded		()
{
	RMakeObj(CCustomMonster,monster,false);
	return								(monster->critically_wounded());
}

bool CScriptGameObject::IsInvBoxEmpty()
{
	RMakeObj(CInventoryBox,ib,false);
	return			ib->IsEmpty		();
}

CZoneCampfire* CScriptGameObject::get_campfire()
{
	return smart_cast<CZoneCampfire*>(&object());
}

CArtefact* CScriptGameObject::get_artefact()
{
	return smart_cast<CArtefact*>(&object());
}

CPhysicObject* CScriptGameObject::get_physics_object()
{
	return smart_cast<CPhysicObject*>(&object());
}
#include "level_changer.h"
void CScriptGameObject::enable_level_changer(bool b)
{
	MakeObj(CLevelChanger,lch);
	lch->EnableLevelChanger(b);
}
bool CScriptGameObject::is_level_changer_enabled()
{
	RMakeObj(CLevelChanger,lch,false);
	return lch->IsLevelChangerEnabled();
}

void CScriptGameObject::set_level_changer_invitation(LPCSTR str)
{
	MakeObj(CLevelChanger,lch);
	lch->SetLEvelChangerInvitationStr(str);
}

void CScriptGameObject::start_particles(LPCSTR pname, LPCSTR bone)
{
	MakeObj(CParticlesPlayer,PP);
	IKinematics* K					= smart_cast<IKinematics*>(object().Visual());
	R_ASSERT						(K);

	u16 play_bone					= K->LL_BoneID(bone);
	R_ASSERT						(play_bone!=BI_NONE);
	if(K->LL_GetBoneVisible(play_bone))
		PP->StartParticles				(pname, play_bone, Fvector().set(0,1,0), 9999);
	else
		Msg("! Cant start particles, bone [%s] is not visible now", bone);
}

void CScriptGameObject::stop_particles(LPCSTR pname, LPCSTR bone)
{
	MakeObj(CParticlesPlayer,PP);
	IKinematics* K					= smart_cast<IKinematics*>(object().Visual());
	R_ASSERT						(K);

	u16 play_bone					= K->LL_BoneID(bone);
	R_ASSERT						(play_bone!=BI_NONE);

	if(K->LL_GetBoneVisible(play_bone))
		PP->StopParticles				(9999, play_bone, true);
	else
		Msg("! Cant stop particles, bone [%s] is not visible now", bone);
}

//AVO: directly set entity health instead of going through normal health property which operates on delta
void CScriptGameObject::SetHealthEx(float hp)
{
	MakeObj(CEntity,obj);
	clamp(hp, -0.01f, 1.0f);
	obj->SetfHealth(hp);
}
//-AVO