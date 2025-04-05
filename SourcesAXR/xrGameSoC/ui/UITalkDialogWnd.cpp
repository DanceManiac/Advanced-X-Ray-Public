#include "stdafx.h"
#include "UITalkDialogWnd.h"

#include "xrUIXmlParser.h"
#include "UIXmlInit.h"
#include "UIScrollView.h"
#include "UI3tButton.h"
#include "UIInventoryUtilities.h"
#include "UIHelper.h"

#include "../UI.h"
#include "../game_news.h"
#include "../level.h"
#include "../actor.h"
#include "../alife_registry_wrappers.h"
#include "dinput.h"

#define				TALK_XML				"talk.xml"
#define				TRADE_CHARACTER_XML		"trade_character.xml"

//////////////////////////////////////////////////////////////////////////

CUITalkDialogWnd::CUITalkDialogWnd()
	:	m_pNameTextFont		(nullptr)
{
	m_ClickedQuestionID = "";
	m_pOurInvOwner = nullptr;
	m_pOthersInvOwner = nullptr;
}

CUITalkDialogWnd::~CUITalkDialogWnd()
{
	xr_delete(m_uiXml);
}

void CUITalkDialogWnd::Init(float x, float y, float width, float height)
{
	m_uiXml						= xr_new<CUIXml>();
	m_uiXml->Load				(CONFIG_PATH, UI_PATH, TALK_XML);
	CUIXmlInit					ml_init;

	inherited::Init				(x, y, width, height);

	AttachChild					(&UIStaticTop);
	CUIXmlInit::InitStatic		(*m_uiXml, "top_background", 0, &UIStaticTop);
	AttachChild					(&UIStaticBottom);
	CUIXmlInit::InitStatic		(*m_uiXml, "bottom_background", 0, &UIStaticBottom);

	//иконки с изображение нас и партнера по торговле
	AttachChild					(&UIOurIcon);
	CUIXmlInit::InitStatic		(*m_uiXml, "left_character_icon", 0, &UIOurIcon);
	AttachChild					(&UIOthersIcon);
	CUIXmlInit::InitStatic		(*m_uiXml, "right_character_icon", 0, &UIOthersIcon);
	UIOurIcon.AttachChild		(&UICharacterInfoLeft);
	UICharacterInfoLeft.Init	(0.0f, 0.0f, UIOurIcon.GetWidth(), UIOurIcon.GetHeight(), TRADE_CHARACTER_XML);
	UIOthersIcon.AttachChild	(&UICharacterInfoRight);
	UICharacterInfoRight.Init	(0.0f, 0.0f, UIOthersIcon.GetWidth(), UIOthersIcon.GetHeight(), TRADE_CHARACTER_XML);

	//основной фрейм диалога
	AttachChild					(&UIDialogFrame);
	CUIXmlInit::InitFrameLine	(*m_uiXml, "frame_line_window", 0, &UIDialogFrame);
//	UIDialogFrame.UITitleText.SetElipsis(CUIStatic::eepEnd, 10);
	// Фрейм с нащими фразами
	AttachChild					(&UIOurPhrasesFrame);
	CUIXmlInit::InitFrameLine	(*m_uiXml, "frame_line_window", 1, &UIOurPhrasesFrame);
//	UIOurPhrasesFrame.UITitleText.SetElipsis(CUIStatic::eepEnd, 10);

	//Ответы
	UIAnswersList				= xr_new<CUIScrollView>();
	UIAnswersList->SetAutoDelete(true);
	UIDialogFrame.AttachChild	(UIAnswersList);
	CUIXmlInit::InitScrollView	(*m_uiXml, "answers_list", 0, UIAnswersList);
	UIAnswersList->SetWindowName("---UIAnswersList");

	//Вопросы
	UIQuestionsList				= xr_new<CUIScrollView>();
	UIQuestionsList->SetAutoDelete(true);
	UIOurPhrasesFrame.AttachChild(UIQuestionsList);
	CUIXmlInit::InitScrollView	(*m_uiXml, "questions_list", 0, UIQuestionsList);
	UIQuestionsList->SetWindowName("---UIQuestionsList");


	//кнопка перехода в режим торговли
	AttachChild					(&UIToTradeButton);
	CUIXmlInit::Init3tButton	(*m_uiXml, "button", 0, &UIToTradeButton);
	UIToTradeButton.SetWindowName("trade_btn");

	//Элементы автоматического добавления
	CUIXmlInit::InitAutoStatic	(*m_uiXml, "auto_static", this);

	// шрифт для индикации имени персонажа в окне разговора
	CUIXmlInit::InitFont		(*m_uiXml, "font", 0, m_iNameTextColor, m_pNameTextFont);

	CGameFont * pFont			= nullptr;
	CUIXmlInit::InitFont		(*m_uiXml, "font", 1, m_uOurReplicsColor, pFont);


	SetWindowName				("----CUITalkDialogWnd");

	Register					(&UIToTradeButton);
	AddCallback					("question_item",LIST_ITEM_CLICKED,CUIWndCallback::void_function(this, &CUITalkDialogWnd::OnQuestionClicked));
	AddCallback					("trade_btn",BUTTON_CLICKED,CUIWndCallback::void_function(this, &CUITalkDialogWnd::OnTradeClicked));
}

