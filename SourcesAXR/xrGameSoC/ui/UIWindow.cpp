// UIWindow.cpp: implementation of the CUIWindow class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "UIWindow.h"
#include "../UICursor.h"
#include "../MainMenu.h"

#include "../Include/xrRender/DebugRender.h"
#include "../Include/xrRender/UIRender.h"

#include <imgui.h>

poolSS< _12b, 128>	ui_allocator;

#ifdef LOG_ALL_WNDS
int ListWndCount = 0;
struct DBGList {
	int				num;
	bool			closed;
};
xr_vector<DBGList>	dbg_list_wnds;
void dump_list_wnd() {
	Msg("------Total  wnds %d", dbg_list_wnds.size());
	xr_vector<DBGList>::iterator _it = dbg_list_wnds.begin();
	for (; _it != dbg_list_wnds.end(); ++_it)
		if (!(*_it).closed)
			Msg("--leak detected ---- wnd = %d", (*_it).num);
}
#else
void dump_list_wnd() {}
#endif

xr_vector<Frect> g_wnds_rects;
/*ref_shader  dbg_draw_sh =0;
ref_geom	dbg_draw_gm =0;*/

BOOL g_show_wnd_rect = FALSE;
BOOL g_show_wnd_rect2 = FALSE;
bool SSFX_UI_DoF_active = false;

void clean_wnd_rects()
{
	/*dbg_draw_sh.destroy();
	dbg_draw_gm.destroy();*/
#ifdef DEBUG
	DRender->DestroyDebugShader(IDebugRender::dbgShaderWindow);
#endif // DEBUG
}

void add_rect_to_draw(Frect r)
{
	g_wnds_rects.push_back(r);
}
void draw_rect(Frect& r, u32 color)
{
#ifdef DEBUG
	DRender->SetDebugShader(IDebugRender::dbgShaderWindow);

	//.	UIRender->StartLineStrip	(5);
	UIRender->StartPrimitive(5, IUIRender::ptLineStrip, IUIRender::pttTL);

	UIRender->PushPoint(r.lt.x, r.lt.y, 0, color, 0, 0);
	UIRender->PushPoint(r.rb.x, r.lt.y, 0, color, 0, 0);
	UIRender->PushPoint(r.rb.x, r.rb.y, 0, color, 0, 0);
	UIRender->PushPoint(r.lt.x, r.rb.y, 0, color, 0, 0);
	UIRender->PushPoint(r.lt.x, r.lt.y, 0, color, 0, 0);

	//.	UIRender->FlushLineStrip();
	UIRender->FlushPrimitive();
	/*if(!dbg_draw_sh){
		dbg_draw_sh.create("hud\\default","ui\\ui_pop_up_active_back");
		dbg_draw_gm.create(FVF::F_TL, RCache.Vertex.Buffer(), 0);
	}
	RCache.set_Shader			(dbg_draw_sh);
	u32							vOffset;
	FVF::TL* pv					= (FVF::TL*)RCache.Vertex.Lock	(5,dbg_draw_gm.stride(),vOffset);
	pv->set(r.lt.x, r.lt.y, color, 0,0); ++pv;
	pv->set(r.rb.x, r.lt.y, color, 0,0); ++pv;
	pv->set(r.rb.x, r.rb.y, color, 0,0); ++pv;
	pv->set(r.lt.x, r.rb.y, color, 0,0); ++pv;
	pv->set(r.lt.x, r.lt.y, color, 0,0); ++pv;
	RCache.Vertex.Unlock		(5,dbg_draw_gm.stride());
	RCache.set_Geometry			(dbg_draw_gm);
	RCache.Render				(D3DPT_LINESTRIP,vOffset,4);*/
#endif // DEBUG
}
void draw_wnds_rects()
{
	if (0 == g_wnds_rects.size())	return;

	xr_vector<Frect>::iterator it = g_wnds_rects.begin();
	xr_vector<Frect>::iterator it_e = g_wnds_rects.end();

	for (; it != it_e; ++it)
	{
		Frect& r = *it;
		UI().ClientToScreenScaled(r.lt, r.lt.x, r.lt.y);
		UI().ClientToScreenScaled(r.rb, r.rb.x, r.rb.y);
		draw_rect(r, color_rgba(255, 0, 0, 255));
	};

	g_wnds_rects.clear();
}

