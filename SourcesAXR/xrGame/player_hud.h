#pragma once
#include "firedeps.h"
#include "../xrEngine/ObjectAnimator.h"
#include "../Include/xrRender/Kinematics.h"
#include "../Include/xrRender/KinematicsAnimated.h"
#include "actor_defs.h"
#include "Weapon.h"

class player_hud;
class CHudItem;
class CMotionDef;

struct motion_descr
{
	MotionID		mid;
	shared_str		name;
};

struct player_hud_motion
{
	shared_str				m_alias_name;
	shared_str				m_base_name;
	shared_str				m_additional_name;
	float					m_anim_speed;
	xr_vector<motion_descr>	m_animations;
};

struct player_hud_motion_container
{
	xr_vector<player_hud_motion>	m_anims;
	player_hud_motion*				find_motion(const shared_str& name);
	void		load				(IKinematicsAnimated* model, const shared_str& sect);
};

struct hand_motions
{
	LPCSTR section;
	player_hud_motion_container pm;
};

struct item_models
{
	LPCSTR name;
	IKinematicsAnimated* model;
};

enum eMovementLayers
{
	eAimWalk = 0,
	eAimCrouch,
	eCrouch,
	eWalk,
	eRun,
	eSprint,
	move_anms_end
};

struct movement_layer
{
	CObjectAnimator* anm;
	float blend_amount[2];
	bool active;
	float m_power;
	Fmatrix blend;
	u8 m_part;

	movement_layer()
	{
		blend.identity();
		anm = xr_new<CObjectAnimator>();
		blend_amount[0] = 0.f;
		blend_amount[1] = 0.f;
		active = false;
		m_power = 1.f;
	}

	void Load(LPCSTR name)
	{
		if (xr_strcmp(name, anm->Name()))
			anm->Load(name);
	}

	void Play(bool bLoop = true)
	{
		if (!anm->Name())
			return;

		if (IsPlaying())
		{
			active = true;
			return;
		}

		anm->Play(bLoop);
		active = true;
	}

	bool IsPlaying()
	{
		return anm->IsPlaying();
	}

	void Stop(bool bForce)
	{
		if (bForce)
		{
			anm->Stop();
			blend_amount[0] = 0.f;
			blend_amount[1] = 0.f;
			blend.identity();
		}

		active = false;
	}

	const Fmatrix& XFORM(u8 part)
	{
		blend.set(anm->XFORM());
		blend.mul(blend_amount[part] * m_power);
		blend.m[0][0] = 1.f;
		blend.m[1][1] = 1.f;
		blend.m[2][2] = 1.f;

		return blend;
	}
};

struct script_layer
{
	LPCSTR m_name;
	CObjectAnimator* anm;
	float blend_amount;
	float m_power;
	bool active;
	Fmatrix blend;
	u8 m_part;

	script_layer(LPCSTR name, u8 part, float speed = 1.f, float power = 1.f, bool looped = true)
	{
		m_name = name;
		m_part = part;
		m_power = power;
		blend.identity();
		anm = xr_new<CObjectAnimator>();
		anm->Load(name);
		anm->Play(looped);
		anm->Speed() = speed;
		blend_amount = 0.f;
		active = true;
	}

	bool IsPlaying()
	{
		return anm->IsPlaying();
	}

	void Stop(bool bForce)
	{
		if (bForce)
		{
			anm->Stop();
			blend_amount = 0.f;
			blend.identity();
		}

		active = false;
	}

	const Fmatrix& XFORM()
	{
		blend.set(anm->XFORM());
		blend.mul(blend_amount * m_power);
		blend.m[0][0] = 1.f;
		blend.m[1][1] = 1.f;
		blend.m[2][2] = 1.f;

		return blend;
	}
};

struct hud_item_measures
{
	enum{e_fire_point=(1<<0), e_fire_point2=(1<<1), e_shell_point=(1<<2), e_16x9_mode_now=(1<<3)};
	void							merge_measures_params();
	Flags8							m_prop_flags;

	Fvector							m_item_attach[2];//pos,rot
	Fvector							m_collision_offset[2];//pos,rot

	Fvector							m_hands_offset[2][4];//pos,rot/ normal,aim,GL,alt aim

	u16								m_fire_bone;
	Fvector							m_fire_point_offset;
	u16								m_fire_bone2;
	Fvector							m_fire_point2_offset;
	u16								m_shell_bone;
	Fvector							m_shell_point_offset;

	Fvector							m_hands_attach[2];//pos,rot

	void load						(const shared_str& sect_name, IKinematics* K);
	bool							bReloadShooting; //--#SM+#--

