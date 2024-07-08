#pragma once
#include "uiwindow.h"

class CInventoryItem;
class CUIStatic;
class CUIScrollView;
class CUIProgressBar;
class CUIWpnParams;
class CUIArtefactParams;
class CUIOutfitItem;
class CUIBoosterInfo;
class CUIInventoryItem;

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
	CInventoryItem*			m_pInvItem;
public:
						CUIItemInfo			();
	virtual				~CUIItemInfo		();

	void				Init				(float x, float y, float width, float height, LPCSTR xml_name);
	void				Init				(LPCSTR xml_name);
	void				InitItem			(CInventoryItem* pInvItem);
	void				TryAddWpnInfo		(const shared_str& wpn_section);
	void				TryAddArtefactInfo	(CInventoryItem& pInvItem);
	void				TryAddOutfitInfo	(CInventoryItem& pInvItem, CInventoryItem* pCompareItem);
	void				TryAddBoosterInfo	(CInventoryItem& pInvItem);
	void				TryAddItemInfo		(CInventoryItem& pInvItem);

	void				ResetInventoryItem	();

	virtual void		Draw				();
	bool				m_b_force_drawing;
	CUIStatic*			UIName;
	CUIStatic*			UIWeight;
	CUIStatic*			UICost;
	CUIStatic*			UICondition;
	CUIStatic*			UICharge;
	CUIScrollView*		UIDesc;
	CUIProgressBar*		UICondProgresBar;
	CUIWpnParams*		UIWpnParams;
	CUIArtefactParams*	UIArtefactParams;
	CUIOutfitItem*		UIOutfitItem;
	CUIBoosterInfo*		UIBoosterInfo;
	CUIInventoryItem*	UIInventoryItem;

	Fvector2			UIItemImageSize; 
	CUIStatic*			UIItemImage;
};