#include "pch_script.h"
#include "uigamesp.h"
#include "actor.h"
#include "level.h"
#include "../xrEngine/xr_input.h"

#ifdef DEBUG
#include "attachable_item.h"
#endif

#include "game_cl_Single.h"
#include "xr_level_controller.h"
#include "actorcondition.h"
#include "../xrEngine/xr_ioconsole.h"
#include "object_broker.h"
#include "GameTaskManager.h"
#include "GameTask.h"

#include "ui/UIActorMenu.h"
#include "ui/UITradeWnd.h"
#include "ui/UIPdaWnd.h"
#include "ui/UITalkWnd.h"
#include "ui/UIMessageBox.h"
#include "UIDialogHolder.h"

#include "AdvancedXrayGameConstants.h"
#include "Inventory.h"
#include "alife_registry_wrappers.h"
#include "../xrServerEntitiesCS/script_engine.h"
#include "HUDManager.h"
#include "PDA.h"
#include "CustomBackpack.h"

#include <imgui.h>

CUIGameSP::CUIGameSP()
:m_game(NULL)
{
	TalkMenu		= xr_new<CUITalkWnd>		();
	UIChangeLevelWnd= xr_new<CChangeLevelWnd>	();
}

CUIGameSP::~CUIGameSP() 
{
	delete_data(TalkMenu);
	delete_data(UIChangeLevelWnd);
}

void CUIGameSP::HideShownDialogs()
{
	HideActorMenu();
	HidePdaMenu();

	CUIDialogWnd* mir = MainInputReceiver();
	if ( mir && mir == TalkMenu )
	{
		mir->GetHolder()->StartStopMenu( mir, true );
	}
}

void CUIGameSP::SetClGame (game_cl_GameState* g)
{
	inherited::SetClGame				(g);
	m_game = smart_cast<game_cl_Single*>(g);
	R_ASSERT							(m_game);
}

void attach_adjust_mode_keyb(int dik);
void attach_draw_adjust_mode();
void hud_adjust_mode_keyb(int dik);
void hud_draw_adjust_mode();


bool CUIGameSP::IR_OnKeyboardPress(int dik) 
{
	if(inherited::IR_OnKeyboardPress(dik)) return true;

	if( Device.Paused()		) return false;

	if (Actor()->active_cam() == eacFirstEye)
	{
		hud_adjust_mode_keyb(dik);
	}
	if (Actor()->active_cam() == eacFreeLook)
	{
		attach_adjust_mode_keyb(dik);
	}

	CInventoryOwner* pInvOwner = smart_cast<CInventoryOwner*>( Level().CurrentEntity() );
	if ( !pInvOwner )				return false;
	CEntityAlive* EA			= smart_cast<CEntityAlive*>(Level().CurrentEntity());
	if (!EA || !EA->g_Alive() )	return false;

	CActor *pActor = smart_cast<CActor*>(pInvOwner);
	if( !pActor ) 
		return false;

	if (pActor && !pActor->g_Alive())
		return false;

	auto Pda = pActor->GetPDA();

	switch ( get_binded_action(dik) )
	{
	case kACTIVE_JOBS:
		{
			if (!psActorFlags.test(AF_3D_PDA) || (psActorFlags.test(AF_3D_PDA) && Pda && !Pda->Is3DPDA()))
			{
				luabind::functor<bool> funct;
				if (ai().script_engine().functor("pda.pda_use", funct))
				{
					if (funct())
						ShowPdaMenu();
				}
				else
					ShowPdaMenu();	// cari0us -- для совместимости с оригинальной игрой
			}
			break;
		}
	case kINVENTORY:
		{
			if (Pda && Pda->Is3DPDA() && psActorFlags.test(AF_3D_PDA) && HUD().GetUI()->UIGame()->PdaMenu().IsShown())
				pActor->inventory().Activate(NO_ACTIVE_SLOT);

			CCustomBackpack* backpack = smart_cast<CCustomBackpack*>(pActor->inventory().ItemFromSlot(BACKPACK_SLOT));

			if (!GameConstants::GetBackpackAnimsEnabled() || !backpack)
				ShowActorMenu();

		break;
		}

	case kSCORES:
		{
			CActor* pActor = smart_cast<CActor*>(pInvOwner);
			if (!pActor) return false;
			if (!pActor->g_Alive())	return false;

			SDrawStaticStruct* sm = AddCustomStatic("main_task", true);
			CGameTask* t1 = Level().GameTaskManager().ActiveTask(eTaskTypeStoryline);
			CGameTask* t2 = Level().GameTaskManager().ActiveTask(eTaskTypeAdditional);

			sm->m_static->SetTextST((t1) ? t1->m_Title.c_str() : "st_no_active_task");

			if (t1 && t2)
			{
				SDrawStaticStruct* sm2 = AddCustomStatic("secondary_task", true);
				sm2->m_static->SetTextST((t2) ? t2->m_Title.c_str() : "");

				if (t1 && t1->m_difficulty_icon_name.c_str())
					sm->m_static->InitTexture(t1->m_difficulty_icon_name.c_str());
			} 
			else if (t1 || t2)
			{
				CGameTask* t = (t1) ? t1 : t2;
				sm->m_static->SetTextST((t) ? t->m_Title.c_str() : "st_no_active_task");

				if (t && t->m_difficulty_icon_name.c_str())
					sm->m_static->InitTexture(t->m_difficulty_icon_name.c_str());
			}
			else
				sm->m_static->SetTextST("st_no_active_task");
			
			break;
		}
	}
	return false;
}

