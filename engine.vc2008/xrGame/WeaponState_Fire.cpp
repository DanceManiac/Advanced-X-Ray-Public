/********************************/
/***** Состояние "Стрельба" *****/ //--#SM+#--
/********************************/

#include "StdAfx.h"
#include "Weapon.h"

#include "reward_event_generator.h"
#include "HUDManager.h"
#include "ExplosiveRocket.h"
#include "MPPlayersBag.h"

#ifdef DEBUG
#include "object_handler_planner.h"
#endif

// Калбэк на выстрел
void CWeapon::OnShot(bool bIsRocket, bool bIsBaseDispersionedBullet)
{
    Fvector vel;
    PHGetLinearVell(vel);

    // Партиклы, звуки, анимации, эффекты
    if (bIsBaseDispersionedBullet == false || m_iShotNum == 1)
    {
        PlayAnimShoot();   // Анимация выстрела
        AddShotEffector(); // Эффекты экрана (отдача от выстрела)

        // Звук
        if (m_bGrenadeMode)
            PlaySound("sndShotG", get_LastFP2());
        else
        {
            // Основной звук выстрела
            //--> Получаем алиас текущего звука выстрела
            LPCSTR sShotSnd = m_sSndShotCurrent.c_str();

            //--> Считаем случайную частоту звука и выставляем её глобально для всей игры
            float fRndFreq = Random.randF(m_vShotSndFreq.x, m_vShotSndFreq.y);
            HUD_SOUND_ITEM::SetHudSndGlobalFrequency(fRndFreq);

            //--> Запускаем звук
            PlaySound(sShotSnd, get_LastFP());

            //--> Восстанавливаем оригинальную частоту
            HUD_SOUND_ITEM::SetHudSndGlobalFrequency(1.0f);

            // Эхо выстрела
            if (IsSilencerAttached() == false)
            {
                bool bIndoor = false;
                if (H_Parent() != nullptr)
                {
                    bIndoor = H_Parent()->renderable_ROS()->get_luminocity_hemi() < WEAPON_INDOOR_HEMI_FACTOR;
                }

                if (bIndoor == false && m_sounds.FindSoundItem("sndReflect", false))
                {
                    if (IsHudModeNow())
                    {
                        HUD_SOUND_ITEM::SetHudSndGlobalVolumeFactor(WEAPON_SND_REFLECTION_HUD_FACTOR);
                    }
                    PlaySound("sndReflect", get_LastFP());
                    HUD_SOUND_ITEM::SetHudSndGlobalVolumeFactor(1.0f);
                }
            }

            // Звук ленты
            if (m_sounds.FindSoundItem("sndTape", false))
            {
                PlaySound("sndTape", get_LastFP());
            }
        }

        // Партиклы
        if (m_bGrenadeMode)
        {
            // Огонь из ствола
            StartFlameParticles2();
        }
        else
        {
            // Огонь из ствола
            StartFlameParticles();

            // Дым из ствола
            ForceUpdateFireParticles();
            StartSmokeParticles(get_LastFP(), vel);
        }
    }

    // Гильзы
    if (m_bGrenadeMode == false)
    {
        // 2D-Гильзы
        if (!CanUsePumpMode())
            LaunchShell2D(&vel);

        // 3D-Гильзы
        do
        {
            if (m_bGrenadeMode == false)
            {
                if (m_bDontSpawnShell3DForFirstBullet == true && GetMainAmmoElapsed() == (GetMainMagSize() - 1))
                { //--> Не спавним гильзу при первом выстреле из полного магазина
                    break;
                }

                if (m_bDontSpawnShell3DForLastBullet == true && GetMainAmmoElapsed() == 0)
                { //--> Не спавним гильзу при последнем выстреле
                    break;
                }
            }

            if (bIsBaseDispersionedBullet == true && m_iShotNum > 1)
            { //--> При выстреле нескольких пуль за раз - растягиваем спавн гильзы по времени
                m_Shells3DQueue.push_back(Device.dwTimeGlobal + ((m_iShotNum - 1) * 80));
            }
            else
            { //--> Запускаем гильзу сразу
                for (int _idx = 1; _idx <= GetLPCount(); _idx++)
                    LaunchShell3D(_idx, (_idx == 1 ? m_sCurShell3DSect.c_str() : nullptr));
            }
        } while (false);
    }

    // Эффект сдвига (отдача)
    AddHUDShootingEffect();
}