#include "stdafx.h"
#include "UIInventoryWnd.h"
#include "UISleepWnd.h"
#include "../level.h"
#include "../actor.h"
#include "../ActorCondition.h"
#include "../hudmanager.h"
#include "../inventory.h"
#include "UIInventoryUtilities.h"

#include "../Artefact.h"
#include "../ArtefactContainer.h"
#include "../CustomBackpack.h"

#include "UICellItem.h"
#include "UICellItemFactory.h"
#include "UIDragDropListEx.h"
#include "UI3tButton.h"

CUICellItem* CUIInventoryWnd::CurrentItem()
{
	return m_pCurrentCellItem;
}

PIItem CUIInventoryWnd::CurrentIItem()
{
	return	(m_pCurrentCellItem)?(PIItem)m_pCurrentCellItem->m_pData : NULL;
}

void CUIInventoryWnd::SetCurrentItem(CUICellItem* itm)
{
	if(m_pCurrentCellItem == itm) return;
	m_pCurrentCellItem				= itm;
	UIItemInfo.InitItem			(CurrentIItem());
}

void CUIInventoryWnd::SendMessage(CUIWindow *pWnd, s16 msg, void *pData)
{
	CUIWndCallback::OnEvent(pWnd, msg, pData);
}

void CUIInventoryWnd::OnExitBtnClicked(CUIWindow* w, void* d)
{
	if (GameConstants::GetBackpackAnimsEnabled() && smart_cast<CCustomBackpack*>(Actor()->inventory().ItemFromSlot(BACKPACK_SLOT)) && Actor()->inventory().GetActiveSlot() == BACKPACK_SLOT && Actor()->inventory().ActiveItem())
		Actor()->inventory().Activate(NO_ACTIVE_SLOT);

	GetHolder()->StartStopMenu(this, true);
}


void CUIInventoryWnd::InitInventory_delayed()
{
	m_b_need_reinit = true;
}

void CUIInventoryWnd::InitInventory() 
{
	CInventoryOwner *pInvOwner	= smart_cast<CInventoryOwner*>(Level().CurrentEntity());
	if(!pInvOwner)				return;

	m_pInv						= &pInvOwner->inventory();

	UIPropertiesBox->Hide		();
	ClearAllLists				();
	m_pMouseCapturer			= NULL;
	SetCurrentItem				(NULL);

	//Slots
	PIItem _itm							= m_pInv->m_slots[PISTOL_SLOT].m_pIItem;
	if(_itm)
	{
		CUICellItem* itm				= create_cell_item(_itm);
		m_pUIPistolList->SetItem		(itm);
	}

	_itm								= m_pInv->m_slots[RIFLE_SLOT].m_pIItem;
	if(_itm)
	{
		CUICellItem* itm				= create_cell_item(_itm);
		m_pUIAutomaticList->SetItem		(itm);
	}

	if (m_pUIKnifeList)
	{
		_itm = m_pInv->m_slots[KNIFE_SLOT].m_pIItem;
		if (_itm)
		{
			CUICellItem* itm = create_cell_item(_itm);
			m_pUIKnifeList->SetItem(itm);
		}
	}
	
	if (m_pUITorchList)
	{
		_itm = m_pInv->m_slots[TORCH_SLOT].m_pIItem;
		if (_itm)
		{
			CUICellItem* itm = create_cell_item(_itm);
			m_pUITorchList->SetItem(itm);
		}
	}
	
	if (m_pUIBinocularList)
	{
		_itm = m_pInv->m_slots[APPARATUS_SLOT].m_pIItem;
		if (_itm)
		{
			CUICellItem* itm = create_cell_item(_itm);
			m_pUIBinocularList->SetItem(itm);
		}
	}

	if (m_pUIPdaList)
	{
		_itm = m_pInv->m_slots[PDA_SLOT].m_pIItem;
		if (_itm)
		{
			CUICellItem* itm = create_cell_item(_itm);
			m_pUIPdaList->SetItem(itm);
		}
	}

	if (m_pUIDosimeterList)
	{
		_itm = m_pInv->m_slots[DETECTOR_SLOT].m_pIItem;
		if (_itm)
		{
			CUICellItem* itm = create_cell_item(_itm);
			m_pUIDosimeterList->SetItem(itm);
		}
	}
	
	if (m_pUIBackpackList)
	{
		_itm = m_pInv->m_slots[BACKPACK_SLOT].m_pIItem;
		if (_itm)
		{
			CUICellItem* itm = create_cell_item(_itm);
			m_pUIBackpackList->SetItem(itm);
		}
	}

	if (m_pUIPantsList)
	{
		_itm = m_pInv->m_slots[PANTS_SLOT].m_pIItem;
		if (_itm)
		{
			CUICellItem* itm = create_cell_item(_itm);
			m_pUIPantsList->SetItem(itm);
		}
	}

	PIItem _outfit						= m_pInv->m_slots[OUTFIT_SLOT].m_pIItem;
	CUICellItem* outfit					= (_outfit)?create_cell_item(_outfit):NULL;
	m_pUIOutfitList->SetItem			(outfit);

	TIItemContainer::iterator it, it_e;
	for(it=m_pInv->m_belt.begin(),it_e=m_pInv->m_belt.end(); it!=it_e; ++it) 
	{
		CUICellItem* itm			= create_cell_item(*it);
		m_pUIBeltList->SetItem		(itm);
	}


	
	ruck_list		= m_pInv->m_ruck;
	std::sort		(ruck_list.begin(),ruck_list.end(),InventoryUtilities::GreaterRoomInRuck);

	int i=1;
	for(it=ruck_list.begin(),it_e=ruck_list.end(); it!=it_e; ++it,++i) 
	{
		CUICellItem* itm			= create_cell_item(*it);
		m_pUIBagList->SetItem		(itm);
	}
	//fake
	_itm								= m_pInv->m_slots[GRENADE_SLOT].m_pIItem;
	if(_itm)
	{
		CUICellItem* itm				= create_cell_item(_itm);
		m_pUIBagList->SetItem			(itm);
	}

	InventoryUtilities::UpdateWeight					(UIBagWnd, true);

	if (GameConstants::GetLimitedInventory())
		InventoryUtilities::UpdateCapacityStr(*m_ActorInvFullness, *m_ActorInvCapacity);

	m_b_need_reinit					= false;
}  

