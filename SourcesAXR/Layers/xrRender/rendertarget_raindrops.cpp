#include "stdafx.h"

void CRenderTarget::PhaseRainDrops()
{
	if (Device.m_SecondViewport.IsSVPFrame()) //В прицеле не рендерим
		return;

	static float rain_drops_factor = 0.f;
	static u32 steps_finished = 0;

	// Чтобы эффект перезапускался по HudGlassEnabled.
	static bool saved_rain_drops_control = false;

	bool current_rain_drops_control = g_pGamePersistent->GetHudGlassEnabled();

	if (saved_rain_drops_control != current_rain_drops_control)
	{
		saved_rain_drops_control = current_rain_drops_control;

		rain_drops_factor = 0.f;
		steps_finished = 0;
	}

	if (!current_rain_drops_control)
		return;

	// Функция рассчитывает интенсивность эффекта капель на худе. В шейдере нормально рассчитать слишком муторно, проще посчитать здесь и получить в шейдере через c_timers.w
	auto update_rain_drops_factor = [](bool act_on_rain)
	{
		float rain_factor = g_pGamePersistent->pEnvironment->CurrentEnv->rain_density;
		if (!fis_zero(rain_factor))
		{
			// В данном варианте настроек - при выходе из укрытия в шторм, капли заработают на полную мощность за 20 секунд. При заходе в укрытие - эффект отключится так же через 20 секунд.
			constexpr u32 change_step = 200; //Интервал в миллисекундах между ступенями изменения rain_drops_factor
			constexpr u32 steps_count = 100; //Кол-во ступеней. Чем меньше интервал - тем больше ступеней должно быть.
			constexpr float step_rain_factor_change = 1.f / float(steps_count);

			static bool saved_rain_flag = act_on_rain;
			if (saved_rain_flag != act_on_rain)
			{
				saved_rain_flag = act_on_rain;
				steps_finished = 0;
			}

			if (steps_finished < (steps_count + 1))
			{ // + 1 обязательно из за неровного деления. Иначе эффект при максимальном шторме может не до конца отключаться при входе в укрытие.
				static u32 last_update = Device.dwTimeGlobal;
				if (Device.dwTimeGlobal > (last_update + change_step))
				{
					last_update = Device.dwTimeGlobal;
					steps_finished++;
					if (act_on_rain)
					{ //плавное повышение интенсивности капель.
						rain_drops_factor += step_rain_factor_change;
					}
					else
					{ //плавное понижение интенсивности капель.
						rain_drops_factor -= step_rain_factor_change;
					}
				}
			}
			else if (act_on_rain)
			{ //Если актор не находится в укрытии - синхронизируем rain_drops_factor с интенсивностью дождя
				rain_drops_factor = std::max(rain_drops_factor, rain_factor);
			}

			rain_drops_factor = std::clamp(rain_drops_factor, 0.f, rain_factor); //Уравниваем, чтобы не было превышения
		}
		else
		{
			steps_finished = 0;
			rain_drops_factor = 0.f;
		}
	};

	static bool actor_in_hideout = true;
	static u32 last_ray_pick_time = Device.dwTimeGlobal;
	if (Device.dwTimeGlobal > (last_ray_pick_time + 1000))
	{ //Апдейт рейтрейса - раз в секунду. Чаще апдейтить нет смысла.
		last_ray_pick_time = Device.dwTimeGlobal;

		collide::rq_result RQ;
		actor_in_hideout = !!g_pGameLevel->ObjectSpace.RayPick(Device.vCameraPosition, Fvector().set(0, 1, 0), 50.f, collide::rqtBoth, RQ, g_pGameLevel->CurrentViewEntity());
	}

	update_rain_drops_factor(!actor_in_hideout);

	if (fis_zero(rain_drops_factor))
		return;

	u32 Offset = 0;
	Fvector2 p0, p1;

	struct v_aa
	{
		Fvector4 p;
		Fvector2 uv0;
		Fvector2 uv1;
		Fvector2 uv2;
		Fvector2 uv3;
		Fvector2 uv4;
		Fvector4 uv5;
		Fvector4 uv6;
	};

	float _w = float(Device.dwWidth);
	float _h = float(Device.dwHeight);
	float ddw = 1.f / _w;
	float ddh = 1.f / _h;
	p0.set(.5f / _w, .5f / _h);
	p1.set((_w + .5f) / _w, (_h + .5f) / _h);

	// Set RT's
#if defined(USE_DX10) || defined(USE_DX11)
	ref_rt dest_rt = RImplementation.o.dx10_msaa ? rt_Generic : rt_Color;
	u_setrt(dest_rt, NULL, NULL, HW.pBaseZB);
#else
	u_setrt(rt_Generic_0, NULL, NULL, HW.pBaseZB);
#endif

	RCache.set_CullMode(CULL_NONE);
	RCache.set_Stencil(FALSE);

	// Fill vertex buffer
	v_aa* pv = (v_aa*)RCache.Vertex.Lock(4, g_rain_drops->vb_stride, Offset);
	pv->p.set(EPS, float(_h + EPS), EPS, 1.f); pv->uv0.set(p0.x, p1.y); pv->uv1.set(p0.x - ddw, p1.y - ddh); pv->uv2.set(p0.x + ddw, p1.y + ddh); pv->uv3.set(p0.x + ddw, p1.y - ddh); pv->uv4.set(p0.x - ddw, p1.y + ddh); pv->uv5.set(p0.x - ddw, p1.y, p1.y, p0.x + ddw); pv->uv6.set(p0.x, p1.y - ddh, p1.y + ddh, p0.x); pv++;
	pv->p.set(EPS, EPS, EPS, 1.f); pv->uv0.set(p0.x, p0.y); pv->uv1.set(p0.x - ddw, p0.y - ddh); pv->uv2.set(p0.x + ddw, p0.y + ddh); pv->uv3.set(p0.x + ddw, p0.y - ddh); pv->uv4.set(p0.x - ddw, p0.y + ddh); pv->uv5.set(p0.x - ddw, p0.y, p0.y, p0.x + ddw); pv->uv6.set(p0.x, p0.y - ddh, p0.y + ddh, p0.x); pv++;
	pv->p.set(float(_w + EPS), float(_h + EPS), EPS, 1.f); pv->uv0.set(p1.x, p1.y); pv->uv1.set(p1.x - ddw, p1.y - ddh); pv->uv2.set(p1.x + ddw, p1.y + ddh); pv->uv3.set(p1.x + ddw, p1.y - ddh); pv->uv4.set(p1.x - ddw, p1.y + ddh); pv->uv5.set(p1.x - ddw, p1.y, p1.y, p1.x + ddw); pv->uv6.set(p1.x, p1.y - ddh, p1.y + ddh, p1.x); pv++;
	pv->p.set(float(_w + EPS), EPS, EPS, 1.f); pv->uv0.set(p1.x, p0.y); pv->uv1.set(p1.x - ddw, p0.y - ddh); pv->uv2.set(p1.x + ddw, p0.y + ddh); pv->uv3.set(p1.x + ddw, p0.y - ddh); pv->uv4.set(p1.x - ddw, p0.y + ddh); pv->uv5.set(p1.x - ddw, p0.y, p0.y, p1.x + ddw); pv->uv6.set(p1.x, p0.y - ddh, p0.y + ddh, p1.x); pv++;
	RCache.Vertex.Unlock(4, g_rain_drops->vb_stride);

	// Draw COLOR
	RCache.set_Element(s_rain_drops->E[0]);
	RCache.set_c("rain_drops_params", rain_drops_factor, ps_r2_rain_drops_intensity, ps_r2_rain_drops_speed, 0.0f);
	RCache.set_Geometry(g_rain_drops);
	RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 4, 0, 2);

#if defined(USE_DX10) || defined(USE_DX11)
	HW.pContext->CopyResource(rt_Generic_0->pTexture->surface_get(), dest_rt->pTexture->surface_get());
#endif
}