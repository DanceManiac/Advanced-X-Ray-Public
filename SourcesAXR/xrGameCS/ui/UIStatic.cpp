#include "stdafx.h"
#include "uistatic.h"
#include "UIXmlInit.h"
#include "UITextureMaster.h"
#include "uiframewindow.h"
#include "../HUDManager.h"
#include "../../xrEngine/LightAnimLibrary.h"
#include "uilines.h"
#include "../string_table.h"
#include "../ui_base.h"

#include "../Include/xrRender/UIRender.h"

const char * const	clDefault	= "default";
#define CREATE_LINES if (!m_pLines) {m_pLines = xr_new<CUILines>(); m_pLines->SetTextAlignment(CGameFont::alLeft);}
#define LA_CYCLIC			(1<<0)
#define LA_ONLYALPHA		(1<<1)
#define LA_TEXTCOLOR		(1<<2)
#define LA_TEXTURECOLOR		(1<<3)
//(1<<4) registered !!!
void lanim_cont::set_defaults()
{
	m_lanim					= NULL;	
	m_lanim_start_time		= -1.0f;
	m_lanim_delay_time		= 0.0f;
	m_lanimFlags.zero		();
}
void lanim_cont_xf::set_defaults()
{
	lanim_cont::set_defaults();
	m_origSize.set			(0,0);
}

CUIStatic:: CUIStatic()
{
	m_bTextureEnable		= true;
	m_bClipper				= false;
	m_bStretchTexture		= false;

	m_TextureOffset.set		(0.0f,0.0f);
	m_TextOffset.set		(0.0f,0.0f);

	m_ClipRect.set			(-1,-1,-1,-1);

	m_bCursorOverWindow		= false;
	m_bHeading				= false;
	m_bConstHeading			= false;
	m_fHeading				= 0.0f;
	m_lanim_clr.set_defaults	();
	m_lanim_xform.set_defaults	();

	m_pLines				= NULL;
	m_bEnableTextHighlighting = false;
}

CUIStatic::~ CUIStatic()
{
	xr_delete(m_pLines);
}

void CUIStatic::SetXformLightAnim(LPCSTR lanim, bool bCyclic)
{
	if(lanim && lanim[0]!=0)
		m_lanim_xform.m_lanim			= LALib.FindItem(lanim);
	else
		m_lanim_xform.m_lanim			= NULL;
	
	m_lanim_xform.m_lanimFlags.zero		();

	m_lanim_xform.m_lanimFlags.set		(LA_CYCLIC,			bCyclic);
	m_lanim_xform.m_origSize			= GetWndSize();
}

void CUIStatic::SetClrLightAnim(LPCSTR lanim, bool bCyclic, bool bOnlyAlpha, bool bTextColor, bool bTextureColor)
{
	if(lanim && lanim[0]!=0)
		m_lanim_clr.m_lanim	= LALib.FindItem(lanim);
	else
		m_lanim_clr.m_lanim	= NULL;
	
	m_lanim_clr.m_lanimFlags.zero		();

	m_lanim_clr.m_lanimFlags.set		(LA_CYCLIC,			bCyclic);
	m_lanim_clr.m_lanimFlags.set		(LA_ONLYALPHA,		bOnlyAlpha);
	m_lanim_clr.m_lanimFlags.set		(LA_TEXTCOLOR,		bTextColor);
	m_lanim_clr.m_lanimFlags.set		(LA_TEXTURECOLOR,	bTextureColor);
}


void CUIStatic::InitTexture(LPCSTR texture){
	InitTextureEx(texture);
}

void CUIStatic::CreateShader(const char* tex, const char* sh){
	m_UIStaticItem.CreateShader(tex,sh);	
}

void CUIStatic::SetTextureColor(u32 color){
	m_UIStaticItem.SetColor(color);
}

u32 CUIStatic::GetTextureColor() const{
	return m_UIStaticItem.GetColor();
}

void CUIStatic::InitTextureEx(LPCSTR tex_name, LPCSTR sh_name)
{

	/*
	string_path buff;
	u32		v_dev	= CAP_VERSION(HW.Caps.raster_major, HW.Caps.raster_minor);
	u32		v_need	= CAP_VERSION(2,0);
	//strstr(Core.Params,"-ps_movie") &&
	if ( (v_dev >= v_need) && FS.exist(buff,"$game_textures$", tex_name, ".ogm") )
		CUITextureMaster::InitTexture	(tex_name, "hud\\movie", &m_UIStaticItem);
	else
		CUITextureMaster::InitTexture	(tex_name, sh_name, &m_UIStaticItem);
	*/

	LPCSTR res_shname = UIRender->UpdateShaderName(tex_name, sh_name);
	CUITextureMaster::InitTexture	(tex_name, res_shname, &m_UIStaticItem);

	Fvector2 p						= GetWndPos();
	m_UIStaticItem.SetPos			(p.x, p.y);
}

