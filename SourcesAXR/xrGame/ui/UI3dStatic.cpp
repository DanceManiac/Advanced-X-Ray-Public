//////////////////////////////////////////////////////////////////////
//-- UI3dStatic.cpp: класс статического элемента, который рендерит
//-- 3d объект в себя
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "ui3dstatic.h"
#include "../Include/xrRender/Kinematics.h"

#include <imgui.h>

//расстояние от камеры до вещи, перед глазами
#define DIST (VIEWPORT_NEAR + 0.1f)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUI3dStatic::CUI3dStatic()
{
    m_fItemScale = 1.f;
    m_trans.identity();
    m_offset.identity();
    m_vRotPerFrame = READ_IF_EXISTS(pSettings, r_fvector3, "3d_ui_base", "rotation_per_frame", (Fvector{}));
}

CUI3dStatic::~CUI3dStatic() 
{ 
    ModelDelete(); 
}

void CUI3dStatic::SetupDefault()
{
    m_fItemScale = 1.f;
    m_bIsWeapon = false;
    m_bAnimPlayed = false;
    pItem = nullptr;
    ModelDelete();
}

void CUI3dStatic::ModelCreate()
{
    if (m_pCurrentVisual)
        ModelDelete();

    if (m_sVisualName.size())
    {
        m_pCurrentVisual = smart_cast<IKinematics*>(::Render->model_Create(m_sVisualName.c_str()));
        
        if (m_bIsWeapon)
        {
            Fbox& box = m_pCurrentVisual->dcast_RenderVisual()->getVisData().box;
            box.getcenter(m_vboxCenter);
            m_fboxRadius = box.getradius();
            Msg("box center = %f,%f,%f", VPUSH(m_vboxCenter));
            Msg("box radius = %f", m_fboxRadius);
        }
        else
            m_sphere = m_pCurrentVisual->dcast_RenderVisual()->getVisData().sphere;
    }
}

void CUI3dStatic::ModelDelete()
{
    if (m_pCurrentVisual)
    {
        IRenderVisual* v = m_pCurrentVisual->dcast_RenderVisual();
        ::Render->model_Delete(v, TRUE);
        m_pCurrentVisual = nullptr;
    }
}

void CUI3dStatic::FromScreenToItem(float x_screen, float y_screen, float& x_item, float& y_item)
{
    float x = x_screen;
    float y = y_screen;
    float halfwidth = (float)Device.dwWidth / 2.f;
    float halfheight = (float)Device.dwHeight / 2.f;
    float size_y = VIEWPORT_NEAR * tanf(deg2rad(5.f) * 0.5f);
    float size_x = size_y / (Device.fASPECT);
    float r_pt = float(x - halfwidth) * size_x / (float)halfwidth;
    float u_pt = float(halfheight - y) * size_y / (float)halfheight;
    x_item = r_pt * DIST / VIEWPORT_NEAR;
    y_item = u_pt * DIST / VIEWPORT_NEAR;
}

void CUI3dStatic::Draw()
{ 
    DrawStatic();
    inherited::Draw(); 
}

