#include "stdafx.h"
#include "UICellCustomItems.h"
#include "UIInventoryUtilities.h"
#include "../Weapon.h"
#include "UIDragDropListEx.h"
#include "UIProgressBar.h"
#include "../eatable_item.h"
#include "../Artefact.h"
#include "../CustomOutfit.h"
#include "../ActorHelmet.h"
#include "../AntigasFilter.h"
#include "../AnomalyDetector.h"
#include "../Torch.h"
#include "../ArtefactContainer.h"
#include "../Weapon.h"

#define INV_BACKGR_ICON_NAME "__bgr_icon"  // Название CUIStatic иконки, которое используется для определения порядка отрисовки аддонов оружия --#SM+#--

namespace detail 
{

struct is_helper_pred
{
	bool operator ()(CUICellItem* child)
	{
		return child->IsHelper();
	}

}; // struct is_helper_pred

} //namespace detail 


CUIInventoryCellItem::CUIInventoryCellItem(CInventoryItem* itm)
{
	m_pData											= (void*)itm;

	inherited::SetShader							(InventoryUtilities::GetEquipmentIconsShader());

	m_grid_size.set									(itm->GetInvGridRect().rb);
	Frect rect{}; 
	rect.lt.set										(	UI().inv_grid_kx() * itm->GetXPos(),
														UI().inv_grid_kx() * itm->GetYPos());

	rect.rb.set										(	rect.lt.x + UI().inv_grid_kx() * m_grid_size.x,
														rect.lt.y + UI().inv_grid_kx() * m_grid_size.y);

	inherited::SetOriginalRect						(rect);
	inherited::SetStretchTexture					(true);
}

bool CUIInventoryCellItem::EqualTo(CUICellItem* itm)
{
	CUIInventoryCellItem* ci = smart_cast<CUIInventoryCellItem*>( itm );
	if ( !itm )
	{
		return false;
	}
	if ( object()->object().cNameSect() != ci->object()->object().cNameSect() )
	{
		return false;
	}
	if ( !fsimilar( object()->GetCondition(), ci->object()->GetCondition(), 0.01f ) )
	{
		return false;
	}
	/*if ( !object()->equal_upgrades( ci->object()->upgardes() ) )
	{
		return false;
	}*/
	if (object()->m_eItemPlace != ci->object()->m_eItemPlace)
	{
		return false;
	}
	auto eatable = smart_cast<CEatableItem*>(object());
	if (eatable && eatable->GetPortionsNum() != smart_cast<CEatableItem*>(ci->object())->GetPortionsNum())
	{
		return false;
	}
	auto artefact = smart_cast<CArtefact*>(object());
	if (artefact && artefact->GetCurrentChargeLevel() != smart_cast<CArtefact*>(ci->object())->GetCurrentChargeLevel())
	{
		return false;
	}
	if (artefact && artefact->GetCurrentAfRank() != smart_cast<CArtefact*>(ci->object())->GetCurrentAfRank())
	{
		return false;
	}
	auto outfit = smart_cast<CCustomOutfit*>(object());
	if (outfit && outfit->GetFilterCondition() != smart_cast<CCustomOutfit*>(ci->object())->GetFilterCondition())
	{
		return false;
	}
	auto helmet = smart_cast<CHelmet*>(object());
	if (helmet && helmet->GetFilterCondition() != smart_cast<CHelmet*>(ci->object())->GetFilterCondition())
	{
		return false;
	}
	auto filter = smart_cast<CAntigasFilter*>(object());
	if (filter && filter->GetFilterCondition() != smart_cast<CAntigasFilter*>(ci->object())->GetFilterCondition())
	{
		return false;
	}
	auto torch = smart_cast<CTorch*>(object());
	if (torch && torch->GetCurrentChargeLevel() != smart_cast<CTorch*>(ci->object())->GetCurrentChargeLevel())
	{
		return false;
	}
	/*auto art_det = smart_cast<CCustomDetector*>(object());
	if (art_det && art_det->GetCurrentChargeLevel() != smart_cast<CCustomDetector*>(ci->object())->GetCurrentChargeLevel())
	{
		return false;
	}*/
	auto ano_det = smart_cast<CDetectorAnomaly*>(object());
	if (ano_det && ano_det->GetCurrentChargeLevel() != smart_cast<CDetectorAnomaly*>(ci->object())->GetCurrentChargeLevel())
	{
		return false;
	}
	auto art_con = smart_cast<CArtefactContainer*>(object());
	if (art_con && art_con->GetArtefactsInside() != smart_cast<CArtefactContainer*>(ci->object())->GetArtefactsInside())
	{
		return false;
	}
	auto wpn = smart_cast<CWeapon*>(object());
	if (wpn && wpn->m_weapon_attaches != smart_cast<CWeapon*>(ci->object())->m_weapon_attaches)
	{
		return false;
	}
	return true;
}

