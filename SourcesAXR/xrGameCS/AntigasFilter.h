#pragma once

#include "eatable_item_object.h"

class CAntigasFilter : public CEatableItemObject
{
	using inherited = CEatableItemObject;

public:
	CAntigasFilter();
	virtual					~CAntigasFilter();
	virtual CAntigasFilter* cast_filter() { return this; }

	virtual void			Load(LPCSTR section);
	virtual bool			Useful() const;

	virtual BOOL			net_Spawn(CSE_Abstract* DC);

	virtual	void			UseBy(CEntityAlive* npc);
	virtual	bool			Empty() { return m_iPortionsNum == 0; };
	float					m_fCondition;
	void					ChangeInOutfit();
	void					ChangeFilterCondition(float val);
	float					GetFilterCondition(void) const;
	bool					UseAllowed();
protected:
	int						m_iPortionsNum;
};