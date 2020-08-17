#include "stdafx.h"
#include "xrGameManager.h"
#include "..\..\Include\xrAPI\xrAPI.h"
XRAPI_API EGamePath GCurrentGame = EGamePath::NONE;
EGame xrGameManager::GetGame()
{
	switch (GCurrentGame)
	{
	case EGamePath::COP_1602:
		return EGame::COP;
	case EGamePath::CS_1510:
		return EGame::CS;
	case EGamePath::SHOC_10006:
	case EGamePath::SHOC_10004:
		return EGame::SHOC;
	default:
		return EGame::COP;
	}
}

EGamePath xrGameManager::GetPath()
{
    return GCurrentGame;
}