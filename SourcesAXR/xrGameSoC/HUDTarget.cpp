// exxZERO Time Stamp AddIn. Document modified at : Thursday, March 07, 2002 14:13:00 , by user : Oles , from computer : OLES
// HUDCursor.cpp: implementation of the CHUDTarget class.
//
//////////////////////////////////////////////////////////////////////

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
#include "HudItem.h"
#include "Weapon.h"
#include "PDA.h"

#include "AdvancedXrayGameConstants.h"

#include "ui/UIMainIngameWnd.h"

#define C_DEFAULT	color_xrgb(0xff,0xff,0xff)
#define C_SIZE		0.025f
#define NEAR_LIM	0.5f

#define SHOW_INFO_SPEED		0.5f
#define HIDE_INFO_SPEED		10.f

u32	crosshair_type = 1;

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

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHUDTarget::CHUDTarget	()
{    
	fuzzyShowInfo		= 0.f;
	RQ.range			= 0.f;
	//	hGeom.create		(FVF::F_TL, RCache.Vertex.Buffer(), RCache.QuadIB);
	hShader->create("hud\\cursor", "ui\\cursor");
	hShaderCrosshairPoint->create("hud\\cursor", "ui\\crosshair_cop_point");

	RQ.set				(NULL, 0.f, -1);

	Load				();
	m_bShowCrosshair	= false;
}

void CHUDTarget::net_Relcase(CObject* O)
{
	if(RQ.O == O)
		RQ.O = NULL;

	RQR.r_clear	();
}

void CHUDTarget::Load		()
{
	HUDCrosshair.Load();
}

ICF static BOOL pick_trace_callback(collide::rq_result& result, LPVOID params)
{
	collide::rq_result* RQ = (collide::rq_result*)params;
	if(result.O){	
		*RQ				= result;
		return FALSE;
	}else{
		//получить треугольник и узнать его материал
		CDB::TRI* T		= Level().ObjectSpace.GetStaticTris()+result.element;
		if (GMLib.GetMaterialByIdx(T->material)->Flags.is(SGameMtl::flPassable))
			return TRUE;
	}
	*RQ					= result;
	return FALSE;
}

void CHUDTarget::CursorOnFrame ()
{
	Fvector				p1,dir;

	p1					= Device.vCameraPosition;
	dir					= Device.vCameraDirection;
	
	// Render cursor
	if(Level().CurrentEntity()){
		RQ.O			= 0; 
		RQ.range		= g_pGamePersistent->Environment().CurrentEnv->far_plane*0.99f;
		RQ.element		= -1;
		
		collide::ray_defs	RD(p1, dir, RQ.range, CDB::OPT_CULL, collide::rqtBoth);
		RQR.r_clear			();
		VERIFY				(!fis_zero(RD.dir.square_magnitude()));
		if(Level().ObjectSpace.RayQuery(RQR,RD, pick_trace_callback, &RQ, NULL, Level().CurrentEntity()))
			clamp			(RQ.range,NEAR_LIM,RQ.range);
	}

}

