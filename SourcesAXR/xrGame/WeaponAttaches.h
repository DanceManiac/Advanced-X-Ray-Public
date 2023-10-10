#pragma once

#include "../Include/xrRender/KinematicsAnimated.h"
#include "Weapon.h"

class WeaponAttach
{
public:
	WeaponAttach();
	~WeaponAttach() {};
	IRenderVisual* attach_hud_visual;
	Fvector			hud_attach_pos[2];
	Fvector			world_attach_pos[2];
	Fmatrix			m_hud_attach_pos;
	Fmatrix			m_hud_attach_offset;
	Fmatrix			m_world_attach_pos;
	Fmatrix			m_world_attach_offset;

	shared_str		m_attach_bone_name;		//позиция кости которая будет использоваться для аттача, если пусто то аттачим к кости wpn_body
	shared_str		m_visualHUDName;		//путь до меша
	shared_str		m_section;				//название секции

	WeaponAttach* CreateAttach(shared_str attach_section, xr_vector<WeaponAttach*>& m_attaches);
	void RemoveAttach(shared_str attach_section, xr_vector<WeaponAttach*>& m_attaches);
	void UpdateAttachesPosition(IRenderVisual* model, const Fmatrix& parent, bool hud_mode = true);
	void RenderAttach(bool hud_mode = true);
	void Load(shared_str attach_sect);
};