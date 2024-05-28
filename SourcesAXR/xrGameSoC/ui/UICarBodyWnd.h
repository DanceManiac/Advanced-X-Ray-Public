#pragma once

#include "UIDialogWnd.h"
#include "UIEditBox.h"
#include "UIWndCallback.h"
#include "inventory_space.h"

class CUIDragDropListEx;
class CUIItemInfo;
class CUICharacterInfo;
class CUIPropertiesBox;
class CUI3tButton;
class CUICellItem;
class CInventoryBox;
class CInventoryOwner;

class CUICarBodyWnd: public CUIDialogWnd, public CUIWndCallback
{
private:
	typedef CUIDialogWnd	inherited;
	bool					m_b_need_update;
public:
							CUICarBodyWnd				();
	virtual					~CUICarBodyWnd				();

	virtual void			Init						();
	void					InitCallbacks				();
	virtual bool			StopAnyMove					(){return true;}

	virtual void			SendMessage					(CUIWindow *pWnd, s16 msg, void *pData);

	void					InitCarBody					(CInventoryOwner* pOurInv, CInventoryOwner* pOthersInv);
	void					InitCarBody					(CInventoryOwner* pOur, CInventoryBox* pInvBox);
	virtual void			Draw						();
	virtual void			Update						();
		
	virtual void			Show						();
	virtual void			Hide						();

	void					DisableAll					();
	void					EnableAll					();
	virtual bool			OnKeyboardAction					(int dik, EUIMessages keyboard_action);

	void					UpdateLists_delayed			();

protected:
	CInventoryOwner*		m_pOurObject;

	CInventoryOwner*		m_pOthersObject;
	CInventoryBox*			m_pInventoryBox;

	CUIDragDropListEx*		m_pUIOurBagList;
	CUIDragDropListEx*		m_pUIOthersBagList;

	CUIStatic*				m_pUIStaticTop;
	CUIStatic*				m_pUIStaticBottom;

	CUIFrameWindow*			m_pUIDescWnd;
	CUIStatic*				m_pUIStaticDesc;
	CUIItemInfo*			m_pUIItemInfo;

	CUIStatic*				m_pUIOurBagWnd;
	CUIStatic*				m_pUIOthersBagWnd;

	//информация о персонажах 
	CUIStatic*				m_pUIOurIcon;
	CUIStatic*				m_pUIOthersIcon;
	CUICharacterInfo*		m_pUICharacterInfoLeft;
	CUICharacterInfo*		m_pUICharacterInfoRight;
	CUIPropertiesBox*		m_pUIPropertiesBox;
	CUI3tButton*			m_pUITakeAll;

	CUICellItem*			m_pCurrentCellItem;

	void					UpdateLists					();

	void					EatItem						(CUICellItem* itm);

	void					PropertiesBoxForUsing		(PIItem item, bool& b_show);
	void					PropertiesBoxForWeapon		(CUICellItem* cell_item, PIItem item, bool& b_show);
	void					PropertiesBoxForDrop		(CUICellItem* cell_item, PIItem item, bool& b_show);
	void					ActivatePropertiesBox		();
	void		xr_stdcall	ProcessPropertiesBoxClicked	(CUIWindow* w, void* d);
	void		xr_stdcall	OnBtnTakeAll				(CUIWindow* w, void* d);

	bool					ToOurBag					();
	bool					ToOthersBag					();
	
	void					SetCurrentItem				(CUICellItem* itm);
	CUICellItem*			CurrentItem					();
	PIItem					CurrentIItem				();

	// Взять все
	void					TakeAll						();


	bool		xr_stdcall	OnItemDrop					(CUICellItem* itm);
	bool		xr_stdcall	OnItemStartDrag				(CUICellItem* itm);
	bool		xr_stdcall	OnItemDbClick				(CUICellItem* itm);
	bool		xr_stdcall	OnItemSelected				(CUICellItem* itm);
	bool		xr_stdcall	OnItemRButtonClick			(CUICellItem* itm);

	bool					TransferItem				(PIItem itm, CInventoryOwner* owner_from, CInventoryOwner* owner_to, bool b_check);
	void					move_item					(u16 from_id, u16 to_id, u16 what_id);

	void					BindDragDropListEnents		(CUIDragDropListEx* lst);

	void					DetachAddon					(LPCSTR addon_name);
	
	void					DropCurrentItem				(bool b_all);
	void					ColorizeItem				(CUICellItem* itm);

};