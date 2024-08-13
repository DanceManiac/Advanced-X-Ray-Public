#pragma once

class CInventory;

#include "UIDialogWnd.h"
#include "UIStatic.h"

#include "UIProgressBar.h"

#include "UIPropertiesBox.h"
#include "UIOutfitSlot.h"

#include "UIActorStats.h"
#include "UIItemInfo.h"
#include "UIWndCallback.h"
#include "inventory_space.h"

class CArtefact;
class CUI3tButton;
class CUIDragDropListEx;
class CUICellItem;


enum EDDListType {
	iInvalid,
	iwSlot,
	iwBag,
	iwBelt,

	iListTypeMax
};

class CUIInventoryWnd : public CUIDialogWnd, public CUIWndCallback
{
private:
	typedef CUIDialogWnd	inherited;
	bool					m_b_need_reinit;
public:
							CUIInventoryWnd				();
	virtual					~CUIInventoryWnd			();

	virtual void			Init						();
			void			InitHighlights				(CUIXml& uiXml);

	void					InitCallbacks				();

	void					InitInventory				();
	void					InitInventory_delayed		();
	virtual bool			StopAnyMove					()					{return false;}

	virtual void			SendMessage					(CUIWindow *pWnd, s16 msg, void *pData);
	virtual bool			OnMouseAction				(float x, float y, EUIMessages mouse_action);
	virtual bool			OnKeyboardAction			(int dik, EUIMessages keyboard_action);


	IC CInventory*			GetInventory				()					{return m_pInv;}

	virtual void			Update						();
	virtual void			Draw						();

	virtual void			Show						();
	virtual void			Hide						();

	void					AddItemToBag				(PIItem pItem);

protected:
	enum eInventorySndAction{	eInvSndOpen	=0,
								eInvSndClose,
								eInvItemToSlot,
								eInvItemToBelt,
								eInvItemToRuck,
								eInvProperties,
								eInvDropItem,
								eInvAttachAddon,
								eInvDetachAddon,
								eInvItemUse,
								eInvSndMax};

	ref_sound					sounds					[eInvSndMax];
	void						PlaySnd					(eInventorySndAction a);

	CUIStatic					UIBeltSlots;
	CUIStatic					UIBack;
	CUIStatic*					UIRankFrame;
	CUIStatic*					UIRank;

	CUIStatic					UIBagWnd;
	CUIStatic					UIMoneyWnd;
	CUIStatic					UIDescrWnd;
	CUIFrameWindow				UIPersonalWnd;

	CUI3tButton*				UIExitButton;

	CUIStatic					UIStaticBottom;
	CUIStatic					UIStaticTime;
	CUIStatic					UIStaticTimeString;

	CUIStatic					UIStaticPersonal;

	CUIStatic*					m_ActorInvCapacityInfo;
	CUIStatic*					m_ActorInvFullness;
	CUIStatic*					m_ActorInvCapacity;
		
	CUIDragDropListEx*			m_pUIBagList;
	CUIDragDropListEx*			m_pUIBeltList;
	CUIDragDropListEx*			m_pUIPistolList;
	CUIDragDropListEx*			m_pUIAutomaticList;
	CUIOutfitDragDropList*		m_pUIOutfitList;

	CUIStatic*					m_InvSlot2Highlight;
	CUIStatic*					m_InvSlot3Highlight;
	CUIStatic*					m_OutfitSlotHighlight;
	CUIStatic*					m_DetectorSlotHighlight;
	//CUIStatic*					m_QuickSlotsHighlight[4];
	xr_vector<CUIStatic*>		m_ArtefactSlotsHighlight;
	CUIStatic*					m_KnifeSlotHighlight;
	CUIStatic*					m_BinocularSlotHighlight;
	CUIStatic*					m_TorchSlotHighlight;
	CUIStatic*					m_BackpackSlotHighlight;
	CUIStatic*					m_PantsSlotHighlight;
	CUIStatic*					m_PdaSlotHighlight;

	bool						m_highlight_clear;

	// M.F.S. Team: New Slots
	CUIDragDropListEx*			m_pUIKnifeList;
	CUIDragDropListEx*			m_pUITorchList;
	CUIDragDropListEx*			m_pUIBinocularList;
	CUIDragDropListEx*			m_pUIPdaList;
	CUIDragDropListEx*			m_pUIDosimeterList;
	CUIDragDropListEx*			m_pUIBackpackList;
	CUIDragDropListEx*			m_pUIPantsList;

