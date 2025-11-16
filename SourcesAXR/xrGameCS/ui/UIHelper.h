////////////////////////////////////////////////////////////////////////////
//	Module 		: UIHelper.h
//	Created 	: 17.01.2008
//	Author		: Evgeniy Sokolov
//	Description : UI Helper class
////////////////////////////////////////////////////////////////////////////

#include "UIWindow.h"

class CUIXml;
class CUIStatic;
class CUIProgressBar;
class CUIProgressShape;
class CUIFrameLineWnd;
class CUIFrameWindow;
class CUI3tButton;
class CUI3tButtonEx;
class CUICheckButton;
class UIHint;
class CUIDragDropListEx;
class CUIDragDropReferenceList;
class CUIScrollView;
class CUIListWnd;
class CUIAnimatedStatic;

class UIHelper
{
public:
	UIHelper		() {};
	~UIHelper		() {};

	static	CUIStatic*			CreateStatic		( CUIXml& xml, LPCSTR ui_path, CUIWindow* parent, bool critical = true );
	static	CUIProgressBar*		CreateProgressBar	( CUIXml& xml, LPCSTR ui_path, CUIWindow* parent, bool critical = true );
	static	CUIProgressShape*	CreateProgressShape	( CUIXml& xml, LPCSTR ui_path, CUIWindow* parent, bool critical = true );
	static	CUIFrameLineWnd*	CreateFrameLine		( CUIXml& xml, LPCSTR ui_path, CUIWindow* parent, bool critical = true );
	static	CUIFrameWindow*		CreateFrameWindow	( CUIXml& xml, LPCSTR ui_path, CUIWindow* parent, bool critical = true );
	static	CUI3tButton*		Create3tButton		( CUIXml& xml, LPCSTR ui_path, CUIWindow* parent, bool critical = true );
	static	CUI3tButtonEx*		Create3tButtonEx	( CUIXml& xml, LPCSTR ui_path, CUIWindow* parent, bool critical = true );
	static	CUICheckButton*		CreateCheck			( CUIXml& xml, LPCSTR ui_path, CUIWindow* parent, bool critical = true );

	static	UIHint*				CreateHint			( CUIXml& xml, LPCSTR ui_path /*, CUIWindow* parent*/ );
	static	CUIDragDropListEx*	CreateDragDropListEx( CUIXml& xml, LPCSTR ui_path, CUIWindow* parent, bool critical = true  );
	static	CUIDragDropReferenceList*	CreateDragDropReferenceList( CUIXml& xml, LPCSTR ui_path, CUIWindow* parent, bool critical = true  );
	static	CUIScrollView*		CreateScrollView	( CUIXml& xml, LPCSTR ui_path, CUIWindow* parent, bool critical = true );
	static	CUIListWnd*			CreateListWnd		( CUIXml& xml, LPCSTR ui_path, CUIWindow* parent, bool critical = true );
	static	CUIAnimatedStatic*	CreateAnimatedStatic( CUIXml& xml, LPCSTR ui_path, CUIWindow* parent, bool critical = true );

}; // class UIHelper
