#include "stdafx.h"
#include "pch_script.h"
#include "Actor.h"
#include "ActorEffector.h"
#include "weapon.h"
#include "mercuryball.h"
#include "inventory.h"
#include "character_info.h"
#include "xr_level_controller.h"
#include "UsableScriptObject.h"
#include "customzone.h"
#include "../xrEngine/gamemtllib.h"
#include "ui/UIMainIngameWnd.h"
#include "UIGameCustom.h"
#include "Grenade.h"
#include "WeaponRPG7.h"
#include "ExplosiveRocket.h"
#include "game_cl_base.h"
#include "Level.h"
#include "clsid_game.h"
#include "hudmanager.h"
#include "ui\UIStatic.h"
#include "ui\UIPdaWnd.h"
#include "ui\UIInventoryWnd.h"
#include "ui\UICarBodyWnd.h"
#include "UIGameSP.h"
#include "string_table.h"
#include "AdvancedXrayGameConstants.h"
#include "script_engine.h"
#include "ai_space.h"
#include "player_hud.h"
#include "GamePersistent.h"

#define PICKUP_INFO_COLOR 0xFFDDDDDD

extern bool g_block_all_except_movement;
extern bool g_actor_allow_ladder;

std::atomic<bool> isHidingInProgressInv(false);
std::atomic<bool> TakeItemAnimNeeded(false);

void CActor::feel_touch_new				(CObject* O)
{
}

void CActor::feel_touch_delete	(CObject* O)
{
	CPhysicsShellHolder* sh=smart_cast<CPhysicsShellHolder*>(O);
	if(sh&&sh->character_physics_support()) m_feel_touch_characters--;
}

BOOL CActor::feel_touch_contact		(CObject *O)
{
	CInventoryItem	*item = smart_cast<CInventoryItem*>(O);
	CInventoryOwner	*inventory_owner = smart_cast<CInventoryOwner*>(O);

	if (item && item->Useful() && !item->object().H_Parent()) 
		return TRUE;

	if(inventory_owner && inventory_owner != smart_cast<CInventoryOwner*>(this))
	{
		CPhysicsShellHolder* sh=smart_cast<CPhysicsShellHolder*>(O);
		if(sh&&sh->character_physics_support()) m_feel_touch_characters++;
		return TRUE;
	}

	return		(FALSE);
}

BOOL CActor::feel_touch_on_contact	(CObject *O)
{
	CCustomZone	*custom_zone = smart_cast<CCustomZone*>(O);
	if (!custom_zone)
		return	(TRUE);

	Fsphere		sphere;
	sphere.P	= Position();
	sphere.R	= EPS_L;
	if (custom_zone->inside(sphere))
		return	(TRUE);

	return		(FALSE);
}

void CActor::PickupModeOn()
{
	m_bPickupMode = true;
}

void CActor::PickupModeOff()
{
	m_bPickupMode = false;
}

BOOL CActor::CanPickItem(const CFrustum& frustum, const Fvector& from, CObject* item)
{
	if (!item->getVisible())
		return FALSE;

	struct callback_data
	{
		BOOL bOverlaped;
		CObject* item;
	} data;

	data.bOverlaped = FALSE;
	data.item = item;

	Fvector dir, to;
	item->Center(to);
	float range = dir.sub(to, from).magnitude();

	if (range > 0.25f)
	{
		if (frustum.testSphere_dirty(to, item->Radius()))
		{
			dir.div(range);
			collide::ray_defs RD(from, dir, range, CDB::OPT_CULL, collide::rqtBoth);
			VERIFY(!fis_zero(RD.dir.square_magnitude()));
			RQR.r_clear();
			Level().ObjectSpace.RayQuery(RQR, RD,
				[](collide::rq_result& result, LPVOID params) -> BOOL
				{
					callback_data* data = (callback_data*)params;

					if (result.O)
					{
						if (Level().CurrentEntity() == result.O)
							return TRUE;
						else
						{
							if (result.O->spatial.type & STYPE_OBSTACLE)
								data->bOverlaped = TRUE;

							CInventoryItem* inventory_item = smart_cast<CInventoryItem*>(data->item);
							
							if (inventory_item && inventory_item->CanPickThroughGeom())
								return TRUE;
						}
					}
					else
					{
						CDB::TRI* T = Level().ObjectSpace.GetStaticTris() + result.element;
						
						if (GMLib.GetMaterialByIdx(T->material)->Flags.is(SGameMtl::flPassable))
							return TRUE;
					}

					data->bOverlaped = TRUE;
					return FALSE;
				},
				&data, NULL, item);
		}
	}
	return !data.bOverlaped;
}

