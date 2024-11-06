#include "stdafx.h"
#include <functional>

#include "UIMainIngameWnd.h"
#include "UIMessagesWindow.h"
#include "../UIZoneMap.h"


#include <dinput.h>
#include "../actor.h"
#include "../HUDManager.h"
#include "../PDA.h"
#include "character_info.h"
#include "../inventory.h"
#include "../UIGameSP.h"
#include "../weaponmagazined.h"
#include "../missile.h"
#include "../Grenade.h"
#include "xrServer_objects_ALife.h"
#include "../alife_simulator.h"
#include "../alife_object_registry.h"
#include "../game_cl_base.h"
#include "../level.h"
#include "../seniority_hierarchy_holder.h"

#include "../date_time.h"
#include "xrServer_Objects_ALife_Monsters.h"
#include "../../xrEngine/LightAnimLibrary.h"

#include "../../Include/xrRender/RenderVisual.h"
#include "../../Include/xrRender/Kinematics.h"
#include "../../Include/xrRender/KinematicsAnimated.h"

#include "UIInventoryUtilities.h"


#include "UIXmlInit.h"
#include "UIPdaMsgListItem.h"
#include "../alife_registry_wrappers.h"
#include "../actorcondition.h"

#include "../string_table.h"
#include "clsid_game.h"
#include "UIArtefactPanel.h"

#ifdef DEBUG
#	include "../attachable_item.h"
#	include "../../xrEngine/xr_input.h"
#endif

#include "UIScrollView.h"
#include "map_hint.h"
#include "UIColorAnimatorWrapper.h"
#include "../game_news.h"

#include "AdvancedXrayGameConstants.h"

#include "UIStatic.h"
#include "UIHelper.h"
#include "UICellItem.h"
#include "UICellItemFactory.h"

using namespace InventoryUtilities;

const u32	g_clWhite					= 0xffffffff;

#define		DEFAULT_MAP_SCALE			1.f

#define		C_SIZE						0.025f
#define		NEAR_LIM					0.5f

#define		SHOW_INFO_SPEED				0.5f
#define		HIDE_INFO_SPEED				10.f
#define		C_ON_ENEMY					color_xrgb(0xff,0,0)
#define		C_DEFAULT					color_xrgb(0xff,0xff,0xff)

#define				MAININGAME_XML				"maingame.xml"

CUIMainIngameWnd::CUIMainIngameWnd()
{
	m_pActor					= NULL;
	m_pWeapon					= NULL;
	m_pGrenade					= NULL;
	m_pItem						= NULL;
	UIZoneMap					= xr_new<CUIZoneMap>();
	m_pPickUpItem				= NULL;
	m_artefactPanel				= xr_new<CUIArtefactPanel>();
	m_pMPChatWnd				= NULL;
	m_pMPLogWnd					= NULL;	
	uiPickUpItemIconNew_		= NULL;	
	fuzzyShowInfo_				= 0.f;
}

#include "UIProgressShape.h"
extern CUIProgressShape* g_MissileForceShape;

CUIMainIngameWnd::~CUIMainIngameWnd()
{
	DestroyFlashingIcons		();
	xr_delete					(UIZoneMap);

	if (m_artefactPanel)
		xr_delete				(m_artefactPanel);

	xr_delete					(g_MissileForceShape);
}

