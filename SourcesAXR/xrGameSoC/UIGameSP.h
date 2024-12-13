#pragma once
#include "uigamecustom.h"
#include "ui/UIDialogWnd.h"
#include "../../xrCore/net_utils.h"
#include "game_graph_space.h"

class CUIInventoryWnd;
class CUITradeWnd;
class CUIPdaWnd;
class CUITalkWnd;
class CUICarBodyWnd;
class CInventory;

class game_cl_Single;
class CChangeLevelWnd;
class CUIMessageBox;
class CInventoryBox;
class CInventoryOwner;
class CCar;

class CUIGameSP : public CUIGameCustom
{
private:
	game_cl_Single*		m_game;
	typedef CUIGameCustom inherited;
public:
	CUIGameSP									();
	virtual				~CUIGameSP				();

	virtual	void		reset_ui				();
	virtual	void		shedule_Update			(u32 dt);
	virtual void		SetClGame				(game_cl_GameState* g);
	virtual bool		IR_OnKeyboardPress		(int dik);
	virtual bool		IR_OnKeyboardRelease	(int dik);

	void				StartTalk				();
	void				StartCarBody			(CInventoryOwner* pOurInv, CInventoryOwner* pOthers);
	void				StartCarBody			(CInventoryOwner* pOurInv, CInventoryBox* pBox);
	void				StartCarBody			(CInventoryOwner* pActorInv, CCar* pCar);
	virtual void		ReInitShownUI			();
	void				ChangeLevel				(GameGraph::_GRAPH_ID game_vert_id, u32 level_vert_id, Fvector pos, Fvector ang, Fvector pos2, Fvector ang2, bool b);

	virtual void		HideShownDialogs		();
	virtual void		Render					();
	virtual void		HideTradeMenu			();
			void		StartTrade				(CInventoryOwner* pActorInv, CInventoryOwner* pMech);

	CUIInventoryWnd*	InventoryMenu;
	CUITradeWnd*		TradeMenu;
	CUIPdaWnd*			PdaMenu;
	CUITalkWnd*			TalkMenu;
	CUICarBodyWnd*		UICarBodyMenu;
	CChangeLevelWnd*	UIChangeLevelWnd;
};


class CChangeLevelWnd :public CUIDialogWnd
{
	CUIMessageBox*			m_messageBox;
	typedef CUIDialogWnd	inherited;
	void					OnCancel			();
	void					OnOk				();
public:
	GameGraph::_GRAPH_ID	m_game_vertex_id;
	u32						m_level_vertex_id;
	Fvector					m_position;
	Fvector					m_angles;
	Fvector					m_position_cancel;
	Fvector					m_angles_cancel;
	bool					m_b_position_cancel;

						CChangeLevelWnd				();
	virtual				~CChangeLevelWnd			()									{};
	virtual void		SendMessage					(CUIWindow *pWnd, s16 msg, void *pData);
	virtual bool		WorkInPause					()const {return true;}
	virtual void		Show						();
	virtual void		Hide						();
	virtual bool		OnKeyboardAction					(int dik, EUIMessages keyboard_action);
};