void CUITalkDialogWnd::Show(bool status) // Hrust: made different func for disabling drawing without "ui_talk_hide" message
{
	inherited::Show(status);
	inherited::Enable(status);
}

void CUITalkDialogWnd::Show()
{
	InventoryUtilities::SendInfoToActor				("ui_talk");
	inherited::Show(true);
	inherited::Enable(true);

	ResetAll();
}

void CUITalkDialogWnd::Hide()
{
	InventoryUtilities::SendInfoToActor				("ui_talk_hide");
	inherited::Show(false);
	inherited::Enable(false);
}

void CUITalkDialogWnd::OnQuestionClicked(CUIWindow* w, void*)
{
		m_ClickedQuestionID = ((CUIQuestionItem*)w)->m_s_value;
		GetMessageTarget()->SendMessage(this, TALK_DIALOG_QUESTION_CLICKED);
}

void CUITalkDialogWnd::OnTradeClicked(CUIWindow* w, void*)
{
		GetTop()->SendMessage(this, TALK_DIALOG_TRADE_BUTTON_CLICKED);
}

//пересылаем сообщение родительскому окну для обработки
//и фильтруем если оно пришло от нашего дочернего окна
void CUITalkDialogWnd::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
	CUIWndCallback::OnEvent(pWnd, msg, pData);
}

void CUITalkDialogWnd::ClearAll()
{
	UIAnswersList->Clear	();
	ClearQuestions			();
}

void CUITalkDialogWnd::ClearQuestions()
{
	UIQuestionsList->Clear();
}


void CUITalkDialogWnd::AddQuestion(LPCSTR str, LPCSTR value, int number, SPhraseInfo phInfo)
{
	CUIQuestionItem* itm			= xr_new<CUIQuestionItem>(m_uiXml,"question_item");
	itm->Init						(value, str);

	++number; // zero-based index
	string16 buff;
	xr_sprintf(buff, "%d.", number);
	if (itm->m_num_text)
		itm->m_num_text->SetText(buff);

	if (number > 9)
		itm->m_text->SetTextX(itm->m_fOffset);

	if (number < 10)
		itm->m_text->SetAccelerator(DIK_ESCAPE + number, 0);

	if (phInfo.bFinalizer)
	{
		itm->m_text->SetAccelerator(kQUIT, 2);
		itm->m_text->SetAccelerator(kUSE, 3);
	}

	itm->SetWindowName				("question_item");
	UIQuestionsList->AddWindow		(itm, true);
	Register						(itm);
}

void CUITalkDialogWnd::AddAnswer(LPCSTR SpeakerName, LPCSTR str, bool bActor)
{
	CUIAnswerItem* itm				= xr_new<CUIAnswerItem>(m_uiXml,bActor?"actor_answer_item":"other_answer_item");
	itm->Init						(str, SpeakerName);
	UIAnswersList->AddWindow		(itm, true);
	UIAnswersList->ScrollToEnd		();

	GAME_NEWS_DATA					news_data;
	xr_string						str_;
	str_							= SpeakerName;
	str_							+= " >> ";
	str_							+= str;

	news_data.m_type				= GAME_NEWS_DATA::eTalk;
	news_data.news_text				= str_.c_str();
//.	news_data.texture_name			= "ui\\ui_icons_npc";
	CUICharacterInfo& ci			= bActor?UICharacterInfoLeft:UICharacterInfoRight; 
	
	m_pOurInvOwner = smart_cast<CInventoryOwner*>(Actor());
	m_pOthersInvOwner = Actor()->GetTalkPartner();

	auto owner = bActor ? m_pOurInvOwner->IconName() : (m_pOthersInvOwner ? m_pOthersInvOwner->IconName() : m_pOurInvOwner->IconName());

	news_data.texture_name			= (bActor) ? owner : ci.IconName();
	news_data.tex_rect				= ci.UIIcon().GetUIStaticItem().GetOriginalRect();
	news_data.tex_rect.x2			= news_data.tex_rect.width();
	news_data.tex_rect.y2			= news_data.tex_rect.height();
	news_data.receive_time			= Level().GetGameTime();

	Actor()->game_news_registry->registry().objects().push_back(news_data);
}

