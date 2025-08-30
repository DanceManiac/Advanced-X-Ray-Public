#include "stdafx.h"
#include "hudtarget.h"
#include "hudmanager.h"
#include "../xrEngine/gamemtllib.h"

#include "../xrEngine/Environment.h"
#include "../xrEngine/CustomHUD.h"
#include "Actor.h"
#include "level.h"
#include "game_cl_base.h"
#include "../xrEngine/igame_persistent.h"


#include "InventoryOwner.h"
#include "relation_registry.h"
#include "character_info.h"

#include "string_table.h"
#include "entity_alive.h"

#include "inventory_item.h"
#include "inventory.h"

#include "../Include/xrRender/UIRender.h"
#include "HudItem.h"
#include "Weapon.h"
#include "PDA.h"

#include "AdvancedXrayGameConstants.h"

#include "ui/UIMainIngameWnd.h"


u32	crosshair_type = 1;

#define C_DEFAULT	color_rgba(0xff,0xff,0xff,0x80)
#define C_SIZE		0.025f
#define NEAR_LIM	0.5f

#define SHOW_INFO_SPEED		0.5f
#define HIDE_INFO_SPEED		10.f


IC	float	recon_mindist	()		{
	return 2.f;
}
IC	float	recon_maxdist	()		{
	return 50.f;
}
IC	float	recon_minspeed	()		{
	return 0.5f;
}
IC	float	recon_maxspeed	()		{
	return 10.f;
}

CHUDTarget::CHUDTarget	()
{    
	fuzzyShowInfo		= 0.f;
	PP.RQ.range			= 0.f;

	hShader->create("hud\\cursor", "shaders\\ui_cursor\\crosshair_cursor");
	hShaderCrosshairBuild->create("hud\\cursor", "shaders\\ui_cursor\\crosshair_build");
	hShaderCrosshairLFOa->create("hud\\cursor", "shaders\\ui_cursor\\crosshair_lfo_1");
	hShaderCrosshairLFOb->create("hud\\cursor", "shaders\\ui_cursor\\crosshair_lfo_2");
	hShaderCrosshairLFOc->create("hud\\cursor", "shaders\\ui_cursor\\crosshair_lfo_3");

	PP.RQ.set				(NULL, 0.f, -1);

	Load				();
	m_bShowCrosshair	= false;
}

CHUDTarget::~CHUDTarget	()
{
}


void CHUDTarget::Load		()
{
	HUDCrosshair.Load();
}

void CHUDTarget::ShowCrosshair(bool b)
{
	m_bShowCrosshair = b;
}
//. fVisTransparencyFactor
float fCurrentPickPower;
ICF static BOOL pick_trace_callback(collide::rq_result& result, LPVOID params)
{
	SPickParam*	pp			= (SPickParam*)params;
//	collide::rq_result* RQ	= pp->RQ;
	++pp->pass;

	if(result.O)
	{	
		pp->RQ				= result;
		return FALSE;
	}else
	{
		//получить треугольник и узнать его материал
		CDB::TRI* T		= Level().ObjectSpace.GetStaticTris()+result.element;
		
		SGameMtl* mtl = GMLib.GetMaterialByIdx(T->material);
		pp->power		*= mtl->fVisTransparencyFactor;
		if(pp->power>0.4f)
		{
			return TRUE;
		}
//.		if (mtl->Flags.is(SGameMtl::flPassable)) 
//.			return TRUE;
	}
	pp->RQ					= result;
	return					FALSE;
}

void CHUDTarget::CursorOnFrame ()
{
	Fvector				p1,dir;

	p1					= Device.vCameraPosition;
	dir					= Device.vCameraDirection;
	
	// Render cursor
	if(Level().CurrentEntity())
	{
		PP.RQ.O			= 0; 
		PP.RQ.range		= g_pGamePersistent->Environment().CurrentEnv->far_plane*0.99f;
		PP.RQ.element		= -1;
		
		collide::ray_defs	RD(p1, dir, PP.RQ.range, CDB::OPT_CULL, collide::rqtBoth);
		RQR.r_clear			();
		VERIFY				(!fis_zero(RD.dir.square_magnitude()));
		
		PP.power			= 1.0f;
		PP.pass				= 0;

		if(Level().ObjectSpace.RayQuery(RQR,RD, pick_trace_callback, &PP, NULL, Level().CurrentEntity()))
			clamp			(PP.RQ.range, NEAR_LIM, PP.RQ.range);
	}

}

