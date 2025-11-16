#pragma once

#include "embedded_editor/embedded_editor_ui.h"

class CUIDialogWnd;
class CUIWindow;

class dlgItem{
public:
	dlgItem			(CUIWindow* pWnd);
	CUIWindow*		wnd;
	bool			enabled;
	bool operator < (const dlgItem& itm) const;
};

class recvItem{
public:
	enum{	eCrosshair		= (1<<0),
			eIndicators		= (1<<1)};
	recvItem		(CUIDialogWnd*);
	CUIDialogWnd*	m_item;
	Flags8			m_flags;
};

class CDialogHolder :public pureFrame, public CUIDebuggable
{
	//dialogs
	xr_vector<recvItem>										m_input_receivers;
	xr_vector<dlgItem>										m_dialogsToRender;


	void					StartMenu						(CUIDialogWnd* pDialog, bool bDoHideIndicators);
	void					StopMenu						(CUIDialogWnd* pDialog);
protected:
	void					DoRenderDialogs					();
	void					CleanInternals					();
public:
	CDialogHolder					();
	virtual					~CDialogHolder					();

	//dialogs
	CUIDialogWnd*			MainInputReceiver				();
	virtual void			StartStopMenu					(CUIDialogWnd* pDialog, bool bDoHideIndicators);
	void					AddDialogToRender				(CUIWindow* pDialog);
	void					RemoveDialogToRender			(CUIWindow* pDialog);
	virtual void			OnFrame							();
	virtual bool			UseIndicators					()						{return true;}
	virtual void			StartDialog						(CUIDialogWnd* pDialog, bool bDoHideIndicators);
	virtual void			StopDialog						(CUIDialogWnd* pDialog);
	void					SetMainInputReceiver			(CUIDialogWnd* ir, bool _find_remove);

	pcstr					GetDebugType					() override { return typeid(*this).name(); }
	bool					FillDebugTree					(const CUIDebugState& debugState) override;
	void					FillDebugInfo					() override;
};
