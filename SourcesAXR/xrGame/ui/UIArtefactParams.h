#pragma once
#include "UIWindow.h"
#include "..\..\xrServerEntities\alife_space.h"
#include "..\inventory_item.h"

class CUIXml;
class CUIStatic;
class CUITextWnd;
class UIArtefactParamItem;
class CArtefact;

class CUIArtefactParams : public CUIWindow
{
public:
					CUIArtefactParams		();
	virtual			~CUIArtefactParams		();
			void	InitFromXml				(CUIXml& xml);
			bool	Check					(const shared_str& af_section);
			bool	CheckDescrInfoPortions	(const shared_str& af_section);
			void	SetInfo					(CInventoryItem& pInvItem);

protected:
	UIArtefactParamItem*	m_immunity_item[ALife::eHitTypeMax];
	UIArtefactParamItem*	m_restore_item[ALife::eRestoreTypeMax];
	UIArtefactParamItem*	m_additional_weight;
	UIArtefactParamItem*	m_fJumpSpeed;
	UIArtefactParamItem*	m_fWalkAccel;
	UIArtefactParamItem*	m_iArtefactRank;
	UIArtefactParamItem*	m_fChargeLevel;

	CUIStatic*				m_Prop_line;

}; // class CUIArtefactParams

// -----------------------------------

class UIArtefactParamItem : public CUIWindow
{
public:
				UIArtefactParamItem	();
	virtual		~UIArtefactParamItem();
		
		void	Init				( CUIXml& xml, LPCSTR section );
		void	SetCaption			( LPCSTR name );
		void	SetValue			( float value, int vle = 0 );
	
private:
	CUIStatic*	m_caption;
	CUITextWnd*	m_value;
	float		m_magnitude;
	bool		m_sign_inverse;
	bool		m_show_sign;
	shared_str	m_unit_str;
	shared_str	m_texture;

	//Color
	u32			m_negative_color;
	u32			m_neutral_color;
	u32			m_positive_color;
	bool		clr_invert;
	bool		use_color;
	bool		clr_dynamic;

}; // class UIArtefactParamItem
