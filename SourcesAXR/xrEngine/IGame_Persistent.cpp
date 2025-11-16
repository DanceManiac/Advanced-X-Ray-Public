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
#	include "perlin.h"
#endif

#ifdef INGAME_EDITOR
#	include "editor_environment_manager.hpp"
#endif // INGAME_EDITOR

ENGINE_API	IGame_Persistent*		g_pGamePersistent	= NULL;

//ECO_RENDER add
bool IGame_Persistent::IsMainMenuActive()
{
    return g_pGamePersistent && g_pGamePersistent->m_pMainMenu && g_pGamePersistent->m_pMainMenu->IsActive();
}

IGame_Persistent::IGame_Persistent	()
{
    ZoneScoped;

	RDEVICE.seqAppStart.Add			(this);
	RDEVICE.seqAppEnd.Add			(this);
	RDEVICE.seqFrame.Add			(this,REG_PRIORITY_HIGH+1);
	RDEVICE.seqAppActivate.Add		(this);
	RDEVICE.seqAppDeactivate.Add	(this);

	m_pMainMenu						= NULL;

	PerlinNoise1D = xr_new<CPerlinNoise1D>(Random.randI(0, 0xFFFF));
	PerlinNoise1D->SetOctaves(2);
	PerlinNoise1D->SetAmplitude(0.66666f);

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
    ZoneScoped;

	xr_delete(PerlinNoise1D);
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
    ZoneScoped;

#ifndef _EDITOR
	Environment().load				();
#endif    
}

void IGame_Persistent::OnAppEnd		()
{
    ZoneScoped;

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
    ZoneScoped;

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
    ZoneScoped;

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
    ZoneScoped;

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
    ZoneScoped;

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
    ZoneScoped;

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
    ZoneScoped;

#ifndef _EDITOR
	ObjectPool.clear					();
	Render->models_Clear				(TRUE);
#endif
}

void IGame_Persistent::OnFrame		()
{
    ZoneScoped;

#ifndef _EDITOR

    if (pEnvironment && (!Device.Paused() || Device.dwPrecacheFrame))
    {
        Environment().OnFrame();
        UpdateHudRaindrops();
        UpdateRainGloss();
    }


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
    ZoneScoped;

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

void IGame_Persistent::GrassBendersUpdate(u16 id, u8& data_idx, u32& data_frame, Fvector& position, float init_radius, float init_str, bool CheckDistance)
{
    ZoneScoped;

    // Interactive grass disabled
    if (ps_ssfx_grass_interactive.y < 1)
        return;

    // Just update position if not NULL
    if (data_idx != 0)
    {
        // Explosions can take the mem spot, unassign and try to get a spot later.
        if (grass_shader_data.id[data_idx] != id)
        {
            data_idx = 0;
            data_frame = Device.dwFrame + Random.randI(10, 35);
        }
        else
        {
            grass_shader_data.pos[data_idx] = position;
        }
    }

    if (Device.dwFrame < data_frame)
        return;

    // Wait some random frames to split the checks
    data_frame = Device.dwFrame + Random.randI(10, 35);

    // Check Distance
    if (CheckDistance)
    {
        if (position.distance_to_xz_sqr(Device.vCameraPosition) > ps_ssfx_grass_interactive.z)
        {
            GrassBendersRemoveByIndex(data_idx);
            return;
        }
    }

    CFrustum& view_frust = ::Render->ViewBase;
    u32 mask = 0xff;
    float rad = data_idx == 0 ? 1.0 : std::max(1.0f, grass_shader_data.radius_curr[data_idx] + 0.5f);

    // In view frustum?
    if (!view_frust.testSphere(position, rad, mask))
    {
        GrassBendersRemoveByIndex(data_idx);
        return;
    }

    // Empty slot, let's use this
    if (data_idx == 0)
    {
        u8 idx = grass_shader_data.index + 1;

        // Add to grass blenders array
        if (grass_shader_data.id[idx] == 0)
        {
            data_idx = idx;
            GrassBendersSet(idx, id, position, Fvector3().set(0, -99, 0), 0, 0, 0.0f, init_radius, BENDER_ANIM_DEFAULT, true);

            grass_shader_data.str_target[idx] = init_str;
            grass_shader_data.radius_curr[idx] = init_radius;
        }
        // Back to 0 when the array limit is reached
        grass_shader_data.index = idx < ps_ssfx_grass_interactive.y ? idx : 0;
    }
    else
    {
        // Already in view, let's add more time to re-check
        data_frame += 60;
        grass_shader_data.pos[data_idx] = position;
    }
}

void IGame_Persistent::GrassBendersAddExplosion(u16 id, Fvector position, Fvector3 dir, float fade, float speed, float intensity, float radius)
{
    ZoneScoped;

    if (ps_ssfx_grass_interactive.y < 1)
        return;

    for (int idx = 1; idx < ps_ssfx_grass_interactive.y + 1; idx++)
    {
        // Add explosion to any spot not already taken by an explosion.
        if (grass_shader_data.anim[idx] != BENDER_ANIM_EXPLOSION)
        {
            // Add 99 to the ID to avoid conflicts between explosions and basic benders happening at the same time with the same ID.
            GrassBendersSet((u8)idx, id + 99, position, dir, fade, speed, intensity, radius, BENDER_ANIM_EXPLOSION, true);
            grass_shader_data.str_target[idx] = intensity;
            break;
        }
    }
}

void IGame_Persistent::GrassBendersAddShot(u16 id, Fvector position, Fvector3 dir, float fade, float speed, float intensity, float radius)
{
    ZoneScoped;

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
            GrassBendersSet(idx, id, position, dir, fade, speed, currentSTR, radius, BENDER_ANIM_EXPLOSION, false);
            grass_shader_data.str_target[idx] += intensity;
            AddAt = -1;
            break;
        }
        else
        {
            // Check all indexes and keep usable index to use later if needed...
            if (AddAt == -1 && fsimilar(grass_shader_data.radius[idx], 0.f))
                AddAt = idx;
        }
    }

    // We got an available index... Add bender at AddAt
    if (AddAt != -1)
    {
        GrassBendersSet(AddAt, id, position, dir, fade, speed, 0.001f, radius, BENDER_ANIM_EXPLOSION, true);
        grass_shader_data.str_target[AddAt] = intensity;
    }
}

