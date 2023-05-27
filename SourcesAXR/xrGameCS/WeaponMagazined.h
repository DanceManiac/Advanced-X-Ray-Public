#pragma once

#include "weapon.h"
#include "hudsound.h"
#include "ai_sounds.h"
#include "hud_item_object.h"

class ENGINE_API CMotionDef;

//������ ������� ��������� �������������
//����������� ��������, ������, ���� ��������� �������
#define WEAPON_ININITE_QUEUE -1


class CWeaponMagazined: public CWeapon
{
private:
	typedef CWeapon inherited;
protected:
	//���� �������� ��������
	shared_str		m_sSndShotCurrent;

	//�������������� ���������� � ���������
	LPCSTR			m_sSilencerFlameParticles;
	LPCSTR			m_sSilencerSmokeParticles;

	ESoundTypes		m_eSoundShow;
	ESoundTypes		m_eSoundHide;
	ESoundTypes		m_eSoundShot;
	ESoundTypes		m_eSoundEmptyClick;
	ESoundTypes		m_eSoundReload;
	ESoundTypes		m_eSoundClose;
	ESoundTypes		m_eSoundReflect;
	bool			m_sounds_enabled;
	// General
	//���� ������� ��������� UpdateSounds
	u32				dwUpdateSounds_Frame;

	virtual void    CheckMagazine();

	bool            m_bNeedBulletInGun;

	bool            m_bCustomShotSounds;
protected:
	virtual void	OnMagazineEmpty	();

	virtual void	switch2_Idle	();
	virtual void	switch2_Fire	();
	virtual void	switch2_Empty	();
	virtual void	switch2_Reload	();
	virtual void	switch2_Hiding	();
	virtual void	switch2_Hidden	();
	virtual void	switch2_Showing	();
	virtual void    switch2_Unmis	();
	virtual void	switch2_ChangeFireMode();
	
	virtual void	OnShot			();	
	
	virtual void	OnEmptyClick	();

	virtual void	OnAnimationEnd	(u32 state);
	virtual void	OnStateSwitch	(u32 S);

	virtual void	UpdateSounds	();

	bool			TryReload		();

protected:
	virtual void	ReloadMagazine();
			void	ApplySilencerKoeffs();
			void	ResetSilencerKoeffs();

	virtual void	state_Fire		(float dt);
	virtual void	state_MagEmpty	(float dt);
	virtual void	state_Misfire	(float dt);
public:
					CWeaponMagazined	(ESoundTypes eSoundType=SOUND_TYPE_WEAPON_SUBMACHINEGUN);
	virtual			~CWeaponMagazined	();

	virtual void	Load			(LPCSTR section);
			void	LoadSilencerKoeffs();
	virtual CWeaponMagazined*cast_weapon_magazined	()		 {return this;}

	virtual bool    UseScopeTexture	();
	virtual void	SetDefaults		();
	virtual void	FireStart		();
	virtual void	FireEnd			();
	virtual void	Reload			();
	

	virtual	void	UpdateCL		();
	virtual void	net_Destroy		();
	virtual void	net_Export		(NET_Packet& P);
	virtual void	net_Import		(NET_Packet& P);

	virtual void	OnH_A_Chield		();

	virtual bool	Attach(PIItem pIItem, bool b_send_event);
	virtual bool	Detach(const char* item_section_name, bool b_spawn_item);
			bool	DetachScope(const char* item_section_name, bool b_spawn_item);
	virtual bool	CanAttach(PIItem pIItem);
	virtual bool	CanDetach(const char* item_section_name);

	virtual void	InitAddons();

	virtual bool	Action			(s32 cmd, u32 flags);
	bool			IsAmmoAvailable	();
	virtual void	UnloadMagazine	(bool spawn_ammo = true);
	virtual int     CheckAmmoBeforeReload(u8& v_ammoType);
	virtual void	OnMotionMark	(u32 state, const motion_marks& M);

