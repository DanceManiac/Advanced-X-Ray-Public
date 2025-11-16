#include "pch_script.h"
#include "UIGameCustom.h"
#include "level.h"
#include "ui/UIXmlInit.h"
#include "ui/UIStatic.h"
#include "object_broker.h"
#include "string_table.h"
#include "Actor.h"
#include "ActorCondition.h"
#include "RadioactiveZone.h"
#include "CustomDetector.h"
#include "ai/monsters/basemonster/base_monster.h"

#include "InventoryOwner.h"
#include "ui/UIMainIngameWnd.h"
#include "ui/UIHudStatesWnd.h"
#include "ui/ui_arrow.h"
#include "ui/UIActorMenu.h"
#include "ui/UIPdaWnd.h"
#include "ui/UIMainIngameWnd.h"
#include "ui/UIMessagesWindow.h"
#include "ui/UIHudStatesWnd.h"
#include "actor.h"
#include "inventory.h"
#include "game_cl_base.h"

#include "../xrEngine/x_ray.h"
#include "PDA.h"
#include "AdvancedXrayGameConstants.h"
#include "ui\UICellItem.h" //Alundaio

#include <imgui.h>

EGameIDs ParseStringToGameType(LPCSTR str);

bool predicate_sort_stat(const SDrawStaticStruct* s1, const SDrawStaticStruct* s2) 
{
	return ( s1->IsActual() > s2->IsActual() );
}

struct predicate_find_stat 
{
	LPCSTR	m_id;
	predicate_find_stat(LPCSTR id):m_id(id)	{}
	bool	operator() (SDrawStaticStruct* s) 
	{
		return ( s->m_name==m_id );
	}
};

CUIGameCustom::CUIGameCustom()
:m_msgs_xml(NULL),m_ActorMenu(NULL),m_PdaMenu(NULL),m_window(NULL),UIMainIngameWnd(NULL),m_pMessagesWnd(NULL)
{
	ShowGameIndicators		(true);
	ShowCrosshair			(true);

	m_radia_self	= 0.0f;
	m_radia_hit		= 0.0f;

	for (int i = 0; i < ALife::infl_max_count; ++i)
	{
		m_zone_cur_power[i]		= 0.0f;
		m_zone_feel_radius[i]	= 1.0f;
	}
	m_zone_hit_type[ALife::infl_rad] = ALife::eHitTypeRadiation;
	m_zone_hit_type[ALife::infl_fire] = ALife::eHitTypeBurn;
	m_zone_hit_type[ALife::infl_acid] = ALife::eHitTypeChemicalBurn;
	m_zone_hit_type[ALife::infl_psi] = ALife::eHitTypeTelepatic;
	m_zone_hit_type[ALife::infl_electra] = ALife::eHitTypeShock;

	m_zone_feel_radius_max = 0.0f;
}
bool g_b_ClearGameCaptions = false;

CUIGameCustom::~CUIGameCustom()
{
	delete_data				(m_custom_statics);
	g_b_ClearGameCaptions	= false;
}

BOOL UIRedraw = false;

void CUIGameCustom::OnFrame() 
{
	if (UIRedraw)
	{
		m_msgs_xml->Load(CONFIG_PATH, UI_PATH, "ui_custom_msgs.xml");
		m_ActorMenu->Construct();
		m_PdaMenu->Init();
		UIMainIngameWnd->Init();

		UIRedraw = false;
	}

	CDialogHolder::OnFrame();
	st_vec_it it = m_custom_statics.begin();
	st_vec_it it_e = m_custom_statics.end();
	for(;it!=it_e;++it)
		(*it)->Update();

	std::sort(	it, it_e, predicate_sort_stat );

	
	for (auto it = m_custom_statics.begin(); it != m_custom_statics.end(); )
	{
		if (!(*it)->IsActual())
		{
			delete_data(*it);
			it = m_custom_statics.erase(it);
		}
		else
			++it;
	}
	
	if(g_b_ClearGameCaptions)
	{
		delete_data				(m_custom_statics);
		g_b_ClearGameCaptions	= false;
	}
	m_window->Update();

	//update windows
	if (GameIndicatorsShown() && psHUD_Flags.is(HUD_DRAW | HUD_DRAW_RT))
		UIMainIngameWnd->Update();

	m_pMessagesWnd->Update();

	if (!ActorMenu().IsShown())
		UpdateZones();
}

