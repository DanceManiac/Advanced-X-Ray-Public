#include "stdafx.h"
//#include "../xrEngine/resourcemanager.h"
//#include "../xrEngine/resourcemanager.h"
#include "HUDmanager.h"
#include "PHdynamicdata.h"
#include "Physics.h"
#include "level.h"
#include "../xrEngine/x_ray.h"
#include "../xrEngine/igame_persistent.h"
#include "PhysicsGamePars.h"
#include "ai_space.h"
#include "game_cl_base.h"
#include "NET_Queue.h"
#include "file_transfer.h"

extern	pureFrame*				g_pNetProcessor;

BOOL CLevel::net_Start_client	( LPCSTR options )
{
	return FALSE;
}
#include "string_table.h"
bool	CLevel::net_start_client1				()
{
	pApp->LoadBegin	();
	// name_of_server
	string64					name_of_server = "";
//	strcpy_s						(name_of_server,*m_caClientOptions);
	if (strchr(*m_caClientOptions, '/'))
		strncpy(name_of_server,*m_caClientOptions, strchr(*m_caClientOptions, '/')-*m_caClientOptions);

	if (strchr(name_of_server,'/'))	*strchr(name_of_server,'/') = 0;

	// Startup client
	string256					temp;
	sprintf_s						(temp,"%s %s",
								CStringTable().translate("st_client_connecting_to").c_str(), name_of_server);

	g_pGamePersistent->LoadTitle				(temp);
	return true;
}

#include "xrServer.h"

bool	CLevel::net_start_client2				()
{
	if(psNET_direct_connect)
	{
		Server->create_direct_client();
	}

	connected_to_server = Connect2Server(*m_caClientOptions);

	return true;
}
void rescan_mp_archives()
{
	FS_Path* mp_archs_path = FS.get_path("$game_arch_mp$");
	CLocatorAPI* RealFS = dynamic_cast<CLocatorAPI*>(xr_FS);
	VERIFY(RealFS);
	RealFS->rescan_path(mp_archs_path->m_Path,
		mp_archs_path->m_Flags.is(FS_Path::flRecurse)
	);
}

bool	CLevel::net_start_client3				()
{
	if(connected_to_server)
	{
		LPCSTR					level_name = NULL;
		LPCSTR					level_ver = NULL;
		LPCSTR					download_url = NULL;

		if (psNET_direct_connect)	//single
		{
			shared_str const & server_options = Server->GetConnectOptions();
			level_name	= name().c_str();//Server->level_name		(server_options).c_str();
			level_ver	= Server->level_version		(server_options).c_str(); //1.0
		} else					//multiplayer
		{
			level_name		= get_net_DescriptionData().map_name;
			level_ver		= get_net_DescriptionData().map_version;
			download_url	= get_net_DescriptionData().download_url;
			rescan_mp_archives(); //because if we are using psNET_direct_connect, we not download map...
		}
		// Determine internal level-ID
		int						level_id = pApp->Level_ID(level_name, level_ver, true);
		if (level_id==-1)	
		{
			Disconnect			();

			connected_to_server = FALSE;
			Msg("! Level (name:%s), (version:%s), not found, try to download from:%s",
				level_name, level_ver, download_url);
			map_data.m_name					= level_name;
			map_data.m_map_version			= level_ver;
			map_data.m_map_download_url		= download_url;
			map_data.m_map_loaded			= false;
			return false;
		}
#ifdef DEBUG
		Msg("--- net_start_client3: level_id [%d], level_name[%s], level_version[%s]", level_id, level_name, level_ver);
#endif // #ifdef DEBUG
		map_data.m_name					= level_name;
		map_data.m_map_version			= level_ver;
		map_data.m_map_download_url		= download_url;
		map_data.m_map_loaded			= true;
		
		deny_m_spawn			= FALSE;
		// Load level
		R_ASSERT2				(Load(level_id),"Loading failed.");
		map_data.m_level_geom_crc32 = 0;
		if (!IsGameTypeSingle())
			CalculateLevelCrc32		();
	}
	return true;
}

bool	CLevel::net_start_client4				()
{
	if(connected_to_server){
		// Begin spawn
		g_pGamePersistent->LoadTitle		("st_client_spawning");

		// Send physics to single or multithreaded mode
		LoadPhysicsGameParams				();
		ph_world							= xr_new<CPHWorld>();
		ph_world->Create					();

		// Send network to single or multithreaded mode
		// *note: release version always has "mt_*" enabled
		Device.seqFrameMT.Remove			(g_pNetProcessor);
		Device.seqFrame.Remove				(g_pNetProcessor);
		if (psDeviceFlags.test(mtNetwork))	Device.seqFrameMT.Add	(g_pNetProcessor,REG_PRIORITY_HIGH	+ 2);
		else								Device.seqFrame.Add		(g_pNetProcessor,REG_PRIORITY_LOW	- 2);

		if(!psNET_direct_connect)
		{
			// Waiting for connection/configuration completition
			CTimer	timer_sync	;	timer_sync.Start	();
			while	(!net_isCompleted_Connect())	Sleep	(5);
			Msg		("* connection sync: %d ms", timer_sync.GetElapsed_ms());
			while	(!net_isCompleted_Sync())	{ ClientReceive(); Sleep(5); }
		}
/*
		if(psNET_direct_connect)
		{
			ClientReceive(); 
			if(Server)
					Server->Update()	;
			Sleep(5);
		}else

			while(!game_configured)			
			{ 
				ClientReceive(); 
				if(Server)
					Server->Update()	;
				Sleep(5); 
			}
*/
		}
	return true;
}

bool	CLevel::net_start_client5				()
{
	if(connected_to_server){
		// HUD

		// Textures
		if	(!g_dedicated_server)
		{
			//HUD().Load							();
			g_pGamePersistent->LoadTitle				("st_loading_textures");
			//Device.Resources->DeferredLoad		(FALSE);
			Device.m_pRender->DeferredLoad(FALSE);
			//Device.Resources->DeferredUpload	();
			Device.m_pRender->ResourcesDeferredUpload();
			LL_CheckTextures					();
		}
		sended_request_connection_data	= FALSE;
		deny_m_spawn					= TRUE;
	}
	return true;
}

bool	CLevel::net_start_client6				()
{
	if (connected_to_server) {
		// Sync
		if (!synchronize_map_data				())
			return false;

		if (!game_configured)
		{
			pApp->LoadEnd						(); 
			return true;
		}
		
		if	(!g_dedicated_server)
		{
			HUD().Load							();
			//g_pGamePersistent->LoadTitle				("st_loading_textures");
		}

		if(g_hud)
			g_hud->OnConnected				();

#ifdef DEBUG
		Msg("--- net_start_client6");
#endif // #ifdef DEBUG

		if (game)
		{
			game->OnConnected				();
			if (game->Type() != eGameIDSingle)
			{
				m_file_transfer = xr_new<file_transfer::client_site>();
			}
		}

		g_pGamePersistent->LoadTitle		("st_client_synchronising");
		Device.PreCache						(60,true,false);
		net_start_result_total				= TRUE;

	}else{
		net_start_result_total				= FALSE;
	}

	pApp->LoadEnd							(); 
	return true;
}