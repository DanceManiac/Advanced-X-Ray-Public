#include "stdafx.h"
#pragma hdrstop

#include "IGame_Persistent.h"

#ifndef _EDITOR
#include "environment.h"
#	include "x_ray.h"
#	include "IGame_Level.h"
#	include "XR_IOConsole.h"
#	include "Render.h"
#	include "ps_instance.h"
#	include "CustomHUD.h"
#endif

#ifdef _EDITOR
	bool g_dedicated_server	= false;
#endif

#ifdef INGAME_EDITOR
#	include "editor_environment_manager.hpp"
#endif // INGAME_EDITOR

extern Fvector4 ps_ssfx_grass_interactive;
ENGINE_API	IGame_Persistent*		g_pGamePersistent	= NULL;

//ECO_RENDER add
bool IGame_Persistent::IsMainMenuActive()
{
    return g_pGamePersistent && g_pGamePersistent->m_pMainMenu && g_pGamePersistent->m_pMainMenu->IsActive();
}

IGame_Persistent::IGame_Persistent	()
{
	RDEVICE.seqAppStart.Add			(this);
	RDEVICE.seqAppEnd.Add			(this);
	RDEVICE.seqFrame.Add			(this,REG_PRIORITY_HIGH+1);
	RDEVICE.seqAppActivate.Add		(this);
	RDEVICE.seqAppDeactivate.Add	(this);

	m_pMainMenu						= NULL;

	m_pGShaderConstants = new ShadersExternalData(); //--#SM+#--

#ifndef INGAME_EDITOR
	#ifndef _EDITOR
	pEnvironment					= xr_new<CEnvironment>();
	#endif
#else // #ifdef INGAME_EDITOR
	if (RDEVICE.editor())
		pEnvironment				= xr_new<editor::environment::manager>();
	else
		pEnvironment				= xr_new<CEnvironment>();
#endif // #ifdef INGAME_EDITOR

	render_scene = true;
}

IGame_Persistent::~IGame_Persistent	()
{
	RDEVICE.seqFrame.Remove			(this);
	RDEVICE.seqAppStart.Remove		(this);
	RDEVICE.seqAppEnd.Remove			(this);
	RDEVICE.seqAppActivate.Remove	(this);
	RDEVICE.seqAppDeactivate.Remove	(this);
#ifndef _EDITOR
	xr_delete						(pEnvironment);
#endif
	xr_delete						(m_pGShaderConstants); //--#SM+#--
}

void IGame_Persistent::OnAppActivate		()
{
}

void IGame_Persistent::OnAppDeactivate		()
{
}

void IGame_Persistent::OnAppStart	()
{
#ifndef _EDITOR
	Environment().load				();
#endif    
}

void IGame_Persistent::OnAppEnd		()
{
#ifndef _EDITOR
	Environment().unload			 ();
#endif    
	OnGameEnd						();

#ifndef _EDITOR
	DEL_INSTANCE					(g_hud);
#endif    
}


void IGame_Persistent::PreStart		(LPCSTR op)
{
	string256						prev_type;
	params							new_game_params;
	xr_strcpy							(prev_type,m_game_params.m_game_type);
	new_game_params.parse_cmd_line	(op);

	// change game type
	if (0!=xr_strcmp(prev_type,new_game_params.m_game_type)){
		OnGameEnd					();
	}
}
void IGame_Persistent::Start		(LPCSTR op)
{
	string256						prev_type;
	xr_strcpy							(prev_type,m_game_params.m_game_type);
	m_game_params.parse_cmd_line	(op);
	// change game type
	if ((0!=xr_strcmp(prev_type,m_game_params.m_game_type))) 
	{
		if (*m_game_params.m_game_type)
			OnGameStart					();
#ifndef _EDITOR
		if(g_hud)
			DEL_INSTANCE			(g_hud);
#endif            
	}
	else UpdateGameType();

	VERIFY							(ps_destroy.empty());
}