void CUIGameCustom::Render()
{
	st_vec_it it = m_custom_statics.begin();
	st_vec_it it_e = m_custom_statics.end();
	for(;it!=it_e;++it)
		(*it)->Draw();

	m_window->Draw();

	CEntity* pEntity = smart_cast<CEntity*>(Level().CurrentEntity());

	if (pEntity)
	{
		CActor* pActor			=	smart_cast<CActor*>(pEntity);
		CPda* pda				= pActor->GetPDA();
		if(pActor && pActor->HUDview() && pActor->g_Alive() && psHUD_Flags.is(HUD_WEAPON|HUD_WEAPON_RT|HUD_WEAPON_RT2))
		{
			u16 ISlot = pActor->inventory().FirstSlot();
			u16 ESlot = pActor->inventory().LastSlot();

			for( ; ISlot<=ESlot; ++ISlot)
			{
				PIItem itm			= pActor->inventory().ItemFromSlot(ISlot);
				if(itm && itm->render_item_ui_query())
					itm->render_item_ui();
			}
		}

		if (pda)
		{
			if (GameIndicatorsShown() && psHUD_Flags.is(HUD_DRAW | HUD_DRAW_RT) && !pda->m_bZoomed && !GameConstants::GetHideHudOnMaster())
				UIMainIngameWnd->Draw();
		}
		else
		{
			if (GameIndicatorsShown() && psHUD_Flags.is(HUD_DRAW | HUD_DRAW_RT) && !GameConstants::GetHideHudOnMaster())
				UIMainIngameWnd->Draw();
		}
	}

	m_pMessagesWnd->Draw();

	DoRenderDialogs();
}

SDrawStaticStruct* CUIGameCustom::AddCustomStatic(LPCSTR id, bool bSingleInstance)
{
	if(bSingleInstance)
	{
		st_vec::iterator it = std::find_if(m_custom_statics.begin(),m_custom_statics.end(), predicate_find_stat(id) );
		if(it!=m_custom_statics.end())
			return (*it);
	}
	
	if (!m_msgs_xml->NavigateToNode(id, 0))
	{
		Msg("! CUIGameCustom::AddCustomStatic: XML node not found: %s%s", id, m_msgs_xml->m_xml_file_name);
		return nullptr;
	}

	CUIXmlInit xml_init;
	m_custom_statics.push_back		( xr_new<SDrawStaticStruct>() );
	SDrawStaticStruct* sss			= m_custom_statics.back();

	sss->m_static					= xr_new<CUIStatic>();
	sss->m_name						= id;
	xml_init.InitStatic				(*m_msgs_xml, id, 0, sss->m_static);
	float ttl						= m_msgs_xml->ReadAttribFlt(id, 0, "ttl", -1);
	if(ttl>0.0f)
		sss->m_endTime				= Device.fTimeGlobal + ttl;

	return sss;
}

SDrawStaticStruct* CUIGameCustom::GetCustomStatic(LPCSTR id)
{
	st_vec::iterator it = std::find_if(m_custom_statics.begin(),m_custom_statics.end(), predicate_find_stat(id));
	if(it!=m_custom_statics.end())
		return (*it);

	return NULL;
}

void CUIGameCustom::RemoveCustomStatic(LPCSTR id)
{
	st_vec::iterator it = std::find_if(m_custom_statics.begin(),m_custom_statics.end(), predicate_find_stat(id) );
	if(it!=m_custom_statics.end())
	{
			delete_data				(*it);
		m_custom_statics.erase	(it);
	}
}

