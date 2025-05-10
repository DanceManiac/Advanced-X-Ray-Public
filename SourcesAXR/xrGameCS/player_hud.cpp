#include "stdafx.h"
#include "pch_script.h"
#include "player_hud.h"
#include "HudItem.h"
#include "ui_base.h"
#include "actor.h"
#include "physic_item.h"
#include "static_cast_checked.hpp"
#include "actoreffector.h"
#include "ai_space.h"
#include "script_engine.h"
#include "../xrEngine/IGame_Persistent.h"
#include "Weapon.h"

player_hud* g_player_hud = NULL;
Fvector _ancor_pos;
Fvector _wpn_root_pos;

// Рассчитать стартовую секунду анимации --#SM+#--
float CalculateMotionStartSeconds(float fStartFromTime, float fMotionLength)
{
	R_ASSERT(fStartFromTime >= -1.0f);

	//if (fStartFromTime >= 0.0f)
	//{ 
	//    // Выставляем время в точных значениях
	//    clamp(fStartFromTime, 0.0f, fMotionLength);
	//    return abs(fStartFromTime);
	//}
	//else
	{   // Выставляем время в процентных значениях (от всей длины анимации)
		return (abs(fStartFromTime) * fMotionLength);
	}
}

player_hud_motion* player_hud_motion_container::find_motion(const shared_str& name, bool withSuffix)
{
	if (!withSuffix)
	{
		for (auto& motion : m_anims)
		{
			const shared_str& s = motion.m_alias_name;
			if (s == name)
				return &motion;
		}
	}
	else
	{
		for (auto& motion : m_anims)
		{
			const shared_str& s = motion.m_alias_name;
			if (strstr(s.c_str(), name.c_str()) == s.c_str())
				return &motion;
		}
	}

	return nullptr;
}

void player_hud_motion_container::load(IKinematicsAnimated* model, const shared_str& sect)
{
	CInifile::Sect& _sect		= pSettings->r_section(sect);
	CInifile::SectCIt _b		= _sect.Data.begin();
	CInifile::SectCIt _e		= _sect.Data.end();
	player_hud_motion* pm		= NULL;
	
	string512					buff;
	MotionID					motion_ID;

	for(;_b!=_e;++_b)
	{
		if( (strstr(_b->first.c_str(), "anm_")==_b->first.c_str())
			&& !strstr(_b->first.c_str(), "_speed_k") && !strstr(_b->first.c_str(), "_start_k") && !strstr(_b->first.c_str(), "_stop_k") && !strstr(_b->first.c_str(), "_effector"))
		{
			const shared_str& anm	= _b->second;
			m_anims.resize			(m_anims.size()+1);
			pm						= &m_anims.back();
			//base and alias name
			pm->m_alias_name		= _b->first;
			
			if(_GetItemCount(anm.c_str())==1)
			{
				pm->m_base_name			= anm;
				pm->m_additional_name	= anm;
			}
			else
			{
				R_ASSERT2(_GetItemCount(anm.c_str())<=3, anm.c_str());
				string512				str_item;
				_GetItem(anm.c_str(),0,str_item);
				pm->m_base_name			= str_item;

				_GetItem(anm.c_str(),1,str_item);
				pm->m_additional_name	= (xr_strlen(str_item) > 0) ? pm->m_additional_name = str_item : pm->m_base_name;

				_GetItem(anm.c_str(), 2, str_item);
				pm->m_anim_speed = atof(str_item);
			}

			string128 speed_param;
			strconcat(sizeof(speed_param), speed_param, _b->first.c_str(), "_speed_k");
			if (pSettings->line_exist(sect, speed_param))
			{
				const float k = pSettings->r_float(sect, speed_param);
				if (!fsimilar(k, 1.f) && k > 0.001f)
					pm->params.speed_k = k;
			}

			string128 stop_param;
			strconcat(sizeof(stop_param), stop_param, _b->first.c_str(), "_stop_k");
			if (pSettings->line_exist(sect, stop_param))
			{
				const float k = pSettings->r_float(sect, stop_param);
				if (k < 1.f && k > 0.001f)
					pm->params.stop_k = k;
			}

			string128 start_param;
			strconcat(sizeof(start_param), start_param, _b->first.c_str(), "_start_k");
			if (pSettings->line_exist(sect, start_param))
			{
				const float k = pSettings->r_float(sect, start_param);
				if (k < 1.f && k > 0.001f)
					pm->params.start_k = k;
			}

			//and load all motions for it

			for(u32 i=0; i<=8; ++i)
			{
				if(i==0)
					xr_strcpy				(buff,pm->m_base_name.c_str());		
				else
					xr_sprintf				(buff,"%s%d",pm->m_base_name.c_str(),i);		

				motion_ID				= model->ID_Cycle_Safe(buff);
				if(motion_ID.valid())
				{
					pm->m_animations.resize			(pm->m_animations.size()+1);
					pm->m_animations.back().mid		= motion_ID;
					pm->m_animations.back().name	= buff;
					
					string128 eff_param;
					pm->m_animations.back().eff_name = READ_IF_EXISTS(pSettings, r_string, sect, strconcat(sizeof(eff_param), eff_param, _b->first.c_str(), "_effector"), nullptr);

#ifdef DEBUG
					Msg(" alias=[%s] base=[%s] name=[%s]",pm->m_alias_name.c_str(), pm->m_base_name.c_str(), buff);
#endif // #ifdef DEBUG
				}
			}
			//VERIFY2(pm->m_animations.size(),make_string("motion not found [%s]", pm->m_base_name.c_str()).c_str());

			if (pm->m_animations.empty())
			{
				Msg("! [%s] motion [%s](%s) not found in section [%s]", __FUNCTION__, pm->m_base_name.c_str(), _b->first.c_str(), sect.c_str());
				continue;
			}
		}
	}
}

Fvector& attachable_hud_item::hands_attach_pos()
{
	return m_measures.m_hands_attach[0];
}

Fvector& attachable_hud_item::hands_attach_rot()
{
	return m_measures.m_hands_attach[1];
}

Fvector& attachable_hud_item::hands_offset_pos()
{
	u8 idx	= m_parent_hud_item->GetCurrentHudOffsetIdx();
	return m_measures.m_hands_offset[0][idx];
}

Fvector& attachable_hud_item::hands_offset_rot()
{
	u8 idx	= m_parent_hud_item->GetCurrentHudOffsetIdx();
	return m_measures.m_hands_offset[1][idx];
}

void attachable_hud_item::set_bone_visible(const shared_str& bone_name, BOOL bVisibility, BOOL bSilent)
{
	u16  bone_id;
	BOOL bVisibleNow;
	bone_id			= m_model->LL_BoneID			(bone_name);
	if(bone_id==BI_NONE)
	{
		if(bSilent)	return;
		R_ASSERT2	(0,			make_string("model [%s] has no bone [%s]",pSettings->r_string(m_sect_name, "item_visual"), bone_name.c_str()).c_str());
	}
	bVisibleNow		= m_model->LL_GetBoneVisible	(bone_id);
	if(bVisibleNow!=bVisibility)
		m_model->LL_SetBoneVisible	(bone_id,bVisibility, TRUE);
}

void attachable_hud_item::update(bool bForce)
{
	if(!bForce && m_upd_firedeps_frame==Device.dwFrame)	return;
	bool is_16x9 = UI().is_widescreen();
	
	if(!!m_measures.m_prop_flags.test(hud_item_measures::e_16x9_mode_now)!=is_16x9)
		m_measures.load(m_sect_name, m_model);

	Fvector ypr						= m_measures.m_item_attach[1];
	ypr.mul							(PI/180.f);
	m_attach_offset.setHPB			(ypr.x,ypr.y,ypr.z);
	m_attach_offset.translate_over	(m_measures.m_item_attach[0]);

	m_parent->calc_transform		(m_attach_place_idx, m_attach_offset, m_item_transform);
	m_parent_hud_item->UpdateAddonsTransform(true);
	m_upd_firedeps_frame			= Device.dwFrame;

	IKinematicsAnimated* ka			=	m_model->dcast_PKinematicsAnimated();
	if(ka)
	{
		ka->UpdateTracks									();
		ka->dcast_PKinematics()->CalculateBones_Invalidate	();
		ka->dcast_PKinematics()->CalculateBones				(TRUE);
	}
	m_parent_hud_item->merge_measures_params();
}

