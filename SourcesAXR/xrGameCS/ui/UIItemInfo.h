#pragma once
#include "uiwindow.h"


class CInventoryItem;
class CUIStatic;
class CUIScrollView;
class CUIProgressBar;
class CUIConditionParams;
class CUIWpnParams;
class CUIArtefactParams;
class CUIFrameWindow;
class UIInvUpgPropertiesWnd;
class CUIOutfitItem;
class CUIOutfitItemInfo;
class CUIBoosterInfo;
class CUICellItem;
class CUIInventoryItem;
class CUIAmmoParams;

class CUIItemInfo: public CUIWindow
{
private:
	typedef CUIWindow inherited;
	struct _desc_info
	{
		CGameFont*			pDescFont;
		u32					uDescClr;
		bool				bShowDescrText;
	};
	_desc_info				m_desc_info;
	CInventoryItem* m_pInvItem;
public:
						CUIItemInfo			();
	virtual				~CUIItemInfo		();
	CInventoryItem*		CurrentItem			() const {return m_pInvItem;}
	void				InitItemInfo		(Fvector2 pos, Fvector2 size, LPCSTR xml_name);
	void				InitItemInfo		(LPCSTR xml_name);
	void				InitItem			(CUICellItem* pCellItem, CInventoryItem* pCompareItem = NULL, u32 item_price=u32(-1), LPCSTR trade_tip=NULL);
	void				InitItemUpgrade		(CInventoryItem* pInvItem, bool icon_2d = true);


	void				TryAddConditionInfo	(CInventoryItem& pInvItem, CInventoryItem* pCompareItem);
	void				TryAddWpnInfo		(CInventoryItem& pInvItem, CInventoryItem* pCompareItem);
	void				TryAddArtefactInfo	(CInventoryItem& pInvItem);
	void				TryAddOutfitInfo	(CInventoryItem& pInvItem, CInventoryItem* pCompareItem);
	void				TryAddUpgradeInfo	(CInventoryItem& pInvItem);
	void				TryAddBoosterInfo	(CInventoryItem& pInvItem);
	void				TryAddItemInfo		(CInventoryItem& pInvItem);
	void				TryAddAmmoInfo		(CInventoryItem& pInvItem);

	void				ResetInventoryItem	();
	
	virtual void		Draw				();
	bool				m_b_FitToHeight;
	u32					delay;
	
	CUIFrameWindow*		UIBackground;
	CUIStatic*			UIName;
	CUIStatic*			UIWeight;
	CUIStatic*			UICost;
	CUIStatic*			UITradeTip;
	CUIScrollView*		UIDesc;
	bool				m_complex_desc;

	CUIConditionParams*		UIConditionWnd;
	CUIWpnParams*			UIWpnParams;
	CUIArtefactParams*		UIArtefactParams;
	UIInvUpgPropertiesWnd*	UIProperties;
	CUIOutfitItem*			UIOutfitItem;
	CUIBoosterInfo*			UIBoosterInfo;
	CUIInventoryItem*		UIInventoryItem;
	CUIAmmoParams*			UIAmmoParams;

	Fvector2			UIItemImageSize; 
	CUIStatic*			UIItemImage;
};