//прорисовка
void CUI3dStatic::DrawStatic()
{
    if (m_pCurrentVisual)
    {
        m_vRotOffset.add(m_vRotPerFrame);

        if (!m_bIsWeapon)
            m_sphere = m_pCurrentVisual->dcast_RenderVisual()->getVisData().sphere;

        if (auto wpn = smart_cast<CWeapon*>(pItem))
            wpn->UpdateAddonsVisibility(m_pCurrentVisual);

        Frect rect;
        GetAbsoluteRect(rect);
        // Apply scale
        rect.top = rect.top * GetScaleY();
        rect.left = rect.left * GetScaleX();
        rect.bottom = rect.bottom * GetScaleY();
        rect.right = rect.right * GetScaleX();
        Fmatrix translate_matrix;
        Fmatrix scale_matrix;
        m_trans.identity();

        //поместить объект в центр сферы
        translate_matrix.identity();
        Fvector tmp;

        if (m_bIsWeapon)
        {
            tmp.x = -m_vboxCenter.x;
            tmp.y = -m_vboxCenter.y;
            tmp.z = -m_vboxCenter.z;
        }
        else
        {
            tmp.x = -m_sphere.P.x;
            tmp.y = -m_sphere.P.y;
            tmp.z = -m_sphere.P.z;
        }

        translate_matrix.translate(VPUSH(tmp));
        m_trans.mulA_44(translate_matrix);
        
        m_offset.identity();
        Fvector ymp = m_vRotOffset;
        ymp.mul(PI / 180.f);
        m_offset.setHPB(ymp.x, ymp.y, ymp.z);
        m_trans.mulA_44(m_offset);
        float x1, y1, x2, y2;
        FromScreenToItem(rect.left, rect.top, x1, y1);
        FromScreenToItem(rect.right, rect.bottom, x2, y2);
        const float normal_size = _min(_abs(x2 - x1), _abs(y2 - y1));
        const float radius = m_bIsWeapon ? m_fboxRadius : m_sphere.R;
        const float scale = (normal_size / (radius * 2.f)) * m_fItemScale;

        scale_matrix.identity();
        scale_matrix.scale(scale, scale, scale);
        m_trans.mulA_44(scale_matrix);
        float right_item_offset, up_item_offset;
        ///////////////////////////////
        FromScreenToItem(
            rect.left + (GetWidth() / 2.f * GetScaleX()), 
            rect.top + (GetHeight() / 2.f * GetScaleY()), 
            right_item_offset, 
            up_item_offset
        );
        translate_matrix.identity();
        translate_matrix.translate(right_item_offset, up_item_offset, DIST);
        m_trans.mulA_44(translate_matrix);
        Fmatrix camera_matrix;
        camera_matrix.identity();
        camera_matrix = Device.mView;
        camera_matrix.invert();
        m_trans.mulA_44(camera_matrix);
        
        if (m_bIsWeapon)
        {
            if (!m_bAnimPlayed)
            {
                PlayItemAnimation();
                m_bAnimPlayed = true;
            } 
        }
        
        ::Render->set_UI(TRUE);
        
        ::Render->set_Transform(&m_trans);
        ::Render->add_Visual(m_pCurrentVisual->dcast_RenderVisual());
        ::Render->Render3DStatic();
        ::Render->set_UI(FALSE);
    }
}

void CUI3dStatic::SetGameObject(PIItem pInvItem)
{ 
    SetupDefault();
    pItem = pInvItem;
    
    const char* sect_name = pItem->object().cNameSect().c_str();
    m_sVisualName = pSettings->r_string(sect_name, "visual");
    m_fItemScale = READ_IF_EXISTS(pSettings, r_float, sect_name, "ui_3d_icon_scale", 1.f);
    m_vRotOffset = READ_IF_EXISTS(pSettings, r_fvector3, sect_name, "ui_3d_icon_rotation", (Fvector{0.f, 0.f, 0.f }));

    ModelCreate();
}

void CUI3dStatic::PlayItemAnimation()
{
    if (m_pCurrentVisual && m_bIsWeapon)
    {
        // TODO: Animation for Actor
        if (auto kA = m_pCurrentVisual->dcast_PKinematicsAnimated())
        {
            MotionID mid = kA->ID_Cycle_Safe(m_sAnimName.c_str());
            //-- anim lenght
            // float len = iFloor(0.5 + 1000.f * kA->LL_GetRootMotion(mid)->GetLength() / 1.f);
            u16 pc = kA->partitions().count();
            for (u16 pid = 0; pid < pc; ++pid)
            {
                CBlend* B = kA->PlayCycle(pid, mid);
                R_ASSERT(B);
                B->speed *= 1.f;
                // B->timeCurrent = CalculateMotionStartSeconds(anm->params.start_k, B->timeTotal);
            }
            m_pCurrentVisual->CalculateBones_Invalidate();
            m_pCurrentVisual->CalculateBones();
            //m_bAllowAnim = false;
            //m_uAmAnimLCurr = Device.dwTimeGlobal + m_uAMAnimLenght;
        }
    }
}

void CUI3dStatic::FillDebugInfo()
{
#ifndef MASTER_GOLD
    CUIStatic::FillDebugInfo();

    if (!ImGui::CollapsingHeader(CUI3dStatic::GetDebugType()))
        return;

    ImGui::DragFloat3("3d static rotation", (float*)&m_vRotOffset, 1.0f, NULL, NULL, "%.6f");
    ImGui::DragFloat("3d static scale", &m_fItemScale, 0.1f);
#endif
}