void CUIGameCustom::OnInventoryAction(PIItem item, u16 action_type)
{
	if ( m_ActorMenu->IsShown() )
		m_ActorMenu->OnInventoryAction( item, action_type );
}

#include "ui/UIGameTutorial.h"

extern CUISequencer* g_tutorial;
extern CUISequencer* g_tutorial2;

bool CUIGameCustom::ShowActorMenu()
{
	if ( m_ActorMenu->IsShown() )
	{
		m_ActorMenu->HideDialog();
	}
	else
	{
		if (!psActorFlags.test(AF_3D_PDA)) HidePdaMenu();

		//HidePdaMenu();
		CInventoryOwner* pIOActor	= smart_cast<CInventoryOwner*>( Level().CurrentViewEntity() );
		VERIFY						(pIOActor);
		m_ActorMenu->SetActor		(pIOActor);
		m_ActorMenu->SetMenuMode	(mmInventory);
		m_ActorMenu->ShowDialog		(true);
	}
	return true;
}

void CUIGameCustom::HideActorMenu()
{
	if ( m_ActorMenu->IsShown() )
	{
		m_ActorMenu->HideDialog();
	}
}

//Alundaio:
void CUIGameCustom::UpdateActorMenu()
{
	if (m_ActorMenu->IsShown())
	{
		m_ActorMenu->UpdateActor();
		m_ActorMenu->RefreshCurrentItemCell();
	}
}

CScriptGameObject* CUIGameCustom::CurrentItemAtCell()
{
	CUICellItem* itm = m_ActorMenu->CurrentItem();
	if (!itm->m_pData)
		return nullptr;

	PIItem IItm = static_cast<PIItem>(itm->m_pData);
	if (!IItm)
		return nullptr;

	CGameObject* GO = smart_cast<CGameObject*>(IItm);

	if (GO)
		return GO->lua_game_object();

	return nullptr;
}
//-Alundaio

void CUIGameCustom::HideMessagesWindow()
{
	if ( m_pMessagesWnd->IsShown() )
		m_pMessagesWnd->Show(false);
}

void CUIGameCustom::ShowMessagesWindow()
{
	if ( !m_pMessagesWnd->IsShown() )
		m_pMessagesWnd->Show(true);
}

bool CUIGameCustom::ShowPdaMenu()
{
	HideActorMenu();
	m_PdaMenu->ShowDialog(true);
	return true;
}

void CUIGameCustom::HidePdaMenu()
{
	if ( m_PdaMenu->IsShown() )
	{
		m_PdaMenu->HideDialog();
	}
}

void CUIGameCustom::SetClGame(game_cl_GameState* g)
{
	g->SetGameUI(this);
}

void CUIGameCustom::UnLoad()
{
	xr_delete					(m_msgs_xml);
	xr_delete					(m_ActorMenu);
	xr_delete					(m_PdaMenu);
	xr_delete					(m_window);
	xr_delete					(UIMainIngameWnd);
	xr_delete					(m_pMessagesWnd);
}

void CUIGameCustom::Load()
{
	ZoneScoped;

	if(g_pGameLevel)
	{
		R_ASSERT				(NULL==m_msgs_xml);
		m_msgs_xml				= xr_new<CUIXml>();
		m_msgs_xml->Load		(CONFIG_PATH, UI_PATH, "ui_custom_msgs.xml");

		R_ASSERT				(NULL==m_ActorMenu);
		m_ActorMenu				= xr_new<CUIActorMenu>		();

		R_ASSERT				(NULL==m_PdaMenu);
		m_PdaMenu				= xr_new<CUIPdaWnd>			();
		
		R_ASSERT				(NULL==m_window);
		m_window				= xr_new<CUIWindow>			();

		R_ASSERT				(NULL==UIMainIngameWnd);
		UIMainIngameWnd			= xr_new<CUIMainIngameWnd>	();
		UIMainIngameWnd->Init	();

		R_ASSERT				(NULL==m_pMessagesWnd);
		m_pMessagesWnd			= xr_new<CUIMessagesWindow>();
		
		Init					(0);
		Init					(1);
		Init					(2);
	}
}