	struct inertion_params
	{
		float m_pitch_offset_r;
		float m_pitch_offset_n;
		float m_pitch_offset_d;
		float m_pitch_low_limit;
		float m_tendto_speed;
		float m_tendto_speed_aim;
		float m_tendto_ret_speed;
		float m_tendto_ret_speed_aim;

		float m_min_angle;
		float m_min_angle_aim;

		Fvector4 m_offset_LRUD;
		Fvector4 m_offset_LRUD_aim;
	};
	inertion_params m_inertion_params; //--#SM+#--

	struct shooting_params
	{
		Fvector4 m_shot_max_offset_LRUD;     // √раницы сдвига в бок при выстреле от бедра (-x, +x, +y, -y) 
		Fvector4 m_shot_max_offset_LRUD_aim; // √раницы сдвига в бок при выстреле в зуме (-x, +x, +y, -y) 
		Fvector2 m_shot_max_rot_UD;          // √раницы поворота по вертикали при выстреле от бедра (при смещении вверх \ вниз) 
		Fvector2 m_shot_max_rot_UD_aim;      // √раницы поворота по вертикали при выстреле в зуме (при смещении вверх \ вниз) 
		float m_shot_offset_BACKW;           // –ассто€ние сдвига назад при выстреле от бедра [>= 0.0f]
		float m_shot_offset_BACKW_aim;       // –ассто€ние сдвига назад при выстреле в зуме [>= 0.0f]
		Fvector2 m_shot_offsets_strafe;      // ‘актор изменени€ наклона (стрейфа) ствола при выстреле (мин.\макс. от бедра) vec2[0.f - 1.f]
		Fvector2 m_shot_offsets_strafe_aim;  // ‘актор изменени€ наклона (стрейфа) ствола при выстреле (мин.\макс. в зуме) vec2[0.f - 1.f]
		Fvector2 m_shot_diff_per_shot;       // ‘актор того, насколько большой может быть разница между текущей и новой позицией на каждый выстрел (от бедра \ в зуме) [0.f - 1.f]
		Fvector2 m_shot_power_per_shot;      // ‘актор того, насколько сильнее мы приближаемс€ к границам сдвига и наклона с каждым выстрелом (от бедра \ в зуме) [0.f - 1.f]
		Fvector2 m_ret_time;                 // ћаксимально возможное врем€ стабилизации ствола в исходное положение после анимации стрельбы (от бедра / в зуме) [секунд][>= 0.0f] - не вли€ет на стрейф
		Fvector2 m_ret_time_fire;            // ћаксимально возможное врем€ стабилизации ствола в исходное положение во врем€ анимации стрельбы (от бедра / в зуме) [секунд][>= 0.0f] - не вли€ет на стрейф
		float m_ret_time_backw_koef;         //  оэфицент времени стабилизации дл€ сдвига назад [>= 0.0f]
	};
	shooting_params m_shooting_params; //--#SM+#--
};

struct attachable_hud_item
{
	player_hud*						m_parent;
	CHudItem*						m_parent_hud_item;
	shared_str						m_sect_name;
	IKinematics*					m_model;
	u16								m_attach_place_idx;
	hud_item_measures				m_measures;

	//runtime positioning
	Fmatrix							m_attach_offset;
	Fmatrix							m_item_transform;

	player_hud_motion_container		m_hand_motions;
			
			attachable_hud_item		(player_hud* pparent):m_parent(pparent),m_upd_firedeps_frame(u32(-1)),m_parent_hud_item(NULL){}
			~attachable_hud_item	();
	void load						(const shared_str& sect_name);
	void update						(bool bForce);
	void update_hud_additional		(Fmatrix& trans);
	void setup_firedeps				(firedeps& fd);
	void render						();	
	void render_item_ui				();
	bool render_item_ui_query		();
	bool need_renderable			();
	void set_bone_visible			(const shared_str& bone_name, BOOL bVisibility, BOOL bSilent=FALSE);
	void debug_draw_firedeps		();

	//hands bind position
	Fvector&						hands_attach_pos();
	Fvector&						hands_attach_rot();

	//hands runtime offset
	Fvector&						hands_offset_pos();
	Fvector&						hands_offset_rot();

//props
	u32								m_upd_firedeps_frame;
	void		tune				(Ivector values);
	u32			anim_play			(const shared_str& anim_name, BOOL bMixIn, const CMotionDef*& md, u8& rnd, float speed = 1.f);

};

class player_hud
{
public: 
					player_hud			();
					~player_hud			();
	void			load				(const shared_str& model_name);
	void			load_default		(){load("actor_hud_05");};
	void			update				(const Fmatrix& trans);
	void			StopScriptAnim		();
	void			updateMovementLayerState();
	void			PlayBlendAnm		(LPCSTR name, u8 part = 0, float speed = 1.f, float power = 1.f, bool bLooped = true, bool no_restart = false);
	void			StopBlendAnm		(LPCSTR name, bool bForce = false);
	void			StopAllBlendAnms	(bool bForce);
	float			SetBlendAnmTime		(LPCSTR name, float time);
	bool			IsBlendAnmActive	(LPCSTR name);

