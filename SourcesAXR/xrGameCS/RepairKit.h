﻿#pragma once

#include "eatable_item_object.h"

class CRepairKit : public CEatableItemObject
{
	using inherited = CEatableItemObject;

public:
	CRepairKit();
	virtual					~CRepairKit();
	virtual CRepairKit*		cast_repair_kit()	{return this;}

	virtual void			Load(LPCSTR section);
	virtual bool			Useful() const;

	virtual BOOL			net_Spawn(CSE_Abstract* DC);

	virtual	void			UseBy(CEntityAlive* npc);
	virtual	bool			Empty() { return m_iPortionsNum == 0; };
	int						m_iUseFor;
	float					m_fRestoreCondition;
	void					ChangeInOutfit();
	void					ChangeInKnife();
	void					ChangeInWpn1();
	void					ChangeInWpn2();
	void					ChangeRepairKitCondition(float val);
	float					GetRepairKitCondition(void) const;
protected:
	int						m_iPortionsNum;
};