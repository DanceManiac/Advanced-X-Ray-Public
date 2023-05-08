////////////////////////////////////////////////////////////////////////////
//	Module 		: ArtefactContainer.cpp
//	Created 	: 08.05.2023
//  Modified 	: 08.05.2023
//	Author		: Dance Maniac (M.F.S. Team)
//	Description : Artefact container
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ArtefactContainer.h"

ArtefactContainer::ArtefactContainer()
{
	m_iContainerSize = 1;
}

ArtefactContainer::~ArtefactContainer()
{
}

void ArtefactContainer::Load(LPCSTR section)
{
	inherited::Load(section);

	m_iContainerSize = pSettings->r_s32(section, "container_size");
}

BOOL ArtefactContainer::net_Spawn(CSE_Abstract* DC)
{
	if (!inherited::net_Spawn(DC)) return FALSE;

	return TRUE;
};

void ArtefactContainer::PutArtefactToContainer(shared_str artefact)
{
}

void ArtefactContainer::TakeArtefactFromContainer(shared_str artefact)
{
}