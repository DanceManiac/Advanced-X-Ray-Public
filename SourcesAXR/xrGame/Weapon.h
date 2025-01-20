#pragma once

#include "../xrphysics/PhysicsShell.h"
#include "weaponammo.h"
#include "PHShellCreator.h"

#include "ShootingObject.h"
#include "hud_item_object.h"
#include "Actor_Flags.h"
#include "../Include/xrRender/KinematicsAnimated.h"
#include "firedeps.h"
#include "game_cl_single.h"
#include "first_bullet_controller.h"

#include "CameraRecoil.h"
#include "WeaponAttaches.h"

class CEntity;
class ENGINE_API CMotionDef;
class CSE_ALifeItemWeapon;
class CSE_ALifeItemWeaponAmmo;
class CWeaponMagazined;
class CParticlesObject;
class CUIWindow;
class CBinocularsVision;
class CNightVisionEffector;
class CLAItem;
class WeaponAttach;

#define WEAPON_INDOOR_HEMI_FACTOR 0.01f         // Сила освещённости персонжаей солнечным светом, ниже которой считается что персонаж в помещении 
#define WEAPON_SND_REFLECTION_HUD_FACTOR 0.7f   // Коэфицент на который домножается громкость звука эха от выстрела, если он был сделат от 1-го лица

