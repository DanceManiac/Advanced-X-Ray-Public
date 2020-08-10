/************************************/
/***** ������ �� ������� ������ *****/ //--#SM+#--
/************************************/
#include "StdAfx.h"
#include "Weapon.h"
// ������������ ���� ������, � ������ ������� �������
// ���� �������� ������ - �� ����� ��������� ������ �����, �������������� � ��� (��� ���������)
void CWeapon::ReloadAllSounds(LPCSTR sFromSect)
{
    sReloadSndSectOverride = (sFromSect ? sFromSect : nullptr);
    ReloadAllSounds();
    sReloadSndSectOverride = nullptr;
}
// ������������ ���� ������, � ������ ������� �������
void CWeapon::ReloadAllSounds()
{
    // clang-format off
	//		<��� ����� �� �������>		  <��������� SID �����> <���� ��������������>  <��� ����� ��� ���>
	//		������������ �� ������ ����� �������� ������ ���� ������������ ���� - ������ ����������� ����� ������������ <!>
    ReloadSound( "snd_reflection",              "sndReflect"            , ESndExcl::eExNot,	    SOUND_TYPE_WORLD_AMBIENT			);
    ReloadSound( "snd_knife",					"sndKnife"				, ESndExcl::eExNot,	    SOUND_TYPE_WEAPON_SHOOTING			);
}