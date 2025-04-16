#include "stdafx.h"
#include "UIDialogHolder.h"
#include "ui\UIPdaWnd.h"
#include "ui\UIBtnHint.h"
#include "UICursor.h"
#include "level.h"
#include "actor.h"
#include "xr_level_controller.h"
#include "PDA.h"
#include "Inventory.h"
#include "UIGameSP.h"
#include <imgui.h>

dlgItem::dlgItem(CUIWindow* pWnd)
{
	wnd		= pWnd;
	enabled = true;
}

bool dlgItem::operator < (const dlgItem& itm) const
{
	return (int)enabled > (int)itm.enabled;
}

bool operator == (const dlgItem& i1, const dlgItem& i2)
{
	return i1.wnd == i2.wnd;
}

recvItem::recvItem(CUIDialogWnd* r)
{
	m_item			= r;
	m_flags.zero	();
}
bool operator == (const recvItem& i1, const recvItem& i2)
{
	return i1.m_item == i2.m_item;
}

CDialogHolder::CDialogHolder()
{
	Device.seqFrame.Add		(this,REG_PRIORITY_LOW-1000);
	RegisterDebuggable();
}

CDialogHolder::~CDialogHolder()
{
	Device.seqFrame.Remove		(this);
	UnregisterDebuggable();
}
#include "HUDManager.h"

void CDialogHolder::StartMenu (CUIDialogWnd* pDialog, bool bDoHideIndicators)
{
	R_ASSERT						( !pDialog->IsShown() );

	if (psActorFlags.test(AF_3D_PDA) && IsGameTypeSingle() && !smart_cast<CUIPdaWnd*>(pDialog) && Actor())
	{
		const auto pda = smart_cast<CPda*>(Actor()->inventory().ActiveItem());
		if (pda && pda->Is3DPDA())
		{
			CUIGameSP* pGameSP = smart_cast<CUIGameSP*>(HUD().GetUI()->UIGame());
			pGameSP->PdaMenu->HideDialog1();
			Actor()->inventory().Action(kACTIVE_JOBS, CMD_START);
		}
	}

	AddDialogToRender				(pDialog);
	SetMainInputReceiver			(pDialog, false);

	if(UseIndicators())
	{
		bool b							= !!psHUD_Flags.test(HUD_CROSSHAIR_RT);
		m_input_receivers.back().m_flags.set(recvItem::eCrosshair, b);

		b								= HUD().GetUI()->GameIndicatorsShown();
		m_input_receivers.back().m_flags.set(recvItem::eIndicators, b);
		
		if(bDoHideIndicators){
			psHUD_Flags.set				(HUD_CROSSHAIR_RT, FALSE);
			HUD().GetUI					()->ShowGameIndicators(false);
		}
	}
	pDialog->SetHolder				(this);
	pDialog->Show					();

	if( pDialog->NeedCursor() )
		GetUICursor().Show();

	if(g_pGameLevel)
	{
		CActor* A	= smart_cast<CActor*>( Level().CurrentViewEntity() );
		if ( A && pDialog->StopAnyMove() )
		{
			A->StopAnyMove				();
			A->PickupModeOff			();
		};
		if(A)
		{	
			A->IR_OnKeyboardRelease		(kWPN_ZOOM);
			A->IR_OnKeyboardRelease		(kWPN_FIRE);
		}
	}
}


void CDialogHolder::StopMenu (CUIDialogWnd* pDialog)
{
	R_ASSERT( pDialog->IsShown() );

	if( MainInputReceiver()==pDialog )
	{
		if(UseIndicators())
		{
			bool b					= !!m_input_receivers.back().m_flags.test(recvItem::eCrosshair);
			psHUD_Flags.set			(HUD_CROSSHAIR_RT, b);
			b						= !!m_input_receivers.back().m_flags.test(recvItem::eIndicators);
			HUD().GetUI()->ShowGameIndicators(b);
		}
		RemoveDialogToRender	(pDialog);
		SetMainInputReceiver	(NULL,false);
		pDialog->SetHolder		(NULL);
		pDialog->Hide			();
	}else{
		RemoveDialogToRender	(pDialog);
		SetMainInputReceiver	(pDialog, true);
		pDialog->SetHolder		(NULL);
		pDialog->Hide			();
	}

	if(!MainInputReceiver() || !MainInputReceiver()->NeedCursor() )
		GetUICursor().Hide();
}

void CDialogHolder::AddDialogToRender(CUIWindow* pDialog)
{
	dlgItem itm(pDialog);
	xr_vector<dlgItem>::iterator it = std::find(m_dialogsToRender.begin(),m_dialogsToRender.end(),itm);
	if( (it == m_dialogsToRender.end()) || ( it != m_dialogsToRender.end() && (*it).enabled==false)  )
	{
		m_dialogsToRender.push_back(itm);
		pDialog->Show(true);
	}
}