bool CUIInventoryCellItem::IsHelperOrHasHelperChild()
{
	return std::count_if(m_childs.begin(), m_childs.end(), detail::is_helper_pred()) > 0 || IsHelper();
}

CUIDragItem* CUIInventoryCellItem::CreateDragItem()
{
	return IsHelperOrHasHelperChild() ? NULL : inherited::CreateDragItem();
}

bool CUIInventoryCellItem::IsHelper ()
{
	return object()->is_helper_item();
}

void CUIInventoryCellItem::SetIsHelper (bool is_helper)
{
	object()->set_is_helper(is_helper);
}

void CUIInventoryCellItem::Update()
{
	inherited::Update	();
	UpdateItemText();

	u32 color = GetColor();
	if ( IsHelper() && !ChildsCount() )
	{
		color = 0xbbbbbbbb;
	}
	else if ( IsHelperOrHasHelperChild() )
	{
		color = 0xffffffff;
	}

	SetColor(color);
}

void CUIInventoryCellItem::UpdateItemText()
{
	const u32	helper_count	=  	(u32)std::count_if(m_childs.begin(), m_childs.end(), detail::is_helper_pred()) 
									+ IsHelper() ? 1 : 0;

	const u32	count			=	ChildsCount() + 1 - helper_count;

	string32	str;

	if ( count > 1 || helper_count )
	{
		xr_sprintf						( str, "x%d", count );
		m_text->SetText	( str );
		m_text->Show					( true );
	}
	else
	{
		xr_sprintf						( str, "");
		m_text->SetText	( str );
		m_text->Show					( false );
	}
}

CUIAmmoCellItem::CUIAmmoCellItem(CWeaponAmmo* itm)
:inherited(itm)
{}

bool CUIAmmoCellItem::EqualTo(CUICellItem* itm)
{
	if(!inherited::EqualTo(itm))	return false;

	CUIAmmoCellItem* ci				= smart_cast<CUIAmmoCellItem*>(itm);
	if(!ci)							return false;

	return					( (object()->cNameSect() == ci->object()->cNameSect()) );
}

CUIDragItem* CUIAmmoCellItem::CreateDragItem()
{
	return IsHelper() ? NULL : inherited::CreateDragItem();
}

u32 CUIAmmoCellItem::CalculateAmmoCount()
{
	xr_vector<CUICellItem*>::iterator it   = m_childs.begin();
	xr_vector<CUICellItem*>::iterator it_e = m_childs.end();

	u32 total	= IsHelper() ? 0 : object()->m_boxCurr;
	for ( ; it != it_e; ++it )
	{
		CUICellItem* child = *it;

		if ( !child->IsHelper() )
		{
			total += ((CUIAmmoCellItem*)(*it))->object()->m_boxCurr;
		}
	}

	return total;
}

void CUIAmmoCellItem::UpdateItemText()
{
	m_text->Show( false );
	if ( !m_custom_draw )
	{
		const u32 total = CalculateAmmoCount();
		
		string32			str;
		xr_sprintf			(str, "%d", total);
		m_text->SetText(str);
		m_text->Show		(true);
	}
}


