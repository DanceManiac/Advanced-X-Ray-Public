#include "stdafx.h"
#include "UICellItem.h"
#include "uicursor.h"
#include "../inventory_item.h"
#include "UIDragDropListEx.h"
#include "../xr_level_controller.h"
#include "../../xrEngine/xr_input.h"
#include "../HUDManager.h"
#include "../level.h"
#include "object_broker.h"
#include "UIXmlInit.h"
#include "UIProgressBar.h"
#include "UIGameCustom.h"
#include "UIGameSP.h"
#include "UIInventoryWnd.h"
#include "HUDManager.h"

#include "eatable_item.h"
//#include "AntigasFilter.h"
#include "Artefact.h"
//#include "CustomDetector.h"
#include "Torch.h"
#include "AnomalyDetector.h"

#include "AdvancedXrayGameConstants.h"

CUICellItem* CUICellItem::m_mouse_selected_item = NULL;

CUICellItem::CUICellItem()
{
	m_pParentList		= nullptr;
	m_pData				= nullptr;
	m_custom_draw		= nullptr;
	m_text				= nullptr;
	m_custom_text		= nullptr;
	m_qmark				= nullptr;
#pragma todo("planning to port upgrades stuff")
//	m_upgrade			= nullptr;
	m_pConditionState	= nullptr;
	m_pPortionsState	= nullptr;
	m_pChargeState		= nullptr;
	m_drawn_frame		= 0;
	SetAccelerator		(0);
	m_b_destroy_childs	= true;
	m_selected			= false;
	m_select_armament	= false;
	m_cur_mark			= false;
//	m_has_upgrade		= false;
	m_is_quest			= false;
	
	init();
}

CUICellItem::~CUICellItem()
{
	if(m_b_destroy_childs)
		delete_data	(m_childs);

	delete_data		(m_custom_draw);
}

void CUICellItem::init()
{
	CUIXml	uiXml;
	uiXml.Load( CONFIG_PATH, UI_PATH, "actor_menu_item.xml" );
	
	m_text					= xr_new<CUIStatic>();
	m_text->SetAutoDelete	( true );
	AttachChild				( m_text );
	CUIXmlInit::InitStatic	( uiXml, "cell_item_text", 0, m_text );
	m_text->Show			( false );

	if (uiXml.NavigateToNode("cell_item_custom_text", 0))
	{
		m_custom_text = xr_new<CUIStatic>();
		m_custom_text->SetAutoDelete(true);
		AttachChild(m_custom_text);
		CUIXmlInit::InitStatic(uiXml, "cell_item_custom_text", 0, m_custom_text);
		m_custom_text_pos = m_custom_text->GetWndPos();
		m_custom_text->Show(false);
	}

	if (uiXml.NavigateToNode("cell_item_quest_mark", 0))
	{
		m_qmark = xr_new<CUIStatic>();
		m_qmark->SetAutoDelete(true);
		AttachChild(m_qmark);
		CUIXmlInit::InitStatic(uiXml, "cell_item_quest_mark", 0, m_qmark);
		m_qmark_pos = m_qmark->GetWndPos();
		m_qmark->Show(false);
	}

#pragma todo("planning to port upgrades stuff")
	/*m_upgrade				= xr_new<CUIStatic>();
	m_upgrade->SetAutoDelete( true );
	AttachChild				( m_upgrade );
	CUIXmlInit::InitStatic	( uiXml, "cell_item_upgrade", 0, m_upgrade );
	m_upgrade_pos			= m_upgrade->GetWndPos();
	m_upgrade->Show			( false );*/

	if (uiXml.NavigateToNode("condition_progess_bar", 0))
	{
		m_pConditionState = xr_new<CUIProgressBar>();
		m_pConditionState->SetAutoDelete(true);
		AttachChild(m_pConditionState);
		CUIXmlInit::InitProgressBar(uiXml, "condition_progess_bar", 0, m_pConditionState);
		m_pConditionState->Show(false);
	}

	if (uiXml.NavigateToNode("portions_progess_bar", 0))
	{
		m_pPortionsState = xr_new<CUIProgressBar>();
		m_pPortionsState->SetAutoDelete(true);
		AttachChild(m_pPortionsState);
		CUIXmlInit::InitProgressBar(uiXml, "portions_progess_bar", 0, m_pPortionsState);
		m_pPortionsState->Show(false);
	}

	if (uiXml.NavigateToNode("charge_level_progess_bar", 0))
	{
		m_pChargeState = xr_new<CUIProgressBar>();
		m_pChargeState->SetAutoDelete(true);
		AttachChild(m_pChargeState);
		CUIXmlInit::InitProgressBar(uiXml, "charge_level_progess_bar", 0, m_pChargeState);
		m_pChargeState->Show(false);
	}
}