void CUIMainIngameWnd::Init()
{
	CUIXml						uiXml;
	uiXml.Load					(CONFIG_PATH, UI_PATH, MAININGAME_XML);
	
	CUIXmlInit					xml_init;
	CUIWindow::Init				(0,0, UI_BASE_WIDTH, UI_BASE_HEIGHT);

	Enable(false);


	AttachChild					(&UIStaticHealth);
	xml_init.InitStatic			(uiXml, "static_health", 0, &UIStaticHealth);

	AttachChild					(&UIStaticArmor);
	xml_init.InitStatic			(uiXml, "static_armor", 0, &UIStaticArmor);

	AttachChild					(&UIWeaponBack);
	xml_init.InitStatic			(uiXml, "static_weapon", 0, &UIWeaponBack);

	UIWeaponBack.AttachChild	(&UIWeaponSignAmmo);
	xml_init.InitStatic			(uiXml, "static_ammo", 0, &UIWeaponSignAmmo);
	//UIWeaponSignAmmo.SetElipsis	(CUIStatic::eepEnd, 2);

	UIWeaponBack.AttachChild	(&UIWeaponIcon);
	xml_init.InitStatic			(uiXml, "static_wpn_icon", 0, &UIWeaponIcon);
	UIWeaponIcon.SetShader		(GetEquipmentIconsShader());
	UIWeaponIcon_rect			= UIWeaponIcon.GetWndRect();
	//---------------------------------------------------------
	m_iPickUpItemIconX			= uiXml.ReadAttribFlt("pick_up_item", 0, "x", 512.f);
	m_iPickUpItemIconY			= uiXml.ReadAttribFlt("pick_up_item", 0, "y", 550.f);

	m_iPickUpItemIconScale		= uiXml.ReadAttribFlt("pick_up_item", 0, "scale", 1.f);
	//---------------------------------------------------------


	UIWeaponIcon.Enable			(false);

	//индикаторы 
	UIZoneMap->Init				();
	UIZoneMap->SetScale			(DEFAULT_MAP_SCALE);

	if(IsGameTypeSingle())
	{
		xml_init.InitStatic					(uiXml, "static_pda_online", 0, &UIPdaOnline);
		UIZoneMap->Background().AttachChild	(&UIPdaOnline);
	}


	//Полоса прогресса здоровья
	UIStaticHealth.AttachChild	(&UIHealthBar);
//.	xml_init.InitAutoStaticGroup(uiXml,"static_health", &UIStaticHealth);
	xml_init.InitProgressBar	(uiXml, "progress_bar_health", 0, &UIHealthBar);

	//Полоса прогресса армора
	UIStaticArmor.AttachChild	(&UIArmorBar);
//.	xml_init.InitAutoStaticGroup(uiXml,"static_armor", &UIStaticArmor);
	xml_init.InitProgressBar	(uiXml, "progress_bar_armor", 0, &UIArmorBar);

	

	// Подсказки, которые возникают при наведении прицела на объект
	AttachChild					(&UIStaticQuickHelp);
	xml_init.InitStatic			(uiXml, "quick_info", 0, &UIStaticQuickHelp);

	uiXml.SetLocalRoot			(uiXml.GetRoot());

	m_UIIcons					= xr_new<CUIScrollView>(); m_UIIcons->SetAutoDelete(true);
	xml_init.InitScrollView		(uiXml, "icons_scroll_view", 0, m_UIIcons);
	AttachChild					(m_UIIcons);

	m_ind_boost_psy			= UIHelper::CreateStatic(uiXml, "indicator_booster_psy", this);
	m_ind_boost_radia		= UIHelper::CreateStatic(uiXml, "indicator_booster_radia", this);
	m_ind_boost_chem		= UIHelper::CreateStatic(uiXml, "indicator_booster_chem", this);
	m_ind_boost_wound		= UIHelper::CreateStatic(uiXml, "indicator_booster_wound", this);
	m_ind_boost_weight		= UIHelper::CreateStatic(uiXml, "indicator_booster_weight", this);
	m_ind_boost_health		= UIHelper::CreateStatic(uiXml, "indicator_booster_health", this);
	m_ind_boost_power		= UIHelper::CreateStatic(uiXml, "indicator_booster_power", this);
	m_ind_boost_rad			= UIHelper::CreateStatic(uiXml, "indicator_booster_rad", this);

	m_ind_boost_satiety		= UIHelper::CreateStatic(uiXml, "indicator_booster_satiety", this);
	m_ind_boost_thirst		= UIHelper::CreateStatic(uiXml, "indicator_booster_thirst", this);
	m_ind_boost_psy_health	= UIHelper::CreateStatic(uiXml, "indicator_booster_psy_health", this);
	m_ind_boost_intoxication = UIHelper::CreateStatic(uiXml, "indicator_booster_intoxication", this);
	m_ind_boost_sleepeness	= UIHelper::CreateStatic(uiXml, "indicator_booster_sleepeness", this);
	m_ind_boost_alcoholism	= UIHelper::CreateStatic(uiXml, "indicator_booster_alcoholism", this);
	m_ind_boost_hangover	= UIHelper::CreateStatic(uiXml, "indicator_booster_hangover", this);
	m_ind_boost_narcotism	= UIHelper::CreateStatic(uiXml, "indicator_booster_narcotism", this);
	m_ind_boost_withdrawal	= UIHelper::CreateStatic(uiXml, "indicator_booster_withdrawal", this);

	m_ind_boost_psy			->Show(false);
	m_ind_boost_radia		->Show(false);
	m_ind_boost_chem		->Show(false);
	m_ind_boost_wound		->Show(false);
	m_ind_boost_weight		->Show(false);
	m_ind_boost_health		->Show(false);
	m_ind_boost_power		->Show(false);
	m_ind_boost_rad			->Show(false);
	m_ind_boost_satiety		->Show(false);
	m_ind_boost_thirst		->Show(false);
	m_ind_boost_psy_health	->Show(false);
	m_ind_boost_intoxication->Show(false);
	m_ind_boost_sleepeness	->Show(false);
	m_ind_boost_alcoholism	->Show(false);
	m_ind_boost_hangover	->Show(false);
	m_ind_boost_narcotism	->Show(false);
	m_ind_boost_withdrawal	->Show(false);

	// Загружаем иконки 
	if(IsGameTypeSingle())
	{
		xml_init.InitStatic		(uiXml, "starvation_static", 0, &UIStarvationIcon);
		UIStarvationIcon.Show	(false);

		xml_init.InitStatic		(uiXml, "psy_health_static", 0, &UIPsyHealthIcon);
		UIPsyHealthIcon.Show	(false);
	}

	xml_init.InitStatic			(uiXml, "weapon_jammed_static", 0, &UIWeaponJammedIcon);
	UIWeaponJammedIcon.Show		(false);

	xml_init.InitStatic			(uiXml, "radiation_static", 0, &UIRadiaitionIcon);
	UIRadiaitionIcon.Show		(false);

	xml_init.InitStatic			(uiXml, "wound_static", 0, &UIWoundIcon);
	UIWoundIcon.Show			(false);

	xml_init.InitStatic			(uiXml, "invincible_static", 0, &UIInvincibleIcon);
	UIInvincibleIcon.Show		(false);

	xml_init.InitStatic			(uiXml, "frostbite_static", 0, &UIFrostbiteIcon);
	UIFrostbiteIcon.Show		(false);

	xml_init.InitStatic			(uiXml, "heating_static", 0, &UIHeatingIcon);
	UIHeatingIcon.Show			(false);

	hud_info_x					= uiXml.ReadAttribFlt("hud_info:position",		0, "x", 0.f);
	hud_info_y					= uiXml.ReadAttribFlt("hud_info:position",		0, "y", 0.f);

	u32 clr{};
	if (uiXml.NavigateToNode("hud_info:font", 0))
		xml_init.InitFont		(uiXml, "hud_info:font", 0, clr, m_HudInfoFont);
	else
		m_HudInfoFont			= UI().Font().pFontGraffiti19Russian;

	hud_info_item_x				= uiXml.ReadAttribFlt("hud_info:item_name",		0, "x", 0.f);
	hud_info_item_y1			= uiXml.ReadAttribFlt("hud_info:item_name",		0, "y1",0.25f);
	hud_info_item_y2			= uiXml.ReadAttribFlt("hud_info:item_name",		0, "y2",0.3f);
	hud_info_item_y3			= uiXml.ReadAttribFlt("hud_info:item_name",		0, "y3",0.32f);

	hud_info_r_e 				= uiXml.ReadAttribInt("hud_info_color:enemy",   0, "r", 0xff);
	hud_info_g_e 				= uiXml.ReadAttribInt("hud_info_color:enemy",   0, "g", 0);
	hud_info_b_e 				= uiXml.ReadAttribInt("hud_info_color:enemy",   0, "b", 0);
	hud_info_a_e 				= uiXml.ReadAttribInt("hud_info_color:enemy",   0, "a", 0x80);

	hud_info_r_n 				= uiXml.ReadAttribInt("hud_info_color:neutral", 0, "r", 0xff);
	hud_info_g_n 				= uiXml.ReadAttribInt("hud_info_color:neutral", 0, "g", 0xff);
	hud_info_b_n 				= uiXml.ReadAttribInt("hud_info_color:neutral", 0, "b", 0x80);
	hud_info_a_n 				= uiXml.ReadAttribInt("hud_info_color:neutral", 0, "a", 0x80);

	hud_info_r_f 				= uiXml.ReadAttribInt("hud_info_color:friend",  0, "r", 0);
	hud_info_g_f 				= uiXml.ReadAttribInt("hud_info_color:friend",  0, "g", 0xff);
	hud_info_b_f 				= uiXml.ReadAttribInt("hud_info_color:friend",  0, "b", 0);
	hud_info_a_f 				= uiXml.ReadAttribInt("hud_info_color:friend",  0, "a", 0x80);

	if(GameID()==GAME_ARTEFACTHUNT){
		xml_init.InitStatic		(uiXml, "artefact_static", 0, &UIArtefactIcon);
		UIArtefactIcon.Show		(false);
	}
	
	shared_str warningStrings[8] = 
	{	
		"jammed",
		"radiation",
		"wounds",
		"frostbite",
		"starvation",
		"fatigue",
		"heating",
		"invincible"
	};

	// Загружаем пороговые значения для индикаторов
	EWarningIcons j = ewiWeaponJammed;
	while (j < ewiInvincible)
	{
		// Читаем данные порогов для каждого индикатора
		shared_str cfgRecord = pSettings->r_string("main_ingame_indicators_thresholds", *warningStrings[static_cast<int>(j) - 1]);
		u32 count = _GetItemCount(*cfgRecord);

		char	singleThreshold[8];
		float	f = 0;
		for (u32 k = 0; k < count; ++k)
		{
			_GetItem(*cfgRecord, k, singleThreshold);
			sscanf(singleThreshold, "%f", &f);

			m_Thresholds[j].push_back(f);
		}

		j = static_cast<EWarningIcons>(j + 1);
	}


	// Flashing icons initialize
	uiXml.SetLocalRoot						(uiXml.NavigateToNode("flashing_icons"));
	InitFlashingIcons						(&uiXml);

	uiXml.SetLocalRoot						(uiXml.GetRoot());
	
	AttachChild								(&UICarPanel);
	xml_init.InitWindow						(uiXml, "car_panel", 0, &UICarPanel);

	AttachChild								(&UIMotionIcon);
	UIMotionIcon.Init						();

	if(GameConstants::GetArtefactPanelEnabled() && IsGameTypeSingle())
	{
		m_artefactPanel->InitFromXML		(uiXml, "artefact_panel", 0);
		this->AttachChild					(m_artefactPanel);	
	}

	AttachChild								(&UIStaticDiskIO);
	xml_init.InitStatic						(uiXml, "disk_io", 0, &UIStaticDiskIO);


	HUD_SOUND_ITEM::LoadSound				("maingame_ui", "snd_new_contact", m_contactSnd, SOUND_TYPE_IDLE);
}

