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
			EGameIDs	m_e_game_type;
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

	void GrassBendersUpdateExplosions();
	void GrassBendersAddExplosion(u16 id, Fvector position, Fvector3 dir, float fade, float speed, float intensity, float radius);
	void GrassBendersAddShot(u16 id, Fvector position, Fvector3 dir, float fade, float speed, float intensity, float radius);
	void GrassBendersRemoveById(u16 id);
	void GrassBendersRemoveByIndex(u8 & idx);
	void GrassBendersUpdate(u16 id, u8 & data_idx, u32 & data_frame, Fvector & position);
	void GrassBendersReset(u8 idx);
	void GrassBendersSet(u8 idx, u16 id, Fvector position, Fvector3 dir, float fade, float speed, float intensity, float radius, bool resetTime);

	struct grass_data
	{
		u8 index;
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
	} grass_shader_data;

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

	// ���������� ������ ����� ���������� ��� ����
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
	virtual bool					GetActorAliveStatus () { return 0; };
	virtual bool					GetActor			() { return 0; };
	virtual bool					GetActorNightvision	() { return 0; };
	virtual int						GetNightvisionType	() { return 0; };
	virtual bool					IsCamFirstEye		() { return 0; };

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

	struct pda_data
	{
		float pda_display_factor;
		float pda_psy_influence;
		float pda_displaybrightness;
	} pda_shader_data;
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

extern ENGINE_API	bool g_dedicated_server;
extern ENGINE_API	IGame_Persistent*	g_pGamePersistent;
#endif //IGame_PersistentH