void CUICellItem::Draw()
{	
	m_drawn_frame		= Device.dwFrame;
	
	inherited::Draw();
	if(m_custom_draw) 
		m_custom_draw->OnDraw(this);
};

void CUICellItem::Update()
{
	if (m_pParentList)
		EnableHeading(m_pParentList->GetVerticalPlacement() && m_pParentList);
	if(Heading())
	{
		SetHeading			( 90.0f * (PI/180.0f) );
		SetHeadingPivot		(Fvector2().set(0.0f,0.0f), Fvector2().set(0.0f,GetWndSize().y), true);
	}else
		ResetHeadingPivot	();

	inherited::Update();
	
	if ( CursorOverWindow() && m_pParentList)
	{
		Frect clientArea;
		m_pParentList->GetClientArea(clientArea);
		Fvector2 cp			= GetUICursor().GetCursorPosition();
		if(clientArea.in(cp))
			GetMessageTarget()->SendMessage(this, DRAG_DROP_ITEM_FOCUSED_UPDATE, NULL);
	}
	UpdateIndicators();
	UpdateCellItemProgressBars();
}

void CUICellItem::UpdateIndicators()
{
	PIItem item = (PIItem)m_pData;
	if ( item )
	{
#pragma todo("planning to port upgrades stuff")
		//m_has_upgrade	= item->has_any_upgrades();
		m_is_quest		= item->IsQuestItem();
		m_with_custom_text = item->m_custom_text!=nullptr;

//		Fvector2 size      = GetWndSize();
//		Fvector2 up_size = m_upgrade->GetWndSize();
//		pos.x = size.x - up_size.x - 4.0f;
		Fvector2 pos;
		/*if (m_has_upgrade)
		{
			pos.set(m_upgrade_pos);
			if (ChildsCount())
			{
				pos.x				+= m_text->GetWndSize().x + 2.0f;
			}
			m_upgrade->SetWndPos	(pos);
		}*/
		if (m_is_quest)
		{
			pos.set(m_qmark_pos);
			Fvector2 size		= GetWndSize();
			Fvector2 up_size	= m_qmark->GetWndSize();
			pos.x				= size.x - up_size.x - 4.0f;// making pos at right-end of cell
			pos.y				= size.y - up_size.y - 4.0f;// making pos at bottom-end of cell
			m_qmark->SetWndPos	(pos);
		}
		if (m_custom_text && m_with_custom_text)
		{
			pos.set(m_custom_text_pos);
			Fvector2 size		= GetWndSize();
			Fvector2 up_size	= m_custom_text->GetWndSize();
			pos.x				= size.x - up_size.x - 4.0f;// making pos at right-end of cell
			pos.y				= size.y - up_size.y - 4.0f;// making pos at bottom-end of cell
			m_custom_text->SetWndPos	(pos);
			m_custom_text->SetTextST	(*item->m_custom_text);

			if (item->m_custom_text_clr_inv != NULL)
			{
				CUIGameSP* pGameSP = smart_cast<CUIGameSP*>(HUD().GetUI()->UIGame());
				if (pGameSP && pGameSP->InventoryMenu->IsShown()) // Hrust: that's bad, but check with GetPickUpItem() makes a bug with item text colorizing in inventory when the same item is targeted
				{
					m_custom_text->SetTextColor(item->m_custom_text_clr_inv);
				}
				else
				{
					if (item->m_custom_text_clr_hud != NULL)
						m_custom_text->SetTextColor(item->m_custom_text_clr_hud);
					else
						m_custom_text->SetTextColor(item->m_custom_text_clr_inv);
				}
				m_custom_text->SetTextColor(item->m_custom_text_clr_inv);
			}

			if (item->m_custom_text_font != nullptr)
			{
				m_custom_text->SetFont(item->m_custom_text_font);
			}
		}
	}
#pragma todo("planning to port upgrades stuff")
	//m_upgrade->Show				(m_has_upgrade);
	m_qmark->Show				(m_is_quest);
	if (m_custom_text)
		m_custom_text->Show		(m_with_custom_text);
}

