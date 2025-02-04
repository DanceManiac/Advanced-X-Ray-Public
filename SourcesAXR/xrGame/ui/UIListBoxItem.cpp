#include "StdAfx.h"
#include "UIListBoxItem.h"
#include "UIScrollView.h"
#include "UIBtnHint.h"
#include "object_broker.h"
#include "UIStatic.h"
#include "UICursor.h"

CUIListBoxItem::CUIListBoxItem(float height)
:m_text(NULL),tag(u32(-1))
{
	SetHeight		(height);
	m_text			= AddTextField("---", 10.0f);
}

void CUIListBoxItem::SetTAG(u32 value)
{
	tag = value;
}

u32 CUIListBoxItem::GetTAG()
{
	return tag;
}

void CUIListBoxItem::SetHint(LPCSTR hint_str)
{
	item_hint = hint_str;
}

void CUIListBoxItem::Draw()
{
	if(m_bSelected)
		DrawElements();

	CUIWindow::Draw	();

	if (g_btnHint->Owner() == this)
		g_btnHint->Draw_();
}

bool is_in3(const Frect& b1, const Frect& b2)
{
	return (b1.x1 < b2.x1) && (b1.x2 > b2.x2) && (b1.y1 < b2.y1) && (b1.y2 > b2.y2);
}

void CUIListBoxItem::Update()
{
	CUIWindow::Update();
	if (CursorOverWindow() && item_hint != nullptr && !g_btnHint->Owner() && Device.dwTimeGlobal > m_dwFocusReceiveTime + 700)
	{
		g_btnHint->SetHintText(this, item_hint);

		Fvector2 c_pos = GetUICursor().GetCursorPosition();
		Frect vis_rect;
		vis_rect.set(0, 0, UI_BASE_WIDTH, UI_BASE_HEIGHT);

		//select appropriate position
		Frect r;
		r.set(0.0f, 0.0f, g_btnHint->GetWidth(), g_btnHint->GetHeight());
		r.add(c_pos.x, c_pos.y);

		r.sub(0.0f, r.height());
		if (false == is_in3(vis_rect, r))
			r.sub(r.width(), 0.0f);
		if (false == is_in3(vis_rect, r))
			r.add(0.0f, r.height());

		if (false == is_in3(vis_rect, r))
			r.add(r.width(), 45.0f);

		g_btnHint->SetWndPos(r.lt);
	}
}

void CUIListBoxItem::OnFocusLost()
{
	inherited::OnFocusLost();

	if (g_btnHint->Owner() == this)
		g_btnHint->Discard();
}

void CUIListBoxItem::OnFocusReceive()
{
	inherited::OnFocusReceive();
	GetMessageTarget()->SendMessage(this, LIST_ITEM_FOCUS_RECEIVED);
}

void CUIListBoxItem::InitDefault()
{
	InitTexture("ui_listline","hud\\default");
}

void CUIListBoxItem::SetFont(CGameFont* F)
{
	m_text->SetFont(F);
}

CGameFont* CUIListBoxItem::GetFont()
{
	return (m_text)?m_text->GetFont():NULL;
}

bool CUIListBoxItem::OnMouseDown(int mouse_btn)
{
	if (mouse_btn==MOUSE_1)
	{
		smart_cast<CUIScrollView*>(GetParent()->GetParent())->SetSelected(this);
		GetMessageTarget()->SendMessage(this, LIST_ITEM_SELECT, &tag);
		GetMessageTarget()->SendMessage(this, LIST_ITEM_CLICKED, &tag);
		return true;
	}else
		return false;
}

void CUIListBoxItem::SetTextColor(u32 color)
{
	m_text->SetTextColor(color);
}

u32 CUIListBoxItem::GetTextColor()
{
	return (m_text)?m_text->GetTextColor():0xffffffff;
}

float CUIListBoxItem::FieldsLength() const
{
	if(m_ChildWndList.empty())
		return 0.0f;

	float len = 0.0f;
/*
	WINDOW_LIST::const_iterator it		= m_ChildWndList.begin();
	WINDOW_LIST::const_iterator it_e	= m_ChildWndList.end();

	for(;it!=it_e;++it)
	{
		CUIWindow* w	= *it;
		len				+= w->GetWndPos().x + w->GetWidth();
	}
*/
	CUIWindow* w	= m_ChildWndList.back();
	len				+= w->GetWndPos().x + w->GetWidth();
	return len;
}

CUIStatic* CUIListBoxItem::AddIconField(float width)
{
	CUIStatic* st			= xr_new<CUIStatic>();
	st->SetAutoDelete		(true);
	st->SetWndPos			(Fvector2().set(FieldsLength(),0.0f));
	st->SetWndSize			(Fvector2().set(width, GetHeight()));
	AttachChild				(st);
	return					st;
}

CUITextWnd* CUIListBoxItem::AddTextField(LPCSTR txt, float width)
{
	CUITextWnd* st			= xr_new<CUITextWnd>();
	st->SetAutoDelete		(true);
	st->SetWndPos			(Fvector2().set(FieldsLength(),0.0f));
	st->SetWndSize			(Fvector2().set(width, GetHeight()));

	AttachChild				(st);

	st->SetFont				(GetFont());
	st->SetTextColor		(GetTextColor());
	st->SetText				(txt);	
	st->SetVTextAlignment	(valCenter);	
	return							st;
}

void CUIListBoxItem::SetData(void* data)
{
	pData = data;
}

void* CUIListBoxItem::GetData()
{
	return pData;
}

void CUIListBoxItem::SetText(LPCSTR txt)
{
	m_text->SetText(txt);
}

LPCSTR CUIListBoxItem::GetText()							
{
	return m_text->GetText();
}
