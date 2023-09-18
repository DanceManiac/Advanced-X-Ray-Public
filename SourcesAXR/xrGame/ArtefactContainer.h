#pragma once

#include "inventory_item_object.h"

class CArtefactContainer : public CInventoryItemObject
{
	typedef CInventoryItemObject inherited;

protected:
	size_t					m_iContainerSize;
	xr_vector<CArtefact*>	m_sArtefactsInside;

public:

	CArtefactContainer(void);
	virtual ~CArtefactContainer(void);

	virtual void			Load						(LPCSTR section);
	virtual BOOL			net_Spawn					(CSE_Abstract* DC);

	virtual void			save						(NET_Packet& output_packet);
	virtual void			load						(IReader& input_packet);

	virtual	u32				Cost						() const;
	virtual float			Weight						() const;

	size_t					GetContainerSize			() const { return m_iContainerSize; }
	void					SetContainerSize			(size_t new_size) { m_iContainerSize = new_size; }
	xr_vector<CArtefact*>	GetArtefactsInside			() { return m_sArtefactsInside; }
	bool					IsFull						() const { return m_sArtefactsInside.size() >= m_iContainerSize; }

	void					PutArtefactToContainer		(const CArtefact& artefact);
	void					TakeArtefactFromContainer	(CArtefact* artefact);
};