class CWeapon : public CHudItemObject,
				public CShootingObject
{
private:
	typedef CHudItemObject inherited;

public:
							CWeapon				();
	virtual					~CWeapon			();

	// [FFT++]: аддоны и управление аддонами
	bool					bUseAltScope;
	bool					bScopeIsHasTexture;
	bool					bNVsecondVPavaible;
	bool					bNVsecondVPstatus;

	virtual	bool			bInZoomRightNow() const { return m_zoom_params.m_fZoomRotationFactor > 0.05; }
	IC		bool			bIsSecondVPZoomPresent() const { return (GetSecondVPZoomFactor() > 0.000f && !m_bAltZoomActive); }
			bool			bLoadAltScopesParams(LPCSTR section);
			bool            bReloadSectionScope(LPCSTR section);
			bool            bChangeNVSecondVPStatus();
	virtual	bool            bMarkCanShow() { return IsZoomed(); }
	virtual void			UpdateAddonsTransform(bool for_hud);


	virtual void			UpdateSecondVP(bool bInGrenade = false);
	void					Load3DScopeParams(LPCSTR section);
	void					LoadOriginalScopesParams(LPCSTR section);
	void					LoadCurrentScopeParams(LPCSTR section);
	void					LoadSilencerParams(LPCSTR section);
	void					LoadLaserDesignatorParams(LPCSTR section);
	void					LoadTacticalTorchParams(LPCSTR section);
	void					LoadGrenadeLauncherParams(LPCSTR section);
	void					GetZoomData(const float scope_factor, float& delta, float& min_zoom_factor);
	void					ZoomDynamicMod(bool bIncrement, bool bForceLimit);
	void					UpdateAltScope();
	void					UpdateAimOffsets();

	// Up
	// Magazine system & etc
	xr_vector<shared_str>	bullets_bones;
	int						bullet_cnt;
	int						last_hide_bullet;
	bool					bHasBulletsToHide;

	xr_vector<WeaponAttach*> m_weapon_attaches;

	virtual void			HUD_VisualBulletUpdate(bool force = false, int force_idx = -1);

	virtual float			GetControlInertionFactor() const;
	IC		float			GetZRotatingFactor()    const { return m_zoom_params.m_fZoomRotationFactor; }
	IC		float			GetSecondVPZoomFactor() const { return m_zoom_params.m_fSecondVPFovFactor; }
	float					GetSecondVPFov() const;

	shared_str				GetNameWithAttachment();


	float					m_fScopeInertionFactor;
	float					m_fZoomStepCount;
	float					m_fZoomMinKoeff;

	// Generic
	virtual void			Load				(LPCSTR section);

	virtual BOOL			net_Spawn			(CSE_Abstract* DC);
	virtual void			net_Destroy			();
	virtual void			net_Export			(NET_Packet& P);
	virtual void			net_Import			(NET_Packet& P);
	
	virtual CWeapon			*cast_weapon			()					{return this;}
	virtual CWeaponMagazined*cast_weapon_magazined	()					{return 0;}


	//serialization
	virtual void			save				(NET_Packet &output_packet);
	virtual void			load				(IReader &input_packet);
	virtual BOOL			net_SaveRelevant	()								{return inherited::net_SaveRelevant();}

	virtual void			UpdateCL			();
	virtual void			shedule_Update		(u32 dt);

	virtual void			renderable_Render	();
	virtual void			render_hud_mode		();
	virtual bool			need_renderable		();

	virtual void			render_item_ui		();
	virtual bool			render_item_ui_query();

	virtual void			OnH_B_Chield		();
	virtual void			OnH_A_Chield		();
	virtual void			OnH_B_Independent	(bool just_before_destroy);
	virtual void			OnH_A_Independent	();
	virtual void			OnEvent				(NET_Packet& P, u16 type);// {inherited::OnEvent(P,type);}

	virtual	void			Hit					(SHit* pHDS);


	virtual void			reinit				();
	virtual void			reload				(LPCSTR section);
	virtual void			create_physic_shell	();
	virtual void			activate_physic_shell();
	virtual void			setup_physic_shell	();

	virtual void			SwitchState			(u32 S);

	virtual void			OnActiveItem		();
	virtual void			OnHiddenItem		();
	virtual void			SendHiddenItem		();	//same as OnHiddenItem but for client... (sends message to a server)...

public:
	virtual bool			can_kill			() const;
	virtual CInventoryItem	*can_kill			(CInventory *inventory) const;
	virtual const CInventoryItem *can_kill		(const xr_vector<const CGameObject*> &items) const;
	virtual bool			ready_to_kill		() const;
	virtual bool			NeedToDestroyObject	() const; 
	virtual ALife::_TIME_ID	TimePassedAfterIndependant() const;
protected:
	//время удаления оружия
	ALife::_TIME_ID			m_dwWeaponRemoveTime;
	ALife::_TIME_ID			m_dwWeaponIndependencyTime;

	virtual bool			IsHudModeNow		();
public:
	void					signal_HideComplete	();
	virtual bool			Action(u16 cmd, u32 flags);

	enum EWeaponStates 
	{
		eFire		= eLastBaseState+1,
		eFire2,
		eReload,
		eMisfire,
		eMagEmpty,
		eSwitch,
		eUnMisfire,
		eFiremodePrev,
		eFiremodeNext,
		eLaserSwitch,
		eFlashlightSwitch,
	};
	enum EWeaponSubStates{
		eSubstateReloadBegin		=0,
		eSubstateReloadInProcess,
		eSubstateReloadEnd,
	};
	enum { undefined_ammo_type = u8(-1) };

	IC BOOL					IsValid				()	const		{	return iAmmoElapsed;						}
	// Does weapon need's update?
	BOOL					IsUpdating			();

	BOOL					IsMisfire			() const;
	BOOL					CheckForMisfire		();
	BOOL					IsEmptyMagazine		() const;


	BOOL					AutoSpawnAmmo		() const		{ return m_bAutoSpawnAmmo; };
	bool					IsTriStateReload	() const		{ return m_bTriStateReload;}
	EWeaponSubStates		GetReloadState		() const		{ return (EWeaponSubStates)m_sub_state;}
protected:
	bool					m_bTriStateReload;
	// a misfire happens, you'll need to rearm weapon
	bool					bMisfire;				

	BOOL					m_bAutoSpawnAmmo;
	virtual bool			AllowBore		();
public:
			u8   m_sub_state;
			bool IsGrenadeLauncherAttached	() const;
			bool IsScopeAttached			() const;
			bool IsSilencerAttached			() const;
			bool IsLaserAttached			() const;
			bool IsTacticalTorchAttached	() const;

	virtual bool GrenadeLauncherAttachable();
	virtual bool ScopeAttachable();
	virtual bool SilencerAttachable();
	virtual bool LaserAttachable();
	virtual bool TacticalTorchAttachable();

	ALife::EWeaponAddonStatus	get_GrenadeLauncherStatus	() const { return m_eGrenadeLauncherStatus; }
	ALife::EWeaponAddonStatus	get_ScopeStatus				() const { return m_eScopeStatus; }
	ALife::EWeaponAddonStatus	get_SilencerStatus			() const { return m_eSilencerStatus; }
	ALife::EWeaponAddonStatus	get_LaserDesignatorStatus	() const { return m_eLaserDesignatorStatus; }
	ALife::EWeaponAddonStatus	get_TacticalTorchStatus		() const { return m_eTacticalTorchStatus; }

	virtual bool UseScopeTexture() {return true;};

	//обновление видимости для косточек аддонов
			void UpdateAddonsVisibility();
			void UpdateHUDAddonsVisibility();
	//инициализация свойств присоединенных аддонов
	virtual void InitAddons();

	float		m_fLR_MovingFactor; // Фактор бокового наклона худа при ходьбе [-1; +1]
	float		m_fLR_CameraFactor; // Фактор бокового наклона худа при движении камеры [-1; +1]
	float		m_fLR_InertiaFactor; // Фактор горизонтальной инерции худа при движении камеры [-1; +1]
	float		m_fUD_InertiaFactor; // Фактор вертикальной инерции худа при движении камеры [-1; +1]
	Fvector		m_strafe_offset[4][2]; //pos,rot,data1,data2/ normal,aim-GL --#SM+#--

	//для отоброажения иконок апгрейдов в интерфейсе
	int GetScopeX();
	int GetScopeY();
	int	GetSilencerX() {return m_iSilencerX;}
	int	GetSilencerY() {return m_iSilencerY;}
	int	GetGrenadeLauncherX() {return m_iGrenadeLauncherX;}
	int	GetGrenadeLauncherY() {return m_iGrenadeLauncherY;}
	int	GetLaserDesignatorX() { return m_iLaserX; }
	int	GetLaserDesignatorY() { return m_iLaserY; }
	int	GetTacticalTorchX() { return m_iTacticalTorchX; }
	int	GetTacticalTorchY() { return m_iTacticalTorchY; }

	const shared_str& GetGrenadeLauncherName	() const{return m_sGrenadeLauncherName;}
	const shared_str GetScopeName				() const;
	const shared_str& GetSilencerName			() const{return m_sSilencerName;}
	const shared_str& GetLaserName				() const{return m_sLaserName;}
	const shared_str& GetTacticalTorchName		() const{return m_sTacticalTorchName;}

	IC void	ForceUpdateAmmo						()		{ m_BriefInfo_CalcFrame = 0; }

	u8		GetAddonsState						()		const		{return m_flagsAddOnState;};
	void	SetAddonsState						(u8 st)	{m_flagsAddOnState=st;}

	bool	IsAltAimEnabled						() const {return m_bAltZoomEnabled;}
	bool	GetAltZoomStatus					() const {return m_bAltZoomActive;}
	bool	SetAltZoomStatus					(bool status) { m_bAltZoomActive = status; }
	void	SwitchZoomMode						();

	void	ModifyUpgradeBones					(shared_str bone_name, bool show);
	void	RemoveBones							(xr_vector<shared_str>& m_all_bones, const xr_vector<shared_str>& bones_to_remove);

protected:
	//состояние подключенных аддонов
	u8 m_flagsAddOnState;

	//возможность подключения различных аддонов
	ALife::EWeaponAddonStatus	m_eScopeStatus;
	ALife::EWeaponAddonStatus	m_eSilencerStatus;
	ALife::EWeaponAddonStatus	m_eGrenadeLauncherStatus;
	ALife::EWeaponAddonStatus	m_eLaserDesignatorStatus;
	ALife::EWeaponAddonStatus	m_eTacticalTorchStatus;

	//названия секций подключаемых аддонов
	shared_str		m_sScopeName;
	shared_str		m_sSilencerName;
	shared_str		m_sGrenadeLauncherName;
	shared_str		m_sLaserName;
	shared_str		m_sTacticalTorchName;

	shared_str		m_sScopeAttachSection{};
	shared_str		m_sSilencerAttachSection{};
	shared_str		m_sLaserAttachSection{};
	shared_str		m_sTacticalTorchAttachSection{};
	shared_str		m_sGrenadeLauncherAttachSection{};

	shared_str		m_sWpn_scope_bone;
	shared_str		m_sWpn_silencer_bone;
	shared_str		m_sWpn_launcher_bone;
	shared_str		m_sWpn_laser_bone;
	shared_str		m_sWpn_flashlight_bone;
	shared_str		m_sWpn_laser_ray_bone;
	shared_str		m_sWpn_flashlight_cone_bone;
	shared_str		m_sHud_wpn_laser_ray_bone;
	shared_str		m_sHud_wpn_flashlight_cone_bone;

	xr_vector<shared_str> m_all_scope_bones;
	shared_str		m_cur_scope_bone;

	//смещение иконов апгрейдов в инвентаре
	int	m_iScopeX, m_iScopeY;
	int	m_iSilencerX, m_iSilencerY;
	int	m_iGrenadeLauncherX, m_iGrenadeLauncherY;
	int m_iLaserX, m_iLaserY;
	int m_iTacticalTorchX, m_iTacticalTorchY;

	RStringVec		m_defShownBones;
	RStringVec		m_defHiddenBones;

protected:

	struct SZoomParams
	{
		bool			m_bZoomEnabled;			//разрешение режима приближения
		bool			m_bHideCrosshairInZoom;
//		bool			m_bZoomDofEnabled;

		bool			m_bIsZoomModeNow;		//когда режим приближения включен
		float			m_fCurrentZoomFactor;	//текущий фактор приближения
		float			m_fZoomRotateTime;		//время приближения
	
		float			m_fIronSightZoomFactor;	//коэффициент увеличения прицеливания
		float			m_fScopeZoomFactor;		//коэффициент увеличения прицела
		float           m_f3dZoomFactor;        //коэффициент мирового зума при использовании второго вьюпорта

		float			m_fZoomRotationFactor;
		float           m_fSecondVPFovFactor;

//		float           m_fSecondVP_FovFactor;
		
//		Fvector			m_ZoomDof;
		Fvector4		m_ReloadDof;
		Fvector4		m_ReloadEmptyDof;
		BOOL			m_bUseDynamicZoom;
		shared_str		m_sUseZoomPostprocess;
		shared_str		m_sUseBinocularVision;
		CBinocularsVision*		m_pVision;
		CNightVisionEffector*	m_pNight_vision;

	} m_zoom_params;
	
		float			m_fFactor;
		float			m_fRTZoomFactor; //run-time zoom factor
		CUIWindow*		m_UIScope;
public:

	IC bool					IsZoomEnabled		()	const		{return m_zoom_params.m_bZoomEnabled;}
	virtual	void			ZoomInc				();
	virtual	void			ZoomDec				();
	virtual void			OnZoomIn			();
	virtual void			OnZoomOut			();
	IC		bool			IsZoomed			()	const		{return m_zoom_params.m_bIsZoomModeNow;};
	CUIWindow*				ZoomTexture			();	
			bool			IsPartlyReloading	();


			bool			ZoomHideCrosshair	();

	IC float				GetZoomFactor		() const		{return m_zoom_params.m_fCurrentZoomFactor;}
	IC void					SetZoomFactor		(float f) 		{m_zoom_params.m_fCurrentZoomFactor = f;}

	virtual	float			CurrentZoomFactor	();
	//показывает, что оружие находится в соостоянии поворота для приближенного прицеливания
			bool			IsRotatingToZoom	() const		{	return (m_zoom_params.m_fZoomRotationFactor<1.f);}
			bool			IsRotatingFromZoom	() const		{	return (m_zoom_params.m_fZoomRotationFactor>0.f);}

	virtual	u8				GetCurrentHudOffsetIdx ();

	virtual float				Weight			() const;		
	virtual	u32					Cost			() const;
public:
    virtual EHandDependence		HandDependence		()	const		{	return eHandDependence;}
			bool				IsSingleHanded		()	const		{	return m_bIsSingleHanded; }

public:
	int m_strap_bone0_id;
	int m_strap_bone1_id;
	bool m_strapped_mode_rifle;
	bool m_can_be_strapped_rifle;

	IC		LPCSTR			strap_bone0			() const {return m_strap_bone0;}
	IC		LPCSTR			strap_bone1			() const {return m_strap_bone1;}
	IC		void			strapped_mode		(bool value) {m_strapped_mode = value;}
	IC		bool			strapped_mode		() const {return m_strapped_mode;}

			Fvector			GetHandsOffsetPos	() { return m_Offset.c; }
			Fvector			GetHandsOffsetRot	() { Fvector v; m_Offset.getHPB(v); return v; };
			void			SetHandsPosRot		(Fvector pos, Fvector rot) { m_Offset.setHPB(rot.x, rot.y, rot.z); m_Offset.translate_over(pos); }

			Fvector			GetStrapOffsetPos	() { return m_StrapOffset.c; }
			Fvector			GetStrapOffsetRot	() { Fvector v; m_StrapOffset.getHPB(v); return v; };
			void			SetStrapPosRot		(Fvector pos, Fvector rot) { m_StrapOffset.setHPB(rot.x, rot.y, rot.z); m_StrapOffset.translate_over(pos); }

			Fvector			GetStrapOffsetAltPos() { return m_StrapOffset_alt.c; }
			Fvector			GetStrapOffsetAltRot() { Fvector v; m_StrapOffset_alt.getHPB(v); return v; }
			void			SetStrapAltPosRot	(Fvector pos, Fvector rot) { m_StrapOffset_alt.setHPB(rot.x, rot.y, rot.z); m_StrapOffset_alt.translate_over(pos); }

			void			SetStrapBone0		(LPCSTR bone) { m_strap_bone0 = bone; }
			void			SetStrapBone1		(LPCSTR bone) { m_strap_bone1 = bone; }

			void			SetStrapBone0_ID	(int bone_id) { m_strap_bone0_id = bone_id; }
			void			SetStrapBone1_ID	(int bone_id) { m_strap_bone1_id = bone_id; }

protected:
	LPCSTR					m_strap_bone0;
	LPCSTR					m_strap_bone1;
	Fmatrix					m_StrapOffset;
	Fmatrix                 m_StrapOffset_alt;

	bool					m_strapped_mode;
	bool					m_can_be_strapped;
	bool					m_freelook_switch_back;

	Fmatrix					m_Offset;

	// 0-используется без участия рук, 1-одна рука, 2-две руки
	EHandDependence			eHandDependence;
	bool					m_bIsSingleHanded;
	bool					m_bUseAimAnmDirDependency;
	bool					m_bUseScopeAimMoveAnims;
	bool					m_bAltZoomEnabled;
	bool					m_bAltZoomActive;
public:
	//загружаемые параметры
	Fvector					vLoadedFirePoint;
	Fvector					vLoadedFirePoint2;

private:
	firedeps				m_current_firedeps;

protected:
	virtual void			UpdateFireDependencies_internal	();
	virtual void            UpdatePosition          (const Fmatrix& transform);
	virtual void            UpdatePosition_alt      (const Fmatrix& transform);
	virtual void			UpdateXForm				();
	virtual void			UpdateHudAdditional		(Fmatrix&);
	IC		void			UpdateFireDependencies	()			{ if (m_dwFP_Frame==Device.dwFrame) return; UpdateFireDependencies_internal(); };

	virtual void			LoadFireParams		(LPCSTR section);
public:	
	IC		const Fvector&	get_LastFP				()			{ UpdateFireDependencies(); return m_current_firedeps.vLastFP;	}
	IC		const Fvector&	get_LastFP2				()			{ UpdateFireDependencies(); return m_current_firedeps.vLastFP2;	}
	IC		const Fvector&	get_LastFD				()			{ UpdateFireDependencies(); return m_current_firedeps.vLastFD;	}
	IC		const Fvector&	get_LastSP				()			{ UpdateFireDependencies(); return m_current_firedeps.vLastSP;	}

	virtual const Fvector&	get_CurrentFirePoint	()			{ return get_LastFP();				}
	virtual const Fvector&	get_CurrentFirePoint2	()			{ return get_LastFP2();				}
	virtual const Fmatrix&	get_ParticlesXFORM		()			{ UpdateFireDependencies(); return m_current_firedeps.m_FireParticlesXForm;	}
	virtual void			ForceUpdateFireParticles();
	virtual void			debug_draw_firedeps		();

private:
	string64 guns_aim_anm;
protected:
	virtual void			SetDefaults				();
	
	virtual bool			MovingAnimAllowedNow	();
	virtual bool			IsMisfireNow			();
	virtual bool			IsMagazineEmpty			();
	virtual void			OnStateSwitch			(u32 S);
	virtual void			OnAnimationEnd			(u32 state);

	const char*				GetAnimAimName			();
	const char*				GenerateAimAnimName		(string64 base_anim);

	//трассирование полета пули
	virtual	void			FireTrace(const Fvector& P, const Fvector& D);
	virtual float			GetWeaponDeterioration();
public:
	virtual void			FireStart();
	virtual void			FireEnd();

	virtual void			Reload();
	void					StopShooting();

	// обработка визуализации выстрела
	virtual void			OnShot() {};
	virtual void			AddShotEffector();
	virtual void			RemoveShotEffector();
	virtual	void			ClearShotEffector();
	virtual	void			StopShotEffector();

	float					GetBaseDispersion(float cartridge_k);
	float					GetFireDispersion(bool with_cartridge, bool for_crosshair = false);
	virtual float			GetFireDispersion(float cartridge_k, bool for_crosshair = false);
	virtual	int				ShotsFired() { return 0; }
	virtual	int				GetCurrentFireMode() { return 1; }

	//параметы оружия в зависимоти от его состояния исправности
	float					GetConditionDispersionFactor	() const;
	float					GetConditionMisfireProbability	() const;
	virtual	float			GetConditionToShow				() const;

	struct BlendCamParams
	{
		shared_str name	= nullptr;
		float speed		= 1.0f;
		float power		= 1.0f;
	};

	void					ProcessBlendCamParams			(LPCSTR params, BlendCamParams& cam_params);

public:
	CameraRecoil			cam_recoil;			// simple mode (walk, run)
	CameraRecoil			zoom_cam_recoil;	// using zoom =(ironsight or scope)

protected:
	//фактор увеличения дисперсии при максимальной изношености 
	//(на сколько процентов увеличится дисперсия)
	float					fireDispersionConditionFactor;
	//вероятность осечки при максимальной изношености

	// modified by Peacemaker [17.10.08]
	float					misfireProbability;
	float					misfireConditionK;
	float					misfireStartCondition;			//изношенность, при которой появляется шанс осечки
	float					misfireEndCondition;			//изношеность при которой шанс осечки становится константным
	float					misfireStartProbability;		//шанс осечки при изношености больше чем misfireStartCondition
	float					misfireEndProbability;			//шанс осечки при изношености больше чем misfireEndCondition
	float					conditionDecreasePerQueueShot;	//увеличение изношености при выстреле очередью
	float					conditionDecreasePerShot;		//увеличение изношености при одиночном выстреле
	float					conditionDecreasePerShotOnHit;

	BlendCamParams			m_BlendAimStartCam{};
	BlendCamParams			m_BlendAimEndCam{};
	BlendCamParams			m_BlendAimIdleCam{};
	BlendCamParams			m_BlendFakeShootCam{};
public:
	float GetMisfireStartCondition	() const {return misfireStartCondition;};
	float GetMisfireEndCondition	() const {return misfireEndCondition;};

protected:
	struct SPDM
	{
		float					m_fPDM_disp_base			;
		float					m_fPDM_disp_vel_factor		;
		float					m_fPDM_disp_accel_factor	;
		float					m_fPDM_disp_crouch			;
		float					m_fPDM_disp_crouch_no_acc	;
	};
	SPDM					m_pdm;
	
	float					m_crosshair_inertion;
	first_bullet_controller	m_first_bullet_controller;
protected:
	//для отдачи оружия
	Fvector					m_vRecoilDeltaAngle;

	//для сталкеров, чтоб они знали эффективные границы использования 
	//оружия
	float					m_fMinRadius;
	float					m_fMaxRadius;

protected:
	//для второго ствола
	void			StartFlameParticles2();
	void			StopFlameParticles2();
	void			UpdateFlameParticles2();
protected:
	shared_str				m_sFlameParticles2;
	//объект партиклов для стрельбы из 2-го ствола
	CParticlesObject*		m_pFlameParticles2;

public:
	// Alundaio
	int						GetAmmoCount_forType	(shared_str const& ammo_type) const;
	virtual void			set_ef_main_weapon_type	(u32 type) { m_ef_main_weapon_type = type; };
	virtual void			set_ef_weapon_type		(u32 type) { m_ef_weapon_type = type; };
	virtual void			SetAmmoType				(u8 type) { m_ammoType = type; };
	u8						GetAmmoType				() { return m_ammoType; };
	//-Alundaio

	   int					GetAmmoCount		(u8 ammo_type) const;
	IC int					GetAmmoElapsed		()	const		{	return /*int(m_magazine.size())*/iAmmoElapsed;}
	IC int					GetAmmoMagSize		()	const		{	return iMagazineSize;						}
	int						GetSuitableAmmoTotal(bool use_item_to_spawn = false) const;

	void					SetAmmoElapsed		(int ammo_count);

	virtual void			OnMagazineEmpty		();
			void			SpawnAmmo			(u32 boxCurr = 0xffffffff, 
													LPCSTR ammoSect = NULL, 
													u32 ParentID = 0xffffffff);
			bool			SwitchAmmoType		(u32 flags);

	virtual	float			Get_PDM_Base		()	const	{ return m_pdm.m_fPDM_disp_base			; };
	virtual	float			Get_PDM_Vel_F		()	const	{ return m_pdm.m_fPDM_disp_vel_factor		; };
	virtual	float			Get_PDM_Accel_F		()	const	{ return m_pdm.m_fPDM_disp_accel_factor	; };
	virtual	float			Get_PDM_Crouch		()	const	{ return m_pdm.m_fPDM_disp_crouch			; };
	virtual	float			Get_PDM_Crouch_NA	()	const	{ return m_pdm.m_fPDM_disp_crouch_no_acc	; };
	virtual	float			GetCrosshairInertion()	const	{ return m_crosshair_inertion; };
	virtual bool			IsNecessaryItem		(const shared_str& item_sect_);
	bool					IsNecessaryItem		(const shared_str& item_sect_, xr_vector<shared_str> item);

			float			GetFirstBulletDisp	()	const	{ return m_first_bullet_controller.get_fire_dispertion(); };
protected:
	int						iAmmoElapsed;		// ammo in magazine, currently
	int						iMagazineSize;		// size (in bullets) of magazine

	//для подсчета в GetSuitableAmmoTotal
	mutable int				m_iAmmoCurrentTotal;
	mutable u32				m_BriefInfo_CalcFrame;	//кадр на котором просчитали кол-во патронов
	bool					m_bAmmoWasSpawned;

public:
	xr_vector<shared_str>	m_ammoTypes;

	DEFINE_VECTOR(shared_str, SCOPES_VECTOR, SCOPES_VECTOR_IT);
	SCOPES_VECTOR			m_scopes;
	u8						m_cur_scope;

	CWeaponAmmo*			m_pCurrentAmmo;
	u8						m_ammoType;
//-	shared_str				m_ammoName; <== deleted
	bool					m_bHasTracers;
	bool					m_bShowWpnStats;
	bool					m_bEnableBoreDof;
	bool					m_bUseAimSilShotAnim;
	u8						m_u8TracerColorID;
	u8						m_set_next_ammoType_on_reload;
	// Multitype ammo support
	xr_vector<CCartridge>	m_magazine;
	CCartridge				m_DefaultCartridge;
	float					m_fCurrentCartirdgeDisp;

	Fmatrix					m_scopeAttachTransform{};

		bool				unlimited_ammo				();
	IC	bool				can_be_strapped				() const {return m_can_be_strapped;};
	float					GetMagazineWeight			(const decltype(m_magazine)& mag) const;

	xr_vector<shared_str>	m_SuitableRepairKits;
	xr_vector<std::pair<shared_str, int>> m_ItemsForRepair;
protected:
	u32						m_ef_main_weapon_type;
	u32						m_ef_weapon_type;

public:
	virtual u32				ef_main_weapon_type	() const;
	virtual u32				ef_weapon_type		() const;

protected:
	// This is because when scope is attached we can't ask scope for these params
	// therefore we should hold them by ourself :-((
	float					m_addon_holder_range_modifier;
	float					m_addon_holder_fov_modifier;

public:
	virtual	void			modify_holder_params		(float &range, float &fov) const;
	virtual bool			use_crosshair				()	const {return true;}
			bool			show_crosshair				();
			bool			show_indicators				();
	virtual BOOL			ParentMayHaveAimBullet		();
	virtual bool			ParentIsActor				();
	
private:
	virtual	bool			install_upgrade_ammo_class	( LPCSTR section, bool test );
			bool			install_upgrade_disp		( LPCSTR section, bool test );
			bool			install_upgrade_hit			( LPCSTR section, bool test );
			bool			install_upgrade_addon		( LPCSTR section, bool test );
			bool			install_upgrade_other		( LPCSTR section, bool test );
protected:
	virtual bool			install_upgrade_impl		( LPCSTR section, bool test );

private:
	float					m_hit_probability[egdCount];

public:
	const float				&hit_probability			() const;

private:
	Fvector					m_overriden_activation_speed;
	bool					m_activation_speed_is_overriden;
	virtual bool			ActivationSpeedOverriden	(Fvector& dest, bool clear_override);

	bool					m_bRememberActorNVisnStatus;

public:
	virtual void			SetActivationSpeedOverride	(Fvector const& speed);
			bool			GetRememberActorNVisnStatus	() {return m_bRememberActorNVisnStatus;};
	virtual void			EnableActorNVisnAfterZoom	();
	
	virtual void				DumpActiveParams			(shared_str const & section_name, CInifile & dst_ini) const;
	virtual shared_str const	GetAnticheatSectionName		() const { return cNameSect(); };
	virtual void				OnBulletHit();

	virtual void processing_deactivate() override 
	{
		UpdateLaser();
		UpdateFlashlight();
		inherited::processing_deactivate();
	}

	void GetBoneOffsetPosDir(const shared_str& bone_name, Fvector& dest_pos, Fvector& dest_dir, const Fvector& offset);
	//Функция из ганслингера для приблизительной коррекции разности фовов худа и мира. Так себе на самом деле, но более годных способов я не нашел.
	void CorrectDirFromWorldToHud(Fvector& dir);

private:
	float hud_recalc_koef;

	bool has_laser;
	shared_str laserdot_attach_bone;
	Fvector laserdot_attach_offset, laserdot_world_attach_offset;
	ref_light laser_light_render;
	CLAItem* laser_lanim;
	float laser_fBrightness{ 1.f };

	void UpdateLaser();
public:
	void SwitchLaser(bool on);
	
	inline bool IsLaserOn() const 
	{
		return (m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonLaserOn && (m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonLaserDesignator || m_eLaserDesignatorStatus == ALife::eAddonPermanent));
	}

private:
	bool has_flashlight;
	shared_str flashlight_attach_bone;
	Fvector flashlight_attach_offset, flashlight_omni_attach_offset, flashlight_world_attach_offset, flashlight_omni_world_attach_offset;
	ref_light flashlight_render;
	ref_light flashlight_omni;
	ref_glow flashlight_glow;
	CLAItem* flashlight_lanim;
	float flashlight_fBrightness{ 1.f };

	void UpdateFlashlight();
public:
	void SwitchFlashlight(bool on);

	inline bool IsFlashlightOn() const 
	{
		return (m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonFlashlightOn && (m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonTacticalTorch || m_eTacticalTorchStatus == ALife::eAddonPermanent));
	}
};
