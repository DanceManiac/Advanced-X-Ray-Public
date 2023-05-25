#include "pch_script.h"
#include "script_game_object.h"
#include "script_game_object_impl.h"
#include "ai/monsters/bloodsucker/bloodsucker.h"
#include "ai/monsters/zombie/zombie.h"
#include "script_sound_info.h"
#include "script_monster_hit_info.h"
#include "ai/monsters/monster_home.h"
#include "ai/monsters/poltergeist/poltergeist.h"

//////////////////////////////////////////////////////////////////////////
//CAI_Bloodsucker
void CScriptGameObject::set_invisible(bool val)
{
	MakeObj(CAI_Bloodsucker,monster);
	val ? monster->manual_activate() : monster->manual_deactivate();
}

void CScriptGameObject::set_manual_invisibility(bool val)
{
	MakeObj(CAI_Bloodsucker,monster);
	val ? monster->set_manual_control(true) : monster->set_manual_control(false);
	
}

void CScriptGameObject::bloodsucker_drag_jump(CScriptGameObject* e, LPCSTR e_str, const Fvector &position, float factor)
{
	MakeObj(CAI_Bloodsucker,monster);
	CGameObject *game_object = &e->object();
	CEntityAlive *entity_alive = smart_cast<CEntityAlive*>(game_object);
	monster->set_drag_jump(entity_alive, e_str, position, factor);
}

void CScriptGameObject::set_enemy(CScriptGameObject* e)
{
	MakeObj(CAI_Bloodsucker,monster);
	CGameObject *game_object = &e->object();
	CEntityAlive *entity_alive = smart_cast<CEntityAlive*>(game_object);
	monster->SetEnemy(entity_alive);
}

void CScriptGameObject::set_vis_state(float val)
{
	MakeObj(CAI_Bloodsucker,monster);
	if(val==1){
		monster->set_vis();
	}
	if(val==-1){
		monster->set_invis();
	}
}

void CScriptGameObject::off_collision(bool val)
{
	MakeObj(CAI_Bloodsucker,monster);
	monster->set_collision_off(val);
}

void CScriptGameObject::set_alien_control(bool val)
{
	MakeObj(CAI_Bloodsucker,monster);
	monster->set_alien_control(val);
}

CScriptSoundInfo CScriptGameObject::GetSoundInfo()
{
	CScriptSoundInfo	ret_val;
	CBaseMonster *l_tpMonster = smart_cast<CBaseMonster *>(&object());
	if (l_tpMonster) {
		if (l_tpMonster->SoundMemory.IsRememberSound()) {
			SoundElem se; 
			bool bDangerous;
			l_tpMonster->SoundMemory.GetSound(se, bDangerous);

			const CGameObject *pO = smart_cast<const CGameObject *>(se.who);
			ret_val.set((pO && !pO->getDestroy()) ?  pO->lua_game_object() : NULL, bDangerous, se.position, se.power, int(se.time));
		}
	} else
		Msg("! CScriptGameObject: cannot access class member GetSoundInfo");
	return			(ret_val);
}

CScriptMonsterHitInfo CScriptGameObject::GetMonsterHitInfo()
{
	CScriptMonsterHitInfo	ret_val;
	CBaseMonster *l_tpMonster = smart_cast<CBaseMonster *>(&object());
	if (l_tpMonster) {
		if (l_tpMonster->HitMemory.is_hit()) {
			CGameObject *pO = smart_cast<CGameObject *>(l_tpMonster->HitMemory.get_last_hit_object());
			ret_val.set((pO && !pO->getDestroy()) ?  pO->lua_game_object() : NULL, l_tpMonster->HitMemory.get_last_hit_dir(), l_tpMonster->HitMemory.get_last_hit_time());
		}
	} else
		Msg("! CScriptGameObject: cannot access class member GetMonsterHitInfo");
	return			(ret_val);
}

//////////////////////////////////////////////////////////////////////////
// CBaseMonster
void CScriptGameObject::skip_transfer_enemy(bool val)
{
	MakeObj(CBaseMonster,monster);
	monster->skip_transfer_enemy(val);
}

void CScriptGameObject::set_home(LPCSTR name, float r_min, float r_max, bool aggressive, float r_mid)
{
	MakeObj(CBaseMonster,monster);
	monster->Home->setup(name,r_min,r_max,aggressive, r_mid);
}
void CScriptGameObject::remove_home()
{
	MakeObj(CBaseMonster,monster);
	monster->Home->remove_home();
}

bool CScriptGameObject::fake_death_fall_down()
{
	RMakeObj(CZombie,monster,false);
	return monster->fake_death_fall_down();
}
void CScriptGameObject::fake_death_stand_up()
{
	MakeObj(CZombie,monster);
	monster->fake_death_stand_up();
}

void CScriptGameObject::berserk()
{
	MakeObj(CBaseMonster,monster);
	monster->set_berserk();
}

void CScriptGameObject::set_custom_panic_threshold(float value)
{
	MakeObj(CBaseMonster,monster);
	monster->set_custom_panic_threshold(value);
}

void CScriptGameObject::set_default_panic_threshold()
{
	MakeObj(CBaseMonster,monster);
	monster->set_default_panic_threshold();
}

void   CScriptGameObject::poltergeist_set_actor_ignore (bool ignore)
{
	MakeObj(CPoltergeist,monster);
	monster->set_actor_ignore(ignore);
}

bool   CScriptGameObject::poltergeist_get_actor_ignore ()
{
	RMakeObj(CPoltergeist,monster,false);
	return	monster->get_actor_ignore();
}