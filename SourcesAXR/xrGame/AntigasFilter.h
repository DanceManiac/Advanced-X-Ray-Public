#pragma once

#include "eatable_item_object.h"

class CAntigasFilter : public CEatableItemObject
{
	using inherited = CEatableItemObject;

public:
	CAntigasFilter();
	virtual					~CAntigasFilter();
	virtual CAntigasFilter  *cast_filter			()	{return this;}

	virtual void			Load(LPCSTR section);
	virtual bool			Useful() const;

	virtual BOOL			net_Spawn(CSE_Abstract* DC);

	virtual	bool			UseBy(CEntityAlive* npc);
	virtual	bool			Empty() { return PortionsNum() == 0; };
	int						PortionsNum()	const { return m_iPortionsNum; }
	float					m_fCondition;
	void					ChangeInOutfit();
	void					ChangeInHelmet();
	void					ChangeFilterCondition(float val);
	float					GetFilterCondition(void) const;
protected:
	int						m_iPortionsNum;
};