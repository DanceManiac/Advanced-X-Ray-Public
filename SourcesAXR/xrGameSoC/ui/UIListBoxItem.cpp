#include "StdAfx.h"
#include "UIListBoxItem.h"
#include "UIScrollView.h"
#include "UIBtnHint.h"
#include "object_broker.h"
#include "UICursor.h"

CUIListBoxItem::CUIListBoxItem(float height)
{
	txt_color			= 0xffffffff;
	txt_color_s			= 0xffffffff;
	tag					= u32(-1);
	m_bTextureAvailable = false;
	AttachChild			(&m_text);
	SetHeight			(height);
}

CUIListBoxItem::~CUIListBoxItem()
{
	delete_data			(fields);
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
	m_bTextureAvailable = m_bSelected;

	u32 CurColor = m_text.GetTextColor();
	u32 ResColor = (IsEnabled() ? 0xff000000 : 0x80000000) | (CurColor & 0x00ffffff);
	m_text.SetTextColor(ResColor);

	inherited::Draw();

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
	m_text.SetFont(F);
}

CGameFont* CUIListBoxItem::GetFont()
{
	return m_text.GetFont();
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

void CUIListBoxItem::SetSelected(bool b)
{
	CUISelectable::SetSelected(b);
	u32 col;
	if (b)
		col = txt_color_s;	
	else
		col = txt_color;

	m_text.SetTextColor(col);
	for (u32 i = 0; i<fields.size(); i++)
		fields[i]->SetTextColor(col);
}

void CUIListBoxItem::SetTextColor(u32 color, u32 color_s)
{
	txt_color			= color;
	txt_color_s			= color_s;
	m_text.SetTextColor(color);
}

float CUIListBoxItem::FieldsLength()
{
	if (m_ChildWndList.empty())
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
	CUIWindow* w = m_ChildWndList.back();
	len += w->GetWndPos().x + w->GetWidth();
	return len;
}

CUIStatic* CUIListBoxItem::AddField(LPCSTR txt, float len, LPCSTR key)
{
	fields.push_back		(xr_new<CUIStatic>());
	CUIStatic* st			= fields.back();
	AttachChild				(st);
	st->SetWndPos			(Fvector2().set(FieldsLength(),0.0f));
	st->SetWndSize			(Fvector2().set(GetWidth(), GetHeight()));
	st->SetFont				(m_text.GetFont());
	st->SetTextAlignment	(m_text.GetTextAlignment());
	st->SetVTextAlignment	(m_text.GetVTextAlignment());
	st->SetTextColor		(txt_color);
	st->SetText				(txt);	
	st->SetWindowName		(key);

	return st;
}

LPCSTR CUIListBoxItem::GetField(LPCSTR key)
{
	for (u32 i = 0; i<fields.size(); i++)
	{
		if (0 == xr_strcmp(fields[i]->WindowName(),key))
			return fields[i]->GetText();
	}
	return NULL;
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
	m_text.SetText(txt);
}

LPCSTR CUIListBoxItem::GetText()
{
	return m_text.GetText();
}
