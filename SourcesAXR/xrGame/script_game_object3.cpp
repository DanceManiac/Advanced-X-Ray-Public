////////////////////////////////////////////////////////////////////////////
//	Module 		: script_game_object_script3.cpp
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
#include "cover_evaluators.h"
#include "cover_point.h"
#include "cover_manager.h"
#include "ai/stalker/ai_stalker.h"
#include "stalker_animation_manager.h"
#include "stalker_planner.h"
#include "weapon.h"
#include "inventory.h"
#include "customzone.h"
#include "patrol_path_manager.h"
#include "object_handler_planner.h"
#include "object_handler_space.h"
#include "memory_manager.h"
#include "visual_memory_manager.h"
#include "sound_memory_manager.h"
#include "hit_memory_manager.h"
#include "sight_manager.h"
#include "stalker_movement_manager_smart_cover.h"
#include "movement_manager_space.h"
#include "detail_path_manager_space.h"
#include "level_debug.h"
#include "ai/monsters/BaseMonster/base_monster.h"
#include "trade_parameters.h"
#include "script_ini_file.h"
#include "sound_player.h"
#include "stalker_decision_space.h"
#include "space_restriction_manager.h"
#include "eatable_item.h"
//Alundaio
#include "level_path_manager.h"
#include "game_path_manager.h"
#include "danger_manager.h"
#include "danger_object.h"

#include "Artefact.h"
#include "holder_custom.h"
#include "Actor.h"
#include "WeaponAmmo.h"
#include "WeaponMagazinedWGrenade.h"

namespace MemorySpace {
	struct CVisibleObject;
	struct CSoundObject;
	struct CHitObject;
};

const CCoverPoint *CScriptGameObject::best_cover	(const Fvector &position, const Fvector &enemy_position, float radius, float min_enemy_distance, float max_enemy_distance)
{
	RMakeObj(CAI_Stalker,stalker,NULL);
	stalker->m_ce_best->setup(enemy_position,min_enemy_distance,max_enemy_distance,0.f);
	const CCoverPoint	*point = ai().cover_manager().best_cover(position,radius,*stalker->m_ce_best);
	return			(point);
}

const CCoverPoint *CScriptGameObject::safe_cover	(const Fvector &position, float radius, float min_distance)
{
	RMakeObj(CAI_Stalker,stalker,NULL);
	stalker->m_ce_safe->setup(min_distance);
	const CCoverPoint	*point = ai().cover_manager().best_cover(position,radius,*stalker->m_ce_safe);
	return			(point);
}

const xr_vector<MemorySpace::CVisibleObject>	&CScriptGameObject::memory_visible_objects	() const
{
	CCustomMonster	*monster = smart_cast<CCustomMonster*>(&object());
	if (!monster) {
		CAST_ERR(CCustomMonster,monster);
		NODEFAULT;
	}
	return (monster->memory().visual().objects());
}

const xr_vector<MemorySpace::CSoundObject>	&CScriptGameObject::memory_sound_objects	() const
{
	CCustomMonster	*monster = smart_cast<CCustomMonster*>(&object());
	if (!monster) {
		CAST_ERR(CCustomMonster,monster);
		NODEFAULT;
	}
	return			(monster->memory().sound().objects());
}

const xr_vector<MemorySpace::CHitObject>		&CScriptGameObject::memory_hit_objects		() const
{
	CCustomMonster	*monster = smart_cast<CCustomMonster*>(&object());
	if (!monster) {
		CAST_ERR(CCustomMonster,monster);
		NODEFAULT;
	}
	return			(monster->memory().hit().objects());
}

void CScriptGameObject::ChangeTeam(u8 team, u8 squad, u8 group)
{
	MakeObj(CCustomMonster,monster);
	monster->ChangeTeam(team,squad,group);
}

void CScriptGameObject::SetVisualMemoryEnabled	(bool enabled)
{
	MakeObj(CCustomMonster,monster);
	monster->memory().visual().enable(enabled);
}

CScriptGameObject *CScriptGameObject::GetEnemy() const
{
	RMakeObj(CCustomMonster,monster,nullptr);
	if (!monster->g_Alive() )
		return nullptr;
	if (monster->GetCurrentEnemy() && !monster->GetCurrentEnemy()->getDestroy())
		return (monster->GetCurrentEnemy()->lua_game_object());
	return nullptr;
}

CScriptGameObject *CScriptGameObject::GetCorpse() const
{
	RMakeObj(CCustomMonster,monster,nullptr);
	if (monster->GetCurrentCorpse() && !monster->GetCurrentCorpse()->getDestroy())
		return (monster->GetCurrentCorpse()->lua_game_object());
	return nullptr;
}