float UIStaticDiskIO_start_time = 0.0f;
void CUIMainIngameWnd::Draw()
{
	// show IO icon
	bool IOActive	= (FS.dwOpenCounter>0);
	if	(IOActive)	UIStaticDiskIO_start_time = Device.fTimeGlobal;

	if ((UIStaticDiskIO_start_time+1.0f) < Device.fTimeGlobal)	UIStaticDiskIO.Show(false); 
	else {
		u32		alpha			= clampr(iFloor(255.f*(1.f-(Device.fTimeGlobal-UIStaticDiskIO_start_time)/1.f)),0,255);
		UIStaticDiskIO.Show		( true  ); 
		UIStaticDiskIO.SetColor	(color_rgba(255,255,255,alpha));
	}
	FS.dwOpenCounter = 0;

	if(!IsGameTypeSingle())
	{
		float		luminocity = smart_cast<CGameObject*>(Level().CurrentEntity())->ROS()->get_luminocity();
		float		power = log(luminocity > .001f ? luminocity : .001f)*(1.f/*luminocity_factor*/);
		luminocity	= exp(power);

		static float cur_lum = luminocity;
		cur_lum = luminocity*0.01f + cur_lum*0.99f;
		UIMotionIcon.SetLuminosity((s16)iFloor(cur_lum*100.0f));
	}
	if(!m_pActor) return;

	UIMotionIcon.SetNoise		((s16)(0xffff&iFloor(m_pActor->m_snd_noise*100.0f)));
	CUIWindow::Draw				();
	UIZoneMap->Render			();			

	RenderQuickInfos			();

	if (uiPickUpItemIconNew_)
		uiPickUpItemIconNew_->Draw();
}


void CUIMainIngameWnd::SetMPChatLog(CUIWindow* pChat, CUIWindow* pLog){
	m_pMPChatWnd = pChat;
	m_pMPLogWnd  = pLog;
}

