#pragma once

#include "inventory_item_object.h"

class ArtefactContainer : public CInventoryItemObject
{
	using inherited = CInventoryItemObject;

	ArtefactContainer();
	virtual ~ArtefactContainer();

	size_t					m_iContainerSize;
	xr_vector<shared_str>	m_sArtefactsInside;

public:
	virtual void		Load(LPCSTR section);
	virtual BOOL		net_Spawn(CSE_Abstract* DC);

	size_t				GetContainerSize() const {return m_iContainerSize;}
	void				SetContainerSize(size_t new_size) { m_iContainerSize = new_size; }
	void				PutArtefactToContainer(shared_str artefact);
	void				TakeArtefactFromContainer(shared_str artefact);
};
