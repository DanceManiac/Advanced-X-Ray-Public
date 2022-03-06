#pragma once

#include "UIDialogWnd.h"
#include "UIFrameWindow.h"
#include "UIListWnd.h"
#include "UIListItem.h"
#include "UIXmlInit.h"
#include "UIButton.h"
#include "UIMultiTextStatic.h"

DEF_VECTOR (FIELDS_VECTOR, CUIButton*)

// ����� ��� ����������� ������ ����� ������
class CUIStatsListItem: public CUIListItem
{
	typedef CUIListItem inherited;
public:
	virtual ~CUIStatsListItem() {};
	void XmlInit(const char *path, CUIXml &uiXml);
	void Highlight(bool bHighlight);
	void SetSubItemColor(u32 uItemIndex, u32 uColor);

	// ���� ������
	FIELDS_VECTOR FieldsVector;
};

class CUIStatsWnd: public CUIDialogWnd
{
private:
	typedef CUIDialogWnd inherited;

	string1024	XML_NAME;
public:
	CUIStatsWnd(LPCSTR XML = NULL);
	virtual ~CUIStatsWnd();

	virtual void Init(LPCSTR XML = NULL);
//	virtual void SendMessage(CUIWindow* pWnd, s16 msg, void* pData);

	// �������� 1 �������. ��������� ���� ���������� ��������������. ���������� ��������� 
	// �� ����������� �������
	CUIStatsListItem * AddItem();
	// �������� �������, ��� �		����� ������ � ����� �� ������. ����� ������
	// ������� ������� � ��������� ������
	CUIStatsListItem * FindFrom(u32 beg_pos, const char *strCaption);
	// ������� ������� � ������� ���� ������ � ������� strCaption. � ������ Item'� ����� 
	// ������ � ������� beg_pos
	void RemoveItemFrom(u32 beg_pos, const char *strCaption);
	// ���������� ������ �������
	void HighlightItem(u32 uItem);
	// �������� ����� ������������ ��-��
	u32	GetHighlightedItem() { return m_uHighlightedItem; }
	// �������� ������ �������
	void SelectItem(u32 uItem);
	// ���������� ����� ��������� ������ �������
	void SetHeaderColumnText(u32 headerItem, const shared_str &text);
	
	Frect GetFrameRect () { return UIFrameWnd.GetWndRect();};
	void RemoveItem (const u32 Index) {UIStatsList.RemoveItem(Index);};

	CUIFrameWindow*		GetFrameWindow	()	{return &UIFrameWnd;};
protected:
//	CUIButton			UIBtn;
	// ����� - ��������
	CUIFrameWindow		UIFrameWnd;
	// ���� ��� ����������� ������ ���������� �������
	CUIListWnd			UIStatsList;
	// ����������� �������
	u32					m_uHighlightedItem;
	// ���������
	CUIMultiTextStatic	UIHeader;
};
