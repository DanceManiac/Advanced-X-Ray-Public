#include "stdafx.h"

void CRenderTarget::RenderScreenQuad(u32 w, u32 h, ID3DRenderTargetView* rt, ref_selement &sh, xr_unordered_map<LPCSTR, Fvector4*>* consts)
{
	u32 Offset = 0;
	float d_Z = EPS_S;
	float d_W = 1.0f;
	u32	C = color_rgba(0, 0, 0, 255);

	// Half-pixel offset
	Fvector2 p0, p1;
	p0.set(0.5f / w, 0.5f / h);
	p1.set((w + 0.5f) / w, (h + 0.5f) / h);

	if (rt)
		u_setrt(w, h, rt, nullptr, nullptr, nullptr);

	RCache.set_Stencil(FALSE);
	RCache.set_CullMode(CULL_NONE);

	FVF::TL* pv = (FVF::TL*)RCache.Vertex.Lock(4, g_combine->vb_stride, Offset);
	pv->set(0, h, d_Z, d_W, C, p0.x, p1.y); pv++;
	pv->set(0, 0, d_Z, d_W, C, p0.x, p0.y); pv++;
	pv->set(w, h, d_Z, d_W, C, p1.x, p1.y); pv++;
	pv->set(w, 0, d_Z, d_W, C, p1.x, p0.y); pv++;
	RCache.Vertex.Unlock(4, g_combine->vb_stride);

	RCache.set_Element(sh);
	if (consts)
	{
		for (const auto &C : *consts)
			RCache.set_c(C.first, *C.second);
	}
	RCache.set_Geometry(g_combine);
	RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 4, 0, 2);
}

void CRenderTarget::RenderScreenQuad(u32 w, u32 h, ref_rt &rt, ref_selement &sh, xr_unordered_map<LPCSTR, Fvector4*>* consts)
{
	RenderScreenQuad(w, h, rt ? rt->pRT : nullptr, sh, consts);
}