CUIWeaponCellItem::CUIWeaponCellItem(CWeapon* itm)
:inherited(itm)
{
	m_addons[eSilencer]		= nullptr;
	m_addons[eScope]		= nullptr;
	m_addons[eLauncher]		= nullptr;
	m_addons[eLaser]		= nullptr;
	m_addons[eTorch]		= nullptr;

	if(itm->SilencerAttachable())
		m_addon_offset[eSilencer].set(object()->GetSilencerX(), object()->GetSilencerY());

	if(itm->ScopeAttachable())
		m_addon_offset[eScope].set(object()->GetScopeX(), object()->GetScopeY());

	if(itm->GrenadeLauncherAttachable())
		m_addon_offset[eLauncher].set(object()->GetGrenadeLauncherX(), object()->GetGrenadeLauncherY());

	if (itm->LaserAttachable())
		m_addon_offset[eLaser].set(object()->GetLaserDesignatorX(), object()->GetLaserDesignatorY());

	if (itm->TacticalTorchAttachable())
		m_addon_offset[eTorch].set(object()->GetTacticalTorchX(), object()->GetTacticalTorchY());
}

#include "object_broker.h"
CUIWeaponCellItem::~CUIWeaponCellItem()
{
}

bool CUIWeaponCellItem::is_scope()
{
	return object()->ScopeAttachable()&&object()->IsScopeAttached();
}

bool CUIWeaponCellItem::is_silencer()
{
	return object()->SilencerAttachable()&&object()->IsSilencerAttached();
}

bool CUIWeaponCellItem::is_launcher()
{
	return object()->GrenadeLauncherAttachable()&&object()->IsGrenadeLauncherAttached();
}

bool CUIWeaponCellItem::is_laser()
{
	return object()->LaserAttachable() && object()->IsLaserAttached();
}

bool CUIWeaponCellItem::is_torch()
{
	return object()->TacticalTorchAttachable() && object()->IsTacticalTorchAttached();
}

void CUIWeaponCellItem::CreateIcon(eAddonType t, const shared_str& sAddonName) //--#SM+#--
{
	if(m_addons[t])				return;
	m_addons[t]					= xr_new<CUIStatic>();	
	m_addons[t]->SetAutoDelete	(true);
	AttachChild					(m_addons[t]);
	m_addons[t]->SetShader		(InventoryUtilities::GetEquipmentIconsShader());

	// Регулируем порядок отрисовки иконок аддонов --#SM+#--
	bool bIconToBackground = READ_IF_EXISTS(pSettings, r_bool, sAddonName, "inv_icon_to_back", false);
	if (bIconToBackground)
	{
		m_addons[t]->SetWindowName(INV_BACKGR_ICON_NAME);
	}

	m_addons[t]->SetColor		(GetColor());
}

void CUIWeaponCellItem::DestroyIcon(eAddonType t)
{
	DetachChild		(m_addons[t]);
	m_addons[t]		= nullptr;
}

CUIStatic* CUIWeaponCellItem::GetIcon(eAddonType t)
{
	return m_addons[t];
}
void CUIWeaponCellItem::RefreshOffset()
{
	if(object()->SilencerAttachable())
		m_addon_offset[eSilencer].set(object()->GetSilencerX(), object()->GetSilencerY());

	if(object()->ScopeAttachable())
		m_addon_offset[eScope].set(object()->GetScopeX(), object()->GetScopeY());

	if(object()->GrenadeLauncherAttachable())
		m_addon_offset[eLauncher].set(object()->GetGrenadeLauncherX(), object()->GetGrenadeLauncherY());

	if (object()->LaserAttachable())
		m_addon_offset[eLaser].set(object()->GetLaserDesignatorX(), object()->GetLaserDesignatorY());

	if (object()->TacticalTorchAttachable())
		m_addon_offset[eTorch].set(object()->GetTacticalTorchX(), object()->GetTacticalTorchY());
}

void CUIWeaponCellItem::Draw()
{
	// Рисуем только аддоны заднего плана
	bool bBackgrIconsFound = false;
	for (auto it = m_ChildWndList.begin(); m_ChildWndList.end() != it; ++it)
	{
		if (auto pStatic = smart_cast<CUIStatic*>(*it))
		{
			if (pStatic->WindowName().equal(INV_BACKGR_ICON_NAME))
			{
				bBackgrIconsFound = true;
				pStatic->TextureOn(); //--> Включаем текстуру у аддонов заднего плана
			}
			else
				pStatic->TextureOff(); //--> Отключаем текстуру у аддонов переднего плана
		}
	}
	if (bBackgrIconsFound == true)
	{
		TextureOff(); //--> Отключаем текстуру оружия
		inherited::Draw(); //--> Рисуем иконки заднего плана
		TextureOn(); //--> Включаем текстуру оружия
	}

	// Рисуем только оружие и аддоны переднего плана
	for (auto it = m_ChildWndList.begin(); m_ChildWndList.end() != it; ++it)
	{
		if (auto pStatic = smart_cast<CUIStatic*>(*it))
		{
			if (pStatic->WindowName().equal(INV_BACKGR_ICON_NAME))
				pStatic->TextureOff(); //--> Отключаем текстуру у аддонов заднего плана
			else
				pStatic->TextureOn(); //--> Включаем текстуру у аддонов переднего плана
		}
	}
	inherited::Draw(); //--> Рисуем оружие и иконки переднего плана

	//if(m_upgrade && m_upgrade->IsShown())
	//	m_upgrade->Draw();
}