void CUIMainIngameWnd::SetAmmoIcon (const shared_str& sect_name)
{
	if ( !sect_name.size() )
	{
		UIWeaponIcon.Show			(false);
		return;
	};

	UIWeaponIcon.Show			(true);
	//properties used by inventory menu
	float iGridWidth			= pSettings->r_float(sect_name, "inv_grid_width");
	float iGridHeight			= pSettings->r_float(sect_name, "inv_grid_height");

	float iXPos				= pSettings->r_float(sect_name, "inv_grid_x");
	float iYPos				= pSettings->r_float(sect_name, "inv_grid_y");

	UIWeaponIcon.GetUIStaticItem().SetOriginalRect(	(iXPos		 * INV_GRID_WIDTH(GameConstants::GetUseHQ_Icons())),
													(iYPos		 * INV_GRID_HEIGHT(GameConstants::GetUseHQ_Icons())),
													(iGridWidth	 * INV_GRID_WIDTH(GameConstants::GetUseHQ_Icons())),
													(iGridHeight * INV_GRID_HEIGHT(GameConstants::GetUseHQ_Icons())));
	UIWeaponIcon.SetStretchTexture(true);

	// now perform only width scale for ammo, which (W)size >2
	// all others ammo (1x1, 1x2) will be not scaled (original picture)
	float w = ((iGridWidth > 2) ? 1.6f : iGridWidth) * INV_GRID_WIDTH(GameConstants::GetUseHQ_Icons()) * 0.9f;
	float h = INV_GRID_HEIGHT(GameConstants::GetUseHQ_Icons()) * 0.9f;//1 cell

	if (GameConstants::GetUseHQ_Icons())
	{
		float w = ((iGridWidth > 2) ? 1.6f : iGridWidth) * INV_GRID_WIDTH(GameConstants::GetUseHQ_Icons()) / 2 * 0.9f;
		float h = INV_GRID_HEIGHT(GameConstants::GetUseHQ_Icons()) / 2 * 0.9f;//1 cell
	}

	float x = UIWeaponIcon_rect.x1;
	float posx_16 = 8.0f;
	float posx = 10.0f;

	if (iGridWidth == iGridHeight == 1)
	{
		posx_16 = 28.0f;
		posx = 30.0f;
	}

	UIWeaponIcon.SetWndPos(x + UI().is_widescreen() ? posx_16 : posx, UIWeaponIcon_rect.y1);
	
	if (UI().is_widescreen())
		UIWeaponIcon.SetWidth(w * UI().get_current_kx() * 1.05f);
	else
		UIWeaponIcon.SetWidth(w);
	UIWeaponIcon.SetHeight(h);
};

void CUIMainIngameWnd::Update()
{
	if (m_pMPChatWnd)
		m_pMPChatWnd->Update();
	if (m_pMPLogWnd)
		m_pMPLogWnd->Update();



	m_pActor = smart_cast<CActor*>(Level().CurrentViewEntity());
	if (!m_pActor) 
	{
		m_pItem					= NULL;
		m_pWeapon				= NULL;
		m_pGrenade				= NULL;
		CUIWindow::Update		();
		return;
	}

	if( !(Device.dwFrame%30) && IsGameTypeSingle() )
	{
			string256				text_str;
			CPda* _pda	= m_pActor->GetPDA();
			u32 _cn		= 0;
			if(_pda && 0!= (_cn=_pda->ActiveContactsNum()) )
			{
				xr_sprintf(text_str, "%d", _cn);
				UIPdaOnline.SetText(text_str);
			}
			else
			{
				UIPdaOnline.SetText("");
			}
	};

	if( !(Device.dwFrame%5) )
	{

		if(!(Device.dwFrame%30))
		{
			bool b_God = (GodMode()||(!Game().local_player)) ? true : Game().local_player->testFlag(GAME_PLAYER_FLAG_INVINCIBLE);
			if(b_God)
				SetWarningIconColor	(ewiInvincible,0xffffffff);
			else
				SetWarningIconColor	(ewiInvincible,0x00ffffff);
		}

		if (m_pActor->GetHeatingStatus() && GameConstants::GetActorFrostbite())
		{
			SetWarningIconColor(ewiHeating, 0xffffffff);
		}
		else
		{
			SetWarningIconColor(ewiHeating, 0x00ffffff);
		}

		// ewiArtefact
		if( (GameID() == GAME_ARTEFACTHUNT) && !(Device.dwFrame%30) ){
			bool b_Artefact = (NULL != m_pActor->inventory().ItemFromSlot(ARTEFACT_SLOT));
			if(b_Artefact)
				SetWarningIconColor	(ewiArtefact,0xffffffff);
			else
				SetWarningIconColor	(ewiArtefact,0x00ffffff);
		}

		// Armor indicator stuff
		PIItem	pItem = m_pActor->inventory().ItemFromSlot(OUTFIT_SLOT);
		if (pItem)
		{
			UIArmorBar.Show					(true);
			UIStaticArmor.Show				(true);
			UIArmorBar.SetProgressPos		(pItem->GetCondition()*100);
		}
		else
		{
			UIArmorBar.Show					(false);
			UIStaticArmor.Show				(false);
		}

		UpdateActiveItemInfo				();


		EWarningIcons i					= ewiWeaponJammed;

		while (i < ewiInvincible)
		{
			float value = 0;
			switch (i)
			{
				//radiation
			case ewiRadiation:
				value = m_pActor->conditions().GetRadiation();
				break;
			case ewiWound:
				value = m_pActor->conditions().BleedingSpeed();
				break;
			case ewiWeaponJammed:
				if (m_pWeapon)
					value = 1 - m_pWeapon->GetConditionToShow();
				break;
			case ewiFrostbite:
				{
					if (GameConstants::GetActorFrostbite())
						value = m_pActor->conditions().GetFrostbite();

					break;
				}
			case ewiStarvation:
				value = 1 - m_pActor->conditions().GetSatiety();
				break;		
			case ewiPsyHealth:
				value = 1 - m_pActor->conditions().GetPsyHealth();
				break;
			default:
				R_ASSERT(!"Unknown type of warning icon");
			}

			xr_vector<float>::reverse_iterator	rit;

			// Сначала проверяем на точное соответсвие
			rit  = std::find(m_Thresholds[i].rbegin(), m_Thresholds[i].rend(), value);

			// Если его нет, то берем последнее меньшее значение ()
			if (rit == m_Thresholds[i].rend())
				rit = std::find_if(m_Thresholds[i].rbegin(), m_Thresholds[i].rend(), [&value](float a) {return a < value; });;

			// Минимальное и максимальное значения границы
			float min = m_Thresholds[i].front();
			float max = m_Thresholds[i].back();

			if (rit != m_Thresholds[i].rend()){
				float v = *rit;
				SetWarningIconColor(i, color_argb(0xFF, clampr<u32>(static_cast<u32>(255 * ((v - min) / (max - min) * 2)), 0, 255), 
					clampr<u32>(static_cast<u32>(255 * (2.0f - (v - min) / (max - min) * 2)), 0, 255),
					0));
			}else
				TurnOffWarningIcon(i);

			i = (EWarningIcons)(i + 1);
		}
	}

	// health&armor
	UIHealthBar.SetProgressPos		(m_pActor->GetfHealth()*100.0f);
	UIMotionIcon.SetPower			(m_pActor->conditions().GetPower()*100.0f);

	UIZoneMap->UpdateRadar			(Device.vCameraPosition);
	float h,p;
	Device.vCameraDirection.getHP	(h,p);
	UIZoneMap->SetHeading			(-h);

	fuzzyShowInfo_ += SHOW_INFO_SPEED * Device.fTimeDelta;

	if (uiPickUpItemIconNew_ && fuzzyShowInfo_ > 0.f)
	{
		uiPickUpItemIconNew_->Update();
	}

	CUIWindow::Update				();
}