bool CUICellItem::OnMouseAction(float x, float y, EUIMessages mouse_action)
{
	if ( mouse_action == WINDOW_LBUTTON_DOWN )
	{
		GetMessageTarget()->SendMessage( this, DRAG_DROP_ITEM_LBUTTON_CLICK, NULL );
		GetMessageTarget()->SendMessage( this, DRAG_DROP_ITEM_SELECTED, NULL );
		m_mouse_selected_item = this;
		return false;
	}
	else if ( mouse_action == WINDOW_MOUSE_MOVE )
	{
		if ( pInput->iGetAsyncBtnState(0) && m_mouse_selected_item && m_mouse_selected_item == this )
		{
			GetMessageTarget()->SendMessage( this, DRAG_DROP_ITEM_DRAG, NULL );
			return true;
		}
	}
	else if ( mouse_action == WINDOW_LBUTTON_DB_CLICK )
	{
		GetMessageTarget()->SendMessage( this, DRAG_DROP_ITEM_DB_CLICK, NULL );
#pragma todo("do not forget about refresh eatable portions")
		//HUD().GetUI()->UIGame()->ActorMenu().SetCurrentConsumable(this);
		return true;
	}
	else if ( mouse_action == WINDOW_RBUTTON_DOWN )
	{
		GetMessageTarget()->SendMessage( this, DRAG_DROP_ITEM_RBUTTON_CLICK, NULL );
		return true;
	}
	else if ( mouse_action == WINDOW_CBUTTON_DOWN )
	{
		GetMessageTarget()->SendMessage( this, DRAG_DROP_ITEM_CBUTTON_CLICK, NULL );
		return true;
	}
	
	m_mouse_selected_item = NULL;
	return false;
};

bool CUICellItem::OnKeyboardAction(int dik, EUIMessages keyboard_action)
{
	if (WINDOW_KEY_PRESSED == keyboard_action)
	{
		if (GetAccelerator() == dik)
		{
			GetMessageTarget()->SendMessage(this, DRAG_DROP_ITEM_DB_CLICK, NULL);
			return		true;
		}
	}
	return inherited::OnKeyboardAction(dik, keyboard_action);
}

CUIDragItem* CUICellItem::CreateDragItem()
{
	CUIDragItem* tmp;
	tmp = xr_new<CUIDragItem>(this);
	Frect r;
	GetAbsoluteRect(r);

	if( m_UIStaticItem.GetFixedLTWhileHeading() )
	{
		float t1,t2;
		t1				= r.width();
		t2				= r.height()*UI().get_current_kx();

		Fvector2 cp = GetUICursor().GetCursorPosition();

		r.x1			= (cp.x-t2/2.0f);
		r.y1			= (cp.y-t1/2.0f);
		r.x2			= r.x1 + t2;
		r.y2			= r.y1 + t1;
	}

	if (Heading() && UI().is_widescreen())
	{
		r.y2 /= UI().get_current_kx() * 1.26f;
		r.x2 /= UI().get_current_kx() * 1.3f;
	}

	tmp->Init(GetShader(), r, GetUIStaticItem().GetOriginalRect());
	return tmp;
}

void CUICellItem::SetOwnerList(CUIDragDropListEx* p)	
{
	m_pParentList = p;
	UpdateCellItemProgressBars();
}

void CUICellItem::UpdateCellItemProgressBars()
{
	UpdateConditionProgressBar();

	if (m_pPortionsState)
		UpdatePortionsProgressBar();

	if (m_pChargeState)
		UpdateChargeLevelProgressBar();
}

void CUICellItem::UpdateConditionProgressBar()
{
	if (!m_pConditionState)
		return;

	if (m_pParentList && m_pParentList->GetConditionProgBarVisibility())
	{
		PIItem itm = (PIItem)m_pData;

		if (itm && itm->IsUsingCondition())
		{
			Ivector2 itm_grid_size = GetGridSize();

			if(m_pParentList->GetVerticalPlacement())
				std::swap(itm_grid_size.x, itm_grid_size.y);

			Ivector2 cell_size = m_pParentList->CellSize();
			Ivector2 cell_space = m_pParentList->CellsSpacing();
			float x = 1.f;
			float y = itm_grid_size.y * (cell_size.y + cell_space.y) - m_pConditionState->GetHeight() - 2.f;

			m_pConditionState->SetWndPos(Fvector2().set(x, y));
			m_pConditionState->SetProgressPos(iCeil(itm->GetCondition() * 13.0f) / 13.0f);

			m_pConditionState->Show(true);

			return;
		}
	}
	m_pConditionState->Show(false);
}