bool CScriptGameObject::CheckTypeVisibility(const char *section_name)
{
	RMakeObj(CCustomMonster,monster,false);
	return			(monster->CheckTypeVisibility(section_name));
}

CScriptGameObject *CScriptGameObject::GetCurrentWeapon() const
{
	RMakeObj(CAI_Stalker,stalker,nullptr);
	CGameObject		*current_weapon = stalker->GetCurrentWeapon();
	return			(current_weapon ? current_weapon->lua_game_object() : nullptr);
}

void CScriptGameObject::deadbody_closed(bool status)
{
	MakeObj(CInventoryOwner,inventoryOwner);
	inventoryOwner->deadbody_closed(status);
}

bool CScriptGameObject::deadbody_closed_status()
{
	RMakeObj(CInventoryOwner,inventoryOwner,false);
	return  inventoryOwner->deadbody_closed_status();
}

void CScriptGameObject::can_select_weapon(bool status)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->can_select_weapon(status);
}

bool CScriptGameObject::can_select_weapon() const
{
	RMakeObj(CAI_Stalker,stalker,false);
	return  stalker->can_select_weapon();
}

void CScriptGameObject::deadbody_can_take(bool status)
{
	MakeObj(CInventoryOwner,inventoryOwner);
	inventoryOwner->deadbody_can_take(status);
}

bool CScriptGameObject::deadbody_can_take_status()
{
	RMakeObj(CInventoryOwner,inventoryOwner,false);
	return  inventoryOwner->deadbody_can_take_status();
}
#include "CustomOutfit.h"

CScriptGameObject *CScriptGameObject::GetCurrentOutfit() const
{
	RMakeObj(CInventoryOwner,inventoryOwner,nullptr);
	CGameObject		*current_equipment = inventoryOwner->GetOutfit();
	return			(current_equipment ? current_equipment->lua_game_object() : nullptr);
}


float CScriptGameObject::GetCurrentOutfitProtection(int hit_type)
{
	RMakeObj(CInventoryOwner,inventoryOwner,0.0f);
	CGameObject		*current_equipment = inventoryOwner->GetOutfit();
	CCustomOutfit* o = smart_cast<CCustomOutfit*>(current_equipment);
	if(!o)				return 0.0f;

	return		o->GetDefHitTypeProtection(ALife::EHitType(hit_type));
}

CScriptGameObject *CScriptGameObject::GetFood() const
{
	RMakeObj(CAI_Stalker,stalker,nullptr);
	CGameObject		*food = stalker->GetFood() ? &stalker->GetFood()->object() : nullptr;
	return			(food ? food->lua_game_object() : nullptr);
}

CScriptGameObject *CScriptGameObject::GetMedikit() const
{
	RMakeObj(CAI_Stalker,stalker,nullptr);
	CGameObject		*medikit = stalker->GetMedikit() ? &stalker->GetMedikit()->object() : nullptr;
	return			(medikit ? medikit->lua_game_object() : nullptr);
}

LPCSTR CScriptGameObject::GetPatrolPathName()
{
	CAI_Stalker			*stalker = smart_cast<CAI_Stalker*>(&object());
	if (!stalker) {
		RMakeObj(CScriptEntity,script_monster,"");
		return (script_monster->GetPatrolPathName());
	}

	return			(*stalker->movement().patrol().path_name());
}

void CScriptGameObject::add_animation			(LPCSTR animation, bool hand_usage, bool use_movement_controller)
{
	MakeObj(CAI_Stalker,stalker);
	if (stalker->movement().current_params().cover()) {
		Msg("! Cannot add animation [%s]: object [%s] is in smart_cover", animation, stalker->cName().c_str());
	}

	if (stalker->animation().global_selector()) {
		Msg(
			"! Cannot add animation [%s]: global selector is set for object [%s], in_smart_cover returned [%s]",
			animation,
			stalker->cName().c_str(),
			in_smart_cover() ? "true" : "false"
		);
		return;
	}
	
	stalker->animation().add_script_animation(animation,hand_usage,use_movement_controller);
}

void CScriptGameObject::add_animation			(LPCSTR animation, bool hand_usage, Fvector position, Fvector rotation, bool local_animation)
{
	MakeObj(CAI_Stalker,stalker);
	
	if (stalker->movement().current_params().cover()) {
		Msg("! Cannot add animation [%s]: object [%s] is in smart_cover", animation, stalker->cName().c_str());
	}

	if (stalker->animation().global_selector()) {
		Msg(
			"! Cannot add animation [%s]: global selector is set for object [%s], in_smart_cover returned [%s]",
			animation,
			stalker->cName().c_str(),
			in_smart_cover() ? "true" : "false"
		);
		return;
	}
	
	stalker->animation().add_script_animation( animation, hand_usage, position, rotation, local_animation);
}

