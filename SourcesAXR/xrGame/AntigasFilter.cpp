////////////////////////////////////////////////////////////////////////////
//	Module 		: AntigasFilter.cpp
//	Created 	: 28.08.2022
//  Modified 	: 28.08.2022
//	Author		: Dance Maniac (M.F.S. Team)
//	Description : Antigas filter
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AntigasFilter.h"
#include "Actor.h"
#include "inventory.h"
#include "CustomOutfit.h"
#include "ActorHelmet.h"

CAntigasFilter::CAntigasFilter()
{
	m_iPortionsNum = -1;
	m_fCondition = 1.0f;
	m_physic_item = 0;
}

CAntigasFilter::~CAntigasFilter()
{
}

void CAntigasFilter::Load(LPCSTR section)
{
	inherited::Load(section);

	m_iPortionsNum = pSettings->r_s32(section, "eat_portions_num");
	m_fCondition = READ_IF_EXISTS(pSettings, r_float, section, "filter_condition", 1.0f);
	VERIFY(m_iPortionsNum < 10000);
}

BOOL CAntigasFilter::net_Spawn(CSE_Abstract* DC)
{
	if (!inherited::net_Spawn(DC)) return FALSE;

	return TRUE;
};

bool CAntigasFilter::Useful() const
{
	if (!inherited::Useful()) return false;

	//проверить не все ли еще съедено
	if (m_iPortionsNum == 0) return false;

	CCustomOutfit* outfit = smart_cast<CCustomOutfit*>(Actor()->inventory().ItemFromSlot(OUTFIT_SLOT));
	CHelmet* helmet = smart_cast<CHelmet*>(Actor()->inventory().ItemFromSlot(HELMET_SLOT));

	if (outfit || helmet)
	{
		if (outfit && outfit->m_bUseFilter && outfit->m_fFilterCondition <= 0.99f)
			return true;
		else if (helmet && helmet->m_bUseFilter && helmet->m_fFilterCondition <= 0.99f)
			return true;
		else
			return false;
	}
	else
		return false;
}

bool CAntigasFilter::UseBy(CEntityAlive* entity_alive)
{
	if (!inherited::Useful()) return false;

	CCustomOutfit* outfit = smart_cast<CCustomOutfit*>(Actor()->inventory().ItemFromSlot(OUTFIT_SLOT));
	CHelmet* helmet = smart_cast<CHelmet*>(Actor()->inventory().ItemFromSlot(HELMET_SLOT));

	if (outfit && !helmet)
	{
		if (outfit->m_bUseFilter)
			ChangeInOutfit();
	}
	else if (!outfit && helmet)
	{
		if (helmet->m_bUseFilter)
			ChangeInHelmet();
	}
	else if (outfit && helmet)
	{
		float outfit_filter = outfit->m_fFilterCondition;
		float helmet_filter = helmet->m_fFilterCondition;
		if (outfit->m_bUseFilter && (outfit_filter < helmet_filter))
			ChangeInOutfit();
		else if (helmet->m_bUseFilter)
			ChangeInHelmet();
	}

	if (m_iPortionsNum > 0)
		--m_iPortionsNum;
	else
		m_iPortionsNum = 0;

	return true;
}

void CAntigasFilter::ChangeInOutfit()
{
	CCustomOutfit* outfit = smart_cast<CCustomOutfit*>(Actor()->inventory().ItemFromSlot(OUTFIT_SLOT));

	if (outfit)
		outfit->FilterReplace(m_fCondition);

	//Msg("Battery Charge is: %f", m_fBatteryChargeLevel); //Для тестов
}

void CAntigasFilter::ChangeInHelmet()
{
	CHelmet* helmet = smart_cast<CHelmet*>(Actor()->inventory().ItemFromSlot(HELMET_SLOT));

	if (helmet)
		helmet->FilterReplace(m_fCondition);

	//Msg("Battery Charge is: %f", m_fBatteryChargeLevel); //Для тестов
}

void CAntigasFilter::ChangeFilterCondition(float val)
{
	m_fCondition += val;
	clamp(m_fCondition, 0.f, 1.f);
}

float CCustomOutfit::GetFilterCondition() const
{
	return m_fFilterCondition;
}