#include "ai\monsters\ai_monster_utils.h"

void CActor::PickupModeUpdate()
{
	const auto pGameSP = smart_cast<CUIGameSP*>(HUD().GetUI()->UIGame());
	const auto pda = pGameSP->PdaMenu;

	if(!m_bPickupMode) return;
	if (GameID() != GAME_SINGLE) return;
	if (pda->IsShown()) return;

	//подбирание объекта
	if(inventory().m_pTarget && inventory().m_pTarget->Useful() &&
		m_pUsableObject && m_pUsableObject->nonscript_usable() &&
		!Level().m_feel_deny.is_object_denied(smart_cast<CGameObject*>(inventory().m_pTarget)) )
	{
		CInventoryItem* inv_item = smart_cast<CInventoryItem*>(m_pUsableObject);
		if (GameConstants::GetLimitedInventory() && !inv_item->IsQuestItem() && MaxCarryInvCapacity() < (GetInventoryFullness() + inv_item->GetOccupiedInvSpace()))
		{
			SDrawStaticStruct* _s = HUD().GetUI()->UIGame()->AddCustomStatic("backpack_full", true);
			_s->wnd()->SetText(CStringTable().translate("st_backpack_full").c_str());

			return;
		}

		shared_str take_precond = inv_item->GetTakePreconditionFunc();
		if (xr_strcmp(take_precond, ""))
		{
			luabind::functor<bool> m_functor;
			if (ai().script_engine().functor(take_precond.c_str(), m_functor))
			{
				if (!m_functor())
					return;

#ifdef DEBUG
				Msg("[ActorFeel::PickupModeUpdate]: Lua function [%s] called from item [%s] by use_precondition.", take_precond.c_str(), inv_item->m_section_id.c_str());
#endif
			}
#ifdef DEBUG
			else
			{
				Msg("[ActorFeel::PickupModeUpdate]: ERROR: Lua function [%s] called from item [%s] by use_precondition not found!", take_precond.c_str(), inv_item->m_section_id.c_str());
			}
#endif
		}

		if (CUIGameSP* pGameSP = smart_cast<CUIGameSP*>(HUD().GetUI()->UIGame()))
		{
			const bool use_pickup_anim = (Position().distance_to(inventory().m_pTarget->cast_game_object()->Position()) > 0.2f)
				&& !pGameSP->InventoryMenu->IsShown()
				&& !pGameSP->UICarBodyMenu->IsShown()
				&& !Actor()->m_bActionAnimInProcess
				&& pAdvancedSettings->line_exist("actions_animations", "take_item_section");

			m_pObjectToTake = inventory().m_pTarget->cast_game_object();

			TakeItemAnimCheck(use_pickup_anim);
		}
	}

	if (eacFirstEye != cam_active)
		feel_touch_update(Position(), m_fPickupInfoRadius);
	else
		feel_touch_update(get_bone_position(this, "bip01_spine"), m_fPickupInfoRadius);
	
	CFrustum frustum;
	frustum.CreateFromMatrix(Device.mFullTransform,FRUSTUM_P_LRTB|FRUSTUM_P_FAR);
	//. slow (ray-query test)
	for(xr_vector<CObject*>::iterator it = feel_touch.begin(); it != feel_touch.end(); it++)
		if (CanPickItem(frustum, Device.vCameraPosition, *it) && m_fPickupInfoRadius > 0)
			PickupInfoDraw(*it);
}