void attachable_hud_item::setup_firedeps(firedeps& fd)
{
	update							(false);
	// fire point&direction
	if(m_measures.m_prop_flags.test(hud_item_measures::e_fire_point))
	{
		Fmatrix& fire_mat								= m_model->LL_GetTransform(m_measures.m_fire_bone);
		fire_mat.transform_tiny							(fd.vLastFP, m_measures.m_fire_point_offset);
		m_item_transform.transform_tiny					(fd.vLastFP);
		fd.vLastFP.add									(Device.vCameraPosition);

		fd.vLastFD.set									(0.f,0.f,1.f);
		m_item_transform.transform_dir					(fd.vLastFD);

		auto Wpn = smart_cast<CWeapon*>(m_parent_hud_item);

		if (Wpn)
			Wpn->CorrectDirFromWorldToHud(fd.vLastFD);

		VERIFY(_valid(fd.vLastFD));

		fd.m_FireParticlesXForm.identity				();
		fd.m_FireParticlesXForm.k.set					(fd.vLastFD);
		Fvector::generate_orthonormal_basis_normalized	(	fd.m_FireParticlesXForm.k,
															fd.m_FireParticlesXForm.j, 
															fd.m_FireParticlesXForm.i);
		VERIFY(_valid(fd.m_FireParticlesXForm));
	}

	if(m_measures.m_prop_flags.test(hud_item_measures::e_fire_point2))
	{
		Fmatrix& fire_mat			= m_model->LL_GetTransform(m_measures.m_fire_bone2);
		fire_mat.transform_tiny		(fd.vLastFP2,m_measures.m_fire_point2_offset);
		m_item_transform.transform_tiny	(fd.vLastFP2);
		fd.vLastFP2.add(Device.vCameraPosition);
		VERIFY(_valid(fd.vLastFP2));
	}

	if(m_measures.m_prop_flags.test(hud_item_measures::e_shell_point))
	{
		Fmatrix& fire_mat			= m_model->LL_GetTransform(m_measures.m_shell_bone);
		fire_mat.transform_tiny		(fd.vLastSP,m_measures.m_shell_point_offset);
		m_item_transform.transform_tiny	(fd.vLastSP);
		fd.vLastSP.add(Device.vCameraPosition);
		VERIFY(_valid(fd.vLastSP));
	}

	if (m_measures.m_prop_flags.test(hud_item_measures::e_overheating_smoke_point))
	{
		Fmatrix& fire_mat			= m_model->LL_GetTransform(m_measures.m_overheating_smoke_bone);
		fire_mat.transform_tiny		(fd.vLastOSP, m_measures.m_overheating_smoke_offset);
		m_item_transform.transform_tiny(fd.vLastOSP);
		fd.vLastOSP.add(Device.vCameraPosition);
		VERIFY(_valid(fd.vLastOSP));
	}
}

bool  attachable_hud_item::need_renderable()
{
	return m_parent_hud_item->need_renderable();
}

void attachable_hud_item::render()
{
	::Render->set_Transform		(&m_item_transform);
	::Render->add_Visual		(m_model->dcast_RenderVisual(), true);
	debug_draw_firedeps			();
	m_parent_hud_item->render_hud_mode();
}

bool attachable_hud_item::render_item_ui_query()
{
	return m_parent_hud_item->render_item_3d_ui_query();
}

void attachable_hud_item::render_item_ui()
{
	m_parent_hud_item->render_item_3d_ui();
}

void hud_item_measures::load(const shared_str& sect_name, IKinematics* K)
{
	bool is_16x9 = UI().is_widescreen();
	string64	_prefix;
	xr_sprintf	(_prefix,"%s",is_16x9?"_16x9":"");
	string128	val_name;

	strconcat					(sizeof(val_name),val_name,"hands_position",_prefix);
	if (is_16x9 && !pSettings->line_exist(sect_name, val_name))
		xr_strcpy(val_name, "hands_position");
	m_hands_attach[0]			= READ_IF_EXISTS(pSettings, r_fvector3, sect_name, val_name, Fvector{});
	strconcat					(sizeof(val_name),val_name,"hands_orientation",_prefix);
	if (is_16x9 && !pSettings->line_exist(sect_name, val_name))
		xr_strcpy(val_name, "hands_orientation");
	m_hands_attach[1]			= READ_IF_EXISTS(pSettings, r_fvector3, sect_name, val_name, Fvector{});

	if (!pSettings->line_exist(sect_name, "item_position") && pSettings->line_exist(sect_name, "position"))
		m_item_attach[0]		= pSettings->r_fvector3(sect_name, "position");
	else
		m_item_attach[0]		= pSettings->r_fvector3(sect_name, "item_position");

	if (!pSettings->line_exist(sect_name, "item_orientation") && pSettings->line_exist(sect_name, "orientation"))
		m_item_attach[1]		= pSettings->r_fvector3(sect_name, "orientation");
	else
		m_item_attach[1]		= pSettings->r_fvector3(sect_name, "item_orientation");

	shared_str					 bone_name;
	m_prop_flags.set			 (e_fire_point,pSettings->line_exist(sect_name,"fire_bone"));
	
	if(m_prop_flags.test(e_fire_point))
	{
		bone_name				= pSettings->r_string(sect_name, "fire_bone");
		m_fire_bone				= K->LL_BoneID(bone_name);
		R_ASSERT4				(m_fire_bone != BI_NONE, "!![%s] bone [%s] not found in weapon [%s]", bone_name.c_str(), sect_name.c_str());
		m_fire_point_offset		= pSettings->r_fvector3(sect_name, "fire_point");
	}
	else
		m_fire_point_offset.set(0.f, 0.f, 0.f);

	m_prop_flags.set			 (e_fire_point2,pSettings->line_exist(sect_name,"fire_bone2"));
	
	if(m_prop_flags.test(e_fire_point2))
	{
		bone_name				= pSettings->r_string(sect_name, "fire_bone2");
		m_fire_bone2			= K->LL_BoneID(bone_name);
		R_ASSERT4				(m_fire_bone2 != BI_NONE, "!![%s] bone [%s] not found in weapon [%s]", bone_name.c_str(), sect_name.c_str());
		m_fire_point2_offset	= pSettings->r_fvector3(sect_name, "fire_point2");
	}
	else
		m_fire_point2_offset.set(0.f, 0.f, 0.f);

	m_prop_flags.set			 (e_shell_point,pSettings->line_exist(sect_name,"shell_bone"));
	
	if(m_prop_flags.test(e_shell_point))
	{
		bone_name				= pSettings->r_string(sect_name, "shell_bone");
		m_shell_bone			= K->LL_BoneID(bone_name);
		R_ASSERT4				(m_shell_bone != BI_NONE, "!![%s] bone [%s] not found in weapon [%s]", bone_name.c_str(), sect_name.c_str());
		m_shell_point_offset	= pSettings->r_fvector3(sect_name, "shell_point");
	}
	else
		m_shell_point_offset.set(0.f, 0.f, 0.f);

	LPCSTR overheating_bone_var	= pSettings->line_exist(sect_name, "overheating_smoke_bone") ? "overheating_smoke_bone" : "fire_bone";
	m_prop_flags.set			 (e_overheating_smoke_point, pSettings->line_exist(sect_name, overheating_bone_var));
	
	if(m_prop_flags.test(e_overheating_smoke_point))
	{
		bone_name				= pSettings->r_string(sect_name, overheating_bone_var);
		m_overheating_smoke_bone = K->LL_BoneID(bone_name);
		R_ASSERT4				(m_overheating_smoke_bone != BI_NONE, "!![%s] bone [%s] not found in weapon [%s]", bone_name.c_str(), sect_name.c_str());
		m_overheating_smoke_offset = pSettings->r_fvector3(sect_name, overheating_bone_var);
	}
	else
		m_overheating_smoke_offset.set(0.f, 0.f, 0.f);

	m_hands_offset[0][0].set	(0,0,0);
	m_hands_offset[1][0].set	(0,0,0);

	strconcat					(sizeof(val_name),val_name,"aim_hud_offset_pos",_prefix);
	if (is_16x9 && !pSettings->line_exist(sect_name, val_name))
		xr_strcpy(val_name, "aim_hud_offset_pos");
	m_hands_offset[0][1] = READ_IF_EXISTS(pSettings, r_fvector3, sect_name, val_name, Fvector{});

	strconcat					(sizeof(val_name),val_name,"aim_hud_offset_rot",_prefix);
	if (is_16x9 && !pSettings->line_exist(sect_name, val_name))
		xr_strcpy(val_name, "aim_hud_offset_rot");
	m_hands_offset[1][1] = READ_IF_EXISTS(pSettings, r_fvector3, sect_name, val_name, Fvector{});

	strconcat					(sizeof(val_name),val_name,"gl_hud_offset_pos",_prefix);
	if (is_16x9 && !pSettings->line_exist(sect_name, val_name))
		xr_strcpy(val_name, "gl_hud_offset_pos");
	m_hands_offset[0][2] = READ_IF_EXISTS(pSettings, r_fvector3, sect_name, val_name, Fvector{});

	strconcat					(sizeof(val_name),val_name,"gl_hud_offset_rot",_prefix);
	if (is_16x9 && !pSettings->line_exist(sect_name, val_name))
		xr_strcpy(val_name, "gl_hud_offset_rot");
	m_hands_offset[1][2] = READ_IF_EXISTS(pSettings, r_fvector3, sect_name, val_name, Fvector{});

	strconcat					(sizeof(val_name),val_name,"aim_alt_hud_offset_pos",_prefix);
	if (is_16x9 && !pSettings->line_exist(sect_name, val_name))
		xr_strcpy(val_name, "aim_alt_hud_offset_pos");
	m_hands_offset[0][3]		= READ_IF_EXISTS(pSettings, r_fvector3, sect_name, val_name, m_hands_offset[0][1]);

	strconcat					(sizeof(val_name),val_name,"aim_alt_hud_offset_rot",_prefix);
	if (is_16x9 && !pSettings->line_exist(sect_name, val_name))
		xr_strcpy(val_name, "aim_alt_hud_offset_rot");
	m_hands_offset[1][3]		= READ_IF_EXISTS(pSettings, r_fvector3, sect_name, val_name, m_hands_offset[1][1]);

	if (READ_IF_EXISTS(pSettings, r_bool, sect_name, "hud_collision_enabled", false))
	{
		strconcat(sizeof(val_name), val_name, "hud_collision_offset_pos", _prefix);
		m_collision_offset[0] = pSettings->r_fvector3(sect_name, val_name);
		strconcat(sizeof(val_name), val_name, "hud_collision_offset_rot", _prefix);
		m_collision_offset[1] = pSettings->r_fvector3(sect_name, val_name);
	}
	else
	{
		m_collision_offset[0] = Fvector().set(0.f, 0.f, 0.f);
		m_collision_offset[1] = Fvector().set(0.f, 0.f, 0.f);
	}

	R_ASSERT2(pSettings->line_exist(sect_name,"fire_point")==pSettings->line_exist(sect_name,"fire_bone"),		sect_name.c_str());
	R_ASSERT2(pSettings->line_exist(sect_name,"fire_point2")==pSettings->line_exist(sect_name,"fire_bone2"),	sect_name.c_str());
	R_ASSERT2(pSettings->line_exist(sect_name,"shell_point")==pSettings->line_exist(sect_name,"shell_bone"),	sect_name.c_str());
	R_ASSERT2(pSettings->line_exist(sect_name, "overheating_smoke_point") == pSettings->line_exist(sect_name, "overheating_smoke_bone"), sect_name.c_str());

	m_prop_flags.set(e_16x9_mode_now,is_16x9);

}

