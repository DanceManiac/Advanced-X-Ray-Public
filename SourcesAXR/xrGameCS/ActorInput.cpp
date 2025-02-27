#include "stdafx.h"
#include <dinput.h>
#include "Actor.h"
#include "Torch.h"
#include "trade.h"
#include "../xrEngine/CameraBase.h"

#ifdef DEBUG
#	include "PHDebug.h"
#endif

#include "hit.h"
#include "PHDestroyable.h"
#include "Car.h"
#include "UIGameSP.h"
#include "inventory.h"
#include "level.h"
#include "game_cl_base.h"
#include "xr_level_controller.h"
#include "UsableScriptObject.h"
#include "actorcondition.h"
#include "actor_input_handler.h"
#include "string_table.h"
#include "UI/UIStatic.h"
#include "UI/UIActorMenu.h"
#include "UI/UIDragDropReferenceList.h"
#include "CharacterPhysicsSupport.h"
#include "InventoryBox.h"
#include "player_hud.h"
#include "../xrEngine/xr_input.h"
#include "flare.h"
#include "CustomDetector.h"
#include "clsid_game.h"
#include "HudManager.h"
#include "Weapon.h"
#include "WeaponMagazined.h"
#include "Grenade.h"
#include "CustomOutfit.h"
#include "script_engine.h"
#include "PDA.h"
#include "ui/UIPdaWnd.h"

#include "AdvancedXrayGameConstants.h"

extern int hud_adj_mode;
extern int hud_adj_item_idx;
bool g_block_actor_movement = false;

