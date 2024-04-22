#include "stdafx.h"
#include "r4_rendertarget.h"

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

#include "../../xrEngine/xr_efflensflare.h"

#define BLEND_INC_SPEED 8.0f
#define BLEND_DEC_SPEED 4.0f

extern u32 reset_frame;

BOOL RayPick_LF(const Fvector& s, const Fvector& d, float& range, collide::rq_target tgt)
{
    BOOL bRes = TRUE;
    collide::rq_result RQ;
    CObject* E = g_pGameLevel->CurrentViewEntity();
    bRes = g_pGameLevel->ObjectSpace.RayPick(s, d, range, tgt, RQ, E);

    if (bRes)
        range = RQ.range;

    return bRes;
}

void CRenderTarget::phase_flares()
{
    if (!rt_flares)
        return;

    // Targets
    if (dwFlareClearMark == Device.dwFrame)
        u_setrt(rt_flares, NULL, NULL, HW.pBaseZB);
    else
    {
        // initial setup
        dwFlareClearMark = Device.dwFrame;

        // clear
        u_setrt(rt_flares, NULL, NULL, HW.pBaseZB);
        FLOAT ColorRGBA[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        HW.pContext->ClearRenderTargetView(rt_flares->pRT, ColorRGBA);
    }
}

void CRenderTarget::render_flare(light* L)
{
    if (!rt_flares)
        return;
    //	if (reset_frame==Device.dwFrame || reset_frame==Device.dwFrame - 1)		return;
    phase_flares();

    Fvector vLightDir, vecX, vecY, vecSx, vecSy;
    vLightDir.sub(L->position, Device.vCameraPosition);

    // Calculate the point directly in front of us, on the far clip plane
    float fDistance = vLightDir.magnitude();

    Fmatrix matEffCamPos;
    matEffCamPos.identity();

    // Calculate our position and direction
    matEffCamPos.i.set(Device.vCameraRight);
    matEffCamPos.j.set(Device.vCameraTop);
    matEffCamPos.k.set(Device.vCameraDirection);

    Fvector vecDir = { 0.0f, 0.0f, 1.0f };
    matEffCamPos.transform_dir(vecDir);
    vecDir.normalize();

    // Figure out if light is behind something else
    vecX.set(1.0f, 0.0f, 0.0f);
    matEffCamPos.transform_dir(vecX);
    vecX.normalize();
    R_ASSERT(_valid(vecX));

    vecY.crossproduct(vecX, vecDir);
    R_ASSERT(_valid(vecY));

    vLightDir.normalize();
    float cur_dist = fDistance;
    RayPick_LF(Device.vCameraPosition, vLightDir, cur_dist, collide::rqtBoth);
    float vis = ((fDistance - cur_dist) > 0.2f) ? 0.0f : 1.0f;

    blend_lerp(L->fBlend, vis, BLEND_DEC_SPEED, Device.fTimeDelta);
    //	Msg("max_dist = %f, fDistance = %f, vis =%f, fBlend = %f", max_dist,fDistance,vis,L->fBlend);
    clamp(L->fBlend, 0.0f, 1.0f);

    vLightDir.normalize();

    if (L->fBlend <= EPS_L)
        return;

    // Figure out of light (or flare) might be visible
    Fvector vecLight = L->position;

    float fDot = vLightDir.dotproduct(vecDir);

    if (fDot <= 0.01f)
        return;

    Fvector scr_pos;
    Device.mFullTransform.transform(scr_pos, vecLight);
    float kx = 1, ky = 1;
    float sun_blend = 0.5f;
    float sun_max = 2.5f;
    scr_pos.y *= -1.0;

    if (_abs(scr_pos.x) > sun_blend)
        kx = ((sun_max - (float)_abs(scr_pos.x))) / (sun_max - sun_blend);
    if (_abs(scr_pos.y) > sun_blend)
        ky = ((sun_max - (float)_abs(scr_pos.y))) / (sun_max - sun_blend);

    float fGradientValue = 0;

    if (!((_abs(scr_pos.x) > sun_max) || (_abs(scr_pos.y) > sun_max)))
        fGradientValue = kx * ky * L->fBlend;

    vecSx.mul(vecX, fGradientValue * (1.f + 0.02f * fDistance) * 3.f);
    vecSy.mul(vecY, fGradientValue * (1.f + 0.02f * fDistance) * 3.f);
    Fcolor color = L->color;
    color.mul_rgba(fGradientValue * L->fBlend);
    u32 c = color.get();
    u32 VS_Offset;
    FVF::LIT* pv = (FVF::LIT*)RCache.Vertex.Lock(4, g_flare.stride(), VS_Offset);

    pv->set(vecLight.x + vecSx.x - vecSy.x, vecLight.y + vecSx.y - vecSy.y, vecLight.z + vecSx.z - vecSy.z, c, 0, 0);
    pv++;
    pv->set(vecLight.x + vecSx.x + vecSy.x, vecLight.y + vecSx.y + vecSy.y, vecLight.z + vecSx.z + vecSy.z, c, 0, 1);
    pv++;
    pv->set(vecLight.x - vecSx.x - vecSy.x, vecLight.y - vecSx.y - vecSy.y, vecLight.z - vecSx.z - vecSy.z, c, 1, 0);
    pv++;
    pv->set(vecLight.x - vecSx.x + vecSy.x, vecLight.y - vecSx.y + vecSy.y, vecLight.z - vecSx.z + vecSy.z, c, 1, 1);
    pv++;

    RCache.Vertex.Unlock(4, g_flare.stride());

    RCache.set_xform_world(Fidentity);
    RCache.set_Geometry(g_flare);

    RCache.set_Stencil(FALSE);
    RCache.set_CullMode(CULL_NONE);

    float intensity = 0.0f;

    if (cur_dist > 5.0f)
        intensity = smoothstep(5.0f, 100.0f, cur_dist);
    else
        intensity = 1.0f - smoothstep(2.0f, 5.0f, cur_dist);

    RCache.set_Shader(s_flare);
    RCache.set_c("flare_params", intensity, L->flags.bFlare ? 1.f : 0.f, 0.f, 0.f);
    RCache.set_c("flare_color", L->color.r, L->color.g, L->color.b, L->color.a);
    RCache.Render(D3DPT_TRIANGLELIST, VS_Offset, 0, 4, 0, 2);
}