void CScriptGameObject::clear_animations		()
{
	MakeObj(CAI_Stalker,stalker);
	stalker->animation().clear_script_animations();
}

int	CScriptGameObject::animation_count		() const
{
	RMakeObj(CAI_Stalker,stalker,-1);
	return				((int)stalker->animation().script_animations().size());
}

Flags32 CScriptGameObject::get_actor_relation_flags () const
{
	CAI_Stalker* stalker = smart_cast<CAI_Stalker*>(&object());
	THROW(stalker);
	return stalker->m_actor_relation_flags;
}

void CScriptGameObject::set_actor_relation_flags (Flags32 flags)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->m_actor_relation_flags = flags;
}

void CScriptGameObject::set_patrol_path		(LPCSTR path_name, const PatrolPathManager::EPatrolStartType patrol_start_type, const PatrolPathManager::EPatrolRouteType patrol_route_type, bool random)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->movement().patrol().set_path		(path_name,patrol_start_type,patrol_route_type,random);
}

void CScriptGameObject::inactualize_patrol_path		()
{
	MakeObj(CAI_Stalker,stalker);
	stalker->movement().patrol().make_inactual();
}

void CScriptGameObject::inactualize_level_path()
{
	MakeObj(CAI_Stalker,stalker);
	stalker->movement().level_path().make_inactual();
}

void CScriptGameObject::inactualize_game_path()
{
	MakeObj(CAI_Stalker,stalker);
	stalker->movement().game_path().make_inactual();
}

u32 CScriptGameObject::get_dest_game_vertex_id()
{
	RMakeObj(CAI_Stalker,stalker,u32(-1));
	return (stalker->movement().game_dest_vertex_id());
}

u32 CScriptGameObject::get_dest_level_vertex_id()
{
	RMakeObj(CAI_Stalker,stalker,u32(-1));
	return stalker->movement().level_dest_vertex_id();
}

void CScriptGameObject::set_dest_level_vertex_id(u32 level_vertex_id)
{
	MakeObj(CAI_Stalker,stalker);
	if (!ai().level_graph().valid_vertex_id(level_vertex_id)) {
#ifdef DEBUG
		ai().script_engine().script_log				(ScriptStorage::eLuaMessageTypeError,"CAI_Stalker: invalid vertex id being setup by action %s",stalker->brain().CStalkerPlanner::current_action().m_action_name);
#endif
		return;
	}
	if (!stalker->movement().restrictions().accessible(level_vertex_id)) {
		Msg(
			"! you are trying to setup destination for the stalker %s, which is not accessible by its restrictors in[%s] out[%s]",
			stalker->cName().c_str(),
			Level().space_restriction_manager().in_restrictions (stalker->ID()).c_str(),
			Level().space_restriction_manager().out_restrictions(stalker->ID()).c_str()
		);
		return;
	}
	stalker->movement().set_level_dest_vertex	(level_vertex_id);
}

void CScriptGameObject::set_dest_game_vertex_id( GameGraph::_GRAPH_ID game_vertex_id)
{
	MakeObj(CAI_Stalker,stalker);
	if (!ai().game_graph().valid_vertex_id(game_vertex_id)) {
#ifdef DEBUG
		ai().script_engine().script_log				(ScriptStorage::eLuaMessageTypeError,"CAI_Stalker: invalid vertex id being setup by action %s",stalker->brain().CStalkerPlanner::current_action().m_action_name);
#endif
		return;
	}
	stalker->movement().set_game_dest_vertex(game_vertex_id);
}

void CScriptGameObject::set_movement_selection_type(ESelectionType selection_type){
	MakeObj(CAI_Stalker,stalker);
	stalker->movement().game_selector().set_selection_type		(selection_type);
}

CHARACTER_RANK_VALUE CScriptGameObject::GetRank		()
{
	RMakeObj(CAI_Stalker,stalker,CHARACTER_RANK_VALUE(0));
	return					(stalker->Rank());
}

void CScriptGameObject::set_desired_position	()
{
	MakeObj(CAI_Stalker,stalker);
	stalker->movement().set_desired_position	(0);
}

void CScriptGameObject::set_desired_position	(const Fvector *desired_position)
{
	MakeObj(CAI_Stalker,stalker);
	THROW2										(desired_position || stalker->movement().restrictions().accessible(*desired_position),*stalker->cName());
	stalker->movement().set_desired_position	(desired_position);
}

void  CScriptGameObject::set_desired_direction	()
{
	MakeObj(CAI_Stalker,stalker);
	stalker->movement().set_desired_direction	(0);
}

