#pragma once

#include "UIWindow.h"
#include "../alife_space.h"

class CUIScrollView;
class CCustomOutfit;
class CUIStatic;
class CUIXml;

class CUIActorStats : public CUIWindow
{
CCustomOutfit*		m_outfit;
public:
					CUIActorStats			();
	virtual			~CUIActorStats			();

			void 	Update					(CCustomOutfit* outfit);	
			void 	InitFromXml				(CUIXml& xml_doc);
protected:
	void			SetItem					(u32 hitType, bool force_add);

	CUIScrollView*	m_listWnd;

	CUIStatic*		m_items[ALife::eHitTypeMax];
};