bool CUIMainIngameWnd::OnKeyboardPress(int dik)
{
	bool flag = false;
#ifdef DEBUG
		if(CAttachableItem::m_dbgItem){
			static float rot_d = deg2rad(0.5f);
			static float mov_d = 0.01f;
			bool shift = !!pInput->iGetAsyncKeyState(DIK_LSHIFT);
			flag = true;
			switch (dik)
			{
				// Shift +x
			case DIK_A:
				if(shift)	CAttachableItem::rot_dx(rot_d);
				else		CAttachableItem::mov_dx(rot_d);
				break;
				// Shift -x
			case DIK_D:
				if(shift)	CAttachableItem::rot_dx(-rot_d);
				else		CAttachableItem::mov_dx(-rot_d);
				break;
				// Shift +z
			case DIK_Q:
				if(shift)	CAttachableItem::rot_dy(rot_d);
				else		CAttachableItem::mov_dy(rot_d);
				break;
				// Shift -z
			case DIK_E:
				if(shift)	CAttachableItem::rot_dy(-rot_d);
				else		CAttachableItem::mov_dy(-rot_d);
				break;
				// Shift +y
			case DIK_S:
				if(shift)	CAttachableItem::rot_dz(rot_d);
				else		CAttachableItem::mov_dz(rot_d);
				break;
				// Shift -y
			case DIK_W:
				if(shift)	CAttachableItem::rot_dz(-rot_d);
				else		CAttachableItem::mov_dz(-rot_d);
				break;

			case DIK_SUBTRACT:
				if(shift)	rot_d-=deg2rad(0.01f);
				else		mov_d-=0.001f;
				Msg("rotation delta=[%f]; moving delta=[%f]",rot_d,mov_d);
				break;
			case DIK_ADD:
				if(shift)	rot_d+=deg2rad(0.01f);
				else		mov_d+=0.001f;
				Msg("rotation delta=[%f]; moving delta=[%f]",rot_d,mov_d);
				break;

			case DIK_P:
				Msg("LTX section [%s]",*CAttachableItem::m_dbgItem->item().object().cNameSect());
				Msg("attach_angle_offset [%f,%f,%f]",VPUSH(CAttachableItem::get_angle_offset()));
				Msg("attach_position_offset [%f,%f,%f]",VPUSH(CAttachableItem::get_pos_offset()));
				break;
			default:
				flag = false;
				break;
			}		
		if(flag)return true;;
		}
#endif		

	if(Level().IR_GetKeyState(DIK_LSHIFT) || Level().IR_GetKeyState(DIK_RSHIFT))
	{
		switch(dik)
		{
		case DIK_NUMPADMINUS:
			UIZoneMap->ZoomOut();
			return true;
			break;
		case DIK_NUMPADPLUS:
			UIZoneMap->ZoomIn();
			return true;
			break;
		}
	}
	else
	{
		switch(dik)
		{
		case DIK_NUMPADMINUS:
			//.HideAll();
			HUD().GetUI()->ShowGameIndicators(false);
			return true;
			break;
		case DIK_NUMPADPLUS:
			//.ShowAll();
			HUD().GetUI()->ShowGameIndicators(true);
			return true;
			break;
		}
	}

	return false;
}


void CUIMainIngameWnd::RenderQuickInfos()
{
	if (!m_pActor)
		return;

	static CGameObject *pObject			= NULL;
	LPCSTR actor_action					= m_pActor->GetDefaultActionForObject();
	UIStaticQuickHelp.Show				(NULL!=actor_action);

	if(NULL!=actor_action){
		if(stricmp(actor_action,UIStaticQuickHelp.GetText()))
			UIStaticQuickHelp.SetTextST				(actor_action);
	}

	if (pObject!=m_pActor->ObjectWeLookingAt())
	{
		UIStaticQuickHelp.SetTextST				(actor_action);
		UIStaticQuickHelp.ResetClrAnimation		();
		pObject	= m_pActor->ObjectWeLookingAt	();
	}
}