void  CScriptGameObject::set_desired_direction	(const Fvector *desired_direction)
{
	MakeObj(CAI_Stalker,stalker);
	if (fsimilar(desired_direction->magnitude(), 0.f))
		Msg("! CAI_Stalker: [%s] set_desired_direction - you passed zero direction", stalker->cName().c_str());
	else if (!fsimilar(desired_direction->magnitude(), 1.f))
		Msg("! CAI_Stalker: [%s] set_desired_direction - you passed non-normalized direction", stalker->cName().c_str());

	Fvector											direction = *desired_direction;
	direction.normalize_safe						();
	stalker->movement().set_desired_direction		(&direction);
}

void  CScriptGameObject::set_body_state			(EBodyState body_state)
{
	THROW						((body_state == eBodyStateStand) || (body_state == eBodyStateCrouch));
	MakeObj(CAI_Stalker,stalker);
	stalker->movement().set_body_state	(body_state);
}

void  CScriptGameObject::set_movement_type		(EMovementType movement_type)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->movement().set_movement_type	(movement_type);
}

void  CScriptGameObject::set_mental_state		(EMentalState mental_state)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->movement().set_mental_state	(mental_state);
}

void  CScriptGameObject::set_path_type			(MovementManager::EPathType path_type)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->movement().set_path_type	(path_type);
}

void  CScriptGameObject::set_detail_path_type	(DetailPathManager::EDetailPathType detail_path_type)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->movement().set_detail_path_type	(detail_path_type);
}

MonsterSpace::EBodyState CScriptGameObject::body_state					() const
{
	RMakeObj(CAI_Stalker,stalker,MonsterSpace::eBodyStateStand);
	return			(stalker->movement().body_state());
}

MonsterSpace::EBodyState CScriptGameObject::target_body_state			() const
{
	RMakeObj(CAI_Stalker,stalker,MonsterSpace::eBodyStateStand);
	return			(stalker->movement().target_body_state());
}

MonsterSpace::EMovementType CScriptGameObject::movement_type			() const
{
	RMakeObj(CAI_Stalker,stalker,MonsterSpace::eMovementTypeStand);
	return			(stalker->movement().movement_type());
}

MonsterSpace::EMovementType CScriptGameObject::target_movement_type		() const
{
	RMakeObj(CAI_Stalker,stalker,MonsterSpace::eMovementTypeStand);
	return			(stalker->movement().target_movement_type());
}

MonsterSpace::EMentalState CScriptGameObject::mental_state				() const
{
	RMakeObj(CAI_Stalker,stalker,MonsterSpace::eMentalStateDanger);
	return			(stalker->movement().mental_state());
}

MonsterSpace::EMentalState CScriptGameObject::target_mental_state		() const
{
	RMakeObj(CAI_Stalker,stalker,MonsterSpace::eMentalStateDanger);
	return			(stalker->movement().target_mental_state());
}

MovementManager::EPathType CScriptGameObject::path_type					() const
{
	RMakeObj(CAI_Stalker,stalker,MovementManager::ePathTypeNoPath);
	return			(stalker->movement().path_type());
}

DetailPathManager::EDetailPathType CScriptGameObject::detail_path_type	() const
{
	return			(DetailPathManager::eDetailPathTypeSmooth);
}

void CScriptGameObject::set_sight		(SightManager::ESightType sight_type, Fvector *vector3d, u32 dwLookOverDelay)
{
	MakeObj(CAI_Stalker,stalker);
	if ( (sight_type == SightManager::eSightTypeDirection) && vector3d && (_abs(vector3d->magnitude() - 1.f) > .01f) ) {
		VERIFY2				( false, make_string("non-normalized direction passed [%f][%f][%f]", VPUSH(*vector3d)) );
		vector3d->normalize	( );
	}
	stalker->sight().setup	(sight_type,vector3d);
}

void CScriptGameObject::set_sight		(SightManager::ESightType sight_type, bool torso_look, bool path)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->sight().setup	(sight_type,torso_look,path);
}

void CScriptGameObject::set_sight		(SightManager::ESightType sight_type, Fvector &vector3d, bool torso_look = false)
{
	MakeObj(CAI_Stalker,stalker);
	if ( (sight_type == SightManager::eSightTypeDirection) && (_abs(vector3d.magnitude() - 1.f) > .01f) ) {
		VERIFY2				( false, make_string("non-normalized direction passed [%f][%f][%f]", VPUSH(vector3d)) );
		vector3d.normalize	( );
	}
	stalker->sight().setup	(sight_type,vector3d,torso_look);
}

