#include "stdafx.h"
#include "UIDragDropReferenceList.h"
#include "UICellItem.h"
#include "UICellItemFactory.h"
#include "UIStatic.h"
#include "../inventory.h"
#include "../inventoryOwner.h"
#include "../actor.h"
#include "../actor_defs.h"
#include "UIInventoryUtilities.h"
#include "../../xrEngine/xr_input.h"
#include "../UICursor.h"
#include "UICellItemFactory.h"

CUIDragDropReferenceList::CUIDragDropReferenceList()
{
	AddCallbackStr("cell_item_reference", WINDOW_LBUTTON_DB_CLICK, CUIWndCallback::void_function(this, &CUIDragDropReferenceList::OnItemDBClick));
}
CUIDragDropReferenceList::~CUIDragDropReferenceList()
{
}
void CUIDragDropReferenceList::Initialize(bool is_horizontal)
{
	horizontal = is_horizontal;

	for(int i=0; horizontal ? i<m_container->CellsCapacity().x : i < m_container->CellsCapacity().y; i++)
	{
		m_references.push_back(xr_new<CUIStatic>());
		Fvector2 pos;
		if (horizontal)
			pos = Fvector2().set((m_container->CellSize().x + m_container->CellsSpacing().x) * i, 0);
		else
			pos = Fvector2().set(0, (m_container->CellSize().y + m_container->CellsSpacing().y) * i);

		m_references.back()->SetAutoDelete(true);
		m_references.back()->SetWndPos(pos);
		m_references.back()->SetWndSize(Fvector2().set(m_container->CellSize().x, m_container->CellSize().y));
		AttachChild(m_references.back());
		m_references.back()->SetWindowName("cell_item_reference");
		Register(m_references.back());
	}
}
void CUIDragDropReferenceList::SetItem(CUICellItem* itm)
{
	inherited::SetItem(itm);
}
void CUIDragDropReferenceList::SetItem(CUICellItem* itm, Fvector2 abs_pos)
{
	const Ivector2 dest_cell_pos = m_container->PickCell(abs_pos);
	if(m_container->ValidCell(dest_cell_pos) && m_container->IsRoomFree(dest_cell_pos,itm->GetGridSize()))
		SetItem(itm, dest_cell_pos);
	else
	{
		if(dest_cell_pos.x!=-1&&dest_cell_pos.y!=-1)
		{
			CUICellItem* old_itm = GetCellAt(dest_cell_pos).m_item;
			RemoveItem(old_itm, false);
			SetItem(itm, dest_cell_pos);
		}
	}
}
void CUIDragDropReferenceList::SetItem(CUICellItem* itm, Ivector2 cell_pos)
{
	CUIStatic* ref = horizontal ? m_references[cell_pos.x] : m_references[cell_pos.y];
	ref->SetShader(itm->GetShader());
	ref->SetTextureRect(itm->GetTextureRect());
	ref->TextureOn();
	ref->SetTextureColor(color_rgba(255,255,255,255));
	ref->SetStretchTexture(true);

	CUICell& C = m_container->GetCellAt(cell_pos);
	if(C.m_item!=itm)
	{
		m_container->PlaceItemAtPos(itm, cell_pos);
		itm->SetWindowName("cell_item");
		Register(itm);
		itm->SetOwnerList(this);
	}
}
CUICellItem* CUIDragDropReferenceList::RemoveItem(CUICellItem* itm, bool force_root)
{
	Ivector2 vec2 = m_container->GetItemPos(itm);
	if(vec2.x!=-1&&vec2.y!=-1)
	{
		u8 index = u8(horizontal ? vec2.x : vec2.y);
		xr_strcpy(ACTOR_DEFS::g_quick_use_slots[index], "");
		m_references[index]->SetTextureColor(color_rgba(255,255,255,0));
	}
	inherited::RemoveItem(itm, force_root);
	return NULL;
}

