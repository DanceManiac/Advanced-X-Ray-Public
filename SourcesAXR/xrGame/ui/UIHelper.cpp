////////////////////////////////////////////////////////////////////////////
//	Module 		: UIHelper.cpp
//	Created 	: 17.01.2008
//	Author		: Evgeniy Sokolov
//	Description : UI Helper class implementation
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UIHelper.h"
#include "UIXmlInit.h"

#include "UIStatic.h"
#include "UIProgressBar.h"
#include "UIProgressShape.h"
#include "UIFrameLineWnd.h"
#include "UIFrameWindow.h"
#include "UI3tButton.h"
#include "UICheckButton.h"
#include "UIHint.h"
#include "UIDragDropListEx.h"
#include "UIDragDropReferenceList.h"
#include "UIScrollView.h"
#include "UIListWnd.h"
#include "UIAnimatedStatic.h"
#include "UIEditBox.h"

#include "UIWndCallback.h"

CUIStatic* UIHelper::CreateStatic( CUIXml& xml, LPCSTR ui_path, CUIWindow* parent, bool critical )
{
    // If it's not critical element, then don't crash if it doesn't exist
    if (!critical && !xml.NavigateToNode(ui_path, 0))
        return nullptr;

	auto ui			= xr_new<CUIStatic>();
	if(parent)
	{
		parent->AttachChild	( ui );
		ui->SetAutoDelete	( true );
	}
	CUIXmlInit::InitStatic	( xml, ui_path, 0, ui );
	return ui;
}

CUITextWnd* UIHelper::CreateTextWnd( CUIXml& xml, LPCSTR ui_path, CUIWindow* parent, bool critical  )
{
	// If it's not critical element, then don't crash if it doesn't exist
	if (!critical && !xml.NavigateToNode(ui_path, 0))
		return nullptr;

	auto ui			= xr_new<CUITextWnd>();
	if(parent)
	{
		parent->AttachChild	( ui );
		ui->SetAutoDelete	( true );
	}
	CUIXmlInit::InitTextWnd	( xml, ui_path, 0, ui );
	return ui;
}

CUIEditBox* UIHelper::CreateEditBox( CUIXml& xml, LPCSTR ui_path, CUIWindow* parent, bool critical  )
{
	// If it's not critical element, then don't crash if it doesn't exist
	if (!critical && !xml.NavigateToNode(ui_path, 0))
		return nullptr;

	auto ui			= xr_new<CUIEditBox>();
	if(parent)
	{
		parent->AttachChild	( ui );
		ui->SetAutoDelete	( true );
	}
	CUIXmlInit::InitEditBox	( xml, ui_path, 0, ui );
	return ui;
}

CUIProgressBar* UIHelper::CreateProgressBar( CUIXml& xml, LPCSTR ui_path, CUIWindow* parent, bool critical )
{
	// If it's not critical element, then don't crash if it doesn't exist
	if (!critical && !xml.NavigateToNode(ui_path, 0))
		return nullptr;

	auto ui						= xr_new<CUIProgressBar>();
	parent->AttachChild			( ui );
	ui->SetAutoDelete			( true );
	CUIXmlInit::InitProgressBar ( xml, ui_path, 0, ui );
	return ui;
}

CUIProgressShape* UIHelper::CreateProgressShape( CUIXml& xml, LPCSTR ui_path, CUIWindow* parent, bool critical )
{
	// If it's not critical element, then don't crash if it doesn't exist
	if (!critical && !xml.NavigateToNode(ui_path, 0))
		return nullptr;

	auto ui = xr_new<CUIProgressShape>();
	parent->AttachChild			( ui );
	ui->SetAutoDelete			( true );
	CUIXmlInit::InitProgressShape ( xml, ui_path, 0, ui );
	return ui;
}

CUIFrameLineWnd* UIHelper::CreateFrameLine( CUIXml& xml, LPCSTR ui_path, CUIWindow* parent, bool critical )
{
	// If it's not critical element, then don't crash if it doesn't exist
	if (!critical && !xml.NavigateToNode(ui_path, 0))
		return nullptr;

	auto ui						= xr_new<CUIFrameLineWnd>();
	if(parent)
	{
		parent->AttachChild		( ui );
		ui->SetAutoDelete		( true );
	}
	CUIXmlInit::InitFrameLine	( xml, ui_path, 0, ui );
	return ui;
}

