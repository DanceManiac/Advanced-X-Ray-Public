#include "stdafx.h"

void CRenderTarget::phase_rain()
{
   if( !RImplementation.o.dx10_msaa )
      u_setrt	(rt_Color,NULL,NULL,HW.pBaseZB);
   else
      u_setrt	(rt_Color,NULL,NULL,rt_MSAADepth->pZRT);
	//u_setrt	(rt_Normal,NULL,NULL,HW.pBaseZB);
	RImplementation.rmNormal();
}

void CRenderTarget::phase_ssfx_rain()
{
	//Constants
	u32 Offset = 0;
	u32 C = color_rgba(0, 0, 0, 255);

	float w = float(Device.dwWidth);
	float h = float(Device.dwHeight);

	set_viewport_size(HW.pContext, w / 8.0f, h / 8.0f);

	u_setrt(rt_ssfx_rain, 0, 0, NULL);
	RCache.set_CullMode(CULL_NONE);
	RCache.set_Stencil(FALSE);

	// Fill vertex buffer
	FVF::TL* pv = (FVF::TL*)RCache.Vertex.Lock(4, g_combine->vb_stride, Offset);
	pv->set(0, h, EPS_S, 1.0f, C, 0.0f, 1.0f); pv++;
	pv->set(0, 0, EPS_S, 1.0f, C, 0.0f, 0.0f); pv++;
	pv->set(w, h, EPS_S, 1.0f, C, 1.0f, 1.0f); pv++;
	pv->set(w, 0, EPS_S, 1.0f, C, 1.0f, 0.0f); pv++;
	RCache.Vertex.Unlock(4, g_combine->vb_stride);

	// Draw COLOR
	RCache.set_Element(!RImplementation.o.dx10_msaa ? s_ssfx_rain->E[0] : s_ssfx_rain->E[1]);
	RCache.set_Geometry(g_combine);
	RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 4, 0, 2);

	set_viewport_size(HW.pContext, w, h);
}