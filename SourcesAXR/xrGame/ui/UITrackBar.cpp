#include "StdAfx.h"

#include "string_table.h"
#include "UITrackBar.h"
#include "UI3tButton.h"
#include "UITextureMaster.h"
#include "../../xrEngine/xr_input.h"

CUITrackBar::CUITrackBar()
	: m_f_min(0),
	  m_f_max(1),
	  m_f_val(0),
	  m_f_opt_backup_value(0),
	 m_f_step(0.01f)
{	
	m_pSlider						= xr_new<CUI3tButton>();			
	AttachChild						(m_pSlider);		
	m_pSlider->SetAutoDelete		(true);

	m_static = new CUIStatic();
	m_static->Enable(false);
	AttachChild(m_static);
	m_static->SetAutoDelete(true);

	m_b_mouse_capturer				= false;
	m_def_control_height			= 16.0f;
}

void CUITrackBar::OnValueChanged()
{
	GetMessageTarget()->SendMessage(this, BUTTON_CLICKED, nullptr);

	UpdatePos();
	SaveOptValue();
}

bool CUITrackBar::OnMouseAction(float x, float y, EUIMessages mouse_action)
{
	CUIWindow::OnMouseAction(x, y, mouse_action);

	switch (mouse_action)
	{
	case WINDOW_MOUSE_MOVE:
		{
			if(m_bCursorOverWindow && m_b_mouse_capturer)
			{
				if (pInput->iGetAsyncBtnState(0))
					UpdatePosRelativeToMouse();
			}
		}break;
	case WINDOW_LBUTTON_DOWN:
		{
			m_b_mouse_capturer = m_bCursorOverWindow;
			if(m_b_mouse_capturer)
				UpdatePosRelativeToMouse();
		}break;

	case WINDOW_LBUTTON_UP:
		{
			SaveOptValue();
			m_b_mouse_capturer = false;
		}
		break;
	case WINDOW_MOUSE_WHEEL_UP:
		{
			if(m_b_is_float)
			{
				m_f_val -= GetInvert()?-m_f_step:m_f_step;
				clamp(m_f_val, m_f_min, m_f_max);
			}
			else
			{
				m_i_val -= GetInvert()?-m_i_step:m_i_step;
				clamp(m_i_val, m_i_min, m_i_max);
			}
			GetMessageTarget()->SendMessage(this, BUTTON_CLICKED, NULL);
			UpdatePos			();
			OnChangedOptValue	();
		}
		break;
	case WINDOW_MOUSE_WHEEL_DOWN:
		{
			if(m_b_is_float)
			{
				m_f_val += GetInvert()?-m_f_step:m_f_step;
				clamp(m_f_val, m_f_min, m_f_max);
			}
			else
			{
				m_i_val += GetInvert()?-m_i_step:m_i_step;
				clamp(m_i_val, m_i_min, m_i_max);
			}
			GetMessageTarget()->SendMessage(this, BUTTON_CLICKED, NULL);
			UpdatePos();
			OnChangedOptValue	();
		}
		break;
	};
	return true;
}

void CUITrackBar::InitTrackBar(Fvector2 pos, Fvector2 size)
{
	string128			buf;
	float				item_height;
	float				item_width;

	m_slider_texture == nullptr ? "ui_inGame2_opt_slider_box" : m_slider_texture;

	InitIB				(pos, size);

	

	InitState			(S_Enabled, "ui_inGame2_opt_slider_bar");
	InitState			(S_Disabled, "ui_inGame2_opt_slider_bar");
	
	strconcat			(sizeof(buf),buf, m_slider_texture,"_e");
	item_width			= CUITextureMaster::GetTextureWidth(buf);
    item_height			= CUITextureMaster::GetTextureHeight(buf);

	item_width			*= UI().get_current_kx();

	m_pSlider->InitButton(	Fvector2().set(0.0f, 0.0f) /*(size.y - item_height)/2.0f)*/,
							Fvector2().set(item_width, item_height) );			//size
	m_pSlider->InitTexture(m_slider_texture);
	
	SetCurrentState(S_Enabled);
}	

void CUITrackBar::Draw()
{
	CUI_IB_FrameLineWnd::Draw	();
	m_pSlider->Draw				();
	m_static->Draw				();
}

