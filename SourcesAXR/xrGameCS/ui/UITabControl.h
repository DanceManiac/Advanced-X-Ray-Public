#pragma once

#include "uiwindow.h"
#include "../../XrServerEntitiesCS/script_export_space.h"
#include "UIOptionsItem.h"

class CUITabButton;
class CUIButton;

DEF_VECTOR (TABS_VECTOR, CUITabButton*)

class CUITabControl: public CUIWindow, public CUIOptionsItem 
{
	typedef				CUIWindow inherited;
public:
						CUITabControl				();
	virtual				~CUITabControl				();

	// options item
	virtual void		SetCurrentValue				();
	virtual void		SaveValue					();
	virtual bool		IsChanged					();

	virtual bool		OnKeyboard					(int dik, EUIMessages keyboard_action);
	virtual void		OnTabChange					(const shared_str& sCur, const shared_str& sPrev);
	virtual void		OnStaticFocusReceive		(CUIWindow* pWnd);
	virtual void		OnStaticFocusLost			(CUIWindow* pWnd);

	// ���������� ������-�������� � ������ �������� ��������
	bool				AddItem						(LPCSTR pItemName, LPCSTR pTexName, Fvector2 pos, Fvector2 size);
	bool				AddItem						(CUITabButton *pButton);

//.	void				RemoveItem					(const shared_str& Id);
	void				RemoveAll					();

	virtual void		SendMessage					(CUIWindow *pWnd, s16 msg, void *pData);

	const shared_str&	GetActiveId					()								{ return m_sPushedId; }
	LPCSTR				GetActiveId_script			();
	const shared_str&	GetPrevActiveId				()								{ return m_sPrevPushedId; }
			void		SetActiveTab				(const shared_str& sNewTab);
			void		SetActiveTab_script			(LPCSTR sNewTab)				{SetActiveTab(sNewTab);};
	const	u32			GetTabsCount				() const						{ return m_TabsArr.size(); }
	
	// ����� ������������� ������������� (���/����)
	IC bool				GetAcceleratorsMode			() const						{ return m_bAcceleratorsEnable; }
	void				SetAcceleratorsMode			(bool bEnable)					{ m_bAcceleratorsEnable = bEnable; }


	TABS_VECTOR *		GetButtonsVector			()								{ return &m_TabsArr; }
	CUITabButton*		GetButtonById				(const shared_str& id);
	CUITabButton*		GetButtonById_script		(LPCSTR s)						{ return GetButtonById(s);}
//.	const shared_str	GetCommandName				(const shared_str& id);
//.	CUITabButton*		GetButtonByCommand			(const shared_str& n);

	void		ResetTab					();
protected:
	// ������ ������ - �������������� ��������
	TABS_VECTOR			m_TabsArr;

	shared_str			m_sPushedId;
	shared_str			m_sPrevPushedId;
// ������� ������� ������. -1 - �� ����, 0 - ������, 1 - ������, � �.�.
//.	int					m_iPushedIndex;
//.	int					m_iPrevPushedIndex;

	// ���� ���������� ���������
	u32					m_cGlobalTextColor;
	u32					m_cGlobalButtonColor;

	// ���� ������� �� �������� ��������
	u32					m_cActiveTextColor;
	u32					m_cActiveButtonColor;

	bool				m_bAcceleratorsEnable;
	DECLARE_SCRIPT_REGISTER_FUNCTION
};

add_to_type_list(CUITabControl)
#undef script_type_list
#define script_type_list save_type_list(CUITabControl)