	virtual void	GetBriefInfo				(xr_string& str_name, xr_string& icon_sect_name, xr_string& str_count, string16& fire_mode);

public:
	virtual bool	SwitchMode				();
	virtual bool	SingleShotMode			()			{return 1 == m_iQueueSize;}
	virtual void	SetQueueSize			(int size);
	IC		int		GetQueueSize			() const	{return m_iQueueSize;};
	virtual bool	StopedAfterQueueFired	()			{return m_bStopedAfterQueueFired; }
	virtual void	StopedAfterQueueFired	(bool value){m_bStopedAfterQueueFired = value; }

protected:
	//������������ ������ �������, ������� ����� ����������
	int				m_iQueueSize;
	//���������� ������� ����������� ��������
	int				m_iShotNum;
	//����� ������ �������, ��� ����������� ��������, ���������� ������ (������� ��-�� �������)
	int				m_iShootEffectorStart;
	Fvector			m_vStartPos, m_vStartDir;
	//���� ����, ��� �� ������������ ����� ���� ��� ����������
	//����� ������� ��������, ������� ���� ������ � m_iQueueSize
	bool			m_bStopedAfterQueueFired;
	//���� ����, ��� ���� �� ���� ������� �� ������ �������
	//(���� ���� ����� ������ ������ �� ����� � ��������� FireEnd)
	bool			m_bFireSingleShot;
	//������ ��������
	bool			m_bHasDifferentFireModes;
	xr_vector<s8>	m_aFireModes;
	int				m_iCurFireMode;
	int				m_iPrefferedFireMode;

	//���������� ��������� �������������
	//������ ������ ����� ��������
	bool m_bLockType;
	bool m_bAutoreloadEnabled;
	bool m_opened;
	bool m_bUseFiremodeChangeAnim;

public:
	virtual void	OnZoomIn			();
	virtual void	OnZoomOut			();
			void	OnNextFireMode		();
			void	OnPrevFireMode		();
			bool	HasFireModes		() { return m_bHasDifferentFireModes; };
	virtual	int		GetCurrentFireMode	() { return m_aFireModes[m_iCurFireMode]; };	

	virtual void	save				(NET_Packet &output_packet);
	virtual void	load				(IReader &input_packet);

protected:
	virtual bool	install_upgrade_impl( LPCSTR section, bool test );

protected:
	virtual bool	AllowFireWhileWorking() {return false;}

	//����������� ������� ��� ������������ �������� HUD
	virtual void	PlayAnimShow		();
	virtual void	PlayAnimHide		();
	virtual void	PlayAnimReload		();
	virtual void	PlayAnimIdle		();
	virtual void	PlayAnimShoot		();
	virtual void	PlayReloadSound		();
	virtual void	PlayAnimAim			();
	virtual void	PlayAnimBore		();
	virtual void	PlayAnimIdleSprint	();
	virtual void	PlayAnimIdleMoving	();
	virtual void	PlayAnimFireMode	();

	virtual void    SetAnimFlag(u32 flag, LPCSTR anim_name);

private:
	string64 guns_aim_anm;
protected:
	const	char*	GetAnimAimName		();

	enum {
		ANM_SHOW_EMPTY = (1 << 0),
		ANM_HIDE_EMPTY = (1 << 1),
		ANM_AIM_EMPTY = (1 << 2),
		ANM_BORE_EMPTY = (1 << 3),
		ANM_SHOT_EMPTY = (1 << 4),
		ANM_SPRINT_EMPTY = (1 << 5),
		ANM_MOVING_EMPTY = (1 << 6),
		ANM_RELOAD_EMPTY = (1 << 7),
		ANM_RELOAD_EMPTY_GL = (1 << 8),
		ANM_SHOT_AIM = (1 << 9),
		ANM_SHOT_AIM_GL = (1 << 10),
		ANM_MISFIRE = (1 << 11),
		ANM_MISFIRE_GL = (1 << 12),
		ANM_IDLE_EMPTY = (1 << 13),
	};

	Flags32 psWpnAnimsFlag;

	bool WeaponSoundExist(LPCSTR section, LPCSTR sound_name, bool log = false) const;

	virtual	int		ShotsFired			() { return m_iShotNum; }
	virtual float	GetWeaponDeterioration	();

};
