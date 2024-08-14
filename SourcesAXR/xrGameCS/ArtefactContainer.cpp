////////////////////////////////////////////////////////////////////////////
//	Module 		: ArtefactContainer.cpp
//	Created 	: 08.05.2023
//  Modified 	: 19.09.2023
//	Author		: Dance Maniac (M.F.S. Team)
//	Description : Artefact container
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ArtefactContainer.h"
#include "Artefact.h"
#include "level.h"
#include "Actor.h"

float af_from_container_charge_level = 1.0f;
int af_from_container_rank = 1;
CArtefactContainer* m_LastAfContainer = nullptr;

CArtefactContainer::CArtefactContainer()
{
	m_iContainerSize = 1;
	m_sArtefactsInside.clear();
}

CArtefactContainer::~CArtefactContainer()
{
}

void CArtefactContainer::Load(LPCSTR section)
{
	inherited::Load(section);

	m_iContainerSize = pSettings->r_s32(section, "container_size");
}

BOOL CArtefactContainer::net_Spawn(CSE_Abstract* DC)
{
	return		(inherited::net_Spawn(DC));
}

void CArtefactContainer::save(NET_Packet& packet)
{
	inherited::save(packet);

	u32 numArtefacts = m_sArtefactsInside.size();
	save_data(numArtefacts, packet);

	for (const auto& artefact : m_sArtefactsInside)
	{
		shared_str section = artefact->cNameSect();
		save_data(section, packet);

		artefact->save(packet);
	}
}

void CArtefactContainer::load(IReader& packet)
{
	inherited::load(packet);

	u32 numArtefacts;
	load_data(numArtefacts, packet);

	m_sArtefactsInside.clear();

	for (u32 i = 0; i < numArtefacts; ++i)
	{
		CArtefact* artefact = xr_new<CArtefact>();
		shared_str section;

		load_data(section, packet);

		artefact->Load(section.c_str());

		artefact->load(packet);

		m_sArtefactsInside.push_back(artefact);
	}
}

void CArtefactContainer::PutArtefactToContainer(const CArtefact& artefact)
{
	CArtefact* af = xr_new<CArtefact>(artefact);

	af->m_bInContainer = true;

	m_sArtefactsInside.push_back(af);
}

void CArtefactContainer::TakeArtefactFromContainer(CArtefact* artefact)
{
	for (auto it = m_sArtefactsInside.begin(); it != m_sArtefactsInside.end();)
	{
		if (*it == artefact)
		{
			CArtefact* item_to_spawn = smart_cast<CArtefact*>(*it);
			it = m_sArtefactsInside.erase(it);

			Level().spawn_item(item_to_spawn->cNameSect().c_str(), Actor()->Position(), false, Actor()->ID());

			af_from_container_charge_level = artefact->GetCurrentChargeLevel();
			af_from_container_rank = artefact->GetCurrentAfRank();
			m_LastAfContainer = this;

			return;
		}
		++it;
	}
}

u32 CArtefactContainer::Cost() const
{
	u32 res = CInventoryItem::Cost();

	for (const auto& artefact : m_sArtefactsInside)
		res += artefact->Cost();

	return res;
}

float CArtefactContainer::Weight() const
{
	float res = CInventoryItemObject::Weight();

	for (const auto& artefact : m_sArtefactsInside)
		res += artefact->Weight();

	return res;
}