#pragma once
#include "UIWindow.h"
#include "..\..\XrServerEntitiesCS\alife_space.h"


class CUIXml;
class CUIStatic;
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
	UIArtefactParamItem*	m_immunity_item[ALife::infl_max_count];
	UIArtefactParamItem*	m_additional_weight;
	UIArtefactParamItem*	m_fHealthRestoreSpeed;
	UIArtefactParamItem*	m_fRadiationRestoreSpeed;
	UIArtefactParamItem*	m_fSatietyRestoreSpeed;
	UIArtefactParamItem*	m_fPowerRestoreSpeed;
	UIArtefactParamItem*	m_fBleedingRestoreSpeed;
	UIArtefactParamItem*	m_fThirstRestoreSpeed;
	UIArtefactParamItem*	m_fChargeLevel;

}; // class CUIArtefactParams

// -----------------------------------

class UIArtefactParamItem : public CUIWindow
{
public:
				UIArtefactParamItem	();
	virtual		~UIArtefactParamItem();
		
		void	Init				( CUIXml& xml, LPCSTR section );
		void	SetCaption			( LPCSTR name );
		void	SetValue			( float value );
	
private:
	CUIStatic*	m_caption;
	CUIStatic*	m_value;
	float		m_magnitude;
	bool		m_sign_inverse;
	bool		m_show_sign;
	shared_str	m_unit_str;
	shared_str	m_texture_minus;
	shared_str	m_texture_plus;

}; // class UIArtefactParamItem
