#include "stdafx.h"
#include "editor_render.h"
#include "editor_render_ai_map.h"
#include "editor_render_spawn_element.h"

extern bool isRenderAiMap;
extern bool isRenderSpawnElement;

void embedded_editor_render()
{
    if (isRenderAiMap)
        renderAiMap();
    if (isRenderSpawnElement)
        renderSpawnElements();
}