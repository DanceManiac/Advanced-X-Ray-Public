#pragma once

#include "UIWindow.h"
#include "UIOptionsItem.h"
#include "UI_IB_Static.h"
#include "UIStatic.h"

class CUI3tButton;
class CUIFrameLineWnd;
class CUITrackButton;

class CUITrackBar : public CUIWindow, public CUIOptionsItem 
{
	friend class CUITrackButton;
public:
					CUITrackBar				();
	// CUIOptionsItem
	virtual void 	SetCurrentValue			();
	virtual void 	SaveValue				();
	virtual bool 	IsChanged				();
	virtual void 	SeveBackUpValue			();
	virtual void 	Undo					();
	virtual void	Draw					();
	virtual void	Update					();
	virtual bool	OnMouseAction					(float x, float y, EUIMessages mouse_action);
	virtual	void 	OnMessage				(LPCSTR message);
	// CUIWindow
			void	InitTrackBar			(Fvector2 pos, Fvector2 size);
	virtual void	Enable					(bool status);
			void	SetInvert				(bool v){m_b_invert=v;}
			bool	GetInvert				() const	{return m_b_invert;}

			float	GetStep					();
			void	SetStep					(float step);
			void	SetFloat				(bool b_float){m_b_is_float=b_float; if (b_float) {m_b_is_token = false; m_b_is_bool = false;} };
			void	SetToken				(bool b_token){m_b_is_token= b_token; if (b_token) {m_b_is_float = false; m_b_is_bool = false;} };
			void	SetBool					(bool b_bool){m_b_is_bool= b_bool; if (b_bool) {m_b_is_token = false; m_b_is_float = false;} };
			float	GetTrackValue			() const;
			void SetMin(float v)
			{
				m_b_min_xml_set = true;
				m_f_min_xml = v;
			}
			void SetMax(float v)
			{
				m_b_max_xml_set = true;
				m_f_max_xml = v;
			}
			void	SetTrackValue			(float v);
			bool	GetCheck				();
			void	SetCheck				(bool b);
			int		GetIValue				(){return m_i_val;}
			float	GetFValue				(){return m_f_val;}
			void	SetOptIBounds			(float imin, float imax) { SetMin(imin); SetMax(imax); };
			void	SetOptFBounds			(float fmin, float fmax) { SetMin(fmin); SetMax(fmax); };

			pcstr	GetDebugType			() override { return "CUITrackBar"; }

			CUIStatic* m_static;
			shared_str m_static_format;
			float m_def_control_height;

			LPCSTR m_frame_line_texture;
			LPCSTR m_frame_line_texture_d;
			LPCSTR m_slider_texture;
			void	SetFrameLineTexture		(LPCSTR texture) { m_frame_line_texture = texture; };
			void	SetFrameLineTextureD	(LPCSTR texture) { m_frame_line_texture_d = texture; };
			void	SetSliderTexture		(LPCSTR texture) { m_slider_texture = texture; };
protected:
			//void	ShowCurrentValue		() const;
			void 	UpdatePos				();
			void 	UpdatePosRelativeToMouse();
			void	OnValueChanged			();
			void	UpdateMinMax			();

    CUI3tButton*		m_pSlider;
	CUIFrameLineWnd*	m_pFrameLine;
	CUIFrameLineWnd*	m_pFrameLine_d;
	bool				m_b_invert{ false };
	bool				m_b_is_float{ true };
	bool				m_b_is_token{ false };
	bool				m_b_is_bool{ false };

	bool				m_b_mouse_capturer{ false };

	float				m_f_max_xml = 0.f;
	float				m_f_min_xml = 0.f;

	bool				m_b_max_xml_set{};
	bool				m_b_min_xml_set{};


	union{
		struct{
			float				m_f_val;
			float				m_f_max;
			float				m_f_min;
			float				m_f_step;
			float				m_f_back_up;
		};
		struct{
			int					m_i_val;
			int					m_i_max;
			int					m_i_min;
			int					m_i_step;
			int					m_i_back_up;
		};
	};
};