void CActor::IR_OnKeyboardPress(int cmd)
{
	if (m_blocked_actions.find((EGameActions)cmd) != m_blocked_actions.end()) return; // Real Wolf. 14.10.2014

	if (hud_adj_mode && pInput->iGetAsyncKeyState(DIK_LSHIFT))
	{
		if (pInput->iGetAsyncKeyState(DIK_RETURN) || pInput->iGetAsyncKeyState(DIK_BACKSPACE) ||
			pInput->iGetAsyncKeyState(DIK_DELETE))
			g_player_hud->tune(Ivector().set(0, 0, 0));

		return;
	}

	if (Remote())		return;

	if (IsTalking())	return;
	if (m_input_external_handler && !m_input_external_handler->authorized(cmd))	return;
	
	if (pInput->iGetAsyncKeyState(DIK_ADD))
		inventory().Action((u16)kWPN_ZOOM_INC, CMD_START);
	else if (pInput->iGetAsyncKeyState(DIK_SUBTRACT))
		inventory().Action((u16)kWPN_ZOOM_DEC, CMD_START);
	else if (pInput->iGetAsyncKeyState(DIK_HOME))
		inventory().Action((u16)kWPN_NV_CHANGE, CMD_START);

	switch (cmd)
	{
	case kWPN_FIRE:
		{
			if( (mstate_wishful & mcLookout) && !IsGameTypeSingle() ) return;

			u32 slot = inventory().GetActiveSlot();
			if(inventory().ActiveItem() && (slot==RIFLE_SLOT || slot==PISTOL_SLOT) )
				mstate_wishful &=~mcSprint;
			//-----------------------------
			if (OnServer())
			{
				NET_Packet P;
				P.w_begin(M_PLAYER_FIRE); 
				P.w_u16(ID());
				u_EventSend(P);
			}
		}break;
	default:
		{
		}break;
	}

	if (!g_Alive() || g_block_actor_movement) return;
	switch (cmd)
	{
		case kQUICK_USE_1:
		case kQUICK_USE_2:
		case kQUICK_USE_3:
		case kQUICK_USE_4:
		{
			const shared_str& item_name		= g_quick_use_slots[cmd-kQUICK_USE_1];
			if(item_name.size())
			{
				CEatableItem* itm = nullptr;

				for (auto& it : inventory().m_ruck)
				{
					CEatableItem* pEatable = smart_cast<CEatableItem*>(it);
					if (!pEatable)
						continue;
					if (pEatable->GetPortionsNum() == 1)
					{
						itm = pEatable;
						break;
					}
					if (pEatable->m_section_id == item_name && !itm || itm && (pEatable->GetPortionsNum() < itm->GetPortionsNum()))
						itm = pEatable;
				}

				if(itm)
				{
					if (IsGameTypeSingle())
					{
						inventory().ChooseItmAnimOrNot(itm);
					}
					else
					{
						inventory().ClientEat(itm);
					}

					static const bool enabled = READ_IF_EXISTS(pSettings, r_bool, "null_features", "show_item_used_hud_text", true);
					if (enabled && !inventory().ItmHasAnim(itm))
					{
						SDrawStaticStruct* _s		= HUD().GetUI()->UIGame()->AddCustomStatic("item_used", true);
						_s->m_endTime				= Device.fTimeGlobal+3.0f;
						string1024					str;
						strconcat					(sizeof(str),str,*CStringTable().translate("st_item_used"),": ", itm->NameItem());
						_s->wnd()->SetText			(str);
					}
					
					if (HUD().GetUI()->UIGame()->ActorMenu().m_pQuickSlot)
						HUD().GetUI()->UIGame()->ActorMenu().m_pQuickSlot->ReloadReferences(this);
				}
			}
		}break;

		case kUSE_BANDAGE:
		case kUSE_MEDKIT:
		{
			const shared_str& item_name = inventory().item((cmd == kUSE_BANDAGE) ? CLSID_IITEM_BANDAGE : CLSID_IITEM_MEDKIT)->m_section_id;
			if (item_name.size())
			{
				CEatableItem* itm = nullptr;

				for (auto& it : inventory().m_ruck)
				{
					CEatableItem* pEatable = smart_cast<CEatableItem*>(it);
					if (!pEatable)
						continue;
					if (pEatable->GetPortionsNum() == 1)
					{
						itm = pEatable;
						break;
					}
					if (pEatable->m_section_id == item_name && !itm || itm && (pEatable->GetPortionsNum() < itm->GetPortionsNum()))
						itm = pEatable;
				}
				if (itm)
				{
					if (IsGameTypeSingle())
					{
						inventory().ChooseItmAnimOrNot(itm);
					}
					else
					{
						inventory().ClientEat(itm);
					}

					if (GameConstants::GetHUD_UsedItemTextEnabled() && !inventory().ItmHasAnim(itm))
					{
						SDrawStaticStruct* _s = HUD().GetUI()->UIGame()->AddCustomStatic("item_used", true);
						_s->m_endTime = Device.fTimeGlobal + 3.0f;
						string1024 str;
						strconcat(sizeof(str), str, *CStringTable().translate("st_item_used"), ": ", itm->NameItem());
						_s->wnd()->SetText(str);
					}
				}
			}
		}break;
	};
	
	
	if(m_holder && kUSE != cmd)
	{
		m_holder->OnKeyboardPress			(cmd);
		if(m_holder->allowWeapon() && inventory().Action(cmd, CMD_START))		return;
		return;
	}else
		if(inventory().Action(cmd, CMD_START))					return;

#ifdef DEBUG
	if(psActorFlags.test(AF_NO_CLIP))
	{
		NoClipFly(cmd);
		return;
	}
#endif //DEBUG

	switch(cmd)
	{
	case kJUMP:		
		{
			mstate_wishful |= mcJump;
		}break;
	case kSPRINT_TOGGLE:	
		{
			CWeapon* W = smart_cast<CWeapon*>(inventory().ActiveItem());

			if (IsReloadingWeapon() && !GameConstants::GetReloadIfSprint())
			{
				if (m_iTrySprintCounter == 0) // don't interrupt reloading on first key press and skip sprint request
				{
					m_iTrySprintCounter++;

					return;
				}
				else if (m_iTrySprintCounter >= 1) // break reloading, if player insist(presses two or more times) and do sprint
				{
					W->StopAllSounds();
					W->SwitchState(CHUDState::EHudStates::eIdle);
				}
			}

			if (mstate_wishful & mcSprint && !GameConstants::GetReloadIfSprint())
				mstate_wishful &= ~mcSprint;
			else
				mstate_wishful |= mcSprint;
		}break;
	case kCROUCH:
		{
			if (psActorFlags.test(AF_CROUCH_TOGGLE))
				mstate_wishful ^= mcCrouch;
		}break;
	case kCAM_1:	cam_Set			(eacFirstEye);				break;
	case kCAM_2:	cam_Set			(eacLookAt);				break;
	case kCAM_3:	cam_Set			(eacFreeLook);				break;
	case kNIGHT_VISION:
		{
			if (hud_adj_mode)
				return;

			if (auto Wpn = smart_cast<CHudItem*>(inventory().ActiveItem()); Wpn && !Wpn->IsPending() && (Wpn->isHUDAnimationExist("anm_headlamp_on") || Wpn->isHUDAnimationExist("anm_headlamp_off")))
			{
				auto outfit = smart_cast<CCustomOutfit*>(inventory().ItemFromSlot(OUTFIT_SLOT));

				if (outfit && outfit->m_NightVisionSect.size())
				{
					Wpn->NightVisionSwitch = true;
					Wpn->SwitchState(CHUDState::EHudStates::eDeviceSwitch);
				}
			}
			else
				SwitchNightVision();
		} break;
	case kTORCH:
		{
			if (hud_adj_mode)
				return;

			if (auto Wpn = smart_cast<CHudItem*>(inventory().ActiveItem()); Wpn && !Wpn->IsPending() && (Wpn->isHUDAnimationExist("anm_nv_on") || Wpn->isHUDAnimationExist("anm_nv_off")))
			{
				if (smart_cast<CTorch*>(inventory().ItemFromSlot(TORCH_SLOT)))
				{
					Wpn->HeadLampSwitch = true;
					Wpn->SwitchState(CHUDState::EHudStates::eDeviceSwitch);
				}
			}
			else
				SwitchTorch();
		} break;
	case kCLEAN_MASK:
		{
			if (hud_adj_mode)
				return;

			if (auto Wpn = smart_cast<CHudItem*>(inventory().ActiveItem()); Wpn && !Wpn->IsPending() && Wpn->isHUDAnimationExist("anm_clean_mask"))
			{
				auto outfit = smart_cast<CCustomOutfit*>(inventory().ItemFromSlot(OUTFIT_SLOT));

				if (outfit && outfit->m_b_HasGlass)
				{
					Wpn->CleanMaskAction = true;
					Wpn->SwitchState(CHUDState::EHudStates::eDeviceSwitch);
				}
			}
			else
				CleanMaskAnimCheckDetector();

		} break;
	case kQUICK_KICK:
		{
			if (hud_adj_mode || m_bQuickWeaponBlocked)
				return;

			QuickKick();
		} break;
	case kQUICK_GRENADE:
		{
			if (!GameConstants::GetQuickThrowGrenadesEnabled() || hud_adj_mode || m_bQuickWeaponBlocked)
				return;

			CGrenade* grenade = smart_cast<CGrenade*>(inventory().ItemFromSlot(GRENADE_SLOT));
			
			if (grenade)
			{
				if (inventory().GetActiveSlot() == GRENADE_SLOT)
					return;

				m_last_active_slot = inventory().GetActiveSlot();

				inventory().Activate(GRENADE_SLOT, true);
				grenade->SetQuickThrowActive(true);

				if (grenade->isHUDAnimationExist("anm_throw_quick"))
					grenade->SwitchState(CGrenade::eThrowQuick);
			}
		} break;
	case kDETECTOR:
		{
			PIItem det_active					= inventory().ItemFromSlot(DETECTOR_SLOT);
			if(det_active)
			{
				CCustomDetector* det			= smart_cast<CCustomDetector*>(det_active);
				det->ToggleDetector				(g_player_hud->attached_item(0)!=NULL);
				return;
			}
		} break;
/*
	case kFLARE:{
			PIItem fl_active = inventory().ItemFromSlot(FLARE_SLOT);
			if(fl_active)
			{
				CFlare* fl			= smart_cast<CFlare*>(fl_active);
				fl->DropFlare		();
				return				;
			}

			PIItem fli = inventory().Get(CLSID_DEVICE_FLARE, true);
			if(!fli)			return;

			CFlare* fl			= smart_cast<CFlare*>(fli);
			
			if(inventory().Slot(fl))
				fl->ActivateFlare	();
		}break;
*/
	case kUSE:
		ActorUse();
		break;
	case kDROP:
		b_DropActivated			= TRUE;
		f_DropPower				= 0;
		break;
	case kNEXT_SLOT:
		{
			OnNextWeaponSlot();
		}break;
	case kPREV_SLOT:
		{
			OnPrevWeaponSlot();
		}break;

	case kLASER_ON:
		{
		auto wpn = smart_cast<CWeapon*>(inventory().ActiveItem());
			if (wpn)
				wpn->SwitchLaser(!wpn->IsLaserOn());
		}break;
	case kFLASHLIGHT:
		{
			auto wpn = smart_cast<CWeapon*>(inventory().ActiveItem());
			if (wpn)
				wpn->SwitchFlashlight(!wpn->IsFlashlightOn());
		}break;
	case kWPN_ALT_AIM:
		{
			auto wpn = smart_cast<CWeapon*>(inventory().ActiveItem());

			if (wpn && wpn->IsScopeAttached())
			{
				if (!wpn->IsAltAimEnabled())
					return;

				wpn->SwitchZoomMode();

				string256 alt_aim_status;
				strconcat(sizeof(alt_aim_status), alt_aim_status, "st_alt_aim_switched_", wpn->GetAltZoomStatus() ? "on" : "off");

				if (SDrawStaticStruct* custom_static = HUD().GetUI()->UIGame()->AddCustomStatic("alt_aim_switched", true))
					custom_static->wnd()->SetText(CStringTable().translate(alt_aim_status).c_str());
			}
		}break;
	}
}