void  CUIStatic::Draw()
{
	if(m_bClipper){
		Frect clip_rect;
		if (-1 == m_ClipRect.left && -1 == m_ClipRect.right && -1 == m_ClipRect.top && -1 == m_ClipRect.left){
			Frect			our_rect;
			GetAbsoluteRect	(our_rect);
			clip_rect		= our_rect;
			Frect			_r;
			GetParent()->GetAbsoluteRect(_r);
			if(GetParent())	clip_rect.intersection(our_rect,_r);			
		}else				
			clip_rect		= m_ClipRect;

		UI()->PushScissor	(clip_rect);
	}

	DrawTexture				();	
	inherited::Draw			();
	DrawText				();

	if(m_bClipper)	UI()->PopScissor();
}


void CUIStatic::DrawText()
{
	if (m_pLines)
	{
		m_pLines->SetWndSize(m_wndSize);

		if(IsHighlightText() && xr_strlen(m_pLines->GetText())>0 && m_bEnableTextHighlighting)
			DrawHighlightedText();		
		else{
			Fvector2			p;
			GetAbsolutePos		(p);
			p.add				(m_TextOffset);
			m_pLines->Draw		(p.x, p.y);
		}

	}
}
#include "../../Include/xrRender/UIShader.h"

void CUIStatic::DrawTexture()
{
	if(m_bTextureEnable && GetShader() && GetShader()->inited())
	{
		Frect			rect;
		GetAbsoluteRect	(rect);
		m_UIStaticItem.SetPos	(rect.left + m_TextureOffset.x, rect.top + m_TextureOffset.y);

		if(m_bStretchTexture)
		{
			if(Heading())
			{
				if( m_UIStaticItem.GetFixedLTWhileHeading() )
				{
					float t1,t2;
					t1			= rect.width();
					t2			= rect.height();
					rect.y2		= rect.y1 + t1;
					rect.x2		= rect.x1 + t2;
				}
			}
			m_UIStaticItem.SetRect(0, 0, rect.width(), rect.height());
		}else
		{
			Frect r={0.0f,0.0f,
				m_UIStaticItem.GetOriginalRect().width(),
				m_UIStaticItem.GetOriginalRect().height()};

			{	
				if(Heading())
				{
					float t1,t2;
					t1			= rect.width();
					t2			= rect.height();
					rect.y2		= rect.y1 + t1;
					rect.x2		= rect.x1 + t2;
				}

				m_UIStaticItem.SetRect(r);
			}
		}

		if( Heading() )
		{
			m_UIStaticItem.Render( GetHeading() );
		}else
			m_UIStaticItem.Render();
	}
}

void CUIStatic::Update()
{
	inherited::Update();
	//update light animation if defined
	if (m_lanim_clr.m_lanim)
	{
		if(m_lanim_clr.m_lanim_start_time<0.0f)		ResetClrAnimation	();
		float t = Device.dwTimeContinual/1000.0f;

		if (t < m_lanim_clr.m_lanim_start_time)	// consider animation delay
			return;

		if(m_lanim_clr.m_lanimFlags.test(LA_CYCLIC) || t-m_lanim_clr.m_lanim_start_time < m_lanim_clr.m_lanim->Length_sec()){

			int frame;
			u32 clr					= m_lanim_clr.m_lanim->CalculateRGB(t-m_lanim_clr.m_lanim_start_time,frame);

			if(m_lanim_clr.m_lanimFlags.test(LA_TEXTURECOLOR))
				if(m_lanim_clr.m_lanimFlags.test(LA_ONLYALPHA))
					SetColor				(subst_alpha(GetColor(), color_get_A(clr)));
				else
					SetColor				(clr);

			if(m_lanim_clr.m_lanimFlags.test(LA_TEXTCOLOR))
				if(m_lanim_clr.m_lanimFlags.test(LA_ONLYALPHA))
					SetTextColor				(subst_alpha(GetTextColor(), color_get_A(clr)));
				else
					SetTextColor				(clr);
			
		}
	}
	
	if(m_lanim_xform.m_lanim)
	{
		if(m_lanim_xform.m_lanim_start_time<0.0f){
			ResetXformAnimation();
		}
		float t = Device.dwTimeGlobal/1000.0f;

		if(	m_lanim_xform.m_lanimFlags.test(LA_CYCLIC) || 
			t - m_lanim_xform.m_lanim_start_time < m_lanim_xform.m_lanim->Length_sec() )
		{
			int frame;
			u32 clr				= m_lanim_xform.m_lanim->CalculateRGB(t-m_lanim_xform.m_lanim_start_time,frame);
			
			EnableHeading_int	(true);
			float heading		= (PI_MUL_2/255.0f) * color_get_A(clr);
			SetHeading			(heading);

			float _value		= (float)color_get_R(clr);
			
			float f_scale		= _value / 64.0f;
			Fvector2 _sz;
			_sz.set				(m_lanim_xform.m_origSize.x*f_scale, m_lanim_xform.m_origSize.y*f_scale );
			SetWndSize			(_sz);
		}else{
			EnableHeading_int	( !!m_lanim_xform.m_lanimFlags.test(1<<4) );
			SetWndSize			(m_lanim_xform.m_origSize);
		}
	}
}