void CUIInventoryWnd::DropCurrentItem(bool b_all)
{

	CActor *pActor			= smart_cast<CActor*>(Level().CurrentEntity());
	if(!pActor)				return;

	if(!b_all && CurrentIItem() && !CurrentIItem()->IsQuestItem())
	{
		SendEvent_Item_Drop		(CurrentIItem());
		SetCurrentItem			(NULL);
		InventoryUtilities::UpdateWeight			(UIBagWnd, true);

		if (GameConstants::GetLimitedInventory())
			InventoryUtilities::UpdateCapacityStr(*m_ActorInvFullness, *m_ActorInvCapacity);

		return;
	}

	if(b_all && CurrentIItem() && !CurrentIItem()->IsQuestItem())
	{
		u32 cnt = CurrentItem()->ChildsCount();

		for(u32 i=0; i<cnt; ++i){
			CUICellItem*	itm				= CurrentItem()->PopChild(NULL);
			PIItem			iitm			= (PIItem)itm->m_pData;
			SendEvent_Item_Drop				(iitm);
		}

		SendEvent_Item_Drop					(CurrentIItem());
		SetCurrentItem						(NULL);
		InventoryUtilities::UpdateWeight	(UIBagWnd, true);

		if (GameConstants::GetLimitedInventory())
			InventoryUtilities::UpdateCapacityStr(*m_ActorInvFullness, *m_ActorInvCapacity);

		return;
	}
}

//------------------------------------------