	void			render_hud			();	
	void			render_item_ui		();
	bool			render_item_ui_query();
	u32				anim_play			(u16 part, const MotionID& M, BOOL bMixIn, const CMotionDef*& md, float speed, u16 override_part = u16(-1));
	u32				script_anim_play	(u8 hand, LPCSTR itm_name, LPCSTR anm_name, bool bMixIn = true, float speed = 1.f, LPCSTR attach_visual = nullptr);
	const shared_str& section_name		() const {return m_sect_name;}

	attachable_hud_item* create_hud_item(const shared_str& sect);

	void			attach_item			(CHudItem* item);
	void			re_sync_anim		(u8 part);
	bool			allow_activation	(CHudItem* item);
	attachable_hud_item* attached_item	(u16 item_idx)	{return m_attached_items[item_idx];};
	void			detach_item_idx		(u16 idx);
	void			detach_item			(CHudItem* item);
	void			detach_all_items	(){m_attached_items[0]=NULL; m_attached_items[1]=NULL;};
	bool			allow_script_anim	();

	void			calc_transform		(u16 attach_slot_idx, const Fmatrix& offset, Fmatrix& result);
	void			tune				(Ivector values);
	void			SaveCfg				(const int idx) const;
	void			SaveAttachesCfg		(LPCSTR parent_section, CWeapon* parent_wpn) const;
	u32				motion_length		(const MotionID& M, const CMotionDef*& md, float speed);
	u32				motion_length		(const shared_str& anim_name, const shared_str& hud_name, const CMotionDef*& md);
	u32				motion_length_script(LPCSTR section, LPCSTR anm_name, float speed);
	void			OnMovementChanged	(ACTOR_DEFS::EMoveCommand cmd);
	void			OnMotionMark		(const motion_marks&);
	void			SetScriptItemVisible(bool visible);
private:
	void			update_inertion		(Fmatrix& trans);
	void			update_additional	(Fmatrix& trans);
public:
	bool			inertion_allowed	();
private:
	const Fvector&	attach_rot			(u8 part) const;
	const Fvector&	attach_pos			(u8 part) const;

	shared_str							m_sect_name;

	Fmatrix								m_attach_offset;
	Fmatrix								m_attach_offset_2;

	Fmatrix								m_transform;
	Fmatrix								m_transform_2;
	IKinematicsAnimated*				m_model;
	IKinematicsAnimated*				m_model_2;
	xr_vector<u16>						m_ancors;
	attachable_hud_item*				m_attached_items[2];
	xr_vector<attachable_hud_item*>		m_pool;

	u8									script_anim_part;
	Fvector								script_anim_offset[2];
	u32									script_anim_end;
	float								script_anim_offset_factor;
	const CMotionDef*					m_current_motion_def;
	u32									m_dwMotionCurrTm;
	u32									m_dwMotionStartTm;
	u32									m_dwMotionEndTm;
	bool								m_bStopAtEndAnimIsRunning;
	bool								m_bStopAtEndScriptAnimIsRunning;
	bool								script_anim_item_attached;
	bool								script_anim_item_visible;
	IKinematics*						script_anim_item_model;
	Fvector								item_pos[2];
	Fmatrix								m_item_pos;
	u8									m_attach_idx;

	//Movement animation layers: 0 = aim_walk, 1 = aim_crouch, 2 = crouch, 3 = walk, 4 = run, 5 = sprint
	xr_vector<movement_layer*>			m_movement_layers;
	xr_vector<script_layer*>			m_script_layers;

	xr_vector<hand_motions*>			m_hand_motions;
	player_hud_motion_container*		get_hand_motions(LPCSTR section);

	void update_script_item();

	static void _BCL Thumb0Callback(CBoneInstance* B);
	static void _BCL Thumb01Callback(CBoneInstance* B);
	static void _BCL Thumb02Callback(CBoneInstance* B);

public:
	Fvector target_thumb0rot, target_thumb01rot, target_thumb02rot;
	Fvector thumb0rot, thumb01rot, thumb02rot;

	void reset_thumb(bool bForce)
	{
		if (bForce)
		{
			thumb0rot.set(0.f, 0.f, 0.f);
			thumb01rot.set(0.f, 0.f, 0.f);
			thumb02rot.set(0.f, 0.f, 0.f);
		}

		target_thumb0rot.set(0.f, 0.f, 0.f);
		target_thumb01rot.set(0.f, 0.f, 0.f);
		target_thumb02rot.set(0.f, 0.f, 0.f);
	}
};

extern player_hud* g_player_hud;