void CUIWeaponCellItem::Update()
{
	bool b						= Heading();
	inherited::Update			();
	
	bool bForceReInitAddons		= (b!=Heading());

	if (object()->SilencerAttachable())
	{
		if (object()->IsSilencerAttached())
		{
			if (!GetIcon(eSilencer) || bForceReInitAddons)
			{
				CreateIcon	(eSilencer, object()->GetSilencerName());
				RefreshOffset();
				InitAddon	(GetIcon(eSilencer), *object()->GetSilencerName(), m_addon_offset[eSilencer], Heading());
			}
		}
		else
		{
			if (m_addons[eSilencer])
				DestroyIcon(eSilencer);
		}
	}

	if (object()->ScopeAttachable()){
		if (object()->IsScopeAttached())
		{
			if (!GetIcon(eScope) || bForceReInitAddons)
			{
				CreateIcon	(eScope, object()->GetScopeName());
				RefreshOffset();
				InitAddon	(GetIcon(eScope), *object()->GetScopeName(), m_addon_offset[eScope], Heading());
			}
		}
		else
		{
			if (m_addons[eScope])
				DestroyIcon(eScope);
		}
	}

	if (object()->GrenadeLauncherAttachable()){
		if (object()->IsGrenadeLauncherAttached())
		{
			if (!GetIcon(eLauncher) || bForceReInitAddons)
			{
				CreateIcon	(eLauncher, object()->GetGrenadeLauncherName());
				RefreshOffset();
				InitAddon	(GetIcon(eLauncher), *object()->GetGrenadeLauncherName(), m_addon_offset[eLauncher], Heading());
			}
		}
		else
		{
			if (m_addons[eLauncher])
				DestroyIcon(eLauncher);
		}
	}

	if (object()->LaserAttachable())
	{
		if (object()->IsLaserAttached())
		{
			if (!GetIcon(eLaser) || bForceReInitAddons)
			{
				CreateIcon(eLaser, object()->GetLaserName());
				RefreshOffset();
				InitAddon(GetIcon(eLaser), *object()->GetLaserName(), m_addon_offset[eLaser], Heading());
			}
		}
		else
		{
			if (m_addons[eLaser])
				DestroyIcon(eLaser);
		}
	}

	if (object()->TacticalTorchAttachable())
	{
		if (object()->IsTacticalTorchAttached())
		{
			if (!GetIcon(eTorch) || bForceReInitAddons)
			{
				CreateIcon(eTorch, object()->GetTacticalTorchName());
				RefreshOffset();
				InitAddon(GetIcon(eTorch), *object()->GetTacticalTorchName(), m_addon_offset[eTorch], Heading());
			}
		}
		else
		{
			if (m_addons[eTorch])
				DestroyIcon(eTorch);
		}
	}
}

void CUIWeaponCellItem::SetColor( u32 color )
{
	inherited::SetColor( color );
	if ( m_addons[eSilencer] )
	{
		m_addons[eSilencer]->SetColor( color );
	}
	if ( m_addons[eScope] )
	{
		m_addons[eScope]->SetColor( color );
	}
	if ( m_addons[eLauncher] )
	{
		m_addons[eLauncher]->SetColor( color );
	}
	if (m_addons[eLaser])
	{
		m_addons[eLaser]->SetColor(color);
	}
	if (m_addons[eTorch])
	{
		m_addons[eTorch]->SetTextureColor(color);
	}
}