void CScriptGameObject::set_sight		(SightManager::ESightType sight_type, Fvector *vector3d)
{
	MakeObj(CAI_Stalker,stalker);
	if ( (sight_type == SightManager::eSightTypeDirection) && vector3d && (_abs(vector3d->magnitude() - 1.f) > .01f) ) {
		VERIFY2				( false, make_string("non-normalized direction passed [%f][%f][%f]", VPUSH(*vector3d)) );
		vector3d->normalize	( );
	}
	stalker->sight().setup	(sight_type,vector3d);
}

void CScriptGameObject::set_sight		(CScriptGameObject *object_to_look)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->sight().setup	(&object_to_look->object());
}

void CScriptGameObject::set_sight		(CScriptGameObject *object_to_look, bool torso_look)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->sight().setup	(&object_to_look->object(),torso_look);
}

void CScriptGameObject::set_sight		(CScriptGameObject *object_to_look, bool torso_look, bool fire_object)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->sight().setup	(&object_to_look->object(),torso_look,fire_object);
}

void CScriptGameObject::set_sight		(CScriptGameObject *object_to_look, bool torso_look, bool fire_object, bool no_pitch)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->sight().setup	(CSightAction(&object_to_look->object(),torso_look,fire_object,no_pitch));
}

void CScriptGameObject::set_sight		(const CMemoryInfo *memory_object, bool	torso_look)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->sight().setup	(memory_object,torso_look);
}

// CAI_Stalker
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

u32	CScriptGameObject::GetInventoryObjectCount() const
{
	RMakeObj(CInventoryOwner,invOwner,0);
	return			(invOwner->inventory().dwfGetObjectCount());
}

CScriptGameObject	*CScriptGameObject::GetActiveItem()
{
	RMakeObj(CInventoryOwner,invOwner,nullptr);
	if (invOwner->inventory().ActiveItem())
		return		(invOwner->inventory().ActiveItem()->object().lua_game_object());
	return nullptr;
}

CScriptGameObject	*CScriptGameObject::GetObjectByName	(LPCSTR caObjectName) const
{
	RMakeObj(CInventoryOwner,invOwner,nullptr);
	CInventoryItem	*l_tpInventoryItem = invOwner->inventory().GetItemFromInventory(caObjectName);
	CGameObject		*l_tpGameObject = smart_cast<CGameObject*>(l_tpInventoryItem);
	if (!l_tpGameObject)
		return nullptr;
	return l_tpGameObject->lua_game_object();
}

CScriptGameObject	*CScriptGameObject::GetObjectByIndex	(int iIndex) const
{
	RMakeObj(CInventoryOwner,invOwner,nullptr);
	CInventoryItem	*l_tpInventoryItem = invOwner->inventory().tpfGetObjectByIndex(iIndex);
	CGameObject		*l_tpGameObject = smart_cast<CGameObject*>(l_tpInventoryItem);
	if (!l_tpGameObject)
		return nullptr;
	return l_tpGameObject->lua_game_object();
}

void CScriptGameObject::EnableAnomaly()
{
	MakeObj(CCustomZone,zone);
	zone->ZoneEnable();
}

void CScriptGameObject::DisableAnomaly()
{
	MakeObj(CCustomZone,zone);
	zone->ZoneDisable();
}

float CScriptGameObject::GetAnomalyPower()
{
	RMakeObj(CCustomZone,zone,0.0f);
	return zone->GetMaxPower();
}

void CScriptGameObject::SetAnomalyPower(float p)
{
	MakeObj(CCustomZone,zone);
	zone->SetMaxPower(p);
}

float CScriptGameObject::GetArtefactHealthRestoreSpeed()
{
	RMakeObj(CArtefact,artefact,0.0f);
	return artefact->GetHealthPower();
}

float CScriptGameObject::GetArtefactRadiationRestoreSpeed()
{
	RMakeObj(CArtefact,artefact,0.0f);
	return artefact->GetRadiationPower();
}

float CScriptGameObject::GetArtefactSatietyRestoreSpeed()
{
	RMakeObj(CArtefact,artefact,0.0f);
	return artefact->GetSatietyPower();
}

float CScriptGameObject::GetArtefactPowerRestoreSpeed()
{
	RMakeObj(CArtefact,artefact,0.0f);
	return artefact->GetPowerPower();
}

float CScriptGameObject::GetArtefactBleedingRestoreSpeed()
{
	RMakeObj(CArtefact,artefact,0.0f);
	return artefact->GetBleedingPower();
}

float CScriptGameObject::GetArtefactImmunity(ALife::EHitType hit_type)
{
	RMakeObj(CArtefact,artefact,0.0f);
	return artefact->GetImmunity(hit_type);
}

void CScriptGameObject::SetArtefactHealthRestoreSpeed(float value)
{
	MakeObj(CArtefact,artefact);
	artefact->SetHealthPower(value);
}