void CActor::IR_OnMouseWheel(int direction)
{
	if(hud_adj_mode)
	{
		g_player_hud->tune	(Ivector().set(0,0,direction));
		return;
	}

	if(inventory().Action( (direction>0)? kWPN_ZOOM_DEC:kWPN_ZOOM_INC , CMD_START)) return;


	if (direction>0)
		OnNextWeaponSlot				();
	else
		OnPrevWeaponSlot				();
}

void CActor::IR_OnKeyboardRelease(int cmd)
{
	if (m_blocked_actions.find((EGameActions)cmd) != m_blocked_actions.end()) return; // Real Wolf. 14.10.2014

	if(hud_adj_mode && pInput->iGetAsyncKeyState(DIK_LSHIFT))	return;

	if (Remote())	return;

	if (m_input_external_handler && !m_input_external_handler->authorized(cmd))	return;

	if (g_Alive())	
	{
		if (cmd == kUSE) 
			PickupModeOff();

		if(m_holder)
		{
			m_holder->OnKeyboardRelease(cmd);
			
			if(m_holder->allowWeapon() && inventory().Action(cmd, CMD_STOP))		return;
			return;
		}else
			if(inventory().Action(cmd, CMD_STOP))		return;



		switch(cmd)
		{
		case kJUMP:		mstate_wishful &=~mcJump;		break;
		case kDROP:		if(GAME_PHASE_INPROGRESS == Game().Phase()) g_PerformDrop();				break;
		}
	}
}

