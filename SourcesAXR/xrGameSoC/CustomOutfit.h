#pragma once

#include "inventory_item_object.h"

struct SBoneProtections;

class CCustomOutfit: public CInventoryItemObject {
private:
    typedef	CInventoryItemObject inherited;
public:
									CCustomOutfit		(void);
	virtual							~CCustomOutfit		(void);

	virtual void					Load				(LPCSTR section);
	
	//уменьшенная версия хита, для вызова, когда костюм надет на персонажа
	virtual void					Hit					(float P, ALife::EHitType hit_type);
	virtual void					UpdateCL			();
	virtual void			save						(NET_Packet& output_packet);
	virtual void			load						(IReader& input_packet);
	void					UpdateFilterCondition		(void);
	float					GetFilterCondition			(void) const;
	void					SetFilterCondition			(float val);
	float					GetDegradationSpeed			(void) const;
	void					FilterReplace				(float val);

	//коэффициенты на которые домножается хит
	//при соответствующем типе воздействия
	//если на персонаже надет костюм
	float							GetHitTypeProtection(ALife::EHitType hit_type, s16 element);
	float							GetDefHitTypeProtection(ALife::EHitType hit_type);

	float							HitThruArmour		(float hit_power, s16 element, float AP);
	//коэффициент на который домножается потеря силы
	//если на персонаже надет костюм
	float							GetPowerLoss		();


	virtual void					OnMoveToSlot		();
	virtual void					OnMoveToRuck		(EItemPlace prev);

	bool							IsNecessaryItem		(const shared_str& item_sect, xr_vector<shared_str> item);
protected:
	float							m_fPowerLoss;

	shared_str						m_ActorVisual;
	shared_str						m_full_icon_name;
	SBoneProtections*				m_boneProtection;	
protected:
	u32								m_ef_equipment_type;

public:
	float							m_additional_weight;
	float							m_additional_weight2;
	float							m_fHealthRestoreSpeed; // as from artefact
	float 							m_fRadiationRestoreSpeed;
	float 							m_fSatietyRestoreSpeed;
	float							m_fPowerRestoreSpeed;
	float							m_fBleedingRestoreSpeed;
	float 							m_fThirstRestoreSpeed;
	float 							m_fIntoxicationRestoreSpeed;
	float 							m_fSleepenessRestoreSpeed;
	float 							m_fAlcoholismRestoreSpeed;
	float 							m_fNarcotismRestoreSpeed;
	float 							m_fPsyHealthRestoreSpeed;
	float 							m_fFrostbiteRestoreSpeed;
	float							m_fJumpSpeed;
	float							m_fWalkAccel;
	float							m_fOverweightWalkK;
	float							m_fFilterDegradation;
	float							m_fMaxFilterCondition;
	float							m_fFilterCondition;

	shared_str						m_NightVisionSect;
	
	xr_vector<shared_str>			m_SuitableFilters;
	xr_vector<shared_str>			m_SuitableRepairKits;
	xr_vector<std::pair<shared_str, int>> m_ItemsForRepair;

	bool							m_b_HasGlass;
	bool							m_bUseFilter;

	shared_str						m_sShaderNightVisionSect;
	u32								m_NightVisionType;
	float							m_fNightVisionLumFactor;

	virtual u32						ef_equipment_type		() const;
	virtual	BOOL					BonePassBullet			(int boneID);
	const shared_str&				GetFullIconName			() const	{return m_full_icon_name;};

	IC int							GetOutfitNV_Type		() const { return m_NightVisionType; }

	virtual CCustomOutfit*			cast_outfit() { return this; }

	HitImmunity::HitTypeSVec		m_ConstHitTypeProtection;
	HitImmunity::HitTypeSVec		m_HitTypeProtection;

	virtual void			net_Export			(NET_Packet& P);
	virtual void			net_Import			(NET_Packet& P);
	
	DECLARE_SCRIPT_REGISTER_FUNCTION
};

add_to_type_list(CCustomOutfit)
#undef script_type_list
#define script_type_list save_type_list(CCustomOutfit)
