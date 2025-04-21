#pragma once

#include "discord_register.h"
#include "discord_rpc.h"
#include "windows.h"

class xrDiscordManager
{
public:
    void OnFailConnect();
    void OnConnect(std::string userid);
};

extern xrDiscordManager xr_discord_manager;