void CActor::IR_OnKeyboardHold(int cmd)
{
	if (m_blocked_actions.find((EGameActions)cmd) != m_blocked_actions.end()) return; // Real Wolf. 14.10.2014

	if (hud_adj_mode && pInput->iGetAsyncKeyState(DIK_LSHIFT) && g_player_hud)
	{
		u8 idx = g_player_hud->attached_item(hud_adj_item_idx)->m_parent_hud_item->GetCurrentHudOffsetIdx();

		bool bIsRot = (hud_adj_mode == 2) && (idx != 0);

		if (pInput->iGetAsyncKeyState(bIsRot ? DIK_RIGHT : DIK_UP))
			g_player_hud->tune(Ivector().set(0, 1, 0));
		if (pInput->iGetAsyncKeyState(bIsRot ? DIK_LEFT : DIK_DOWN))
			g_player_hud->tune(Ivector().set(0, -1, 0));
		if (pInput->iGetAsyncKeyState(bIsRot ? DIK_DOWN : DIK_LEFT))
			g_player_hud->tune(Ivector().set(1, 0, 0));
		if (pInput->iGetAsyncKeyState(bIsRot ? DIK_UP : DIK_RIGHT))
			g_player_hud->tune(Ivector().set(-1, 0, 0));
		if (pInput->iGetAsyncKeyState(DIK_PRIOR))
			g_player_hud->tune(Ivector().set(0, 0, 1));
		if (pInput->iGetAsyncKeyState(DIK_NEXT))
			g_player_hud->tune(Ivector().set(0, 0, -1));
		if (pInput->iGetAsyncKeyState(DIK_RETURN))
			g_player_hud->tune(Ivector().set(0, 0, 0));
		return;
	}

	if (Remote() || !g_Alive() || g_block_actor_movement) return;
	if (m_input_external_handler && !m_input_external_handler->authorized(cmd))	return;
	if (IsTalking())							return;

	if(m_holder)
	{
		m_holder->OnKeyboardHold(cmd);
		return;
	}

#ifdef DEBUG
	if(psActorFlags.test(AF_NO_CLIP) && (cmd==kFWD || cmd==kBACK || cmd==kL_STRAFE || cmd==kR_STRAFE 
		|| cmd==kJUMP || cmd==kCROUCH))
	{
		NoClipFly(cmd);
		return;
	}
#endif //DEBUG
	float LookFactor = GetLookFactor();
	switch(cmd)
	{
	case kUP:
	case kDOWN: 
		cam_Active()->Move( (cmd==kUP) ? kDOWN : kUP, 0, LookFactor);									break;
	case kCAM_ZOOM_IN: 
	case kCAM_ZOOM_OUT: 
		cam_Active()->Move(cmd);												break;
	case kLEFT:
	case kRIGHT:
		if (eacFreeLook!=cam_active) cam_Active()->Move(cmd, 0, LookFactor);	break;

	case kACCEL:	mstate_wishful |= mcAccel;									break;
	case kL_STRAFE:	mstate_wishful |= mcLStrafe;								break;
	case kR_STRAFE:	mstate_wishful |= mcRStrafe;								break;
	case kL_LOOKOUT:
		if (eacLookAt != cam_active)
			mstate_wishful |= mcLLookout;
		else
			psActorFlags.set(AF_RIGHT_SHOULDER, true);
		break;
	case kR_LOOKOUT:
		if (eacLookAt != cam_active)
			mstate_wishful |= mcRLookout;
		else
			psActorFlags.set(AF_RIGHT_SHOULDER, false);
		break;
	case kFWD:		mstate_wishful |= mcFwd;									break;
	case kBACK:		mstate_wishful |= mcBack;									break;
	case kCROUCH:
		{
			if (!psActorFlags.test(AF_CROUCH_TOGGLE))
				mstate_wishful |= mcCrouch;
	
		}break;
	}

	if (!is_actor_climbing())
		LegsStaticCollisionRay(cmd);
}