#include "../xrEngine/CameraBase.h"
BOOL	g_b_COD_PickUpMode = TRUE;
void	CActor::PickupModeUpdate_COD	()
{
	if (Level().CurrentViewEntity() != this || !g_b_COD_PickUpMode)
		return;
		
	if (!g_Alive() || eacFirstEye != cam_active) 
	{
		HUD().GetUI()->UIMainIngameWnd->SetPickUpItem(NULL);
		return;
	};
	
	CFrustum frustum;
	frustum.CreateFromMatrix(Device.mFullTransform,FRUSTUM_P_LRTB|FRUSTUM_P_FAR);

	//---------------------------------------------------------------------------
	ISpatialResult.clear_not_free	();
	g_SpatialSpace->q_frustum(ISpatialResult, 0, STYPE_COLLIDEABLE, frustum);
	//---------------------------------------------------------------------------

	float maxlen = 1000.0f;
	CInventoryItem* pNearestItem = NULL;
	for (u32 o_it=0; o_it<ISpatialResult.size(); o_it++)
	{
		ISpatial*		spatial_	= ISpatialResult[o_it];
		CInventoryItem*	pIItem	= smart_cast<CInventoryItem*> (spatial_->dcast_CObject        ());

		if (0 == pIItem) continue;
		if (pIItem->object().H_Parent() != NULL) continue;
		if (!pIItem->CanTake()) continue;
		if ( smart_cast<CExplosiveRocket*>( &pIItem->object() ) )	continue;

		CGrenade*	pGrenade	= smart_cast<CGrenade*> (spatial_->dcast_CObject        ());
		if (pGrenade && !pGrenade->Useful()) continue;

		CMissile*	pMissile	= smart_cast<CMissile*> (spatial_->dcast_CObject        ());
		if (pMissile && !pMissile->Useful()) continue;
		
		Fvector A, B, tmp; 
		pIItem->object().Center			(A);
		if (A.distance_to_sqr(Position())>4) continue;

		tmp.sub(A, cam_Active()->vPosition);
		B.mad(cam_Active()->vPosition, cam_Active()->vDirection, tmp.dotproduct(cam_Active()->vDirection));
		float len = B.distance_to_sqr(A);
		if (len > 1) continue;

		if (maxlen>len && !pIItem->object().getDestroy())
		{
			maxlen = len;
			pNearestItem = pIItem;
		};
	}

	if(pNearestItem)
	{
		CFrustum					frustum_;
		frustum_.CreateFromMatrix	(Device.mFullTransform,FRUSTUM_P_LRTB|FRUSTUM_P_FAR);
		if (!CanPickItem(frustum_, Device.vCameraPosition, &pNearestItem->object()))
			pNearestItem = NULL;
	}

	if (pNearestItem && pNearestItem->cast_game_object())
	{
		if (Level().m_feel_deny.is_object_denied(pNearestItem->cast_game_object()))
				pNearestItem = NULL;
	}
	if (pNearestItem && pNearestItem->cast_game_object())
	{
		if(!pNearestItem->cast_game_object()->getVisible())
				pNearestItem = NULL;
	}

	HUD().GetUI()->UIMainIngameWnd->SetPickUpItem(pNearestItem);

	if (pNearestItem && m_bPickupMode)
	{
		if (GameConstants::GetLimitedInventory() && !pNearestItem->IsQuestItem() && MaxCarryInvCapacity() < (GetInventoryFullness() + pNearestItem->GetOccupiedInvSpace()))
		{
			SDrawStaticStruct* _s = HUD().GetUI()->UIGame()->AddCustomStatic("backpack_full", true);
			_s->wnd()->SetText(CStringTable().translate("st_backpack_full").c_str());

			return;
		}

		shared_str take_precond = pNearestItem->GetTakePreconditionFunc();
		if (xr_strcmp(take_precond, ""))
		{
			luabind::functor<bool> m_functor;
			if (ai().script_engine().functor(take_precond.c_str(), m_functor))
			{
				if (!m_functor())
					return;

#ifdef DEBUG
				Msg("[ActorFeel::PickupModeUpdate_COD]: Lua function [%s] called from item [%s] by use_precondition.", take_precond.c_str(), pNearestItem->m_section_id.c_str());
#endif
			}
#ifdef DEBUG
			else
			{
				Msg("[ActorFeel::PickupModeUpdate_COD]: ERROR: Lua function [%s] called from item [%s] by use_precondition not found!", take_precond.c_str(), pNearestItem->m_section_id.c_str());
			}
#endif
		}

		//подбирание объекта
		if (CUIGameSP* pGameSP = smart_cast<CUIGameSP*>(HUD().GetUI()->UIGame()))
		{
			const bool use_pickup_anim = (Position().distance_to(pNearestItem->cast_game_object()->Position()) > 0.2f)
				&& !pGameSP->InventoryMenu->IsShown()
				&& !pGameSP->UICarBodyMenu->IsShown()
				&& !Actor()->m_bActionAnimInProcess
				&& pAdvancedSettings->line_exist("actions_animations", "take_item_section");

			m_pObjectToTake = pNearestItem->cast_game_object();

			TakeItemAnimCheck(use_pickup_anim);
		}
		
		PickupModeOff();
	}
};