void IGame_Persistent::GrassBendersUpdateAnimations()
{
    ZoneScoped;

    for (int idx = 1; idx < ps_ssfx_grass_interactive.y + 1; idx++)
    {
        if (grass_shader_data.id[idx] != 0)
        {
            switch (grass_shader_data.anim[idx])
            {
            case BENDER_ANIM_EXPLOSION: // Internal Only ( You can use BENDER_ANIM_PULSE for anomalies )
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
            break;

            case BENDER_ANIM_WAVY:
            {
                // Anim Speed
                grass_shader_data.time[idx] += Device.fTimeDelta * 1.5f * grass_shader_data.speed[idx];

                // Curve
                float curve = sin(grass_shader_data.time[idx]);

                // Intensity using curve
                grass_shader_data.str[idx] = curve * cos(curve * 1.4f) * 1.8f * grass_shader_data.str_target[idx];
            }

            break;

            case BENDER_ANIM_SUCK:
            {
                // Anim Speed
                grass_shader_data.time[idx] += Device.fTimeDelta * grass_shader_data.speed[idx];

                // Perlin Noise
                float curve = clampr(PerlinNoise1D->GetContinious(grass_shader_data.time[idx]) + 0.5f, 0.f, 1.f) * -1.0;

                // Intensity using Perlin
                grass_shader_data.str[idx] = curve * grass_shader_data.str_target[idx];
            }
            break;

            case BENDER_ANIM_BLOW:
            {
                // Anim Speed
                grass_shader_data.time[idx] += Device.fTimeDelta * 1.2f * grass_shader_data.speed[idx];

                // Perlin Noise
                float curve = clampr(PerlinNoise1D->GetContinious(grass_shader_data.time[idx]) + 1.0f, 0.f, 2.0f) * 0.25f;

                // Intensity using Perlin
                grass_shader_data.str[idx] = curve * grass_shader_data.str_target[idx];
            }
            break;

            case BENDER_ANIM_PULSE:
            {
                // Anim Speed
                grass_shader_data.time[idx] += Device.fTimeDelta * grass_shader_data.speed[idx];

                // Radius
                grass_shader_data.radius_curr[idx] = grass_shader_data.radius[idx] * std::min(1.0f, grass_shader_data.time[idx]);

                // Diminish intensity when radius target is reached
                if (grass_shader_data.radius_curr[idx] >= grass_shader_data.radius[idx])
                    grass_shader_data.str[idx] += GrassBenderToValue(grass_shader_data.str[idx], 0.0f, grass_shader_data.speed[idx] * 0.6f, true);

                // Loop when intensity is <= 0
                if (grass_shader_data.str[idx] <= 0.0f)
                {
                    grass_shader_data.str[idx] = grass_shader_data.str_target[idx];
                    grass_shader_data.radius_curr[idx] = 0.0f;
                    grass_shader_data.time[idx] = 0.0f;
                }

            }
            break;

            case BENDER_ANIM_DEFAULT:

                // Just fade to target strength
                grass_shader_data.str[idx] += GrassBenderToValue(grass_shader_data.str[idx], grass_shader_data.str_target[idx], 2.0f, true);

                break;
            }
        }
    }
}

