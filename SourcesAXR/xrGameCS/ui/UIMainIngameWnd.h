#ifndef	UI_MAIN_INGAME_WND_H_INCLUDED
#define UI_MAIN_INGAME_WND_H_INCLUDED

#include "UIProgressBar.h"
#include "UIGameLog.h"
#include "UICarPanel.h"
#include "UIMotionIcon.h"
#include "UIZoneMap.h"

#include "../hudsound.h"
#include "../../XrServerEntitiesCS/alife_space.h"
#include "../EntityCondition.h"

class	CUIPdaMsgListItem;
class	CLAItem;
class	CUIZoneMap;
class	CUIScrollView;
struct	GAME_NEWS_DATA;
class	CActor;
class	CWeapon;
class	CMissile;
class	CInventoryItem;
class	CUIHudStatesWnd;
class	CUICellItem;
class	CUIArtefactPanel;

class CUIMainIngameWnd: public CUIWindow  
{
public:
			CUIMainIngameWnd();
	virtual ~CUIMainIngameWnd();

	virtual void Init();
	virtual void Draw();
	virtual void Update();

			bool OnKeyboardPress(int dik);
	
	CUIStatic			UIStaticDiskIO;
//	CUIStatic			UIStaticHealth;
//	CUIStatic			UIStaticArmor;
	CUIStatic			UIStaticQuickHelp;
//	CUIProgressBar		UIHealthBar;
//	CUIProgressBar		UIArmorBar;
//	CUICarPanel			UICarPanel;
	CUIMotionIcon		UIMotionIcon;
	CUIZoneMap*			UIZoneMap;
	CUIArtefactPanel*	UIArtefactsPanel;

	CUIStatic*			m_ind_temperature;
	u32					m_min_temperature_clr, m_mid_temperature_clr, m_max_temperature_clr;

	CUIStatic*			m_ind_weather_type;

	CUIStatic*			m_ind_boost_psy;
	CUIStatic*			m_ind_boost_radia;
	CUIStatic*			m_ind_boost_chem;
	CUIStatic*			m_ind_boost_wound;
	CUIStatic*			m_ind_boost_weight;
	CUIStatic*			m_ind_boost_health;
	CUIStatic*			m_ind_boost_power;
	CUIStatic*			m_ind_boost_rad;
	CUIStatic*			m_ind_boost_satiety;
	CUIStatic*			m_ind_boost_thirst;
	CUIStatic*			m_ind_boost_psy_health;
	CUIStatic*			m_ind_boost_intoxication;
	CUIStatic*			m_ind_boost_sleepeness;
	CUIStatic*			m_ind_boost_alcoholism;
	CUIStatic*			m_ind_boost_hangover;
	CUIStatic*			m_ind_boost_narcotism;
	CUIStatic*			m_ind_boost_withdrawal;
	CUIStatic*			m_ind_boost_frostbite;

	//иконка, показывающая количество активных PDA
//	CUIStatic			UIPdaOnline;
	
	CUIHudStatesWnd*	m_ui_hud_states;

	CUIStatic*	m_ind_sleepeness;
	CUIStatic*	m_ind_alcoholism;
	CUIStatic*	m_ind_narcotism;
	CUIStatic*	m_ind_psy_health;
	CUIStatic*	m_ind_filter_dirty;
	CUIStatic*	m_ind_lfo_filter_dirty;
	CUIStatic*	m_ind_battery;
	CUIStatic*	m_ind_battery_torch;
	CUIStatic*	m_ind_battery_artefact_detector;
	CUIStatic*	m_ind_battery_anomaly_detector;
	CUIStatic*	m_ind_intoxication;
	CUIStatic*	m_ind_bleeding;
	CUIStatic*	m_ind_radiation;
	CUIStatic*	m_ind_starvation;
	CUIStatic*	m_ind_infection;
	CUIStatic*	m_ind_frostbite;
	CUIStatic*	m_ind_thirst;
	CUIStatic*	m_ind_weapon_broken;
	CUIStatic*	m_ind_helmet_broken;
	CUIStatic*	m_ind_outfit_broken;
	CUIStatic*	m_ind_overweight;
	CUIStatic*	m_ind_health;
	CUIStatic*	m_clock_value;

	IC	void			ShowZoneMap( bool status ) { UIZoneMap->visible = status; }
		void			DrawZoneMap() { UIZoneMap->Render(); }
		void			UpdateZoneMap() { UIZoneMap->Update(); }
		void			DrawMainIndicatorsForInventory();
	
	CUIHudStatesWnd*	get_hud_states() { return m_ui_hud_states; } //temp
	void				OnSectorChanged			(int sector);


