#pragma once
#include "uiwindow.h"
#include "uilistitem.h"
#include "uiscrollbar.h"

#include "../../XrServerEntitiesCS/script_export_space.h"

#define DEFAULT_ITEM_HEIGHT 30


DEF_LIST (LIST_ITEM_LIST, CUIListItem*);
class CUIScrollBar;
class CUIFrameLineWnd;

class CUIListWnd :public CUIWindow
{
private:
	typedef			CUIWindow inherited;
	friend class	CUIGameLog;

	shared_str		m_scrollbar_profile;
	void			DrawActiveBackFrame		(const Frect& rect, CUIListItem * itm);
public:
					CUIListWnd				();
	virtual			~CUIListWnd				();

	void			InitListWnd				(Fvector2 pos, Fvector2 size, float item_height);

	virtual bool	 OnMouse				(float x, float y, EUIMessages mouse_action);
	virtual bool	 OnKeyboard				(int dik, EUIMessages keyboard_action);

	virtual void	 SendMessage			(CUIWindow *pWnd, s16 msg, void* pData);
	virtual void	 Draw					();
	virtual void	 Update					();
	virtual void	 DetachChild			(CUIWindow* pChild);
			void	SetScrollBarProfile		(LPCSTR profile)		{m_scrollbar_profile=profile;};

	// ���������� ��������� � ��������
	template <class Element>
	bool			 AddItem				(const char*  str, const float shift = 0.0f, void* pData = NULL,
											int value = 0, int insertBeforeIdx = -1);

	virtual bool	AddItem_script			(CUIListItem* item);

	template <class Element>
	bool			AddItem					(Element* pItem, int insertBeforeIdx = -1);
	
	void 			RemoveItem				(int index);
	void 			RemoveAll				();
	//������� ������ ������� � �������� pData, ����� -1
	int 			FindItem				(void* pData);
	int 			FindItemWithValue		(int iValue);
	CUIListItem*	GetItem					(int index);
	// �������� ������ �������� �� ������. ���� -1 ���� ��� ������
	int				GetItemPos				(CUIListItem *pItem);

	void			SetItemWidth			(float iItemWidth);
	float			GetItemWidth			()							{return m_iItemWidth;}

	void			SetItemHeight			(float iItemHeight); 
	float			GetItemHeight			()							{return m_iItemHeight;}
    virtual void	SetHeight				(float height);

	void			SetAlwaysShowScroll		(bool flag = true)			{m_bAlwaysShowScroll = flag;}
	void			EnableAlwaysShowScroll	(bool flag)					{m_bAlwaysShowScroll_enable = flag;}


	int				GetItemsCount			()							{return m_ItemList.size();}

	//����������� ��� �������� ������
	void 			Reset					();

	void 			EnableScrollBar			(bool enable);
	bool 			IsScrollBarEnabled		();
	void 			UpdateScrollBar			();

	void 			ScrollToBegin			();
	void 			ScrollToEnd				();
	void 			ScrollToPos				(int position);
	
	IC bool			IsActiveBackgroundEnabled()								{return m_bActiveBackground;}
	void			EnableActiveBackground	(bool enable);

	virtual void	SetWidth				(float width);

	void			SetTextColor			(u32 color)						{m_dwFontColor = color;} 
	u32				GetTextColor			()								{return m_dwFontColor;}

	//������ ��������� (��� ������) �������� ������
	void 			ActivateList			(bool activity);
	bool 			IsListActive			()								{return m_bListActivity;}

	void 			SetVertFlip				(bool vert_flip)				{m_bVertFlip = vert_flip;}
	bool 			GetVertFlip				() {return m_bVertFlip;}

	// �������������� ��������� ������
	void			SetFocusedItem			(int iNewFocusedItem);
	int				GetFocusedItem			()								{ return m_iFocusedItem; }
	int				GetSelectedItem			()								{ return m_iSelectedItem; }
	void			SetSelectedItem			(int sel)						{m_iSelectedItem = sel;}
	void			ShowSelectedItem		(bool show = true);
	
	void			ResetFocusCapture		()								{ m_bForceFocusedItem = false; }

	int				GetListPosition			() const						{ return m_iFirstShownIndex; }


protected:
	void			create_active_back		();
	void			destroy_active_back		();

	CUIScrollBar*	m_ScrollBar;

	//���������� ��������� ������, ���������
	//���� ��������� ���������
	void			UpdateList				();

	//������ ��������� �����
	LIST_ITEM_LIST	m_ItemList; 

	//������� �������� ������
	float		m_iItemHeight;
	float		m_iItemWidth;

	//���������� ����� ��� ���������
	int		m_iRowNum;
	
	//������ ������� ����������� ��������
	int		m_iFirstShownIndex;

	//������� ��� ������� ������ � ������ ������ ��� -1, ���� ������ ���
	int		m_iFocusedItem;
	int		m_iFocusedItemGroupID;
	int     m_iSelectedItem;
	int     m_iSelectedItemGroupID;

	bool	m_bShowSelectedItem;
	bool	m_bAlwaysShowScroll_enable;
	bool	m_bAlwaysShowScroll;
	bool	m_bActiveBackground;
	// ���� ����� ������������� ���������� �����, �� ������� ���� ����
	bool	m_bForceFocusedItem;

	//��������� ��������� ��������
	CUIFrameLineWnd*	m_ActiveBackgroundFrame;

	//������� ���� ������
	u32		m_dwFontColor;
	bool	m_bListActivity;

	//��������� ������ �� ���������
	bool	m_bVertFlip;
	
	// ������� ����, ��� ���� ���������
	bool	m_bUpdateMouseMove;

	// ������� ���������� �������������
	int		m_iLastUniqueID;

	DECLARE_SCRIPT_REGISTER_FUNCTION
};

#include "UIListWnd_inline.h"

add_to_type_list(CUIListWnd)
#undef script_type_list
#define script_type_list save_type_list(CUIListWnd)
