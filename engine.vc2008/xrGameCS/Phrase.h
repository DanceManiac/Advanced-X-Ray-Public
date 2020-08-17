#pragma once

#include "PhraseScript.h"

class CPhraseDialog;
class CGameObject;

class CPhrase
{
private:
	friend CPhraseDialog;
public:
					CPhrase			();
	virtual			~CPhrase		();

	void				SetText			(LPCSTR text)		{m_text = text;}
	LPCSTR				GetText			()	const;

	LPCSTR				GetScriptText	()	const;

	void				SetID		(const shared_str& id)			{m_ID = id;}
	const shared_str&	GetID		()	const						{return m_ID;}

	int				GoodwillLevel	()	const			{return m_iGoodwillLevel;}

	bool			IsDummy			()	const;
	CDialogScriptHelper* GetScriptHelper	()			{return &m_ScriptHelper;};

	int				GetGoodwillLevel	() const		{return m_iGoodwillLevel;}
	void			SetGoodwillLevel	(int v)			{m_iGoodwillLevel = v;}
protected:

//	LPCSTR			GetScriptText	(const CGameObject* pSpeaker1, const CGameObject* pSpeaker2, LPCSTR dialog_id, int phrase_num) const;

	//���������� ������ � ������ ���� �������
	shared_str		m_ID;
	//��������� ������������� �����
	xr_string		m_text;
	xr_string		m_script_text_id;	
	xr_string		m_script_text_val;
	//����������� ������� ���������������, ����������� ��� ����
	//���� ����� ����� ���� �������
	int				m_iGoodwillLevel;
	
	//��� ������ ���������� �������
	CDialogScriptHelper	m_ScriptHelper;
};