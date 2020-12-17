#pragma once

class ENGINE_API xrDiscordPresense
{
public:
	enum class StatusId
	{
		Menu,
		In_Game,
		Zaton,
		Upiter,
		Pripyat,
		LabX8,
		JupiterUnder,
		Marsh,
		Escape,
		Garbage,
		Darkvalley,
		Agroprom,
		AgrUnder,
		Yantar,
		RedForest,
		Military,
		Limansk,
		Hospital,
		Stancia2
	};

public:

	void Initialize();
	void Shutdown();

	void SetStatus(StatusId status);
	float LowlandFogBaseHeight;
	LPCSTR discord_app_id;

	~xrDiscordPresense();

private:
	bool bInitialize = false;
};

extern ENGINE_API xrDiscordPresense g_discord;