void CDialogHolder::RemoveDialogToRender(CUIWindow* pDialog)
{
	dlgItem itm(pDialog);
	xr_vector<dlgItem>::iterator it = std::find(m_dialogsToRender.begin(),m_dialogsToRender.end(),itm);
	if(it != m_dialogsToRender.end())
	{
		(*it).wnd->Show(false);
		(*it).wnd->Enable(false);
		(*it).enabled = false;
	}
}



void CDialogHolder::DoRenderDialogs()
{
	ZoneScoped;

	xr_vector<dlgItem>::iterator it = m_dialogsToRender.begin();
	for(; it!=m_dialogsToRender.end();++it){
		if( (*it).enabled && (*it).wnd->IsShown() )
			(*it).wnd->Draw();
	}
}

CUIDialogWnd* CDialogHolder::MainInputReceiver()
{ 
	if ( !m_input_receivers.empty() ) 
		return m_input_receivers.back().m_item; 
	return NULL; 
};

void CDialogHolder::SetMainInputReceiver	(CUIDialogWnd* ir, bool _find_remove)	
{ 
	if( MainInputReceiver() == ir ) return;

	if(!ir || _find_remove){
		if(m_input_receivers.empty())	return;

		if(!ir)
			m_input_receivers.pop_back();
		else{
			VERIFY(ir && _find_remove);

			u32 cnt = m_input_receivers.size();
			for(;cnt>0;--cnt)
				if( m_input_receivers[cnt-1].m_item == ir ){
					m_input_receivers[cnt].m_flags.set(recvItem::eCrosshair, m_input_receivers[cnt-1].m_flags.test(recvItem::eCrosshair) );
					m_input_receivers[cnt].m_flags.set(recvItem::eIndicators, m_input_receivers[cnt-1].m_flags.test(recvItem::eIndicators) );
					xr_vector<recvItem>::iterator it = m_input_receivers.begin();
					std::advance			(it,cnt-1);
					m_input_receivers.erase	(it);
					break;
				}
			
		}

	}else{
		m_input_receivers.push_back(recvItem(ir));
	}
};
//. #include "ai_space.h"
//. #include "script_engine.h"
void CDialogHolder::StartStopMenu(CUIDialogWnd* pDialog, bool bDoHideIndicators)
{
//.	ai().script_engine().script_log	(eLuaMessageTypeError,"foo");
	if( pDialog->IsShown() )
		StopMenu(pDialog);
	else
	{
		if (g_statHint->IsShown())
			g_statHint->Discard();

		StartMenu(pDialog, bDoHideIndicators);
	}
	
}

void CDialogHolder::StartDialog(CUIDialogWnd* pDialog, bool bDoHideIndicators)
{
	// cari0us - чтобы отрубить центрирование курсора в принципе, как например нужно мне
	const bool b_disable_center_cursor_global = READ_IF_EXISTS(pAdvancedSettings, r_bool, "ui_settings", "disable_center_cursor_global", false);

	if (pDialog && (pDialog->NeedCenterCursor() && !b_disable_center_cursor_global))
	{
		GetUICursor().SetUICursorPosition(Fvector2().set(512.0f, 384.0f));
	}
	StartMenu(pDialog, bDoHideIndicators);
}

void CDialogHolder::StopDialog(CUIDialogWnd* pDialog)
{
	StopMenu(pDialog);
}

void CDialogHolder::OnFrame	()
{
	ZoneScoped;

	xr_vector<dlgItem>::iterator it = m_dialogsToRender.begin();
	for(; it!=m_dialogsToRender.end();++it)
		if((*it).enabled && (*it).wnd->IsEnabled())
			(*it).wnd->Update();

	if (m_dialogsToRender.empty())
		return;

	std::sort(m_dialogsToRender.begin(), m_dialogsToRender.end());

	while ((m_dialogsToRender.size()) && (!m_dialogsToRender[m_dialogsToRender.size() - 1].enabled))
		m_dialogsToRender.pop_back();
}

void CDialogHolder::CleanInternals()
{
	while( !m_input_receivers.empty() )
		m_input_receivers.pop_back();

	m_dialogsToRender.clear	();
	GetUICursor().Hide		();
}

bool CDialogHolder::FillDebugTree(const CUIDebugState& debugState)
{
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
	if (m_input_receivers.empty())
		ImGui::BulletText("Input receivers: 0");
	else
	{
		if (ImGui::TreeNode(&m_input_receivers, "Input receivers: %zu", m_input_receivers.size()))
		{
			for (const auto& item : m_input_receivers)
				item.m_item->FillDebugTree(debugState);
			ImGui::TreePop();
		}
	}
	if (m_dialogsToRender.empty())
		ImGui::BulletText("Dialogs to render: 0");
	else
	{
		if (ImGui::TreeNode(&m_dialogsToRender, "Dialogs to render: %zu", m_dialogsToRender.size()))
		{
			for (const auto& item : m_dialogsToRender)
				item.wnd->FillDebugTree(debugState);
			ImGui::TreePop();
		}
	}
	return true;
}

void CDialogHolder::FillDebugInfo()
{
#ifndef MASTER_GOLD
	if (ImGui::CollapsingHeader(CDialogHolder::GetDebugType()))
	{
	}
#endif
}