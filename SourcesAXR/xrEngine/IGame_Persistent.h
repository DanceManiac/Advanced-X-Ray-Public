#ifndef IGame_PersistentH
#define IGame_PersistentH
#pragma once

#include "..\xrServerEntities\gametype_chooser.h"
#ifndef _EDITOR
#include "Environment.h"
#include "IGame_ObjectPool.h"
#endif

#include "ShadersExternalData.h" //--#SM+#--

class IRenderVisual;
class IMainMenu;
class ENGINE_API CPS_Instance;
//-----------------------------------------------------------------------------------------------------------
class ENGINE_API IGame_Persistent	: 
#ifndef _EDITOR
	public DLL_Pure,
#endif
	public pureAppStart, 
	public pureAppEnd,
	public pureAppActivate, 
	public pureAppDeactivate,
	public pureFrame
{
public:
	union params {
		struct {
			string256	m_game_or_spawn;
			string256	m_game_type;
			string256	m_alife;
			string256	m_new_or_load;

			union
			{
				EGameIDs	m_e_game_type;
				u32			m_e_game_type_for_soc;
			};
		};
		string256		m_params[4];
						params		()	{	reset();	}
		void			reset		()
		{
			for (int i=0; i<4; ++i)
				xr_strcpy	(m_params[i],"");
		}
		void						parse_cmd_line		(LPCSTR cmd_line)
		{
			reset					();
			int						n = _min(4,_GetItemCount(cmd_line,'/'));
			for (int i=0; i<n; ++i) {
				_GetItem			(cmd_line,i,m_params[i],'/');
				strlwr				(m_params[i]);
			}
		}
	};
	params							m_game_params;

	xr_set<CPS_Instance*>			ps_active;
	xr_vector<CPS_Instance*>		ps_destroy;
	xr_vector<CPS_Instance*>		ps_needtoplay;

public:
	enum GrassBenders_Anim
	{
		BENDER_ANIM_EXPLOSION = 0,
		BENDER_ANIM_DEFAULT = 1,
		BENDER_ANIM_WAVY = 2,
		BENDER_ANIM_SUCK = 3,
		BENDER_ANIM_BLOW = 4,
		BENDER_ANIM_PULSE = 5,
	};

	void GrassBendersUpdateAnimations();
	void GrassBendersAddExplosion(u16 id, Fvector position, Fvector3 dir, float fade, float speed, float intensity, float radius);
	void GrassBendersAddShot(u16 id, Fvector position, Fvector3 dir, float fade, float speed, float intensity, float radius);
	void GrassBendersRemoveById(u16 id);
	void GrassBendersRemoveByIndex(u8& idx);
	void GrassBendersUpdate(u16 id, u8& data_idx, u32& data_frame, Fvector& position, float radius, float str, bool CheckDistance);
	void GrassBendersReset(u8 idx);
	void GrassBendersSet(u8 idx, u16 id, Fvector position, Fvector3 dir, float fade, float speed, float str, float radius, GrassBenders_Anim anim, bool resetTime);
	float GrassBenderToValue(float& current, float go_to, float intensity, bool use_easing);

	CPerlinNoise1D* PerlinNoise1D{};

	struct grass_data
	{
		u8 index;
		s8 anim[16];
		u16 id[16];
		Fvector pos[16];
		Fvector3 dir[16];
		float radius[16];
		float radius_curr[16];
		float str[16];
		float str_target[16];
		float time[16];
		float fade[16];
		float speed[16];
	} grass_shader_data{};


	u32 m_last_ray_pick_time;
	bool m_isInHideout;
	bool IsActorInHideout	() const;
	void UpdateHudRaindrops	() const;
	void UpdateRainGloss	() const;

			void					destroy_particles	(const bool &all_particles);


	virtual void					PreStart			(LPCSTR op);
	virtual void					Start				(LPCSTR op);
	virtual void					Disconnect			();
#ifndef _EDITOR
	IGame_ObjectPool				ObjectPool;
	CEnvironment*					pEnvironment;
	CEnvironment&					Environment()	{return *pEnvironment;};
	void							Prefetch			( );
#endif

	void							DestroyEnvironment	();
	void							CreateEnvironment	();

	IMainMenu*						m_pMainMenu;

	ShadersExternalData*			m_pGShaderConstants; //--#SM+#--

    static bool 					IsMainMenuActive	();


	virtual bool					OnRenderPPUI_query	() { return FALSE; };	// should return true if we want to have second function called
	virtual void					OnRenderPPUI_main	() {};
	virtual void					OnRenderPPUI_PP		() {};

	virtual	void					OnAppStart			();
	virtual void					OnAppEnd			();
	virtual	void					OnAppActivate		();
	virtual void					OnAppDeactivate		();
	virtual void		_BCL		OnFrame				();

	// вызывается только когда изменяется тип игры
	virtual	void					OnGameStart			(); 
	virtual void					OnGameEnd			();

	virtual void					UpdateGameType		() {};
	virtual void					GetCurrentDof		(Fvector3& dof){dof.set(-1.4f, 0.0f, 250.f);};
	virtual void					SetBaseDof			(const Fvector3& dof){};
	virtual void					OnSectorChanged		(int sector){};
	virtual void					OnAssetsChanged		();
	virtual int						GetHudGlassElement	() { return 0; };
	virtual bool					GetHudGlassEnabled	() { return 0; };
	virtual float					GetActorMaxHealth	() { return 0; };
	virtual float					GetActorHealth		() { return 0; };
	virtual float					GetActorMaxPower	() { return 0; };
	virtual float					GetActorPower		() { return 0; };
	virtual float					GetActorBleeding	() { return 0; };
	virtual float					GetActorIntoxication() { return 0; };
	virtual float					GetActorFrostbite	() { return 0; };
	virtual float					GetRainDropsFactor	() { return 0; };
	virtual void					SetRainDropsFactor	(float factor) {};
	virtual bool					GetClearMaskProcess	() { return 0; };
	virtual bool					GetActorAliveStatus () { return 0; };
	virtual bool					GetActor			() { return 0; };
	virtual bool					GetActorNightvision	() { return 0; };
	virtual bool					GetActorHelmetStatus() { return 0; };
	virtual bool					GetFogInfluenceVolumetricLight() { return 0; };

	virtual std::string				GetMoonPhase		() { return 0; };
	virtual u32						GetTimeHours		() { return 0; }

	virtual int						GetNightvisionType	() { return 0; };
	virtual bool					IsCamFirstEye		() { return 0; };

	virtual bool					IsTutorialSequencerActive() { return 0; };

	virtual void					EditorOnFrame		() {};
	virtual void					RegisterModel		(IRenderVisual* V)
#ifndef _EDITOR
     = 0;
#else
	{}
#endif
	virtual	float					MtlTransparent		(u32 mtl_idx)
#ifndef _EDITOR
	= 0;
#else
	{return 1.f;}
#endif

	IGame_Persistent				();
	virtual ~IGame_Persistent		();

	ICF		u32						GameType			() {return m_game_params.m_e_game_type;};
	virtual void					Statistics			(CGameFont* F)
#ifndef _EDITOR
     = 0;
#else
	{}
#endif
	virtual	void					LoadTitle			(bool change_tip=false, shared_str map_name=""){}
	virtual	void					SetLoadStageTitle	(const char* ls_title = nullptr) {}
	virtual bool					CanBePaused			()		{ return true;}
			bool					render_scene;
		 IC bool					SceneRenderingBlocked();

	struct devices_data
	{
		float pda_display_factor;
		float pda_psy_influence;
		float pda_displaybrightness;
		float device_global_psy_influence;
		float device_psy_zone_influence;
		float device_radiation_zone_influence;
		float nightvision_lum_factor;
	} devices_shader_data;
};

