#include "stdafx.h"

void CRenderTarget::RenderScreenQuad(u32 w, u32 h, ID3DRenderTargetView* rt, ref_selement& sh, xr_unordered_map<LPCSTR, Fvector4*>* consts)
{
    u32 Offset = 0;
    const float d_Z = EPS_S;
    const float d_W = 1.0f;
    const u32 C = color_rgba(0, 0, 0, 255);

    if (rt)
        u_setrt(w, h, rt, nullptr, nullptr, HW.pBaseZB);

    RCache.set_CullMode(CULL_NONE);
    RCache.set_Stencil(FALSE);

    // Half-pixel offset (DX9 only)
#ifdef USE_DX11
    constexpr Fvector2 p0{0.0f, 0.0f}, p1{1.0f, 1.0f};
#else
    Fvector2 p0, p1;
    p0.set(0.5f / w, 0.5f / h);
    p1.set((w + 0.5f) / w, (h + 0.5f) / h);
#endif

    FVF::TL* pv = (FVF::TL*)RCache.Vertex.Lock(4, g_combine->vb_stride, Offset);
    pv->set(0, float(h), d_Z, d_W, C, p0.x, p1.y);
    pv++;
    pv->set(0, 0, d_Z, d_W, C, p0.x, p0.y);
    pv++;
    pv->set(float(w), float(h), d_Z, d_W, C, p1.x, p1.y);
    pv++;
    pv->set(float(w), 0, d_Z, d_W, C, p1.x, p0.y);
    pv++;
    RCache.Vertex.Unlock(4, g_combine->vb_stride);

    RCache.set_Element(sh);

    if (consts)
        for (const auto& [k, v] : *consts)
            RCache.set_c(k, *v);

    RCache.set_Geometry(g_combine);
    RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 4, 0, 2);
}

void CRenderTarget::RenderScreenQuad(u32 w, u32 h, ref_rt& rt, ref_selement& sh, xr_unordered_map<LPCSTR, Fvector4*>* consts)
{
    RenderScreenQuad(w, h, rt ? rt->pRT : nullptr, sh, consts);
}