bool CUIInventoryWnd::ToSlot(CUICellItem* itm, bool force_place)
{
	CUIDragDropListEx*	old_owner			= itm->OwnerList();
	PIItem	iitem							= (PIItem)itm->m_pData;
	u32 _slot								= iitem->GetSlot();

	if(GetInventory()->CanPutInSlot(iitem)){
		CUIDragDropListEx* new_owner		= GetSlotList(_slot);
		
		if(_slot==GRENADE_SLOT && !new_owner )return true; //fake, sorry (((

		bool result							= GetInventory()->Slot(iitem);
		VERIFY								(result);

		CUICellItem* i						= old_owner->RemoveItem(itm, (old_owner==new_owner) );
		
		new_owner->SetItem					(i);
	
		SendEvent_Item2Slot					(iitem);

		SendEvent_ActivateSlot				(iitem);
		
		return								true;
	}else
	{ // in case slot is busy
		if(!force_place ||  _slot==NO_ACTIVE_SLOT || GetInventory()->m_slots[_slot].m_bPersistent) return false;

		PIItem	_iitem						= GetInventory()->m_slots[_slot].m_pIItem;
		CUIDragDropListEx* slot_list		= GetSlotList(_slot);
		VERIFY								(slot_list->ItemsCount()==1);

		CUICellItem* slot_cell				= slot_list->GetItemIdx(0);
		VERIFY								(slot_cell && ((PIItem)slot_cell->m_pData)==_iitem);

		bool result							= ToBag(slot_cell, false);
		VERIFY								(result);

		return ToSlot						(itm, false);
	}
}

bool CUIInventoryWnd::ToBag(CUICellItem* itm, bool b_use_cursor_pos)
{
	PIItem	iitem						= (PIItem)itm->m_pData;

	if(GetInventory()->CanPutInRuck(iitem))
	{
		CUIDragDropListEx*	old_owner		= itm->OwnerList();
		CUIDragDropListEx*	new_owner		= NULL;
		if(b_use_cursor_pos){
				new_owner					= CUIDragDropListEx::m_drag_item->BackList();
				VERIFY						(new_owner==m_pUIBagList);
		}else
				new_owner					= m_pUIBagList;


		bool result							= GetInventory()->Ruck(iitem);
		VERIFY								(result);
		CUICellItem* i						= old_owner->RemoveItem(itm, (old_owner==new_owner) );
		
		if(b_use_cursor_pos)
			new_owner->SetItem				(i,old_owner->GetDragItemPosition());
		else
			new_owner->SetItem				(i);

		SendEvent_Item2Ruck					(iitem);
		return true;
	}
	return false;
}

bool CUIInventoryWnd::ToBelt(CUICellItem* itm, bool b_use_cursor_pos)
{
	PIItem	iitem						= (PIItem)itm->m_pData;

	if(GetInventory()->CanPutInBelt(iitem))
	{
		CUIDragDropListEx*	old_owner		= itm->OwnerList();
		CUIDragDropListEx*	new_owner		= NULL;
		if(b_use_cursor_pos){
				new_owner					= CUIDragDropListEx::m_drag_item->BackList();
				VERIFY						(new_owner==m_pUIBeltList);
		}else
				new_owner					= m_pUIBeltList;

		bool result							= GetInventory()->Belt(iitem);
		VERIFY								(result);
		CUICellItem* i						= old_owner->RemoveItem(itm, (old_owner==new_owner) );
		
	//.	UIBeltList.RearrangeItems();
		if(b_use_cursor_pos)
			new_owner->SetItem				(i,old_owner->GetDragItemPosition());
		else
			new_owner->SetItem				(i);

		SendEvent_Item2Belt					(iitem);
		return								true;
	}
	return									false;
}

void CUIInventoryWnd::AddItemToBag(PIItem pItem)
{
	CUICellItem* itm						= create_cell_item(pItem);
	m_pUIBagList->SetItem					(itm);
}

bool CUIInventoryWnd::OnItemStartDrag(CUICellItem* itm)
{
	return false; //default behaviour
}

bool CUIInventoryWnd::OnItemSelected(CUICellItem* itm)
{
	SetCurrentItem		(itm);
	return				false;
}