attachable_hud_item::~attachable_hud_item()
{
	IRenderVisual* v			= m_model->dcast_RenderVisual();
	::Render->model_Delete		(v);
	m_model						= nullptr;
}

void attachable_hud_item::load(const shared_str& sect_name)
{
	m_sect_name					= sect_name;

	// Visual
	const shared_str& visual_name = pSettings->r_string(sect_name, "item_visual");

	::Render->hud_loading = true;

	m_model						 = smart_cast<IKinematics*>(::Render->model_Create(visual_name.c_str()));

	::Render->hud_loading = false;

	m_attach_place_idx			= READ_IF_EXISTS(pSettings, r_u16, sect_name, "attach_place_idx", 0);
	m_measures.load				(sect_name, m_model);
}

u32 attachable_hud_item::anim_play(const shared_str& anm_name_b, BOOL bMixIn, const CMotionDef*& md, u8& rnd_idx, float speed)
{
	R_ASSERT				(strstr(anm_name_b.c_str(),"anm_")==anm_name_b.c_str() || strstr(anm_name_b.c_str(), "anim_") == anm_name_b.c_str());
	string256				anim_name_r{};
	bool is_16x9			= UI().is_widescreen();
	xr_sprintf				(anim_name_r,"%s%s",anm_name_b.c_str(),((m_attach_place_idx==1)&&is_16x9)?"_16x9":"");

	player_hud_motion* anm	= m_hand_motions.find_motion(anim_name_r);
	player_hud_motion* anm_def	= m_hand_motions.find_motion(anm_name_b);
	if (!anm)
		anm = anm_def;
	
	R_ASSERT2				(anm, make_string("model [%s] has no motion alias defined [%s]", m_sect_name.c_str(), anim_name_r).c_str());
	VERIFY2					(anm->m_animations.size(), make_string("model [%s] has no motion defined in motion_alias [%s]", pSettings->r_string(m_sect_name, "item_visual"), anim_name_r).c_str());
	
	rnd_idx					= (u8)Random.randI(anm->m_animations.size()) ;
	const motion_descr& M	= anm->m_animations[ rnd_idx ];

	if (speed == 1.f)
		speed = anm->m_anim_speed != 0 ? anm->m_anim_speed : anm->params.speed_k;

	u32 ret					= g_player_hud->anim_play(m_attach_place_idx, anm->params, M.mid, bMixIn, md, speed);
	
	if(m_model->dcast_PKinematicsAnimated())
	{
		IKinematicsAnimated* ka			= m_model->dcast_PKinematicsAnimated();

		shared_str item_anm_name;
		if(anm->m_base_name!=anm->m_additional_name)
			item_anm_name = anm->m_additional_name;
		else
			item_anm_name = M.name;

		MotionID M2						= ka->ID_Cycle_Safe(item_anm_name);
		if(!M2.valid())
			M2							= ka->ID_Cycle_Safe("idle");
		else
			if(bDebug)
				Msg						("playing item animation [%s]",item_anm_name.c_str());
		
		R_ASSERT3(M2.valid(), make_string("model has no motion [idle], section %s", m_sect_name.c_str()).c_str(), pSettings->r_string(m_sect_name, "item_visual"));

		u16 root_id						= m_model->LL_GetBoneRoot();
		CBoneInstance& root_binst		= m_model->LL_GetBoneInstance(root_id);
		root_binst.set_callback_overwrite(TRUE);
		root_binst.mTransform.identity	();

		u16 pc							= ka->partitions().count();
		for(u16 pid=0; pid<pc; ++pid)
		{
			CBlend* B					= ka->PlayCycle(pid, M2, bMixIn);
			R_ASSERT					(B);
			B->speed					*= speed;
			B->timeCurrent				= CalculateMotionStartSeconds(anm->params.start_k, B->timeTotal);
		}

		m_model->CalculateBones_Invalidate	();
	}

	R_ASSERT2		(m_parent_hud_item, "parent hud item is NULL");
	CPhysicItem&	parent_object = m_parent_hud_item->object();
	//R_ASSERT2		(parent_object, "object has no parent actor");
	//CObject*		parent_object = static_cast_checked<CObject*>(&m_parent_hud_item->object());

	if (IsGameTypeSingle() && parent_object.H_Parent() == Level().CurrentControlEntity())
	{
		CActor* current_actor = static_cast_checked<CActor*>(Level().CurrentControlEntity());
		VERIFY(current_actor);

		string_path ce_path;
		string_path anm_name;
		strconcat(sizeof(anm_name), anm_name, "camera_effects" "\\" "weapon" "\\", M.eff_name ? M.eff_name : M.name.c_str(), ".anm");

		if (FS.exist(ce_path, "$game_anims$", anm_name))
		{
			CEffectorCam* ec = current_actor->Cameras().GetCamEffector(eCEWeaponAction);

			if (ec)
				current_actor->Cameras().RemoveCamEffector(eCEWeaponAction);

			CAnimatorCamEffector* e		= xr_new<CAnimatorCamEffector>();
			e->SetType					(eCEWeaponAction);
			e->SetHudAffect				(false);
			e->SetCyclic				(false);
			e->Start					(anm_name);
			current_actor->Cameras().AddCamEffector(e);
		}
	}
	return ret;
}


player_hud::player_hud()
{
	m_model					= nullptr;
	m_model_2				= nullptr;
	m_attached_items[0]		= nullptr;
	m_attached_items[1]		= nullptr;
	m_transform.identity	();
	m_transform_2.identity	();
	script_anim_part = u8(-1);
	script_anim_offset_factor = 0.f;
	script_anim_item_model	= nullptr;
	m_item_pos.identity();
	reset_thumb(true);

	m_current_motion_def	= nullptr;
	
	if (pAdvancedSettings->section_exist("hud_movement_layers"))
	{
		m_movement_layers.reserve(move_anms_end);

		for (int i = 0; i < move_anms_end; i++)
		{
			movement_layer* anm = xr_new<movement_layer>();

			char temp[20];
			string512 tmp;
			strconcat(sizeof(temp), temp, "movement_layer_", std::to_string(i).c_str());
			R_ASSERT2(pAdvancedSettings->line_exist("hud_movement_layers", temp), make_string("Missing definition for [hud_movement_layers] %s", temp));
			LPCSTR layer_def = pAdvancedSettings->r_string("hud_movement_layers", temp);
			R_ASSERT2(_GetItemCount(layer_def) > 0, make_string("Wrong definition for [hud_movement_layers] %s", temp));

			_GetItem(layer_def, 0, tmp);
			anm->Load(tmp);
			_GetItem(layer_def, 1, tmp);
			anm->anm->Speed() = (atof(tmp) ? atof(tmp) : 1.f);
			_GetItem(layer_def, 2, tmp);
			anm->m_power = (atof(tmp) ? atof(tmp) : 1.f);
			m_movement_layers.push_back(anm);
		}
	}
}

