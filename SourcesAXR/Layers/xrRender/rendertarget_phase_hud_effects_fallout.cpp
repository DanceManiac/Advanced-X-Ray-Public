#include "stdafx.h"

void CRenderTarget::phase_hud_acid_rain()
{
	//Constants
	u32 Offset = 0;
	u32 C = color_rgba(0, 0, 0, 255);

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

	CEnvDescriptorMixer& envdesc = *g_pGamePersistent->Environment().CurrentEnv;

	RCache.set_CullMode(CULL_NONE);
	RCache.set_Stencil(FALSE);

	//Fill vertex buffer
	FVF::TL* pv = (FVF::TL*)RCache.Vertex.Lock(4, g_combine->vb_stride, Offset);
	pv->set(0, float(h), d_Z, d_W, C, p0.x, p1.y); pv++;
	pv->set(0, 0, d_Z, d_W, C, p0.x, p0.y); pv++;
	pv->set(float(w), float(h), d_Z, d_W, C, p1.x, p1.y); pv++;
	pv->set(float(w), 0, d_Z, d_W, C, p1.x, p0.y); pv++;
	RCache.Vertex.Unlock(4, g_combine->vb_stride);

	// Draw COLOR
	RCache.set_Element(s_hud_acid_rain->E[0]);
	RCache.set_c("hud_acid_rain_params", envdesc.m_fWindVolumeFalloutAcid/**/, droplets_power_debug, 0, 0);
	RCache.set_Geometry(g_combine);
	RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 4, 0, 2);

#ifdef USE_DX11
	HW.pContext->CopyResource(rt_Generic_0->pTexture->surface_get(), dest_rt->pTexture->surface_get());
#endif
}

void CRenderTarget::phase_hud_radiation_rain()
{
	//Constants
	u32 Offset = 0;
	u32 C = color_rgba(0, 0, 0, 255);

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

	CEnvDescriptorMixer& envdesc = *g_pGamePersistent->Environment().CurrentEnv;

	RCache.set_CullMode(CULL_NONE);
	RCache.set_Stencil(FALSE);

	//Fill vertex buffer
	FVF::TL* pv = (FVF::TL*)RCache.Vertex.Lock(4, g_combine->vb_stride, Offset);
	pv->set(0, float(h), d_Z, d_W, C, p0.x, p1.y); pv++;
	pv->set(0, 0, d_Z, d_W, C, p0.x, p0.y); pv++;
	pv->set(float(w), float(h), d_Z, d_W, C, p1.x, p1.y); pv++;
	pv->set(float(w), 0, d_Z, d_W, C, p1.x, p0.y); pv++;
	RCache.Vertex.Unlock(4, g_combine->vb_stride);

	// Draw COLOR
	RCache.set_Element(s_hud_radiation_rain->E[1]);
	RCache.set_c("hud_radiation_rain_params", envdesc.m_fWindVolumeFalloutRadiation/**/, droplets_power_debug, 0, 0);
	RCache.set_Geometry(g_combine);
	RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 4, 0, 2);

#ifdef USE_DX11
	HW.pContext->CopyResource(rt_Generic_0->pTexture->surface_get(), dest_rt->pTexture->surface_get());
#endif
}
