// UIMainIngameWnd.h:  окошки-информация в игре
// 
//////////////////////////////////////////////////////////////////////

#pragma once

#include "UIProgressBar.h"
#include "UIGameLog.h"

#include "alife_space.h"

#include "UICarPanel.h"
#include "UIMotionIcon.h"
#include "../hudsound.h"
#include "../EntityCondition.h"

class					CUIPdaMsgListItem;
class					CLAItem;
class					CUIZoneMap;
class					CUIArtefactPanel;
class					CUIScrollView;
struct					GAME_NEWS_DATA;
class					CActor;
class					CWeapon;
class					CMissile;
class					CInventoryItem;
class					CUIHudStatesWnd;
class					CUICellItem;

class CUIMainIngameWnd: public CUIWindow  
{
public:
	CUIMainIngameWnd();
	virtual ~CUIMainIngameWnd();

	virtual void Init();
	virtual void Draw();
	virtual void Update();

	bool OnKeyboardPress(int dik);

protected:
	
	CUIStatic			UIStaticDiskIO;
	CUIStatic			UIStaticHealth;
	CUIStatic			UIStaticArmor;
	CUIStatic			UIStaticQuickHelp;
	CUIProgressBar		UIHealthBar;
	CUIProgressBar		UIArmorBar;
	CUICarPanel			UICarPanel;
	CUIMotionIcon		UIMotionIcon;	
	CUIZoneMap*			UIZoneMap;

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

	//иконка, показывающая количество активных PDA
	CUIStatic			UIPdaOnline;
	
	//изображение оружия
	CUIStatic			UIWeaponBack;
	CUIStatic			UIWeaponSignAmmo;
	CUIStatic			UIWeaponIcon;
	Frect				UIWeaponIcon_rect;
public:
	void				DrawMainIndicatorsForInventory	();
	CUIStatic*			GetPDAOnline					() { return &UIPdaOnline; };

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
	CUIStatic			UIRadiaitionIcon;
	CUIStatic			UIWoundIcon;
	CUIStatic			UIStarvationIcon;
	CUIStatic			UIPsyHealthIcon;
	CUIStatic			UIInvincibleIcon;
//	CUIStatic			UISleepIcon;
	CUIStatic			UIArtefactIcon;
	CUIStatic			UIFrostbiteIcon;
	CUIStatic			UIHeatingIcon;

	CUIScrollView*		m_UIIcons;
	CUIWindow*			m_pMPChatWnd;
	CUIWindow*			m_pMPLogWnd;
public:	
	CUIArtefactPanel*    m_artefactPanel;
	
public:
	
	// Енумы соответсвующие предупреждающим иконкам 
	enum EWarningIcons
	{
		ewiAll				= 0,
		ewiWeaponJammed,
		ewiRadiation,
		ewiWound,
		ewiFrostbite,
		ewiStarvation,
		ewiPsyHealth,
		ewiInvincible,
//		ewiSleep,
		ewiHeating,
		ewiArtefact,
	};

	void				SetMPChatLog					(CUIWindow* pChat, CUIWindow* pLog);

	void				UpdateBoosterIndicators			(const xr_map<EBoostParams, SBooster> influences);

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
		efiPdaTask = 0,
		efiEncyclopedia = 1,
		efiJournal = 2,
		efiMail
	};
	
	void				SetFlashIconState_				(EFlashingIcons type, bool enable);

	void				AnimateContacts					(bool b_snd);
	HUD_SOUND_ITEM		m_contactSnd;

	void				ReceiveNews						(GAME_NEWS_DATA* news);
	
protected:
	void				SetWarningIconColor				(CUIStatic* s, const u32 cl);
	void				InitFlashingIcons				(CUIXml* node);
	void				DestroyFlashingIcons			();
	void				UpdateFlashingIcons				();
	void				UpdateActiveItemInfo			();

	void				SetAmmoIcon						(const shared_str& seсt_name);

	// first - иконка, second - анимация
	DEF_MAP				(FlashingIcons, EFlashingIcons, CUIStatic*);
	FlashingIcons		m_FlashingIcons;

	//для текущего активного актера и оружия
	CActor*				m_pActor;	
	CWeapon*			m_pWeapon;
	CMissile*			m_pGrenade;
	CInventoryItem*		m_pItem;

	// Отображение подсказок при наведении прицела на объект
	void				RenderQuickInfos();

public:
	CUICarPanel&		CarPanel							(){return UICarPanel;};
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
};