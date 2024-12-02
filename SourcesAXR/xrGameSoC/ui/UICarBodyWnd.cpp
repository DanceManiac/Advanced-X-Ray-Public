#include "pch_script.h"
#include "UICarBodyWnd.h"
#include "xrUIXmlParser.h"
#include "UIXmlInit.h"
#include "../HUDManager.h"
#include "../level.h"
#include "UICharacterInfo.h"
#include "UIDragDropListEx.h"
#include "UIFrameWindow.h"
#include "UIStatic.h"
#include "UIGameCustom.h"
#include "UIItemInfo.h"
#include "UIPropertiesBox.h"
#include "../ai/monsters/BaseMonster/base_monster.h"
#include "../inventory.h"
#include "UIInventoryUtilities.h"
#include "UICellItem.h"
#include "UICellItemFactory.h"
#include "../WeaponMagazined.h"
#include "../Actor.h"
#include "../eatable_item.h"
#include "../alife_registry_wrappers.h"
#include "UI3tButton.h"
#include "UIListBoxItem.h"
#include "UIHelper.h"
#include "../InventoryBox.h"
#include "../game_object_space.h"
#include "../script_callback_ex.h"
#include "../script_game_object.h"
#include "../BottleItem.h"
#include "string_table.h"

#include "AdvancedXrayGameConstants.h"
#include "Car.h"
#include "../../xrEngine/x_ray.h"


#define				CAR_BODY_XML		"carbody_new.xml"
#define				CARBODY_ITEM_XML	"carbody_item.xml"

extern bool SSFX_UI_DoF_active;

void move_item (u16 from_id, u16 to_id, u16 what_id);

CUICarBodyWnd::CUICarBodyWnd()
{
	m_pInventoryBox		= NULL;
	m_pCar				= NULL;
	Init				();
	Hide				();
	m_b_need_update		= false;
}

CUICarBodyWnd::~CUICarBodyWnd()
{
	m_pUIOurBagList->ClearAll					(true);
	m_pUIOthersBagList->ClearAll				(true);
}