void IGame_Persistent::Disconnect	()
{
#ifndef _EDITOR
	// clear "need to play" particles
	destroy_particles					(true);

	if(g_hud)
			DEL_INSTANCE			(g_hud);
//.		g_hud->OnDisconnected			();
#endif
}

void IGame_Persistent::OnGameStart()
{
#ifndef _EDITOR
	SetLoadStageTitle("st_prefetching_objects");
	LoadTitle();
	if(!strstr(Core.Params,"-noprefetch"))
		Prefetch();
#endif
}

IC bool IGame_Persistent::SceneRenderingBlocked()
{
	if (!render_scene || (m_pMainMenu && m_pMainMenu->CanSkipSceneRendering()))
	{
		return true;
	}

	return false;
}

#ifndef _EDITOR
void IGame_Persistent::Prefetch()
{
	prefetching_in_progress = true;

	// prefetch game objects & models
	float	p_time		=			1000.f*Device.GetTimerGlobal()->GetElapsed_sec();
	u32	mem_0			=			Memory.mem_usage()	;

	Log				("Loading objects...");
	ObjectPool.prefetch					();
	Log				("Loading models...");
	Render->models_Prefetch				();
	//Device.Resources->DeferredUpload	();
	Device.m_pRender->ResourcesDeferredUpload();

	p_time				=			1000.f*Device.GetTimerGlobal()->GetElapsed_sec() - p_time;
	u32		p_mem		=			Memory.mem_usage() - mem_0	;

	Msg					("* [prefetch] time:    %d ms",	iFloor(p_time));
	Msg					("* [prefetch] memory:  %dKb",	p_mem/1024);

	prefetching_in_progress = false;
}
#endif


void IGame_Persistent::OnGameEnd	()
{
#ifndef _EDITOR
	ObjectPool.clear					();
	Render->models_Clear				(TRUE);
#endif
}

void IGame_Persistent::OnFrame		()
{
#ifndef _EDITOR

	if(!Device.Paused() || Device.dwPrecacheFrame)
		Environment().OnFrame	();


	Device.Statistic->Particles_starting= ps_needtoplay.size	();
	Device.Statistic->Particles_active	= ps_active.size		();
	Device.Statistic->Particles_destroy	= ps_destroy.size		();

	// Play req particle systems
	while (ps_needtoplay.size())
	{
		CPS_Instance*	psi		= ps_needtoplay.back	();
		ps_needtoplay.pop_back	();
		psi->Play				(false);
	}
	// Destroy inactive particle systems
	while (ps_destroy.size())
	{
//		u32 cnt					= ps_destroy.size();
		CPS_Instance*	psi		= ps_destroy.back();
		VERIFY					(psi);
		if (psi->Locked())
		{
			Log("--locked");
			break;
		}
		ps_destroy.pop_back		();
		psi->PSI_internal_delete();
	}
#endif
}

void IGame_Persistent::destroy_particles		(const bool &all_particles)
{
#ifndef _EDITOR
	ps_needtoplay.clear				();

	while (ps_destroy.size())
	{
		CPS_Instance*	psi		= ps_destroy.back	();		
		VERIFY					(psi);
		VERIFY					(!psi->Locked());
		ps_destroy.pop_back		();
		psi->PSI_internal_delete();
	}

	// delete active particles
	if (all_particles) {
		for (;!ps_active.empty();)
			(*ps_active.begin())->PSI_internal_delete	();
	}
	else {
		u32								active_size = ps_active.size();
		CPS_Instance					**I = (CPS_Instance**)_alloca(active_size*sizeof(CPS_Instance*));
		std::copy						(ps_active.begin(),ps_active.end(),I);

		struct destroy_on_game_load {
			static IC bool predicate (CPS_Instance*const& object)
			{
				return					(!object->destroy_on_game_load());
			}
		};

		CPS_Instance					**E = std::remove_if(I,I + active_size,&destroy_on_game_load::predicate);
		for ( ; I != E; ++I)
			(*I)->PSI_internal_delete	();
	}

	VERIFY								(ps_needtoplay.empty() && ps_destroy.empty() && (!all_particles || ps_active.empty()));
#endif
}

