#include "stdafx.h"

void CRenderTarget::phase_hud_blood()
{
	u32			Offset = 0;
	Fvector2	p0, p1;

	float ActorHealth = g_pGamePersistent->GetActorHealth();
	float ActorMaxHealth = g_pGamePersistent->GetActorMaxHealth();

	struct v_aa {
		Fvector4	p;
		Fvector2	uv0;
		Fvector2	uv1;
		Fvector2	uv2;
		Fvector2	uv3;
		Fvector2	uv4;
		Fvector4	uv5;
		Fvector4	uv6;
	};

	float	_w = float(Device.dwWidth);
	float	_h = float(Device.dwHeight);
	float	ddw = 1.f / _w;
	float	ddh = 1.f / _h;

	p0.set(.5f / _w, .5f / _h);
	p1.set((_w + .5f) / _w, (_h + .5f) / _h);

	// Set RT's
#if defined(USE_DX10) || defined(USE_DX11)
	ref_rt dest_rt = RImplementation.o.dx10_msaa ? rt_Generic : rt_Color;
	u_setrt(dest_rt, 0, 0, HW.pBaseZB);
#else
	u_setrt(rt_Generic_0, 0, 0, HW.pBaseZB);
#endif

	RCache.set_CullMode(CULL_NONE);
	RCache.set_Stencil(FALSE);

	// Fill vertex buffer
	v_aa* pv = (v_aa*)RCache.Vertex.Lock(4, g_aa_AA->vb_stride, Offset);
	pv->p.set(EPS, float(_h + EPS), EPS, 1.f); pv->uv0.set(p0.x, p1.y); pv->uv1.set(p0.x - ddw, p1.y - ddh); pv->uv2.set(p0.x + ddw, p1.y + ddh); pv->uv3.set(p0.x + ddw, p1.y - ddh); pv->uv4.set(p0.x - ddw, p1.y + ddh); pv->uv5.set(p0.x - ddw, p1.y, p1.y, p0.x + ddw); pv->uv6.set(p0.x, p1.y - ddh, p1.y + ddh, p0.x); pv++;
	pv->p.set(EPS, EPS, EPS, 1.f); pv->uv0.set(p0.x, p0.y); pv->uv1.set(p0.x - ddw, p0.y - ddh); pv->uv2.set(p0.x + ddw, p0.y + ddh); pv->uv3.set(p0.x + ddw, p0.y - ddh); pv->uv4.set(p0.x - ddw, p0.y + ddh); pv->uv5.set(p0.x - ddw, p0.y, p0.y, p0.x + ddw); pv->uv6.set(p0.x, p0.y - ddh, p0.y + ddh, p0.x); pv++;
	pv->p.set(float(_w + EPS), float(_h + EPS), EPS, 1.f); pv->uv0.set(p1.x, p1.y); pv->uv1.set(p1.x - ddw, p1.y - ddh); pv->uv2.set(p1.x + ddw, p1.y + ddh); pv->uv3.set(p1.x + ddw, p1.y - ddh); pv->uv4.set(p1.x - ddw, p1.y + ddh); pv->uv5.set(p1.x - ddw, p1.y, p1.y, p1.x + ddw); pv->uv6.set(p1.x, p1.y - ddh, p1.y + ddh, p1.x); pv++;
	pv->p.set(float(_w + EPS), EPS, EPS, 1.f); pv->uv0.set(p1.x, p0.y); pv->uv1.set(p1.x - ddw, p0.y - ddh); pv->uv2.set(p1.x + ddw, p0.y + ddh); pv->uv3.set(p1.x + ddw, p0.y - ddh); pv->uv4.set(p1.x - ddw, p0.y + ddh); pv->uv5.set(p1.x - ddw, p0.y, p0.y, p1.x + ddw); pv->uv6.set(p1.x, p0.y - ddh, p0.y + ddh, p1.x); pv++;
	RCache.Vertex.Unlock(4, g_aa_AA->vb_stride);

	// Draw COLOR
	RCache.set_Element(s_hud_blood->E[0]);
	RCache.set_c("hud_blood_params", ActorMaxHealth-ActorHealth, 0, 0, 0);
	RCache.set_Geometry(g_aa_AA);
	RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 4, 0, 2);

#if defined(USE_DX10) || defined(USE_DX11)
	HW.pContext->CopyResource(rt_Generic_0->pTexture->surface_get(), dest_rt->pTexture->surface_get());