void CScriptGameObject::SetArtefactRadiationRestoreSpeed(float value)
{
	MakeObj(CArtefact,artefact);
	artefact->SetRadiationPower(value);
}

void CScriptGameObject::SetArtefactSatietyRestoreSpeed(float value)
{
	MakeObj(CArtefact,artefact);
	artefact->SetSatietyPower(value);
}

void CScriptGameObject::SetArtefactPowerRestoreSpeed(float value)
{
	MakeObj(CArtefact,artefact);
	artefact->SetPowerPower(value);
}

void CScriptGameObject::SetArtefactBleedingRestoreSpeed(float value)
{
	MakeObj(CArtefact,artefact);
	artefact->SetBleedingPower(value);
}

void CScriptGameObject::SetArtefactImmunity(ALife::EHitType hit_type, float value)
{
	MakeObj(CArtefact,artefact);
	artefact->SetImmunity(hit_type, value);
}

void CScriptGameObject::ChangeAnomalyIdlePart(LPCSTR name, bool bIdleLight)
{
	MakeObj(CCustomZone,zone);
	zone->ChangeIdleParticles(name, bIdleLight);
}

float CScriptGameObject::GetAnomalyRadius()
{
	RMakeObj(CCustomZone,zone,0.0f);
	return zone->GetEffectiveRadius();
}

void CScriptGameObject::SetAnomalyRadius(float p)
{
	MakeObj(CCustomZone,zone);
	zone->SetEffectiveRadius(p);
}

void CScriptGameObject::MoveAnomaly(Fvector pos)
{
	MakeObj(CCustomZone,zone);
	zone->MoveScript(pos);
}

bool CScriptGameObject::weapon_strapped	() const
{
	RMakeObj(CAI_Stalker,stalker,false);
	return stalker->weapon_strapped();
}

bool CScriptGameObject::weapon_unstrapped	() const
{
	RMakeObj(CAI_Stalker,stalker,false);
	return stalker->weapon_unstrapped();
}

bool CScriptGameObject::path_completed	() const
{
	RMakeObj(CCustomMonster,monster,false);
	return			(monster->movement().path_completed());
}

void CScriptGameObject::patrol_path_make_inactual	()
{
	MakeObj(CCustomMonster,monster);
	monster->movement().patrol().make_inactual();
}


Fvector	CScriptGameObject::head_orientation		() const
{
	RMakeObj(CAI_Stalker,stalker,Fvector().set(flt_max,flt_max,flt_max));
	const SRotation	&r = stalker->movement().head_orientation().current;
	return			(Fvector().setHP(-r.yaw,-r.pitch));
}

void CScriptGameObject::info_add(LPCSTR text)
{
#ifdef DEBUG
	DBG().object_info(&object(),this).add_item	(text, D3DCOLOR_XRGB(255,0,0), 0);
#endif
}

void CScriptGameObject::info_clear()
{
#ifdef DEBUG
	DBG().object_info(&object(),this).clear		();
#endif
}

void CScriptGameObject::jump(const Fvector &position, float factor)
{
	MakeObj(CBaseMonster,monster);
	monster->jump(position, factor);
}


void CScriptGameObject::make_object_visible_somewhen	(CScriptGameObject *object)
{
	MakeObj2(CAI_Stalker,stalker,&this->object());
	MakeObj2(CEntityAlive,entity_alive,&object->object());
	stalker->memory().make_object_visible_somewhen	(entity_alive);
}

void CScriptGameObject::sell_condition			(CScriptIniFile *ini_file, LPCSTR section)
{
	MakeObj(CInventoryOwner,inventory_owner);
	inventory_owner->trade_parameters().process	(CTradeParameters::action_sell(0),*ini_file,section);
}

void CScriptGameObject::sell_condition			(float friend_factor, float enemy_factor)
{
	MakeObj(CInventoryOwner,inventory_owner);
	inventory_owner->trade_parameters().default_factors	(
		CTradeParameters::action_sell(0),
		CTradeFactors(
			friend_factor,
			enemy_factor
		)
	);
}

void CScriptGameObject::buy_condition			(CScriptIniFile *ini_file, LPCSTR section)
{
	MakeObj(CInventoryOwner,inventory_owner);
	inventory_owner->trade_parameters().process	(CTradeParameters::action_buy(0),*ini_file,section);
}

void CScriptGameObject::buy_condition			(float friend_factor, float enemy_factor)
{
	MakeObj(CInventoryOwner,inventory_owner);
	inventory_owner->trade_parameters().default_factors	(
		CTradeParameters::action_buy(0),
		CTradeFactors(
			friend_factor,
			enemy_factor
		)
	);
}

