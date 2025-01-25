#include "StdAfx.h"
#include "UIListBoxItem.h"
#include "UIScrollView.h"
#include "object_broker.h"

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

void CUIListBoxItem::Draw()
{
	m_bTextureAvailable = m_bSelected;

	u32 CurColor = m_text.GetTextColor();
	u32 ResColor = (IsEnabled() ? 0xff000000 : 0x80000000) | (CurColor & 0x00ffffff);
	m_text.SetTextColor(ResColor);

	inherited::Draw();
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
