#include "stdafx.h"
#include "DiscordRichPresense.h"
#include "DiscordRichPresense/discord_register.h"
#include "DiscordRichPresense/discord_rpc.h"
#include "x_ray.h"

ENGINE_API xrDiscordPresense g_discord;
extern int g_current_renderer;

void xrDiscordPresense::Initialize()
{
	discord_app_id = READ_IF_EXISTS(pAdvancedSettings, r_string, "global", "discord_app_id", "745606008499601438");
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
	LowlandFogBaseHeight = 0.0;

	if (status == StatusId::In_Game)
	{
		// get level name, and set status to different value, when we found vanilla levels
		if (pApp->Level_Current < pApp->Levels.size())
		{
			CApplication::sLevelInfo& LevelInfo = pApp->Levels[pApp->Level_Current];
			if (LevelInfo.name == nullptr) return;
			if (xr_strcmp(LevelInfo.name, "zaton") == 0)
			{
				LowlandFogBaseHeight = READ_IF_EXISTS(pAdvancedSettings, r_float, "lowland_fog_params", "zaton", 0.0f);
				realStatus = StatusId::Zaton;
			}
			if (xr_strcmp(LevelInfo.name, "jupiter") == 0)
			{
				LowlandFogBaseHeight = READ_IF_EXISTS(pAdvancedSettings, r_float, "lowland_fog_params", "jupiter", 0.0f);
				realStatus = StatusId::Upiter;
			}
			if (xr_strcmp(LevelInfo.name, "pripyat") == 0)
			{
				LowlandFogBaseHeight = READ_IF_EXISTS(pAdvancedSettings, r_float, "lowland_fog_params", "pripyat", 0.0f);
				realStatus = StatusId::Pripyat;
			}
			if (xr_strcmp(LevelInfo.name, "labx8") == 0)
			{
				LowlandFogBaseHeight = READ_IF_EXISTS(pAdvancedSettings, r_float, "lowland_fog_params", "labx8", 0.0f);
				realStatus = StatusId::LabX8;
			}
			if (xr_strcmp(LevelInfo.name, "jupiter_underground") == 0)
			{
				LowlandFogBaseHeight = READ_IF_EXISTS(pAdvancedSettings, r_float, "lowland_fog_params", "jupiter_underground", 0.0f);
				realStatus = StatusId::JupiterUnder;
			}
			if (xr_strcmp(LevelInfo.name, "marsh") == 0)
			{
				LowlandFogBaseHeight = READ_IF_EXISTS(pAdvancedSettings, r_float, "lowland_fog_params", "marsh", 0.0f);
				realStatus = StatusId::Marsh;
			}
			if (xr_strcmp(LevelInfo.name, "escape") == 0)
			{
				LowlandFogBaseHeight = READ_IF_EXISTS(pAdvancedSettings, r_float, "lowland_fog_params", "escape", 0.0f);
				realStatus = StatusId::Escape;
			}
			if (xr_strcmp(LevelInfo.name, "garbage") == 0)
			{
				LowlandFogBaseHeight = READ_IF_EXISTS(pAdvancedSettings, r_float, "lowland_fog_params", "garbage", 0.0f);
				realStatus = StatusId::Garbage;
			}
			if (xr_strcmp(LevelInfo.name, "darkvalley") == 0)
			{
				LowlandFogBaseHeight = READ_IF_EXISTS(pAdvancedSettings, r_float, "lowland_fog_params", "darkvalley", 0.0f);
				realStatus = StatusId::Darkvalley;
			}
			if (xr_strcmp(LevelInfo.name, "agroprom") == 0)
			{
				LowlandFogBaseHeight = READ_IF_EXISTS(pAdvancedSettings, r_float, "lowland_fog_params", "agroprom", 0.0f);
				realStatus = StatusId::Agroprom;
			}
			if (xr_strcmp(LevelInfo.name, "agroprom_underground") == 0)
			{
				LowlandFogBaseHeight = READ_IF_EXISTS(pAdvancedSettings, r_float, "lowland_fog_params", "agroprom_underground", 0.0f);
				realStatus = StatusId::AgrUnder;
			}
			if (xr_strcmp(LevelInfo.name, "yantar") == 0)
			{
				LowlandFogBaseHeight = READ_IF_EXISTS(pAdvancedSettings, r_float, "lowland_fog_params", "yantar", 0.0f);
				realStatus = StatusId::Yantar;
			}
			if (xr_strcmp(LevelInfo.name, "red_forest") == 0)
			{
				LowlandFogBaseHeight = READ_IF_EXISTS(pAdvancedSettings, r_float, "lowland_fog_params", "red_forest", 0.0f);
				realStatus = StatusId::RedForest;
			}
			if (xr_strcmp(LevelInfo.name, "military") == 0)
			{
				LowlandFogBaseHeight = READ_IF_EXISTS(pAdvancedSettings, r_float, "lowland_fog_params", "military", 0.0f);
				realStatus = StatusId::Military;
			}
			if (xr_strcmp(LevelInfo.name, "limansk") == 0)
			{
				LowlandFogBaseHeight = READ_IF_EXISTS(pAdvancedSettings, r_float, "lowland_fog_params", "limansk", 0.0f);
				realStatus = StatusId::Limansk;
			}
			if (xr_strcmp(LevelInfo.name, "hospital") == 0)
			{
				LowlandFogBaseHeight = READ_IF_EXISTS(pAdvancedSettings, r_float, "lowland_fog_params", "hospital", 0.0f);
				realStatus = StatusId::Hospital;
			}
			if (xr_strcmp(LevelInfo.name, "stancia_2") == 0)
			{
				LowlandFogBaseHeight = READ_IF_EXISTS(pAdvancedSettings, r_float, "lowland_fog_params", "stancia_2", 0.0f);
				realStatus = StatusId::Stancia2;
			}
		}
	}

	presenseInfo.startTimestamp = time(0);

	if (g_current_renderer == 1)
	{
		presenseInfo.state = "В Игре: Рендер R1 (DX9)";
	}
	else if (g_current_renderer == 2)
	{
		presenseInfo.state = "В Игре: Рендер R2 (DX9)";
	}
	else if (g_current_renderer == 3)
	{
		presenseInfo.state = "В Игре: Рендер R3 (DX10)";
	}
	else
	{
		presenseInfo.state = "В Игре: Рендер R4 (DX11)";
	}

	if (CallOfPripyatMode)
	{
		presenseInfo.largeImageText = "Режим Движка: Зов Припяти";
	}
	else if (ClearSkyMode)
	{
		presenseInfo.largeImageText = "Режим Движка: Чистое Небо";
	}
	else
	{
		presenseInfo.largeImageText = "Зона ожидает...";
	}

	if (discord_app_id != "745606008499601438")
	{
		presenseInfo.smallImageKey = "advanced_xray";
		presenseInfo.smallImageText = "Advanced X-Ray Engine";
	}

	switch (realStatus)
	{
	case StatusId::In_Game:
		presenseInfo.details		= "Главное Меню";
		presenseInfo.largeImageKey	= "main_picture";
		break;
	case StatusId::Zaton:
		presenseInfo.details		= "Локация: Затон";
		presenseInfo.largeImageKey  = "zaton";
		break;
	case StatusId::Upiter:
		presenseInfo.details		= "Локация: Окрестности Юпитера";
		presenseInfo.largeImageKey  = "jupiter";
		break;
	case StatusId::Pripyat:
		presenseInfo.details		= "Локация: Припять";
		presenseInfo.largeImageKey  = "pripyat";
		break;
	case StatusId::LabX8:
		presenseInfo.details		= "Локация: Лаборатория Х-8";
		presenseInfo.largeImageKey	= "labx8";
		break;
	case StatusId::JupiterUnder:
		presenseInfo.details		= "Локация: Путепровод Припять-1";
		presenseInfo.largeImageKey	= "jupiter_underground";
		break;
	case StatusId::Marsh:
		presenseInfo.details		= "Локация: Болота";
		presenseInfo.largeImageKey	= "marsh";
		break;
	case StatusId::Escape:
		presenseInfo.details		= "Локация: Кордон";
		presenseInfo.largeImageKey	= "escape";
		break;
	case StatusId::Garbage:
		presenseInfo.details		= "Локация: Свалка";
		presenseInfo.largeImageKey	= "garbage";
		break;
	case StatusId::Darkvalley:
		presenseInfo.details		= "Локация: Тёмная Долина";
		presenseInfo.largeImageKey	= "darkvalley";
		break;
	case StatusId::Agroprom:
		presenseInfo.details		= "Локация: НИИ Агропром";
		presenseInfo.largeImageKey	= "agroprom";
		break;
	case StatusId::AgrUnder:
		presenseInfo.details		= "Локация: Подземелья Агропрома";
		presenseInfo.largeImageKey	= "agroprom_underground";
		break;
	case StatusId::Yantar:
		presenseInfo.details		= "Локация: Янтарь";
		presenseInfo.largeImageKey	= "yantar";
		break;
	case StatusId::RedForest:
		presenseInfo.details		= "Локация: Рыжий Лес";
		presenseInfo.largeImageKey	= "red_forest";
		break;
	case StatusId::Military:
		presenseInfo.details		= "Локация: Армейские склады";
		presenseInfo.largeImageKey	= "military";
		break;
	case StatusId::Limansk:
		presenseInfo.details		= "Локация: Лиманск";
		presenseInfo.largeImageKey	= "limansk";
		break;
	case StatusId::Hospital:
		presenseInfo.details		= "Локация: Госпиталь";
		presenseInfo.largeImageKey	= "hospital";
		break;
	case StatusId::Stancia2:
		presenseInfo.details		= "Локация: ЧАЭС";
		presenseInfo.largeImageKey	= "stancia_2";
		break;
	default:
	case StatusId::Menu:
		presenseInfo.details		= "Главное Меню";
		presenseInfo.largeImageKey	= "main_picture";
		break;
	}

	Discord_UpdatePresence(&presenseInfo);
}

xrDiscordPresense::~xrDiscordPresense()
{
	Shutdown();
}