void CUITalkDialogWnd::AddIconedAnswer(LPCSTR text, LPCSTR texture_name, Frect texture_rect, LPCSTR templ_name)
{
	CUIAnswerItemIconed* itm				= xr_new<CUIAnswerItemIconed>(m_uiXml,templ_name);
	itm->Init								(text, texture_name, texture_rect);
	UIAnswersList->AddWindow				(itm, true);
	UIAnswersList->ScrollToEnd				();

}
void CUITalkDialogWnd::SetOsoznanieMode(bool b)
{
	UIOurIcon.Show		(!b);
	UIOthersIcon.Show	(!b);

	UIAnswersList->Show	(!b);
	UIDialogFrame.Show (!b);

	UIToTradeButton.Show(!b);
}


void CUIQuestionItem::SendMessage				(CUIWindow* pWnd, s16 msg, void* pData)
{
	CUIWndCallback::OnEvent(pWnd, msg, pData);
}

CUIQuestionItem::CUIQuestionItem			(CUIXml* xml_doc, LPCSTR path)
{
	m_text							= xr_new<CUI3tButton>();
	m_text->SetAutoDelete			(true);
	AttachChild						(m_text);

	string512						str;
	CUIXmlInit						xml_init;

	strcpy							(str,path);
	xml_init.InitWindow				(*xml_doc, str, 0, this);

	m_min_height					= xml_doc->ReadAttribFlt(path,0,"min_height",15.0f);

	strconcat						(sizeof(str),str,path,":content_text");
	xml_init.Init3tButton			(*xml_doc, str, 0, m_text);
	m_fOffset = xml_doc->ReadAttribFlt(str, 0, "offset", 0);

	Register						(m_text);
	m_text->SetWindowName			("text_button");
	AddCallback						("text_button",BUTTON_CLICKED,CUIWndCallback::void_function(this, &CUIQuestionItem::OnTextClicked));

	strconcat						(sizeof(str), str, path, ":num_text");
	m_num_text						= UIHelper::CreateStatic(*xml_doc, str, this, false);
}

void CUIQuestionItem::Init			(LPCSTR val, LPCSTR text)
{
	m_s_value						= val;
	m_text->SetText					(text);
	m_text->AdjustHeightToText		();
	float new_h						= _max(m_min_height, m_text->GetWndPos().y+m_text->GetHeight());
	SetHeight						(new_h);
}

void	CUIQuestionItem::OnTextClicked(CUIWindow* w, void*)
{
	GetMessageTarget()->SendMessage(this, LIST_ITEM_CLICKED, (void*)this);
}


CUIAnswerItem::CUIAnswerItem			(CUIXml* xml_doc, LPCSTR path)
{
	m_text							= xr_new<CUIStatic>();m_text->SetAutoDelete(true);
	m_name							= xr_new<CUIStatic>();m_name->SetAutoDelete(true);
	AttachChild						(m_text);
	AttachChild						(m_name);

	string512						str;
	CUIXmlInit						xml_init;

	strcpy							(str,path);
	xml_init.InitWindow				(*xml_doc, str, 0, this);

	m_min_height					= xml_doc->ReadAttribFlt(path,0,"min_height",15.0f);
	m_bottom_footer					= xml_doc->ReadAttribFlt(path,0,"bottom_footer",0.0f);
	strconcat						(sizeof(str),str,path,":content_text");
	xml_init.InitStatic				(*xml_doc, str, 0, m_text);

	strconcat						(sizeof(str),str,path,":name_caption");
	xml_init.InitStatic				(*xml_doc, str, 0, m_name);
	SetAutoDelete					(true);
}

void CUIAnswerItem::Init			(LPCSTR text, LPCSTR name)
{
	m_name->SetText					(name);
	m_text->SetText					(text);
	m_text->AdjustHeightToText		();
	float new_h						= _max(m_min_height, m_text->GetWndPos().y+m_text->GetHeight());
	new_h							+= m_bottom_footer;
	SetHeight						(new_h);
}

CUIAnswerItemIconed::CUIAnswerItemIconed		(CUIXml* xml_doc, LPCSTR path)
:CUIAnswerItem(xml_doc, path)
{
	m_icon							= xr_new<CUIStatic>();m_icon->SetAutoDelete(true);
	AttachChild						(m_icon);

	string512						str;
	CUIXmlInit						xml_init;

	strconcat						(sizeof(str),str,path,":msg_icon");
	xml_init.InitStatic				(*xml_doc, str, 0, m_icon);
}

void CUIAnswerItemIconed::Init		(LPCSTR text, LPCSTR texture_name, Frect texture_rect)
{
	inherited::Init					(text,"");
	m_icon->InitTextureEx			(texture_name, "hud\\default");
	m_icon->GetUIStaticItem().SetOriginalRect(texture_rect.x1,texture_rect.y1,texture_rect.x2,texture_rect.y2);
	m_icon->TextureOn				();
	m_icon->SetStretchTexture		(true);

}