player_hud::~player_hud()
{
	if (m_model) 
	{
		IRenderVisual* v			= m_model->dcast_RenderVisual();
		::Render->model_Delete		(v);
		m_model						= nullptr;
	}
	if (m_model_2) 
	{
		IRenderVisual* v2			= m_model_2->dcast_RenderVisual();
		::Render->model_Delete		(v2);
		m_model_2					= nullptr;
	}

	auto it = m_pool.begin();
	auto it_e = m_pool.end();
	for(;it!=it_e;++it)
	{
		attachable_hud_item* a	= *it;
		xr_delete				(a);
	}
	m_pool.clear				();

	delete_data(m_hand_motions);
	delete_data(m_script_layers);
	delete_data(m_movement_layers);
}

void player_hud::Thumb0Callback(CBoneInstance* B)
{
	player_hud* P = static_cast<player_hud*>(B->callback_param());

	Fvector& target = P->target_thumb0rot;
	Fvector& current = P->thumb0rot;

	if (!target.similar(current))
	{
		Fvector diff[2];
		diff[0] = target;
		diff[0].sub(current);
		diff[0].mul(Device.fTimeDelta / .1f);
		current.add(diff[0]);
	}
	else
		current.set(target);

	Fmatrix rotation;
	rotation.identity();
	rotation.rotateX(current.x);

	Fmatrix rotation_y;
	rotation_y.identity();
	rotation_y.rotateY(current.y);
	rotation.mulA_43(rotation_y);

	rotation_y.identity();
	rotation_y.rotateZ(current.z);
	rotation.mulA_43(rotation_y);

	B->mTransform.mulB_43(rotation);
}

void player_hud::Thumb01Callback(CBoneInstance* B)
{
	player_hud* P = static_cast<player_hud*>(B->callback_param());

	Fvector& target = P->target_thumb01rot;
	Fvector& current = P->thumb01rot;

	if (!target.similar(current))
	{
		Fvector diff[2];
		diff[0] = target;
		diff[0].sub(current);
		diff[0].mul(Device.fTimeDelta / .1f);
		current.add(diff[0]);
	}
	else
		current.set(target);

	Fmatrix rotation;
	rotation.identity();
	rotation.rotateX(current.x);

	Fmatrix rotation_y;
	rotation_y.identity();
	rotation_y.rotateY(current.y);
	rotation.mulA_43(rotation_y);

	rotation_y.identity();
	rotation_y.rotateZ(current.z);
	rotation.mulA_43(rotation_y);

	B->mTransform.mulB_43(rotation);
}

void player_hud::Thumb02Callback(CBoneInstance* B)
{
	player_hud* P = static_cast<player_hud*>(B->callback_param());

	Fvector& target = P->target_thumb02rot;
	Fvector& current = P->thumb02rot;

	if (!target.similar(current))
	{
		Fvector diff[2];
		diff[0] = target;
		diff[0].sub(current);
		diff[0].mul(Device.fTimeDelta / .1f);
		current.add(diff[0]);
	}
	else
		current.set(target);

	Fmatrix rotation;
	rotation.identity();
	rotation.rotateX(current.x);

	Fmatrix rotation_y;
	rotation_y.identity();
	rotation_y.rotateY(current.y);
	rotation.mulA_43(rotation_y);

	rotation_y.identity();
	rotation_y.rotateZ(current.z);
	rotation.mulA_43(rotation_y);

	B->mTransform.mulB_43(rotation);
}

void player_hud::load(const shared_str& player_hud_sect)
{
	if(player_hud_sect ==m_sect_name)	return;
	bool b_reload = (m_model != nullptr || m_model_2 != nullptr);
	if(m_model)
	{
		IRenderVisual* v			= m_model->dcast_RenderVisual();
		::Render->model_Delete		(v);
		m_model = nullptr;
	}

	if (m_model_2)
	{
		IRenderVisual* v2			= m_model_2->dcast_RenderVisual();
		::Render->model_Delete		(v2);
		m_model_2 = nullptr;
	}

	m_sect_name = player_hud_sect;
	const shared_str& model_name = pSettings->r_string(player_hud_sect, "visual");

	::Render->hud_loading = true;

	m_model = smart_cast<IKinematicsAnimated*>(::Render->model_Create(model_name.c_str()));
	m_model_2 = smart_cast<IKinematicsAnimated*>(::Render->model_Create(pSettings->line_exist(player_hud_sect, "visual_2") ? pSettings->r_string(player_hud_sect, "visual_2") : model_name.c_str()));

	::Render->hud_loading = false;

	u16 l_arm = m_model->dcast_PKinematics()->LL_BoneID("l_clavicle");
	u16 r_arm = m_model_2->dcast_PKinematics()->LL_BoneID("r_clavicle");

	u16 r_finger0 = m_model->dcast_PKinematics()->LL_BoneID("r_finger0");
	u16 r_finger01 = m_model->dcast_PKinematics()->LL_BoneID("r_finger01");
	u16 r_finger02 = m_model->dcast_PKinematics()->LL_BoneID("r_finger02");

	m_model->dcast_PKinematics()->LL_GetBoneInstance(r_finger0).set_callback(bctCustom, Thumb0Callback, this);
	m_model->dcast_PKinematics()->LL_GetBoneInstance(r_finger01).set_callback(bctCustom, Thumb01Callback, this);
	m_model->dcast_PKinematics()->LL_GetBoneInstance(r_finger02).set_callback(bctCustom, Thumb02Callback, this);

	// hides the unused arm meshes
	m_model->dcast_PKinematics()->LL_SetBoneVisible(l_arm, FALSE, TRUE);
	m_model_2->dcast_PKinematics()->LL_SetBoneVisible(r_arm, FALSE, TRUE);

	CInifile::Sect& _sect		= pSettings->r_section(player_hud_sect);
	CInifile::SectCIt _b		= _sect.Data.begin();
	CInifile::SectCIt _e		= _sect.Data.end();
	for(;_b!=_e;++_b)
	{
		if(strstr(_b->first.c_str(), "ancor_")==_b->first.c_str())
		{
			const shared_str& _bone	= _b->second;
			m_ancors.push_back		(m_model->dcast_PKinematics()->LL_BoneID(_bone));
		}
	}
	if(!b_reload)
	{
		m_model->PlayCycle("hand_idle_doun");
		m_model_2->PlayCycle("hand_idle_doun");
	}
	else
	{
		if(m_attached_items[1])
			m_attached_items[1]->m_parent_hud_item->on_a_hud_attach();

		if(m_attached_items[0])
			m_attached_items[0]->m_parent_hud_item->on_a_hud_attach();
	}
	m_model->dcast_PKinematics()->CalculateBones_Invalidate();
	m_model->dcast_PKinematics()->CalculateBones(TRUE);
	m_model_2->dcast_PKinematics()->CalculateBones_Invalidate();
	m_model_2->dcast_PKinematics()->CalculateBones(TRUE);
}

bool player_hud::render_item_ui_query()
{
	bool res = false;
	if(m_attached_items[0])
		res |= m_attached_items[0]->render_item_ui_query();

	if(m_attached_items[1])
		res |= m_attached_items[1]->render_item_ui_query();

	return res;
}

void player_hud::render_item_ui()
{
	if(m_attached_items[0])
		m_attached_items[0]->render_item_ui();

	if(m_attached_items[1])
		m_attached_items[1]->render_item_ui();
}

void player_hud::render_hud()
{
	bool b_r0 = ((m_attached_items[0] && m_attached_items[0]->need_renderable()) || script_anim_part == 0 || script_anim_part == 2);
	bool b_r1 = ((m_attached_items[1] && m_attached_items[1]->need_renderable()) || script_anim_part == 1 || script_anim_part == 2);

	if(!b_r0 && !b_r1)									return;

	::Render->set_Transform		(&m_transform);
	::Render->add_Visual		(m_model->dcast_RenderVisual(), true);
	::Render->set_Transform		(&m_transform_2);
	::Render->add_Visual		(m_model_2->dcast_RenderVisual(), true);
	
	if(m_attached_items[0])
		m_attached_items[0]->render();
	
	if(m_attached_items[1])
		m_attached_items[1]->render();

	if (script_anim_item_model)
	{
		::Render->set_Transform(&m_item_pos);
		::Render->add_Visual(script_anim_item_model->dcast_RenderVisual(), true);
	}
}

