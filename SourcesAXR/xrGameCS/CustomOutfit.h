#pragma once

#include "inventory_item_object.h"

struct SBoneProtections;

class CCustomOutfit: public CInventoryItemObject {
private:
    typedef	CInventoryItemObject inherited;
public:
							CCustomOutfit		(void);
	virtual					~CCustomOutfit		(void);

	virtual void			Load				(LPCSTR section);
	
	//уменьшенная версия хита, для вызова, когда костюм надет на персонажа
	virtual void			Hit					(float P, ALife::EHitType hit_type);
	virtual void			UpdateCL			();
	virtual void			save				(NET_Packet& output_packet);
	virtual void			load				(IReader& input_packet);
	void					UpdateFilterCondition(void);
	float					GetFilterCondition	(void) const;
	void					SetFilterCondition	(float val);
	float					GetDegradationSpeed	(void) const;
	void					FilterReplace		(float val);

	//коэффициенты на которые домножается хит
	//при соответствующем типе воздействия
	//если на персонаже надет костюм
	float					GetHitTypeProtection(ALife::EHitType hit_type, s16 element);
	float					GetDefHitTypeProtection(ALife::EHitType hit_type);
	float					GetBoneArmor(s16 element);

	float					HitThroughArmor		(float hit_power, s16 element, float ap, bool& add_wound );
	//коэффициент на который домножается потеря силы
	//если на персонаже надет костюм
	float					GetPowerLoss		();


	virtual void			OnMoveToSlot		();
	virtual void			OnMoveToRuck		(EItemPlace prev);

protected:
	float							m_fPowerLoss;

	shared_str				m_ActorVisual;
	shared_str				m_full_icon_name;
	SBoneProtections*		m_boneProtection;	
protected:
	u32						m_ef_equipment_type;
	u32						m_artefact_count;

public:
	float					m_additional_weight;
	float					m_additional_weight2;

	float					m_fHealthRestoreSpeed; // as from artefact
	float 					m_fRadiationRestoreSpeed;
	float 					m_fSatietyRestoreSpeed;
	float					m_fPowerRestoreSpeed;
	float					m_fBleedingRestoreSpeed;
	float 					m_fThirstRestoreSpeed;
	float 					m_fIntoxicationRestoreSpeed;
	float 					m_fSleepenessRestoreSpeed;
	float 					m_fAlcoholismRestoreSpeed;
	float 					m_fNarcotismRestoreSpeed;

	float					m_fJumpSpeed;
	float					m_fWalkAccel;
	float					m_fOverweightWalkK;
	float					m_fFilterDegradation;
	float					m_fMaxFilterCondition;
	float					m_fFilterCondition;

	shared_str				m_NightVisionSect;
	shared_str				m_BonesProtectionSect;

	bool					m_b_HasGlass;
	bool					m_bUseFilter;
	u32						m_NightVisionType;

	virtual u32				ef_equipment_type		() const;
	virtual	BOOL			BonePassBullet			(int boneID);
	const shared_str&		GetFullIconName			() const	{ return m_full_icon_name; }
	u32						get_artefact_count		() const	{ return m_artefact_count; }

	virtual BOOL			net_Spawn				(CSE_Abstract* DC);
	virtual void			net_Export				(NET_Packet& P);
	virtual void			net_Import				(NET_Packet& P);
			void			ApplySkinModel			(CActor* pActor, bool bDress, bool bHUDOnly);
			void			ReloadBonesProtection	(CActor* pActor);

			IC int			GetOutfitNV_Type		() const {return m_NightVisionType;}

	HitImmunity::HitTypeSVec m_ConstHitTypeProtection;
	HitImmunity::HitTypeSVec m_HitTypeProtection;
protected:
	virtual bool			install_upgrade_impl( LPCSTR section, bool test );
};
