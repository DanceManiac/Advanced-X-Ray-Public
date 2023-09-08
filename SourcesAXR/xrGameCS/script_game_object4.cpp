////////////////////////////////////////////////////////////////////////////
// script_game_object_trader.�pp :	������� ��� �������� � ���������
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

#include "Actor.h"
#include "Car.h"
#include "helicopter.h"
#include "InventoryOwner.h"
#include "inventoryBox.h"
#include "CustomZone.h"
#include "TorridZone.h"
#include "MosquitoBald.h"
#include "ZoneCampfire.h"
#include "CustomOutfit.h"
#include "Artefact.h"
#include "Weapon.h"
#include "WeaponAmmo.h"
#include "WeaponMagazined.h"
#include "WeaponMagazinedWGrenade.h"
#include "ai\monsters\basemonster\base_monster.h"
#include "scope.h"
#include "silencer.h"
#include "torch.h"
#include "GrenadeLauncher.h"
#include "searchlight.h"
#include "eatable_item.h"
#include "FoodItem.h"
#include "medkit.h"
#include "antirad.h"
#include "BottleItem.h"
#include "eatable_item.h"
#include "CustomDetector.h"
#include "AnomalyDetector.h"
#include "Torch.h"
#include "AntigasFilter.h"

class CWeapon;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool CScriptGameObject::is_body_turning		() const
{
	CCustomMonster		*monster = smart_cast<CCustomMonster*>(&object());
	if (!monster) {
		ai().script_engine().script_log		(ScriptStorage::eLuaMessageTypeError,"CGameObject : cannot access class member is_turning!");
		return			(false);
	}