void CUIGameSP::Render()
{
	inherited::Render();
	hud_draw_adjust_mode();
	attach_draw_adjust_mode();
}

bool CUIGameSP::IR_OnKeyboardRelease(int dik) 
{
	if(inherited::IR_OnKeyboardRelease(dik)) return true;

	if( is_binded(kSCORES, dik))
	{
			RemoveCustomStatic		("main_task");
			RemoveCustomStatic		("secondary_task");
	}

	return false;
}

void  CUIGameSP::StartTrade(CInventoryOwner* pActorInv, CInventoryOwner* pOtherOwner)
{
	//if( MainInputReceiver() )	return;

	m_ActorMenu->SetActor		(pActorInv);
	m_ActorMenu->SetPartner		(pOtherOwner);

	m_ActorMenu->SetMenuMode	(mmTrade);
	m_game->StartStopMenu		(m_ActorMenu,true);
}

void  CUIGameSP::StartUpgrade(CInventoryOwner* pActorInv, CInventoryOwner* pMech)
{
	//if( MainInputReceiver() )	return;

	m_ActorMenu->SetActor		(pActorInv);
	m_ActorMenu->SetPartner		(pMech);

	m_ActorMenu->SetMenuMode	(mmUpgrade);
	m_game->StartStopMenu		(m_ActorMenu,true);
}

void CUIGameSP::StartTalk(bool disable_break)
{
	RemoveCustomStatic		("main_task");
	RemoveCustomStatic		("secondary_task");

	TalkMenu->b_disable_break = disable_break;
	m_game->StartStopMenu(TalkMenu, true);
}


void CUIGameSP::StartCarBody(CInventoryOwner* pActorInv, CInventoryOwner* pOtherOwner) //Deadbody search
{
	if( MainInputReceiver() )		return;

	m_ActorMenu->SetActor		(pActorInv);
	m_ActorMenu->SetPartner		(pOtherOwner);

	m_ActorMenu->SetMenuMode	(mmDeadBodySearch);
	m_game->StartStopMenu(m_ActorMenu, true);
}

void CUIGameSP::StartCarBody(CInventoryOwner* pActorInv, CInventoryBox* pBox) //Deadbody search
{
	if( MainInputReceiver() )		return;
	
	m_ActorMenu->SetActor		(pActorInv);
	m_ActorMenu->SetInvBox		(pBox);
	VERIFY( pBox );

	m_ActorMenu->SetMenuMode	(mmDeadBodySearch);
	m_game->StartStopMenu(m_ActorMenu, true);
}

void CUIGameSP::StartCarBody(CInventoryOwner* pActorInv, CCar* pCar) //Car trunk search
{
	if (MainInputReceiver()) return;

	m_ActorMenu->SetActor(pActorInv);
	m_ActorMenu->SetCarTrunk(pCar);
	VERIFY(pCar);

	m_ActorMenu->SetMenuMode(mmDeadBodySearch);
	m_game->StartStopMenu(m_ActorMenu, true);
}

extern ENGINE_API BOOL bShowPauseString;
void CUIGameSP::ChangeLevel(	GameGraph::_GRAPH_ID game_vert_id, 
								u32 level_vert_id, 
								Fvector pos, 
								Fvector ang, 
								Fvector pos2, 
								Fvector ang2, 
								bool b_use_position_cancel,
								const shared_str& message_str,
								bool b_allow_change_level)
{
	if( !MainInputReceiver() || MainInputReceiver()!=UIChangeLevelWnd)
	{
		UIChangeLevelWnd->m_game_vertex_id		= game_vert_id;
		UIChangeLevelWnd->m_level_vertex_id		= level_vert_id;
		UIChangeLevelWnd->m_position			= pos;
		UIChangeLevelWnd->m_angles				= ang;
		UIChangeLevelWnd->m_position_cancel		= pos2;
		UIChangeLevelWnd->m_angles_cancel		= ang2;
		UIChangeLevelWnd->m_b_position_cancel	= b_use_position_cancel;
		UIChangeLevelWnd->m_b_allow_change_level=b_allow_change_level;
		UIChangeLevelWnd->m_message_str			= message_str;

		m_game->StartStopMenu					(UIChangeLevelWnd,true);
	}
}

void CUIGameSP::reset_ui()
{
	inherited::reset_ui				();
	TalkMenu->Reset					();
	UIChangeLevelWnd->Reset			();
}