void CUIWindow::SetPPMode()
{
	m_bPP = true;
	MainMenu()->RegisterPPDraw(this);
	Show(false);
};

void CUIWindow::ResetPPMode()
{
	if(	GetPPMode() ){
		MainMenu()->UnregisterPPDraw	(this);
		m_bPP							= false;
	}
}

CUIWindow::CUIWindow()
{
//.	m_dbg_flag.zero			();
	m_pFont					= NULL;
	m_pParentWnd			= NULL;
	m_pMouseCapturer		= NULL;
	m_pOrignMouseCapturer	= NULL;
	m_pMessageTarget		= NULL;
	m_pKeyboardCapturer		=  NULL;
	SetWndRect				(0,0,0,0);
	m_bAutoDelete			= false;
    Show					(true);
	Enable					(true);
	m_bCursorOverWindow		= false;
	m_bClickable			= false;
	m_bPP					= false;
	m_dwFocusReceiveTime	= 0;
#ifdef LOG_ALL_WNDS
	ListWndCount++;
	m_dbg_id = ListWndCount;
	dbg_list_wnds.push_back(DBGList());
	dbg_list_wnds.back().num		= m_dbg_id;
	dbg_list_wnds.back().closed		= false;
#endif
}

CUIWindow::~CUIWindow()
{
	VERIFY( !(GetParent()&&IsAutoDelete()) );

	CUIWindow* parent	= GetParent();
	bool ad				= IsAutoDelete();
	if( parent && !ad )
		parent->CUIWindow::DetachChild( this );

	DetachAll();

	if(	GetPPMode() )
		MainMenu()->UnregisterPPDraw	(this);

#ifdef LOG_ALL_WNDS
	xr_vector<DBGList>::iterator _it = dbg_list_wnds.begin();
	bool bOK = false;
	for(;_it!=dbg_list_wnds.end();++_it){
		if( (*_it).num==m_dbg_id && !(*_it).closed){
			bOK = true;
			(*_it).closed = true;
			dbg_list_wnds.erase(_it);
			break;
		}
		if( (*_it).num==m_dbg_id && (*_it).closed){
			Msg("--CUIWindow [%d] already deleted", m_dbg_id);
			bOK = true;
		}
	}
	if(!bOK)
		Msg("CUIWindow::~CUIWindow.[%d] cannot find window in list", m_dbg_id);
#endif
}


void CUIWindow::Init(Frect* pRect)
{
	SetWndRect			(*pRect);
}

void CUIWindow::Draw()
{
	for(WINDOW_LIST_it it = m_ChildWndList.begin(); m_ChildWndList.end() != it; ++it){
		if(!(*it)->IsShown()) continue;
		(*it)->Draw();
	}
#ifdef DEBUG
	if(g_show_wnd_rect2){
		Frect r;
		GetAbsoluteRect(r);
		add_rect_to_draw(r);
	}
#endif
}

void CUIWindow::Draw(float x, float y){
	SetWndPos		(x,y);
	Draw			();
}

void CUIWindow::Update()
{
	if (GetUICursor().IsVisible())
	{
		bool cursor_on_window;

		Fvector2			temp = GetUICursor().GetCursorPosition();
		Frect				r;
		GetAbsoluteRect		(r);
		cursor_on_window	= !!r.in(temp);
#ifndef NDEBUG
		if(cursor_on_window&&g_show_wnd_rect){
			Frect r;
			GetAbsoluteRect(r);
			add_rect_to_draw(r);
		}
#endif
		// RECEIVE and LOST focus
		if(m_bCursorOverWindow != cursor_on_window)
		{
			if(cursor_on_window)
				OnFocusReceive();			
			else
				OnFocusLost();			
		}
	}
	
	for(WINDOW_LIST_it it = m_ChildWndList.begin(); m_ChildWndList.end()!=it; ++it){
		if(!(*it)->IsShown()) continue;
			(*it)->Update();
	}
}

