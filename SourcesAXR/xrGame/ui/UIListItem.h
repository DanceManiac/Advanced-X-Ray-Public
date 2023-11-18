#pragma once
#include "UIbutton.h"

class CUIListItem :	public CUIButton
{
private:
	typedef CUIButton inherited;
public:
				CUIListItem		();
	virtual		~CUIListItem	();

			void InitListItem(Fvector2 pos, Fvector2 size);
//.	virtual void Init(const char* str, float x, float y, float width, float height);
	virtual void InitTexture(LPCSTR tex_name);
	
			void* GetData() {return m_pData;}
			void SetData(void* pData) { m_pData = pData;}

			int GetIndex() {return m_iIndex;}
			void SetIndex(int index) {m_iIndex = index; m_iGroupID = index;}

			int GetValue() {return m_iValue;}
			void SetValue(int value) {m_iValue = value;}

			int	GetGroupID() { return m_iGroupID; }
			void SetGroupID(int ID) { m_iGroupID = ID; }

	virtual void	MarkSelected				(bool b){};
	// �������������� �������� ������������� ������
	virtual bool IsHighlightText();
	virtual void SetHighlightText(bool Highlight)		{ m_bHighlightText = Highlight; }

protected:
	//��������� �� ������������ ������, ������� �����
	//������������� � ��������
	void* m_pData;
	
	//������������ �����, ����������� �������
	int m_iValue;
	
	//������ � ������
	int m_iIndex;

	// ������������� ������
	int m_iGroupID;

	// �������������� ������ ��� ���?
	bool m_bHighlightText;
};
