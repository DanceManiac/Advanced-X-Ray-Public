#include "stdafx.h"
#include "UIMoneyIndicator.h"
#include "UIColorAnimatorWrapper.h"
#include "xrUIXmlParser.h"
#include "UIXmlInit.h"
#include "UIGameLog.h"

CUIMoneyIndicator::CUIMoneyIndicator(){
	AttachChild(&m_back);
	AttachChild(&m_money_amount);
	AttachChild(&m_money_change);
	m_pBonusMoney = xr_new<CUIGameLog>();
	AttachChild(m_pBonusMoney);
	m_pAnimChange = xr_new<CUIColorAnimatorWrapper>("ui_mp_chat");
	m_pAnimChange->Cyclic(false);
	m_pAnimChange->SetDone(true);
}

CUIMoneyIndicator::~CUIMoneyIndicator(){
	xr_delete(m_pAnimChange);
	xr_delete(m_pBonusMoney);
}

void CUIMoneyIndicator::InitFromXML(CUIXml& xml_doc){
	CUIXmlInit::InitWindow(xml_doc, "money_wnd", 0,	this);
	CUIXmlInit::InitStatic(xml_doc, "money_wnd:money_indicator",0, &m_back);
	CUIXmlInit::InitStatic(xml_doc, "money_wnd:money_indicator:total_money",0, &m_money_amount);
	CUIXmlInit::InitStatic(xml_doc, "money_wnd:money_change", 0, &m_money_change);
	CUIXmlInit::InitScrollView(xml_doc, "money_wnd:money_bonus_list", 0, m_pBonusMoney);
	CGameFont* pF;
	u32 color;
	CUIXmlInit::InitFont(xml_doc, "money_wnd:money_bonus_list:font", 0, color, pF);
	m_pBonusMoney->SetTextAtrib(pF, color);
	m_money_change.SetVisible(false);
}

void CUIMoneyIndicator::SetMoneyAmount(LPCSTR money){
	m_money_amount.SetText(money);
}

void CUIMoneyIndicator::SetMoneyChange(LPCSTR money){
	m_money_change.SetText(money);
	m_money_change.SetVisible(true);
	m_pAnimChange->Reset();
}

void CUIMoneyIndicator::AddBonusMoney(KillMessageStruct& msg){
	m_pBonusMoney->AddLogMessage(msg);
}

void CUIMoneyIndicator::Update(){
	if (m_money_change.GetVisible())
        if (!m_pAnimChange->Done())
		{
			m_pAnimChange->Update();
			m_money_change.SetTextColor(subst_alpha(m_money_change.GetTextColor(), m_pAnimChange->GetColor()));
		}
		else
			m_money_change.SetVisible(false);

	CUIWindow::Update();
}