void CUICarBodyWnd::Init()
{
	CUIXml						uiXml;
	uiXml.Load					(CONFIG_PATH, UI_PATH, CAR_BODY_XML);
	
	CUIXmlInit					xml_init;

	xml_init.InitWindow			(uiXml, "main", 0, this);

	m_pUIStaticTop				= xr_new<CUIStatic>(); m_pUIStaticTop->SetAutoDelete(true);
	AttachChild					(m_pUIStaticTop);
	xml_init.InitStatic			(uiXml, "top_background", 0, m_pUIStaticTop);


	m_pUIStaticBottom			= xr_new<CUIStatic>(); m_pUIStaticBottom->SetAutoDelete(true);
	AttachChild					(m_pUIStaticBottom);
	xml_init.InitStatic			(uiXml, "bottom_background", 0, m_pUIStaticBottom);

	m_pUIOurIcon				= xr_new<CUIStatic>(); m_pUIOurIcon->SetAutoDelete(true);
	AttachChild					(m_pUIOurIcon);
	xml_init.InitStatic			(uiXml, "static_icon", 0, m_pUIOurIcon);

	m_pUIOthersIcon				= xr_new<CUIStatic>(); m_pUIOthersIcon->SetAutoDelete(true);
	AttachChild					(m_pUIOthersIcon);
	xml_init.InitStatic			(uiXml, "static_icon", 1, m_pUIOthersIcon);


	m_pUICharacterInfoLeft		= xr_new<CUICharacterInfo>(); m_pUICharacterInfoLeft->SetAutoDelete(true);
	m_pUIOurIcon->AttachChild	(m_pUICharacterInfoLeft);
	m_pUICharacterInfoLeft->Init(0,0, m_pUIOurIcon->GetWidth(), m_pUIOurIcon->GetHeight(), "trade_character.xml");


	m_pUICharacterInfoRight			= xr_new<CUICharacterInfo>(); m_pUICharacterInfoRight->SetAutoDelete(true);
	m_pUIOthersIcon->AttachChild	(m_pUICharacterInfoRight);
	m_pUICharacterInfoRight->Init	(0,0, m_pUIOthersIcon->GetWidth(), m_pUIOthersIcon->GetHeight(), "trade_character.xml");

	m_pUIOurBagWnd					= xr_new<CUIStatic>(); m_pUIOurBagWnd->SetAutoDelete(true);
	AttachChild						(m_pUIOurBagWnd);
	xml_init.InitStatic				(uiXml, "our_bag_static", 0, m_pUIOurBagWnd);


	m_pUIOthersBagWnd				= xr_new<CUIStatic>(); m_pUIOthersBagWnd->SetAutoDelete(true);
	AttachChild						(m_pUIOthersBagWnd);
	xml_init.InitStatic				(uiXml, "others_bag_static", 0, m_pUIOthersBagWnd);

	if (GameConstants::GetLimitedInvBoxes())
	{
		m_PartnerInvCapacityInfo	= UIHelper::CreateStatic(uiXml, "partner_capacity_caption", this);
		m_PartnerInvFullness		= UIHelper::CreateStatic(uiXml, "partner_inv_fullness", this);
		m_PartnerInvCapacity		= UIHelper::CreateStatic(uiXml, "partner_inv_capacity", this);
		m_PartnerInvCapacityInfo->AdjustWidthToText();
	}

	if (GameConstants::GetLimitedInventory())
	{
		m_ActorInvCapacityInfo			= UIHelper::CreateStatic(uiXml, "actor_inv_capacity_caption", this);
		m_ActorInvFullness				= UIHelper::CreateStatic(uiXml, "actor_inv_fullness", this);
		m_ActorInvCapacity				= UIHelper::CreateStatic(uiXml, "actor_inv_capacity", this);
		m_ActorInvCapacityInfo->AdjustWidthToText();
	}

	m_pUIOurBagList					= xr_new<CUIDragDropListEx>(); m_pUIOurBagList->SetAutoDelete(true);
	m_pUIOurBagWnd->AttachChild		(m_pUIOurBagList);	
	xml_init.InitDragDropListEx		(uiXml, "dragdrop_list_our", 0, m_pUIOurBagList);

	m_pUIOthersBagList				= xr_new<CUIDragDropListEx>(); m_pUIOthersBagList->SetAutoDelete(true);
	m_pUIOthersBagWnd->AttachChild	(m_pUIOthersBagList);	
	xml_init.InitDragDropListEx		(uiXml, "dragdrop_list_other", 0, m_pUIOthersBagList);


	//информация о предмете
	m_pUIDescWnd					= xr_new<CUIFrameWindow>(); m_pUIDescWnd->SetAutoDelete(true);
	AttachChild						(m_pUIDescWnd);
	xml_init.InitFrameWindow		(uiXml, "frame_window", 0, m_pUIDescWnd);

	m_pUIStaticDesc					= xr_new<CUIStatic>(); m_pUIStaticDesc->SetAutoDelete(true);
	m_pUIDescWnd->AttachChild		(m_pUIStaticDesc);
	xml_init.InitStatic				(uiXml, "descr_static", 0, m_pUIStaticDesc);

	m_pUIItemInfo					= xr_new<CUIItemInfo>(); m_pUIItemInfo->SetAutoDelete(true);
	m_pUIDescWnd->AttachChild		(m_pUIItemInfo);
	m_pUIItemInfo->Init				(0,0, m_pUIDescWnd->GetWidth(), m_pUIDescWnd->GetHeight(), CARBODY_ITEM_XML);


	xml_init.InitAutoStatic			(uiXml, "auto_static", this);

	m_pUIPropertiesBox				= xr_new<CUIPropertiesBox>(); m_pUIPropertiesBox->SetAutoDelete(true);
	AttachChild						(m_pUIPropertiesBox);
	m_pUIPropertiesBox->Init		(0,0,300,300);
	m_pUIPropertiesBox->Hide		();

	SetCurrentItem					(NULL);
	m_pUIStaticDesc->SetText		(NULL);

	m_pUITakeAll					= xr_new<CUI3tButton>(); m_pUITakeAll->SetAutoDelete(true);
	AttachChild						(m_pUITakeAll);
	xml_init.Init3tButton			(uiXml, "take_all_btn", 0, m_pUITakeAll);

	BindDragDropListEnents			(m_pUIOurBagList);
	BindDragDropListEnents			(m_pUIOthersBagList);

	InitCallbacks					();
}

