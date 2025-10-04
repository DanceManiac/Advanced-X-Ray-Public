#include "stdafx.h"
#include "HUDManager.h"
#include "hudtarget.h"

#include "actor.h"
#include "../xrEngine/igame_level.h"
#include "clsid_game.h"
#include "GamePersistent.h"
#include "MainMenu.h"

#include "Car.h"
#include "Spectator.h"

u32	ui_hud_type;

CFontManager::CFontManager()
{
	Device.seqDeviceReset.Add(this,REG_PRIORITY_HIGH);

	m_all_fonts.push_back(&pFontMedium				);// used cpp
	m_all_fonts.push_back(&pFontDI					);// used cpp
	m_all_fonts.push_back(&pFontArial14				);// used xml
	m_all_fonts.push_back(&pFontArial21				);// used xml
	m_all_fonts.push_back(&pFontGraffiti19Russian	);
	m_all_fonts.push_back(&pFontGraffiti22Russian	);
	m_all_fonts.push_back(&pFontLetterica16Russian	);
	m_all_fonts.push_back(&pFontLetterica18Russian	);
	m_all_fonts.push_back(&pFontGraffiti32Russian	);
	m_all_fonts.push_back(&pFontGraffiti40Russian	);
	m_all_fonts.push_back(&pFontGraffiti50Russian	);
	m_all_fonts.push_back(&pFontLetterica25			);
	m_all_fonts.push_back(&pFontStat				);

	FONTS_VEC_IT it		= m_all_fonts.begin();
	FONTS_VEC_IT it_e	= m_all_fonts.end();
	for(;it!=it_e;++it)
		(**it) = NULL;

	InitializeFonts();

}

void CFontManager::InitializeFonts()
{

	InitializeFont(pFontMedium				,"hud_font_medium"				);
	InitializeFont(pFontDI					,"hud_font_di",					CGameFont::fsGradient|CGameFont::fsDeviceIndependent);
	InitializeFont(pFontArial14				,"ui_font_arial_14"				);
	InitializeFont(pFontArial21				,"ui_font_arial_21"				);
	InitializeFont(pFontGraffiti19Russian	,"ui_font_graffiti19_russian"	);
	InitializeFont(pFontGraffiti22Russian	,"ui_font_graffiti22_russian"	);
	InitializeFont(pFontLetterica16Russian	,"ui_font_letterica16_russian"	);
	InitializeFont(pFontLetterica18Russian	,"ui_font_letterica18_russian"	);
	InitializeFont(pFontGraffiti32Russian	,"ui_font_graff_32"				);
	InitializeFont(pFontGraffiti40Russian	,"ui_font_graff_40"				);
	InitializeFont(pFontGraffiti50Russian	,"ui_font_graff_50"				);
	InitializeFont(pFontLetterica25			,"ui_font_letter_25"			);
	InitializeFont(pFontStat				,"stat_font",					CGameFont::fsDeviceIndependent);

}

LPCSTR CFontManager::GetFontTexName (LPCSTR section)
{
	static char* tex_names[] = { "texture800", "texture", "texture1600", "texture2k", "texture4k" };
	int def_idx		= 1;//default 1024x768
	int idx			= def_idx;

	u32 h = Device.dwHeight;

	if(h<=600)		idx = 0;
	else if (h<1024)	idx = 1;
	else if (h<1200)	idx = 2;
	else if (h<1440)	idx = 3;
	else				idx = 4;

	while(idx>=0){
		if( pSettings->line_exist(section,tex_names[idx]) )
			return pSettings->r_string(section,tex_names[idx]);
		--idx;
	}
	return pSettings->r_string(section,tex_names[def_idx]);
}

