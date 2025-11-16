#pragma once

#include "hud_item_object.h"
#include "hit_immunity.h"
#include "PHObject.h"
#include "script_export_space.h"

struct SArtefactActivation;

class CArtefact :	public CHudItemObject, 
					public CPHUpdateObject {
private:
	typedef			CHudItemObject	inherited;
public:
									CArtefact						();
	virtual							~CArtefact						();

	virtual void					Load							(LPCSTR section);
	
	virtual BOOL					net_Spawn						(CSE_Abstract* DC);
	virtual void					net_Destroy						();

	virtual void					OnH_A_Chield					();
	virtual void					OnH_B_Independent				(bool just_before_destroy);
	virtual void					save							(NET_Packet &output_packet);
	virtual void					load							(IReader &input_packet);
	
	virtual void					UpdateCL						();
	virtual void					shedule_Update					(u32 dt);	
			void					UpdateWorkload					(u32 dt);

	
	virtual bool					CanTake							() const;

	//virtual void					renderable_Render				();
	virtual BOOL					renderable_ShadowGenerate		()		{ return FALSE;	}
	virtual BOOL					renderable_ShadowReceive		()		{ return TRUE;	}
	virtual void					create_physic_shell				();

	virtual CArtefact*				cast_artefact					()		{return this;}
	virtual	u32						Cost							() const;

			float					GetRestoreByType				(ALife::EConditionRestoreType type) const;

			float					GetHealthPower					() const { return m_fHealthRestoreSpeed; }
			float					GetRadiationPower				() const { return m_fRadiationRestoreSpeed; }
			float					GetSatietyPower					() const { return m_fSatietyRestoreSpeed; }
			float					GetPowerPower					() const { return m_fPowerRestoreSpeed; }
			float					GetBleedingPower				() const { return m_fBleedingRestoreSpeed; }

			void					SetHealthPower					(float value) { m_fHealthRestoreSpeed = value; }
			void					SetRadiationPower				(float value) { m_fRadiationRestoreSpeed = value; }
			void					SetSatietyPower					(float value) { m_fSatietyRestoreSpeed = value; }
			void					SetPowerPower					(float value) { m_fPowerRestoreSpeed = value; }
			void					SetBleedingPower				(float value) { m_fBleedingRestoreSpeed = value; }

protected:
	virtual void					UpdateCLChild					()		{};

	u16								m_CarringBoneID;
	shared_str						m_sParticlesName;
protected:
	SArtefactActivation*			m_activationObj;
	ref_light						m_pTrailLight;
	Fcolor							m_TrailLightColor;
	float							m_fTrailLightRange;
	bool							m_bLightsEnabled;

	virtual void					UpdateLights					();
public:
	virtual void					StartLights();
	virtual void					StopLights();
	void							ActivateArtefact					();
	bool							CanBeActivated						() {return m_bCanSpawnZone;};// does artefact can spawn anomaly zone
	float							AdditionalInventoryWeight			() const {return m_additional_weight;}

	virtual void					PhDataUpdate						(dReal step);
	virtual void					PhTune								(dReal step)	{};

	bool							m_bCanSpawnZone;
	float							m_fHealthRestoreSpeed;
	float 							m_fRadiationRestoreSpeed;
	float 							m_fSatietyRestoreSpeed;
	float							m_fPowerRestoreSpeed;
	float							m_fBleedingRestoreSpeed;
	float							m_additional_weight;
	float 							m_fThirstRestoreSpeed;
	float 							m_fIntoxicationRestoreSpeed;
	float 							m_fSleepenessRestoreSpeed;
	float 							m_fAlcoholismRestoreSpeed;
	float 							m_fNarcotismRestoreSpeed;
	float 							m_fPsyHealthRestoreSpeed;
	float 							m_fFrostbiteRestoreSpeed;
	float							m_fJumpSpeed;
	float							m_fWalkAccel;

	CHitImmunity 					m_ArtefactHitImmunities;
	xr_hash_map<ALife::EHitType, float>	m_HitTypeProtection;
	xr_hash_map<ALife::EHitType, float>	m_ConstHitTypeProtection;

	//For Degradation
	float							m_fConstHealthRestoreSpeed;
	float							m_fConstRadiationRestoreSpeed;
	float							m_fConstSatietyRestoreSpeed;
	float							m_fConstPowerRestoreSpeed;
	float							m_fConstBleedingRestoreSpeed;
	float							m_fConstThirstRestoreSpeed;
	float							m_fConstIntoxicationRestoreSpeed;
	float							m_fConstSleepenessRestoreSpeed;
	float							m_fConstAlcoholismRestoreSpeed;
	float							m_fConstNarcotismRestoreSpeed;
	float 							m_fConstPsyHealthRestoreSpeed;
	float							m_fConstFrostbiteRestoreSpeed;
	float							m_fConstAdditionalWeight;
	float							m_fConstTrailLightRange;
	float							m_fConstVolumetricDistance;
	float							m_fConstVolumetricIntensity;
	float							m_fConstJumpSpeed;
	float							m_fConstWalkAccel;
	float							m_fChargeLevel;
	float							m_fCurrentChargeLevel;
	float							m_fDegradationSpeed;
	void							SetChargeLevel(float charge_level);
	float							GetCurrentChargeLevel() const;
	void							SetRank(int rank);
	int								GetCurrentAfRank() const;

	bool							m_bVolumetricLights;
	float							m_fVolumetricQuality;
	float							m_fVolumetricDistance;
	float							m_fVolumetricIntensity;

	int								m_iAfRank;

	bool							m_bInContainer;
public:
	enum EAFHudStates {
		eIdle		= 0,
		eShowing,
		eHiding,
		eHidden,
		eActivating,
	};
	virtual	void					PlayAnimIdle		();
	virtual void					Hide				();
	virtual void					Show				();
	virtual	void					UpdateXForm			();
	virtual bool					Action				(s32 cmd, u32 flags);
	virtual void					OnStateSwitch		(u32 S, u32 oldState = 0);
	virtual void					OnAnimationEnd		(u32 state);
	virtual bool					IsHidden			()	const	{return GetState()==eHidden;}
	virtual bool					GetBriefInfo		(II_BriefInfo& info);

	virtual u16						bone_count_to_synchronize	() const;

			bool					ParentIsActor		();

			void					UpdateDegradation	();

			bool					IsInContainer		();

	// optimization FAST/SLOW mode
	u32						o_render_frame				;
	BOOL					o_fastmode					;
	IC void					o_switch_2_fast				()	{
		if (o_fastmode)		return	;
		o_fastmode			= TRUE	;
		//processing_activate		();
	}
	IC void					o_switch_2_slow				()	{
		if (!o_fastmode)	return	;
		o_fastmode			= FALSE	;
		//processing_deactivate		();
	}

	DECLARE_SCRIPT_REGISTER_FUNCTION
};
add_to_type_list(CArtefact)
#undef script_type_list
#define script_type_list save_type_list(CArtefact)