	void						ClearAllLists				();
	void						BindDragDropListEnents		(CUIDragDropListEx* lst);

	void						clear_highlight_lists		();
	void						highlight_item_slot			(CUICellItem* cell_item);
	void						set_highlight_item			(CUICellItem* cell_item);
	void						highlight_armament			(PIItem item, CUIDragDropListEx* ddlist);
	void						highlight_ammo_for_weapon	(PIItem weapon_item, CUIDragDropListEx* ddlist);
	void						highlight_weapons_for_ammo	(PIItem ammo_item, CUIDragDropListEx* ddlist);
	bool						highlight_addons_for_weapon	(PIItem weapon_item, CUICellItem* ci);
	void						highlight_weapons_for_addon	(PIItem addon_item, CUIDragDropListEx* ddlist);
	
	EDDListType					GetType						(CUIDragDropListEx* l);
	CUIDragDropListEx*			GetSlotList					(u32 slot_idx);

	bool		xr_stdcall		OnItemDrop					(CUICellItem* itm);
	bool		xr_stdcall		OnItemStartDrag				(CUICellItem* itm);
	bool		xr_stdcall		OnItemDbClick				(CUICellItem* itm);
	bool		xr_stdcall		OnItemSelected				(CUICellItem* itm);
	bool		xr_stdcall		OnItemRButtonClick			(CUICellItem* itm);
	bool		xr_stdcall		OnItemFocusReceive			(CUICellItem* itm);
	bool		xr_stdcall		OnItemFocusLost				(CUICellItem* itm);
	bool		xr_stdcall		OnItemFocusedUpdate			(CUICellItem* itm);


	CUIStatic					UIProgressBack;
	CUIStatic					UIProgressBack_rank;
	CUIProgressBar				UIProgressBarHealth;	
	CUIProgressBar				UIProgressBarPsyHealth;
	CUIProgressBar				UIProgressBarRadiation;
	CUIProgressBar				UIProgressBarRank;

	CUIPropertiesBox*			UIPropertiesBox;
	
	//информация о персонаже
	CUIActorStats				UIActorStats;
	CUIItemInfo					UIItemInfo;

	CInventory*					m_pInv;

	CUICellItem*				m_pCurrentCellItem;

	bool						DropItem					(PIItem itm, CUIDragDropListEx* lst);
	void						PropertiesBoxForUsing		(PIItem item, bool& b_show);
	void						PropertiesBoxForPlaying		(PIItem item, bool& b_show);
	void						PropertiesBoxForSlots		(PIItem item, bool& b_show);
	void						PropertiesBoxForWeapon		(CUICellItem* cell_item, PIItem item, bool& b_show);
	void						PropertiesBoxForAddon		(PIItem item, bool& b_show);
	void						PropertiesBoxForDrop		(CUICellItem* cell_item, PIItem item, bool& b_show);
	void						ActivatePropertiesBox		();
	bool						TryUseItem					(CUICellItem* cell_itm);
	//----------------------	-----------------------------------------------
	void						SendEvent_Item2Slot			(PIItem	pItem);
	void						SendEvent_Item2Belt			(PIItem	pItem);
	void						SendEvent_Item2Ruck			(PIItem	pItem);
	void						SendEvent_Item_Drop			(PIItem	pItem);
	void						SendEvent_Item_Eat			(PIItem	pItem);
	void						SendEvent_ActivateSlot		(PIItem	pItem);

	//---------------------------------------------------------------------
	
	void		xr_stdcall		ProcessPropertiesBoxClicked	(CUIWindow* w, void* d);
	void		xr_stdcall		OnExitBtnClicked			(CUIWindow* w, void* d);

	void						DropCurrentItem				(bool b_all);
	void						EatItem						(PIItem itm);
	
	bool						ToSlot						(CUICellItem* itm, bool force_place);
	bool						ToBag						(CUICellItem* itm, bool b_use_cursor_pos);
	bool						ToBelt						(CUICellItem* itm, bool b_use_cursor_pos);


	void						AttachAddon					(PIItem item_to_upgrade);
	void						DetachAddon					(LPCSTR addon_name, PIItem itm = NULL);

	void						SetCurrentItem				(CUICellItem* itm);
	CUICellItem*				CurrentItem					();
	PIItem						CurrentIItem				();

	TIItemContainer				ruck_list;
	u32							m_iCurrentActiveSlot;

};