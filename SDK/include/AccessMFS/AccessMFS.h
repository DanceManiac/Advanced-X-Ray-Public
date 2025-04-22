#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <filesystem>

struct UserAccessData
{
    std::string username;
    std::string userId;
    std::vector<std::string> userKeys;
    std::vector<std::string> requiredRoles;
};

uint64_t CalculateChecksum(const BYTE* data, size_t size);

bool WriteAccessData(const UserAccessData& data, std::filesystem::path base_path);
bool GetAccessData(UserAccessData& outData, std::filesystem::path base_path);
bool AccessDataExists(std::filesystem::path base_path);

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output);
std::vector<std::string> ExtractRoles(const std::string& json_response);
bool CheckDiscordRoles(const std::string& userId, const std::vector<std::string>& requiredRoleIds);

bool StartDiscordOAuth();
void RunLocalServer();
void HandleClient(SOCKET clientSocket);

std::vector<std::string> GetAccessKeys();