void CUICarBodyWnd::InitCallbacks()
{
	Register(m_pUIPropertiesBox);
	Register(m_pUITakeAll);

	AddCallback(m_pUIPropertiesBox->WindowName(), PROPERTY_CLICKED, CUIWndCallback::void_function(this, &CUICarBodyWnd::ProcessPropertiesBoxClicked));
	AddCallback(m_pUITakeAll->WindowName(), BUTTON_CLICKED, CUIWndCallback::void_function(this, &CUICarBodyWnd::OnBtnTakeAll));
}

void CUICarBodyWnd::InitCarBody(CInventoryOwner* pOur, CInventoryBox* pInvBox)
{
    m_pOurObject									= pOur;
	m_pOthersObject									= NULL;
	m_pInventoryBox									= pInvBox;
	m_pCar											= NULL;
	m_pInventoryBox->m_in_use						= true;

	m_pUICharacterInfoLeft->InitCharacter			(m_pOurObject);
	m_pUIOthersIcon->Show							(false);
	m_pUICharacterInfoRight->ClearInfo				();
	m_pUIPropertiesBox->Hide						();
	EnableAll										();
	UpdateLists										();

}

void CUICarBodyWnd::InitCarBody(CInventoryOwner* pOur, CInventoryOwner* pOthers)
{

    m_pOurObject									= pOur;
	m_pOthersObject									= pOthers;
	m_pInventoryBox									= NULL;
	m_pCar											= NULL;
	
	u16 our_id										= smart_cast<CGameObject*>(m_pOurObject)->ID();
	u16 other_id									= smart_cast<CGameObject*>(m_pOthersObject)->ID();

	m_pUICharacterInfoLeft->InitCharacter			(m_pOurObject);
	m_pUIOthersIcon->Show							(true);
	
	CBaseMonster *monster = NULL;
	if(m_pOthersObject) {
		monster										= smart_cast<CBaseMonster *>(m_pOthersObject);
		if (monster || m_pOthersObject->use_simplified_visual() ) 
		{
			m_pUICharacterInfoRight->ClearInfo		();
			if(monster)
			{
				shared_str monster_tex_name = pSettings->r_string(monster->cNameSect(),"icon");
				m_pUICharacterInfoRight->UIIcon().InitTexture(monster_tex_name.c_str());
				m_pUICharacterInfoRight->UIIcon().SetStretchTexture(true);
			}
		}else 
		{
			m_pUICharacterInfoRight->InitCharacter	(m_pOthersObject);
		}
	}

	m_pUIPropertiesBox->Hide						();
	EnableAll										();
	UpdateLists										();

	if(!monster){
		CInfoPortionWrapper	*known_info_registry	= xr_new<CInfoPortionWrapper>();
		known_info_registry->registry().init		(other_id);
		KNOWN_INFO_VECTOR& known_info				= known_info_registry->registry().objects();

		KNOWN_INFO_VECTOR_IT it = known_info.begin();
		for(int i=0;it!=known_info.end();++it,++i){
			(*it).info_id;	
			NET_Packet		P;
			CGameObject::u_EventGen		(P,GE_INFO_TRANSFER, our_id);
			P.w_u16						(0);//not used
			P.w_stringZ					((*it).info_id);			//сообщение
			P.w_u8						(1);						//добавление сообщения
			CGameObject::u_EventSend	(P);
		}
		known_info.clear	();
		xr_delete			(known_info_registry);
	}
}

void CUICarBodyWnd::InitCarBody(CInventoryOwner* pActorInv, CCar* pCar)
{
	m_pOurObject		= pActorInv;
	m_pOthersObject		= NULL;
	m_pInventoryBox		= NULL;
	m_pCar				= pCar;

	u16 our_id			= smart_cast<CGameObject*>(m_pOurObject)->ID();
	u16 other_id		= smart_cast<CGameObject*>(pCar)->ID();

	if (m_pCar || m_pCar->use_simplified_visual())
	{
		m_pUICharacterInfoRight->ClearInfo();
		if (m_pCar)
		{
			shared_str car_tex_name = pSettings->r_string(m_pCar->cNameSect(), "icon");
			m_pUICharacterInfoRight->UIIcon().InitTexture(car_tex_name.c_str());
			m_pUICharacterInfoRight->UIIcon().SetStretchTexture(true);
		}
	}
	else
		return;

	m_pUIPropertiesBox->Hide();
	EnableAll();
	UpdateLists();
}

void CUICarBodyWnd::UpdateLists_delayed()
{
		m_b_need_update = true;
}

#include "UIInventoryUtilities.h"