void CScriptGameObject::show_condition			(CScriptIniFile *ini_file, LPCSTR section)
{
	MakeObj(CInventoryOwner,inventory_owner);
	inventory_owner->trade_parameters().process	(
		CTradeParameters::action_show(0),
		*ini_file,
		section
	);
}

void CScriptGameObject::buy_supplies			(CScriptIniFile *ini_file, LPCSTR section)
{
	MakeObj(CInventoryOwner,inventory_owner);
	inventory_owner->buy_supplies(
		*ini_file,
		section
	);
}

void CScriptGameObject::buy_item_condition_factor(float factor)
{
	MakeObj(CInventoryOwner,inventory_owner);
	inventory_owner->trade_parameters().buy_item_condition_factor = factor;
}

void sell_condition								(CScriptIniFile *ini_file, LPCSTR section)
{
	default_trade_parameters().process	(CTradeParameters::action_sell(0),*ini_file,section);
}

void sell_condition								(float friend_factor, float enemy_factor)
{
	default_trade_parameters().default_factors	(
		CTradeParameters::action_sell(0),
		CTradeFactors(
			friend_factor,
			enemy_factor
		)
	);
}

void buy_condition								(CScriptIniFile *ini_file, LPCSTR section)
{
	default_trade_parameters().process	(CTradeParameters::action_buy(0),*ini_file,section);
}

void buy_condition								(float friend_factor, float enemy_factor)
{
	default_trade_parameters().default_factors	(
		CTradeParameters::action_buy(0),
		CTradeFactors(
			friend_factor,
			enemy_factor
		)
	);
}

void show_condition								(CScriptIniFile *ini_file, LPCSTR section)
{
	default_trade_parameters().process	(CTradeParameters::action_show(0),*ini_file,section);
}

LPCSTR CScriptGameObject::sound_prefix			() const
{
	RMakeObj(CCustomMonster,monster,NULL);
	return									(*monster->sound().sound_prefix());
}

void CScriptGameObject::sound_prefix			(LPCSTR sound_prefix)
{
	MakeObj(CCustomMonster,monster);
	monster->sound().sound_prefix	(sound_prefix);
}

bool CScriptGameObject::is_weapon_going_to_be_strapped	( CScriptGameObject const* object ) const
{
	if ( !object ) {
		RCAST_ERR(CScriptGameObject,object,false);
		return								false;
	}

	RMakeObj2(CAI_Stalker,stalker,false,&this->object());
	return									stalker->is_weapon_going_to_be_strapped	( &object->object() );
}

//Alundaio: Taken from Radium
u16 CScriptGameObject::AmmoGetCount()
{
	RMakeObj(CWeaponAmmo,ammo,0);
	return ammo->m_boxCurr;
}

void CScriptGameObject::AmmoSetCount(u16 count)
{
	MakeObj(CWeaponAmmo,ammo);
	ammo->m_boxCurr = count;
}

u16 CScriptGameObject::AmmoBoxSize()
{
	RMakeObj(CWeaponAmmo,ammo,0);
	return ammo->m_boxSize;
}

void CScriptGameObject::AttachVehicle(CScriptGameObject* veh) { AttachVehicle(veh, false); }
void CScriptGameObject::AttachVehicle(CScriptGameObject* veh, const bool bForce)
{
	MakeObj(CActor,actor);
	MakeObj2(CHolderCustom,vehicle,&veh->object());
	actor->use_HolderEx(vehicle, bForce);
}

void CScriptGameObject::DetachVehicle() { DetachVehicle(false); }
void CScriptGameObject::DetachVehicle(const bool bForce)
{
	MakeObj(CActor,actor);
	actor->use_HolderEx(nullptr, bForce);
}

CScriptGameObject* CScriptGameObject::GetAttachedVehicle()
{
	RMakeObj(CActor,actor,nullptr);
	RMakeObj2(CHolderCustom,H,nullptr,actor->Holder());
	RMakeObj2(CGameObject,GO,nullptr,H);

	return GO->lua_game_object();
}

void CScriptGameObject::SwitchState(u32 state)
{
	CWeapon* Weapon = object().cast_weapon();
	if (Weapon)
	{
		Weapon->SwitchState(state);
		return;
	}

	CInventoryItem* IItem = object().cast_inventory_item();
	if (IItem)
	{
		CHudItem* itm = IItem->cast_hud_item();
		if (itm)
			itm->SwitchState(state);
	}
}

u32 CScriptGameObject::GetState()
{
	CWeapon* Weapon = object().cast_weapon();
	if (Weapon)
		return Weapon->GetState();

	CInventoryItem* IItem = object().cast_inventory_item();
	if (IItem)
	{
		CHudItem* itm = IItem->cast_hud_item();
		if (itm)
			return itm->GetState();
	}

	return 65535;
}