void CFontManager::InitializeFont(CGameFont*& F, LPCSTR section, u32 flags)
{
	LPCSTR font_tex_name = GetFontTexName(section);
	R_ASSERT(font_tex_name);

	const char* sh_name = READ_IF_EXISTS(pSettings, r_string, section, "shader", "font");

	if(!F)
		F = xr_new<CGameFont>(sh_name, font_tex_name, flags);
	else
		F->Initialize(sh_name, font_tex_name);

	F->m_font_name = section;

	if (!(flags & CGameFont::fsDeviceIndependent))
	{
		if (pSettings->line_exist(section, "scale_x"))
		{
			F->SetWidthScale(pSettings->r_float(section, "scale_x"));
		}
		if (pSettings->line_exist(section, "scale_y"))
		{
			F->SetHeightScale(pSettings->r_float(section, "scale_y"));
		}
	}

	if (pSettings->line_exist(section, "font_shadow_enabled"))
		F->SetFontShadowEnabled(pSettings->r_bool(section, "font_shadow_enabled"));
		
	if (pSettings->line_exist(section, "font_shadow_for_black_text"))
		F->SetFontShadowForBlackText(pSettings->r_bool(section, "font_shadow_for_black_text"));
		
	if (pSettings->line_exist(section, "font_shadow_x"))
		F->SetFontShadowX(pSettings->r_float(section, "font_shadow_x"));
		
	if (pSettings->line_exist(section, "font_shadow_y"))
		F->SetFontShadowY(pSettings->r_float(section, "font_shadow_y"));

	if (pSettings->line_exist(section,"size")){
		float sz = pSettings->r_float(section,"size");
		if (flags&CGameFont::fsDeviceIndependent)	F->SetHeightI(sz);
		else										F->SetHeight(sz);
	}

	if (pSettings->line_exist(section,"interval"))
		F->SetInterval(pSettings->r_fvector2(section,"interval"));
}

CFontManager::~CFontManager()
{
	Device.seqDeviceReset.Remove(this);
	FONTS_VEC_IT it		= m_all_fonts.begin();
	FONTS_VEC_IT it_e	= m_all_fonts.end();
	for(;it!=it_e;++it)
		xr_delete(**it);
}

void CFontManager::Render()
{
	FONTS_VEC_IT it		= m_all_fonts.begin();
	FONTS_VEC_IT it_e	= m_all_fonts.end();
	for(;it!=it_e;++it)
		(**it)->OnRender			();
}
void CFontManager::OnDeviceReset()
{
	InitializeFonts();
}

//--------------------------------------------------------------------
CHUDManager::CHUDManager() : m_Renderable(true), pUI(NULL), m_pHUDTarget(xr_new<CHUDTarget>())
{ 
	OnDisconnected			();
}
//--------------------------------------------------------------------
CHUDManager::~CHUDManager()
{
	xr_delete			(pUI);
	xr_delete			(m_pHUDTarget);
	b_online			= false;
}

//--------------------------------------------------------------------

void CHUDManager::Load()
{
	if(pUI){
		pUI->Load			( pUI->UIGame() );
		return;
	}
	pUI					= xr_new<CUI> (this);
	pUI->Load			(NULL);
	OnDisconnected		();
}
//--------------------------------------------------------------------
void CHUDManager::OnFrame()
{
	ZoneScoped;

	if ( !m_Renderable )
	{
		return;
	}

	if(!b_online)					return;
	if (pUI) pUI->UIOnFrame();
	m_pHUDTarget->CursorOnFrame();
}
//--------------------------------------------------------------------

ENGINE_API extern float psHUD_FOV;

bool need_render_hud()
{
	CObject* O = (g_pGameLevel) ? g_pGameLevel->CurrentViewEntity() : nullptr;
	if (!O)
		return false;

	CActor* A = smart_cast<CActor*>(O);
	if (A && (!A->HUDview() || !A->g_Alive()))
		return false;

	if (smart_cast<CCar*>(O) || smart_cast<CSpectator*>(O))
		return false;

	return true;
}

void CHUDManager::Render_First()
{
	ZoneScoped;

	if ( !m_Renderable )
	{
		return;
	}

	if (!psHUD_Flags.is(HUD_WEAPON | HUD_WEAPON_RT))
		return;

	if (pUI == nullptr)
		return;

	if (!need_render_hud())
		return;

	// only shadow 
	CObject* O = g_pGameLevel->CurrentViewEntity();
	::Render->set_Invisible			(TRUE);
	::Render->set_Object			(O->H_Root());
	O->renderable_Render			();
	::Render->set_Invisible			(FALSE);
}

void CHUDManager::Render_Last()
{
	ZoneScoped;

	if (0==pUI)
		return;

	if (!psHUD_Flags.is(HUD_WEAPON | HUD_WEAPON_RT))
		return;

	CObject*	O					= g_pGameLevel->CurrentViewEntity();
	if (0==O)						return;
	CActor*		A					= smart_cast<CActor*> (O);
	if (A && !A->HUDview())			return;
	if(O->CLS_ID == CLSID_CAR)
		return;

	if(O->CLS_ID == CLSID_SPECTATOR)
		return;

	// hud itself
	::Render->set_HUD				(TRUE);
	::Render->set_Object			(O->H_Root());
	O->OnHUDDraw					(this);
	::Render->set_HUD				(FALSE);
}
extern void draw_wnds_rects();

