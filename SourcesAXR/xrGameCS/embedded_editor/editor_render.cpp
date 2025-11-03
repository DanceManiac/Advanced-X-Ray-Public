#include "stdafx.h"
#include "editor_render.h"

#include "Level.h"
#include "ai_space.h"
#include "level_graph.h"
#include "level_changer.h"
#include "space_restrictor.h"
#include "ClimableObject.h"
#include "debug_renderer.h"

extern bool isRenderAiMap;
extern bool isRenderSpawnElement;

void embedded_editor_render()
{
    if (isRenderAiMap)
    {
        if (ai().get_level_graph())
            ai().level_graph().render();
    }

    if (isRenderSpawnElement)
    {
        for (u32 I = 0; I < Level().Objects.o_count(); I++)
        {
            auto _O = Level().Objects.o_get_by_iterator(I);
            auto space_restrictor = smart_cast<CSpaceRestrictor*>(_O);
            if (space_restrictor)
                space_restrictor->OnRender();

            auto level_changer = smart_cast<CLevelChanger*>(_O);
            if (level_changer)
                level_changer->OnRender();

            auto* climable = smart_cast<CClimableObject*>(_O);
            if (climable)
                climable->OnRender();
        }
    }
}