void CUIMainIngameWnd::ReceiveNews(GAME_NEWS_DATA* news)
{
	VERIFY(news->texture_name.size());

	HUD().GetUI()->m_pMessagesWnd->AddIconedPdaMessage(*(news->texture_name), news->tex_rect, news->SingleLineText(), news->show_time);
}

void CUIMainIngameWnd::SetWarningIconColor(CUIStatic* s, const u32 cl)
{
	int bOn = (cl>>24);
	bool bIsShown = s->IsShown();

	if(bOn)
		s->SetColor	(cl);

	if(bOn&&!bIsShown){
		m_UIIcons->AddWindow	(s, false);
		s->Show					(true);
	}

	if(!bOn&&bIsShown){
		m_UIIcons->RemoveWindow	(s);
		s->Show					(false);
	}
}

void CUIMainIngameWnd::SetWarningIconColor(EWarningIcons icon, const u32 cl)
{
	bool bMagicFlag = true;

	// Задаем цвет требуемой иконки
	switch(icon)
	{
	case ewiAll:
		bMagicFlag = false;
	case ewiWeaponJammed:
		SetWarningIconColor		(&UIWeaponJammedIcon, cl);
		if (bMagicFlag) break;
	case ewiRadiation:
		SetWarningIconColor		(&UIRadiaitionIcon, cl);
		if (bMagicFlag) break;
	case ewiWound:
		SetWarningIconColor		(&UIWoundIcon, cl);
		if (bMagicFlag) break;
	case ewiFrostbite:
		SetWarningIconColor		(&UIFrostbiteIcon, cl);
		if (bMagicFlag) break;
	case ewiHeating:
		SetWarningIconColor		(&UIHeatingIcon, cl);
		if (bMagicFlag) break;
	case ewiStarvation:
		SetWarningIconColor		(&UIStarvationIcon, cl);
		if (bMagicFlag) break;	
	case ewiPsyHealth:
		SetWarningIconColor		(&UIPsyHealthIcon, cl);
		if (bMagicFlag) break;
	case ewiInvincible:
		SetWarningIconColor		(&UIInvincibleIcon, cl);
		if (bMagicFlag) break;
		break;
	case ewiArtefact:
		SetWarningIconColor		(&UIArtefactIcon, cl);
		break;

	default:
		R_ASSERT(!"Unknown warning icon type");
	}
}

void CUIMainIngameWnd::TurnOffWarningIcon(EWarningIcons icon)
{
	SetWarningIconColor(icon, 0x00ffffff);
}


void CUIMainIngameWnd::SetFlashIconState_(EFlashingIcons type, bool enable)
{
	if (!GameConstants::GetPDA_FlashingIconsEnabled())
		return;

	// Включаем анимацию требуемой иконки
	FlashingIcons_it icon = m_FlashingIcons.find(type);
	shared_str iconType{};

	switch (type)
	{
	case 0:
		iconType = "pda";
		break;
	case 1:
		iconType = "encyclopedia";
		break;
	case 2:
		iconType = "journal";
		break;
	case 3:
		iconType = "mail";
		break;
	default:
		Msg("! [CUIMainIngameWnd::SetFlashIconState_]: Unknown flashing icon type: [%s]", iconType.c_str());
	}

	R_ASSERT3(icon != m_FlashingIcons.end(), "Flashing icon with this type not existed!", iconType.c_str());

	icon->second->Show(enable);
}

void CUIMainIngameWnd::InitFlashingIcons(CUIXml* node)
{
	const char * const flashingIconNodeName = "flashing_icon";
	int staticsCount = node->GetNodesNum("", 0, flashingIconNodeName);

	CUIXmlInit xml_init;
	CUIStatic *pIcon = NULL;
	// Пробегаемся по всем нодам и инициализируем из них статики
	for (int i = 0; i < staticsCount; ++i)
	{
		pIcon = xr_new<CUIStatic>();
		xml_init.InitStatic(*node, flashingIconNodeName, i, pIcon);
		shared_str iconType = node->ReadAttrib(flashingIconNodeName, i, "type", "none");

		// Теперь запоминаем иконку и ее тип
		EFlashingIcons type = efiPdaTask;

		if		(iconType == "pda")		type = efiPdaTask;
		else if (iconType == "mail")	type = efiMail;
		else	R_ASSERT(!"Unknown type of mainingame flashing icon");

		R_ASSERT2(m_FlashingIcons.find(type) == m_FlashingIcons.end(), "Flashing icon with this type already exists");

		CUIStatic* &val	= m_FlashingIcons[type];
		val			= pIcon;

		AttachChild(pIcon);
		pIcon->Show(false);
	}
}

void CUIMainIngameWnd::DestroyFlashingIcons()
{
	for (FlashingIcons_it it = m_FlashingIcons.begin(); it != m_FlashingIcons.end(); ++it)
	{
		DetachChild(it->second);
		xr_delete(it->second);
	}

	m_FlashingIcons.clear();
}

void CUIMainIngameWnd::UpdateFlashingIcons()
{
	for (FlashingIcons_it it = m_FlashingIcons.begin(); it != m_FlashingIcons.end(); ++it)
	{
		it->second->Update();
	}
}

void CUIMainIngameWnd::AnimateContacts(bool b_snd)
{
	UIPdaOnline.ResetClrAnimation	();

	if(b_snd)
		HUD_SOUND_ITEM::PlaySound	(m_contactSnd, Fvector().set(0, 0, 0), 0, true);

}

