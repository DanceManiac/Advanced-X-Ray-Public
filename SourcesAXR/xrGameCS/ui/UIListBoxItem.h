#pragma once
#include "UIFrameLineWnd.h"

class CUIListBoxItem : public CUIFrameLineWnd, public CUISelectable
{
	typedef				CUIFrameLineWnd inherited;
public:
						CUIListBoxItem(float height);
	virtual				~CUIListBoxItem();

    virtual void		SetSelected(bool b);
	virtual void		Draw();
	virtual bool		OnMouseDown(int mouse_btn);
	virtual void		OnFocusReceive();
			void		InitDefault();
			void		SetTAG(u32 value);
			u32			GetTAG();

			void		SetData(void* data);
			void*		GetData();

		CUIStatic*		AddField(LPCSTR txt, float len, LPCSTR key = "");
		LPCSTR			GetField(LPCSTR key);

	CUIStatic			m_text;

	void				SetText(LPCSTR txt);
	LPCSTR				GetText();
	void				SetTextColor(u32 color, u32 color_s);
	u32					GetTextColor();
	void				SetFont(CGameFont* F);
	CGameFont*			GetFont();

protected:

			float		FieldsLength();
		xr_vector<CUIStatic*>	fields;
		u32				txt_color;
		u32				txt_color_s;
		u32				tag;
		void*			pData;
};