void CUIGameCustom::OnConnected()
{
	if(g_pGameLevel)
	{
		if(!UIMainIngameWnd)
			Load();

		UIMainIngameWnd->OnConnected();
	}
}

void CUIGameCustom::CommonMessageOut(LPCSTR text)
{
	m_pMessagesWnd->AddLogMessage(text);
}
void CUIGameCustom::UpdatePda()
{
	PdaMenu().UpdatePda();
}

void CUIGameCustom::update_fake_indicators(u8 type, float power)
{
	UIMainIngameWnd->get_hud_states()->FakeUpdateIndicatorType(type, power);
}

void CUIGameCustom::enable_fake_indicators(bool enable)
{
	UIMainIngameWnd->get_hud_states()->EnableFakeIndicators(enable);
}

ALife::EInfluenceType CUIGameCustom::get_indik_type(ALife::EHitType hit_type)
{
	ALife::EInfluenceType iz_type = ALife::infl_max_count;
	switch (hit_type)
	{
	case ALife::eHitTypeRadiation:		iz_type = ALife::infl_rad;		break;
	case ALife::eHitTypeLightBurn:
	case ALife::eHitTypeBurn:			iz_type = ALife::infl_fire;		break;
	case ALife::eHitTypeChemicalBurn:	iz_type = ALife::infl_acid;		break;
	case ALife::eHitTypeTelepatic:		iz_type = ALife::infl_psi;		break;
	case ALife::eHitTypeShock:			iz_type = ALife::infl_electra;	break;// it hasnt CStatic

	case ALife::eHitTypeStrike:
	case ALife::eHitTypeWound:
	case ALife::eHitTypeExplosion:
	case ALife::eHitTypeFireWound:
	case ALife::eHitTypeWound_2:
		//	case ALife::eHitTypePhysicStrike:
		return ALife::infl_max_count;
	default:
		NODEFAULT;
	}
	return iz_type;
}

// ------------------------------------------------------------------------------------------------