	CAI_Stalker			*stalker = smart_cast<CAI_Stalker*>(monster);
	if (!stalker)
		return			(!fsimilar(monster->movement().body_orientation().target.yaw,monster->movement().body_orientation().current.yaw));
	else
		return			(!fsimilar(stalker->movement().head_orientation().target.yaw,stalker->movement().head_orientation().current.yaw) || !fsimilar(monster->movement().body_orientation().target.yaw,monster->movement().body_orientation().current.yaw));
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

u32	CScriptGameObject::add_sound		(LPCSTR prefix, u32 max_count, ESoundTypes type, u32 priority, u32 mask, u32 internal_type, LPCSTR bone_name)
{
	CCustomMonster				*monster = smart_cast<CCustomMonster*>(&object());
	if (!monster) {
		ai().script_engine().script_log					(ScriptStorage::eLuaMessageTypeError,"CSoundPlayer : cannot access class member add!");
		return					(0);
	}
	else
		return					(monster->sound().add(prefix,max_count,type,priority,mask,internal_type,bone_name));
}

u32	CScriptGameObject::add_sound		(LPCSTR prefix, u32 max_count, ESoundTypes type, u32 priority, u32 mask, u32 internal_type)
{
	return						(add_sound(prefix,max_count,type,priority,mask,internal_type,"bip01_head"));
}

void CScriptGameObject::remove_sound	(u32 internal_type)
{
	CCustomMonster				*monster = smart_cast<CCustomMonster*>(&object());
	if (!monster)
		ai().script_engine().script_log					(ScriptStorage::eLuaMessageTypeError,"CSoundPlayer : cannot access class member add!");
	else
		monster->sound().remove	(internal_type);
}

void CScriptGameObject::set_sound_mask	(u32 sound_mask)
{
	CCustomMonster				*monster = smart_cast<CCustomMonster*>(&object());
	if (!monster)
		ai().script_engine().script_log					(ScriptStorage::eLuaMessageTypeError,"CSoundPlayer : cannot access class member set_sound_mask!");
	else {
		CEntityAlive			*entity_alive = smart_cast<CEntityAlive*>(monster);
		if (entity_alive) {
			VERIFY2				(entity_alive->g_Alive(),"Stalkers talk after death??? Say why??");
		}
		monster->sound().set_sound_mask(sound_mask);
	}
}

void CScriptGameObject::play_sound		(u32 internal_type)
{
	CCustomMonster				*monster = smart_cast<CCustomMonster*>(&object());
	if (!monster)
		ai().script_engine().script_log					(ScriptStorage::eLuaMessageTypeError,"CSoundPlayer : cannot access class member play!");
	else
		monster->sound().play		(internal_type);
}

void CScriptGameObject::play_sound		(u32 internal_type, u32 max_start_time)
{
	CCustomMonster				*monster = smart_cast<CCustomMonster*>(&object());
	if (!monster)
		ai().script_engine().script_log					(ScriptStorage::eLuaMessageTypeError,"CSoundPlayer : cannot access class member play!");
	else
		monster->sound().play		(internal_type,max_start_time);
}

void CScriptGameObject::play_sound		(u32 internal_type, u32 max_start_time, u32 min_start_time)
{
	CCustomMonster				*monster = smart_cast<CCustomMonster*>(&object());
	if (!monster)
		ai().script_engine().script_log					(ScriptStorage::eLuaMessageTypeError,"CSoundPlayer : cannot access class member play!");
	else
		monster->sound().play		(internal_type,max_start_time,min_start_time);
}

void CScriptGameObject::play_sound		(u32 internal_type, u32 max_start_time, u32 min_start_time, u32 max_stop_time)
{
	CCustomMonster				*monster = smart_cast<CCustomMonster*>(&object());
	if (!monster)
		ai().script_engine().script_log					(ScriptStorage::eLuaMessageTypeError,"CSoundPlayer : cannot access class member play!");
	else
		monster->sound().play		(internal_type,max_start_time,min_start_time,max_stop_time);
}

void CScriptGameObject::play_sound		(u32 internal_type, u32 max_start_time, u32 min_start_time, u32 max_stop_time, u32 min_stop_time)
{
	CCustomMonster				*monster = smart_cast<CCustomMonster*>(&object());
	if (!monster)
		ai().script_engine().script_log					(ScriptStorage::eLuaMessageTypeError,"CSoundPlayer : cannot access class member play!");
	else
		monster->sound().play		(internal_type,max_start_time,min_start_time,max_stop_time,min_stop_time);
}

void CScriptGameObject::play_sound		(u32 internal_type, u32 max_start_time, u32 min_start_time, u32 max_stop_time, u32 min_stop_time, u32 id)
{
	CCustomMonster				*monster = smart_cast<CCustomMonster*>(&object());
	if (!monster)
		ai().script_engine().script_log					(ScriptStorage::eLuaMessageTypeError,"CSoundPlayer : cannot access class member play!");
	else
		monster->sound().play		(internal_type,max_start_time,min_start_time,max_stop_time,min_stop_time,id);
}

int  CScriptGameObject::active_sound_count		(bool only_playing)
{
	CCustomMonster				*monster = smart_cast<CCustomMonster*>(&object());
	if (!monster) {
		ai().script_engine().script_log		(ScriptStorage::eLuaMessageTypeError,"CGameObject : cannot access class member active_sound_count!");
		return								(-1);
	}
	else
		return								(monster->sound().active_sound_count(only_playing));
}

int CScriptGameObject::active_sound_count		()
{
	return									(active_sound_count(false));
}

bool CScriptGameObject::wounded					() const
{
	const CAI_Stalker			*stalker = smart_cast<const CAI_Stalker *>(&object());
	if (!stalker) {
		ai().script_engine().script_log		(ScriptStorage::eLuaMessageTypeError,"CAI_Stalker : cannot access class member wounded!");
		return					(false);
	}

	return						(stalker->wounded());
}

void CScriptGameObject::wounded					(bool value)
{
	CAI_Stalker					*stalker = smart_cast<CAI_Stalker *>(&object());
	if (!stalker) {
		ai().script_engine().script_log		(ScriptStorage::eLuaMessageTypeError,"CAI_Stalker : cannot access class member wounded!");
		return;
	}

	stalker->wounded			(value);
}

CSightParams CScriptGameObject::sight_params	()
{
	CAI_Stalker						*stalker = smart_cast<CAI_Stalker*>(&object());
	if (!stalker) {
		ai().script_engine().script_log			(ScriptStorage::eLuaMessageTypeError,"CAI_Stalker : cannot access class member sight_params!");

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
	CCustomMonster						*custom_monster = smart_cast<CCustomMonster*>(&object());
	if (!custom_monster) {
		ai().script_engine().script_log	(ScriptStorage::eLuaMessageTypeError,"CCustomMonster : cannot access class member critically_wounded!");
		return							(false);
	}

	return								(custom_monster->critically_wounded());
}

bool CScriptGameObject::IsInvBoxEmpty()
{
	CInventoryBox* ib = smart_cast<CInventoryBox*>(&object());
	if(!ib) 
		return			(false);
	else
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
	CLevelChanger* lch = smart_cast<CLevelChanger*>(&object());
	if(lch)
		lch->EnableLevelChanger(b);
}
bool CScriptGameObject::is_level_changer_enabled()
{
	CLevelChanger* lch = smart_cast<CLevelChanger*>(&object());
	if(lch)
		return lch->IsLevelChangerEnabled();
	return false;
}

void CScriptGameObject::set_level_changer_invitation(LPCSTR str)
{
	CLevelChanger* lch = smart_cast<CLevelChanger*>(&object());
	if(lch)
		lch->SetLEvelChangerInvitationStr(str);
}

void CScriptGameObject::start_particles(LPCSTR pname, LPCSTR bone)
{
	CParticlesPlayer* PP			= smart_cast<CParticlesPlayer*>(&object());
	if(!PP)	return;

	IKinematics* K					= smart_cast<IKinematics*>(object().Visual());
	R_ASSERT						(K);

	u16 play_bone					= K->LL_BoneID(bone);
	R_ASSERT						(play_bone!=BI_NONE);
	if(K->LL_GetBoneVisible(play_bone))
		PP->StartParticles				(pname, play_bone, Fvector().set(0,1,0), 9999);
	else
		ai().script_engine().script_log	(ScriptStorage::eLuaMessageTypeError,"Cant start particles, bone [%s] is not visible now", bone);
}

void CScriptGameObject::stop_particles(LPCSTR pname, LPCSTR bone)
{
	CParticlesPlayer* PP			= smart_cast<CParticlesPlayer*>(&object());
	if(!PP)	return;

	IKinematics* K					= smart_cast<IKinematics*>(object().Visual());
	R_ASSERT						(K);

	u16 play_bone					= K->LL_BoneID(bone);
	R_ASSERT						(play_bone!=BI_NONE);

	if(K->LL_GetBoneVisible(play_bone))
		PP->StopParticles				(9999, play_bone, true);
	else
		ai().script_engine().script_log	(ScriptStorage::eLuaMessageTypeError,"Cant stop particles, bone [%s] is not visible now", bone);
}

//AVO: directly set entity health instead of going through normal health property which operates on delta
void CScriptGameObject::SetHealthEx(float hp)
{
	CEntity* obj = smart_cast<CEntity*>(&object());
	if (!obj) return;
	clamp(hp, -0.01f, 1.0f);
	obj->SetfHealth(hp);
}
//-AVO

// AVO: Credits: KD
// functions for testing object class 
#define TEST_OBJECT_CLASS(A,B)\
bool A () const\
{\
    B				*l_tpEntity = smart_cast<B*>(&object());\
    if (!l_tpEntity)\
        return false;\
                else\
        return true;\
};

TEST_OBJECT_CLASS(CScriptGameObject::IsEntityAlive,		CEntityAlive)
TEST_OBJECT_CLASS(CScriptGameObject::IsInventoryItem,	CInventoryItem)
TEST_OBJECT_CLASS(CScriptGameObject::IsInventoryOwner,	CInventoryOwner)
TEST_OBJECT_CLASS(CScriptGameObject::IsActor,			CActor)
TEST_OBJECT_CLASS(CScriptGameObject::IsCustomMonster,	CCustomMonster)
TEST_OBJECT_CLASS(CScriptGameObject::IsWeapon,			CWeapon)
TEST_OBJECT_CLASS(CScriptGameObject::IsCustomOutfit,	CCustomOutfit)
TEST_OBJECT_CLASS(CScriptGameObject::IsScope,			CScope)
TEST_OBJECT_CLASS(CScriptGameObject::IsSilencer,		CSilencer)
TEST_OBJECT_CLASS(CScriptGameObject::IsGrenadeLauncher, CGrenadeLauncher)
TEST_OBJECT_CLASS(CScriptGameObject::IsWeaponMagazined, CWeaponMagazined)
TEST_OBJECT_CLASS(CScriptGameObject::IsSpaceRestrictor, CSpaceRestrictor)
TEST_OBJECT_CLASS(CScriptGameObject::IsStalker,			CAI_Stalker)
TEST_OBJECT_CLASS(CScriptGameObject::IsAnomaly,			CCustomZone)
TEST_OBJECT_CLASS(CScriptGameObject::IsMonster,			CBaseMonster)
TEST_OBJECT_CLASS(CScriptGameObject::IsTrader,			CAI_Trader)
TEST_OBJECT_CLASS(CScriptGameObject::IsHudItem,			CHudItem)
TEST_OBJECT_CLASS(CScriptGameObject::IsArtefact,		CArtefact)
TEST_OBJECT_CLASS(CScriptGameObject::IsAmmo,			CWeaponAmmo)
TEST_OBJECT_CLASS(CScriptGameObject::IsWeaponGL,		CWeaponMagazinedWGrenade)
TEST_OBJECT_CLASS(CScriptGameObject::IsInventoryBox,	CInventoryBox)
TEST_OBJECT_CLASS(CScriptGameObject::IsEatableItem,		CEatableItem)
TEST_OBJECT_CLASS(CScriptGameObject::IsDetector,		CCustomDetector)
TEST_OBJECT_CLASS(CScriptGameObject::IsDetectorAnomaly, CDetectorAnomaly)
TEST_OBJECT_CLASS(CScriptGameObject::IsTorch,			CTorch)
TEST_OBJECT_CLASS(CScriptGameObject::IsAntigasFilter,	CAntigasFilter)