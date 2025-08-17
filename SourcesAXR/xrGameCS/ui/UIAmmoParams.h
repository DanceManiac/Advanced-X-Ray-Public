#pragma once
#include "UIWindow.h"
#include "..\..\xrServerEntities\alife_space.h"
#include "../WeaponAmmo.h"
#include "../inventory.h"
#include "../inventory_item.h"


class CUIXml;
class CUIStatic;
class UIAmmoParamItem;
class CWeaponAmmo;

class CUIAmmoParams : public CUIWindow
{
public:
	CUIAmmoParams();
	virtual ~CUIAmmoParams();
	void InitFromXml(CUIXml& xml);
	void SetInfo(CWeaponAmmo* ammo);

protected:
	UIAmmoParamItem* m_ammo_box_size;
	UIAmmoParamItem* m_ammo_dist;
	UIAmmoParamItem* m_ammo_hit_prc;
	UIAmmoParamItem* m_ammo_ap;
//	UIAmmoParamItem* m_ammo_speed;
	UIAmmoParamItem* m_ammo_disp;
	UIAmmoParamItem* m_ammo_buck_shot;
	UIAmmoParamItem* m_ammo_impair;
	UIAmmoParamItem* m_ammo_tracer;

	CUIStatic* m_Prop_line;

}; // class CUIAmmoParams

// -----------------------------------

class UIAmmoParamItem : public CUIWindow
{
public:
	UIAmmoParamItem();
	virtual ~UIAmmoParamItem();

	void Init(CUIXml& xml, LPCSTR section);
	void SetCaption(LPCSTR name);
	void SetValue(float value, float curr = -1.f);

private:
	CUIStatic* m_caption;
	CUIStatic* m_value;
	float m_magnitude;
	float m_accuracy;
	bool m_show_sign;
	bool m_sign_inverse;
	bool m_colorize;
	shared_str m_unit_str;
	shared_str m_unit_str2;
	shared_str m_texture;

}; // class UIAmmoParamItem