CUIFrameWindow* UIHelper::CreateFrameWindow( CUIXml& xml, LPCSTR ui_path, CUIWindow* parent, bool critical )
{
	// If it's not critical element, then don't crash if it doesn't exist
	if (!critical && !xml.NavigateToNode(ui_path, 0))
		return nullptr;

	auto ui						= xr_new<CUIFrameWindow>();
	if(parent)
	{
		parent->AttachChild		( ui );
		ui->SetAutoDelete		( true );
	}
	CUIXmlInit::InitFrameWindow	( xml, ui_path, 0, ui );
	return ui;
}

CUI3tButton* UIHelper::Create3tButton( CUIXml& xml, LPCSTR ui_path, CUIWindow* parent, bool critical )
{
	// If it's not critical element, then don't crash if it doesn't exist
	if (!critical && !xml.NavigateToNode(ui_path, 0))
		return nullptr;

	auto ui						= xr_new<CUI3tButton>();
	parent->AttachChild			( ui );
	ui->SetAutoDelete			( true );
	CUIXmlInit::Init3tButton	( xml, ui_path, 0, ui );
	return ui;
}

CUICheckButton* UIHelper::CreateCheck( CUIXml& xml, LPCSTR ui_path, CUIWindow* parent, bool critical )
{
	// If it's not critical element, then don't crash if it doesn't exist
	if (!critical && !xml.NavigateToNode(ui_path, 0))
		return nullptr;

	auto ui						= xr_new<CUICheckButton>();
	parent->AttachChild			( ui );
	ui->SetAutoDelete			( true );
	CUIXmlInit::InitCheck		( xml, ui_path, 0, ui );
	return ui;
}

UIHint* UIHelper::CreateHint( CUIXml& xml, LPCSTR ui_path)
{
	UIHint* ui					= xr_new<UIHint>();
	ui->SetAutoDelete			( true );
	ui->init_from_xml			( xml, ui_path );
	return ui;
}

CUIDragDropListEx* UIHelper::CreateDragDropListEx( CUIXml& xml, LPCSTR ui_path, CUIWindow* parent, bool critical )
{
	// If it's not critical element, then don't crash if it doesn't exist
	if (!critical && !xml.NavigateToNode(ui_path, 0))
		return nullptr;

	auto ui							= xr_new<CUIDragDropListEx>();
	parent->AttachChild				( ui );
	ui->SetAutoDelete				( true );
	CUIXmlInit::InitDragDropListEx	( xml, ui_path, 0, ui );
	return ui;
}

CUIDragDropReferenceList* UIHelper::CreateDragDropReferenceList( CUIXml& xml, LPCSTR ui_path, CUIWindow* parent, bool critical )
{
	// If it's not critical element, then don't crash if it doesn't exist
	if (!critical && !xml.NavigateToNode(ui_path, 0))
		return nullptr;

	auto ui							= xr_new<CUIDragDropReferenceList>();
	parent->AttachChild				( ui );
	ui->SetAutoDelete				( true );
	CUIXmlInit::InitDragDropListEx	( xml, ui_path, 0, ui );
	return ui;
}

CUIScrollView* UIHelper::CreateScrollView( CUIXml& xml, LPCSTR ui_path, CUIWindow* parent, bool critical )
{
	// If it's not critical element, then don't crash if it doesn't exist
	if (!critical && !xml.NavigateToNode(ui_path, 0))
		return nullptr;

	auto ui							= xr_new<CUIScrollView>();
	parent->AttachChild				(ui);
	ui->SetAutoDelete				(true);
	CUIXmlInit::InitScrollView		(xml, ui_path, 0, ui);
	return ui;
}

CUIListWnd* UIHelper::CreateListWnd(CUIXml& xml, LPCSTR ui_path, CUIWindow* parent, bool critical)
{
	// If it's not critical element, then don't crash if it doesn't exist
	if (!critical && !xml.NavigateToNode(ui_path, 0))
		return nullptr;

	auto ui							= xr_new<CUIListWnd>();
	parent->AttachChild				(ui);
	ui->SetAutoDelete				(true);
	CUIXmlInit::InitListWnd			(xml, ui_path, 0, ui);
	return ui;
}

CUIAnimatedStatic* UIHelper::CreateAnimatedStatic(CUIXml& xml, LPCSTR ui_path, CUIWindow* parent, bool critical)
{
	// If it's not critical element, then don't crash if it doesn't exist
	if (!critical && !xml.NavigateToNode(ui_path, 0))
		return nullptr;

	auto ui							= xr_new<CUIAnimatedStatic>();
	parent->AttachChild				(ui);
	ui->SetAutoDelete				(true);
	CUIXmlInit::InitAnimatedStatic	(xml, ui_path, 0, ui);
	return ui;
}
