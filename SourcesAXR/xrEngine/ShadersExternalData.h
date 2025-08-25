#pragma once

//    ,    Blender_Recorder_StandartBinding.cpp
class ShadersExternalData //--#SM+#--
{
public:
	Fmatrix m_script_params; // ,     Lua
	Fvector4 hud_params;     // [zoom_rotate_factor, secondVP_zoom_factor, NULL, NULL] -   
	Fvector4 m_blender_mode; // x\y = [0 - default, 1 - night vision, 2 - thermo vision, ... . common.h] -  
							 // x -  , y -  , z = ?, w = [0 -    , 1 -     (, )]
	Fvector4 m_condition_params; //X - weapon

	ShadersExternalData()
	{
		m_script_params = Fmatrix();
		hud_params.set(0.f, 0.f, 0.f, 0.f);
		m_blender_mode.set(0.f, 0.f, 0.f, 0.f);
		m_condition_params.set(0.f, 0.f, 0.f, 0.f);
	}
};
