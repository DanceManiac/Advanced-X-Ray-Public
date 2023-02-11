#pragma once

#include "inventory_item_object.h"
#include "hudsound.h"
#include "script_export_space.h"
#include "Battery.h"

class CLAItem;

class CTorch : public CInventoryItemObject {
private:
    typedef	CInventoryItemObject	inherited;

protected:
	float			fBrightness;
	CLAItem*		lanim;

	u16				guid_bone;
	shared_str		light_trace_bone;

	float			m_delta_h;
	Fvector2		m_prev_hp;
	bool			m_switched_on;
	ref_light		light_render;
	ref_light		light_omni;
	ref_glow		glow_render;
	Fvector			m_focus;
	shared_str		m_light_section;
	Fvector			m_torch_offset;
	Fvector			m_omni_offset;
	float			m_torch_inertion_speed_max;
	float			m_torch_inertion_speed_min;

private:
	inline	bool	can_use_dynamic_lights	();

public:
					CTorch					();
	virtual			~CTorch					();

	virtual void	Load					(LPCSTR section);
	virtual BOOL	net_Spawn				(CSE_Abstract* DC);
	virtual void	net_Destroy				();
	virtual void	net_Export				(NET_Packet& P);				// export to server
	virtual void	net_Import				(NET_Packet& P);				// import from server

	virtual void	OnH_A_Chield			();
	virtual void	OnH_B_Independent		(bool just_before_destroy);

	virtual void	OnMoveToSlot			(const SInvItemPlace& prev);
	virtual void	OnMoveToRuck			(const SInvItemPlace& prev);

	virtual void	UpdateCL				();

			void	Switch					();
			void	Switch					(bool light_on);
			bool	torch_active			() const;
			void	UpdateChargeLevel		(void);
	virtual void	save					(NET_Packet &output_packet);
	virtual void	load					(IReader &input_packet);
			float	GetCurrentChargeLevel	(void) const;
			void	SetCurrentChargeLevel	(float val);
			bool	IsSwitchedOn			(void) const;
			float	GetUnchargeSpeed		(void) const;
			void	Recharge				(float val);

	virtual bool	can_be_attached			() const;

			float	get_range				() const;

	//CAttachableItem
	virtual	void	enable					(bool value);

	float			m_fMaxChargeLevel;
	float			m_fCurrentChargeLevel;
	float			m_fUnchargeSpeed;
	float			m_fMaxRange;
	float			m_fCurveRange;
	shared_str		m_SuitableBattery;

	virtual CTorch* cast_torch				() { return this; }
 
protected:
	HUD_SOUND_COLLECTION_LAYERED m_sounds;

	enum EStats{
		eTorchActive				= (1<<0),
		eNightVisionActive			= (1<<1),
		eAttached					= (1<<2)
	};

public:

	virtual bool			use_parent_ai_locations	() const
	{
		return				(!H_Parent());
	}
	virtual void	create_physic_shell		();
	virtual void	activate_physic_shell	();
	virtual void	setup_physic_shell		();

	virtual void	afterDetach				();
	virtual void	renderable_Render		();

	virtual bool	install_upgrade_impl	(LPCSTR section, bool test);

	DECLARE_SCRIPT_REGISTER_FUNCTION
};

add_to_type_list(CTorch)
#undef script_type_list
#define script_type_list save_type_list(CTorch)