void CUIMainIngameWnd::SetPickUpItem	(CInventoryItem* PickUpItem)
{
	if (m_pPickUpItem != PickUpItem)
		xr_delete(uiPickUpItemIconNew_);

	if (m_pPickUpItem == PickUpItem)
		return;

	m_pPickUpItem = PickUpItem;

	if (!m_pPickUpItem)
		return;

	uiPickUpItemIconNew_ = create_cell_item(m_pPickUpItem); // use inventory cell item class

	float x_size = m_pPickUpItem->GetInvGridRect().x2 * (UI().is_widescreen() ? 30.f : 40.f);
	float y_size = m_pPickUpItem->GetInvGridRect().y2 * 40.f;
	uiPickUpItemIconNew_->SetAlignment(waCenter);
	uiPickUpItemIconNew_->SetWndPos(Fvector2().set(m_iPickUpItemIconX, m_iPickUpItemIconY));
	uiPickUpItemIconNew_->SetWndSize(Fvector2().set(x_size * m_iPickUpItemIconScale, y_size * m_iPickUpItemIconScale));
	fuzzyShowInfo_ = 0.f;
}

void CUIMainIngameWnd::UpdateActiveItemInfo()
{
	PIItem item		=  m_pActor->inventory().ActiveItem();
	if(item) 
	{
		xr_string					str_name;
		xr_string					icon_sect_name;
		xr_string					str_count;
		item->GetBriefInfo			(str_name, icon_sect_name, str_count);

		UIWeaponSignAmmo.Show		(true						);
		UIWeaponBack.SetText		(str_name.c_str			()	);
		UIWeaponSignAmmo.SetText	(str_count.c_str		()	);
		SetAmmoIcon					(icon_sect_name.c_str	()	);

		//-------------------
		m_pWeapon = smart_cast<CWeapon*> (item);		
	}else
	{
		UIWeaponIcon.Show			(false);
		UIWeaponSignAmmo.Show		(false);
		UIWeaponBack.SetText		("");
		m_pWeapon					= NULL;
	}
}

void CUIMainIngameWnd::OnConnected()
{
	UIZoneMap->SetupCurrentMap		();
}

void CUIMainIngameWnd::reset_ui()
{
	m_pActor						= NULL;
	m_pWeapon						= NULL;
	m_pGrenade						= NULL;
	m_pItem							= NULL;
	m_pPickUpItem					= NULL;
	UIMotionIcon.ResetVisibility	();
}

void CUIMainIngameWnd::DrawMainIndicatorsForInventory()
{
	CActor* pActor = smart_cast<CActor*>(Level().CurrentViewEntity());

	if (!pActor)
		return;

	UpdateBoosterIndicators(pActor->conditions().GetCurBoosterInfluences());

	if (m_ind_boost_psy->IsShown())
	{
		m_ind_boost_psy->Update();
		m_ind_boost_psy->Draw();
	}

	if (m_ind_boost_radia->IsShown())
	{
		m_ind_boost_radia->Update();
		m_ind_boost_radia->Draw();
	}

	if (m_ind_boost_chem->IsShown())
	{
		m_ind_boost_chem->Update();
		m_ind_boost_chem->Draw();
	}

	if (m_ind_boost_wound->IsShown())
	{
		m_ind_boost_wound->Update();
		m_ind_boost_wound->Draw();
	}

	if (m_ind_boost_weight->IsShown())
	{
		m_ind_boost_weight->Update();
		m_ind_boost_weight->Draw();
	}

	if (m_ind_boost_health->IsShown())
	{
		m_ind_boost_health->Update();
		m_ind_boost_health->Draw();
	}

	if (m_ind_boost_power->IsShown())
	{
		m_ind_boost_power->Update();
		m_ind_boost_power->Draw();
	}

	if (m_ind_boost_rad->IsShown())
	{
		m_ind_boost_rad->Update();
		m_ind_boost_rad->Draw();
	}

	if (m_ind_boost_satiety->IsShown())
	{
		m_ind_boost_satiety->Update();
		m_ind_boost_satiety->Draw();
	}

	if (m_ind_boost_thirst->IsShown())
	{
		m_ind_boost_thirst->Update();
		m_ind_boost_thirst->Draw();
	}

	if (m_ind_boost_psy_health->IsShown())
	{
		m_ind_boost_psy_health->Update();
		m_ind_boost_psy_health->Draw();
	}

	if (m_ind_boost_intoxication->IsShown())
	{
		m_ind_boost_intoxication->Update();
		m_ind_boost_intoxication->Draw();
	}

	if (m_ind_boost_sleepeness->IsShown())
	{
		m_ind_boost_sleepeness->Update();
		m_ind_boost_sleepeness->Draw();
	}

	if (m_ind_boost_alcoholism->IsShown())
	{
		m_ind_boost_alcoholism->Update();
		m_ind_boost_alcoholism->Draw();
	}

	if (m_ind_boost_hangover->IsShown())
	{
		m_ind_boost_hangover->Update();
		m_ind_boost_hangover->Draw();
	}

	if (m_ind_boost_narcotism->IsShown())
	{
		m_ind_boost_narcotism->Update();
		m_ind_boost_narcotism->Draw();
	}

	if (m_ind_boost_withdrawal->IsShown())
	{
		m_ind_boost_withdrawal->Update();
		m_ind_boost_withdrawal->Draw();
	}
}