class IMainMenu
{
public:
	virtual			~IMainMenu						()													{};
	virtual void	Activate						(bool bActive)										=0; 
	virtual	bool	IsActive						()													=0; 
	virtual	bool	CanSkipSceneRendering			()													=0; 
	virtual void	DestroyInternal					(bool bForce)										=0;
};

// Ascii's shaders
extern ENGINE_API	Fvector4 ps_ssfx_grass_shadows;
extern ENGINE_API	Fvector3 ps_ssfx_shadow_cascades;
extern ENGINE_API	Fvector4 ps_ssfx_grass_interactive;
extern ENGINE_API	Fvector4 ps_ssfx_int_grass_params_1;
extern ENGINE_API	Fvector4 ps_ssfx_int_grass_params_2;
extern ENGINE_API	Fvector4 ps_ssfx_hud_drops_1;
extern ENGINE_API	Fvector4 ps_ssfx_hud_drops_2;
extern ENGINE_API	Fvector4 ps_ssfx_hud_drops_1_cfg;
extern ENGINE_API	Fvector4 ps_ssfx_hud_drops_2_cfg;
extern ENGINE_API	Fvector4 ps_ssfx_blood_decals;
extern ENGINE_API	Fvector4 ps_ssfx_rain_1;
extern ENGINE_API	Fvector4 ps_ssfx_rain_2;
extern ENGINE_API	Fvector4 ps_ssfx_rain_3;
extern ENGINE_API	Fvector4 ps_ssfx_wetsurfaces_1;
extern ENGINE_API	Fvector4 ps_ssfx_wetsurfaces_2;
extern ENGINE_API	float ps_ssfx_gloss_factor;
extern ENGINE_API	Fvector3 ps_ssfx_gloss_minmax;
extern ENGINE_API	Fvector4 ps_ssfx_lightsetup_1;

extern ENGINE_API	float ps_r3_dyn_wet_surf_far; // 30.0f
extern ENGINE_API	int ps_r3_dyn_wet_surf_sm_res; // 256

extern ENGINE_API	IGame_Persistent*	g_pGamePersistent;
#endif //IGame_PersistentH

