#pragma once

#include "hud_item_object.h"

class CCustomBackpack : public CHudItemObject
{
	typedef	CHudItemObject	inherited;
public:
	CCustomBackpack();
	virtual ~CCustomBackpack();

	virtual BOOL 	net_Spawn(CSE_Abstract* DC);
	virtual void	Load(LPCSTR section);

	virtual void 	OnH_A_Chield();
	virtual void 	OnH_B_Independent(bool just_before_destroy);

	virtual void 	shedule_Update(u32 dt);
	virtual void 	UpdateCL();

	virtual void 	OnMoveToSlot();
	virtual void 	OnMoveToRuck(EItemPlace prev);
	virtual void	OnActiveItem();
	virtual void	OnHiddenItem();

	void			ToggleBackpack();
	void			HideBackpack();
	void			ShowBackpack();

	virtual void	OnStateSwitch(u32 S);
	virtual void	OnAnimationEnd(u32 state);
	virtual	void	UpdateXForm();

	float			GetInventoryCapacity() const { return m_fInventoryCapacity; }

	float			m_fPowerLoss;
	float			m_additional_weight;
	float			m_additional_weight2;

	float			m_fHealthRestoreSpeed;
	float 			m_fRadiationRestoreSpeed;
	float 			m_fSatietyRestoreSpeed;
	float			m_fPowerRestoreSpeed;
	float			m_fBleedingRestoreSpeed;
	float 			m_fThirstRestoreSpeed;
	float 			m_fIntoxicationRestoreSpeed;
	float 			m_fSleepenessRestoreSpeed;
	float 			m_fAlcoholismRestoreSpeed;
	float 			m_fNarcotismRestoreSpeed;
	float 			m_fPsyHealthRestoreSpeed;
	float 			m_fFrostbiteRestoreSpeed;

	float			m_fJumpSpeed;
	float			m_fWalkAccel;
	float			m_fOverweightWalkK;

	float			m_fInventoryCapacity;

protected:
	virtual bool	install_upgrade_impl(LPCSTR section, bool test);
};

