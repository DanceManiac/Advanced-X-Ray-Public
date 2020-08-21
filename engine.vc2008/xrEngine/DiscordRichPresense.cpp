#include "stdafx.h"
#include "DiscordRichPresense.h"
#include "DiscordRichPresense/discord_register.h"
#include "DiscordRichPresense/discord_rpc.h"
#include "x_ray.h"

ENGINE_API xrDiscordPresense g_discord;

void xrDiscordPresense::Initialize()
{
	LPCSTR discord_app_id;
	discord_app_id = READ_IF_EXISTS(pSettings, r_string, "global", "discord_app_id", "745606008499601438");
	// We don't have multiplayer mode, so no need to invite system to support
	DiscordEventHandlers nullHandlers;
	ZeroMemory(&nullHandlers, sizeof(nullHandlers));
	Discord_Initialize(discord_app_id, &nullHandlers, TRUE, nullptr);
	bInitialize = true;
}

void xrDiscordPresense::Shutdown()
{
	if (bInitialize)
	{
		Discord_ClearPresence();
		Discord_Shutdown();
		bInitialize = false;
	}
}

void xrDiscordPresense::SetStatus(StatusId status)
{
	if (!bInitialize) return;

	DiscordRichPresence presenseInfo;
	ZeroMemory(&presenseInfo, sizeof(presenseInfo));

	StatusId realStatus = status;
	if (status == StatusId::In_Game)
	{
		// get level name, and set status to different value, when we found vanilla levels
		if (pApp->Level_Current < pApp->Levels.size())
		{
			CApplication::sLevelInfo& LevelInfo = pApp->Levels[pApp->Level_Current];
			if (LevelInfo.name == nullptr) return;
			if (xr_strcmp(LevelInfo.name, "zaton") == 0)
			{
				realStatus = StatusId::Zaton;
			}
			if (xr_strcmp(LevelInfo.name, "jupiter") == 0)
			{
				realStatus = StatusId::Upiter;
			}
			if (xr_strcmp(LevelInfo.name, "pripyat") == 0)
			{
				realStatus = StatusId::Pripyat;
			}
			if (xr_strcmp(LevelInfo.name, "labx8") == 0)
			{
				realStatus = StatusId::LabX8;
			}
			if (xr_strcmp(LevelInfo.name, "jupiter_underground") == 0)
			{
				realStatus = StatusId::JupiterUnder;
			}
			if (xr_strcmp(LevelInfo.name, "marsh") == 0)
			{
				realStatus = StatusId::Marsh;
			}
			if (xr_strcmp(LevelInfo.name, "escape") == 0)
			{
				realStatus = StatusId::Escape;
			}
			if (xr_strcmp(LevelInfo.name, "garbage") == 0)
			{
				realStatus = StatusId::Garbage;
			}
			if (xr_strcmp(LevelInfo.name, "darkvalley") == 0)
			{
				realStatus = StatusId::Darkvalley;
			}
			if (xr_strcmp(LevelInfo.name, "agroprom") == 0)
			{
				realStatus = StatusId::Agroprom;
			}
			if (xr_strcmp(LevelInfo.name, "agroprom_underground") == 0)
			{
				realStatus = StatusId::AgrUnder;
			}
			if (xr_strcmp(LevelInfo.name, "yantar") == 0)
			{
				realStatus = StatusId::Yantar;
			}
			if (xr_strcmp(LevelInfo.name, "red_forest") == 0)
			{
				realStatus = StatusId::RedForest;
			}
			if (xr_strcmp(LevelInfo.name, "military") == 0)
			{
				realStatus = StatusId::Military;
			}
			if (xr_strcmp(LevelInfo.name, "limansk") == 0)
			{
				realStatus = StatusId::Limansk;
			}
			if (xr_strcmp(LevelInfo.name, "hospital") == 0)
			{
				realStatus = StatusId::Hospital;
			}
			if (xr_strcmp(LevelInfo.name, "stancia_2") == 0)
			{
				realStatus = StatusId::Stancia2;
			}
		}
	}

	presenseInfo.startTimestamp = time(0);
	presenseInfo.largeImageText = "Zone Awaits...";

	switch (realStatus)
	{
	case StatusId::In_Game:
		presenseInfo.details		= "In Game";
		presenseInfo.largeImageKey	= "zaton";
		break;
	case StatusId::Zaton:
		presenseInfo.details		= "In Game: Zaton";
		presenseInfo.largeImageKey  = "zaton";
		break;
	case StatusId::Upiter:
		presenseInfo.details		= "In Game: Jupiter";
		presenseInfo.largeImageKey  = "jupiter";
		break;
	case StatusId::Pripyat:
		presenseInfo.details		= "In Game: Pripyat";
		presenseInfo.largeImageKey  = "pripyat";
		break;
	case StatusId::LabX8:
		presenseInfo.details		= "In Game: X-8";
		presenseInfo.largeImageKey	= "labx8";
		break;
	case StatusId::JupiterUnder:
		presenseInfo.details		= "In Game: Jupiter Underground";
		presenseInfo.largeImageKey	= "jupiter_underground";
		break;
	case StatusId::Marsh:
		presenseInfo.details		= "In Game: Marsh";
		presenseInfo.largeImageKey	= "marsh";
		break;
	case StatusId::Escape:
		presenseInfo.details		= "In Game: Escape";
		presenseInfo.largeImageKey	= "escape";
		break;
	case StatusId::Garbage:
		presenseInfo.details		= "In Game: Garbage";
		presenseInfo.largeImageKey	= "garbage";
		break;
	case StatusId::Darkvalley:
		presenseInfo.details		= "In Game: Darkvalley";
		presenseInfo.largeImageKey	= "darkvalley";
		break;
	case StatusId::Agroprom:
		presenseInfo.details		= "In Game: Agroprom";
		presenseInfo.largeImageKey	= "agroprom";
		break;
	case StatusId::AgrUnder:
		presenseInfo.details		= "In Game: Agroprom Underground";
		presenseInfo.largeImageKey	= "agroprom_underground";
		break;
	case StatusId::Yantar:
		presenseInfo.details = "In Game: Yantar";
		presenseInfo.largeImageKey = "yantar";
		break;
	case StatusId::RedForest:
		presenseInfo.details = "In Game: Red Forest";
		presenseInfo.largeImageKey = "red_forest";
		break;
	case StatusId::Military:
		presenseInfo.details = "In Game: Military";
		presenseInfo.largeImageKey = "military";
		break;
	case StatusId::Limansk:
		presenseInfo.details = "In Game: Limansk";
		presenseInfo.largeImageKey = "limansk";
		break;
	case StatusId::Hospital:
		presenseInfo.details = "In Game: Hospital";
		presenseInfo.largeImageKey = "hospital";
		break;
	case StatusId::Stancia2:
		presenseInfo.details = "In Game: Stancia 2";
		presenseInfo.largeImageKey = "stancia_2";
		break;
	default:
	case StatusId::Menu:
		presenseInfo.details		= "In Game: Main Menu";
		presenseInfo.largeImageKey	= "main_picture";
		break;
	}

	Discord_UpdatePresence(&presenseInfo);
}

xrDiscordPresense::~xrDiscordPresense()
{
	Shutdown();
}