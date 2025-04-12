#include "StdAfx.h"

#include "string_table.h"
#include "UITrackBar.h"
//.#include "UITrackButton.h"
#include "UIFrameLineWnd.h"
#include "UI3tButton.h"
#include "UITextureMaster.h"
#include "../../xrEngine/xr_input.h"

CUITrackBar::CUITrackBar()
	: m_f_min(0),
	  m_f_max(1),
	  m_f_val(0),
	  m_f_back_up(0),
	 m_f_step(0.01f)
{
	m_pFrameLine					= xr_new<CUIFrameLineWnd>();	
	AttachChild						(m_pFrameLine);	
	m_pFrameLine->SetAutoDelete		(true);
	m_pFrameLine_d					= xr_new<CUIFrameLineWnd>(); 
	m_pFrameLine_d->SetVisible		(false);
	AttachChild						(m_pFrameLine_d); 
	m_pFrameLine_d->SetAutoDelete	(true);
	m_pSlider						= xr_new<CUI3tButton>();			
	AttachChild						(m_pSlider);		
	m_pSlider->SetAutoDelete		(true);
	
	m_static						= new CUIStatic();
	m_static->Enable				(false);
	AttachChild						(m_static);
	m_static->SetAutoDelete			(true);

	m_b_mouse_capturer				= false;
	m_def_control_height			= 21.0f;
}

void CUITrackBar::OnValueChanged()
{
	GetMessageTarget()->SendMessage(this, BUTTON_CLICKED, nullptr);

	UpdatePos();
	SaveValue();
}

bool CUITrackBar::OnMouseAction(float x, float y, EUIMessages mouse_action)
{
	CUIWindow::OnMouseAction(x, y, mouse_action);
	if (m_pFrameLine->CursorOverWindow()) // хватать можно тока за саму полоску
	{
		if (mouse_action == WINDOW_LBUTTON_DOWN)
			m_b_mouse_capturer = true;
		else if (mouse_action == WINDOW_LBUTTON_UP)
			m_b_mouse_capturer = false;

		if (mouse_action == WINDOW_LBUTTON_DOWN || (m_b_mouse_capturer && mouse_action == WINDOW_MOUSE_MOVE && pInput->iGetAsyncKeyState(MOUSE_1)))
		{
			UpdatePosRelativeToMouse();
			return true;
		}
		else if (mouse_action == WINDOW_LBUTTON_UP)
		{
			SaveValue();
			return true;
		}
	}

	return false;
}

void CUITrackBar::Init(float x, float y, float width, float height)
{
	string128			buf;
	float				item_height;
	float				item_width;
	CUIWindow::Init		(x, y, width, m_def_control_height);
	
	m_frame_line_texture == nullptr ? "ui_slider_e" : m_frame_line_texture;
	m_frame_line_texture_d == nullptr ? "ui_slider_d" : m_frame_line_texture_d;
	m_slider_texture == nullptr ? "ui_slider_button" : m_slider_texture;

	Fvector2 size = Fvector2().set(width, height);

	item_height = CUITextureMaster::GetTextureHeight(strconcat(sizeof(buf), buf, m_frame_line_texture, "_b"));
	m_pFrameLine->SetWndPos(Fvector2().set(0.0f, (size.y - item_height) / 2));
	m_pFrameLine->SetWndSize(Fvector2().set(size.x, item_height));

	m_pFrameLine->InitTexture(m_frame_line_texture, "hud\\default");

	m_pFrameLine_d->SetWndPos(Fvector2().set(0.0f, (size.y - item_height) / 2.0f));
	m_pFrameLine_d->SetWndSize(Fvector2().set(size.x, item_height));
	m_pFrameLine_d->InitTexture(m_frame_line_texture_d, "hud\\default");

	strconcat			(sizeof(buf),buf,m_slider_texture,"_e");
	item_width			= CUITextureMaster::GetTextureWidth(buf);
    item_height			= CUITextureMaster::GetTextureHeight(buf);
	m_pSlider->Init		(0, (height - item_height)/2, item_width, item_height);
	m_pSlider->InitTexture(m_slider_texture);
}	

void CUITrackBar::Draw()
{
	CUIWindow::Draw();
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

void CUITrackBar::SetCurrentValue()
{
	//CUIOptionsItem::SetCurrentValue();
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

void CUITrackBar::SaveValue()
{
	CUIOptionsItem::SaveValue	();
	if(m_b_is_float)
		SaveOptFloatValue			(m_f_val);
	else if (m_b_is_token)
	{
		const xr_token* tok = GetOptToken();
		const char* cur_val = tok[m_i_val].name;

		SaveOptTokenValue(cur_val);
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

bool CUITrackBar::IsChanged()
{
	if(m_b_is_float)
		return !fsimilar(m_f_back_up, m_f_val); 
	else
		return (m_i_back_up != m_i_val);
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

void CUITrackBar::SeveBackUpValue()
{
	if(m_b_is_float)
		m_f_back_up		= m_f_val;
	else
		m_i_back_up		= m_i_val;
}

void CUITrackBar::Undo()
{
	if(m_b_is_float)
		m_f_val			= m_f_back_up;
	else
		m_i_val			= m_i_back_up;

	SaveValue			();
	SetCurrentValue		();
}

void CUITrackBar::Enable(bool status)
{
	m_bIsEnabled				= status;
	m_pFrameLine->SetVisible	(status);
	m_pSlider->Enable			(status);
	m_pFrameLine_d->SetVisible	(!status);
}

void CUITrackBar::UpdatePosRelativeToMouse()
{
	const float slider_width = m_pSlider->GetWidth();
	const float window_width = m_pFrameLine->GetWidth();

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
	const float window_width = m_pFrameLine->GetWidth();

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
		m_static->SetTextST(buff);
	}

	//SaveValue					();
}

void CUITrackBar::OnMessage(const char* message)
{
	if (0 == xr_strcmp(message,"set_default_value"))
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