#endif
}

void CRenderTarget::phase_hud_power()
{
	u32			Offset = 0;
	Fvector2	p0, p1;

	float ActorCurrentPower = g_pGamePersistent->GetActorPower();
	float ActorMaxPower = g_pGamePersistent->GetActorMaxPower();

	struct v_aa {
		Fvector4	p;
		Fvector2	uv0;
		Fvector2	uv1;
		Fvector2	uv2;
		Fvector2	uv3;
		Fvector2	uv4;
		Fvector4	uv5;
		Fvector4	uv6;
	};

	float	_w = float(Device.dwWidth);
	float	_h = float(Device.dwHeight);
	float	ddw = 1.f / _w;
	float	ddh = 1.f / _h;

	p0.set(.5f / _w, .5f / _h);
	p1.set((_w + .5f) / _w, (_h + .5f) / _h);

	// Set RT's
#if defined(USE_DX10) || defined(USE_DX11)
	ref_rt dest_rt = RImplementation.o.dx10_msaa ? rt_Generic : rt_Color;
	u_setrt(dest_rt, 0, 0, HW.pBaseZB);
#else
	u_setrt(rt_Generic_0, 0, 0, HW.pBaseZB);
#endif

	RCache.set_CullMode(CULL_NONE);
	RCache.set_Stencil(FALSE);

	// Fill vertex buffer
	v_aa* pv = (v_aa*)RCache.Vertex.Lock(4, g_aa_AA->vb_stride, Offset);
	pv->p.set(EPS, float(_h + EPS), EPS, 1.f); pv->uv0.set(p0.x, p1.y); pv->uv1.set(p0.x - ddw, p1.y - ddh); pv->uv2.set(p0.x + ddw, p1.y + ddh); pv->uv3.set(p0.x + ddw, p1.y - ddh); pv->uv4.set(p0.x - ddw, p1.y + ddh); pv->uv5.set(p0.x - ddw, p1.y, p1.y, p0.x + ddw); pv->uv6.set(p0.x, p1.y - ddh, p1.y + ddh, p0.x); pv++;
	pv->p.set(EPS, EPS, EPS, 1.f); pv->uv0.set(p0.x, p0.y); pv->uv1.set(p0.x - ddw, p0.y - ddh); pv->uv2.set(p0.x + ddw, p0.y + ddh); pv->uv3.set(p0.x + ddw, p0.y - ddh); pv->uv4.set(p0.x - ddw, p0.y + ddh); pv->uv5.set(p0.x - ddw, p0.y, p0.y, p0.x + ddw); pv->uv6.set(p0.x, p0.y - ddh, p0.y + ddh, p0.x); pv++;
	pv->p.set(float(_w + EPS), float(_h + EPS), EPS, 1.f); pv->uv0.set(p1.x, p1.y); pv->uv1.set(p1.x - ddw, p1.y - ddh); pv->uv2.set(p1.x + ddw, p1.y + ddh); pv->uv3.set(p1.x + ddw, p1.y - ddh); pv->uv4.set(p1.x - ddw, p1.y + ddh); pv->uv5.set(p1.x - ddw, p1.y, p1.y, p1.x + ddw); pv->uv6.set(p1.x, p1.y - ddh, p1.y + ddh, p1.x); pv++;
	pv->p.set(float(_w + EPS), EPS, EPS, 1.f); pv->uv0.set(p1.x, p0.y); pv->uv1.set(p1.x - ddw, p0.y - ddh); pv->uv2.set(p1.x + ddw, p0.y + ddh); pv->uv3.set(p1.x + ddw, p0.y - ddh); pv->uv4.set(p1.x - ddw, p0.y + ddh); pv->uv5.set(p1.x - ddw, p0.y, p0.y, p1.x + ddw); pv->uv6.set(p1.x, p0.y - ddh, p0.y + ddh, p1.x); pv++;
	RCache.Vertex.Unlock(4, g_aa_AA->vb_stride);

	// Draw COLOR
	RCache.set_Element(s_hud_power->E[0]);
	RCache.set_c("hud_power_params", ActorMaxPower - ActorCurrentPower, 0, 0, 0);
	RCache.set_Geometry(g_aa_AA);
	RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 4, 0, 2);

#if defined(USE_DX10) || defined(USE_DX11)
	HW.pContext->CopyResource(rt_Generic_0->pTexture->surface_get(), dest_rt->pTexture->surface_get());
#endif
}
