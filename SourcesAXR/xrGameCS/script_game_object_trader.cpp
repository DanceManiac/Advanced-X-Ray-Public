////////////////////////////////////////////////////////////////////////////
// script_game_object_trader.сpp :	функции для торговли и торговцев
//////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "script_game_object.h"
#include "script_game_object_impl.h"
#include "ai/trader/ai_trader.h"
#include "ai/trader/trader_animation.h"

void CScriptGameObject::set_trader_global_anim(LPCSTR anim)
{
	MakeObj(CAI_Trader,trader);
	trader->animation().set_animation(anim);

}
void CScriptGameObject::set_trader_head_anim(LPCSTR anim)
{
	MakeObj(CAI_Trader,trader);
	trader->animation().set_head_animation(anim);
}

void CScriptGameObject::set_trader_sound(LPCSTR sound, LPCSTR anim)
{
	MakeObj(CAI_Trader,trader);
	trader->animation().set_sound(sound, anim);
}

void CScriptGameObject::external_sound_start(LPCSTR sound)
{
	MakeObj(CAI_Trader,trader);
	trader->animation().external_sound_start(sound);
}

void CScriptGameObject::external_sound_stop()
{
	MakeObj(CAI_Trader,trader);
	trader->animation().external_sound_stop();
}