void CUITrackBar::Update()
{
	CUIWindow::Update();

	if(m_b_mouse_capturer)
	{
		if(!pInput->iGetAsyncBtnState(0))
			m_b_mouse_capturer = false;
	}
}

void CUITrackBar::SetCurrentOptValue()
{
	//CUIOptionsItem::SetCurrentOptValue();
	if(m_b_is_float)
    {
		GetOptFloatValue(m_f_val, m_f_min, m_f_max);
    }
	else if (m_b_is_token)
	{
		const xr_string val = GetOptTokenValue();
		const xr_token* tok = GetOptToken();
		int i = 0;
		while (tok->name)
		{
			if (val == tok->name)
			{
				m_i_val = i;
			}
			i++;
			tok++;
		}
		m_i_min = 0;
		m_i_max = i - 1;
	}
	else if (m_b_is_bool)
	{
		m_i_min = 0;
		m_i_max = 1;

		m_i_val = GetOptBoolValue() ? 1 : 0;
	}
	else
    {
		GetOptIntegerValue(m_i_val, m_i_min, m_i_max);
    }

	UpdateMinMax		();
	UpdatePos			();
}

void CUITrackBar::SaveOptValue()
{
	CUIOptionsItem::SaveOptValue	();
	if(m_b_is_float)
		SaveOptFloatValue			(m_f_val);
	else if (m_b_is_token)
	{
		const xr_token* tok = GetOptToken();
		const char* cur_val = tok[m_i_val].name;

		SaveOptStringValue(cur_val);
	}
	else if (m_b_is_bool)
		SaveOptBoolValue(m_i_val == 1);
	else
		SaveOptIntegerValue			(m_i_val);

	GetMessageTarget()->SendMessage(this, TRACKBAR_CHANGED);
}

float CUITrackBar::GetTrackValue() const
{
	if (m_b_is_float)
		return (m_f_val);
	else
		return float(m_i_val);
}

void CUITrackBar::UpdateMinMax()
{
	if (m_b_is_float)
	{
		if (m_b_min_xml_set && m_f_min_xml > m_f_min)
			m_f_min = m_f_min_xml;

		if (m_b_max_xml_set && m_f_max_xml < m_f_max)
			m_f_max = m_f_max_xml;
	}
	else
	{
		if (m_b_min_xml_set && m_f_min_xml > static_cast<float>(m_i_min))
			m_i_min = iFloor(m_f_min_xml);

		if (m_b_max_xml_set && m_f_max_xml < static_cast<float>(m_i_max))
			m_i_max = iFloor(m_f_max_xml);
	}
}

void CUITrackBar::SetTrackValue(float v)
{
	UpdateMinMax();

	const float max = (m_b_is_float) ? m_f_max : static_cast<float>(m_i_max);
	const float min = (m_b_is_float) ? m_f_min : static_cast<float>(m_i_min);

	clamp(v, min, max);

	if (m_b_is_float)
		m_f_val = v;
	else
		m_i_val = v;

	UpdatePos();
}

bool CUITrackBar::IsChangedOptValue() const
{
	if(m_b_is_float)
		return !fsimilar(m_f_opt_backup_value, m_f_val); 
	else
		return (m_i_opt_backup_value != m_i_val);
}

float CUITrackBar::GetStep()
{
	if (m_b_is_float)
		return m_f_step;
	else
		return m_i_step;
}

void CUITrackBar::SetStep(float step)
{
	if(m_b_is_float)
		m_f_step	= step;
	else if (m_b_is_bool || m_b_is_token)
		m_i_step	= 1;
	else
		m_i_step	= iFloor(step);
}

void CUITrackBar::SaveBackUpOptValue()
{
	//CUIOptionsItem::SaveBackUpOptValue();

	if(m_b_is_float)
		m_f_opt_backup_value		= m_f_val;
	else
		m_i_opt_backup_value		= m_i_val;
}

void CUITrackBar::UndoOptValue()
{
	if(m_b_is_float)
		m_f_val			= m_f_opt_backup_value;
	else
		m_i_val			= m_i_opt_backup_value;

	SaveOptValue		();
	SetCurrentOptValue	();
}

void CUITrackBar::Enable(bool status)
{
	m_bIsEnabled				= status;
	SetCurrentState				(m_bIsEnabled?S_Enabled:S_Disabled);
	m_pSlider->Enable			(m_bIsEnabled);
}

