#pragma once
#include "inventory_item_object.h"
#include "../xrEngine/Feel_Touch.h"
#include "hudsound.h"
#include "Battery.h"

class CCustomZone;
//описание типа зоны
struct ZONE_TYPE
{
	//интервал частот отыгрывани€ звука
	float		min_freq;
	float		max_freq;
	//звук реакции детектора на конкретную зону
	HUD_SOUND_ITEM	detect_snds;

	shared_str	zone_map_location;
};

//описание зоны, обнаруженной детектором
struct ZONE_INFO
{
	u32								snd_time;
	//текуща€ частота работы датчика
	float							cur_freq;
	//particle for night-vision mode
	CParticlesObject*				pParticle;

	ZONE_INFO	();
	~ZONE_INFO	();
};

class CInventoryOwner;

class CDetectorAnomaly :
	public CInventoryItemObject,
	public Feel::Touch
{
	//typedef	CInventoryItemObject	inherited;
	using inherited = CInventoryItemObject;
public:
	CDetectorAnomaly(void);
	virtual ~CDetectorAnomaly(void);

	virtual BOOL net_Spawn			(CSE_Abstract* DC);
	virtual void Load				(LPCSTR section);

	virtual void OnH_A_Chield		();
	virtual void OnH_B_Independent	(bool just_before_destroy);

	virtual void shedule_Update		(u32 dt);
	virtual void UpdateCL			();

	virtual void feel_touch_new		(CObject* O);
	virtual void feel_touch_delete	(CObject* O);
	virtual BOOL feel_touch_contact	(CObject* O);

			void TurnOn				();
			void TurnOff			();
			bool IsWorking			() {return m_bWorking;}

	virtual void OnMoveToSlot		();
	virtual void OnMoveToRuck		(EItemPlace prev);

			void UpdateChargeLevel	(void);
	virtual void save				(NET_Packet &output_packet);
	virtual void load				(IReader &input_packet);
			float GetCurrentChargeLevel(void) const;
			void SetCurrentChargeLevel(float val);
			float GetUnchargeSpeed	(void) const;
			void Recharge			(float val);
			bool IsNecessaryItem	(const shared_str& item_sect, xr_vector<shared_str> item);

			float	m_fUnchargeSpeed;
			xr_vector<shared_str> m_SuitableBatteries;

protected:
	void StopAllSounds				();

	bool m_bWorking;

	float m_fRadius;

	//если хоз€ин текущий актер
	CActor*				m_pCurrentActor;
	CInventoryOwner*	m_pCurrentInvOwner;

	//информаци€ об онаруживаемых зонах
	DEFINE_MAP(CLASS_ID, ZONE_TYPE, ZONE_TYPE_MAP, ZONE_TYPE_MAP_IT);
	ZONE_TYPE_MAP m_ZoneTypeMap;
	
	//список обнаруженных зон и информаци€ о них
	DEFINE_MAP(CCustomZone*, ZONE_INFO, ZONE_INFO_MAP, ZONE_INFO_MAP_IT);
	ZONE_INFO_MAP m_ZoneInfoMap;
	
	shared_str						m_nightvision_particle;

protected:
	u32					m_ef_detector_type;

public:
	virtual u32			ef_detector_type	() const;
};