extern ENGINE_API BOOL g_bRendering; 
void CHUDTarget::Render()
{

	BOOL  b_do_rendering = ( psHUD_Flags.is(HUD_CROSSHAIR|HUD_CROSSHAIR_RT|HUD_CROSSHAIR_RT2) );
	
	if(!b_do_rendering)
		return;

	VERIFY		(g_bRendering);

	CActor* Actor = smart_cast<CActor*>(Level().CurrentEntity());
	if (!Actor)	return;

	Fvector p1 = Device.vCameraPosition;
	Fvector dir = Device.vCameraDirection;

	if (auto Wpn = smart_cast<CHudItem*>(Actor->inventory().ActiveItem()))
	{
		bool AllowedForThis = (Wpn != smart_cast<CHudItem*>(Actor->inventory().ItemFromSlot(BOLT_SLOT)) && Wpn != smart_cast<CHudItem*>(Actor->inventory().ItemFromSlot(GRENADE_SLOT)) && Wpn != smart_cast<CHudItem*>(Actor->inventory().ItemFromSlot(APPARATUS_SLOT)));

		if (AllowedForThis)
			Actor->g_fireParams(Wpn, p1, dir);
	}
	
	// Render cursor
	u32 C				= C_DEFAULT;
	
	//FVF::TL				PT;
	Fvector				p2;
	p2.mad				(p1,dir,PP.RQ.range);
	Fvector4			pt;
	Device.mFullTransform.transform(pt, p2);
	pt.y = -pt.y;
	//PT.transform		(p2,Device.mFullTransform);
	//float				di_size = C_SIZE/powf(PT.p.w,.2f);
	float				di_size = C_SIZE/powf(pt.w,.2f);
	
	CUIMainIngameWnd* pMaingame	= HUD().GetUI()->UIMainIngameWnd;
	CGameFont* F				= pMaingame->m_HudInfoFont;
	float hud_info_x			= pMaingame->hud_info_x * 0.025f;
	float hud_info_y			= pMaingame->hud_info_y * 0.025f;

	int hud_info_r_e			= pMaingame->hud_info_e.x;
	int hud_info_g_e			= pMaingame->hud_info_e.y;
	int hud_info_b_e			= pMaingame->hud_info_e.z;
	int hud_info_a_e			= pMaingame->hud_info_e.w;

	int hud_info_r_n			= pMaingame->hud_info_n.x;
	int hud_info_g_n			= pMaingame->hud_info_n.y;
	int hud_info_b_n			= pMaingame->hud_info_n.z;
	int hud_info_a_n			= pMaingame->hud_info_n.w;

	int hud_info_r_f			= pMaingame->hud_info_f.x;
	int hud_info_g_f			= pMaingame->hud_info_f.y;
	int hud_info_b_f			= pMaingame->hud_info_f.z;
	int hud_info_a_f			= pMaingame->hud_info_f.w;

	u32 C_ON_ENEMY		= color_rgba(hud_info_r_e, hud_info_g_e, hud_info_b_e, hud_info_a_e);
	u32 C_ON_NEUTRAL	= color_rgba(hud_info_r_n, hud_info_g_n, hud_info_b_n, hud_info_a_n);
	u32 C_ON_FRIEND		= color_rgba(hud_info_r_f, hud_info_g_f, hud_info_b_f, hud_info_a_f);

	F->SetAligment		(CGameFont::alCenter);
	F->OutSetI			(0.f + hud_info_x, 0.05f + hud_info_y);

	if (psHUD_Flags.test(HUD_CROSSHAIR_DIST))
		F->OutSkip		();

	if (psHUD_Flags.test(HUD_INFO))
	{ 
		if(PP.RQ.O && PP.RQ.O->getVisible())
		{
			CEntityAlive*	E		= smart_cast<CEntityAlive*>	(PP.RQ.O);
			CEntityAlive*	pCurEnt = smart_cast<CEntityAlive*>	(Level().CurrentEntity());
			PIItem			l_pI	= smart_cast<PIItem>		(PP.RQ.O);

			if (IsGameTypeSingle())
			{
				const auto our_inv_owner = smart_cast<CInventoryOwner*>(pCurEnt);
				const auto pda = Actor->GetPDA(); 
				if(!(pda && pda->m_bZoomed))
				{
					if (E && E->g_Alive() && !E->cast_base_monster())
					{
						CInventoryOwner* others_inv_owner	= smart_cast<CInventoryOwner*>(E);

						if(our_inv_owner && others_inv_owner){

							switch(RELATION_REGISTRY().GetRelationType(others_inv_owner, our_inv_owner))
							{
							case ALife::eRelationTypeEnemy:
								C = C_ON_ENEMY; break;
							case ALife::eRelationTypeNeutral:
								C = C_ON_NEUTRAL; break;
							case ALife::eRelationTypeFriend:
								C = C_ON_FRIEND; break;
							}

							if (fuzzyShowInfo>0.5f)
							{
								CStringTable	strtbl		;
								F->SetColor	(subst_alpha(C,u8(iFloor(255.f*(fuzzyShowInfo-0.5f)*2.f))));
								F->OutNext	("%s", *strtbl.translate(others_inv_owner->Name()) );
								F->OutNext	("%s", *strtbl.translate(others_inv_owner->CharacterInfo().Community().id()) );
							}
						}

						fuzzyShowInfo += SHOW_INFO_SPEED*Device.fTimeDelta;
					}
					else if (l_pI && our_inv_owner && PP.RQ.range < 2.0f*2.0f)
					{
						if (fuzzyShowInfo>0.5f && l_pI->NameItem())
						{
							float hud_info_item_x	= pMaingame->hud_info_item_x;
							float hud_info_item_y1	= pMaingame->hud_info_item_y_pos.x;
							float hud_info_item_y2	= pMaingame->hud_info_item_y_pos.y;
							float hud_info_item_y3	= pMaingame->hud_info_item_y_pos.z;
							int height = l_pI->GetInvGridRect().y2;
							float pos = hud_info_item_y1;
							F->SetColor	(subst_alpha(C,u8(iFloor(255.f*(fuzzyShowInfo-0.5f)*2.f))));
							if (height == 2)
								pos = hud_info_item_y2;
							else if (height == 3)
								pos = hud_info_item_y3; // Hrust: 4 cells by height is not normal)
							F->OutSetI(0.f + hud_info_x + hud_info_item_x, 0.05f + hud_info_y + pos);
							F->OutNext	("%s",l_pI->NameItem());
						}
						fuzzyShowInfo += SHOW_INFO_SPEED*Device.fTimeDelta;
					}
				}
			}
			else
			{
				if (E && (E->GetfHealth()>0))
				{
					if (pCurEnt && GameID() == eGameIDSingle)
					{
						if (GameID() == eGameIDDeathmatch)			C = C_ON_ENEMY;
						else
						{	
							if (E->g_Team() != pCurEnt->g_Team())	C = C_ON_ENEMY;
							else									C = C_ON_FRIEND;
						};
						if (PP.RQ.range >= recon_mindist() && PP.RQ.range <= recon_maxdist())
						{
							float ddist = (PP.RQ.range - recon_mindist())/(recon_maxdist() - recon_mindist());
							float dspeed = recon_minspeed() + (recon_maxspeed() - recon_minspeed())*ddist;
							fuzzyShowInfo += Device.fTimeDelta/dspeed;
						}else{
							if (PP.RQ.range < recon_mindist()) 
								fuzzyShowInfo += recon_minspeed()*Device.fTimeDelta;
							else 
								fuzzyShowInfo = 0;
						};

						if (fuzzyShowInfo>0.5f)
						{
							clamp(fuzzyShowInfo,0.f,1.f);
							int alpha_C = iFloor(255.f*(fuzzyShowInfo-0.5f)*2.f);
							u8 alpha_b	= u8(alpha_C & 0x00ff);
							F->SetColor	(subst_alpha(C,alpha_b));
							F->OutNext	("%s",*PP.RQ.O->cName());
						}
					}
				};
			};

		}else{
			fuzzyShowInfo -= HIDE_INFO_SPEED*Device.fTimeDelta;
		}
		clamp(fuzzyShowInfo,0.f,1.f);
	}

	if (psHUD_Flags.test(HUD_CROSSHAIR_DIST))
	{
		F->OutSetI		(0.f,0.05f);
		F->SetColor		(C);
		F->OutNext		("%4.1f - %4.2f - %d",PP.RQ.range, PP.power, PP.pass);
	}

	Fvector2 scr_size;
	scr_size.set(float(Device.dwWidth), float(Device.dwHeight));
	float size_x = scr_size.x * di_size;
	float size_y = scr_size.y * di_size;
		
	size_y = size_x;

	float w_2 = scr_size.x / 2.0f;
	float h_2 = scr_size.y / 2.0f;

	// Convert to screen coords
	float cx = (pt.x + 1) * w_2;
	float cy = (pt.y + 1) * h_2;

	auto Wpn = smart_cast<CWeapon*>(Actor->inventory().ActiveItem());

	if (Wpn && Wpn->IsLaserOn())
		return;

	auto pda = smart_cast<CPda*>(Actor->inventory().ActiveItem());
	if ((pda && pda->m_bZoomed) || GameConstants::GetHideHudOnMaster())
		return;

	//отрендерить кружочек или крестик
	if (!m_bShowCrosshair && crosshair_type == 1 || crosshair_type == 2 || crosshair_type == 3 || crosshair_type == 4 || crosshair_type == 5 || crosshair_type == 6)
	{
		UIRender->StartPrimitive(6, IUIRender::ptTriList, UI().m_currentPointType);

		//	TODO: return code back to indexed rendering since we use quads
		//	Tri 1
		UIRender->PushPoint(cx - size_x, cy + size_y, 0, C, 0, 1);
		UIRender->PushPoint(cx - size_x, cy - size_y, 0, C, 0, 0);
		UIRender->PushPoint(cx + size_x, cy + size_y, 0, C, 1, 1);
		//	Tri 2
		UIRender->PushPoint(cx + size_x, cy + size_y, 0, C, 1, 1);
		UIRender->PushPoint(cx - size_x, cy - size_y, 0, C, 0, 0);
		UIRender->PushPoint(cx + size_x, cy - size_y, 0, C, 1, 0);

		// unlock VB and Render it as triangle LIST

		if (crosshair_type == 2)
			UIRender->SetShader(*hShaderCrosshairBuild);

		if (crosshair_type == 3)
			UIRender->SetShader(*hShaderCrosshairLFOa);

		if (crosshair_type == 4)
			UIRender->SetShader(*hShaderCrosshairLFOb);

		if (crosshair_type == 5)
			UIRender->SetShader(*hShaderCrosshairLFOc);

		if (!m_bShowCrosshair && crosshair_type == 1 || crosshair_type == 6)
			UIRender->SetShader(*hShader);

		UIRender->FlushPrimitive();
	}
	else
	{
		//отрендерить прицел
		HUDCrosshair.cross_color = C;
		HUDCrosshair.OnRender(Fvector2{ cx, cy }, scr_size);
	}
}

void CHUDTarget::net_Relcase(CObject* O)
{
	if(PP.RQ.O == O)
		PP.RQ.O = NULL;

	RQR.r_clear	();
}