void CUIStatic::ResetXformAnimation()
{
	m_lanim_xform.m_lanim_start_time = Device.dwTimeGlobal/1000.0f;
}

void CUIStatic::ResetClrAnimation()
{
	m_lanim_clr.m_lanim_start_time = Device.dwTimeContinual/1000.0f + m_lanim_clr.m_lanim_delay_time/1000.0f;
}

void CUIStatic::SetClrAnimDelay(float delay){
	m_lanim_clr.m_lanim_delay_time = delay;
}

bool CUIStatic::IsClrAnimStoped(){
	if (m_lanim_clr.m_lanimFlags.test(LA_CYCLIC) || m_lanim_clr.m_lanim_start_time<0.0f)
		return false;
	
	float t = Device.dwTimeContinual/1000.0f;
	if(t-m_lanim_clr.m_lanim_start_time < m_lanim_clr.m_lanim->Length_sec())
		return false;
	else 
		return true;
}

void CUIStatic::SetFont(CGameFont* pFont){
	CUIWindow::SetFont(pFont);
	CREATE_LINES;
	m_pLines->SetFont(pFont);
}

void CUIStatic::SetTextComplexMode(bool md){
	CREATE_LINES;
	m_pLines->SetTextComplexMode(md);
}

CGameFont* CUIStatic::GetFont(){
	CREATE_LINES;
	return m_pLines->GetFont();
}

void CUIStatic::TextureClipper(float offset_x, float offset_y, Frect* pClipRect)
{
	TextureClipper(offset_x, offset_y, pClipRect, m_UIStaticItem);
}

void CUIStatic::TextureClipper(float offset_x, float offset_y, Frect* pClipRect,
							   CUIStaticItem& UIStaticItem)
{
	Frect parent_rect;
	
	if(pClipRect == NULL)
		if(GetParent())
			GetParent()->GetAbsoluteRect(parent_rect);
		else
			GetAbsoluteRect(parent_rect);
	else
		parent_rect = *pClipRect;
		
	Frect			rect;
	GetAbsoluteRect	(rect);
	Frect			out_rect;


	//��������� �������� �� ����������� � ����
	if(rect.left>parent_rect.right || rect.right<parent_rect.left ||
		rect.top>parent_rect.bottom ||  rect.bottom<parent_rect.top)
	{
		Frect r;
		r.set(0.0f,0.0f,0.0f,0.0f);
		UIStaticItem.SetRect(r);
		return;
	}

	
	float out_x, out_y;
	out_x = rect.left;
	out_y = rect.top;

	// out_rect - ������������� ������� � ������� ����� ����������
	// �����������, ����������� � ������ ��������� ������������ �������������
	// ����, � ����� �������� �������������� �� �������� � ������������.

	out_rect.intersection(parent_rect,rect);
	out_rect.left	-= out_x;
	out_rect.top	-= out_y;
	out_rect.right	-= out_x;
	out_rect.bottom -= out_y;

	if( m_bStretchTexture )
		UIStaticItem.SetRect(out_rect);
	else{
		Frect r;
		r.x1 = out_rect.left;
		r.x2 = out_rect.right<UIStaticItem.GetOriginalRect().width()?
			out_rect.right:UIStaticItem.GetOriginalRect().width();

		r.y1 = out_rect.top;
		r.y2 = out_rect.bottom<UIStaticItem.GetOriginalRect().height()?
			out_rect.bottom:UIStaticItem.GetOriginalRect().height();

		UIStaticItem.SetRect(r);
	}
}

void CUIStatic::ClipperOn() 
{
	m_bClipper = true;

	TextureClipper(0, 0);
}

void CUIStatic::ClipperOff(CUIStaticItem& UIStaticItem)
{
	m_bClipper = false;

	Frect out_rect;

	out_rect.top =   0;
	out_rect.bottom = GetHeight();
	out_rect.left =  0;
	out_rect.right = GetWidth();
	
	Frect r;
	r.x1 = out_rect.left;
	r.x2 = out_rect.right<UIStaticItem.GetOriginalRect().width()?
		   out_rect.right:UIStaticItem.GetOriginalRect().width();

	r.y1 = out_rect.top;
	r.y2 = out_rect.bottom<UIStaticItem.GetOriginalRect().height()?
		   out_rect.bottom:UIStaticItem.GetOriginalRect().height();
	UIStaticItem.SetRect(r);
}