#include "../xrEngine/motion.h"

u32 player_hud::motion_length_script(LPCSTR section, LPCSTR anm_name, float speed)
{
	if (!pSettings->section_exist(section))
	{
		Msg("!script motion section [%s] does not exist", section);
		return 0;
	}

	player_hud_motion_container* pm = get_hand_motions(section);
	if (!pm)
		return 0;

	player_hud_motion* phm = pm->find_motion(anm_name);
	if (!phm)
	{
		Msg("!script motion [%s] not found in section [%s]", anm_name, section);
		return 0;
	}

	const CMotionDef* temp;
	return motion_length(phm->params, phm->m_animations[0].mid, temp, speed);
}

u32 player_hud::motion_length(const shared_str& anim_name, const shared_str& hud_name, const CMotionDef*& md, float speed)
{
	//float speed						= CalcMotionSpeed(anim_name);
	attachable_hud_item* pi			= create_hud_item(hud_name);
	player_hud_motion*	pm			= pi->m_hand_motions.find_motion(anim_name);
	if(!pm || !pm->m_animations.size())
		return						100; // ms TEMPORARY
	R_ASSERT2						(pm, 
		make_string	("hudItem model [%s] has no motion with alias [%s]", hud_name.c_str(), anim_name.c_str() ).c_str() 
		);
	return motion_length			(pm->params, pm->m_animations[0].mid, md, speed == 1.f ? pm->params.speed_k : speed);
}

u32 player_hud::motion_length(const motion_params& P, const MotionID& M, const CMotionDef*& md, float speed)
{
	md					= m_model->LL_GetMotionDef(M);
	VERIFY				(md);
	if (md->flags & esmStopAtEnd) 
	{
		CMotion*			motion		= m_model->LL_GetRootMotion(M);

		auto fStartFromTime = CalculateMotionStartSeconds(P.start_k, motion->GetLength());

		if (speed >= 0.0f)
			return				iFloor( 0.5f + 1000.f*(motion->GetLength() - fStartFromTime) / (md->Dequantize(md->speed) * speed) * P.stop_k );
		else
			return				iFloor( 0.5f + 1000.f*(fStartFromTime) / (md->Dequantize(md->speed) * abs(speed)) * P.stop_k );
	}
	return					0;
}

#pragma warning(push)
#pragma warning(disable: 4172)
const Fvector& player_hud::attach_rot(u8 part) const
{
	if (m_attached_items[part])
		return m_attached_items[part]->hands_attach_rot();
	else if (m_attached_items[!part])
		return m_attached_items[!part]->hands_attach_rot();

	return Fvector().set(0.f, 0.f, 0.f);
}

const Fvector& player_hud::attach_pos(u8 part) const
{
	if (m_attached_items[part])
		return m_attached_items[part]->hands_attach_pos();
	else if (m_attached_items[!part])
		return m_attached_items[!part]->hands_attach_pos();

	return Fvector().set(0.f, 0.f, 0.f);
}
#pragma warning(pop)

void player_hud::update(const Fmatrix& cam_trans)
{
	Fmatrix trans = cam_trans;
	Fmatrix trans_b = cam_trans;

	Fvector m1pos = attach_pos(0);
	Fvector m2pos = attach_pos(1);

	Fvector m1rot = attach_rot(0);
	Fvector m2rot = attach_rot(1);

	Fmatrix trans_2 = trans;

	if (m_attached_items[0])
		m_attached_items[0]->m_parent_hud_item->UpdateHudAdditional(trans);

	if (m_attached_items[1])
		m_attached_items[1]->m_parent_hud_item->UpdateHudAdditional(trans_2);
	else
		trans_2 = trans;

	// override hand offset for single hand animation
	//if (script_anim_offset_factor != 0.f)
	{
		if (script_anim_part == 2 || (script_anim_part && !m_attached_items[0] && !m_attached_items[1]))
		{
			m1pos = script_anim_offset[0];
			m2pos = script_anim_offset[0];
			m1rot = script_anim_offset[1];
			m2rot = script_anim_offset[1];

			trans = trans_b;
			trans_2 = trans_b;
		}
		else if (script_anim_offset_factor != 0.f)
		{
			Fvector& hand_pos = script_anim_part == 0 ? m1pos : m2pos;
			Fvector& hand_rot = script_anim_part == 0 ? m1rot : m2rot;

			hand_pos.lerp(script_anim_part == 0 ? m1pos : m2pos, script_anim_offset[0], script_anim_offset_factor);
			hand_rot.lerp(script_anim_part == 0 ? m1rot : m2rot, script_anim_offset[1], script_anim_offset_factor);

			if (script_anim_part == 0)
			{
				trans_b.inertion(trans, script_anim_offset_factor);
				trans = trans_b;
			}
			else
			{
				trans_b.inertion(trans_2, script_anim_offset_factor);
				trans_2 = trans_b;
			}
		}
	}

	m1rot.mul(PI / 180.f);
	m_attach_offset.setHPB(m1rot.x, m1rot.y, m1rot.z);
	m_attach_offset.translate_over(m1pos);

	m2rot.mul(PI / 180.f);
	m_attach_offset_2.setHPB(m2rot.x, m2rot.y, m2rot.z);
	m_attach_offset_2.translate_over(m2pos);

	m_transform.mul(trans, m_attach_offset);
	m_transform_2.mul(trans_2, m_attach_offset_2);
	// insert inertion here

	m_model->UpdateTracks();
	m_model->dcast_PKinematics()->CalculateBones_Invalidate();
	m_model->dcast_PKinematics()->CalculateBones(TRUE);

	m_model_2->UpdateTracks();
	m_model_2->dcast_PKinematics()->CalculateBones_Invalidate();
	m_model_2->dcast_PKinematics()->CalculateBones(TRUE);

	for (script_layer* anm : m_script_layers)
	{
		if (!anm || !anm->anm || (!anm->active && anm->blend_amount == 0.f))
			continue;

		if (anm->active)
			anm->blend_amount += Device.fTimeDelta / .4f;
		else
			anm->blend_amount -= Device.fTimeDelta / .4f;

		clamp(anm->blend_amount, 0.f, 1.f);

		if (anm->blend_amount > 0.f)
		{
			if (anm->anm->bLoop || anm->anm->anim_param().t_current < anm->anm->anim_param().max_t)
				anm->anm->Update(Device.fTimeDelta);
			else
				anm->Stop(false);
		}
		else
		{
			anm->Stop(true);
			continue;
		}

		Fmatrix blend = anm->XFORM();

		if (anm->m_part == 0 || anm->m_part == 2)
			m_transform.mulB_43(blend);

		if (anm->m_part == 1 || anm->m_part == 2)
			m_transform_2.mulB_43(blend);
	}

	bool need_blend[2];
	need_blend[0] = ((script_anim_part == 0 || script_anim_part == 2) || (m_attached_items[0] && m_attached_items[0]->m_parent_hud_item->NeedBlendAnm()));
	need_blend[1] = ((script_anim_part == 1 || script_anim_part == 2) || (m_attached_items[1] && m_attached_items[1]->m_parent_hud_item->NeedBlendAnm()));

	for (movement_layer* anm : m_movement_layers)
	{
		if (!anm || !anm->anm || (!anm->active && anm->blend_amount[0] == 0.f && anm->blend_amount[1] == 0.f))
			continue;

		if (anm->active && (need_blend[0] || need_blend[1]))
		{
			if (need_blend[0])
			{
				anm->blend_amount[0] += Device.fTimeDelta / .4f;

				if (!m_attached_items[1])
					anm->blend_amount[1] += Device.fTimeDelta / .4f;
				else if (!need_blend[1])
					anm->blend_amount[1] -= Device.fTimeDelta / .4f;
			}

			if (need_blend[1])
			{
				anm->blend_amount[1] += Device.fTimeDelta / .4f;

				if (!m_attached_items[0])
					anm->blend_amount[0] += Device.fTimeDelta / .4f;
				else if (!need_blend[0])
					anm->blend_amount[0] -= Device.fTimeDelta / .4f;
			}
		}
		else
		{
			anm->blend_amount[0] -= Device.fTimeDelta / .4f;
			anm->blend_amount[1] -= Device.fTimeDelta / .4f;
		}

		clamp(anm->blend_amount[0], 0.f, 1.f);
		clamp(anm->blend_amount[1], 0.f, 1.f);

		if (anm->blend_amount[0] == 0.f && anm->blend_amount[1] == 0.f)
		{
			anm->Stop(true);
			continue;
		}

		anm->anm->Update(Device.fTimeDelta);

		if (anm->blend_amount[0] == anm->blend_amount[1])
		{
			Fmatrix blend = anm->XFORM(0);
			m_transform.mulB_43(blend);
			m_transform_2.mulB_43(blend);
		}
		else
		{
			if (anm->blend_amount[0] > 0.f)
				m_transform.mulB_43(anm->XFORM(0));

			if (anm->blend_amount[1] > 0.f)
				m_transform_2.mulB_43(anm->XFORM(1));
		}
	}

	if (m_attached_items[0])
		m_attached_items[0]->update(true);

	if (m_attached_items[1])
		m_attached_items[1]->update(true);

	if (script_anim_item_attached && script_anim_item_model)
		update_script_item();

	// single hand offset smoothing + syncing back to other hand animation on end
	if (script_anim_part != u8(-1))
	{
		script_anim_offset_factor += Device.fTimeDelta * 2.5f;

		if (m_bStopAtEndScriptAnimIsRunning && Device.dwTimeGlobal >= script_anim_end)
			StopScriptAnim();
	}
	else
		script_anim_offset_factor -= Device.fTimeDelta * 5.f;

	clamp(script_anim_offset_factor, 0.f, 1.f);

	if (m_current_motion_def)
	{
		if (m_bStopAtEndAnimIsRunning || m_bStopAtEndScriptAnimIsRunning)
		{
			const xr_vector<motion_marks>& marks = m_current_motion_def->marks;
			if (!marks.empty())
			{
				float motion_prev_time = ((float)m_dwMotionCurrTm - (float)m_dwMotionStartTm) / 1000.0f;
				float motion_curr_time = ((float)Device.dwTimeGlobal - (float)m_dwMotionStartTm) / 1000.0f;

				xr_vector<motion_marks>::const_iterator it = marks.begin();
				xr_vector<motion_marks>::const_iterator it_e = marks.end();
				for (; it != it_e; ++it)
				{
					const motion_marks& M = (*it);
					if (M.is_empty())
						continue;

					const motion_marks::interval* Iprev = M.pick_mark(motion_prev_time);
					const motion_marks::interval* Icurr = M.pick_mark(motion_curr_time);
					if (Iprev == NULL && Icurr != NULL)
					{
						OnMotionMark(M);
					}
				}

			}

			m_dwMotionCurrTm = Device.dwTimeGlobal;
			if (m_dwMotionCurrTm > m_dwMotionEndTm)
			{
				m_current_motion_def = NULL;
				m_dwMotionStartTm = 0;
				m_dwMotionEndTm = 0;
				m_dwMotionCurrTm = 0;
				m_bStopAtEndAnimIsRunning = false;
				m_bStopAtEndScriptAnimIsRunning = false;
			}
		}
	}
}