void CUICarBodyWnd::Hide()
{
	InventoryUtilities::SendInfoToActor			("ui_car_body_hide");
	m_pUIOurBagList->ClearAll					(true);
	m_pUIOthersBagList->ClearAll				(true);
	inherited::Hide								();
	if(m_pInventoryBox)
		m_pInventoryBox->m_in_use				= false;

	if (smart_cast<CActor*>(Level().CurrentEntity()) && GameConstants::GetHideWeaponInInventory())
	{
		Actor()->SetWeaponHideState(INV_STATE_BLOCK_ALL, false);
	}

	if (SSFX_UI_DoF_active)
	{
		ps_ssfx_wpn_dof_1 = GameConstants::GetSSFX_DefaultDoF();
		ps_ssfx_wpn_dof_2 = GameConstants::GetSSFX_DefaultDoF().z;
		SSFX_UI_DoF_active = false;
	}
}

void CUICarBodyWnd::UpdateLists()
{
	TIItemContainer								ruck_list;
	m_pUIOurBagList->ClearAll					(true);
	m_pUIOthersBagList->ClearAll				(true);

	ruck_list.clear								();
	m_pOurObject->inventory().AddAvailableItems	(ruck_list, true);
	std::sort									(ruck_list.begin(),ruck_list.end(),InventoryUtilities::GreaterRoomInRuck);

	//Наш рюкзак
	TIItemContainer::iterator it;
	for(it =  ruck_list.begin(); ruck_list.end() != it; ++it) 
	{
		CUICellItem* itm				= create_cell_item(*it);
		m_pUIOurBagList->SetItem		(itm);
		ColorizeItem					(itm);
	}


	ruck_list.clear									();
	if(m_pOthersObject)
		m_pOthersObject->inventory().AddAvailableItems	(ruck_list, false);
	else if (m_pCar)
		m_pCar->AddAvailableItems					(ruck_list);
	else
		m_pInventoryBox->AddAvailableItems			(ruck_list);

	std::sort										(ruck_list.begin(),ruck_list.end(),InventoryUtilities::GreaterRoomInRuck);

	//Чужой рюкзак
	for(it =  ruck_list.begin(); ruck_list.end() != it; ++it) 
	{
		CUICellItem* itm							= create_cell_item(*it);
		m_pUIOthersBagList->SetItem					(itm);
	}

	InventoryUtilities::UpdateWeight				(*m_pUIOurBagWnd);

	if (GameConstants::GetLimitedInventory())
		InventoryUtilities::UpdateCapacityStr(*m_ActorInvFullness, *m_ActorInvCapacity);

	UpdateDeadBodyBag();

	m_b_need_update									= false;
}

void CUICarBodyWnd::UpdateDeadBodyBag()
{
	string64 buf;

	if (!m_pInventoryBox || !GameConstants::GetLimitedInvBoxes())
		return;

	float total = m_pInventoryBox->GetInventoryFullness();
	float max = GameConstants::GetInvBoxCapacity();
	LPCSTR lit_str = CStringTable().translate("st_liters").c_str();
	xr_sprintf(buf, "%.1f", total);

	m_PartnerInvFullness->SetText(buf);
	m_PartnerInvFullness->AdjustWidthToText();
	xr_sprintf(buf, "%s %.1f %s", "/", max, lit_str);
	m_PartnerInvCapacity->SetText(buf);
	m_PartnerInvCapacity->AdjustWidthToText();

	Fvector2 pos = m_PartnerInvFullness->GetWndPos();
	pos.x = m_PartnerInvCapacity->GetWndPos().x - m_PartnerInvFullness->GetWndSize().x - 5.0f;
	m_PartnerInvFullness->SetWndPos(pos);
	pos.x = pos.x - m_PartnerInvCapacityInfo->GetWndSize().x - 5.0f;
	m_PartnerInvCapacityInfo->SetWndPos(pos);
}

void CUICarBodyWnd::SendMessage(CUIWindow *pWnd, s16 msg, void *pData)
{
	CUIWndCallback::OnEvent(pWnd, msg, pData);
}

void CUICarBodyWnd::Draw()
{
	inherited::Draw	();
}

void CUICarBodyWnd::OnBtnTakeAll(CUIWindow* w, void* d)
{
	TakeAll();
}

