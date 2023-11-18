//=============================================================================
//  Filename:   UIDiaryWnd.cpp
//	Created by Roman E. Marchenko, vortex@gsc-game.kiev.ua
//	Copyright 2004. GSC Game World
//	---------------------------------------------------------------------------
//  Encyclopedia window
//=============================================================================

#include "StdAfx.h"
#include "UIDiaryWnd.h"
#include "UIXmlInit.h"
#include "UIFrameWindow.h"
#include "UIFrameLineWnd.h"
#include "UIAnimatedStatic.h"
#include "UIHelper.h"
#include "UIStatic.h"
#include "UIListWnd.h"
#include "UIScrollView.h"
#include "UITreeViewItem.h"
#include "UIEncyclopediaArticleWnd.h"
#include "../encyclopedia_article.h"
#include "../alife_registry_wrappers.h"
#include "../actor.h"
#include "object_broker.h"

#define ENCYCLOPEDIA_DIALOG_XML "diary.xml"

CUIDiaryWnd::CUIDiaryWnd()
{
	prevArticlesCount	= 0;
}

CUIDiaryWnd::~CUIDiaryWnd()
{
	DeleteArticles();
}


void CUIDiaryWnd::Init()
{
	CUIXml				uiXml;
	uiXml.Load			(CONFIG_PATH, UI_PATH, ENCYCLOPEDIA_DIALOG_XML);

	CUIXmlInit			xml_init;

	xml_init.InitWindow		(uiXml, "main_wnd", 0, this);

	// Load xml data

	UIBackground = UIHelper::CreateFrameLine(uiXml, "background", this);

	UIDiaryIdxBkg = UIHelper::CreateFrameLine(uiXml, "left_background", this);
	UIDiaryIdxHeader = UIHelper::CreateStatic(uiXml, "left_caption", UIDiaryIdxBkg);

	UIDiaryInfoBkg = UIHelper::CreateFrameLine(uiXml, "right_background", this);
	UIDiaryInfoHeader = UIHelper::CreateStatic(uiXml, "right_caption", UIDiaryInfoBkg);

	UIArticleHeader = UIHelper::CreateStatic(uiXml, "article_header_static", UIDiaryInfoBkg);

	UIIdxList = xr_new <CUIListWnd>(); UIIdxList->SetAutoDelete(true);
	UIDiaryIdxBkg->AttachChild(UIIdxList);
	CUIXmlInit::InitListWnd(uiXml, "idx_list", 0, UIIdxList);
	UIIdxList->SetMessageTarget(this);

	UIInfoList = xr_new <CUIScrollView>();
	UIInfoList->SetAutoDelete(true);
	UIDiaryInfoBkg->AttachChild(UIInfoList);
	CUIXmlInit::InitScrollView(uiXml, "info_list", 0, UIInfoList);

	CUIXmlInit::InitFont(uiXml, "tree_item_font", 0, m_uTreeItemColor, m_pTreeItemFont);
	CUIXmlInit::InitFont(uiXml, "tree_root_font", 0, m_uTreeRootColor, m_pTreeRootFont);

	R_ASSERT(m_pTreeItemFont);
	R_ASSERT(m_pTreeRootFont);
}

#include "../string_table.h"
void CUIDiaryWnd::SendMessage(CUIWindow *pWnd, s16 msg, void* pData)
{
	if (UIIdxList == pWnd && LIST_ITEM_CLICKED == msg)
	{
		CUITreeViewItem *pTVItem = (CUITreeViewItem*)(pData);
		R_ASSERT		(pTVItem);
		
		if( pTVItem->vSubItems.size() )
		{
			CEncyclopediaArticle* A = m_ArticlesDB[pTVItem->vSubItems[0]->GetValue()];

			xr_string caption		= "# ";
			std::string str			(A->data()->group.c_str());

			std::vector<std::string>splitParts;
			if(!splitParts.empty())
				splitParts.clear();
			size_t pos				= str.find("/");
			while (pos != std::string::npos)
			{
				splitParts.push_back(str.substr(0, pos));
				str.erase			(0, pos + 1);
				pos					= str.find("/");
			}
			splitParts.push_back(str);
			for (size_t i = 0; i < splitParts.size(); i++)
			{
				const std::string& part = splitParts[i];
				string4096 pp;
				xr_sprintf(pp, "%s", part.c_str());
				caption += CStringTable().translate(pp).c_str();
				
				if (i + 1 != splitParts.size())
					caption += "/";
			}

			UIDiaryInfoHeader->TextItemControl()->SetText(caption.c_str());
			//UIArticleHeader->SetText(caption.c_str());
			SetCurrentArtice		(NULL);
		}else
		{
			int idx = pTVItem->GetValue();
			if (idx==-1) return;
			CEncyclopediaArticle* A = m_ArticlesDB[idx];
			xr_string caption		= "# ";
			std::string str			(A->data()->group.c_str());

			std::vector<std::string>splitParts;
			if(!splitParts.empty())
				splitParts.clear();
			size_t pos				= str.find("/");
			while (pos != std::string::npos)
			{
				splitParts.push_back(str.substr(0, pos));
				str.erase			(0, pos + 1);
				pos					= str.find("/");
			}
			splitParts.push_back(str);
			for (const std::string& part : splitParts)
			{
				string4096				pp;
				xr_sprintf				(pp, "%s", part);
				caption					+= CStringTable().translate(pp).c_str();
				caption					+= "/";
			}

			caption					+= CStringTable().translate(A->data()->name).c_str();

			UIDiaryInfoHeader->TextItemControl()->SetText(caption.c_str());
			SetCurrentArtice		(pTVItem);
			UIArticleHeader->TextItemControl()->SetText(CStringTable().translate(A->data()->name).c_str());
		}
	}

	inherited::SendMessage(pWnd, msg, pData);
}

