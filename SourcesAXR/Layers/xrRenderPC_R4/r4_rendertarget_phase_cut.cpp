#include "stdafx.h"

void CRenderTarget::phase_cut()
{
	if (!RImplementation.o.dx10_msaa)
	{
		HW.pContext->ClearDepthStencilView(HW.pBaseZB, D3D_CLEAR_DEPTH | D3D_CLEAR_STENCIL, 0.0f, 0);
	}
	else
	{
		HW.pContext->ClearDepthStencilView(rt_MSAADepth->pZRT, D3D_CLEAR_DEPTH | D3D_CLEAR_STENCIL, 0.0f, 0);
		HW.pContext->ClearDepthStencilView(HW.pBaseZB, D3D_CLEAR_DEPTH | D3D_CLEAR_STENCIL, 0.0f, 0);
	}
	u_setrt(rt_temp_without_samples, 0, HW.pBaseZB);

	u32 Offset;
	u32 C = color_rgba(255, 255, 255, 255);

	float _w = float(Device.dwWidth);
	float _h = float(Device.dwHeight);

	Fvector2 p0, p1;

	p0.set(.5f / _w, .5f / _h);
	p1.set((_w + .5f) / _w, (_h + .5f) / _h);

	float d_Z = 1.f, d_W = 1.f;

	// Fill vertex buffer
	FVF::TL* pv = (FVF::TL*) RCache.Vertex.Lock(4, g_combine->vb_stride, Offset);

	pv->set(EPS, float(_h + EPS), d_Z, d_W, C, p0.x, p1.y);	pv++;
	pv->set(EPS, EPS, d_Z, d_W, C, p0.x, p0.y); pv++;
	pv->set(float(_w + EPS), float(_h + EPS), d_Z, d_W, C, p1.x, p1.y);   pv++;
	pv->set(float(_w + EPS), EPS, d_Z, d_W, C, p1.x, p0.y);  pv++;

	RCache.Vertex.Unlock(4, g_combine->vb_stride);

	RCache.set_Element(s_cut->E[0]);
	RCache.set_Geometry(g_combine);

	// draw
	StateManager.SetDepthFunc(D3DCMP_ALWAYS);
	StateManager.SetDepthEnable(TRUE);
	RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 4, 0, 2);
	if (RImplementation.o.dx10_msaa)
	{
		u_setrt(rt_temp, 0, rt_MSAADepth->pZRT);
		RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 4, 0, 2);
	}
}