void IGame_Persistent::OnAssetsChanged()
{
#ifndef _EDITOR
	Device.m_pRender->OnAssetsChanged(); //Resources->m_textures_description.Load();
#endif    
}

void IGame_Persistent::GrassBendersUpdate(u16 id, u8& data_idx, u32& data_frame, Fvector& position)
{
	// Interactive grass disabled
	if (ps_ssfx_grass_interactive.y < 1)
		return;

	if (RDEVICE.dwFrame < data_frame)
	{
		// Just update position if not NULL
		if (data_idx != NULL)
		{
			// Explosions can take the mem spot, unassign and try to get a spot later.
			if (grass_shader_data.id[data_idx] != id)
			{
				data_idx = NULL;
				data_frame = RDEVICE.dwFrame + Random.randI(10, 35);
			}
			else
			{
				// Just Update... ( FadeIn if str < 1.0f )
				if (grass_shader_data.str[data_idx] < 1.0f)
					grass_shader_data.str[data_idx] += 0.5f * Device.fTimeDelta;
				else
					grass_shader_data.str[data_idx] = 1.0f;

				grass_shader_data.pos[data_idx] = position;
			}
			return;
		}

		// Wait some random frames to split the checks
		data_frame = RDEVICE.dwFrame + Random.randI(10, 35);

		// Check Distance
		if (position.distance_to_xz_sqr(Device.vCameraPosition) > ps_ssfx_grass_interactive.z)
		{
			GrassBendersRemoveByIndex(data_idx);
			return;
		}

		CFrustum& view_frust = ::Render->ViewBase;
		u32 mask = 0xff;

		// In view frustum?
		if (!view_frust.testSphere(position, 1, mask))
		{
			GrassBendersRemoveByIndex(data_idx);
			return;
		}

		// Empty slot, let's use this
		if (data_idx == NULL)
		{
			u8 idx = grass_shader_data.index + 1;

			// Add to grass blenders array
			if (grass_shader_data.id[idx] == NULL)
			{
				data_idx = idx;
				GrassBendersSet(idx, id, position, Fvector3().set(0, -99, 0), 0, 0, 0.0f, NULL, true);
				grass_shader_data.radius_curr[idx] = -1.0f;
			}

			// Back to 0 when the array limit is reached
			grass_shader_data.index = idx < ps_ssfx_grass_interactive.y ? idx : 0;
		}
		else
		{
			// Already inview, let's add more time to re-check
			data_frame += 60;
			grass_shader_data.pos[data_idx] = position;
		}
	}
}

void IGame_Persistent::GrassBendersAddExplosion(u16 id, Fvector position, Fvector3 dir, float fade, float speed, float intensity, float radius)
{
	if (ps_ssfx_grass_interactive.y < 1)
		return;

	for (int idx = 1; idx < ps_ssfx_grass_interactive.y + 1; idx++)
	{
		// Add explosion to any spot not already taken by an explosion.
		if (grass_shader_data.radius[idx] == NULL)
		{
			// Add 99 to avoid conflicts between explosions and basic benders.
			GrassBendersSet(idx, id + 99, position, dir, fade, speed, intensity, radius, true);
			grass_shader_data.str_target[idx] = intensity;
			break;
		}
	}
}

