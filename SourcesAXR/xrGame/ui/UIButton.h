#pragma once

#include "UIStatic.h"
#include "../../xrServerEntities/script_export_space.h"


class CUIButton : public CUIStatic
{
private:
	typedef			CUIStatic				inherited;
public:
					CUIButton				();
	virtual			~CUIButton				()			{};

	virtual bool	OnMouseAction					(float x, float y, EUIMessages mouse_action);
	virtual void	OnClick					();

	//���������� ����
	virtual void	DrawTexture				();
	virtual void	DrawText				();

	virtual void	Update					();
	virtual void	Enable					(bool status);
	virtual bool	OnKeyboardAction				(int dik, EUIMessages keyboard_action);
	virtual void	OnFocusLost				();

	//������ � ������� ����� �������� ������
	typedef enum{NORMAL_PRESS, //������ ���������� ��� 
							   //������� � ���������� �� ��� ����
				 DOWN_PRESS    //����� ��� �������
			} E_PRESS_MODE;

	//��������� � ������� ��������� ������
	typedef enum{BUTTON_NORMAL, //������ ����� �� �������������
		BUTTON_PUSHED,			//� ������� ��������
		BUTTON_UP				//��� ������������ ������ ���� 
	} E_BUTTON_STATE;


	//������ ����������� ���������
    virtual void	Reset					();

	// ��������� ��������� ������: ��������, �� ��������
	void				SetButtonState			(E_BUTTON_STATE eBtnState)	{ m_eButtonState = eBtnState; }
	E_BUTTON_STATE		GetButtonState			() const					{ return m_eButtonState;}

	// ��������� ������ ��� ������������� ����������� ���� ������ � ������ NORMAL_PRESS
	void				SetButtonAsSwitch		(bool bAsSwitch)			{ m_bIsSwitch = bAsSwitch; }

	// ������ � �������������
	// ��� ������������ ������� �� ����� dinput.h, �� DirectX SDK.
	// ��������: ������ A - ��� 0x1E(DIK_A)
	void				SetAccelerator			(int iAccel, int idx);
	const int			GetAccelerator			(int idx) const;
	bool				IsAccelerator			(int iAccel) const;

	void				SetPressMode			(E_PRESS_MODE ePressMode)	{m_ePressMode = ePressMode;}
	E_PRESS_MODE		GetPressMode			()							{return m_ePressMode;}

	void				SetPushOffset			(const Fvector2& offset)	{m_PushOffset = offset;}
	Fvector2			GetPushOffset			()							{return m_PushOffset;}

	shared_str			m_hint_text;
protected:
	
	E_PRESS_MODE		m_ePressMode;
	Fvector2			m_PushOffset;
	E_BUTTON_STATE		m_eButtonState;
	s16					m_uAccelerator[4];
	//bool				m_bButtonClicked;
	bool				m_bIsSwitch;

	DECLARE_SCRIPT_REGISTER_FUNCTION
};