void player_hud::OnMotionMark(const motion_marks& M)
{
	luabind::functor<bool> funct;
	if (ai().script_engine().functor("mfs_functions.on_motion_mark", funct))
		funct(*M.name);
}

void player_hud::update_script_item()
{
	Fvector ypr = item_pos[1];
	ypr.mul(PI / 180.f);
	m_attach_offset.setHPB(ypr.x, ypr.y, ypr.z);
	m_attach_offset.translate_over(item_pos[0]);

	calc_transform(m_attach_idx, m_attach_offset, m_item_pos);

	if (script_anim_item_model && script_anim_item_visible)
	{
		if (script_anim_item_model->dcast_PKinematicsAnimated())
			script_anim_item_model->dcast_PKinematicsAnimated()->UpdateTracks();

		script_anim_item_model->CalculateBones_Invalidate();
		script_anim_item_model->CalculateBones(TRUE);
	}
}

void player_hud::SetScriptItemVisible(bool visible)
{
	if (script_anim_item_model)
	{
		u16 root_id = script_anim_item_model->LL_GetBoneRoot();
		script_anim_item_model->LL_SetBoneVisible(root_id, visible, TRUE);
	}
}

u32 player_hud::script_anim_play(u8 hand, LPCSTR section, LPCSTR anm_name, bool bMixIn, float speed, LPCSTR attach_visual)
{
	if (!pSettings->section_exist(section))
	{
		Msg("!script motion section [%s] does not exist", section);
		m_bStopAtEndScriptAnimIsRunning = true;
		script_anim_end = Device.dwTimeGlobal;
		return 0;
	}
	
	xr_string pos = "hands_position";
	xr_string rot = "hands_orientation";

	if (UI().is_widescreen() && pSettings->line_exist(section, "hands_position_16x9"))
		pos.append("_16x9");

	if (UI().is_widescreen() && pSettings->line_exist(section, "hands_orientation_16x9"))
		rot.append("_16x9");

	Fvector def = { 0.f, 0.f, 0.f };
	Fvector offs = READ_IF_EXISTS(pSettings, r_fvector3, section, pos.c_str(), def);
	Fvector rrot = READ_IF_EXISTS(pSettings, r_fvector3, section, rot.c_str(), def);

	if (pSettings->line_exist(section, "item_visual") && !attach_visual)
		attach_visual = pSettings->r_string(section, "item_visual");

	if (attach_visual)
	{
		::Render->hud_loading = true;
		script_anim_item_model = ::Render->model_Create(attach_visual)->dcast_PKinematics();
		::Render->hud_loading = false;

		item_pos[0] = READ_IF_EXISTS(pSettings, r_fvector3, section, "item_position", def);
		item_pos[1] = READ_IF_EXISTS(pSettings, r_fvector3, section, "item_orientation", def);
		script_anim_item_attached = READ_IF_EXISTS(pSettings, r_bool, section, "item_attached", true);
		script_anim_item_visible = READ_IF_EXISTS(pSettings, r_bool, section, "item_visible", true);

		if (script_anim_item_model)
		{
			u16 root_id = script_anim_item_model->LL_GetBoneRoot();
			script_anim_item_model->LL_SetBoneVisible(root_id, script_anim_item_visible, TRUE);
		}

		m_attach_idx = READ_IF_EXISTS(pSettings, r_u8, section, "attach_place_idx", 0);

		if (!script_anim_item_attached)
		{
			Fmatrix attach_offs;
			Fvector ypr = item_pos[1];
			ypr.mul(PI / 180.f);
			attach_offs.setHPB(ypr.x, ypr.y, ypr.z);
			attach_offs.translate_over(item_pos[0]);
			m_item_pos = attach_offs;
		}
	}

	script_anim_offset[0] = offs;
	script_anim_offset[1] = rrot;
	script_anim_part = hand;
	player_hud_motion_container* pm = get_hand_motions(section);
	player_hud_motion* phm = pm->find_motion(anm_name);

	if (!phm)
	{
		Msg("!script motion [%s] not found in section [%s]", anm_name, section);
		m_bStopAtEndScriptAnimIsRunning = true;
		script_anim_end = Device.dwTimeGlobal;
		return 0;
	}

	const motion_descr& M = phm->m_animations[Random.randI(phm->m_animations.size())];

	if (script_anim_item_model && script_anim_item_model->dcast_PKinematicsAnimated())
	{
		shared_str item_anm_name;
		if (phm->m_base_name != phm->m_additional_name)
			item_anm_name = phm->m_additional_name;
		else
			item_anm_name = M.name;

		MotionID M2 = script_anim_item_model->dcast_PKinematicsAnimated()->ID_Cycle_Safe(item_anm_name);
		if (!M2.valid())
			M2 = script_anim_item_model->dcast_PKinematicsAnimated()->ID_Cycle_Safe("idle");

		R_ASSERT3(M2.valid(), "model %s has no motion [idle] ", pSettings->r_string(m_sect_name, "item_visual"));

		u16 root_id = script_anim_item_model->LL_GetBoneRoot();
		CBoneInstance& root_binst = script_anim_item_model->LL_GetBoneInstance(root_id);
		root_binst.set_callback_overwrite(TRUE);
		root_binst.mTransform.identity();

		u16 pc = script_anim_item_model->dcast_PKinematicsAnimated()->partitions().count();
		for (u16 pid = 0; pid < pc; ++pid)
		{
			CBlend* B = script_anim_item_model->dcast_PKinematicsAnimated()->PlayCycle(pid, M2, bMixIn);
			R_ASSERT(B);
			B->speed *= speed;
			B->timeCurrent = CalculateMotionStartSeconds(phm->params.start_k, B->timeTotal);
		}

		script_anim_item_model->CalculateBones_Invalidate();
	}

	if (hand == 0) // right hand
	{
		CBlend* B = m_model->PlayCycle(0, M.mid, bMixIn);
		B->speed *= speed;
		B->timeCurrent = CalculateMotionStartSeconds(phm->params.start_k, B->timeTotal);
		B = m_model->PlayCycle(2, M.mid, bMixIn);
		B->speed *= speed;
		B->timeCurrent = CalculateMotionStartSeconds(phm->params.start_k, B->timeTotal);
	}
	else if (hand == 1) // left hand
	{
		CBlend* B = m_model_2->PlayCycle(0, M.mid, bMixIn);
		B->speed *= speed;
		B->timeCurrent = CalculateMotionStartSeconds(phm->params.start_k, B->timeTotal);
		B = m_model_2->PlayCycle(1, M.mid, bMixIn);
		B->speed *= speed;
		B->timeCurrent = CalculateMotionStartSeconds(phm->params.start_k, B->timeTotal);
	}
	else if (hand == 2) // both hands
	{
		CBlend* B = m_model->PlayCycle(0, M.mid, bMixIn);
		B->speed *= speed;
		B->timeCurrent = CalculateMotionStartSeconds(phm->params.start_k, B->timeTotal);
		B = m_model_2->PlayCycle(0, M.mid, bMixIn);
		B->speed *= speed;
		B->timeCurrent = CalculateMotionStartSeconds(phm->params.start_k, B->timeTotal);
		B = m_model->PlayCycle(2, M.mid, bMixIn);
		B->speed *= speed;
		B->timeCurrent = CalculateMotionStartSeconds(phm->params.start_k, B->timeTotal);
		B = m_model_2->PlayCycle(1, M.mid, bMixIn);
		B->speed *= speed;
		B->timeCurrent = CalculateMotionStartSeconds(phm->params.start_k, B->timeTotal);
	}

	const CMotionDef* md;
	u32 length = motion_length(phm->params, M.mid, md, speed);

	if (length > 0)
	{
		m_bStopAtEndAnimIsRunning	= true;
		m_bStopAtEndScriptAnimIsRunning = true;
		script_anim_end				= Device.dwTimeGlobal + length;
		m_dwMotionStartTm			= Device.dwTimeGlobal;
		m_dwMotionCurrTm			= m_dwMotionStartTm;
		m_dwMotionEndTm				= m_dwMotionStartTm + length;
		m_current_motion_def		= md;
	}
	else
	{
		m_bStopAtEndAnimIsRunning	= false;
		m_bStopAtEndScriptAnimIsRunning = false;
	}

	updateMovementLayerState();

	return length;
}