void CUIWeaponCellItem::OnAfterChild(CUIDragDropListEx* parent_list)
{
	if(is_silencer() && GetIcon(eSilencer))
		InitAddon	(GetIcon(eSilencer), *object()->GetSilencerName(),	m_addon_offset[eSilencer], parent_list->GetVerticalPlacement());

	if(is_scope() && GetIcon(eScope))
		InitAddon	(GetIcon(eScope),	*object()->GetScopeName(),		m_addon_offset[eScope], parent_list->GetVerticalPlacement());

	if(is_launcher() && GetIcon(eLauncher))
		InitAddon	(GetIcon(eLauncher), *object()->GetGrenadeLauncherName(),m_addon_offset[eLauncher], parent_list->GetVerticalPlacement());

	if (is_laser() && GetIcon(eLaser))
		InitAddon	(GetIcon(eLaser), *object()->GetLaserName(), m_addon_offset[eLaser], parent_list->GetVerticalPlacement());

	if (is_torch() && GetIcon(eTorch))
		InitAddon	(GetIcon(eTorch), *object()->GetTacticalTorchName(), m_addon_offset[eTorch], parent_list->GetVerticalPlacement());
}

void CUIWeaponCellItem::InitAddon(CUIStatic* s, LPCSTR section, Fvector2 addon_offset, bool b_rotate, bool is_dragging, bool is_scope, bool is_silencer, bool is_gl)
{
	
		Frect					tex_rect{};
		Fvector2				base_scale{};

		if(Heading())
		{
			base_scale.x			= GetHeight()/(UI().inv_grid_kx() * m_grid_size.x);
			base_scale.y			= GetWidth()/(UI().inv_grid_kx() * m_grid_size.y);
		}
		else
		{
			base_scale.x			= GetWidth()/(UI().inv_grid_kx() * m_grid_size.x);
			base_scale.y			= GetHeight()/(UI().inv_grid_kx() * m_grid_size.y);
		}
		Fvector2				cell_size{};
		cell_size.x				= pSettings->r_u32(section, "inv_grid_width")*UI().inv_grid_kx();
		cell_size.y				= pSettings->r_u32(section, "inv_grid_height")*UI().inv_grid_kx();

		tex_rect.x1				= pSettings->r_u32(section, "inv_grid_x")*UI().inv_grid_kx();
		tex_rect.y1				= pSettings->r_u32(section, "inv_grid_y")*UI().inv_grid_kx();

		tex_rect.rb.add			(tex_rect.lt,cell_size);

		cell_size.mul			(base_scale);

		if (is_dragging && Heading() && UI().is_widescreen())
		{
			if (is_scope)
			{
				addon_offset.y *= UI().get_current_kx();
			}

			if (is_silencer)
			{
				addon_offset.y *= UI().get_current_kx() * 1.8f;
			}

			if (is_gl)
			{
				addon_offset.y *= UI().get_current_kx() * 1.5f;
			}
			addon_offset.x *= UI().get_current_kx() / 0.9f;
			cell_size.x /= UI().get_current_kx() * 1.6f;
			cell_size.y *= UI().get_current_kx() * 1.6f;
		}
		if (!is_dragging)
		{
			if (b_rotate)
			{
				s->SetWndSize(Fvector2().set(cell_size.y, cell_size.x));
				Fvector2 new_offset;
				new_offset.x = addon_offset.y * base_scale.x;
				new_offset.y = GetHeight() - addon_offset.x * base_scale.x - cell_size.x;
				addon_offset = new_offset;
				addon_offset.x *= UI().get_current_kx();
			}
			else
			{
				s->SetWndSize(cell_size);
				addon_offset.mul(base_scale);
			}
		}
		else
		{
			if (b_rotate)
			{
				s->SetWndSize(Fvector2().set(cell_size.y, cell_size.x));
				Fvector2 new_offset;
				new_offset.x = addon_offset.y * base_scale.x;
				new_offset.y = GetHeight() - addon_offset.x * base_scale.x - cell_size.x;
				addon_offset = new_offset;
				addon_offset.x *= UI().get_current_kx();
			}
			else
			{
				s->SetWndSize(cell_size);
				addon_offset.mul(base_scale);
			}
		}

		s->SetWndPos			(addon_offset);
		s->SetOriginalRect		(tex_rect);
		s->SetStretchTexture	(true);

		s->EnableHeading		(b_rotate);
		
		if(b_rotate)
		{
			s->SetHeading			(GetHeading());
			Fvector2 offs{};
			offs.set				(0.0f, s->GetWndSize().y);
			s->SetHeadingPivot		(Fvector2().set(0.0f,0.0f), /*Fvector2().set(0.0f,0.0f)*/offs, true);
		}
}