void CUIWindow::AttachChild(CUIWindow* pChild)
{
	if(!pChild) return;
	
	R_ASSERT( !IsChild(pChild) );
	pChild->SetParent(this);
	m_ChildWndList.push_back(pChild);
}

void CUIWindow::DetachChild(CUIWindow* pChild)
{
	if(NULL==pChild)
		return;
	
	if(m_pMouseCapturer == pChild)
		SetCapture(pChild, false);

	SafeRemoveChild(pChild);
	pChild->SetParent(NULL);

	if(pChild->IsAutoDelete())
		xr_delete(pChild);
}

void CUIWindow::DetachAll()
{
	while( !m_ChildWndList.empty() ){
		DetachChild( m_ChildWndList.back() );	
	}
}

void CUIWindow::GetAbsoluteRect(Frect& r) 
{
//.	Frect rect;

	if(GetParent() == NULL){
		GetWndRect		(r);
		return;
	}
//.	rect = GetParent()->GetAbsoluteRect();
	GetParent()->GetAbsoluteRect(r);

	Frect			rr;
	GetWndRect		(rr);
	r.left			+= rr.left;
	r.top			+= rr.top;
	r.right			= r.left + GetWidth();
	r.bottom		= r.top	+ GetHeight();
//.	return			rect;
}

#define DOUBLE_CLICK_TIME 250

bool CUIWindow::OnMouseAction(float x, float y, EUIMessages mouse_action)
{	
	Frect	wndRect = GetWndRect();

	m_cursor_pos.x = x;
	m_cursor_pos.y = y;


	if( WINDOW_LBUTTON_DOWN == mouse_action )
	{
		static u32 _last_db_click_frame		= 0;
		u32 dwCurTime						= Device.dwTimeContinual;

		if( (_last_db_click_frame!=Device.dwFrame) && (dwCurTime-m_dwLastClickTime < DOUBLE_CLICK_TIME) )
		{
            mouse_action			= WINDOW_LBUTTON_DB_CLICK;
			_last_db_click_frame	= Device.dwFrame;
		}

		m_dwLastClickTime = dwCurTime;
	}

	if(GetParent()== NULL)
	{
		if(!wndRect.in(m_cursor_pos))
			return false;

		m_cursor_pos.x -= wndRect.left;
		m_cursor_pos.y -= wndRect.top;
	}

	if(m_pMouseCapturer)
	{
		m_pMouseCapturer->OnMouseAction(m_cursor_pos.x - m_pMouseCapturer->GetWndRect().left,
								m_cursor_pos.y - m_pMouseCapturer->GetWndRect().top, 
								mouse_action);
		return true;
	}

	// handle any action
	switch (mouse_action){
		case WINDOW_MOUSE_MOVE:
			OnMouseMove();							break;
		case WINDOW_MOUSE_WHEEL_DOWN:
			OnMouseScroll(WINDOW_MOUSE_WHEEL_DOWN); break;
		case WINDOW_MOUSE_WHEEL_UP:
			OnMouseScroll(WINDOW_MOUSE_WHEEL_UP);	break;
		case WINDOW_LBUTTON_DOWN:
			if(OnMouseDown(MOUSE_1))				return true;	break;
		case WINDOW_RBUTTON_DOWN:
			if(OnMouseDown(MOUSE_2))				return true;	break;
		case WINDOW_CBUTTON_DOWN:
			if(OnMouseDown(MOUSE_3))				return true;	break;
		case WINDOW_LBUTTON_DB_CLICK:
			if (OnDbClick())						return true;	break;
		default:
            break;
	}

	//ѕроверка на попадание мыши в окно,
	//происходит в обратном пор€дке, чем рисование окон
	//(последние в списке имеют высший приоритет)
	WINDOW_LIST::reverse_iterator it = m_ChildWndList.rbegin();

	for(; it!=m_ChildWndList.rend(); ++it)
	{
		CUIWindow* w	= (*it);
		Frect wndRect_	= w->GetWndRect();
		if (wndRect_.in(m_cursor_pos) )
		{
			if(w->IsEnabled())
			{
				if( w->OnMouseAction(m_cursor_pos.x -w->GetWndRect().left,
							m_cursor_pos.y -w->GetWndRect().top, mouse_action))return true;
			}
		}
		else if (w->IsEnabled() && w->CursorOverWindow())
		{
			if( w->OnMouseAction(m_cursor_pos.x -w->GetWndRect().left,
						m_cursor_pos.y -w->GetWndRect().top, mouse_action))return true;
		}
	}


	return false;
}