void player_hud::updateMovementLayerState()
{
	if (!m_movement_layers.size())
		return;

	CActor* pActor = Actor();

	if (!pActor)
		return;

	for (movement_layer* anm : m_movement_layers)
	{
		anm->Stop(false);
	}

	bool need_blend = (script_anim_part != u8(-1) || (m_attached_items[0] && m_attached_items[0]->m_parent_hud_item->NeedBlendAnm()) || (m_attached_items[1] && m_attached_items[1]->m_parent_hud_item->NeedBlendAnm()));

	if (pActor->AnyMove() && need_blend)
	{
		CEntity::SEntityState state;
		pActor->g_State(state);

		CWeapon* wep = nullptr;

		if (m_attached_items[0] && m_attached_items[0]->m_parent_hud_item->object().cast_weapon())
			wep = m_attached_items[0]->m_parent_hud_item->object().cast_weapon();

		if (wep && wep->IsZoomed())
			state.bCrouch ? m_movement_layers[eAimCrouch]->Play() : m_movement_layers[eAimWalk]->Play();
		else if (state.bCrouch)
			m_movement_layers[eCrouch]->Play();
		else if (state.bSprint)
			m_movement_layers[eSprint]->Play();
		else if (!isActorAccelerated(pActor->MovingState(), false))
			m_movement_layers[eWalk]->Play();
		else
			m_movement_layers[eRun]->Play();
	}
}

void player_hud::PlayBlendAnm(LPCSTR name, u8 part, float speed, float power, bool bLooped, bool no_restart)
{
	if (IsGameTypeSingle() && m_attached_items[0])
	{
		CPhysicItem& parent_object = m_attached_items[0]->m_parent_hud_item->object();

		if (parent_object.H_Parent() == Level().CurrentControlEntity())
		{
			CActor* current_actor = static_cast_checked<CActor*>(Level().CurrentControlEntity());
			VERIFY(current_actor);

			LPCSTR fixed_blend_name = name;

			if (strstr(fixed_blend_name, "blend\\") == fixed_blend_name)
				fixed_blend_name += 6;

			string_path ce_path;
			string_path anm_name;
			strconcat(sizeof(anm_name), anm_name, "camera_effects" "\\", "cam_eff_blend" "\\", fixed_blend_name);

			if (FS.exist(ce_path, "$game_anims$", anm_name))
			{
				CEffectorCam* ec = current_actor->Cameras().GetCamEffector(eCEWeaponAction);

				if (ec)
					current_actor->Cameras().RemoveCamEffector(eCEWeaponAction);

				CAnimatorCamEffector* e = xr_new<CAnimatorCamEffector>();
				e->SetType(eCEWeaponAction);
				e->SetHudAffect(false);
				e->SetCyclic(false);
				e->Start(anm_name);
				current_actor->Cameras().AddCamEffector(e);
			}
		}
	}

	for (script_layer* anm : m_script_layers)
	{
		if (!xr_strcmp(anm->m_name, name))
		{
			if (!no_restart)
			{
				anm->anm->Stop();
				anm->blend_amount = 0.f;
				anm->blend.identity();
			}

			if (!anm->anm->IsPlaying())
				anm->anm->Play(bLooped);

			anm->anm->bLoop = bLooped;
			anm->m_part = part;
			anm->anm->Speed() = speed;
			anm->m_power = power;
			anm->active = true;
			return;
		}
	}

	script_layer* anm = xr_new<script_layer>(name, part, speed, power, bLooped);
	m_script_layers.push_back(anm);
}

void player_hud::StopBlendAnm(LPCSTR name, bool bForce)
{
	for (script_layer* anm : m_script_layers)
	{
		if (!xr_strcmp(anm->m_name, name))
		{
			anm->Stop(bForce);
			return;
		}
	}
}

void player_hud::StopAllBlendAnms(bool bForce)
{
	for (script_layer* anm : m_script_layers)
	{
		anm->Stop(bForce);
	}
}

float player_hud::SetBlendAnmTime(LPCSTR name, float time)
{
	for (script_layer* anm : m_script_layers)
	{
		if (!xr_strcmp(anm->m_name, name))
		{
			if (!anm->anm->IsPlaying())
				return 0;

			float speed = (anm->anm->anim_param().max_t - anm->anm->anim_param().t_current) / time;
			anm->anm->Speed() = speed;
			return speed;
		}
	}

	return 0;
}

bool player_hud::IsBlendAnmActive(LPCSTR name)
{
	for (script_layer* anm : m_script_layers)
	{
		if (!xr_strcmp(anm->m_name, name))
		{
			if (anm->active)
				return true;
		}
	}
	return false;
}

void player_hud::StopScriptAnim()
{
	u8 part = script_anim_part;
	script_anim_part = u8(-1);
	script_anim_item_model = nullptr;

	updateMovementLayerState();

	if (part != 2 && !m_attached_items[part])
		re_sync_anim(part + 1);
	else
		OnMovementChanged((ACTOR_DEFS::EMoveCommand)0);
}

player_hud_motion_container* player_hud::get_hand_motions(LPCSTR section)
{
	xr_vector<hand_motions*>::iterator it = m_hand_motions.begin();
	xr_vector<hand_motions*>::iterator it_e = m_hand_motions.end();
	for (; it != it_e; it++)
	{
		if (!xr_strcmp((*it)->section, section))
			return &(*it)->pm;
	}

	hand_motions* res = xr_new<hand_motions>();
	res->section = section;
	res->pm.load(m_model, section);
	m_hand_motions.push_back(res);

	return &res->pm;
}