bool CUIGameSP::FillDebugTree(const CUIDebugState& debugState)
{
	if (!CUIGameCustom::FillDebugTree(debugState))
		return false;

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;

	if (debugState.selected == this)
		flags |= ImGuiTreeNodeFlags_Selected;

	const bool open = ImGui::TreeNodeEx(this, flags, "Game UI (%s)", CUIGameSP::GetDebugType());

	if (ImGui::IsItemClicked())
		debugState.select(this);

	if (open)
	{
		TalkMenu->FillDebugTree(debugState);
		UIChangeLevelWnd->FillDebugTree(debugState);

		//if (m_game_objective)
		//	m_game_objective->wnd()->FillDebugTree(debugState);

		ImGui::TreePop();
	}

	return open;
}

void CUIGameSP::FillDebugInfo()
{
#ifndef MASTER_GOLD
	CUIGameCustom::FillDebugInfo();
	if (ImGui::CollapsingHeader(CUIGameSP::GetDebugType()))
	{
	}
#endif
}

void CUIGameSP::ReinitDialogs()
{
	delete_data(Actor_Menu);
	Actor_Menu = xr_new<CUIActorMenu>();
	delete_data(TalkMenu);
	TalkMenu = xr_new<CUITalkWnd>();
}

CChangeLevelWnd::CChangeLevelWnd		()
{
	m_messageBox			= xr_new<CUIMessageBox>();	
	m_messageBox->SetAutoDelete(true);
	AttachChild				(m_messageBox);
}

void CChangeLevelWnd::SendMessage(CUIWindow *pWnd, s16 msg, void *pData)
{
	if(pWnd==m_messageBox){
		if(msg==MESSAGE_BOX_YES_CLICKED){
			OnOk									();
		}else
		if(msg==MESSAGE_BOX_NO_CLICKED || msg==MESSAGE_BOX_OK_CLICKED)
		{
			OnCancel								();
		}
	}else
		inherited::SendMessage(pWnd, msg, pData);
}

void CChangeLevelWnd::OnOk()
{
	Game().StartStopMenu					(this, true);
	NET_Packet								p;
	p.w_begin								(M_CHANGE_LEVEL);
	p.w										(&m_game_vertex_id,sizeof(m_game_vertex_id));
	p.w										(&m_level_vertex_id,sizeof(m_level_vertex_id));
	p.w_vec3								(m_position);
	p.w_vec3								(m_angles);

	Level().Send							(p,net_flags(TRUE));
}

void CChangeLevelWnd::OnCancel()
{
	Game().StartStopMenu					(this, true);
	if(m_b_position_cancel){
		Actor()->MoveActor(m_position_cancel, m_angles_cancel);
	}
}

bool CChangeLevelWnd::OnKeyboardAction(int dik, EUIMessages keyboard_action)
{
	if(keyboard_action==WINDOW_KEY_PRESSED)
	{
		if(is_binded(kQUIT, dik) )
			OnCancel		();
		return true;
	}
	return inherited::OnKeyboardAction(dik, keyboard_action);
}

bool g_block_pause	= false;
void CChangeLevelWnd::Show()
{
	m_messageBox->InitMessageBox(m_b_allow_change_level?"message_box_change_level":"message_box_change_level_disabled");
	SetWndPos				(m_messageBox->GetWndPos());
	m_messageBox->SetWndPos	(Fvector2().set(0.0f,0.0f));
	SetWndSize				(m_messageBox->GetWndSize());

	m_messageBox->SetText	(m_message_str.c_str());
	

	g_block_pause							= true;
	GAME_PAUSE								(TRUE, TRUE, TRUE, "CChangeLevelWnd_show");
	bShowPauseString						= FALSE;
}

void CChangeLevelWnd::Hide()
{
	g_block_pause							= false;
	GAME_PAUSE								(FALSE, TRUE, TRUE, "CChangeLevelWnd_hide");
}

void CChangeLevelWnd::FillDebugInfo()
{
#ifndef MASTER_GOLD
	CUIDialogWnd::FillDebugInfo();

	if (ImGui::CollapsingHeader(CChangeLevelWnd::GetDebugType()))
	{
		ImGui::Checkbox("Level change allowed", &m_b_allow_change_level);
		ImGui::DragScalar("Game vertex ID", ImGuiDataType_U16, &m_game_vertex_id);
		ImGui::DragScalar("Level vertex ID", ImGuiDataType_U32, &m_level_vertex_id);
		ImGui::DragFloat3("Position", (float*)&m_position);
		ImGui::DragFloat3("Angles", (float*)&m_angles);
		ImGui::Separator();
		ImGui::Checkbox("Teleport Actor on cancel", &m_b_position_cancel);
		ImGui::DragFloat3("Position on cancel", (float*)&m_position_cancel);
		ImGui::DragFloat3("Angles on cancel", (float*)&m_angles_cancel);
	}
#endif
}