void CUIDiaryWnd::Draw()
{
	
	if(	m_flags.test(eNeedReload ))
	{
		if(Actor() && Actor()->encyclopedia_registry->registry().objects_ptr() && Actor()->encyclopedia_registry->registry().objects_ptr()->size() > prevArticlesCount)
		{
			ARTICLE_VECTOR::const_iterator it = Actor()->encyclopedia_registry->registry().objects_ptr()->begin();
			std::advance(it, prevArticlesCount);
			for(; it != Actor()->encyclopedia_registry->registry().objects_ptr()->end(); it++)
			{
				if (ARTICLE_DATA::eJournalArticle == it->article_type)
				{
					AddArticle(it->article_id, it->readed);
				}
			}
			prevArticlesCount = Actor()->encyclopedia_registry->registry().objects_ptr()->size();
		}
		
		m_flags.set(eNeedReload, FALSE);
	}

	inherited::Draw();
}

void CUIDiaryWnd::ReloadArticles()
{
	m_flags.set(eNeedReload, TRUE);
}


void CUIDiaryWnd::Show(bool status)
{
	if (status)
		ReloadArticles();

	inherited::Show(status);
}


bool CUIDiaryWnd::HasArticle(shared_str id)
{
	ReloadArticles();
	for(std::size_t i = 0; i<m_ArticlesDB.size(); ++i)
	{
		if(m_ArticlesDB[i]->Id() == id) return true;
	}
	return false;
}


void CUIDiaryWnd::DeleteArticles()
{
	UIIdxList->RemoveAll();
	delete_data			(m_ArticlesDB);
}

void CUIDiaryWnd::SetCurrentArtice(CUITreeViewItem *pTVItem)
{
	UIInfoList->ScrollToBegin();
	UIInfoList->Clear();

	if(!pTVItem) return;

	// для начала проверим, что нажатый элемент не рутовый
	if (!pTVItem->IsRoot())
	{

		CUIEncyclopediaArticleWnd*	article_info = xr_new<CUIEncyclopediaArticleWnd>();
		article_info->Init			("encyclopedia_item.xml","encyclopedia_wnd:objective_item");
		article_info->SetArticle	(m_ArticlesDB[pTVItem->GetValue()]);
		UIInfoList->AddWindow		(article_info, true);

		// Пометим как прочитанную
		if (!pTVItem->IsArticleReaded())
		{
			if(Actor()->encyclopedia_registry->registry().objects_ptr())
			{
				for(ARTICLE_VECTOR::iterator it = Actor()->encyclopedia_registry->registry().objects().begin();
					it != Actor()->encyclopedia_registry->registry().objects().end(); it++)
				{
					if (ARTICLE_DATA::eJournalArticle == it->article_type &&
						m_ArticlesDB[pTVItem->GetValue()]->Id() == it->article_id)
					{
						it->readed = true;
						break;
					}
				}
			}
		}
	}
}

void CUIDiaryWnd::AddArticle(shared_str article_id, bool bReaded)
{
	for(std::size_t i = 0; i<m_ArticlesDB.size(); i++)
	{
		if(m_ArticlesDB[i]->Id() == article_id) return;
	}

	// Добавляем элемент
	m_ArticlesDB.resize(m_ArticlesDB.size() + 1);
	CEncyclopediaArticle*& a = m_ArticlesDB.back();
	a = xr_new<CEncyclopediaArticle>();
	a->Load(article_id);


	// Теперь создаем иерархию вещи по заданному пути

	CreateTreeBranch(a->data()->group, a->data()->name, UIIdxList, m_ArticlesDB.size() - 1, 
		m_pTreeRootFont, m_uTreeRootColor, m_pTreeItemFont, m_uTreeItemColor, bReaded);
}

void CUIDiaryWnd::ResetAll()
{
	inherited::ResetAll	();
	ReloadArticles		();
}