void IGame_Persistent::GrassBendersAddShot(u16 id, Fvector position, Fvector3 dir, float fade, float speed, float intensity, float radius)
{
	// Is disabled?
	if (ps_ssfx_grass_interactive.y < 1 || intensity <= 0.0f)
		return;

	// Check distance
	if (position.distance_to_xz_sqr(Device.vCameraPosition) > ps_ssfx_grass_interactive.z)
		return;

	int AddAt = -1;

	// Look for a spot
	for (int idx = 1; idx < ps_ssfx_grass_interactive.y + 1; idx++)
	{
		// Already exist, just update and increase intensity
		if (grass_shader_data.id[idx] == id)
		{
			float currentSTR = grass_shader_data.str[idx];
			GrassBendersSet(idx, id, position, dir, fade, speed, currentSTR, radius, false);
			grass_shader_data.str_target[idx] += intensity;
			AddAt = -1;
			break;
		}
		else
		{
			// Check all index and keep usable index to use later if needed...
			if (AddAt == -1 && grass_shader_data.radius[idx] == NULL)
				AddAt = idx;
		}
	}

	// We got an available index... Add bender at AddAt
	if (AddAt != -1)
	{
		GrassBendersSet(AddAt, id, position, dir, fade, speed, 0.001f, radius, true);
		grass_shader_data.str_target[AddAt] = intensity;
	}
}

void IGame_Persistent::GrassBendersUpdateExplosions()
{
	for (int idx = 1; idx < ps_ssfx_grass_interactive.y + 1; idx++)
	{
		if (grass_shader_data.radius[idx] != NULL)
		{
			// Radius
			grass_shader_data.time[idx] += Device.fTimeDelta * grass_shader_data.speed[idx];
			grass_shader_data.radius_curr[idx] = grass_shader_data.radius[idx] * std::min(1.0f, grass_shader_data.time[idx]);

			grass_shader_data.str_target[idx] = std::min(1.0f, grass_shader_data.str_target[idx]);

			// Easing
			float diff = abs(grass_shader_data.str[idx] - grass_shader_data.str_target[idx]);
			diff = std::max(0.1f, diff);

			// Intensity
			if (grass_shader_data.str_target[idx] <= grass_shader_data.str[idx])
			{
				grass_shader_data.str[idx] -= Device.fTimeDelta * grass_shader_data.fade[idx] * diff;
			}
			else
			{
				grass_shader_data.str[idx] += Device.fTimeDelta * grass_shader_data.speed[idx] * diff;

				if (grass_shader_data.str[idx] >= grass_shader_data.str_target[idx])
					grass_shader_data.str_target[idx] = 0;
			}

			// Remove Bender
			if (grass_shader_data.str[idx] < 0.0f)
				GrassBendersReset(idx);
		}
	}
}

void IGame_Persistent::GrassBendersRemoveByIndex(u8 & idx)
{
	if (idx != NULL)
	{
		GrassBendersReset(idx);
		idx = NULL;
	}
}

void IGame_Persistent::GrassBendersRemoveById(u16 id)
{
	// Search by Object ID ( Used when removing benders CPHMovementControl::DestroyCharacter() )
	for (int i = 1; i < ps_ssfx_grass_interactive.y + 1; i++)
		if (grass_shader_data.id[i] == id)
			GrassBendersReset(i);
}

void IGame_Persistent::GrassBendersReset(u8 idx)
{
	GrassBendersSet(idx, NULL, { 0,0,0 }, Fvector3().set(0, -99, 0), 0, 0, 1, NULL, true);
}

void IGame_Persistent::GrassBendersSet(u8 idx, u16 id, Fvector position, Fvector3 dir, float fade, float speed, float intensity, float radius, bool resetTime)
{
	// Set values
	grass_shader_data.pos[idx] = position;
	grass_shader_data.id[idx] = id;
	grass_shader_data.radius[idx] = radius;
	grass_shader_data.str[idx] = intensity;
	grass_shader_data.fade[idx] = fade;
	grass_shader_data.speed[idx] = speed;
	grass_shader_data.dir[idx] = dir;

	if (resetTime)
	{
		grass_shader_data.radius_curr[idx] = 0.01f;
		grass_shader_data.time[idx] = 0;
	}
}