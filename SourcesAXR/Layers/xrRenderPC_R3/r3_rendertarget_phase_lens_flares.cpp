#include "stdafx.h"
#include "r3_rendertarget.h"

void CRenderTarget::phase_lfx(int i)
{
	u32 phaseIndex{ static_cast<u32>(i) };

	if (phaseIndex >= m_miltaka_lfx_coords.size())
		return;

	u32 Offset = 0;
	const float	_w = float(Device.dwWidth);
	const float	_h = float(Device.dwHeight);
	const float	du = ps_r1_pps_u, dv = ps_r1_pps_v;

	FVF::V* pv = (FVF::V*)RCache.Vertex.Lock(4, g_lfx->vb_stride, Offset);
	pv->set(du + 0, dv + float(_h), 0, 0, 1);
	pv++;
	pv->set(du + 0, dv + 0, 0, 0, 0);
	pv++;
	pv->set(du + float(_w), dv + float(_h), 0, 1, 1);
	pv++;
	pv->set(du + float(_w), dv + 0, 0, 1, 0);
	pv++;
	RCache.Vertex.Unlock(4, g_lfx->vb_stride);

	// Draw COLOR
	RCache.set_c("m_lfx_coords", m_miltaka_lfx_coords[phaseIndex]);
	RCache.set_c("m_lfx_color", m_miltaka_lfx_color[phaseIndex]);
	RCache.set_Element(s_lfx->E[0]);
	RCache.set_Geometry(g_lfx);
	RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 4, 0, 2);
}