void CActor::IR_OnMouseMove(int dx, int dy)
{
	if(hud_adj_mode)
	{
		g_player_hud->tune	(Ivector().set(dx,dy,0));
		return;
	}

	PIItem iitem = inventory().ActiveItem();
	if(iitem && iitem->cast_hud_item())
		iitem->cast_hud_item()->ResetSubStateTime();

	if (Remote())		return;

	if(m_holder) 
	{
		m_holder->OnMouseMove(dx,dy);
		return;
	}

	float LookFactor = GetLookFactor();

	CCameraBase* C	= cameras	[cam_active];

	auto wpn = smart_cast<CWeapon*>(inventory().ActiveItem());

	float scale		= (C->f_fov/g_fov)* ((wpn && wpn->IsZoomed() && wpn->bIsSecondVPZoomPresent()) ? psSVP_MouseSens : psMouseSens) * psMouseSensScale/50.f  / LookFactor;
	if (dx){
		float d = float(dx)*scale;
		cam_Active()->Move((d<0)?kLEFT:kRIGHT, _abs(d));
	}
	if (dy){
		float d = ((psMouseInvert.test(1))?-1:1)*float(dy)*scale*3.f/4.f;
		cam_Active()->Move((d>0)?kUP:kDOWN, _abs(d));
	}
}
#include "HudItem.h"
bool CActor::use_Holder				(CHolderCustom* holder)
{

	if(m_holder){
		bool b = false;
		CGameObject* holderGO			= smart_cast<CGameObject*>(m_holder);
		
		if(smart_cast<CCar*>(holderGO))
			b = use_Vehicle(0);
		else
			if (holderGO->CLS_ID==CLSID_OBJECT_W_STATMGUN)
				b = use_MountedWeapon(0);

		if(inventory().ActiveItem()){
			CHudItem* hi = smart_cast<CHudItem*>(inventory().ActiveItem());
			if(hi) hi->OnAnimationEnd(hi->GetState());
		}

		return b;
	}else{
		bool b = false;
		CGameObject* holderGO			= smart_cast<CGameObject*>(holder);
		if(smart_cast<CCar*>(holder))
			b = use_Vehicle(holder);

		if (holderGO->CLS_ID==CLSID_OBJECT_W_STATMGUN)
			b = use_MountedWeapon(holder);
		
		if(b){//used succesfully
			// switch off torch...
			CAttachableItem *I = CAttachmentOwner::attachedItem(CLSID_DEVICE_TORCH);
			if (I){
				CTorch* torch = smart_cast<CTorch*>(I);
				if (torch) torch->Switch(false);
			}
		}

		if(inventory().ActiveItem()){
			CHudItem* hi = smart_cast<CHudItem*>(inventory().ActiveItem());
			if(hi) hi->OnAnimationEnd(hi->GetState());
		}

		return b;
	}
}

