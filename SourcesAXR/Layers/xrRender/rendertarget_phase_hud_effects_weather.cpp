#include "stdafx.h"

void CRenderTarget::phase_hud_rainfall()
{

	bool ActorIsInHideout = g_pGamePersistent->IsActorInHideout();
	if (ActorIsInHideout)
		return;


	bool current_rain_drops_cleaning = g_pGamePersistent->GetHudGlassEnabled() && !g_pGamePersistent->GetClearMaskProcess();

	if (!current_rain_drops_cleaning)
		return;


	//Constants
	u32 Offset = 0;
	u32 C = color_rgba(0, 0, 0, 255);

	CEnvDescriptorMixer& envdesc = *g_pGamePersistent->Environment().CurrentEnv;

	float d_Z = EPS_S;
	float d_W = 1.0f;
	float w = float(Device.dwWidth);
	float h = float(Device.dwHeight);

	Fvector2 p0, p1;
#ifdef USE_DX11
	p0.set(0.0f, 0.0f);
	p1.set(1.0f, 1.0f);
#else
	p0.set(0.5f / w, 0.5f / h);
	p1.set((w + 0.5f) / w, (h + 0.5f) / h);
#endif

	//////////////////////////////////////////////////////////////////////////
	//Set MSAA/NonMSAA rendertarget
#ifdef USE_DX11
	ref_rt& dest_rt = RImplementation.o.dx10_msaa ? rt_Generic : rt_Color;
	u_setrt(dest_rt, nullptr, nullptr, nullptr);
#else
	u_setrt(rt_Generic_0, nullptr, nullptr, nullptr);
#endif		

	RCache.set_CullMode(CULL_NONE);
	RCache.set_Stencil(FALSE);

	//Fill vertex buffer
	FVF::TL* pv = (FVF::TL*)RCache.Vertex.Lock(4, g_combine->vb_stride, Offset);
	pv->set(0, float(h), d_Z, d_W, C, p0.x, p1.y); pv++;
	pv->set(0, 0, d_Z, d_W, C, p0.x, p0.y); pv++;
	pv->set(float(w), float(h), d_Z, d_W, C, p1.x, p1.y); pv++;
	pv->set(float(w), 0, d_Z, d_W, C, p1.x, p0.y); pv++;
	RCache.Vertex.Unlock(4, g_combine->vb_stride);

	RCache.set_Element(s_hud_rainfall->E[0]);
	RCache.set_c("hud_rainfall_params", envdesc.m_fWeatherRainFall, 0, 0, 0);
	RCache.set_Geometry(g_combine);
	RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 4, 0, 2);

#ifdef USE_DX11
	HW.pContext->CopyResource(rt_Generic_0->pTexture->surface_get(), dest_rt->pTexture->surface_get());
#endif
}


void CRenderTarget::phase_hud_sweated()
{
	//Constants
	u32 Offset = 0;
	u32 C = color_rgba(0, 0, 0, 255);

	CEnvDescriptorMixer& envdesc = *g_pGamePersistent->Environment().CurrentEnv;

	float d_Z = EPS_S;
	float d_W = 1.0f;
	float w = float(Device.dwWidth);
	float h = float(Device.dwHeight);

	Fvector2 p0, p1;
#ifdef USE_DX11
	p0.set(0.0f, 0.0f);
	p1.set(1.0f, 1.0f);
#else
	p0.set(0.5f / w, 0.5f / h);
	p1.set((w + 0.5f) / w, (h + 0.5f) / h);
#endif

	//////////////////////////////////////////////////////////////////////////
	//Set MSAA/NonMSAA rendertarget
#ifdef USE_DX11
	ref_rt& dest_rt = RImplementation.o.dx10_msaa ? rt_Generic : rt_Color;
	u_setrt(dest_rt, nullptr, nullptr, nullptr);
#else
	u_setrt(rt_Generic_0, nullptr, nullptr, nullptr);
#endif		

	RCache.set_CullMode(CULL_NONE);
	RCache.set_Stencil(FALSE);

	//Fill vertex buffer
	FVF::TL* pv = (FVF::TL*)RCache.Vertex.Lock(4, g_combine->vb_stride, Offset);
	pv->set(0, float(h), d_Z, d_W, C, p0.x, p1.y); pv++;
	pv->set(0, 0, d_Z, d_W, C, p0.x, p0.y); pv++;
	pv->set(float(w), float(h), d_Z, d_W, C, p1.x, p1.y); pv++;
	pv->set(float(w), 0, d_Z, d_W, C, p1.x, p0.y); pv++;
	RCache.Vertex.Unlock(4, g_combine->vb_stride);

	RCache.set_Element(s_hud_sweated->E[1]);
	RCache.set_c("hud_sweated_params", envdesc.m_fAirTemperature, 0, 0, 0);
	RCache.set_Geometry(g_combine);
	RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 4, 0, 2);