void CUICellItem::UpdatePortionsProgressBar()
{
	if (m_pParentList && m_pParentList->GetConditionProgBarVisibility())
	{
		PIItem itm = (PIItem)m_pData;
		CEatableItem* pEatable = smart_cast<CEatableItem*>(itm);

		if (pEatable && pEatable->GetConstPortionsNum() > 1 && pEatable->GetConstPortionsNum() <= 8)
		{
			int BasePortionsNum = (float)pEatable->GetConstPortionsNum();
			Ivector2 itm_grid_size = GetGridSize();
			if (m_pParentList->GetVerticalPlacement())
				std::swap(itm_grid_size.x, itm_grid_size.y);

			Ivector2 cell_size = m_pParentList->CellSize();
			Ivector2 cell_space = m_pParentList->CellsSpacing();
			float x = 1.f;
			float y = itm_grid_size.y * (cell_size.y + cell_space.y) - m_pPortionsState->GetHeight() - 2.f;
			Fvector2 size;
			float scaler = 1.f;

			// 8 portions is max value!!!
			switch (BasePortionsNum)
			{
				case 2:
				{
					scaler = UI().is_widescreen() ? 3.95f : 3.88f;
					break;
				}
				case 3:
				{
					scaler = UI().is_widescreen() ? 3.95f : 3.9f;
					break;
				}
				case 4:
				{
					scaler = UI().is_widescreen() ? 3.98f : 3.9f;
					break;
				}
				case 5:
				{
					scaler = UI().is_widescreen() ? 4 : 3.9f;
					break;
				}
				case 6:
				{
					scaler = UI().is_widescreen() ? 4 : 3.9f;
					break;
				}
				case 7:
				{
					scaler = UI().is_widescreen() ? 3.98f : 4;
					break;
				}
				case 8:
				{
					scaler = UI().is_widescreen() ? 4 : 3.9f;
					break;
				}
				default:
				{
					break;
				}
			}

			size.x = BasePortionsNum * scaler;
			size.y = m_pPortionsState->GetHeight();

			m_pPortionsState->SetRange(0, BasePortionsNum * scaler);
			m_pPortionsState->SetWndPos(Fvector2().set(x, y));
			m_pPortionsState->SetWndSize(size);
			m_pPortionsState->SetProgressPos(iCeil((pEatable->GetPortionsNum() * scaler) * 13.0f) / 13.0f);

			m_pPortionsState->Show(true);

			return;
		}
	}

	m_pPortionsState->Show(false);
}

void CUICellItem::UpdateChargeLevelProgressBar()
{
	if (m_pParentList && m_pParentList->GetConditionProgBarVisibility())
	{
		PIItem itm = (PIItem)m_pData;
		CTorch* torch = smart_cast<CTorch*>(itm);
		//CCustomDetector* artefact_detector = smart_cast<CCustomDetector*>(itm);
		CDetectorAnomaly* anomaly_detector = smart_cast<CDetectorAnomaly*>(itm);
		CArtefact* artefact = smart_cast<CArtefact*>(itm);

		if (torch || /*artefact_detector ||*/ anomaly_detector || artefact)
		{
			Ivector2 itm_grid_size = GetGridSize();

			if (m_pParentList->GetVerticalPlacement())
				std::swap(itm_grid_size.x, itm_grid_size.y);

			Ivector2 cell_size = m_pParentList->CellSize();
			Ivector2 cell_space = m_pParentList->CellsSpacing();
			float x = 1.f;
			float y = itm_grid_size.y * (cell_size.y + cell_space.y) - m_pChargeState->GetHeight() - 2.f;

			m_pChargeState->SetWndPos(Fvector2().set(x, y));

			if (torch && GameConstants::GetTorchHasBattery())
			{
				m_pChargeState->SetProgressPos(iCeil(torch->GetCurrentChargeLevel() * 13.0f) / 13.0f);
				m_pChargeState->Show(true);
			}
			/*else if (artefact_detector && GameConstants::GetArtDetectorUseBattery())
			{
				m_pChargeState->SetProgressPos(iCeil(artefact_detector->GetCurrentChargeLevel() * 13.0f) / 13.0f);
				m_pChargeState->Show(true);
			}*/
			else if (anomaly_detector && GameConstants::GetAnoDetectorUseBattery())
			{
				m_pChargeState->SetProgressPos(iCeil(anomaly_detector->GetCurrentChargeLevel() * 13.0f) / 13.0f);
				m_pChargeState->Show(true);
			}
			else if (artefact && GameConstants::GetArtefactsDegradation())
			{
				m_pChargeState->SetProgressPos(iCeil(artefact->GetCurrentChargeLevel() * 13.0f) / 13.0f);
				m_pChargeState->Show(true);
			}
			else
				m_pChargeState->Show(false);

			return;
		}
	}
	m_pChargeState->Show(false);
}