void CActor::ActorUse()
{
	//mstate_real = 0;
	auto PdaUI = &HUD().GetUI()->UIGame()->PdaMenu();
	auto Pda = GetPDA();

	if (Pda && Pda->Is3DPDA() && psActorFlags.test(AF_3D_PDA) && PdaUI && PdaUI->IsShown())
		return;
		
	PickupModeOn();

		
	if (m_holder)
	{
		CGameObject*	GO			= smart_cast<CGameObject*>(m_holder);
		NET_Packet		P;
		CGameObject::u_EventGen		(P, GEG_PLAYER_DETACH_HOLDER, ID());
		P.w_u16						(GO->ID());
		CGameObject::u_EventSend	(P);
		return;
	}
				
	if(character_physics_support()->get_movement()->PHCapture())
		character_physics_support()->get_movement()->PHReleaseObject();

	

	if(m_pUsableObject)m_pUsableObject->use(this);
	
	if(m_pInvBoxWeLookingAt && m_pInvBoxWeLookingAt->nonscript_usable())
	{
		CUIGameSP* pGameSP = smart_cast<CUIGameSP*>(HUD().GetUI()->UIGame());
		if(pGameSP) pGameSP->StartCarBody(this, m_pInvBoxWeLookingAt );
		return;
	}

	if(!m_pUsableObject||m_pUsableObject->nonscript_usable())
	{
		if(m_pPersonWeLookingAt)
		{
			CEntityAlive* pEntityAliveWeLookingAt = 
				smart_cast<CEntityAlive*>(m_pPersonWeLookingAt);

			VERIFY(pEntityAliveWeLookingAt);

			if (IsGameTypeSingle())
			{			

				if(pEntityAliveWeLookingAt->g_Alive())
				{
					TryToTalk();
				}else

				//обыск трупа
				if(!Level().IR_GetKeyState(DIK_LSHIFT))
				{
					//только если находимся в режиме single
					CUIGameSP* pGameSP = smart_cast<CUIGameSP*>(HUD().GetUI()->UIGame());
					if(pGameSP)
						pGameSP->StartCarBody(this, m_pPersonWeLookingAt );
				}
			}
		}

		collide::rq_result& RQ = HUD().GetCurrentRayQuery();
		CPhysicsShellHolder* object = smart_cast<CPhysicsShellHolder*>(RQ.O);
		u16 element = BI_NONE;
		if(object) 
			element = (u16)RQ.element;

		if(object && Level().IR_GetKeyState(DIK_LSHIFT))
		{
			bool b_allow = !!pSettings->line_exist("ph_capture_visuals",object->cNameVisual());
			if(b_allow && !character_physics_support()->get_movement()->PHCapture())
			{
				character_physics_support()->get_movement()->PHCaptureObject( object, element );

			}

		}
		else
		{
			if (object && smart_cast<CHolderCustom*>(object))
			{
					NET_Packet		P;
					CGameObject::u_EventGen		(P, GEG_PLAYER_ATTACH_HOLDER, ID());
					P.w_u16						(object->ID());
					CGameObject::u_EventSend	(P);
					return;
			}

		}
	}
}

