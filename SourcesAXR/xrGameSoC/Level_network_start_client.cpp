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

#include "../xrEngine/DiscordRichPresense.h"

extern	pureFrame*				g_pNetProcessor;

BOOL CLevel::net_Start_client	( LPCSTR options )
{
	return FALSE;
}
#include "string_table.h"
bool	CLevel::net_start_client1				()
{
	ZoneScoped;

	pApp->LoadBegin	();
	// name_of_server
	string64					name_of_server = "";
//	xr_strcpy					(name_of_server,*m_caClientOptions);
	if (strchr(*m_caClientOptions, '/'))
		strncpy_s(name_of_server, *m_caClientOptions, strchr(*m_caClientOptions, '/') - *m_caClientOptions);

	if (strchr(name_of_server,'/'))	*strchr(name_of_server,'/') = 0;

	// Startup client
	string256		temp;
	xr_sprintf(temp, "%s %s",
		CStringTable().translate("st_client_connecting_to").c_str(),
		name_of_server);

	pApp->SetLoadStageTitle(temp);
	pApp->LoadStage();
	return true;
}

#include "xrServer.h"

bool	CLevel::net_start_client2				()
{
	ZoneScoped;

	if(psNET_direct_connect)
	{
		Server->create_direct_client();
	}

	connected_to_server = Connect2Server(*m_caClientOptions);
	
	return true;
}

bool	CLevel::net_start_client3				()
{
	ZoneScoped;

	if(connected_to_server){
		LPCSTR					level_name = NULL;
		LPCSTR					level_ver = NULL;

		if(psNET_direct_connect)
		{
			level_name	= ai().get_alife() ? *name() : Server->level_name( Server->GetConnectOptions() ).c_str();
		}else
			level_name	= ai().get_alife() ? *name() : net_SessionName	();

		shared_str const& server_options = Server->GetConnectOptions();
		level_name = name().c_str();//Server->level_name		(server_options).c_str();
		level_ver = "1.0";

		// Determine internal level-ID
		int						level_id = pApp->Level_ID(level_name, level_ver, true);
		if (level_id<0)	{
			Disconnect			();
			pApp->LoadEnd		();
			connected_to_server = FALSE;

			Msg("! Level (name:%s), (version:%s), not found",
				level_name, level_ver);

			return false;
		}
		pApp->Level_Set			(level_id);
		m_name					= level_name;

		snprintf				(rpc_settings.Detail, 128, ToUTF8(*CStringTable().translate(level_name)).c_str());
		snprintf				(rpc_settings.LargeImageKey, 128, level_name);
		g_discord.SetStatus		();

		// Load level
		R_ASSERT2				(Load(level_id),"Loading failed.");

	}
	return true;
}

bool	CLevel::net_start_client4				()
{
	ZoneScoped;

	if(connected_to_server){
		// Begin spawn
		g_pGamePersistent->SetLoadStageTitle("st_client_spawning");
		g_pGamePersistent->LoadTitle();

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

		while(!game_configured)			
		{ 
			ClientReceive(); 
			if(Server)
				Server->Update()	;
			Sleep(5); 
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
	ZoneScoped;

	if(connected_to_server){
		// HUD

		// Textures
		{
			HUD().Load							();
			g_pGamePersistent->SetLoadStageTitle("st_loading_textures");
			g_pGamePersistent->LoadTitle		();
			Device.m_pRender->DeferredLoad		(FALSE);
			Device.m_pRender->ResourcesDeferredUpload();
			LL_CheckTextures					();
		}
	}
	return true;
}

bool	CLevel::net_start_client6				()
{
	ZoneScoped;

	if(connected_to_server)
	{
		// Sync
		if(g_hud)
			g_hud->OnConnected				();


		g_pGamePersistent->SetLoadStageTitle("st_client_synchronising");
		g_pGamePersistent->LoadTitle		();
		Device.PreCache						(60, true, true);
		net_start_result_total				= TRUE;
	}
	else
	{
		net_start_result_total				= FALSE;
	}

	pApp->LoadEnd							(); 
	return true;
}