bool CUICellItem::EqualTo(CUICellItem* itm)
{
	return (m_grid_size.x==itm->GetGridSize().x) && (m_grid_size.y==itm->GetGridSize().y);
}

u32 CUICellItem::ChildsCount()
{
	return m_childs.size();
}

void CUICellItem::PushChild(CUICellItem* c)
{
	R_ASSERT(c->ChildsCount()==0);
	VERIFY				(this!=c);
	m_childs.push_back	(c);
	UpdateItemText		();
}

CUICellItem* CUICellItem::PopChild(CUICellItem* needed)
{
	CUICellItem* itm	= m_childs.back();
	m_childs.pop_back	();
	
	if(needed)
	{	
	  if(itm!=needed)
		std::swap		(itm->m_pData, needed->m_pData);
	}else
	{
		std::swap		(itm->m_pData, m_pData);
	}
	UpdateItemText		();
	R_ASSERT			(itm->ChildsCount()==0);
	itm->SetOwnerList	(NULL);
	return				itm;
}

bool CUICellItem::HasChild(CUICellItem* item)
{
	return (m_childs.end() != std::find(m_childs.begin(), m_childs.end(), item) );
}

void CUICellItem::UpdateItemText()
{
	if ( ChildsCount() )
	{
		string32	str;
		xr_sprintf( str, "x%d", ChildsCount()+1 );
		m_text->SetText( str );
		m_text->Show( true );
	}
	else
	{
		m_text->SetText( "" );
		m_text->Show( false );
	}
}

void CUICellItem::Mark( bool status )
{
	m_cur_mark = status;
}

void CUICellItem::SetCustomDraw(ICustomDrawCellItem* c)
{
	if (m_custom_draw)
		xr_delete(m_custom_draw);
	m_custom_draw = c;
}

// -------------------------------------------------------------------------------------------------

CUIDragItem::CUIDragItem(CUICellItem* parent)
{
	m_custom_draw					= NULL;
	m_back_list						= NULL;
	m_pParent						= parent;
	AttachChild						(&m_static);
	Device.seqRender.Add			(this, REG_PRIORITY_LOW-5000);
	Device.seqFrame.Add				(this, REG_PRIORITY_LOW-5000);
	VERIFY							(m_pParent->GetMessageTarget());
}

CUIDragItem::~CUIDragItem()
{
	Device.seqRender.Remove			(this);
	Device.seqFrame.Remove			(this);
	delete_data						(m_custom_draw);
}

void CUIDragItem::SetCustomDraw(ICustomDrawDragItem* c)
{
	if (m_custom_draw)
		xr_delete(m_custom_draw);
	m_custom_draw = c;
}

void CUIDragItem::Init(const ui_shader& sh, const Frect& rect, const Frect& text_rect)
{
	SetWndRect						(rect);
	m_static.SetShader				(sh);
	m_static.SetOriginalRect		(text_rect);
	m_static.SetWndPos				(0.0f,0.0f);
	m_static.SetWndSize				(GetWndSize());
	m_static.TextureOn				();
	m_static.SetColor				(color_rgba(255,255,255,170));
	m_static.SetStretchTexture		(true);
	m_pos_offset.sub				(rect.lt, GetUICursor().GetCursorPosition());
}

bool CUIDragItem::OnMouseAction(float x, float y, EUIMessages mouse_action)
{
	if(mouse_action == WINDOW_LBUTTON_UP)
	{
		m_pParent->GetMessageTarget()->SendMessage(m_pParent,DRAG_DROP_ITEM_DROP,NULL);
		return true;
	}
	return false;
}

void CUIDragItem::OnRender()
{
	Draw			();
}

void CUIDragItem::OnFrame()
{
	Update			();
}

void CUIDragItem::Draw()
{
	Fvector2 tmp;
	tmp.sub					(GetWndPos(), GetUICursor().GetCursorPosition());
	tmp.sub					(m_pos_offset);
	tmp.mul					(-1.0f);
	MoveWndDelta			(tmp);
	inherited::Draw			();
	if(m_custom_draw) 
		m_custom_draw->OnDraw(this);
}

void CUIDragItem::SetBackList(CUIDragDropListEx* l)
{
	if(m_back_list)
		m_back_list->OnDragEvent(this, false);

	m_back_list					= l;

	if(m_back_list)
		l->OnDragEvent			(this, true);
}

Fvector2 CUIDragItem::GetPosition()
{
	return Fvector2().add(m_pos_offset, GetUICursor().GetCursorPosition());
}