bool CUIInventoryWnd::OnItemDrop(CUICellItem* itm)
{
	CUIDragDropListEx*	old_owner		= itm->OwnerList();
	CUIDragDropListEx*	new_owner		= CUIDragDropListEx::m_drag_item->BackList();
	if ( old_owner==new_owner || !old_owner || !new_owner )
	{
		CUICellItem* cell_item				= new_owner->GetCellItemUnderCursor();
		PIItem item_in_cell					= cell_item ? (PIItem)cell_item->m_pData : NULL;
		CArtefactContainer* pAfContainer	= smart_cast<CArtefactContainer*>(item_in_cell);
		CArtefact*	pArtefact				= smart_cast<CArtefact*>	(CurrentIItem());

		if (old_owner == new_owner && item_in_cell && item_in_cell->CanAttach(CurrentIItem()))
		{
			AttachAddon(item_in_cell);
			//UpdateItemsPlace();
			return true;
		}
		if (old_owner == new_owner && pArtefact)
		{
			if (pAfContainer && !pAfContainer->IsFull())
			{
				pAfContainer->PutArtefactToContainer(*pArtefact);

				pArtefact->DestroyObject();
				return true;	
			}
		}
		return false;
	}
	EDDListType t_new		= GetType(new_owner);
	EDDListType t_old		= GetType(old_owner);
	if(t_new == t_old)	return true;

	switch(t_new){
		case iwSlot:{
			if(GetSlotList(CurrentIItem()->GetSlot())==new_owner)
				ToSlot	(itm, true);
		}break;
		case iwBag:{
			ToBag	(itm, true);
		}break;
		case iwBelt:{
			ToBelt	(itm, true);
		}break;
	};

	DropItem				(CurrentIItem(), new_owner);

	return true;
}

bool CUIInventoryWnd::OnItemDbClick(CUICellItem* itm)
{
	if (TryUseItem(itm))		
		return true;

	CUIDragDropListEx*	old_owner		= itm->OwnerList();
	EDDListType t_old					= GetType(old_owner);

	switch(t_old){
		case iwSlot:{
			ToBag	(itm, false);
		}break;

		case iwBag:{
			if(!ToSlot(itm, false)){
				if( !ToBelt(itm, false) )
					ToSlot	(itm, true);
			}
		}break;

		case iwBelt:{
			ToBag	(itm, false);
		}break;
	};

	return true;
}


bool CUIInventoryWnd::OnItemRButtonClick(CUICellItem* itm)
{
	SetCurrentItem				(itm);
	ActivatePropertiesBox		();
	return						false;
}

CUIDragDropListEx* CUIInventoryWnd::GetSlotList(u32 slot_idx)
{
	if(slot_idx == NO_ACTIVE_SLOT || GetInventory()->m_slots[slot_idx].m_bPersistent)	return NULL;
	switch (slot_idx)
	{
		case PISTOL_SLOT:
			return m_pUIPistolList;
			break;

		case RIFLE_SLOT:
			return m_pUIAutomaticList;
			break;

		case OUTFIT_SLOT:
			return m_pUIOutfitList;
			break;

		case KNIFE_SLOT:
		{
			if (m_pUIKnifeList)
				return m_pUIKnifeList;

			return nullptr;
		} break;
		
		case TORCH_SLOT:
		{
			if (m_pUITorchList)
				return m_pUITorchList;

			return nullptr;
		} break;

		case APPARATUS_SLOT:
		{
			if (m_pUIBinocularList)
				return m_pUIBinocularList;

			return nullptr;
		} break;

		case PDA_SLOT:
		{
			if (m_pUIPdaList)
				return m_pUIPdaList;

			return nullptr;
		} break;
		
		case DETECTOR_SLOT:
		{
			if (m_pUIDosimeterList)
				return m_pUIDosimeterList;

			return nullptr;
		} break;
		
		case BACKPACK_SLOT:
		{
			if (m_pUIBackpackList)
				return m_pUIBackpackList;

			return nullptr;
		} break;

		case PANTS_SLOT:
		{
			if (m_pUIPantsList)
				return m_pUIPantsList;

			return nullptr;
		} break;

	};
	return NULL;
}



void CUIInventoryWnd::ClearAllLists()
{
	m_pUIBagList->ClearAll					(true);
	m_pUIBeltList->ClearAll					(true);
	m_pUIOutfitList->ClearAll				(true);
	m_pUIPistolList->ClearAll				(true);
	m_pUIAutomaticList->ClearAll			(true);

	if (m_pUIKnifeList)
		m_pUIKnifeList->ClearAll			(true);
	
	if (m_pUITorchList)
		m_pUITorchList->ClearAll			(true);
	
	if (m_pUIBinocularList)
		m_pUIBinocularList->ClearAll		(true);

	if (m_pUIPdaList)
		m_pUIPdaList->ClearAll				(true);

	if (m_pUIDosimeterList)
		m_pUIDosimeterList->ClearAll		(true);
	
	if (m_pUIBackpackList)
		m_pUIBackpackList->ClearAll			(true);	
	
	if (m_pUIPantsList)
		m_pUIPantsList->ClearAll			(true);
}