void CActor::PickupInfoDraw(CObject* object)
{
	LPCSTR draw_str = NULL;
	
	CInventoryItem* item = smart_cast<CInventoryItem*>(object);
//.	CInventoryOwner* inventory_owner = smart_cast<CInventoryOwner*>(object);
//.	VERIFY(item || inventory_owner);
	if(!item)		return;

	Fmatrix			res;
	res.mul			(Device.mFullTransform,object->XFORM());
	Fvector4		v_res;
	Fvector			shift;

	draw_str = item->Name/*Complex*/();
	shift.set(0,0,0);

	res.transform(v_res,shift);

	if (v_res.z < 0 || v_res.w < 0)	return;
	if (v_res.x < -1.f || v_res.x > 1.f || v_res.y<-1.f || v_res.y>1.f) return;

	float x = (1.f + v_res.x)/2.f * (Device.dwWidth);
	float y = (1.f - v_res.y)/2.f * (Device.dwHeight);

	UI().Font().pFontLetterica16Russian->SetAligment	(CGameFont::alCenter);
	UI().Font().pFontLetterica16Russian->SetColor		(PICKUP_INFO_COLOR);
	UI().Font().pFontLetterica16Russian->Out			(x,y,draw_str);
}

void CActor::feel_sound_new(CObject* who, int type, CSound_UserDataPtr user_data, const Fvector& Position, float power)
{
	if(who == this)
		m_snd_noise = _max(m_snd_noise,power);
}

void CActor::TakeItemAnimCheck(bool use_pickup_anim)
{
	if (m_bActionAnimInProcess)
		return;

	m_bUsePickupAnim = use_pickup_anim;

	if (isHidingInProgressInv.load())
		return;

	//CCustomDetector* pDet = smart_cast<CCustomDetector*>(inventory().ItemFromSlot(DETECTOR_SLOT));

	//if (!pDet || pDet->IsHidden())
	//{
		TakeItemAnim(use_pickup_anim);
		return;
	//}
	//else
	//{
	//	if (!use_pickup_anim)
	//	{
	//		TakeItemAnim(use_pickup_anim);
	//		return;
	//	}
	//}

	isHidingInProgressInv.store(true);

	std::thread hidingThread([/*&, pDet*/]
		{
			//while (pDet && !pDet->IsHidden())
			//	pDet->HideDetector(true);

			isHidingInProgressInv.store(false);
			TakeItemAnimNeeded.store(true);
		});

	hidingThread.detach();
}

