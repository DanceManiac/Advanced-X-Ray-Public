#include "stdafx.h"
#include "editor_render_spawn_element.h"
#include "../ai_space.h"
#include "../alife_object_registry.h"
#include "../alife_simulator.h"
#include "../Include/xrRender/UIRender.h"
#include "../Include/xrRender/UIShader.h"
#include "ui_defs.h"

typedef void (*RenderFunc)(CSE_ALifeObject*);
void renderLevelChanger(CSE_ALifeLevelChanger* lc);

void renderSpawnElements()
{
    auto alife = ai().get_alife();
    if (!alife)
        return;
    auto& registry = alife->objects();
    for (auto el : registry.objects()) {
        auto lc = smart_cast<CSE_ALifeLevelChanger*>(el.second);
        if (lc)
            renderLevelChanger(lc);
    }
}

void renderLevelChanger(CSE_ALifeLevelChanger* lc)
{
    const u32 transpColor = color_rgba(200, 150, 110, 100);
    const u32 edgeColor = color_rgba(32, 32, 32, 255);

    Fvector zero = { 0.f, 0.f, 0.f };
    Fquaternion q;
    q.rotationYawPitchRoll(lc->angle());
    Fmatrix transform;
    transform.mk_xform(q, lc->position());

    UIRender->CacheSetCullMode(IUIRender::cmNONE);
    for (auto shape : lc->shapes) {
        switch (shape.type) {
        case CShapeData::cfSphere: {
            Fsphere& S = shape.data.sphere;
            Fmatrix B;
            B.scale(S.R, S.R, S.R);
            B.translate_over(S.P);
            B.mulA_43(transform);
            UIRender->CacheSetXformWorld(B);
            DU->DrawCross(zero, 1.f, edgeColor, false);
            DU->DrawIdentSphere(true, true, transpColor, edgeColor);
        } break;
        case CShapeData::cfBox: {
            Fmatrix B = shape.data.box;
            B.mulA_43(transform);
            UIRender->CacheSetXformWorld(B);
            DU->DrawIdentBox(true, true, transpColor, edgeColor);
        } break;
        }
    }
    UIRender->CacheSetCullMode(IUIRender::cmCCW);
}