BOOL CActor::HUDview				( )const 
{ 
	return IsFocused() && (cam_active==eacFirstEye)&&
		((!m_holder) || (m_holder && m_holder->allowWeapon() && m_holder->HUDView() ) ); 
}

static	u32 SlotsToCheck [] = {
		KNIFE_SLOT		,		// 0
		PISTOL_SLOT		,		// 1
		RIFLE_SLOT		,		// 2
		GRENADE_SLOT	,		// 3
		APPARATUS_SLOT	,		// 4
		ARTEFACT_SLOT	,		// 10
		PDA_SLOT		,
		BACKPACK_SLOT	,
};

void	CActor::OnNextWeaponSlot()
{
	u32 ActiveSlot = inventory().GetActiveSlot();
	if (ActiveSlot == NO_ACTIVE_SLOT) 
		ActiveSlot = inventory().GetPrevActiveSlot();

	if (ActiveSlot == NO_ACTIVE_SLOT) 
		ActiveSlot = KNIFE_SLOT;
	
	u32 NumSlotsToCheck = sizeof(SlotsToCheck)/sizeof(u32);	
	u32 CurSlot = 0;
	for (;CurSlot<NumSlotsToCheck; CurSlot++)
	{
		if (SlotsToCheck[CurSlot] == ActiveSlot) break;
	};

	if (CurSlot >= NumSlotsToCheck)
		return;

	for (u32 i=CurSlot+1; i<NumSlotsToCheck; i++)
	{
		if (inventory().ItemFromSlot(SlotsToCheck[i]))
		{
			if (SlotsToCheck[i] == ARTEFACT_SLOT) 
			{
				IR_OnKeyboardPress(kARTEFACT);
			}
			else if (SlotsToCheck[i] == PDA_SLOT)
			{
				IR_OnKeyboardPress(kACTIVE_JOBS);
			}
			else if (SlotsToCheck[i] == BACKPACK_SLOT)
			{
				IR_OnKeyboardPress(kINVENTORY);
			}
			else
				IR_OnKeyboardPress(kWPN_1+(i-KNIFE_SLOT));
			return;
		}
	}
};