bool CScriptGameObject::WeaponInGrenadeMode()
{
	CWeaponMagazinedWGrenade* wpn = smart_cast<CWeaponMagazinedWGrenade*>(&object());
	if (!wpn)
		return false;

	return wpn->m_bGrenadeMode;
}

void CScriptGameObject::SetBoneVisible(pcstr bone_name, bool bVisibility) { SetBoneVisible(bone_name, bVisibility, true); }
void CScriptGameObject::SetBoneVisible(pcstr bone_name, bool bVisibility, bool bRecursive)
{
	IKinematics* k = object().Visual()->dcast_PKinematics();

	if (!k)
		return;

	u16 bone_id = k->LL_BoneID(bone_name);
	if (bone_id == BI_NONE)
		return;

	if (bVisibility == !k->LL_GetBoneVisible(bone_id))
		k->LL_SetBoneVisible(bone_id, bVisibility, bRecursive);
}

bool CScriptGameObject::IsBoneVisible(pcstr bone_name)
{
	IKinematics* k = object().Visual()->dcast_PKinematics();

	if (!k)
		return false;

	u16 bone_id = k->LL_BoneID(bone_name);
	if (bone_id == BI_NONE)
		return false;

	return k->LL_GetBoneVisible(bone_id);
}

float CScriptGameObject::GetLuminocityHemi()
{
	CGameObject* e = smart_cast<CGameObject*>(&object());
	if (!e || !e->renderable_ROS())
		return 0.0f;
	return e->renderable_ROS()->get_luminocity_hemi();
}

float CScriptGameObject::GetLuminocity()
{
	CGameObject* e = smart_cast<CGameObject*>(&object());
	if (!e || !e->renderable_ROS())
		return 0.0f;
	return e->renderable_ROS()->get_luminocity();
}

void CScriptGameObject::ForceSetPosition(Fvector pos) { ForceSetPosition(pos, false); }
void CScriptGameObject::ForceSetPosition(Fvector pos, bool bActivate)
{
	Fmatrix M = object().XFORM();
	M.translate(pos);
	object().ForceTransform(M);
	CPhysicsShellHolder* sh = object().cast_physics_shell_holder();
	if (sh)
	{
		if (bActivate)
			sh->activate_physic_shell();
		if (sh->PPhysicsShell())
			sh->PPhysicsShell()->SetTransform(M, mh_unspecified);
	}
}

void CScriptGameObject::IterateFeelTouch(const luabind::functor<void>& functor)
{
	Feel::Touch* touch = smart_cast<Feel::Touch*>(&object());
	if (!touch)
		return;
	for (const auto& game_object : touch->feel_touch)
	{
		// Xottab_DUTY: Do we need this cast from IGameObject* to IGameObject* ?
		CObject* o = smart_cast<CObject*>(game_object);
		if (o)
			functor(o->ID());
	}
}

void CScriptGameObject::SetSpatialType(u32 sptype)
{
	object().spatial.type = sptype;
}

u32 CScriptGameObject::GetSpatialType()
{
	return object().spatial.type;
}

u8 CScriptGameObject::GetRestrictionType()
{
	RMakeObj(CSpaceRestrictor,restr,u8(-1));
	return restr->m_space_restrictor_type;
}

void CScriptGameObject::SetRestrictionType(u8 type)
{
	MakeObj(CSpaceRestrictor,restr);
	restr->m_space_restrictor_type = type;
	if (type != RestrictionSpace::eRestrictorTypeNone)
		Level().space_restriction_manager().register_restrictor(restr, RestrictionSpace::ERestrictorTypes(type));
}

void CScriptGameObject::RemoveDanger(const CDangerObject& dobject)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->memory().danger().remove(dobject);
}

void CScriptGameObject::SetRemainingUses(u8 value)
{
	CInventoryItem* IItm = object().cast_inventory_item();
	if (!IItm)
		return;

	CEatableItem* eItm = IItm->cast_eatable_item();
	if (!eItm)
		return;

	eItm->SetRemainingUses(value);
}

u8 CScriptGameObject::GetRemainingUses()
{
	CInventoryItem* IItm = object().cast_inventory_item();
	if (!IItm)
		return 0;

	CEatableItem* eItm = IItm->cast_eatable_item();
	if (!eItm)
		return 0;

	return eItm->GetPortionsNum();
}

u8 CScriptGameObject::GetMaxUses()
{
	CInventoryItem* IItm = object().cast_inventory_item();
	if (!IItm)
		return 0;

	CEatableItem* eItm = IItm->cast_eatable_item();
	if (!eItm)
		return 0;

	return eItm->GetMaxUses();
}

void CScriptGameObject::DestroyObject()
{
	object().DestroyObject();
}
//-Alundaio