bool CUIWindow::HasChildMouseHandler(){
	WINDOW_LIST::reverse_iterator it = m_ChildWndList.rbegin();

	for( ; it!=m_ChildWndList.rend(); ++it)
	{
		if ((*it)->m_bClickable)
		{
			Frect wndRect_ = (*it)->GetWndRect();
			if (wndRect_.in(m_cursor_pos) )
				return true;
		}
	}

	return false;
}

void CUIWindow::OnMouseMove(){
}

void CUIWindow::OnMouseScroll(float iDirection){
}

bool CUIWindow::OnDbClick(){
	if (GetMessageTarget())
		GetMessageTarget()->SendMessage(this, WINDOW_LBUTTON_DB_CLICK);
	return false;
}

bool CUIWindow::OnMouseDown(int mouse_btn){
	return false;
}

void CUIWindow::OnMouseUp(int mouse_btn){
}

void CUIWindow::OnFocusReceive()
{
	m_dwFocusReceiveTime	= Device.dwTimeGlobal;
	m_bCursorOverWindow		= true;	
}

void CUIWindow::OnFocusLost()
{
	m_dwFocusReceiveTime	= 0;
	m_bCursorOverWindow		= false;	
}

void CUIWindow::SetCapture(CUIWindow *pChildWindow, bool capture_status)
{
	if(NULL != GetParent())
	{
		if(m_pOrignMouseCapturer == NULL || m_pOrignMouseCapturer == pChildWindow)
			GetParent()->SetCapture(this, capture_status);
	}

	if(capture_status)
	{
		if(NULL!=m_pMouseCapturer)
			m_pMouseCapturer->SendMessage(this, WINDOW_MOUSE_CAPTURE_LOST);

		m_pMouseCapturer = pChildWindow;
	}
	else
	{
			m_pMouseCapturer = NULL;
	}
}

bool CUIWindow::OnKeyboardAction(int dik, EUIMessages keyboard_action)
{
	bool result;

	if(NULL!=m_pKeyboardCapturer)
	{
		result = m_pKeyboardCapturer->OnKeyboardAction(dik, keyboard_action);
		
		if(result) return true;
	}

	WINDOW_LIST::reverse_iterator it = m_ChildWndList.rbegin();

	for(; it!=m_ChildWndList.rend(); ++it)
	{
		if((*it)->IsEnabled())
		{
			result = (*it)->OnKeyboardAction(dik, keyboard_action);
			
			if(result)	return true;
		}
	}
	return false;
}

bool CUIWindow::OnKeyboardHold(int dik)
{
	bool result;

	if(NULL!=m_pKeyboardCapturer)
	{
		result = m_pKeyboardCapturer->OnKeyboardHold(dik);
		
		if(result) return true;
	}

	WINDOW_LIST::reverse_iterator it = m_ChildWndList.rbegin();

	for(; it!=m_ChildWndList.rend(); ++it)
	{
		if((*it)->IsEnabled())
		{
			result = (*it)->OnKeyboardHold(dik);
			
			if(result)	return true;
		}
	}

	return false;
}

