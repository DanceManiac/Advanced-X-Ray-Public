////////////////////////////////////////////////////////////////////////////
//	Module 		: GlobalAchievements.cpp
//	Created 	: 22.02.2025
//  Modified 	: 22.02.2025
//	Author		: Dance Maniac (M.F.S. Team)
//	Description : Advanced X-Ray global achievements class
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <fstream>
#include "GlobalAchievements.h"

CGlobalAchievement::CGlobalAchievement()
{
    m_name          = nullptr;
    m_description   = nullptr;
    m_iconName      = nullptr;
}

CGlobalAchievementsManager::CGlobalAchievementsManager()
{
	m_AchievementsData.clear();
}

void CGlobalAchievementsManager::Create(std::string name, std::string description, std::string iconName)
{
    CGlobalAchievement* achievement = xr_new<CGlobalAchievement>(name, description, iconName);
	m_AchievementsData.push_back(achievement);

#ifdef DEBUG
    Msg("[CGlobalAchievementsManager::Create]: Achievement created! Name: %s, Description: %s, Icon: %s", name.c_str(), description.c_str(), iconName.c_str());
#endif

	SaveAchievements();
}

void CGlobalAchievementsManager::Delete(std::string name)
{
    for (auto it = m_AchievementsData.begin(); it != m_AchievementsData.end(); ++it)
    {
        if ((*it)->getName() == name)
        {
            m_AchievementsData.erase(it, m_AchievementsData.end());

#ifdef DEBUG
            Msg("[CGlobalAchievementsManager::Delete]: Achievement with name [%s] : Deleted", (*it)->getName().c_str());
#endif
            xr_delete(*it);
        }
    }

	SaveAchievements();
}

CGlobalAchievement* CGlobalAchievementsManager::GetAchievement(std::string name)
{
	for (const auto& achievement : m_AchievementsData)
	{
		if (achievement && ((*achievement).getName() == name))
		{
			return achievement;
		}
	}

#ifdef DEBUG
    Msg("![CGlobalAchievementsManager::GetAchievement]: Achievement with name [%s] not founded!", name.c_str());
#endif

	return nullptr;
}

void CGlobalAchievementsManager::SaveAchievements()
{
    std::string userDir = FS.get_path("$app_data_root$")->m_Path, fileToWrite = "//axr_achievements.ach";
    std::string path = userDir + fileToWrite;
    std::ofstream outFile(path, std::ios::binary);
    if (!outFile)
    {
#ifdef DEBUG
        Msg("![CGlobalAchievementsManager::SaveAchievements]: Error with open file for write! File: [%s]", path.c_str());
#endif
        return;
    }

    for (const auto& achievement : m_AchievementsData)
    {
        // Имя
        std::string name = achievement->getName().c_str();
        size_t nameLength = name.size();
        outFile.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
        outFile.write(name.c_str(), nameLength);

        // Описание
        std::string description = achievement->getDescription().c_str();
        size_t descriptionLength = description.size();
        outFile.write(reinterpret_cast<const char*>(&descriptionLength), sizeof(descriptionLength));
        outFile.write(description.c_str(), descriptionLength);

        // Название иконки
        std::string iconName = achievement->getIconName().c_str();
        size_t iconNameLength = iconName.size();
        outFile.write(reinterpret_cast<const char*>(&iconNameLength), sizeof(iconNameLength));
        outFile.write(iconName.c_str(), iconNameLength);

#ifdef DEBUG
        Msg("[CGlobalAchievementsManager::SaveAchievements]: Achievement saved! Name: %s, Description: %s, Icon: %s", name.c_str(), description.c_str(), iconName.c_str());
#endif
    }

    outFile.close();
}

xr_vector<CGlobalAchievement*> CGlobalAchievementsManager::LoadAchievements()
{
    std::string userDir = FS.get_path("$app_data_root$")->m_Path, fileToWrite = "//axr_achievements.ach";
    std::string path = userDir + fileToWrite;
    std::ifstream inFile(path, std::ios::binary);
    if (!inFile.is_open())
    {
#ifdef DEBUG
        Msg("[CGlobalAchievementsManager::LoadAchievements]: File not found or cannot be opened! File: [%s]", path.c_str());
#endif
        return {};
    }

    while (inFile)
    {
        size_t nameLength;
        inFile.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));

        // Имя
        std::string name(nameLength, '\0');
        inFile.read(&name[0], nameLength);

        size_t descriptionLength;
        inFile.read(reinterpret_cast<char*>(&descriptionLength), sizeof(descriptionLength));

        // Описание
        std::string description(descriptionLength, '\0');
        inFile.read(&description[0], descriptionLength);

        size_t iconNameLength;
        inFile.read(reinterpret_cast<char*>(&iconNameLength), sizeof(iconNameLength));

        // Название иконки
        std::string iconName(iconNameLength, '\0');
        inFile.read(&iconName[0], iconNameLength);

        CGlobalAchievement* achievement = xr_new<CGlobalAchievement>(name, description, iconName);
        m_AchievementsData.push_back(achievement);

#ifdef DEBUG
        Msg("[CGlobalAchievementsManager::LoadAchievements]: Achievement loaded! Name: %s, Description: %s, Icon: %s", name.c_str(), description.c_str(), iconName.c_str());
#endif
    }
    inFile.close();

    return m_AchievementsData;
}