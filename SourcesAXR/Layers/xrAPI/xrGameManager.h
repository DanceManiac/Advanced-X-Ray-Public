#pragma once
#include "..\..\Include\xrAPI\xrAPI.h"
enum class EGame
{
	COP,
	CS,
	SHOC,
};
enum class EGamePath
{
	NONE=-1,
	COP_1602,
	CS_1510,
	SHOC_10006,
	SHOC_10004,
};

class XRAPI_API xrGameManager
{
public:
	static EGame GetGame();
	static EGamePath GetPath();
};