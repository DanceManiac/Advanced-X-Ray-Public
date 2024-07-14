////////////////////////////////////////////////////////////////////////////
//	Module 		: alife_object.cpp
//	Created 	: 27.10.2005
//  Modified 	: 27.10.2005
//	Author		: Dmitriy Iassenev
//	Description : ALife object class
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "xrServer_Objects_ALife.h"
#include "alife_simulator.h"
#include "xrServer_Objects_ALife_Items.h"
#include "ai_space.h"
#include "game_graph.h"

void CSE_ALifeObject::spawn_supplies		()
{
	spawn_supplies(*m_ini_string);
}

void CSE_ALifeObject::spawn_supplies		(LPCSTR ini_string)
{
	if (!ini_string)
		return;

	if (!xr_strlen(ini_string))
		return;

#pragma warning(push)
#pragma warning(disable:4238)
	CInifile ini(&IReader((void*)(ini_string), xr_strlen(ini_string)), FS.get_path("$game_config$")->m_Path);
#pragma warning(pop)

    // Alundaio: This will spawn a single random section listed in [spawn_loadout]
    // No need to spawn ammo, this will automatically spawn 1 box for weapon and if ammo_type is specficied it will spawn that type
    // count is used only for ammo boxes (ie wpn_pm = 3) will spawn 3 boxes, not 3 wpn_pm
    // Usage: to create random weapon loadouts
    LPCSTR loadout_section = "spawn_loadout";
    u8 iItr = 1;

    while (ini.section_exist(loadout_section))
    {
        LPCSTR itmSection, V;
        xr_vector<u32> OnlyOne;
        OnlyOne.clear();
        LPCSTR lname = *ai().game_graph().header().level(ai().game_graph().vertex(m_tGraphID)->level_id()).name();

        for (u32 k = 0; ini.r_line(loadout_section, k, &itmSection, &V); k++)
        {
            // If level=<lname> then only spawn items if object on that level
            if (strstr(V, "level=") != NULL)
            {
                if (strstr(V, lname) != NULL)
                    OnlyOne.push_back(k);
            }
            else
            {
                OnlyOne.push_back(k);
            }
        }

        if (!OnlyOne.empty())
        {
            s32 sel = ::Random.randI(0, OnlyOne.size() - 1);

            if (ini.r_line(loadout_section, OnlyOne.at(sel), &itmSection, &V))
            {
                VERIFY(xr_strlen(itmSection));

                if (pSettings->section_exist(itmSection))
                {
                    u32 spawn_count = 1;
                    bool bScope = false;
                    bool bSilencer = false;
                    bool bLauncher = false;
                    //bool bLaser = false;
                    //bool bTacticalTorch = false;
                    float f_cond = 1.0f;
                    int i_ammo_type = 0, n = 0;
                    int cur_scope = 0;

                    if (V && xr_strlen(V))
                    {
                        n = _GetItemCount(V);

                        if (n > 0)
                        {
                            string64 tmp;
                            spawn_count = atoi(_GetItem(V, 0, tmp)); //count
                        }

                        if (!spawn_count)
                            spawn_count = 1;

                        if (NULL != strstr(V, "cond="))
                            f_cond = (float)atof(strstr(V, "cond=") + 5);

                        bScope = (NULL != strstr(V, "scope"));
                        bSilencer = (NULL != strstr(V, "silencer"));
                        bLauncher = (NULL != strstr(V, "launcher"));
                        //bLaser = (NULL != strstr(V, "laser"));
                        //bTacticalTorch = (NULL != strstr(V, "torch"));

                        if (NULL != strstr(V, "ammo_type="))
                            i_ammo_type = atoi(strstr(V, "ammo_type=") + 10);

                        if (nullptr != strstr(V, "scope="))
                            cur_scope = atoi(strstr(V, "scope=") + 6);
                    }


                    CSE_Abstract* E = alife().spawn_item(itmSection, o_Position, m_tNodeID, m_tGraphID, ID);
                    CSE_ALifeItemWeapon* W = smart_cast<CSE_ALifeItemWeapon*>(E);

                    if (W)
                    {
                        if (W->m_scope_status == ALife::eAddonAttachable)
                        {
                            W->m_addon_flags.set(CSE_ALifeItemWeapon::eWeaponAddonScope, bScope);
                            W->cur_scope = cur_scope;
                        }
                        if (W->m_silencer_status == ALife::eAddonAttachable)
                            W->m_addon_flags.set(CSE_ALifeItemWeapon::eWeaponAddonSilencer, bSilencer);
                        if (W->m_grenade_launcher_status == ALife::eAddonAttachable)
                            W->m_addon_flags.set(CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher, bLauncher);
                        /*if (W->m_laser_designator_status == ALife::eAddonAttachable)
                            W->m_addon_flags.set(CSE_ALifeItemWeapon::eWeaponAddonLaserDesignator, bLaser);
                        if (W->m_tactical_torch_status == ALife::eAddonAttachable)
                            W->m_addon_flags.set(CSE_ALifeItemWeapon::eWeaponAddonTacticalTorch, bTacticalTorch); */

                        //spawn count box(es) of the correct ammo for weapon
                        if (pSettings->line_exist(itmSection, "ammo_class"))
                        {
                            LPCSTR ammo_class = pSettings->r_string(itmSection, "ammo_class");
                            LPCSTR ammoSec = "";

                            for (int i = 0, n = _GetItemCount(ammo_class); i < n; ++i)
                            {
                                string128 tmp;
                                ammoSec = _GetItem(ammo_class, i, tmp);
                                if (i == i_ammo_type)
                                    break;
                            }

                            if (xr_strlen(ammoSec) && pSettings->section_exist(ammoSec))
                            {
                                for (u32 i = 1; i <= spawn_count; ++i)
                                    alife().spawn_item(ammoSec, o_Position, m_tNodeID, m_tGraphID, ID);
                            }
                        }
                    }

                    CSE_ALifeInventoryItem* IItem = smart_cast<CSE_ALifeInventoryItem*>(E);

                    if (IItem)
                        IItem->m_fCondition = f_cond;
                }
            }
        }

        iItr++;
        string32 buf;
        loadout_section = strconcat(sizeof(buf), buf, "spawn_loadout", std::to_string(iItr).c_str());
    }
    //-Alundaio

    if (ini.section_exist("spawn"))
    {
        pcstr N, V;
        float p;
        for (u32 k = 0, j; ini.r_line("spawn", k, &N, &V); k++)
        {
            VERIFY(xr_strlen(N));

            if (pSettings->section_exist(N)) //Alundaio: verify item section exists!
            {
                float f_cond = 1.0f;
                bool bScope = false;
                bool bSilencer = false;
                bool bLauncher = false;
                //bool bLaser = false;
                //bool bTacticalTorch = false;
                int cur_scope = 0;

                j = 1;
                p = 1.f;

                if (V && xr_strlen(V))
                {
                    string64 buf;
                    j = atoi(_GetItem(V, 0, buf));
                    if (!j)
                        j = 1;

                    bScope = nullptr != strstr(V, "scope");
                    bSilencer = nullptr != strstr(V, "silencer");
                    bLauncher = nullptr != strstr(V, "launcher");
                    //bLaser = nullptr != strstr(V, "laser");
                    //bTacticalTorch = nullptr != strstr(V, "torch");

                    // probability
                    if (nullptr != strstr(V, "prob="))
                        p = static_cast<float>(atof(strstr(V, "prob=") + 5));
                    if (fis_zero(p))
                        p = 1.0f;
                    if (nullptr != strstr(V, "cond="))
                        f_cond = static_cast<float>(atof(strstr(V, "cond=") + 5));
                    if (nullptr != strstr(V, "scope="))
                        cur_scope = atoi(strstr(V, "scope=") + 6);
                }
                for (u32 i = 0; i < j; ++i)
                {
                    if (randF(1.f) < p)
                    {
                        CSE_Abstract* E = alife().spawn_item(N, o_Position, m_tNodeID, m_tGraphID, ID);
                        //подсоединить аддоны к оружию, если включены соответствующие флажки
                        CSE_ALifeItemWeapon* W = smart_cast<CSE_ALifeItemWeapon*>(E);
                        if (W)
                        {
                            if (W->m_scope_status == ALife::eAddonAttachable)
                            {
                                W->m_addon_flags.set(CSE_ALifeItemWeapon::eWeaponAddonScope, bScope);
                                W->cur_scope = cur_scope;
                            }
                            if (W->m_silencer_status == ALife::eAddonAttachable)
                                W->m_addon_flags.set(CSE_ALifeItemWeapon::eWeaponAddonSilencer, bSilencer);
                            if (W->m_grenade_launcher_status == ALife::eAddonAttachable)
                                W->m_addon_flags.set(CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher, bLauncher);
                            /*if (W->m_laser_designator_status == ALife::eAddonAttachable)
                                W->m_addon_flags.set(CSE_ALifeItemWeapon::eWeaponAddonLaserDesignator, bLaser);
                            if (W->m_tactical_torch_status == ALife::eAddonAttachable)
                                W->m_addon_flags.set(CSE_ALifeItemWeapon::eWeaponAddonTacticalTorch, bTacticalTorch); */
                        }
                        CSE_ALifeInventoryItem* IItem = smart_cast<CSE_ALifeInventoryItem*>(E);
                        if (IItem)
                            IItem->m_fCondition = f_cond;
                    }
                }
            }
        }
    }
}

bool CSE_ALifeObject::keep_saved_data_anyway() const
{
	return			(false);
}
