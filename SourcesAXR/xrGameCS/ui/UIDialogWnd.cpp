// UIDialogWnd.cpp: класс простого диалога, нужен для стандартного запуска
// разным менюшек путем вызова виртуальных Show() И Hide()
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "uidialogwnd.h"
#include "UIBtnHint.h"
#include "../hudmanager.h"
#include "../xr_level_controller.h"
#include "../../xrEngine/xr_ioconsole.h"
#include "../level.h"
#include "../GameObject.h"

#include <imgui.h>

CUIDialogWnd:: CUIDialogWnd()
{
	m_pHolder		= NULL;
	m_bWorkInPause	= false;
	Hide			();
}

CUIDialogWnd::~ CUIDialogWnd()
{
}

void CUIDialogWnd::Show()
{
	inherited::Enable(true);
	inherited::Show(true);

	ResetAll();
}


void CUIDialogWnd::Hide()
{

	inherited::Enable(false);
	inherited::Show(false);
	
}

bool CUIDialogWnd::IR_OnKeyboardHold(int dik)
{
	if(!IR_process()) return false;
	if (OnKeyboardHold(dik)) 
		return true;

	if( !StopAnyMove() && g_pGameLevel ){
		CObject* O = Level().CurrentEntity();
		if( O ){
			IInputReceiver*		IR	= smart_cast<IInputReceiver*>( smart_cast<CGameObject*>(O) );
			if (!IR)
				return			(false);
			IR->IR_OnKeyboardHold(get_binded_action(dik));
		}
	}
	return false;
}

bool CUIDialogWnd::IR_OnKeyboardPress(int dik)
{
	if(!IR_process()) return false;
	//mouse click
	if(dik==MOUSE_1 || dik==MOUSE_2 || dik==MOUSE_3)
	{
		Fvector2 cp = GetUICursor().GetCursorPosition();
		EUIMessages action = (dik==MOUSE_1)?WINDOW_LBUTTON_DOWN :(dik==MOUSE_2)?WINDOW_RBUTTON_DOWN:WINDOW_CBUTTON_DOWN;
		if (OnMouseAction(cp.x,cp.y, action))
            return true;
	}

	if (OnKeyboardAction(dik,	WINDOW_KEY_PRESSED))
		return true;

	if( !StopAnyMove() && g_pGameLevel ){
		CObject* O = Level().CurrentEntity();
		if( O ){
			IInputReceiver*		IR	= smart_cast<IInputReceiver*>( smart_cast<CGameObject*>(O) );
			if (!IR)
				return			(false);
			IR->IR_OnKeyboardPress(get_binded_action(dik));
		}
	}
	return false;
}

bool CUIDialogWnd::IR_OnKeyboardRelease(int dik)
{
	if(!IR_process()) return false;
	
	//mouse click
	if(dik==MOUSE_1 || dik==MOUSE_2 || dik==MOUSE_3)
	{
		Fvector2 cp = GetUICursor().GetCursorPosition();
		EUIMessages action = (dik==MOUSE_1)?WINDOW_LBUTTON_UP :(dik==MOUSE_2)?WINDOW_RBUTTON_UP:WINDOW_CBUTTON_UP;
		if (OnMouseAction(cp.x, cp.y, action))
            return true;
	}

	if (OnKeyboardAction(dik,	WINDOW_KEY_RELEASED))
		return true;

	if( !StopAnyMove() && g_pGameLevel ){
		CObject* O = Level().CurrentEntity();
		if( O ){
			IInputReceiver*		IR	= smart_cast<IInputReceiver*>( smart_cast<CGameObject*>(O) );
			if (!IR)
				return			(false);
			IR->IR_OnKeyboardRelease(get_binded_action(dik));
		}
	}
	return false;
}

bool CUIDialogWnd::IR_OnMouseWheel (int direction)
{
	if(!IR_process()) return false;
	Fvector2 pos = GetUICursor().GetCursorPosition();

	if(direction>0)
		OnMouseAction(pos.x,pos.y,WINDOW_MOUSE_WHEEL_UP);
	else
		OnMouseAction(pos.x,pos.y,WINDOW_MOUSE_WHEEL_DOWN);

	return true;
}

bool CUIDialogWnd::IR_OnMouseMove(int dx, int dy)
{
	if(!IR_process()) return false;
	
	if (GetUICursor().IsVisible())
	{ 
//		GetUICursor().MoveDelta(float(dx), float(dy));
		GetUICursor().UpdateCursorPosition(dx, dy);
		Fvector2 cPos = GetUICursor().GetCursorPosition();
		OnMouseAction(cPos.x, cPos.y , WINDOW_MOUSE_MOVE);
	}
	else if( !StopAnyMove() && g_pGameLevel ){
		CObject* O = Level().CurrentEntity();
		if( O ){
			IInputReceiver*		IR	= smart_cast<IInputReceiver*>( smart_cast<CGameObject*>(O) );
			if (!IR)
				return			(false);

			IR->IR_OnMouseMove(dx,dy);
		}
	};

	return true;
}

bool CUIDialogWnd::OnKeyboardHold(int dik)
{
	if(!IR_process()) return false;
	return inherited::OnKeyboardHold(dik);
}

bool CUIDialogWnd::OnKeyboardAction(int dik, EUIMessages keyboard_action)
{
	if(!IR_process()) return false;
	if (inherited::OnKeyboardAction(dik, keyboard_action) )
		return true;
	return false;
}

bool CUIDialogWnd::IR_process()
{
	if(!IsEnabled())					return false;

	if(Device.Paused()&&!WorkInPause())	return false;
	return true;
}

void CUIDialogWnd::Update()
{
	CUIWindow::Update();
}

void CUIDialogWnd::FillDebugInfo()
{
#ifndef MASTER_GOLD
	CUIWindow::FillDebugInfo();

	if (ImGui::CollapsingHeader(CUIDialogWnd::GetDebugType()))
	{
		ImGui::LabelText("Current holder", "%s", m_pHolder ? m_pHolder->GetDebugType() : "none");
		ImGui::LabelText("Work in pause", m_bWorkInPause ? "true" : "false");
	}
#endif
}

CDialogHolder* CurrentDialogHolder();

void CUIDialogWnd::ShowDialog1(bool bDoHideIndicators)
{
	if(!IsShown())
		CurrentDialogHolder()->StartStopMenu(this,bDoHideIndicators);
}

void CUIDialogWnd::HideDialog1()
{
	if (g_statHint->IsShown())
		g_statHint->Discard();

	if (IsShown() && GetHolder())
		GetHolder()->StartStopMenu(this, false);
}
