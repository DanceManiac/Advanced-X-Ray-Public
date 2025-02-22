// UI3dStatic.h: класс статического элемента, который рендерит
// 3d объект в себя
//////////////////////////////////////////////////////////////////////
#pragma once
//#include "ui/uiwindow.h"
#include "ui/UIStatic.h"
#include "inventory_item.h"
#include "inventory_space.h"
#include "Weapon.h"

class CUI3dStatic : public CUIStatic
{
    typedef CUIWindow inherited;
public:
    CUI3dStatic();
    virtual ~CUI3dStatic();
    virtual void Draw();
    void SetScale(float s) { m_fItemScale = s; }
    float GetScale() { return m_fItemScale; }
    //прорисовка окна
    void DrawStatic();
    void SetGameObject(PIItem pInvItem);
    void ModelDelete();
    void SetupDefault();
    pcstr GetDebugType() override { return "CUI3dStatic"; }
    void FillDebugInfo() override;
private:
    float m_fItemScale;
    PIItem pItem{};
    IKinematics* m_pCurrentVisual{};
    shared_str m_sVisualName;
    Fsphere m_sphere;
    Fmatrix m_trans;
    Fmatrix m_offset;
    Fvector m_vRotOffset;
    Fvector m_vRotPerFrame;
    void ModelCreate();
    //перевод из координат экрана в координаты той плоскости
    //где находиться объект
    void FromScreenToItem(float x_screen, float y_screen, float& x_item, float& y_item);
    float GetScaleX() { return float(::Render->getTarget()->get_width()) / float(UI_BASE_WIDTH); }
    float GetScaleY() { return float(::Render->getTarget()->get_height()) / float(UI_BASE_HEIGHT); }
    shared_str m_sAnimName;
    bool m_bIsWeapon{};
    Fvector m_vboxCenter{};
    float m_fboxRadius{0.5};
    void PlayItemAnimation();
    bool m_bAnimPlayed{};
};