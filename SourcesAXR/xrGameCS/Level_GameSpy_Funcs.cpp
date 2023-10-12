#include "stdafx.h"
#include "Level.h"
#include "xrMessages.h"
#include "../xrEngine/x_ray.h"
#include "GameSpy/GameSpy_GCD_Client.h"


#include "../xrEngine/igame_persistent.h"
void						CLevel::OnGameSpyChallenge			(NET_Packet* P)
{
#ifndef MASTER_GOLD
	Msg("xrGS::CDKey::Level : Responding on Challenge");
#endif // #ifndef MASTER_GOLD

	u8	Reauth = P->r_u8();
	string64 ChallengeStr;
	P->r_stringZ(ChallengeStr);
	
	//--------------------------------------------------------------------
	string128 ResponseStr="";
	CGameSpy_GCD_Client GCD;
	GCD.CreateRespond(ResponseStr, ChallengeStr, Reauth);
	//--------- Send Respond ---------------------------------------------
	NET_Packet newP;

	newP.w_begin	(M_GAMESPY_CDKEY_VALIDATION_CHALLENGE_RESPOND);
	newP.w_stringZ(ResponseStr);
	Send(newP, net_flags(TRUE, TRUE, TRUE, TRUE));

//	g_pGamePersistent->LoadTitle("st_validating_cdkey");
	g_pGamePersistent->LoadTitle();
};

