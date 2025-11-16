#pragma once

#include "inventory_item_object.h"

struct SBoneProtections;

class CHelmet: public CInventoryItemObject {
private:
	typedef	CInventoryItemObject inherited;
public:
							CHelmet					();
	virtual					~CHelmet				();

	virtual void			Load					(LPCSTR section);
	
	virtual void			Hit						(float P, ALife::EHitType hit_type);
	virtual void			UpdateCL				();

	shared_str				m_BonesProtectionSect;
	shared_str				m_NightVisionSect;

	xr_vector<shared_str>	m_SuitableFilters;
	xr_vector<shared_str>	m_SuitableRepairKits;
	xr_vector<std::pair<shared_str, int>> m_ItemsForRepair;
	xr_vector<LPCSTR>		m_ItemsForRepairNames;

	virtual void			OnMoveToSlot			(EItemPlace previous_place);
	virtual void			OnMoveToRuck			(EItemPlace previous_place);
	virtual BOOL			net_Spawn				(CSE_Abstract* DC);
	virtual void			net_Export				(NET_Packet& P);
	virtual void			net_Import				(NET_Packet& P);
	virtual void			OnH_A_Chield			();
	virtual void			save					(NET_Packet& output_packet);
	virtual void			load					(IReader& input_packet);
			void			UpdateFilterCondition	(void);
			float			GetFilterCondition		(void) const;
			void			SetFilterCondition		(float val);
			float			GetDegradationSpeed		(void) const;
			void			FilterReplace			(float val);
			bool			IsNecessaryItem			(const shared_str& item_sect, xr_vector<shared_str> item);

			IC int			GetHelmetNV_Type		() const { return m_NightVisionType; }

	float					GetDefHitTypeProtection	(ALife::EHitType hit_type);
	float					GetHitTypeProtection	(ALife::EHitType hit_type, s16 element);
	float					GetBoneArmor			(s16 element);

	float					HitThroughArmor			(float hit_power, s16 element, float ap, bool& add_wound, ALife::EHitType hit_type);

	float					m_fPowerLoss;
	float					m_fHealthRestoreSpeed;
	float 					m_fRadiationRestoreSpeed;
	float 					m_fSatietyRestoreSpeed;
	float					m_fPowerRestoreSpeed;
	float					m_fBleedingRestoreSpeed;
	float 					m_fThirstRestoreSpeed;
	float 					m_fIntoxicationRestoreSpeed;
	float 					m_fSleepenessRestoreSpeed;
	float 					m_fAlcoholismRestoreSpeed;
	float 					m_fNarcotismRestoreSpeed;
	float 					m_fPsyHealthRestoreSpeed;
	float 					m_fFrostbiteRestoreSpeed;

	float					m_fFilterDegradation;
	float					m_fMaxFilterCondition;
	float					m_fFilterCondition;

	float					m_fShowNearestEnemiesDistance;

	bool					m_bSecondHelmetEnabled;
	bool					m_b_HasGlass;
	bool					m_bUseFilter;
	bool					m_bFilterProtectionDropsInstantly;
	bool					m_bUseAttach;

	shared_str				m_sShaderNightVisionSect;
	u32						m_NightVisionType;
	float					m_fNightVisionLumFactor;

	void					ReloadBonesProtection	();

protected:
	HitImmunity::HitTypeSVec	m_ConstHitTypeProtection;
	HitImmunity::HitTypeSVec	m_HitTypeProtection;
	SBoneProtections*		m_boneProtection;	

protected:
	virtual bool			install_upgrade_impl	( LPCSTR section, bool test );

	virtual bool			use_parent_ai_locations	() const override { return (!H_Parent()); }
};