void CUIDragDropReferenceList::LoadItemTexture(LPCSTR section, Ivector2 cell_pos)
{
	CUIStatic* ref = horizontal ? m_references[cell_pos.x] : m_references[cell_pos.y];
	ref->SetShader(InventoryUtilities::GetEquipmentIconsShader());
	Frect texture_rect{};
	texture_rect.x1	= pSettings->r_float(section, "inv_grid_x")		* UI().inv_grid_kx();
	texture_rect.y1	= pSettings->r_float(section, "inv_grid_y")		* UI().inv_grid_kx();
	texture_rect.x2	= pSettings->r_float(section, "inv_grid_width")	* UI().inv_grid_kx();
	texture_rect.y2	= pSettings->r_float(section, "inv_grid_height")* UI().inv_grid_kx();
	texture_rect.rb.add(texture_rect.lt);
	ref->SetTextureRect(texture_rect);
	ref->TextureOn();
	ref->SetTextureColor(color_rgba(255,255,255,255));
	ref->SetStretchTexture(true);
}

void CUIDragDropReferenceList::ReloadReferences(CInventoryOwner* pActor)
{
	if (!pActor)
		return;

	if(m_drag_item)
		DestroyDragItem();

	m_container->ClearAll(true);
	m_selected_item	= NULL;

	for(u8 i=0; i < (horizontal ? m_container->CellsCapacity().x : m_container->CellsCapacity().y); i++)
	{
		CUIStatic* ref = m_references[i];
		LPCSTR item_name = ACTOR_DEFS::g_quick_use_slots[i];
		if(item_name && xr_strlen(item_name))
		{
			PIItem itm = pActor->inventory().GetAny(item_name);
			if(itm)
			{
				SetItem(create_cell_item(itm), horizontal ? Ivector2().set(i, 0) : Ivector2().set(0, i));
			}
			else
			{
				LoadItemTexture(item_name, horizontal ? Ivector2().set(i, 0) : Ivector2().set(0, i));
				ref->SetTextureColor(color_rgba(255,255,255,100));
			}
		}
		else
		{
			ref->SetTextureColor(color_rgba(255,255,255,0));
		}
	}
}

void CUIDragDropReferenceList::OnItemDBClick(CUIWindow* w, void* pData)
{
	CUIStatic* ref = smart_cast<CUIStatic*>(w);
	ITEMS_REFERENCES_VEC_IT it = std::find(m_references.begin(), m_references.end(), ref);
	if(it != m_references.end())
	{
		u8 index = u8(it-m_references.begin());
		CActor* actor = smart_cast<CActor*>(Level().CurrentViewEntity());
		if(actor)
		{
			PIItem itm = actor->inventory().GetAny(ACTOR_DEFS::g_quick_use_slots[index]);
			if(itm)
				inherited::RemoveItem(GetCellAt(horizontal ? Ivector2().set(index, 0) 
					: Ivector2().set(0, index)
					).m_item, false);
		}
		xr_strcpy(ACTOR_DEFS::g_quick_use_slots[index], "");
		(*it)->SetTextureColor(color_rgba(255,255,255,0));
	}
}

void CUIDragDropReferenceList::OnItemDrop(CUIWindow* w, void* pData)
{
	OnItemSelected(w, pData);
	CUICellItem* itm = smart_cast<CUICellItem*>(w);
	VERIFY(itm->OwnerList() == itm->OwnerList());

	if(m_f_item_drop && m_f_item_drop(itm))
	{
		DestroyDragItem();
		return;
	}

	CUIDragDropListEx*	old_owner		= itm->OwnerList();
	CUIDragDropListEx*	new_owner		= m_drag_item->BackList();
	if(old_owner && new_owner && old_owner!=new_owner)
	{
		inherited::OnItemDrop(w, pData);
		return;
	}

	CActor* actor = smart_cast<CActor*>(Level().CurrentViewEntity());
	if(actor)
	{
		Ivector2 vec = PickCell(GetUICursor().GetCursorPosition());
		if(vec.x!=-1&&vec.y!=-1)
		{
			Ivector2 vec2 = m_container->GetItemPos(itm);
			if(vec2.x!=-1&&vec2.y!=-1)
			{
				u8 index = horizontal ? u8(vec2.x) : u8(vec2.y);
				u8 index_vec = horizontal ? u8(vec.x) : u8(vec.y);
				shared_str tmp = ACTOR_DEFS::g_quick_use_slots[index_vec];
				xr_strcpy(ACTOR_DEFS::g_quick_use_slots[index_vec], ACTOR_DEFS::g_quick_use_slots[index]);
				xr_strcpy(ACTOR_DEFS::g_quick_use_slots[index], tmp.c_str());
				ReloadReferences(actor);
				return;
			}
		}
	}
	DestroyDragItem();
}
