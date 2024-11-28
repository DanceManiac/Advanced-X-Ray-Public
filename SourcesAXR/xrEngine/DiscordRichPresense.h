#pragma once

class ENGINE_API xrDiscordPresense
{
public:
	struct RPC_Settings
	{
		void Default()
		{
			strcpy_s(Detail, "");
			strcpy_s(State, "");
			strcpy_s(LargeImageKey, "main_picture");
			strcpy_s(LargeImageText, "");
		}
		char Detail[128];
		char State[128];
		char LargeImageKey[128];
		char SmallImageKey[128];
		char LargeImageText[128];
		char SmallImageText[128];
	};
	void Initialize();
	void Shutdown();

	void SetStatus();
	~xrDiscordPresense();

	float LowlandFogBaseHeight;
	Fvector4 SSS_TerrainOffset;
	LPCSTR discord_app_id;
private:
	bool bInitialize = false;
	bool bGameRPCInfoInit = false;
	bool show_task = true;
};

extern ENGINE_API xrDiscordPresense g_discord;
extern ENGINE_API xrDiscordPresense::RPC_Settings rpc_settings;