#ifdef USE_DX11
	HW.pContext->CopyResource(rt_Generic_0->pTexture->surface_get(), dest_rt->pTexture->surface_get());
#endif
}


void CRenderTarget::phase_hud_rainfall_acid()
{

	bool ActorIsInHideout = g_pGamePersistent->IsActorInHideout();
	if (ActorIsInHideout)
		return;


	bool current_rain_drops_cleaning = g_pGamePersistent->GetHudGlassEnabled() && !g_pGamePersistent->GetClearMaskProcess();

	if (!current_rain_drops_cleaning)
		return;


	//Constants
	u32 Offset = 0;
	u32 C = color_rgba(0, 0, 0, 255);

	CEnvDescriptorMixer& envdesc = *g_pGamePersistent->Environment().CurrentEnv;

	float d_Z = EPS_S;
	float d_W = 1.0f;
	float w = float(Device.dwWidth);
	float h = float(Device.dwHeight);

	Fvector2 p0, p1;
#ifdef USE_DX11
	p0.set(0.0f, 0.0f);
	p1.set(1.0f, 1.0f);
#else
	p0.set(0.5f / w, 0.5f / h);
	p1.set((w + 0.5f) / w, (h + 0.5f) / h);
#endif

	//////////////////////////////////////////////////////////////////////////
	//Set MSAA/NonMSAA rendertarget
#ifdef USE_DX11
	ref_rt& dest_rt = RImplementation.o.dx10_msaa ? rt_Generic : rt_Color;
	u_setrt(dest_rt, nullptr, nullptr, nullptr);
#else
	u_setrt(rt_Generic_0, nullptr, nullptr, nullptr);
#endif		

	RCache.set_CullMode(CULL_NONE);
	RCache.set_Stencil(FALSE);

	//Fill vertex buffer
	FVF::TL* pv = (FVF::TL*)RCache.Vertex.Lock(4, g_combine->vb_stride, Offset);
	pv->set(0, float(h), d_Z, d_W, C, p0.x, p1.y); pv++;
	pv->set(0, 0, d_Z, d_W, C, p0.x, p0.y); pv++;
	pv->set(float(w), float(h), d_Z, d_W, C, p1.x, p1.y); pv++;
	pv->set(float(w), 0, d_Z, d_W, C, p1.x, p0.y); pv++;
	RCache.Vertex.Unlock(4, g_combine->vb_stride);

	RCache.set_Element(s_hud_rainfall_acid->E[2]);
	RCache.set_c("hud_rainfall_acid_params", envdesc.m_fWindVolumeFalloutAcid/**/, droplets_power_debug, 0, 0);
	//RCache.set_c("hud_rainfall_acid_params", envdesc.m_fWeatherRainFall, 0, 0, 0);
	RCache.set_Geometry(g_combine);
	RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 4, 0, 2);

#ifdef USE_DX11
	HW.pContext->CopyResource(rt_Generic_0->pTexture->surface_get(), dest_rt->pTexture->surface_get());
#endif
}