extern ENGINE_API BOOL g_bRendering; 
void CHUDTarget::Render()
{
	VERIFY		(g_bRendering);

	CActor* Actor			= smart_cast<CActor*>(Level().CurrentEntity());
	
	if (!Actor)
		return;

	Fvector p1				= Device.vCameraPosition;
	Fvector dir				= Device.vCameraDirection;

	if (auto Wpn = smart_cast<CHudItem*>(Actor->inventory().ActiveItem()))
	{
		bool AllowedForThis = (Wpn != smart_cast<CHudItem*>(Actor->inventory().ItemFromSlot(BOLT_SLOT)) && Wpn != smart_cast<CHudItem*>(Actor->inventory().ItemFromSlot(GRENADE_SLOT)) && Wpn != smart_cast<CHudItem*>(Actor->inventory().ItemFromSlot(APPARATUS_SLOT)));

		if (AllowedForThis)
			Actor->g_fireParams(Wpn, p1, dir);
	}
	
	// Render cursor
	u32 C				= C_DEFAULT;

	Fvector				p2;
	p2.mad				(p1,dir,RQ.range);
	Fvector4			pt;
	Device.mFullTransform.transform(pt, p2);
	pt.y				= -pt.y;
	float				di_size = C_SIZE / powf(pt.w, .2f);

	float hud_info_x	= HUD().GetUI()->UIMainIngameWnd->hud_info_x * 0.025f;
	float hud_info_y	= HUD().GetUI()->UIMainIngameWnd->hud_info_y * 0.025f;

	int hud_info_r_e	= HUD().GetUI()->UIMainIngameWnd->hud_info_r_e;
	int hud_info_g_e	= HUD().GetUI()->UIMainIngameWnd->hud_info_g_e;
	int hud_info_b_e	= HUD().GetUI()->UIMainIngameWnd->hud_info_b_e;
	int hud_info_a_e	= HUD().GetUI()->UIMainIngameWnd->hud_info_a_e;

	int hud_info_r_n	= HUD().GetUI()->UIMainIngameWnd->hud_info_r_n;
	int hud_info_g_n	= HUD().GetUI()->UIMainIngameWnd->hud_info_g_n;
	int hud_info_b_n	= HUD().GetUI()->UIMainIngameWnd->hud_info_b_n;
	int hud_info_a_n	= HUD().GetUI()->UIMainIngameWnd->hud_info_a_n;

	int hud_info_r_f	= HUD().GetUI()->UIMainIngameWnd->hud_info_r_f;
	int hud_info_g_f	= HUD().GetUI()->UIMainIngameWnd->hud_info_g_f;
	int hud_info_b_f	= HUD().GetUI()->UIMainIngameWnd->hud_info_b_f;
	int hud_info_a_f	= HUD().GetUI()->UIMainIngameWnd->hud_info_a_f;

	u32 C_ON_ENEMY		= color_rgba(hud_info_r_e, hud_info_g_e, hud_info_b_e, hud_info_a_e);
	u32 C_ON_NEUTRAL	= color_rgba(hud_info_r_n, hud_info_g_n, hud_info_b_n, hud_info_a_n);
	u32 C_ON_FRIEND		= color_rgba(hud_info_r_f, hud_info_g_f, hud_info_b_f, hud_info_a_f);

	CGameFont* F		= HUD().GetUI()->UIMainIngameWnd->m_HudInfoFont;
	F->SetAligment		(CGameFont::alCenter);
	F->OutSetI			(0.f + hud_info_x, 0.05f + hud_info_y);

	if (psHUD_Flags.test(HUD_CROSSHAIR_DIST)){
		F->SetColor		(C);
		F->OutNext		("%4.1f",RQ.range);
	}

	if (psHUD_Flags.test(HUD_INFO)){ 
		if (RQ.O){
			CEntityAlive*	E		= smart_cast<CEntityAlive*>	(RQ.O);
			CEntityAlive*	pCurEnt = smart_cast<CEntityAlive*>	(Level().CurrentEntity());
			PIItem			l_pI	= smart_cast<PIItem>		(RQ.O);

			if (IsGameTypeSingle())
			{
				const auto our_inv_owner = smart_cast<CInventoryOwner*>(pCurEnt);
				const auto pda = Actor->GetPDA();

				if (!(pda && pda->m_bZoomed))
				{
					if (E && E->g_Alive() && !E->cast_base_monster())
					{
						CInventoryOwner* others_inv_owner = smart_cast<CInventoryOwner*>(E);

						if (our_inv_owner && others_inv_owner) {

							switch (RELATION_REGISTRY().GetRelationType(others_inv_owner, our_inv_owner))
							{
							case ALife::eRelationTypeEnemy:
								C = C_ON_ENEMY; break;
							case ALife::eRelationTypeNeutral:
								C = C_ON_NEUTRAL; break;
							case ALife::eRelationTypeFriend:
								C = C_ON_FRIEND; break;
							}

							if (fuzzyShowInfo > 0.5f)
							{
								CStringTable	strtbl;
								F->SetColor(subst_alpha(C, u8(iFloor(255.f * (fuzzyShowInfo - 0.5f) * 2.f))));
								F->OutNext("%s", *strtbl.translate(others_inv_owner->Name()));
								F->OutNext("%s", *strtbl.translate(others_inv_owner->CharacterInfo().Community().id()));
							}
						}

						fuzzyShowInfo += SHOW_INFO_SPEED * Device.fTimeDelta;
					}
					else if (l_pI && our_inv_owner && RQ.range < 2.0f * our_inv_owner->inventory().GetTakeDist())
					{
						if (fuzzyShowInfo > 0.5f) {
							float hud_info_item_x = HUD().GetUI()->UIMainIngameWnd->hud_info_item_x;
							float hud_info_item_y1 = HUD().GetUI()->UIMainIngameWnd->hud_info_item_y1;
							float hud_info_item_y2 = HUD().GetUI()->UIMainIngameWnd->hud_info_item_y2;
							float hud_info_item_y3 = HUD().GetUI()->UIMainIngameWnd->hud_info_item_y3;
							int height = l_pI->GetInvGridRect().y2;
							float pos = hud_info_item_y1;
							F->SetColor(subst_alpha(C, u8(iFloor(255.f * (fuzzyShowInfo - 0.5f) * 2.f))));
							if (height == 2)
								pos = hud_info_item_y2;
							else if (height == 3)
								pos = hud_info_item_y3; // Hrust: 4 cells by height is not normal)
							F->OutSetI(0.f + hud_info_x + hud_info_item_x, 0.05f + hud_info_y + pos);
							F->OutNext("%s", l_pI->Name/*Complex*/());
						}
						fuzzyShowInfo += SHOW_INFO_SPEED * Device.fTimeDelta;
					}
				}
			}
			else
			{
				if (E && (E->GetfHealth()>0))
				{
					if (pCurEnt && GameID() == GAME_SINGLE){	
						if (GameID() == GAME_DEATHMATCH)			C = C_ON_ENEMY;
						else{	
							if (E->g_Team() != pCurEnt->g_Team())	C = C_ON_ENEMY;
							else									C = C_ON_FRIEND;
						};
						if (RQ.range >= recon_mindist() && RQ.range <= recon_maxdist()){
							float ddist = (RQ.range - recon_mindist())/(recon_maxdist() - recon_mindist());
							float dspeed = recon_minspeed() + (recon_maxspeed() - recon_minspeed())*ddist;
							fuzzyShowInfo += Device.fTimeDelta/dspeed;
						}else{
							if (RQ.range < recon_mindist()) fuzzyShowInfo += recon_minspeed()*Device.fTimeDelta;
							else fuzzyShowInfo = 0;
						};

						if (fuzzyShowInfo>0.5f){
							clamp(fuzzyShowInfo,0.f,1.f);
							int alpha_C = iFloor(255.f*(fuzzyShowInfo-0.5f)*2.f);
							u8 alpha_b	= u8(alpha_C & 0x00ff);
							F->SetColor	(subst_alpha(C,alpha_b));
							F->OutNext	("%s",*RQ.O->cName());
						}
					}
				};
			};

		}else{
			fuzzyShowInfo -= HIDE_INFO_SPEED*Device.fTimeDelta;
		}
		clamp(fuzzyShowInfo,0.f,1.f);
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

	if (GameConstants::GetHideHudOnMaster())
		return;

	auto Wpn = smart_cast<CWeapon*>(Actor->inventory().ActiveItem());

	if (Wpn && Wpn->IsLaserOn())
		return;

	auto pda = smart_cast<CPda*>(Actor->inventory().ActiveItem());
	if ((pda && pda->m_bZoomed) || GameConstants::GetHideHudOnMaster())
		return;

	//отрендерить кружочек или крестик
	if (!m_bShowCrosshair &&  crosshair_type == 1 || crosshair_type == 2 || crosshair_type == 3)
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

		if (crosshair_type == 3)
			UIRender->SetShader(*hShaderCrosshairPoint);

		if (!m_bShowCrosshair &&  crosshair_type == 1 || crosshair_type == 2)
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

