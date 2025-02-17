#pragma once

#include "WeaponCustomPistol.h"
#include "script_export_space.h"

class CWeaponKnife: public CWeapon {
private:
	typedef CWeapon inherited;
protected:

	bool				m_attackStart;
	bool				m_attackMotionMarksAvailable;

protected:

	virtual void		switch2_Idle				();
	virtual void		switch2_Hiding				();
	virtual void		switch2_Hidden				();
	virtual void		switch2_Showing				();
			void		switch2_Attacking			(u32 state);

	virtual void		OnMotionMark				(u32 state, const motion_marks& M);
	virtual void		OnAnimationEnd				(u32 state);
	virtual void		OnStateSwitch				(u32 S, u32 oldState);
	virtual void		DeviceUpdate				() override;
	virtual void		UpdateCL					() override;

	virtual void		PlayAnimDeviceSwitch		() override;

	void				state_Attacking				(float dt);

	virtual void		KnifeStrike					(const Fvector& pos, const Fvector& dir);

	float				fWallmarkSize;
	u16					knife_material_idx;

protected:	
	ALife::EHitType		m_eHitType;

	ALife::EHitType		m_eHitType_1;
	//float				fHitPower_1;
	Fvector4			fvHitPower_1;
	float				fHitImpulse_1;

	ALife::EHitType		m_eHitType_2;
	//float				fHitPower_2;
	Fvector4			fvHitPower_2;
	float				fCurrentHit;
	float				fHitImpulse_2;

	bool				m_bIsAltShootNow;

protected:
	virtual void		LoadFireParams					(LPCSTR section, LPCSTR prefix);
public:
						CWeaponKnife(); 
	virtual				~CWeaponKnife(); 

	void				Load							(LPCSTR section);

	virtual void		Fire2Start						();
	virtual void		FireStart						();


	virtual bool		Action							(s32 cmd, u32 flags);

	virtual bool		GetBriefInfo					(II_BriefInfo& info);

			void		FastStrike						(u32 state);

	DECLARE_SCRIPT_REGISTER_FUNCTION
};
add_to_type_list(CWeaponKnife)
#undef script_type_list
#define script_type_list save_type_list(CWeaponKnife)
