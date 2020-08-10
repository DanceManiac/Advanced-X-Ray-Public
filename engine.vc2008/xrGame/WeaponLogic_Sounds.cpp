/************************************/
/***** Работа со звуками оружия *****/ //--#SM+#--
/************************************/
#include "StdAfx.h"
#include "Weapon.h"
// Перезагрузка всех звуков, с учётом текущих аддонов
// Если передана секция - то будут загружены только звуки, присутствующие в ней (для апгрейдов)
void CWeapon::ReloadAllSounds(LPCSTR sFromSect)
{
    sReloadSndSectOverride = (sFromSect ? sFromSect : nullptr);
    ReloadAllSounds();
    sReloadSndSectOverride = nullptr;
}
// Перезагрузка всех звуков, с учётом текущих аддонов
void CWeapon::ReloadAllSounds()
{
    // clang-format off
	//		<Имя звука из конфига>		  <Движковый SID звука> <Флаг эксклюзивности>  <Тип звука для НПС>
	//		Одновременно на оружии может играться только один эксклюзивный звук - другие экслюзивные будут остановленны <!>
    ReloadSound( "snd_reflection",              "sndReflect"            , ESndExcl::eExNot,	    SOUND_TYPE_WORLD_AMBIENT			);
    ReloadSound( "snd_knife",					"sndKnife"				, ESndExcl::eExNot,	    SOUND_TYPE_WEAPON_SHOOTING			);
}