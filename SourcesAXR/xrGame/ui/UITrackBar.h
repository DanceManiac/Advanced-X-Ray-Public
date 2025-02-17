#pragma once

#include "UIOptionsItem.h"
#include "UI_IB_Static.h"
#include "UIStatic.h"

class CUI3tButton;
class CUITrackButton;

class CUITrackBar : public CUI_IB_FrameLineWnd, public CUIOptionsItem 
{
	friend class CUITrackButton;
public:
					CUITrackBar				();
	// CUIOptionsItem
	virtual void	SetCurrentOptValue		();	// opt->current
	virtual void	SaveBackUpOptValue		();	// current->backup
	virtual void	SaveOptValue			();	// current->opt
	virtual void	UndoOptValue			();	// backup->current
	virtual bool	IsChangedOptValue		() const;	// backup!=current
	
	virtual void	Draw					();
	virtual void	Update					();
	virtual bool	OnMouseAction			(float x, float y, EUIMessages mouse_action);
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
			void	SetBoundReady			(bool b_ready) {m_b_bound_already_set = b_ready;};
			float	GetTrackValue			() const;
			void	SetTrackValue			(float v);
			bool	GetCheck				();
			void	SetCheck				(bool b);
			int		GetIValue				(){return m_i_val;}
			float	GetFValue				(){return m_f_val;}
			void	SetOptIBounds			(int imin, int imax);
			void	SetOptFBounds			(float fmin, float fmax);

			pcstr	GetDebugType			() override { return "CUITrackBar"; }

			CUIStatic* m_static;
			shared_str m_static_format;
			float m_def_control_height;

			LPCSTR m_slider_texture;
			void	SetSliderTexture		(LPCSTR texture) { m_slider_texture = texture; };
protected:
			void 	UpdatePos				();
			void 	UpdatePosRelativeToMouse();
			void	OnValueChanged			();

    CUI3tButton*		m_pSlider;
	bool				m_b_invert{ false };
	bool				m_b_is_float{ true };
	bool				m_b_is_token{ false };
	bool				m_b_is_bool{ false };

	bool				m_b_mouse_capturer{ false };
    bool 				m_b_bound_already_set{ false };

	union{
		struct{
			float				m_f_val;
			float				m_f_max;
			float				m_f_min;
			float				m_f_step;
			float				m_f_opt_backup_value;
		};
		struct{
			int					m_i_val;
			int					m_i_max;
			int					m_i_min;
			int					m_i_step;
			int					m_i_opt_backup_value;
		};
	};
};