void CActor::TakeItemAnim(bool use_pickup_anim)
{
	if (use_pickup_anim && !m_pObjectToTake)
		return;

	LPCSTR anim_sect = READ_IF_EXISTS(pAdvancedSettings, r_string, "actions_animations", "take_item_section", nullptr);

	if (!anim_sect || !use_pickup_anim)
	{
		Game().SendPickUpEvent(ID(), m_pObjectToTake->ID());
		return;
	}

	CWeapon* Wpn = smart_cast<CWeapon*>(inventory().ActiveItem());

	if (Wpn && !(Wpn->GetState() == CWeapon::eIdle))
		return;

	m_bTakeItemActivated = true;

	int anim_timer = READ_IF_EXISTS(pSettings, r_u32, anim_sect, "anim_timing", 0);

	g_block_all_except_movement = true;
	g_actor_allow_ladder = false;

	LPCSTR use_cam_effector = READ_IF_EXISTS(pSettings, r_string, anim_sect, !Wpn ? "anim_camera_effector" : "anim_camera_effector_weapon", nullptr);
	float effector_intensity = READ_IF_EXISTS(pSettings, r_float, anim_sect, "cam_effector_intensity", 1.0f);
	float anim_speed = READ_IF_EXISTS(pSettings, r_float, anim_sect, "anim_speed", 1.0f);

	if (pSettings->line_exist(anim_sect, "anm_use"))
	{
		bool auto_attach_enabled = READ_IF_EXISTS(pSettings, r_bool, anim_sect, "auto_attach_enabled", false);

		g_player_hud->script_anim_play(!inventory().GetActiveSlot() ? 2 : 1, anim_sect, !Wpn ? "anm_use" : "anm_use_weapon", true, anim_speed, auto_attach_enabled ? m_pObjectToTake->cNameVisual().c_str() : nullptr);

		if (use_cam_effector)
		{
			if (Wpn)
				g_player_hud->PlayBlendAnm(use_cam_effector, 0, anim_speed, effector_intensity, false);
			else
			{
				CEffectorCam* effector = Cameras().GetCamEffector((ECamEffectorType)effUseItem);

				if (!effector && use_cam_effector != nullptr)
					AddEffector(this, effUseItem, use_cam_effector, effector_intensity);
			}
		}

		m_iTakeAnimLength = Device.dwTimeGlobal + g_player_hud->motion_length_script(anim_sect, !Wpn ? "anm_use" : "anm_use_weapon", anim_speed);
	}

	if (pSettings->line_exist(anim_sect, "snd_using"))
	{
		if (m_action_anim_sound._feedback())
			m_action_anim_sound.stop();

		shared_str snd_name = pSettings->r_string(anim_sect, "snd_using");
		m_action_anim_sound.create(snd_name.c_str(), st_Effect, sg_SourceType);
		m_action_anim_sound.play(NULL, sm_2D);
	}

	m_iActionTiming = Device.dwTimeGlobal + anim_timer;

	m_bItemTaked = false;
	m_bActionAnimInProcess = true;
}

void CActor::UpdateUseAnim()
{
	if (TakeItemAnimNeeded.load())
	{
		TakeItemAnim(m_bUsePickupAnim);
		TakeItemAnimNeeded.store(false);
	}

	if (!m_bTakeItemActivated)
		return;

	if (!m_bActionAnimInProcess)
		return;

	bool IsActorAlive = g_pGamePersistent->GetActorAliveStatus();

	if ((m_iActionTiming <= Device.dwTimeGlobal && !m_bItemTaked) && IsActorAlive)
	{
		m_iActionTiming = Device.dwTimeGlobal;

		bool vis_status = READ_IF_EXISTS(pSettings, r_bool, m_pObjectToTake->cNameSect(), "visible_with_take_anim", true);

		g_player_hud->SetScriptItemVisible(vis_status);
		Game().SendPickUpEvent(ID(), m_pObjectToTake->ID());

		m_bItemTaked = true;
		m_pObjectToTake = nullptr;
	}

	if (m_bTakeItemActivated)
	{
		if ((m_iTakeAnimLength <= Device.dwTimeGlobal) || !IsActorAlive)
		{
			m_iTakeAnimLength = Device.dwTimeGlobal;
			m_iActionTiming = Device.dwTimeGlobal;
			m_action_anim_sound.stop();
			g_block_all_except_movement = false;
			g_actor_allow_ladder = true;
			m_bActionAnimInProcess = false;
			m_bTakeItemActivated = false;
		}
	}
}