void	CActor::OnPrevWeaponSlot()
{
	u32 ActiveSlot = inventory().GetActiveSlot();
	if (ActiveSlot == NO_ACTIVE_SLOT) 
		ActiveSlot = inventory().GetPrevActiveSlot();

	if (ActiveSlot == NO_ACTIVE_SLOT) 
		ActiveSlot = KNIFE_SLOT;

	u32 NumSlotsToCheck = sizeof(SlotsToCheck)/sizeof(u32);
	u32 CurSlot			= 0;

	for (;CurSlot<NumSlotsToCheck; CurSlot++)
	{
		if (SlotsToCheck[CurSlot] == ActiveSlot) break;
	};
	if (CurSlot >= NumSlotsToCheck)
		return;
	for (s32 i=s32(CurSlot-1); i>=0; i--)
	{
		if (inventory().ItemFromSlot(SlotsToCheck[i]))
		{
			if (SlotsToCheck[i] == ARTEFACT_SLOT) 
			{
				IR_OnKeyboardPress(kARTEFACT);
			}
			else
			{
				if (!IsDetectorActive())
					IR_OnKeyboardPress(kWPN_1 + (i - KNIFE_SLOT));
				else
					IR_OnKeyboardPress(kWPN_2);
			}
			return;
		}
	}
};

float	CActor::GetLookFactor()
{
	if (m_input_external_handler) 
		return m_input_external_handler->mouse_scale_factor();

	
	float factor	= 1.f;

	PIItem pItem	= inventory().ActiveItem();

	if (pItem)
		factor *= pItem->GetControlInertionFactor();

	VERIFY(!fis_zero(factor));

	return factor;
}

void CActor::set_input_external_handler(CActorInputHandler *handler) 
{
	// clear state
	if (handler) 
		mstate_wishful			= 0;

	// release fire button
	if (handler)
		IR_OnKeyboardRelease	(kWPN_FIRE);

	// set handler
	m_input_external_handler	= handler;
}

void CActor::SwitchNightVision()
{
	if (!Actor()->m_bActionAnimInProcess)
		NVGAnimCheckDetector();
}

void CActor::SwitchTorch()
{ 
	CTorch* pTorch = smart_cast<CTorch*>(inventory().ItemFromSlot(TORCH_SLOT));

	if (pTorch && !Actor()->m_bActionAnimInProcess)
		pTorch->Switch();
}

#ifdef DEBUG
void CActor::NoClipFly(int cmd)
{
	Fvector cur_pos;// = Position();
	cur_pos.set(0,0,0);
	float scale = 1.0f;
	if(pInput->iGetAsyncKeyState(DIK_LSHIFT))
		scale = 0.25f;
	else if(pInput->iGetAsyncKeyState(DIK_LMENU))
		scale = 4.0f;

	switch(cmd)
	{
	case kJUMP:		
		cur_pos.y += 0.1f;
		break;
	case kCROUCH:	
		cur_pos.y -= 0.1f;
		break;
	case kFWD:	
		cur_pos.z += 0.1f;
		break;
	case kBACK:
		cur_pos.z -= 0.1f;
		break;
	case kL_STRAFE:
		cur_pos.x -= 0.1f;
		break;
	case kR_STRAFE:
		cur_pos.x += 0.1f;
		break;
	case kCAM_1:	
		cam_Set(eacFirstEye);				
		break;
	case kCAM_2:	
		cam_Set(eacLookAt);				
		break;
	case kCAM_3:	
		cam_Set(eacFreeLook);
		break;
	case kNIGHT_VISION:
		SwitchNightVision();
		break;
	case kTORCH:
		SwitchTorch();
		break;
	case kDETECTOR:
		{
			PIItem det_active = inventory().ItemFromSlot(DETECTOR_SLOT);
			if(det_active)
			{
				CCustomDetector* det = smart_cast<CCustomDetector*>(det_active);
				det->ToggleDetector(g_player_hud->attached_item(0)!=NULL);
				return;
			}
		}
		break;
	case kUSE:
		ActorUse();
		break;
	}
	cur_pos.mul(scale);
	Fmatrix	mOrient;
	mOrient.rotateY(-(cam_Active()->GetWorldYaw()));
	mOrient.transform_dir(cur_pos);
	Position().add(cur_pos);
	character_physics_support()->get_movement()->SetPosition(Position());
}
#endif //DEBUG