#pragma once

#include "UIDialogWnd.h"
#include "UIPdaAux.h"
#include "../encyclopedia_article_defs.h"

class CInventoryOwner;
class CUIFrameLineWnd;
class CUIButton;
class CUITabControl;
class CUIStatic;
class CUIMapWnd;
class CUIEncyclopediaWnd;
class CUIDiaryWnd;
class CUIActorInfoWnd;
class CUIStalkersRankingWnd;
class CUIEventsWnd;
class CUIPdaContactsWnd;

 

class CUIPdaWnd: public CUIDialogWnd
{
private:
	typedef CUIDialogWnd	inherited;
protected:
	//элементы декоративного интерфейса
	CUIFrameLineWnd*		UIMainButtonsBackground;
	CUIFrameLineWnd*		UITimerBackground;

	// кнопки PDA
	CUITabControl*			UITabControl;

	// Установить игровое время
    void UpdateDateTime() const;
    void DrawUpdatedSections() const;

protected:
	// Бэкграунд
	CUIStatic*				UIMainPdaFrame;
	CUIStatic*				m_updatedSectionImage;
	CUIStatic*				m_oldSectionImage;

	// Текущий активный диалог
	CUIWindow*				m_pActiveDialog;
	EPdaTabs				m_pActiveSection;
	xr_vector<Fvector2>		m_sign_places_main;

public:
	// Поддиалоги PDA
	CUIMapWnd*				UIMapWnd;
	CUIPdaContactsWnd*		UIPdaContactsWnd;
	CUIEncyclopediaWnd*		UIEncyclopediaWnd;
	CUIDiaryWnd*			UIDiaryWnd;
	CUIActorInfoWnd*		UIActorInfo;
	CUIStalkersRankingWnd*	UIStalkersRanking;
	CUIEventsWnd*			UIEventsWnd;
	
	Frect					m_cursor_box;

	virtual void			Reset				();

public:
							CUIPdaWnd			();
	virtual					~CUIPdaWnd			();

	virtual void 			Init				();

    virtual void SendMessage(CUIWindow* pWnd, s16 msg, void* pData = nullptr);

	virtual void 			Draw				();
	virtual void 			Update				();
	virtual void 			Show				();
	virtual void 			Hide				();

    virtual bool OnMouseAction(float x, float y, EUIMessages mouse_action) override;
    virtual bool OnKeyboardAction(int dik, EUIMessages keyboard_action) override;
	virtual void Enable(bool status);
			void MouseMovement(float x, float y);
	
	void					SetActiveSubdialog	(EPdaTabs section);
	virtual bool			StopAnyMove			(){return false;}

    void PdaContentsChanged(pda_section::part type, bool = true) const;

	void ResetCursor();
	float m_power;
    Fvector2 last_cursor_pos{};
    bool bButtonL{}, bButtonR{};
    Fvector target_joystickrot{}, joystickrot{};
    float target_buttonpress{}, buttonpress{};

    void ResetJoystick(bool bForce)
    {
        if (bForce)
        {
            joystickrot.set(0.f, 0.f, 0.f);
            buttonpress = 0.f;
        }

        target_joystickrot.set(0.f, 0.f, 0.f);
        target_buttonpress = 0.f;
    }
};