CUIDragItem* CUIWeaponCellItem::CreateDragItem()
{
	CUIDragItem* i		= inherited::CreateDragItem();
	CUIStatic* s		= NULL;

	if(GetIcon(eSilencer))
	{
		s				= xr_new<CUIStatic>(); s->SetAutoDelete(true);
		s->SetShader	(InventoryUtilities::GetEquipmentIconsShader());
		InitAddon		(s, *object()->GetSilencerName(), m_addon_offset[eSilencer], false, true, is_scope(), is_silencer(), is_launcher());
		s->SetColor		(i->wnd()->GetColor());
		i->wnd			()->AttachChild	(s);
	}
	
	if(GetIcon(eScope))
	{
		s				= xr_new<CUIStatic>(); s->SetAutoDelete(true);
		s->SetShader	(InventoryUtilities::GetEquipmentIconsShader());
		InitAddon		(s,	*object()->GetScopeName(),		m_addon_offset[eScope], false, true, is_scope(), is_silencer(), is_launcher());
		s->SetColor		(i->wnd()->GetColor());
		i->wnd			()->AttachChild	(s);
	}

	if(GetIcon(eLauncher))
	{
		s				= xr_new<CUIStatic>(); s->SetAutoDelete(true);
		s->SetShader	(InventoryUtilities::GetEquipmentIconsShader());
		InitAddon		(s, *object()->GetGrenadeLauncherName(),m_addon_offset[eLauncher], false, true, is_scope(), is_silencer(), is_launcher());
		s->SetColor		(i->wnd()->GetColor());
		i->wnd			()->AttachChild	(s);
	}

	if (GetIcon(eLaser))
	{
		s				= xr_new<CUIStatic>(); s->SetAutoDelete(true);
		s->SetShader	(InventoryUtilities::GetEquipmentIconsShader());
		InitAddon		(s, *object()->GetLaserName(), m_addon_offset[eLaser], false, true, is_scope(), is_silencer(), is_launcher());
		s->SetColor		(i->wnd()->GetTextureColor());
		i->wnd			()->AttachChild(s);
	}

	if (GetIcon(eTorch))
	{
		s				= xr_new<CUIStatic>(); s->SetAutoDelete(true);
		s->SetShader	(InventoryUtilities::GetEquipmentIconsShader());
		InitAddon		(s, *object()->GetTacticalTorchName(), m_addon_offset[eTorch], false, true, is_scope(), is_silencer(), is_launcher());
		s->SetColor		(i->wnd()->GetTextureColor());
		i->wnd			()->AttachChild(s);
	}

	return				i;
}

bool CUIWeaponCellItem::EqualTo(CUICellItem* itm)
{
	if(!inherited::EqualTo(itm))	return false;

	CUIWeaponCellItem* ci			= smart_cast<CUIWeaponCellItem*>(itm);
	if(!ci)							return false;

//	bool b_addons					= ( (object()->GetAddonsState() == ci->object()->GetAddonsState()) );
	if ( object()->GetAddonsState() != ci->object()->GetAddonsState() )
	{
		return false;
	}
	if(this->is_scope() && ci->is_scope())
	{
		if ( object()->GetScopeName() != ci->object()->GetScopeName() )
		{
			return false;
		}
	}
//	bool b_place					= ( (object()->m_eItemCurrPlace == ci->object()->m_eItemCurrPlace) );

	return true;
}

CBuyItemCustomDrawCell::CBuyItemCustomDrawCell	(LPCSTR str, CGameFont* pFont)
{
	m_pFont		= pFont;
	VERIFY		(xr_strlen(str)<16);
	xr_strcpy		(m_string,str);
}

void CBuyItemCustomDrawCell::OnDraw(CUICellItem* cell)
{
	Fvector2							pos;
	cell->GetAbsolutePos				(pos);
	UI().ClientToScreenScaled			(pos, pos.x, pos.y);
	m_pFont->Out						(pos.x, pos.y, m_string);
	m_pFont->OnRender					();
}