#include "player_hud.h"
bool   CHUDManager::RenderActiveItemUIQuery()
{
	if ( !m_Renderable )
	{
		return false;
	}

	if (!psHUD_Flags.is(HUD_WEAPON | HUD_WEAPON_RT))
		return false;

	if (!need_render_hud())
		return false;

	return (g_player_hud && g_player_hud->render_item_ui_query());
}

void   CHUDManager::RenderActiveItemUI()
{
	g_player_hud->render_item_ui();
}
extern ENGINE_API BOOL bShowPauseString;
//отрисовка элементов интерфейса
#include "string_table.h"
void  CHUDManager::RenderUI()
{
	ZoneScoped;

	if (!m_Renderable)
		return;

	if (!b_online)
		return;

	BOOL bAlready					= FALSE;

	if (psHUD_Flags.is(HUD_DRAW | HUD_DRAW_RT))
		HitMarker.Render		();

	bAlready					= ! (pUI && !pUI->Render());
	UI().RenderFont				();

	if (psHUD_Flags.is(HUD_CROSSHAIR|HUD_CROSSHAIR_RT|HUD_CROSSHAIR_RT2) && !bAlready)	
		m_pHUDTarget->Render();

	draw_wnds_rects		();

	if (Device.Paused() && bShowPauseString)
	{
		CGameFont* pFont	= UI().Font().pFontGraffiti50Russian;
		pFont->SetColor		(0x80FF0000	);
		LPCSTR _str			= CStringTable().translate("st_game_paused").c_str();
		
		Fvector2			_pos;
		_pos.set			(UI_BASE_WIDTH/2.0f, UI_BASE_HEIGHT/2.0f);
		UI().ClientToScreenScaled(_pos);
		pFont->SetAligment	(CGameFont::alCenter);
		pFont->Out			(_pos.x, _pos.y, _str);
		pFont->OnRender		();
	}

}

void CHUDManager::OnEvent(EVENT E, u64 P1, u64 P2)
{
}

collide::rq_result&	CHUDManager::GetCurrentRayQuery	() 
{
	return m_pHUDTarget->RQ;
}

void CHUDManager::SetCrosshairDisp	(float dispf, float disps)
{	
	m_pHUDTarget->HUDCrosshair.SetDispersion(disps/*psHUD_Flags.test(HUD_CROSSHAIR_DYNAMIC) ? dispf : disps */);
}

void  CHUDManager::ShowCrosshair	(bool show)
{
	m_pHUDTarget->m_bShowCrosshair = show;
}


void CHUDManager::Hit(int idx, float power, const Fvector& dir)	
{
	HitMarker.Hit(idx, dir);
}

void CHUDManager::SetHitmarkType		(LPCSTR tex_name)
{
	HitMarker.InitShader				(tex_name);
}
#include "ui\UIMainInGameWnd.h"
void CHUDManager::OnScreenResolutionChanged()
{
	ZoneScoped;

	xr_delete							(pUI->UIMainIngameWnd);

	pUI->UIMainIngameWnd				= xr_new<CUIMainIngameWnd>	();
	pUI->UIMainIngameWnd->Init			();
	pUI->UnLoad							();
	pUI->Load							(pUI->UIGame());

	pUI->OnConnected					();
}

void CHUDManager::OnDisconnected()
{
	ZoneScoped;

//.	if(!b_online)			return;
	b_online				= false;
	if(pUI)
		Device.seqFrame.Remove	(pUI);
}

void CHUDManager::OnConnected()
{
	ZoneScoped;

	if(b_online)			return;
	b_online				= true;
	if(pUI){
		Device.seqFrame.Add	(pUI,REG_PRIORITY_LOW-1000);
	}
}

void CHUDManager::net_Relcase	(CObject *object)
{
	ZoneScoped;

	VERIFY						(m_pHUDTarget);
	m_pHUDTarget->net_Relcase	(object);
}

CDialogHolder* CurrentDialogHolder()
{
	if (MainMenu()->IsActive())
		return MainMenu();
	else
		return HUD().GetUI();
}