u32 player_hud::anim_play(u16 part, const motion_params& P, const MotionID& M, BOOL bMixIn, const CMotionDef*& md, float speed, u16 override_part)
{
	u16 part_id = u16(-1);
	if (attached_item(0) && attached_item(1))
		part_id = m_model->partitions().part_id((part == 0) ? "right_hand" : "left_hand");

	if (override_part != u16(-1))
		part_id = override_part;
	else if (script_anim_part != u8(-1))
	{
		if (script_anim_part != 2)
			part_id = script_anim_part == 0 ? 1 : 0;
		else
			return 0;
	}

	if (part_id == u16(-1))
	{
		for (u8 pid = 0; pid < 3; pid++)
		{
			if (pid == 0 || pid == 2)
			{
				CBlend* B = m_model->PlayCycle(pid, M, bMixIn);
				R_ASSERT(B);
				B->speed *= speed;
				B->timeCurrent = CalculateMotionStartSeconds(P.start_k, B->timeTotal);
			}
			if (pid == 0 || pid == 1)
			{
				CBlend* B = m_model_2->PlayCycle(pid, M, bMixIn);
				R_ASSERT(B);
				B->speed *= speed;
				B->timeCurrent = CalculateMotionStartSeconds(P.start_k, B->timeTotal);
			}
		}

		m_model->dcast_PKinematics()->CalculateBones_Invalidate();
		m_model_2->dcast_PKinematics()->CalculateBones_Invalidate();
	}
	else if (part_id == 0 || part_id == 2)
	{
		for (u8 pid = 0; pid < 3; pid++)
		{
			if (pid != 1)
			{
				CBlend* B = m_model->PlayCycle(pid, M, bMixIn);
				R_ASSERT(B);
				B->speed *= speed;
				B->timeCurrent = CalculateMotionStartSeconds(P.start_k, B->timeTotal);
			}
		}

		m_model->dcast_PKinematics()->CalculateBones_Invalidate();
	}
	else if (part_id == 1)
	{
		for (u8 pid = 0; pid < 3; pid++)
		{
			if (pid != 2)
			{
				CBlend* B = m_model_2->PlayCycle(pid, M, bMixIn);
				R_ASSERT(B);
				B->speed *= speed;
				B->timeCurrent = CalculateMotionStartSeconds(P.start_k, B->timeTotal);
			}
		}

		m_model_2->dcast_PKinematics()->CalculateBones_Invalidate();
	}
	
	u32 length = motion_length(P, M, md, speed);

	if (length > 0)
	{
		m_bStopAtEndAnimIsRunning	= true;
		script_anim_end				= Device.dwTimeGlobal + length;
		m_dwMotionStartTm			= Device.dwTimeGlobal;
		m_dwMotionCurrTm			= m_dwMotionStartTm;
		m_dwMotionEndTm				= m_dwMotionStartTm + length;
		m_current_motion_def		= md;
	}
	else
		m_bStopAtEndAnimIsRunning	= false;

	return length;
}

attachable_hud_item* player_hud::create_hud_item(const shared_str& sect)
{
	xr_vector<attachable_hud_item*>::iterator it = m_pool.begin();
	xr_vector<attachable_hud_item*>::iterator it_e = m_pool.end();
	for(;it!=it_e;++it)
	{
		attachable_hud_item* itm = *it;
		if(itm->m_sect_name==sect)
			return itm;
	}
	attachable_hud_item* res	= xr_new<attachable_hud_item>(this);
	res->load					(sect);
	res->m_hand_motions.load	(m_model, sect);
	res->m_hand_motions.load	(m_model_2, sect);		// cari0us - верно ли тут?
	m_pool.push_back			(res);

	return	res;
}

bool player_hud::allow_activation(CHudItem* item)
{
	if(m_attached_items[1])
		return m_attached_items[1]->m_parent_hud_item->CheckCompatibility(item);
	else
		return true;
}

void player_hud::attach_item(CHudItem* item)
{
	attachable_hud_item* pi			= create_hud_item(item->HudSection());
	int item_idx					= pi->m_attach_place_idx;
	
	if (m_attached_items[item_idx] != pi || pi->m_parent_hud_item != item)
	{
		if(m_attached_items[item_idx])
			m_attached_items[item_idx]->m_parent_hud_item->on_b_hud_detach();

		m_attached_items[item_idx]						= pi;
		pi->m_parent_hud_item							= item;

		if(item_idx==0 && m_attached_items[1])
			m_attached_items[1]->m_parent_hud_item->CheckCompatibility(item);

		item->on_a_hud_attach();

		updateMovementLayerState();
	}
	pi->m_parent_hud_item							= item;
}

//sync anim of other part to selected part (1 = sync to left hand anim; 2 = sync to right hand anim)
void player_hud::re_sync_anim(u8 part)
{
	u32 bc = part == 1 ? m_model_2->LL_PartBlendsCount(part) : m_model->LL_PartBlendsCount(part);
	for (u32 bidx = 0; bidx < bc; ++bidx)
	{
		CBlend* BR = part == 1 ? m_model_2->LL_PartBlend(part, bidx) : m_model->LL_PartBlend(part, bidx);
		if (!BR)
			continue;

		MotionID M = BR->motionID;

		u16 pc = m_model->partitions().count(); //same on both armatures
		for (u16 pid = 0; pid < pc; ++pid)
		{
			if (pid == 0)
			{
				CBlend* B = m_model->PlayCycle(0, M, TRUE);
				B->timeCurrent = BR->timeCurrent;
				B->speed = BR->speed;
				B = m_model_2->PlayCycle(0, M, TRUE);
				B->timeCurrent = BR->timeCurrent;
				B->speed = BR->speed;
			}
			else if (pid != part)
			{
				CBlend* B = part == 1 ? m_model->PlayCycle(pid, M, TRUE) : m_model_2->PlayCycle(pid, M, TRUE);
				B->timeCurrent = BR->timeCurrent;
				B->speed = BR->speed;
			}
		}
	}
}

void player_hud::detach_item_idx(u16 idx)
{
	if (!m_attached_items[idx])
		return;

	m_attached_items[idx]->m_parent_hud_item->on_b_hud_detach();

	m_attached_items[idx]->m_parent_hud_item		= nullptr;
	m_attached_items[idx]							= nullptr;

	if (idx == 1)
	{
		if (m_attached_items[0])
			re_sync_anim(2);
		else
		{
			m_model_2->PlayCycle("hand_idle_doun");
		}
	}
	else if (idx == 0)
	{
		if (m_attached_items[1])
		{
			//fix for a rare case where the right hand stays visible on screen after detaching the right hand's attached item
			player_hud_motion* pm = m_attached_items[1]->m_hand_motions.find_motion("anm_idle");
			if (pm) 
			{
				const motion_descr& M = pm->m_animations[0];
				m_model->PlayCycle(0, M.mid, false);
				m_model->PlayCycle(2, M.mid, false);
			}
		}
		else
		{
			m_model->PlayCycle("hand_idle_doun");
		}
	}

	if (!m_attached_items[0] && !m_attached_items[1])
	{
		m_model->PlayCycle("hand_idle_doun");
		m_model_2->PlayCycle("hand_idle_doun");
	}
}

void player_hud::detach_item(CHudItem* item)
{
	if(!item->HudItemData())
		return;

	u16 item_idx						= item->HudItemData()->m_attach_place_idx;

	if( m_attached_items[item_idx]==item->HudItemData() )
	{
		detach_item_idx	(item_idx);
	}
}

void player_hud::calc_transform(u16 attach_slot_idx, const Fmatrix& offset, Fmatrix& result)
{
	IKinematics* kin = (attach_slot_idx == 0) ? m_model->dcast_PKinematics() : m_model_2->dcast_PKinematics();
	Fmatrix ancor_m = kin->LL_GetTransform(m_ancors[attach_slot_idx]);
	result.mul((attach_slot_idx == 0) ? m_transform : m_transform_2, ancor_m);
	result.mulB_43(offset);
}

void player_hud::OnMovementChanged(ACTOR_DEFS::EMoveCommand cmd)
{
	if(cmd==0)
	{
		if(m_attached_items[0])
		{
			if(m_attached_items[0]->m_parent_hud_item->GetState()==CHUDState::eIdle)
				m_attached_items[0]->m_parent_hud_item->PlayAnimIdle();
		}
		if(m_attached_items[1])
		{
			if(m_attached_items[1]->m_parent_hud_item->GetState()==CHUDState::eIdle)
				m_attached_items[1]->m_parent_hud_item->PlayAnimIdle();
		}
	}else
	{
		if(m_attached_items[0])
			m_attached_items[0]->m_parent_hud_item->OnMovementChanged(cmd);

		if(m_attached_items[1])
			m_attached_items[1]->m_parent_hud_item->OnMovementChanged(cmd);
	}
	updateMovementLayerState();
}

bool player_hud::allow_script_anim()
{
	if (m_attached_items[0] && (m_attached_items[0]->m_parent_hud_item->IsPending() || m_attached_items[0]->m_parent_hud_item->GetState() == CHudItem::EHudStates::eBore))
		return false;
	else if (m_attached_items[1] && (m_attached_items[1]->m_parent_hud_item->IsPending() || m_attached_items[1]->m_parent_hud_item->GetState() == CHudItem::EHudStates::eBore))
		return false;
	else if (script_anim_part != u8(-1))
		return false;

	return true;
}