void CUICarBodyWnd::Update()
{
	if(	m_b_need_update||
		m_pOurObject->inventory().ModifyFrame()==Device.dwFrame || 
		(m_pOthersObject&&m_pOthersObject->inventory().ModifyFrame()==Device.dwFrame))

		UpdateLists		();

	
	if(m_pOthersObject && (smart_cast<CGameObject*>(m_pOurObject))->Position().distance_to((smart_cast<CGameObject*>(m_pOthersObject))->Position()) > 3.0f)
	{
		GetHolder()->StartStopMenu(this,true);
	}
	inherited::Update();
}


void CUICarBodyWnd::Show() 
{ 
	InventoryUtilities::SendInfoToActor		("ui_car_body");
	inherited::Show							();
	SetCurrentItem							(NULL);
	InventoryUtilities::UpdateWeight		(*m_pUIOurBagWnd);

	if (GameConstants::GetLimitedInventory())
		InventoryUtilities::UpdateCapacityStr(*m_ActorInvFullness, *m_ActorInvCapacity);

	if (smart_cast<CActor*>(Level().CurrentEntity()) && GameConstants::GetHideWeaponInInventory())
	{
		Actor()->SetWeaponHideState(INV_STATE_BLOCK_ALL, true);
	}

	if (!SSFX_UI_DoF_active)
	{
		ps_ssfx_wpn_dof_1 = GameConstants::GetSSFX_FocusDoF();
		ps_ssfx_wpn_dof_2 = GameConstants::GetSSFX_FocusDoF().z;
		SSFX_UI_DoF_active = true;
	}
}

void CUICarBodyWnd::DisableAll()
{
	m_pUIOurBagWnd->Enable			(false);
	m_pUIOthersBagWnd->Enable		(false);
}

void CUICarBodyWnd::EnableAll()
{
	m_pUIOurBagWnd->Enable			(true);
	m_pUIOthersBagWnd->Enable		(true);
}

CUICellItem* CUICarBodyWnd::CurrentItem()
{
	return m_pCurrentCellItem;
}

PIItem CUICarBodyWnd::CurrentIItem()
{
	return	(m_pCurrentCellItem)?(PIItem)m_pCurrentCellItem->m_pData : NULL;
}

void CUICarBodyWnd::SetCurrentItem(CUICellItem* itm)
{
	if(m_pCurrentCellItem == itm) return;
	m_pCurrentCellItem		= itm;
	m_pUIItemInfo->InitItem(CurrentIItem());
}

void CUICarBodyWnd::TakeAll()
{
	u32 cnt				= m_pUIOthersBagList->ItemsCount();
	u16 tmp_id = 0;
	if(m_pInventoryBox){
		tmp_id	= (smart_cast<CGameObject*>(m_pOurObject))->ID();
	}

	for(u32 i=0; i<cnt; ++i)
	{
		CUICellItem*	ci = m_pUIOthersBagList->GetItemIdx(i);
		for(u32 j=0; j<ci->ChildsCount(); ++j)
		{
			PIItem _itm		= (PIItem)(ci->Child(j)->m_pData);
			if(m_pOthersObject)
				TransferItem	(_itm, m_pOthersObject, m_pOurObject, false);
			else{
				move_item		(m_pInventoryBox->ID(), tmp_id, _itm->object().ID());
//.				Actor()->callback(GameObject::eInvBoxItemTake)( m_pInventoryBox->lua_game_object(), _itm->object().lua_game_object() );
			}
		
		}
		PIItem itm		= (PIItem)(ci->m_pData);
		if(m_pOthersObject)
			TransferItem	(itm, m_pOthersObject, m_pOurObject, false);
		else
		{
			if (m_pInventoryBox)
				move_item		(m_pInventoryBox->ID(), tmp_id, itm->object().ID());
			else
				move_item		(m_pCar->ID(), tmp_id, itm->object().ID());
//.			Actor()->callback(GameObject::eInvBoxItemTake)(m_pInventoryBox->lua_game_object(), itm->object().lua_game_object() );
		}

	}
}

void CUICarBodyWnd::DetachAddon(LPCSTR addon_name)
{
	if (OnClient())
	{
		NET_Packet								P;
		CurrentIItem()->object().u_EventGen(P, GE_ADDON_DETACH, CurrentIItem()->object().ID());
		P.w_stringZ(addon_name);
		CurrentIItem()->object().u_EventSend(P);
	};
	CurrentIItem()->Detach(addon_name, true);
}

#include "../xr_level_controller.h"

