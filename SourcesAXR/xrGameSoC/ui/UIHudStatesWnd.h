#ifndef	UI_HUD_STATES_WND_H_INCLUDED
#define UI_HUD_STATES_WND_H_INCLUDED

#include "UIWindow.h"

class CUIHudStatesWnd : public CUIWindow
{
	typedef CUIWindow						inherited;
public:
	bool				m_b_force_update;
					CUIHudStatesWnd		();
	virtual			~CUIHudStatesWnd	();

}; // class CUIHudStatesWnd

#endif // UI_HUD_STATES_WND_H_INCLUDED
