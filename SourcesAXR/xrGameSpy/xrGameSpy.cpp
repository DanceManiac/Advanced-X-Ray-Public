// xrGameSpy.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "xrGameSpy.h"
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

void	FillSecretKey(char* SecretKey)
{
	SecretKey[0] = 'L';
	SecretKey[1] = 'T';
	SecretKey[2] = 'U';
	SecretKey[3] = '2';
	SecretKey[4] = 'z';
	SecretKey[5] = '2';
	SecretKey[6] = '\0';
}

const char* GetGameVersion	()
{
	return GAME_VERSION;
}

const char* GetAxrPlatform()
{	
	return GAME_PLATFORM;
}

XRGAMESPY_API const char* xrGS_GetGameVersion	()
{
	return GetGameVersion();
}

XRGAMESPY_API const char* xrGS_GetAxrPlatform()
{
	return GetAxrPlatform();
}

XRGAMESPY_API void xrGS_GetGameID	(int* GameID, int verID)
{
	*GameID = int(GAMESPY_GAMEID);

#ifdef DEMO_BUILD
	switch (verID)
	{
	case 1: *GameID = int(1067); break;
	case 2: *GameID = int(1576); break;
	case 3: *GameID = int(1620); break;
	default: *GameID = int(GAMESPY_GAMEID); break;
	}	
#endif
}