bool CUICarBodyWnd::OnKeyboardAction(int dik, EUIMessages keyboard_action)
{
	if (inherited::OnKeyboardAction(dik, keyboard_action))
		return true;

	if (keyboard_action == WINDOW_KEY_PRESSED)
	{
		if (is_binded(kUSE, dik))
		{
			GetHolder()->StartStopMenu(this, true);
			return true;
		}
		else if (is_binded(kSPRINT_TOGGLE, dik))
		{
			TakeAll();
			return true;
		}
	}
	return false;
}

void CUICarBodyWnd::EatItem(CUICellItem* itm)
{
	CActor *pActor				= smart_cast<CActor*>(Level().CurrentEntity());
	if(!pActor)					return;
	PIItem item					= (PIItem)itm->m_pData;
	CUIDragDropListEx* owner_list		= itm->OwnerList();
	if(owner_list==m_pUIOthersBagList)
	{
		u16 owner_id{};

		if (m_pCar)
			owner_id = m_pCar->ID();
		else
			owner_id				= (m_pInventoryBox)?m_pInventoryBox->ID():smart_cast<CGameObject*>(m_pOthersObject)->ID();

		move_item(owner_id, //from
			Actor()->ID(), //to
			item->object().ID());
	}

	NET_Packet					P;
	CGameObject::u_EventGen		(P, GEG_PLAYER_ITEM_EAT, Actor()->ID());
	P.w_u16						(item->object().ID());
	CGameObject::u_EventSend	(P);

}


bool CUICarBodyWnd::OnItemDrop(CUICellItem* itm)
{
	CUIDragDropListEx*	old_owner		= itm->OwnerList();
	CUIDragDropListEx*	new_owner		= CUIDragDropListEx::m_drag_item->BackList();
	
	if(old_owner==new_owner || !old_owner || !new_owner || (false&&new_owner==m_pUIOthersBagList&&m_pInventoryBox))
					return true;

	if(m_pOthersObject)
	{
		if( TransferItem		(	CurrentIItem(),
								(old_owner==m_pUIOthersBagList)?m_pOthersObject:m_pOurObject, 
								(old_owner==m_pUIOurBagList)?m_pOthersObject:m_pOurObject, 
								(old_owner==m_pUIOurBagList)
							)
			)
		{
			CUICellItem* ci					= old_owner->RemoveItem(CurrentItem(), false);
			new_owner->SetItem				(ci);
		}
	}
	else
	{
		u16 tmp_id	= (smart_cast<CGameObject*>(m_pOurObject))->ID();

		bool bMoveDirection		= (old_owner==m_pUIOthersBagList);

		if (m_pInventoryBox)
		{
			move_item(
				bMoveDirection ? m_pInventoryBox->ID() : tmp_id,
				bMoveDirection ? tmp_id : m_pInventoryBox->ID(),
				CurrentIItem()->object().ID());
		}
		else
		{
			move_item(
				bMoveDirection ? m_pCar->ID() : tmp_id,
				bMoveDirection ? tmp_id : m_pCar->ID(),
				CurrentIItem()->object().ID());
		}


//		Actor()->callback		(GameObject::eInvBoxItemTake)(m_pInventoryBox->lua_game_object(), CurrentIItem()->object().lua_game_object() );

		CUICellItem* ci			= old_owner->RemoveItem(CurrentItem(), false);
		new_owner->SetItem		(ci);
	}
	SetCurrentItem					(NULL);

	return				true;
}

bool CUICarBodyWnd::OnItemStartDrag(CUICellItem* itm)
{
	return				false; //default behaviour
}

