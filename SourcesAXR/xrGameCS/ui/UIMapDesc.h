#pragma once

#include "UIDialogWnd.h"

class CUIStatic;
class CUIScrollView;
class CUI3tButtonEx;
class CUIMapInfo;
class CUIStatsPlayerList;

class CUIMapDesc : public CUIDialogWnd {
public:
	CUIMapDesc();
	~CUIMapDesc();

    virtual void SendMessage(CUIWindow* pWnd, s16 msg, void *pData = 0);
	virtual bool OnKeyboard(int dik, EUIMessages keyboard_action);

private:
	void	Init();
	

	CUIStatic*		m_pCaption;
	CUIStatic*		m_pBackground;
	CUIStatic*		m_pFrame[3];
	CUIScrollView*	m_pTextDesc;
	CUIStatic*		m_pImage;
	CUI3tButtonEx*	m_pBtnSpectator;
	CUI3tButtonEx*	m_pBtnNext;
	CUIMapInfo*		m_pMapInfo;
};