void CUIGameCustom::UpdateZones()
{
	//float actor_radia = m_actor->conditions().GetRadiation() * m_actor_radia_factor;
	//m_radia_hit = _max( m_zone_cur_power[it_rad], actor_radia );

	CActor* actor = smart_cast<CActor*>(Level().CurrentViewEntity());
	if (!actor)
	{
		return;
	}
	CPda* const pda = actor->GetPDA();
	if (pda)
	{
		typedef xr_vector<CObject*>	monsters;
		for (monsters::const_iterator it = pda->feel_touch.begin();
			it != pda->feel_touch.end(); ++it)
		{
			CBaseMonster* const	monster = smart_cast<CBaseMonster*>(*it);
			if (!monster || !monster->g_Alive())
				continue;

			monster->play_detector_sound();
		}
	}


	m_radia_self = actor->conditions().GetRadiation();

	float zone_max_power = actor->conditions().GetZoneMaxPower(ALife::infl_rad);
	float power = actor->conditions().GetInjuriousMaterialDamage();
	power = power / zone_max_power;
	clamp(power, 0.0f, 1.1f);
	if (m_zone_cur_power[ALife::infl_rad] < power)
	{
		m_zone_cur_power[ALife::infl_rad] = power;
	}
	m_radia_hit = m_zone_cur_power[ALife::infl_rad];

	/*	if ( Device.dwFrame % 20 == 0 )
		{
			Msg(" self = %.2f   hit = %.2f", m_radia_self, m_radia_hit );
		}*/

		//	m_arrow->SetNewValue( m_radia_hit );
		//	m_arrow_shadow->SetPos( m_arrow->GetPos() );
		/*
			power = actor->conditions().GetPsy();
			clamp( power, 0.0f, 1.1f );
			if ( m_zone_cur_power[ALife::infl_psi] < power )
			{
				m_zone_cur_power[ALife::infl_psi] = power;
			}
		*/
	if (!Level().hud_zones_list)
	{
		return;
	}

	for (int i = 0; i < ALife::infl_max_count; ++i)
	{
		if (Device.fTimeDelta < 1.0f)
		{
			m_zone_cur_power[i] *= 0.9f * (1.0f - Device.fTimeDelta);
		}
		if (m_zone_cur_power[i] < 0.01f)
		{
			m_zone_cur_power[i] = 0.0f;
		}
	}

	Fvector posf;
	posf.set(Level().CurrentControlEntity()->Position());
	Level().hud_zones_list->feel_touch_update(posf, m_zone_feel_radius_max);

	if (Level().hud_zones_list->m_ItemInfos.size() == 0)
	{
		return;
	}

	CZoneList::ItemsMapIt itb = Level().hud_zones_list->m_ItemInfos.begin();
	CZoneList::ItemsMapIt ite = Level().hud_zones_list->m_ItemInfos.end();
	for (; itb != ite; ++itb)
	{
		CCustomZone* pZone = itb->first;
		ITEM_INFO& zone_info = itb->second;
		ITEM_TYPE* zone_type = zone_info.curr_ref;

		ALife::EHitType			hit_type = pZone->GetHitType();
		ALife::EInfluenceType	z_type = get_indik_type(hit_type);
		/*		if ( z_type == indik_type_max )
				{
					continue;
				}
		*/

		Fvector P = Level().CurrentControlEntity()->Position();
		P.y -= 0.5f;
		float dist_to_zone = 0.0f;
		float rad_zone = 0.0f;
		pZone->CalcDistanceTo(P, dist_to_zone, rad_zone);
		clamp(dist_to_zone, 0.0f, flt_max * 0.5f);

		float fRelPow = (dist_to_zone / (rad_zone + (z_type == ALife::infl_max_count) ? 5.0f : m_zone_feel_radius[z_type] + 0.1f)) - 0.1f;

		zone_max_power = actor->conditions().GetZoneMaxPower(z_type);
		power = pZone->Power(dist_to_zone, rad_zone);
		//power = power / zone_max_power;
		clamp(power, 0.0f, 1.1f);

		if ((z_type != ALife::infl_max_count) && (m_zone_cur_power[z_type] < power)) //max
		{
			m_zone_cur_power[z_type] = power;
		}

		if (dist_to_zone < rad_zone + 0.9f * ((z_type == ALife::infl_max_count) ? 5.0f : m_zone_feel_radius[z_type]))
		{
			fRelPow *= 0.6f;
			if (dist_to_zone < rad_zone)
			{
				fRelPow *= 0.3f;
				fRelPow *= (2.5f - 2.0f * power);
			}
		}
		clamp(fRelPow, 0.0f, 1.0f);

		zone_info.cur_period = zone_type->freq.x + (zone_type->freq.y - zone_type->freq.x) * (fRelPow * fRelPow);

		//string256	buff_z;
		//xr_sprintf( buff_z, "zone %2.2f\n", zone_info.cur_period );
		//xr_strcat( buff, buff_z );
		if (zone_info.snd_time > zone_info.cur_period)
		{
			zone_info.snd_time = 0.0f;

			if (!GameConstants::GetDosimeterSlotEnabled())
				HUD_SOUND_ITEM::PlaySound(zone_type->detect_snds, Fvector().set(0, 0, 0), NULL, true, false);
		}
		else
		{
			zone_info.snd_time += Device.fTimeDelta;
		}
	} // for itb
}

float CUIGameCustom::get_zone_cur_power(ALife::EHitType hit_type)
{
	ALife::EInfluenceType iz_type = get_indik_type(hit_type);
	if (iz_type == ALife::infl_max_count)
	{
		return 0.0f;
	}
	return m_zone_cur_power[iz_type];
}

SDrawStaticStruct::SDrawStaticStruct	()
{
	m_static	= NULL;
	m_endTime	= -1.0f;	
}

