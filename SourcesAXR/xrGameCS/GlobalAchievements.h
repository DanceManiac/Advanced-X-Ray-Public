#pragma once

class CGlobalAchievement
{
    CGlobalAchievement(const CGlobalAchievement&) = delete;
public:
    CGlobalAchievement          ();
    CGlobalAchievement(std::string name, std::string description, std::string iconName)
        : m_name(name), m_description(description), m_iconName(iconName) {}

    std::string getName         () const { return m_name; }
    std::string getDescription  () const { return m_description; }
    std::string getIconName     () const { return m_iconName; }

private:
    std::string m_name;
    std::string m_description;
    std::string m_iconName;
};

class CGlobalAchievementsManager
{
public:
    CGlobalAchievementsManager          ();

    ~CGlobalAchievementsManager()
    {
        for (auto& achievement : m_AchievementsData)
            xr_delete(achievement);

        m_AchievementsData.clear();
    }

    void Create                         (std::string name, std::string description, std::string iconName);
    void Delete                         (std::string name);
    void SaveAchievements               ();
    CGlobalAchievement* GetAchievement  (std::string name);
    bool HasAchievements                () { return m_AchievementsData.size(); }

    xr_vector<CGlobalAchievement*> GetAllAchievements   () { return m_AchievementsData; }
    xr_vector<CGlobalAchievement*> LoadAchievements     ();

private:
    xr_vector<CGlobalAchievement*> m_AchievementsData{};
};