	xr_vector<CUIStatic* > m_quick_slots_icons;
	CUIStatic* m_QuickSlotText1;
	CUIStatic* m_QuickSlotText2;
	CUIStatic* m_QuickSlotText3;
	CUIStatic* m_QuickSlotText4;
	CUIStatic* m_QuickSlotText5;
	CUIStatic* m_QuickSlotText6;

	float				hud_info_x;
	float				hud_info_y;

	CGameFont*			m_HudInfoFont;

	float				hud_info_item_x;
	Fvector3			hud_info_item_y_pos;

	Ivector4			hud_info_n;
	Ivector4			hud_info_e;
	Ivector4			hud_info_f;

	Ivector4			ch_info_n;
	Ivector4			ch_info_e;
	Ivector4			ch_info_f;

protected:

	// 5 статиков для отображения иконок:
	// - сломанного оружия
	// - радиации
	// - ранения
	// - голода
	// - усталости
	CUIStatic			UIWeaponJammedIcon;
//	CUIStatic			UIRadiaitionIcon;
//	CUIStatic			UIWoundIcon;
	CUIStatic			UIStarvationIcon;
//	CUIStatic			UIPsyHealthIcon;
	CUIStatic			UIInvincibleIcon;
//	CUIStatic			UISleepIcon;
	CUIStatic			UIArtefactIcon;
	CUIStatic			UIFrostbiteIcon;
	CUIStatic			UIHeatingIcon;

	CUIScrollView*		m_UIIcons;
	CUIWindow*			m_pMPChatWnd;
	CUIWindow*			m_pMPLogWnd;

public:
	
	// Енумы соответсвующие предупреждающим иконкам 
	enum EWarningIcons
	{
		ewiAll				= 0,
		ewiWeaponJammed,
//		ewiRadiation,
//		ewiWound,
		ewiFrostbite,
		ewiStarvation,
//		ewiPsyHealth,
//		ewiSleep,
		ewiHeating,
		ewiInvincible,
		ewiArtefact,
	};

	void				SetMPChatLog					(CUIWindow* pChat, CUIWindow* pLog);

	// Задаем цвет соответствующей иконке
	void				SetWarningIconColor				(EWarningIcons icon, const u32 cl);
	void				TurnOffWarningIcon				(EWarningIcons icon);

	// Пороги изменения цвета индикаторов, загружаемые из system.ltx
	typedef				xr_map<EWarningIcons, xr_vector<float> >	Thresholds;
	typedef				Thresholds::iterator						Thresholds_it;
	Thresholds			m_Thresholds;

	// Енум перечисления возможных мигающих иконок
	enum EFlashingIcons
	{
		efiPdaTask	= 0,
		efiEncyclopedia = 1,
		efiJournal = 2,
		efiMail
	};
	
	void				SetFlashIconState_				(EFlashingIcons type, bool enable);

	void				AnimateContacts					(bool b_snd);
	HUD_SOUND_ITEM		m_contactSnd;

	void				ReceiveNews						(GAME_NEWS_DATA* news);
	void				UpdateMainIndicators			();
	void				UpdateBoosterIndicators			(const xr_map<EBoostParams, SBooster> influences);

protected:
	void				UpdateQuickSlots				();
	void				SetWarningIconColorUI			(CUIStatic* s, const u32 cl);
	void				InitFlashingIcons				(CUIXml* node);
	void				DestroyFlashingIcons			();
	void				UpdateFlashingIcons				();
//	void				UpdateActiveItemInfo			();

//	void				SetAmmoIcon						(const shared_str& sect_name);

	// first - иконка, second - анимация
	DEF_MAP				(FlashingIcons, EFlashingIcons, CUIStatic*);

	FlashingIcons		m_FlashingIcons;

//	CWeapon*			m_pWeapon;
	CMissile*			m_pGrenade;
	CInventoryItem*		m_pItem;

	// Отображение подсказок при наведении прицела на объект
	void				RenderQuickInfos();

public:
//	CUICarPanel&		CarPanel							(){return UICarPanel;};
	CUIMotionIcon&		MotionIcon							(){return UIMotionIcon;}
	void				OnConnected							();
	void				reset_ui							();

protected:
	CInventoryItem*		m_pPickUpItem;
	float				fuzzyShowInfo_;
	CUICellItem*		uiPickUpItemIconNew_;
	float				m_iPickUpItemIconX;
	float				m_iPickUpItemIconY;
	float				m_iPickUpItemIconScale;
public:
	void				SetPickUpItem	(CInventoryItem* PickUpItem);
#ifdef DEBUG
	void				draw_adjust_mode					();
#endif
};

#endif // UI_MAIN_INGAME_WND_H_INCLUDED