void SDrawStaticStruct::destroy()
{
	delete_data(m_static);
}

bool SDrawStaticStruct::IsActual() const
{
	if(m_endTime<0)			return true;
	return (Device.fTimeGlobal < m_endTime);
}

void SDrawStaticStruct::SetText(LPCSTR text)
{
	m_static->Show(text!=NULL);
	if(text)
	{
		m_static->TextItemControl()->SetTextST(text);
		m_static->ResetColorAnimation();
	}
}

void SDrawStaticStruct::Draw()
{
	if(m_static && m_static->IsShown())
		m_static->Draw();
}

void SDrawStaticStruct::Update()
{
	if(IsActual() && m_static->IsShown())	
		m_static->Update();
}

CMapListHelper	gMapListHelper;
xr_token		game_types[];

void CMapListHelper::LoadMapInfo(LPCSTR map_cfg_fn, const xr_string& map_name, LPCSTR map_ver)
{
	CInifile	ini				(map_cfg_fn);

	shared_str _map_name		= map_name.substr(0,map_name.find('\\')).c_str();
	shared_str _map_ver			= map_ver;

	if(ini.section_exist("map_usage"))
	{
		if(ini.line_exist("map_usage","ver") && !map_ver)
			_map_ver				= ini.r_string("map_usage", "ver");

		CInifile::Sect S			= ini.r_section("map_usage");
		CInifile::SectCIt si		= S.Data.begin();
		CInifile::SectCIt si_e		= S.Data.end();
		for( ;si!=si_e; ++si)
		{
			const shared_str& game_type = (*si).first;
			
			if(game_type=="ver")		continue;

			SGameTypeMaps* M			= GetMapListInt(game_type);
			if(!M)
			{
				Msg						("--unknown game type-%s",game_type.c_str());
				m_storage.resize		(m_storage.size()+1);
				SGameTypeMaps&	Itm		= m_storage.back();
				Itm.m_game_type_name	= game_type;
				Itm.m_game_type_id		= ParseStringToGameType(game_type.c_str());
				M						= &m_storage.back();
			}
			
			SGameTypeMaps::SMapItm	Itm;
			Itm.map_name				= _map_name;
			Itm.map_ver					= _map_ver;
			
			if(M->m_map_names.end()!=std::find(M->m_map_names.begin(),M->m_map_names.end(),Itm))
			{
				Msg("! duplicate map found [%s] [%s]", _map_name.c_str(), _map_ver.c_str());
			}else
			{
#ifndef MASTER_GOLD
				Msg("added map [%s] [%s]", _map_name.c_str(), _map_ver.c_str());
#endif // #ifndef MASTER_GOLD
				M->m_map_names.push_back	(Itm);
			}
		}			
	}

}

