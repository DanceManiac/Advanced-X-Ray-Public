#include "stdafx.h"
#include "xrserver.h"
#include "xrmessages.h"

void xrServer::Perform_game_export	()
{
	if (GetClientsCount() == 0)
		return;

	// Broadcase game state to every body
	// But it is slightly different view for each "player"

	NET_Packet		P;
	u32				mode			= net_flags(TRUE,TRUE);

	// Game config (all, info includes _new_ player)
	ForEachClientDoSender([&](IClient* cl) {
		ClientID ID = cl->ID;
		xrClientData* CL = (xrClientData*)cl;
		if (!CL->net_Accepted) return;
		P.w_begin(M_SV_CONFIG_GAME);
		game->net_Export_State(P, ID);
		SendTo(ID, P, mode);
		});

	game->sv_force_sync	= FALSE;
}

void xrServer::Export_game_type(IClient* CL)
{
	NET_Packet			P;
	u32					mode = net_flags(TRUE,TRUE);

	P.w_begin			(M_SV_CONFIG_NEW_CLIENT);
	P.w_stringZ			(game->type_name() );
	SendTo				(CL->ID,P,mode);
}