void CUIStatic::ClipperOff() 
{
	ClipperOff(m_UIStaticItem);
}

void  CUIStatic::SetShader(const ui_shader& sh)
{
	m_UIStaticItem.SetShader(sh);
}

LPCSTR CUIStatic::GetText(){
	static const char empty = 0;
	if (m_pLines)
		return m_pLines->GetText();
	else
		return &empty;
}

void CUIStatic::SetTextColor(u32 color){
	CREATE_LINES;
	m_pLines->SetTextColor(color);
}

u32 CUIStatic::GetTextColor(){
	CREATE_LINES;
	return m_pLines->GetTextColor();
}

u32& CUIStatic::GetTextColorRef(){
	return m_pLines->GetTextColorRef();
}

void CUIStatic::SetText(LPCSTR str)
{
	if (!str ) 
		return;
	CREATE_LINES;
	m_pLines->SetText(str);
}

void CUIStatic::SetTextColor(u32 color, E4States state){
	m_dwTextColor[state] = color;
	m_bUseTextColor[state] = true;
}

Frect CUIStatic::GetClipperRect()
{
	if (m_bClipper)
		return m_ClipRect;
	else
		return GetSelfClipRect();
}

Frect CUIStatic::GetSelfClipRect()
{
	Frect	r;
	if (m_bClipper)
	{
		r.set(GetUIStaticItem().GetRect());
		r.add(GetUIStaticItem().GetPosX(), GetUIStaticItem().GetPosY());
	}
	else
		r.set(0.0f, 0.0f, UI_BASE_WIDTH, UI_BASE_HEIGHT);

	return r;
}


//CGameFont::EAligment CUIStatic::GetTextAlign(){
//	return m_pLines->GetTextAlignment();
//}

CGameFont::EAligment CUIStatic::GetTextAlignment(){
	return m_pLines->GetTextAlignment();
}

//void CUIStatic::SetTextAlign(CGameFont::EAligment align){
//	CREATE_LINES;
//	m_pLines->SetTextAlignment(align);
//}

void CUIStatic::SetTextAlignment(CGameFont::EAligment align){
	CREATE_LINES;
	m_pLines->SetTextAlignment(align);
	m_pLines->GetFont()->SetAligment((CGameFont::EAligment)align);
}

void CUIStatic::SetVTextAlignment(EVTextAlignment al){
	CREATE_LINES;
	m_pLines->SetVTextAlignment(al);
}

void CUIStatic::SetTextAlign_script(u32 align)
{
	m_pLines->SetTextAlignment((CGameFont::EAligment)align);
	m_pLines->GetFont()->SetAligment((CGameFont::EAligment)align);
}

u32 CUIStatic::GetTextAlign_script()
{
	return static_cast<u32>(m_pLines->GetTextAlignment());
}

void CUIStatic::SetEllipsis(int pos, int indent)
{
	if(m_pLines)
		m_pLines->SetEllipsis(pos!=0);
}

void CUIStatic::SetClipRect(Frect r)
{
	m_ClipRect = r;
}

void CUIStatic::OnFocusReceive()
{
	inherited::OnFocusReceive();
	if (GetMessageTarget())
        GetMessageTarget()->SendMessage(this, STATIC_FOCUS_RECEIVED, NULL);
}

void CUIStatic::OnFocusLost(){

	inherited::OnFocusLost();
	if (GetMessageTarget())
		GetMessageTarget()->SendMessage(this, STATIC_FOCUS_LOST, NULL);
}

void CUIStatic::AdjustHeightToText(){
	m_pLines->SetWidth		(GetWidth());
	m_pLines->ParseText();
	SetHeight				(m_pLines->GetVisibleHeight());
}

void CUIStatic::AdjustWidthToText()
{
	float _len		= m_pLines->GetFont()->SizeOf_(m_pLines->GetText());
	UI()->ClientToScreenScaledWidth(_len);
	SetWidth		(_len);
}


void CUIStatic::SetTextST				(LPCSTR str_id)
{
	SetText					(*CStringTable().translate(str_id));
}

void CUIStatic::DrawHighlightedText(){
	Frect				rect;
	GetAbsoluteRect		(rect);
	u32 def_col			= m_pLines->GetTextColor();
	m_pLines->SetTextColor(m_HighlightColor);
	m_pLines->Draw(	rect.left + 0 + m_TextOffset.x, rect.top - 0 + m_TextOffset.y);
	m_pLines->SetTextColor(def_col);
}

bool CUIStatic::IsHighlightText()
{
	return m_bCursorOverWindow;
}