void CMapListHelper::Load()
{
	string_path					fn;
	FS.update_path				(fn, "$game_config$", "mp\\map_list.ltx");
	CInifile map_list_cfg		(fn);

	//read weathers set
	CInifile::Sect w			= map_list_cfg.r_section("weather");
	CInifile::SectCIt wi		= w.Data.begin();
	CInifile::SectCIt wi_e		= w.Data.end();
	for( ;wi!=wi_e; ++wi)
	{
		m_weathers.resize		(m_weathers.size()+1);
		SGameWeathers& gw		= m_weathers.back();
		gw.m_weather_name		= (*wi).first;
		gw.m_start_time			= (*wi).second;
	}

	// scan for additional maps
	FS_FileSet			fset;
	FS.file_list		(fset,"$game_levels$",FS_ListFiles,"*level.ltx");

	FS_FileSetIt fit	= fset.begin();
	FS_FileSetIt fit_e	= fset.end();

	for( ;fit!=fit_e; ++fit)
	{
		string_path					map_cfg_fn;
		FS.update_path				(map_cfg_fn, "$game_levels$", (*fit).name.c_str());
		LoadMapInfo					(map_cfg_fn, (*fit).name);
	}
	//scan all not laoded archieves
	LPCSTR tmp_entrypoint			= "temporary_gamedata\\";
	FS_Path* game_levels			= FS.get_path("$game_levels$");
	xr_string prev_root				= game_levels->m_Root;
	game_levels->_set_root			(tmp_entrypoint);

	CLocatorAPI::archives_it it		= FS.m_archives.begin();
	CLocatorAPI::archives_it it_e	= FS.m_archives.end();

	for(;it!=it_e;++it)
	{
		CLocatorAPI::archive& A		= *it;
		if(A.hSrcFile)				continue;

		LPCSTR ln					= A.header->r_string("header", "level_name");
		LPCSTR lv					= A.header->r_string("header", "level_ver");
		FS.LoadArchive				(A, tmp_entrypoint);

		string_path					map_cfg_fn;
		FS.update_path				(map_cfg_fn, "$game_levels$", ln);

		
		xr_strcat					(map_cfg_fn,"\\level.ltx");
		LoadMapInfo					(map_cfg_fn, ln, lv);
		FS.unload_archive			(A);
	}
	game_levels->_set_root			(prev_root.c_str());


	R_ASSERT2	(m_storage.size(), "unable to fill map list");
	R_ASSERT2	(m_weathers.size(), "unable to fill weathers list");
}


const SGameTypeMaps& CMapListHelper::GetMapListFor(const shared_str& game_type)
{
	if( !m_storage.size() )
		Load		();

	return *GetMapListInt(game_type);
}

SGameTypeMaps* CMapListHelper::GetMapListInt(const shared_str& game_type)
{

	TSTORAGE_CIT it		= m_storage.begin();
	TSTORAGE_CIT it_e	= m_storage.end();
	for( ;it!=it_e; ++it)
	{
		if(game_type==(*it).m_game_type_name )
			return &(*it);
	}
	return NULL;
}

const SGameTypeMaps& CMapListHelper::GetMapListFor(const EGameIDs game_id)
{
	if( !m_storage.size() )
	{
		Load		();
		R_ASSERT2	(m_storage.size(), "unable to fill map list");
	}
	TSTORAGE_CIT it		= m_storage.begin();
	TSTORAGE_CIT it_e	= m_storage.end();
	for( ;it!=it_e; ++it)
	{
		if(game_id==(*it).m_game_type_id )
			return (*it);
	}
	return m_storage[0];
}

const GAME_WEATHERS& CMapListHelper::GetGameWeathers() 
{
	if(!m_weathers.size())
		Load();

	return m_weathers;
}

bool CUIGameCustom::FillDebugTree(const CUIDebugState& debugState)
{
#ifndef MASTER_GOLD
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
	
	if (debugState.selected == this)
		flags |= ImGuiTreeNodeFlags_Selected;

	const bool open = ImGui::TreeNodeEx(this, flags, "Game UI (%s)", CUIGameCustom::GetDebugType());
	
	if (ImGui::IsItemClicked())
		debugState.select(this);

	if (open)
	{
		CDialogHolder::FillDebugTree(debugState);

		m_window->FillDebugTree(debugState);
		ActorMenu().FillDebugTree(debugState);
		PdaMenu().FillDebugTree(debugState);
		UIMainIngameWnd->FillDebugTree(debugState);
		m_pMessagesWnd->FillDebugTree(debugState);

		for (const auto& custom_static : m_custom_statics)
		{
			if (custom_static)
				custom_static->wnd()->FillDebugTree(debugState);
		}
		ImGui::TreePop();
	}

	return open;
#else
	return false;
#endif
}

void CUIGameCustom::FillDebugInfo()
{
#ifndef MASTER_GOLD
	CDialogHolder::FillDebugInfo();
	if (ImGui::CollapsingHeader(CUIGameCustom::GetDebugType()))
	{
		ImGui::Checkbox(toUtf8(CStringTable().translate("st_editor_imgui_show_game_indicators").c_str()).c_str(), &m_bShowGameIndicators);
	}
#endif
}