void CUIWindow::SetKeyboardCapture(CUIWindow* pChildWindow, bool capture_status)
{
	if(NULL != GetParent())
		GetParent()->SetKeyboardCapture(this, capture_status);

	if(capture_status)
	{
		if(NULL!=m_pKeyboardCapturer)
			m_pKeyboardCapturer->SendMessage(this, WINDOW_KEYBOARD_CAPTURE_LOST);
			
		m_pKeyboardCapturer = pChildWindow;
	}
	else
		m_pKeyboardCapturer = NULL;
}

void CUIWindow::SendMessage(CUIWindow *pWnd, s16 msg, void *pData)
{
	for(WINDOW_LIST_it it = m_ChildWndList.begin(); m_ChildWndList.end()!=it; ++it)
	{
		if((*it)->IsEnabled())
			(*it)->SendMessage(pWnd,msg,pData);
	}
}

CUIWindow* CUIWindow::GetCurrentMouseHandler(){
	return GetTop()->GetChildMouseHandler();
}

CUIWindow* CUIWindow::GetChildMouseHandler(){
	CUIWindow* pWndResult;
	WINDOW_LIST::reverse_iterator it = m_ChildWndList.rbegin();

	for(; it!=m_ChildWndList.rend(); ++it)
	{
		Frect wndRect = (*it)->GetWndRect();
		// very strange code.... i can't understand difference between
		// first and second condition. I Got It from OnMouseAction() method;
		if (wndRect.in(m_cursor_pos) )
		{
			if((*it)->IsEnabled())
			{
				return pWndResult = (*it)->GetChildMouseHandler();				
			}
		}
		else if ((*it)->IsEnabled() && (*it)->CursorOverWindow())
		{
			return pWndResult = (*it)->GetChildMouseHandler();
		}
	}

    return this;
}

bool CUIWindow::BringToTop(CUIWindow* pChild)
{
/*	WINDOW_LIST_it it = std::find(m_ChildWndList.begin(), 
										m_ChildWndList.end(), 
										pChild);
*/
	if( !IsChild(pChild) ) return false;

	SafeRemoveChild(pChild);
//	m_ChildWndList.remove(pChild);
	m_ChildWndList.push_back(pChild);

	return true;
}

void CUIWindow::BringAllToTop()
{
	if(GetParent() == NULL)
			return;
	else
	{
		GetParent()->BringToTop(this);
		GetParent()->BringAllToTop();
	}
}

void CUIWindow::Reset()
{
	m_pOrignMouseCapturer = m_pMouseCapturer = NULL;
}
void CUIWindow::ResetAll()
{
//.	m_dbg_flag.set(128,TRUE);
	for(WINDOW_LIST_it it = m_ChildWndList.begin(); m_ChildWndList.end()!=it; ++it)
	{
		(*it)->Reset();
	}
//.	m_dbg_flag.set(128,FALSE);
}

CUIWindow* CUIWindow::GetMessageTarget()
{
	return m_pMessageTarget?m_pMessageTarget:GetParent();
}

bool CUIWindow::IsChild(CUIWindow *pPossibleChild) const
{
	WINDOW_LIST::const_iterator it = std::find(m_ChildWndList.begin(), m_ChildWndList.end(), pPossibleChild);
	return it != m_ChildWndList.end();
}


CUIWindow*	CUIWindow::FindChild(const shared_str name)
{
	if(WindowName()==name)
		return this;

//.	m_dbg_flag.set(256,TRUE);
	WINDOW_LIST::const_iterator it = m_ChildWndList.begin();
	WINDOW_LIST::const_iterator it_e = m_ChildWndList.end();
	for(;it!=it_e;++it){
		CUIWindow* pRes = (*it)->FindChild(name);
		if(pRes != NULL)
			return pRes;
	}

//.	m_dbg_flag.set(256,FALSE);
	return NULL;
}

