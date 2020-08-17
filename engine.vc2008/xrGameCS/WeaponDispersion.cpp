// WeaponDispersion.cpp: ������ ��� ��������
// 
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "weapon.h"
#include "inventoryowner.h"
#include "actor.h"
#include "inventory_item_impl.h"

#include "actoreffector.h"
#include "effectorshot.h"
#include "EffectorShotX.h"


//���������� 1, ���� ������ � �������� ��������� � >1 ���� ����������
float CWeapon::GetConditionDispersionFactor() const
{
	return (1.f + fireDispersionConditionFactor*(1.f-GetCondition()));
}

float CWeapon::GetFireDispersion	(bool with_cartridge) 
{
	if (!with_cartridge) return GetFireDispersion(1.0f);
	if (!m_magazine.empty()) m_fCurrentCartirdgeDisp = m_magazine.back().param_s.kDisp;
	return GetFireDispersion	(m_fCurrentCartirdgeDisp);
}

//������� ��������� (� ��������) ������ � ������ ������������� �������
float CWeapon::GetFireDispersion	(float cartridge_k) 
{
	//���� ������� ���������, ��������� ������ � �������� �������
	float fire_disp = fireDispersionBase * cur_silencer_koef.fire_dispersion * cartridge_k * GetConditionDispersionFactor();
	
	//��������� ���������, �������� ����� ��������
	const CInventoryOwner* pOwner	=	smart_cast<const CInventoryOwner*>(H_Parent());
	VERIFY (pOwner);

	float parent_disp = pOwner->GetWeaponAccuracy();
	fire_disp += parent_disp;

	return fire_disp;
}


//////////////////////////////////////////////////////////////////////////
// ��� ������� ������ ������
void CWeapon::AddShotEffector		()
{
	inventory_owner().on_weapon_shot_start	(this);
}

void  CWeapon::RemoveShotEffector	()
{
	CInventoryOwner* pInventoryOwner = smart_cast<CInventoryOwner*>(H_Parent());
	if (pInventoryOwner)
		pInventoryOwner->on_weapon_shot_remove	(this);
}

void	CWeapon::ClearShotEffector	()
{
	CInventoryOwner* pInventoryOwner = smart_cast<CInventoryOwner*>(H_Parent());
	if (pInventoryOwner)
		pInventoryOwner->on_weapon_hide	(this);
}

void	CWeapon::StopShotEffector	()
{
	CInventoryOwner* pInventoryOwner = smart_cast<CInventoryOwner*>(H_Parent());
	if (pInventoryOwner)
		pInventoryOwner->on_weapon_shot_stop();
}
