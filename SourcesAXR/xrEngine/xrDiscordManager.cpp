#include "stdafx.h"
#include "Shellapi.h"
#include "xrDiscordManager.h"

xrDiscordManager xr_discord_manager;

#ifdef PROTECT_CBT
XRCORE_API std::string authorizedID;
#endif

void xrDiscordManager::OnFailConnect()
{
#ifdef PROTECT_CBT
    CHECK_OR_EXIT(0, "Unable to initialize Discord Rich Presence. Make sure Discord is opened or try again.");
    Discord_ClearPresence();
    Discord_Shutdown();
#endif
}

void xrDiscordManager::OnConnect(std::string userid)
{
#ifdef PROTECT_CBT
    if (authorizedID != userid)
    {
        ShellExecute(0, "open", "https://discord.gg/AFPqkfBfQs", NULL, NULL, SW_HIDE);
        CHECK_OR_EXIT(0, "This Advanced X-Ray Engine build is only available for testers. Check the discord channel.");
    }
#endif
}