bool CUICarBodyWnd::OnItemDbClick(CUICellItem* itm)
{
	CUIDragDropListEx*	old_owner		= itm->OwnerList();
	CUIDragDropListEx*	new_owner		= (old_owner==m_pUIOthersBagList)?m_pUIOurBagList:m_pUIOthersBagList;

	if(m_pOthersObject)
	{
		if( TransferItem		(	CurrentIItem(),
								(old_owner==m_pUIOthersBagList)?m_pOthersObject:m_pOurObject, 
								(old_owner==m_pUIOurBagList)?m_pOthersObject:m_pOurObject, 
								(old_owner==m_pUIOurBagList)
								)
			)
		{
			CUICellItem* ci			= old_owner->RemoveItem(CurrentItem(), false);
			new_owner->SetItem		(ci);
		}
	}
	else
	{
		if(false && old_owner==m_pUIOurBagList) return true;
		bool bMoveDirection		= (old_owner==m_pUIOthersBagList);

		if (GameConstants::GetLimitedInvBoxes() && !bMoveDirection && m_pInventoryBox && m_pInventoryBox->GetInventoryFullness() >= GameConstants::GetInvBoxCapacity())
		{
			SDrawStaticStruct* _s = HUD().GetUI()->UIGame()->AddCustomStatic("backpack_full", true);
			_s->wnd()->SetText(CStringTable().translate("st_inv_box_full").c_str());
			return false;
		}

		u16 tmp_id				= smart_cast<CGameObject*>(m_pOurObject)->ID();

		if (m_pInventoryBox)
		{
			move_item(
				bMoveDirection ? m_pInventoryBox->ID() : tmp_id,
				bMoveDirection ? tmp_id : m_pInventoryBox->ID(),
				CurrentIItem()->object().ID());
		}
		else
		{
			move_item(
				bMoveDirection ? m_pCar->ID() : tmp_id,
				bMoveDirection ? tmp_id : m_pCar->ID(),
				CurrentIItem()->object().ID());
		}

//.		Actor()->callback		(GameObject::eInvBoxItemTake)(m_pInventoryBox->lua_game_object(), CurrentIItem()->object().lua_game_object() );

	}
	SetCurrentItem				(NULL);

	return						true;
}

bool CUICarBodyWnd::OnItemSelected(CUICellItem* itm)
{
	SetCurrentItem		(itm);
	return				false;
}

bool CUICarBodyWnd::OnItemRButtonClick(CUICellItem* itm)
{
	SetCurrentItem				(itm);
	ActivatePropertiesBox		();
	return						false;
}

void CUICarBodyWnd::move_item (u16 from_id, u16 to_id, u16 what_id)
{
	NET_Packet P;
	CGameObject::u_EventGen					(	P,
												GE_OWNERSHIP_REJECT,
												from_id
											);

	P.w_u16									(what_id);
	CGameObject::u_EventSend				(P);

	//другому инвентарю - взять вещь 
	CGameObject::u_EventGen					(	P,
												GE_OWNERSHIP_TAKE,
												to_id
											);
	P.w_u16									(what_id);
	CGameObject::u_EventSend				(P);

}

bool CUICarBodyWnd::TransferItem(PIItem itm, CInventoryOwner* owner_from, CInventoryOwner* owner_to, bool b_check)
{
	VERIFY									(NULL==m_pInventoryBox);
	CGameObject* go_from					= smart_cast<CGameObject*>(owner_from);
	CGameObject* go_to						= smart_cast<CGameObject*>(owner_to);

	if(smart_cast<CBaseMonster*>(go_to))	return false;
	if(b_check)
	{
		float invWeight						= owner_to->inventory().CalcTotalWeight();
		float maxWeight						= owner_to->inventory().GetMaxWeight();
		float itmWeight						= itm->Weight();
		if(invWeight+itmWeight >=maxWeight)	return false;
	}

	move_item(go_from->ID(), go_to->ID(), itm->object().ID());

	return true;
}

void CUICarBodyWnd::BindDragDropListEnents(CUIDragDropListEx* lst)
{
	lst->m_f_item_drop				= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUICarBodyWnd::OnItemDrop);
	lst->m_f_item_start_drag		= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUICarBodyWnd::OnItemStartDrag);
	lst->m_f_item_db_click			= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUICarBodyWnd::OnItemDbClick);
	lst->m_f_item_selected			= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUICarBodyWnd::OnItemSelected);
	lst->m_f_item_rbutton_click		= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUICarBodyWnd::OnItemRButtonClick);
	lst->m_f_item_focus_received	= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUICarBodyWnd::OnItemFocusReceive);
	lst->m_f_item_focus_lost		= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUICarBodyWnd::OnItemFocusLost);
	lst->m_f_item_focused_update	= CUIDragDropListEx::DRAG_CELL_EVENT(this,&CUICarBodyWnd::OnItemFocusedUpdate);
}

void CUICarBodyWnd::ColorizeItem(CUICellItem* itm)
{
	CInventoryItem* jitem = (CInventoryItem*)itm->m_pData;
	if (jitem->m_eItemPlace == eItemPlaceBelt || jitem->m_eItemPlace == eItemPlaceSlot)
		itm->SetColor(color_rgba(180, 255, 180, 255));
}