void IGame_Persistent::GrassBendersRemoveByIndex(u8& idx)
{
    if (idx != 0)
    {
        GrassBendersReset(idx);
        idx = 0;
    }
}

void IGame_Persistent::GrassBendersRemoveById(u16 id)
{
    ZoneScoped;

    // Search by Object ID ( Used when removing benders CPHMovementControl::DestroyCharacter() )
    for (int i = 1; i < ps_ssfx_grass_interactive.y + 1; i++)
        if (grass_shader_data.id[i] == id)
            GrassBendersReset(i);
}

void IGame_Persistent::GrassBendersReset(u8 idx)
{
    ZoneScoped;

    // Reset Everything
    GrassBendersSet(idx, 0, Fvector3().set(0, 0, 0), Fvector3().set(0, -99, 0), 0, 0, 0, 0, BENDER_ANIM_DEFAULT, true);
    grass_shader_data.str_target[idx] = 0;
}

void IGame_Persistent::GrassBendersSet(u8 idx, u16 id, Fvector position, Fvector3 dir, float fade, float speed, float intensity, float radius, GrassBenders_Anim anim, bool resetTime)
{
    ZoneScoped;

    // Set values
    grass_shader_data.anim[idx] = anim;
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

bool IGame_Persistent::IsActorInHideout() const
{
    static bool actor_in_hideout = true;
    static u32 last_ray_pick_time = Device.dwTimeGlobal;

    if (Device.dwTimeGlobal > (last_ray_pick_time + 1000))
    { // Апдейт рейтрейса - раз в секунду. Чаще апдейтить нет смысла.
        last_ray_pick_time = Device.dwTimeGlobal;
        collide::rq_result RQ;
        actor_in_hideout = !!g_pGameLevel->ObjectSpace.RayPick(Device.vCameraPosition, Fvector{ 0.f, 1.f, 0.f }, 50.f, collide::rqtBoth, RQ, g_pGameLevel->CurrentViewEntity());
    }
    return actor_in_hideout;
}

void IGame_Persistent::UpdateHudRaindrops() const
{
    ZoneScoped;

    const struct // Настройки
    {
        float density = ps_ssfx_hud_drops_1_cfg.x; // Quantity of drops
        float reflection_str = ps_ssfx_hud_drops_1_cfg.y; // Refrelction intensity
        float refraction_str = ps_ssfx_hud_drops_1_cfg.z; // Refraction intensity
        float animation_speed = ps_ssfx_hud_drops_1_cfg.w; // Speed of the drops animation
        float buildup = ps_ssfx_hud_drops_2_cfg.x; // Drops build up speed
        float drying = ps_ssfx_hud_drops_2_cfg.y; // Drying speed
        float size = ps_ssfx_hud_drops_2_cfg.z; // Size of the drops
        float gloss = ps_ssfx_hud_drops_2_cfg.w; // Raindrops gloss intensity
        float extra_gloss{ ps_ssfx_gloss_minmax.z }; // Extra gloss to the weapons HUD elements when raining
    } ssfx_default_settings;

    const float ssfx_hud_raindrops_density = ssfx_default_settings.density;
    const float ssfx_hud_raindrops_refle = 30.f * ssfx_default_settings.reflection_str;
    const float ssfx_hud_raindrops_refra = 0.05f * ssfx_default_settings.refraction_str;
    const float ssfx_hud_raindrops_anim_speed = 0.02f * ssfx_default_settings.animation_speed;
    const float ssfx_hud_raindrops_build_speed = 0.1f * ssfx_default_settings.buildup;
    const float ssfx_hud_raindrops_drying_speed = 0.1f * ssfx_default_settings.drying;
    const float ssfx_hud_raindrops_size = ssfx_default_settings.size;
    const float ssfx_hud_raindrops_gloss = ssfx_default_settings.gloss;
    const float ssfx_hud_raindrops_extragloss = ssfx_default_settings.extra_gloss;
    const float val_density = 0.15f * (3.5f - ssfx_hud_raindrops_density); // 0.5 ~3.0
    const float val_texsize = 2.0f - ssfx_hud_raindrops_size;
    ps_ssfx_hud_drops_2.set(val_density, val_texsize, ssfx_hud_raindrops_extragloss, ssfx_hud_raindrops_gloss);
    static float drops_int{}, drops_anim{};
    const float Rain_factor = g_pGamePersistent->Environment().CurrentEnv->rain_density;

    const float delta_time = Device.fTimeDelta;
    if (Rain_factor > 0.f)
    {
        if (!IsActorInHideout())
        {
            // Use rain intensity factor to slowdown <->speedup rain animation
            float rain_speed_factor = (1.5f - Rain_factor) * 10.f;
            drops_anim = drops_anim + ssfx_hud_raindrops_anim_speed * delta_time / rain_speed_factor;
            drops_int = drops_int + ssfx_hud_raindrops_build_speed * delta_time / 100.f;
        }
        else
            drops_int = drops_int - ssfx_hud_raindrops_drying_speed * delta_time / 100.f;
    }
    else
        drops_int = drops_int - ssfx_hud_raindrops_drying_speed * delta_time / 100.f;

    // Saturate drops intensity
    clamp(drops_int, 0.0f, 1.0f);
    // Reset after 99k
    if (drops_anim > 99000.f)
        drops_anim = 0.f;
    // Update shader data
    ps_ssfx_hud_drops_1.set(drops_anim, drops_int, ssfx_hud_raindrops_refle, ssfx_hud_raindrops_refra);
}

void IGame_Persistent::UpdateRainGloss() const
{
    ZoneScoped;

    const struct // Настройки
    {
        bool auto_gloss{ !fis_zero(ps_ssfx_lightsetup_1.z) }; // Automatic adjustment of gloss based on wetness.
        float auto_gloss_max{ ps_ssfx_lightsetup_1.w }; // Value to control the maximum value of gloss when full wetness is reached. ( 0 = 0% | 1 = 100% )

        float ripples_size{ ps_ssfx_wetsurfaces_1.x };
        float ripples_speed{ ps_ssfx_wetsurfaces_1.y };
        float ripples_min_speed{ ps_ssfx_wetsurfaces_1.z };
        float ripples_intensity{ ps_ssfx_wetsurfaces_1.w };

        float waterfall_size{ ps_ssfx_wetsurfaces_2.x };
        float waterfall_speed{ ps_ssfx_wetsurfaces_2.y };
        float waterfall_min_speed{ ps_ssfx_wetsurfaces_2.z };
        float waterfall_intensity{ ps_ssfx_wetsurfaces_2.w };
    } ssfx_default_settings;

    if (ssfx_default_settings.auto_gloss)
    {
        const float Wetness_gloss =
            ps_ssfx_gloss_minmax.x + fmax(ssfx_default_settings.auto_gloss_max - ps_ssfx_gloss_minmax.x, 0.f) * g_pGamePersistent->Environment().wetness_accum;

        ps_ssfx_gloss_factor = Wetness_gloss * 0.96f;
    }
    else
        ps_ssfx_gloss_factor = 0.5f;

    const float ripples_size = fmax(2.0f - ssfx_default_settings.ripples_size, 0.01f); // Change how the value works to be more intuitive(<1.0 smaller |> 1.0 bigger)
    ps_ssfx_wetsurfaces_1.set(ripples_size, ssfx_default_settings.ripples_speed, ssfx_default_settings.ripples_min_speed, ssfx_default_settings.ripples_intensity);

    const float waterfall_size = fmax(2.0f - ssfx_default_settings.waterfall_size, 0.01f); // Change how the value works to be more intuitive(<1.0 smaller |> 1.0 bigger) get_console()
    ps_ssfx_wetsurfaces_2.set(waterfall_size, ssfx_default_settings.waterfall_speed, ssfx_default_settings.waterfall_min_speed, ssfx_default_settings.waterfall_intensity);
}

float IGame_Persistent::GrassBenderToValue(float& current, float go_to, float intensity, bool use_easing)
{
    ZoneScoped;

    float diff = abs(current - go_to);

    float r_value = Device.fTimeDelta * intensity * (use_easing ? std::min(0.5f, diff) : 1.0f);

    if (diff - r_value <= 0)
    {
        current = go_to;
        return 0;
    }

    return current < go_to ? r_value : -r_value;
}

void IGame_Persistent::DestroyEnvironment()
{
    Environment().unload();
}

void IGame_Persistent::CreateEnvironment()
{
    Environment().load();
    Environment().bNeed_re_create_env = TRUE;
}