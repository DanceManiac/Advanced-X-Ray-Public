/********************************/
/***** ��������� "��������" *****/ //--#SM+#--
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

// ������ �� �������
void CWeapon::OnShot(bool bIsRocket, bool bIsBaseDispersionedBullet)
{
    Fvector vel;
    PHGetLinearVell(vel);

    // ��������, �����, ��������, �������
    if (bIsBaseDispersionedBullet == false || m_iShotNum == 1)
    {
        PlayAnimShoot();   // �������� ��������
        AddShotEffector(); // ������� ������ (������ �� ��������)

        // ����
        if (m_bGrenadeMode)
            PlaySound("sndShotG", get_LastFP2());
        else
        {
            // �������� ���� ��������
            //--> �������� ����� �������� ����� ��������
            LPCSTR sShotSnd = m_sSndShotCurrent.c_str();

            //--> ������� ��������� ������� ����� � ���������� � ��������� ��� ���� ����
            float fRndFreq = Random.randF(m_vShotSndFreq.x, m_vShotSndFreq.y);
            HUD_SOUND_ITEM::SetHudSndGlobalFrequency(fRndFreq);

            //--> ��������� ����
            PlaySound(sShotSnd, get_LastFP());

            //--> ��������������� ������������ �������
            HUD_SOUND_ITEM::SetHudSndGlobalFrequency(1.0f);

            // ��� ��������
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

            // ���� �����
            if (m_sounds.FindSoundItem("sndTape", false))
            {
                PlaySound("sndTape", get_LastFP());
            }
        }

        // ��������
        if (m_bGrenadeMode)
        {
            // ����� �� ������
            StartFlameParticles2();
        }
        else
        {
            // ����� �� ������
            StartFlameParticles();

            // ��� �� ������
            ForceUpdateFireParticles();
            StartSmokeParticles(get_LastFP(), vel);
        }
    }

    // ������
    if (m_bGrenadeMode == false)
    {
        // 2D-������
        if (!CanUsePumpMode())
            LaunchShell2D(&vel);

        // 3D-������
        do
        {
            if (m_bGrenadeMode == false)
            {
                if (m_bDontSpawnShell3DForFirstBullet == true && GetMainAmmoElapsed() == (GetMainMagSize() - 1))
                { //--> �� ������� ������ ��� ������ �������� �� ������� ��������
                    break;
                }

                if (m_bDontSpawnShell3DForLastBullet == true && GetMainAmmoElapsed() == 0)
                { //--> �� ������� ������ ��� ��������� ��������
                    break;
                }
            }

            if (bIsBaseDispersionedBullet == true && m_iShotNum > 1)
            { //--> ��� �������� ���������� ���� �� ��� - ����������� ����� ������ �� �������
                m_Shells3DQueue.push_back(Device.dwTimeGlobal + ((m_iShotNum - 1) * 80));
            }
            else
            { //--> ��������� ������ �����
                for (int _idx = 1; _idx <= GetLPCount(); _idx++)
                    LaunchShell3D(_idx, (_idx == 1 ? m_sCurShell3DSect.c_str() : nullptr));
            }
        } while (false);
    }

    // ������ ������ (������)
    AddHUDShootingEffect();
}