void CUITrackBar::UpdatePosRelativeToMouse()
{
	const float slider_width = m_pSlider->GetWidth();
	const float window_width = GetWidth();

	float mouse_position = m_cursor_pos.x;

	if (GetInvert())
		mouse_position = window_width - mouse_position;

	clamp(mouse_position, slider_width / 2, window_width - slider_width / 2);

	const float max = (m_b_is_float) ? m_f_max : static_cast<float>(m_i_max);
	const float min = (m_b_is_float) ? m_f_min : static_cast<float>(m_i_min);
	const float step = (m_b_is_float) ? m_f_step : static_cast<float>(m_i_step);

	const float position = (max - min) * (mouse_position - slider_width / 2) / (window_width - slider_width);

	float delta = step * iFloor(position / step);

	if (position - delta > step / 2.0f)
	{
		delta += step;
	}

	float new_value = min + delta;

	clamp(new_value, min, max);

	bool changed;

	if (m_b_is_float)
	{
		const float backup_float = m_f_val;
		m_f_val = new_value;
		changed = !fsimilar(backup_float, m_f_val);
	}
	else
	{
		const int backup_int = m_i_val;
		m_i_val = iFloor(new_value);
		changed = (backup_int != m_i_val);
	}

	if (changed)
		OnValueChanged();
}

void CUITrackBar::UpdatePos()
{
#ifdef DEBUG
	
	if(m_b_is_float)
		R_ASSERT2(m_f_val >= m_f_min && m_f_val <= m_f_max, "CUITrackBar::UpdatePos() - m_val >= m_min && m_val <= m_max" );
	else
		R_ASSERT2(m_i_val >= m_i_min && m_i_val <= m_i_max, "CUITrackBar::UpdatePos() - m_val >= m_min && m_val <= m_max" );

#endif

	const float slider_width = m_pSlider->GetWidth();
	const float window_width = GetWidth();		

	Fvector2 pos = m_pSlider->GetWndPos();

	const float val = (m_b_is_float) ? m_f_val : static_cast<float>(m_i_val);
	const float max = (m_b_is_float) ? m_f_max : static_cast<float>(m_i_max);
	const float min = (m_b_is_float) ? m_f_min : static_cast<float>(m_i_min);

	const float free_space = window_width - slider_width;

	pos.x = (val - min) * free_space / (max - min);

	if( GetInvert() )
		pos.x					= free_space-pos.x;

	m_pSlider->SetWndPos		(pos);

	if (m_static->IsEnabled())
	{
		string256 buff;

		if (m_b_is_token)
		{
			int i = m_i_val;

			if (GetInvert())
				i = m_i_max - i;
			
			const xr_token* tok = GetOptToken();
			const LPCSTR cur_val = *CStringTable().translate(tok[i].name);
			xr_sprintf(buff, cur_val);
		}
		else if (m_b_is_bool)
		{
			const LPCSTR cur_val = *CStringTable().translate(m_i_val == 1 ? "_null_opt_enabled" : "_null_opt_disabled");
			xr_sprintf(buff, cur_val);
		}
		else if (m_b_is_float)
		{
			float f = m_f_val;

			if (GetInvert())
				f = m_f_max - f;
			
			const LPCSTR cur_val = m_static_format == NULL ? "%.1f" : m_static_format.c_str();
			xr_sprintf(buff, cur_val, f);
		}
		else
		{
			int i = m_i_val;

			if (GetInvert())
				i = m_i_max - i;
				
			const LPCSTR cur_val = m_static_format == NULL ? "%d" : m_static_format.c_str();
			xr_sprintf(buff, cur_val, i);
		}
		m_static->TextItemControl()->SetTextST(buff);
	}
}

void CUITrackBar::OnMessage(LPCSTR message)
{
	if (0 == xr_strcmp(message, "set_default_value"))
	{
		if(m_b_is_float)
			m_f_val = m_f_min + (m_f_max - m_f_min)/2.0f;
		else
			m_i_val = m_i_min + iFloor((m_i_max - m_i_min)/2.0f);

		UpdatePos();
	}
}

bool CUITrackBar::GetCheck()
{
	VERIFY(!m_b_is_float);
	return !!m_i_val;
}

void CUITrackBar::SetCheck(bool b)
{
	VERIFY(!m_b_is_float);
	m_i_val = (b)?m_i_max:m_i_min;
}