void CUIWindow::SetParent(CUIWindow* pNewParent) 
{
	R_ASSERT( !(m_pParentWnd && m_pParentWnd->IsChild(this)) );

	m_pParentWnd = pNewParent;
}

void CUIWindow::ShowChildren(bool show){
//.	m_dbg_flag.set(512,TRUE);
	for(WINDOW_LIST_it it = m_ChildWndList.begin(); m_ChildWndList.end()!=it; ++it)		
			(*it)->Show(show);
//.	m_dbg_flag.set(512,FALSE);
}

bool CUIWindow::FillDebugTree(const CUIDebugState& debugState)
{
#ifndef MASTER_GOLD
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;

	if (debugState.selected == this)
		flags |= ImGuiTreeNodeFlags_Selected;

	if (m_ChildWndList.empty())
		flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;

	const bool open = ImGui::TreeNodeEx(this, flags, "%s (%s)", WindowName().c_str(), GetDebugType());

	if (ImGui::IsItemClicked())
		debugState.select(this);

	const bool hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled);

	if (debugState.drawWndRects && (IsShown() || hovered))
	{
		Frect rect;
		GetAbsoluteRect(rect);
		UI().ClientToScreenScaled(rect.lt, rect.lt.x, rect.lt.y);
		UI().ClientToScreenScaled(rect.rb, rect.rb.x, rect.rb.y);

		// This is pseudo RNG, so when we are seeding it with 'this' pointer
		// we can expect predictable and stable values (no *blinking* at all)
		CRandom rnd;
		rnd.seed((s32)(intptr_t)this);
		u32 color = color_rgba(255, 0, 0, 255);

		if (hovered)
			color = color_rgba(255, 255, 0, 255);
		else if (debugState.coloredRects)
			color = color_rgba(rnd.randI(255), rnd.randI(255), rnd.randI(255), 255);

		const auto draw_list = hovered ? ImGui::GetForegroundDrawList() : ImGui::GetBackgroundDrawList();
		draw_list->AddRect((const ImVec2&)rect.lt, (const ImVec2&)rect.rb, color);
	}

	if (open)
	{
		for (const auto& child : m_ChildWndList)
		{
			child->FillDebugTree(debugState);
		}
		if (!m_ChildWndList.empty())
			ImGui::TreePop();
	}

	return open;
#else
	return nullptr;
#endif
}

void CUIWindow::FillDebugInfo()
{
#ifndef MASTER_GOLD
	if (!ImGui::CollapsingHeader(CUIWindow::GetDebugType()))
		return;

	ImGui::DragFloat2("Position", (float*)&m_wndPos);
	ImGui::DragFloat2("Size", (float*)&m_wndSize);

	ImGui::Checkbox("Visible", &m_bShowMe);
	ImGui::Checkbox("Enabled", &m_bIsEnabled);

	ImGui::Separator();
	ImGui::BeginDisabled();
	ImGui::Checkbox("Auto delete", &m_bAutoDelete);
	ImGui::Checkbox("Cursor over window", &m_bCursorOverWindow);

	ImGui::DragFloat2("Last cursor position", (float*)&m_cursor_pos);
	ImGui::DragScalar("Last click time", ImGuiDataType_U32, &m_dwLastClickTime);
	ImGui::DragScalar("Focus receive time", ImGuiDataType_U32, &m_dwFocusReceiveTime);
	ImGui::EndDisabled();

	ImGui::Separator();
	ImGui::LabelText("Parent", "%s", m_pParentWnd ? m_pParentWnd->WindowName().c_str() : "none");
	ImGui::LabelText("Mouse capturer", "%s", m_pMouseCapturer ? m_pMouseCapturer->WindowName().c_str() : "none");
	ImGui::LabelText("Keyboard capturer", "%s", m_pKeyboardCapturer ? m_pKeyboardCapturer->WindowName().c_str() : "none");
	ImGui::LabelText("Message target", "%s", m_pMessageTarget ? m_pMessageTarget->WindowName().c_str() : "none");
#endif
}