void CUIMainIngameWnd::UpdateBoosterIndicators(const xr_map<EBoostParams, SBooster> influences)
{
	m_ind_boost_psy->Show(false);
	m_ind_boost_radia->Show(false);
	m_ind_boost_chem->Show(false);
	m_ind_boost_wound->Show(false);
	m_ind_boost_weight->Show(false);
	m_ind_boost_health->Show(false);
	m_ind_boost_power->Show(false);
	m_ind_boost_rad->Show(false);
	m_ind_boost_satiety->Show(false);
	m_ind_boost_thirst->Show(false);
	m_ind_boost_psy_health->Show(false);
	m_ind_boost_intoxication->Show(false);
	m_ind_boost_sleepeness->Show(false);
	m_ind_boost_alcoholism->Show(false);
	m_ind_boost_hangover->Show(false);
	m_ind_boost_narcotism->Show(false);
	m_ind_boost_withdrawal->Show(false);

	LPCSTR str_flag	= "ui_slow_blinking_alpha";

	xr_map<EBoostParams, SBooster>::const_iterator b = influences.begin(), e = influences.end();
	for(; b!=e; b++)
	{
		switch(b->second.m_type)
		{
			case eBoostHpRestore: 
				{
					m_ind_boost_health->Show(true);
					if (b->second.fBoostTime <= 3.0f)
						m_ind_boost_health->SetClrLightAnim(str_flag, true, true, false, true);
					else
						m_ind_boost_health->ResetClrAnimation();
				}
				break;
			case eBoostPowerRestore: 
				{
					m_ind_boost_power->Show(true); 
					if(b->second.fBoostTime<=3.0f)
						m_ind_boost_power->SetClrLightAnim(str_flag, true, true, false, true);
					else
						m_ind_boost_power->ResetClrAnimation();
				}
				break;
			case eBoostRadiationRestore: 
				{
					m_ind_boost_rad->Show(true); 
					if(b->second.fBoostTime<=3.0f)
						m_ind_boost_rad->SetClrLightAnim(str_flag, true, true, false, true);
					else
						m_ind_boost_rad->ResetClrAnimation();
				}
				break;
			case eBoostBleedingRestore: 
				{
					m_ind_boost_wound->Show(true); 
					if(b->second.fBoostTime<=3.0f)
						m_ind_boost_wound->SetClrLightAnim(str_flag, true, true, false, true);
					else
						m_ind_boost_wound->ResetClrAnimation();
				}
				break;
			case eBoostMaxWeight: 
				{
					m_ind_boost_weight->Show(true); 
					if(b->second.fBoostTime<=3.0f)
						m_ind_boost_weight->SetClrLightAnim(str_flag, true, true, false, true);
					else
						m_ind_boost_weight->ResetClrAnimation();
				}
				break;
			case eBoostRadiationImmunity: 
			case eBoostRadiationProtection: 
				{
					m_ind_boost_radia->Show(true); 
					if(b->second.fBoostTime<=3.0f)
						m_ind_boost_radia->SetClrLightAnim(str_flag, true, true, false, true);
					else
						m_ind_boost_radia->ResetClrAnimation();
				}
				break;
			case eBoostTelepaticImmunity: 
			case eBoostTelepaticProtection: 
				{
					m_ind_boost_psy->Show(true); 
					if(b->second.fBoostTime<=3.0f)
						m_ind_boost_psy->SetClrLightAnim(str_flag, true, true, false, true);
					else
						m_ind_boost_psy->ResetClrAnimation();
				}
				break;
			case eBoostChemicalBurnImmunity: 
			case eBoostChemicalBurnProtection: 
				{
					m_ind_boost_chem->Show(true); 
					if(b->second.fBoostTime<=3.0f)
						m_ind_boost_chem->SetClrLightAnim(str_flag, true, true, false, true);
					else
						m_ind_boost_chem->ResetClrAnimation();
				}
				break;
			case eBoostSatietyRestore:
				{
					m_ind_boost_satiety->Show(true);
					if (b->second.fBoostTime <= 3.0f)
						m_ind_boost_satiety->SetClrLightAnim(str_flag, true, true, false, true);
					else
						m_ind_boost_satiety->ResetClrAnimation();
				}
				break;
			case eBoostThirstRestore:
				{
					m_ind_boost_thirst->Show(true);
					if (b->second.fBoostTime <= 3.0f)
						m_ind_boost_thirst->SetClrLightAnim(str_flag, true, true, false, true);
					else
						m_ind_boost_thirst->ResetClrAnimation();
				}
				break;
			case eBoostPsyHealthRestore:
				{
					m_ind_boost_psy_health->Show(true);
					if (b->second.fBoostTime <= 3.0f)
						m_ind_boost_psy_health->SetClrLightAnim(str_flag, true, true, false, true);
					else
						m_ind_boost_psy_health->ResetClrAnimation();
				}
				break;
			case eBoostIntoxicationRestore:
				{
					m_ind_boost_intoxication->Show(true);
					if (b->second.fBoostTime <= 3.0f)
						m_ind_boost_intoxication->SetClrLightAnim(str_flag, true, true, false, true);
					else
						m_ind_boost_intoxication->ResetClrAnimation();
				}
				break;
			case eBoostSleepenessRestore:
				{
					m_ind_boost_sleepeness->Show(true);
					if (b->second.fBoostTime <= 3.0f)
						m_ind_boost_sleepeness->SetClrLightAnim(str_flag, true, true, false, true);
					else
						m_ind_boost_sleepeness->ResetClrAnimation();
				}
				break;
			case eBoostAlcoholismRestore:
				{
					m_ind_boost_alcoholism->Show(true);
					if (b->second.fBoostTime <= 3.0f)
						m_ind_boost_alcoholism->SetClrLightAnim(str_flag, true, true, false, true);
					else
						m_ind_boost_alcoholism->ResetClrAnimation();
				}
				break;
			case eBoostHangoverRestore:
				{
					m_ind_boost_hangover->Show(true);
					if (b->second.fBoostTime <= 3.0f)
						m_ind_boost_hangover->SetClrLightAnim(str_flag, true, true, false, true);
					else
						m_ind_boost_hangover->ResetClrAnimation();
				}
				break;
			case eBoostNarcotismRestore:
				{
					m_ind_boost_narcotism->Show(true);
					if (b->second.fBoostTime <= 3.0f)
						m_ind_boost_narcotism->SetClrLightAnim(str_flag, true, true, false, true);
					else
						m_ind_boost_narcotism->ResetClrAnimation();
				}
				break;
			case eBoostWithdrawalRestore:
				{
					m_ind_boost_withdrawal->Show(true);
					if (b->second.fBoostTime <= 3.0f)
						m_ind_boost_withdrawal->SetClrLightAnim(str_flag, true, true, false, true);
					else
						m_ind_boost_withdrawal->ResetClrAnimation();
				}
				break;
		}
	}
}