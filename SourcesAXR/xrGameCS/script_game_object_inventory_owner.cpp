////////////////////////////////////////////////////////////////////////////
// script_game_object_inventory_owner.сpp :	функции для inventory owner
//////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "script_game_object.h"
#include "script_game_object_impl.h"
#include "InventoryOwner.h"
#include "Pda.h"
#include "xrMessages.h"
#include "character_info.h"
#include "gametask.h"
#include "actor.h"
#include "level.h"
#include "date_time.h"
#include "uigamesp.h"
#include "hudmanager.h"
#include "restricted_object.h"
#include "script_engine.h"
#include "attachable_item.h"
#include "script_entity.h"
#include "string_table.h"
#include "alife_registry_wrappers.h"
#include "relation_registry.h"
#include "custommonster.h"
#include "actorcondition.h"
#include "level_graph.h"
#include "huditem.h"
#include "ui/UItalkWnd.h"
#include "ui/UITradeWnd.h"
#include "inventory.h"
#include "infoportion.h"
#include "AI/Monsters/BaseMonster/base_monster.h"
#include "weaponmagazined.h"
#include "ai/stalker/ai_stalker.h"
#include "agent_manager.h"
#include "agent_member_manager.h"
#include "stalker_animation_manager.h"
#include "CameraFirstEye.h"
#include "stalker_movement_manager_smart_cover.h"
#include "script_callback_ex.h"
#include "memory_manager.h"
#include "enemy_manager.h"
#include "ai/stalker/ai_stalker_impl.h"
#include "smart_cover_object.h"
#include "smart_cover.h"
#include "CustomOutfit.h"
#include "Torch.h"

//Alundaio
#include "inventory_upgrade_manager.h"
//-Alundaio

bool CScriptGameObject::GiveInfoPortion(LPCSTR info_id)
{
	RMakeObj(CInventoryOwner,owner,false);
	owner->TransferInfo(info_id, true);
	return			true;
}

bool CScriptGameObject::DisableInfoPortion(LPCSTR info_id)
{
	RMakeObj(CInventoryOwner,owner,false);
	owner->TransferInfo(info_id, false);
	return true;
}

void _AddIconedTalkMessage(LPCSTR caption, LPCSTR text, LPCSTR texture_name, LPCSTR templ_name);

void  CScriptGameObject::AddIconedTalkMessage(LPCSTR caption, LPCSTR text, LPCSTR texture_name, LPCSTR templ_name)
{
	_AddIconedTalkMessage( caption, text, texture_name, templ_name );
}

void _AddIconedTalkMessage(LPCSTR caption, LPCSTR text, LPCSTR texture_name, LPCSTR templ_name)
{
	CUIGameSP* pGameSP = smart_cast<CUIGameSP*>(HUD().GetUI()->UIGame());
	if(!pGameSP) return;

	if(pGameSP->TalkMenu->IsShown())
	{
		pGameSP->TalkMenu->AddIconedMessage( caption, text, texture_name, templ_name ? templ_name : "iconed_answer_item" );
	}
}

void _give_news	(LPCSTR caption, LPCSTR news, LPCSTR texture_name, int delay, int show_time, int type);

void  CScriptGameObject::GiveGameNews(LPCSTR caption, LPCSTR news, LPCSTR texture_name, int delay, int show_time)
{
	GiveGameNews(caption, news,	texture_name, delay, show_time, 0);
}

void  CScriptGameObject::GiveGameNews(LPCSTR caption, LPCSTR news, LPCSTR texture_name, int delay, int show_time, int type)
{
	_give_news(caption, news, texture_name, delay, show_time, type);	
}

void _give_news	(LPCSTR caption, LPCSTR text, LPCSTR texture_name, int delay, int show_time, int type)
{
	GAME_NEWS_DATA				news_data;
	news_data.m_type			= (GAME_NEWS_DATA::eNewsType)type;
	news_data.news_caption		= caption;
	news_data.news_text			= text;
	if(show_time!=0)
		news_data.show_time		= show_time;// override default

	VERIFY(xr_strlen(texture_name)>0);

	news_data.texture_name			= texture_name;

	if(delay==0)
		Actor()->AddGameNews(news_data);
	else
		Actor()->AddGameNews_deffered(news_data,delay);
}

bool  CScriptGameObject::HasInfo				(LPCSTR info_id)
{
	RMakeObj(CInventoryOwner,owner,false);
	return owner->HasInfo(info_id);

}
bool  CScriptGameObject::DontHasInfo			(LPCSTR info_id)
{
	RMakeObj(CInventoryOwner,owner,true);
	return !owner->HasInfo(info_id);
}

xrTime CScriptGameObject::GetInfoTime			(LPCSTR info_id)
{
	RMakeObj(CInventoryOwner,owner,xrTime(0));
	INFO_DATA info_data;
	if(owner->GetInfo(info_id, info_data))
		return xrTime(info_data.receive_time);
	return xrTime(0);
}

bool CScriptGameObject::IsTalking()
{
	RMakeObj(CInventoryOwner,owner,false);
	return			owner->IsTalking();
}

void CScriptGameObject::StopTalk()
{
	MakeObj(CInventoryOwner,owner);
	owner->StopTalk();
}

void CScriptGameObject::EnableTalk()
{
	MakeObj(CInventoryOwner,owner);
	owner->EnableTalk();
}
void CScriptGameObject::DisableTalk()
{
	MakeObj(CInventoryOwner,owner);
	owner->DisableTalk();
}

bool CScriptGameObject::IsTalkEnabled()
{
	RMakeObj(CInventoryOwner,owner,false);
	return owner->IsTalkEnabled();
}

void CScriptGameObject::EnableTrade			()
{
	MakeObj(CInventoryOwner,owner);
	owner->EnableTrade();
}
void CScriptGameObject::DisableTrade		()
{
	MakeObj(CInventoryOwner,owner);
	owner->DisableTrade();
}

bool CScriptGameObject::IsTradeEnabled		()
{
	RMakeObj(CInventoryOwner,owner,false);
	return owner->IsTradeEnabled();
}

void CScriptGameObject::EnableInvUpgrade		()
{
	MakeObj(CInventoryOwner,owner);
	owner->EnableInvUpgrade();
}
void CScriptGameObject::DisableInvUpgrade		()
{
	MakeObj(CInventoryOwner,owner);
	owner->DisableInvUpgrade();
}

bool CScriptGameObject::IsInvUpgradeEnabled		()
{
	RMakeObj(CInventoryOwner,owner,false);
	return owner->IsInvUpgradeEnabled();
}

void CScriptGameObject::ForEachInventoryItems(const luabind::functor<void> &functor)
{
	MakeObj(CInventoryOwner,owner);
	CInventory* pInv = &owner->inventory();
	TIItemContainer item_list;
	pInv->AddAvailableItems(item_list, true);

	TIItemContainer::iterator it;
	for(it =  item_list.begin(); item_list.end() != it; ++it) 
	{
		CGameObject* inv_go = smart_cast<CGameObject*>(*it);
		if( inv_go ){
			functor(inv_go->lua_game_object(),this);
		}
	}
}

//1
void CScriptGameObject::IterateInventory	(luabind::functor<void> functor, luabind::object object)
{
	MakeObj2(CInventoryOwner,owner,&this->object());
	TIItemContainer::iterator	I = owner->inventory().m_all.begin();
	TIItemContainer::iterator	E = owner->inventory().m_all.end();
	for ( ; I != E; ++I)
		functor				(object,(*I)->object().lua_game_object());
}

#include "InventoryBox.h"
void CScriptGameObject::IterateInventoryBox	(luabind::functor<void> functor, luabind::object object)
{
	MakeObj2(CInventoryBox,inv_box,&this->object());
	xr_vector<u16>::const_iterator	I = inv_box->m_items.begin();
	xr_vector<u16>::const_iterator	E = inv_box->m_items.end();
	for ( ; I != E; ++I)
	{
		CGameObject* GO		= smart_cast<CGameObject*>(Level().Objects.net_Find(*I));
		if(GO)
			functor				(object,GO->lua_game_object());
	}
}

void CScriptGameObject::IterateBelt(luabind::functor<bool> functor, luabind::object object)
{
	MakeObj2(CInventoryOwner,owner,&this->object());
	TIItemContainer::iterator I = owner->inventory().m_belt.begin();
	TIItemContainer::iterator E = owner->inventory().m_belt.end();
	for (; I != E; ++I)
		if (functor(object, (*I)->object().lua_game_object()) == true)
			return;
}

void CScriptGameObject::IterateRuck(luabind::functor<bool> functor, luabind::object object)
{
	MakeObj2(CInventoryOwner,owner,&this->object());
	TIItemContainer::iterator I = owner->inventory().m_ruck.begin();
	TIItemContainer::iterator E = owner->inventory().m_ruck.end();
	for (; I != E; ++I)
		if (functor(object, (*I)->object().lua_game_object()) == true)
			return;
}

void CScriptGameObject::MoveItemToRuck(CScriptGameObject* pItem)
{
	MakeObj(CInventoryOwner,owner);
	MakeObj2(CInventoryItem,item,&pItem->object());
	if (!owner->inventory().CanPutInRuck(item))
		return;
	
	NET_Packet P;
	CGameObject::u_EventGen(P, GEG_PLAYER_ITEM2RUCK, owner->object_id());
	P.w_u16(item->object().ID());
	CGameObject::u_EventSend(P);
}

void CScriptGameObject::MoveItemToSlot(CScriptGameObject* pItem, u16 slot_id)
{
	MakeObj(CInventoryOwner,owner);
	MakeObj2(CInventoryItem,item,&pItem->object());
	
	// Have a crash if you want
	/*
	if (!owner->inventory().CanPutInSlot(item, slot_id))
	{
		ai().script_engine().script_log(ScriptStorage::eLuaMessageTypeError,
			"CScriptGameObject::MoveItemToSlot can't put in slot !!!");
		return;
	}
	*/
	
	CInventoryItem* item_in_slot = owner->inventory().ItemFromSlot(slot_id);

	NET_Packet P;
	if (item_in_slot)
	{
		CGameObject::u_EventGen(P, GEG_PLAYER_ITEM2RUCK, owner->object_id());
		P.w_u16(item_in_slot->object().ID());
		CGameObject::u_EventSend(P);
	}
	
	CGameObject::u_EventGen(P, GEG_PLAYER_ITEM2SLOT, owner->object_id());
	P.w_u16(item->object().ID());
	P.w_u16(slot_id);
	CGameObject::u_EventSend(P);
}

void CScriptGameObject::MoveItemToBelt(CScriptGameObject* pItem)
{
	MakeObj(CInventoryOwner,owner);
	MakeObj2(CInventoryItem,item,&pItem->object());
	
	if (!owner->inventory().CanPutInBelt(item))
		return;
	
	NET_Packet P;
	CGameObject::u_EventGen(P, GEG_PLAYER_ITEM2BELT, owner->object_id());
	P.w_u16(item->object().ID());
	CGameObject::u_EventSend(P);
}

void CScriptGameObject::MarkItemDropped		(CScriptGameObject *item)
{
	MakeObj(CInventoryOwner,owner);
	MakeObj2(CInventoryItem,inventory_item,&item->object());
	inventory_item->SetDropManual	(TRUE);
}

bool CScriptGameObject::MarkedDropped		(CScriptGameObject *item)
{
	RMakeObj(CInventoryOwner,owner,false);
	RMakeObj2(CInventoryItem,inventory_item,false,&item->object());
	return					(!!inventory_item->GetDropManual());
}

void CScriptGameObject::UnloadMagazine		()
{
	MakeObj(CWeaponMagazined,weapon_magazined);
	CAI_Stalker				*stalker = smart_cast<CAI_Stalker*>(weapon_magazined->H_Parent());
	if (stalker && stalker->hammer_is_clutched())
		return;

	weapon_magazined->UnloadMagazine	(false);
}

void CScriptGameObject::DropItem			(CScriptGameObject* pItem)
{
	MakeObj(CInventoryOwner,owner);
	MakeObj2(CInventoryItem,item,&pItem->object());

	NET_Packet						P;
	CGameObject::u_EventGen			(P,GE_OWNERSHIP_REJECT, object().ID());
	P.w_u16							(pItem->object().ID());
	CGameObject::u_EventSend		(P);
}

void CScriptGameObject::DropItemAndTeleport	(CScriptGameObject* pItem, Fvector position)
{
	DropItem						(pItem);

	NET_Packet						PP;
	CGameObject::u_EventGen			(PP,GE_CHANGE_POS, pItem->object().ID());
	PP.w_vec3						(position);
	CGameObject::u_EventSend		(PP);
}

void CScriptGameObject::MakeItemActive(CScriptGameObject* pItem)
{
	MakeObj(CInventoryOwner,owner);
	MakeObj2(CInventoryItem,item,&pItem->object());
	u32 slot						= item->GetSlot();
	
	CInventoryItem* item_in_slot	= owner->inventory().ItemFromSlot(slot);

	NET_Packet						P;
	if(item_in_slot)
	{
		CGameObject::u_EventGen		(P, GEG_PLAYER_ITEM2RUCK, owner->object_id());
		P.w_u16						(item_in_slot->object().ID());
		CGameObject::u_EventSend	(P);
	}
	CGameObject::u_EventGen			(P, GEG_PLAYER_ITEM2SLOT, owner->object_id());
	P.w_u16							(item->object().ID());
	CGameObject::u_EventSend		(P);

	CGameObject::u_EventGen			(P, GEG_PLAYER_ACTIVATE_SLOT, owner->object_id());
	P.w_u32							(slot);
	CGameObject::u_EventSend		(P);
}

void CScriptGameObject::TakeItem(CScriptGameObject* pItem)
{
	if (!pItem)
	{
		Msg("! CScriptGameObject::TakeItem | cannot take NULL item");
		return;
	}

	MakeObj2(CInventoryItem,pIItem,&pItem->object());

	// In case of an existing parent, transfer item as usual
	// probably doesn't work on NPC for now and needs fixing if needed in the future
	if (pIItem->object().H_Parent())
	{
		const CInventoryOwner* owner = smart_cast<CInventoryOwner*>(pIItem->object().H_Parent());
		const CInventoryBox* inv_box = smart_cast<CInventoryBox*>(pIItem->object().H_Parent());
		const CGameObject* parentGO = smart_cast<CGameObject*>(pIItem->object().H_Parent());
		if ((owner || inv_box) && parentGO)
		{
			NET_Packet P;
			CGameObject::u_EventGen(P, GE_TRADE_SELL, parentGO->ID());
			P.w_u16(pIItem->object().ID());
			CGameObject::u_EventSend(P);

			CGameObject::u_EventGen(P, GE_TRADE_BUY, object().ID());
			P.w_u16(pIItem->object().ID());
			CGameObject::u_EventSend(P);
		}
		else
			Msg("! CScriptGameObject::TakeItem | Unknown parent type found?");

		return; // added return here just in case parent isn't identified as inventory owner or a box
	}

	// In case of no parent, do a take action
	Game().SendPickUpEvent(object().ID(), pIItem->object().ID());
}

//передаче вещи из своего инвентаря в инвентарь партнера
void CScriptGameObject::TransferItem(CScriptGameObject* pItem, CScriptGameObject* pForWho)
{
	if (!pItem || !pForWho) {
		Msg("! cannot transfer NULL item");
		return;
	}

	MakeObj2(CInventoryItem,pIItem,&pItem->object());
	// выбросить у себя 
	NET_Packet						P;
	CGameObject::u_EventGen			(P,GE_TRADE_SELL, object().ID());
	P.w_u16							(pIItem->object().ID());
	CGameObject::u_EventSend		(P);

	// отдать партнеру
	CGameObject::u_EventGen			(P,GE_TRADE_BUY, pForWho->object().ID());
	P.w_u16							(pIItem->object().ID());
	CGameObject::u_EventSend		(P);
}

u32 CScriptGameObject::Money	()
{
	RMakeObj(CInventoryOwner,pOurOwner,0);
	return pOurOwner->get_money();
}

void CScriptGameObject::TransferMoney(int money, CScriptGameObject* pForWho)
{
	if (!pForWho) {
		Msg("! cannot transfer money for NULL object");
		return;
	}
	MakeObj(CInventoryOwner,pOurOwner);

	if (pOurOwner->get_money()-money<0) {
		Msg("! Character does not have enought money");
		return;
	}

	pOurOwner->set_money		(pOurOwner->get_money() - money, true );
	MakeObj2(CInventoryOwner,pOtherOwner,&pForWho->object());
	pOtherOwner->set_money		(pOtherOwner->get_money() + money, true );
}

void CScriptGameObject::GiveMoney(int money)
{
	MakeObj(CInventoryOwner,pOurOwner);
	pOurOwner->set_money		(pOurOwner->get_money() + money, true );
}
//////////////////////////////////////////////////////////////////////////

int	CScriptGameObject::GetGoodwill(CScriptGameObject* pToWho)
{
	RMakeObj(CInventoryOwner,owner,0);
	return RELATION_REGISTRY().GetGoodwill(owner->object_id(), pToWho->object().ID());
}

void CScriptGameObject::SetGoodwill(int goodwill, CScriptGameObject* pWhoToSet)
{
	MakeObj(CInventoryOwner,owner);
	return RELATION_REGISTRY().SetGoodwill(owner->object_id(), pWhoToSet->object().ID(), goodwill);
}

void CScriptGameObject::ChangeGoodwill(int delta_goodwill, CScriptGameObject* pWhoToSet)
{
	MakeObj(CInventoryOwner,owner);
	RELATION_REGISTRY().ChangeGoodwill(owner->object_id(), pWhoToSet->object().ID(), delta_goodwill);
}


//////////////////////////////////////////////////////////////////////////

void CScriptGameObject::SetRelation(ALife::ERelationType relation, CScriptGameObject* pWhoToSet)
{
	MakeObj(CInventoryOwner,owner);
	MakeObj2(CInventoryOwner,pOthersInventoryOwner,&pWhoToSet->object());
	RELATION_REGISTRY().SetRelationType(owner, pOthersInventoryOwner, relation);
}

float CScriptGameObject::GetSympathy()
{
	RMakeObj(CInventoryOwner,owner,0.0f);
	return owner->Sympathy();
}

void CScriptGameObject::SetSympathy( float sympathy )
{
	MakeObj(CInventoryOwner,owner);
	owner->CharacterInfo().SetSympathy( sympathy );
}

int CScriptGameObject::GetCommunityGoodwill_obj( LPCSTR community )
{
	RMakeObj(CInventoryOwner,owner,0);
	CHARACTER_COMMUNITY c;
	c.set( community );

	return RELATION_REGISTRY().GetCommunityGoodwill( c.index(), owner->object_id() );
}

void CScriptGameObject::SetCommunityGoodwill_obj( LPCSTR community, int goodwill )
{
	MakeObj(CInventoryOwner,owner);
	CHARACTER_COMMUNITY c;
	c.set( community );

	RELATION_REGISTRY().SetCommunityGoodwill( c.index(), owner->object_id(), goodwill );
}

//////////////////////////////////////////////////////////////////////////

int	CScriptGameObject::GetAttitude			(CScriptGameObject* pToWho)
{
	RMakeObj(CInventoryOwner,owner,0);
	RMakeObj2(CInventoryOwner,pOthersInventoryOwner,0,&pToWho->object());
	return RELATION_REGISTRY().GetAttitude(owner, pOthersInventoryOwner);
}


//////////////////////////////////////////////////////////////////////////

LPCSTR CScriptGameObject::ProfileName			()
{
	RMakeObj(CInventoryOwner,owner,NULL);
	shared_str profile_id =  owner->CharacterInfo().Profile();
	if(!profile_id || !profile_id.size() )
		return NULL;

	return *profile_id;
}


LPCSTR CScriptGameObject::CharacterName			()
{
	RMakeObj(CInventoryOwner,owner,NULL);
	return owner->Name();
}

LPCSTR CScriptGameObject::CharacterIcon()
{
	RMakeObj(CInventoryOwner,owner,NULL);
	return owner->IconName();
}

int CScriptGameObject::CharacterRank			()
{
	// rank support for monster
	CBaseMonster *monster = smart_cast<CBaseMonster*>(&object());
	if (!monster) {
		RMakeObj(CInventoryOwner,owner,0);
		return owner->Rank();
	} 	
	return monster->Rank();
}
void CScriptGameObject::SetCharacterRank			(int char_rank)
{
	MakeObj(CInventoryOwner,owner);
	return owner->SetRank(char_rank);
}

void CScriptGameObject::ChangeCharacterRank			(int char_rank)
{
	MakeObj(CInventoryOwner,owner);
	return owner->ChangeRank(char_rank);
}

int CScriptGameObject::CharacterReputation			()
{
	RMakeObj(CInventoryOwner,owner,0);
	return owner->Reputation();
}

void CScriptGameObject::ChangeCharacterReputation		(int char_rep)
{
	MakeObj(CInventoryOwner,owner);
	owner->ChangeReputation(char_rep);
}

LPCSTR CScriptGameObject::CharacterCommunity	()
{
	RMakeObj(CInventoryOwner,owner,NULL);
	return *owner->CharacterInfo().Community().id();
}

void CScriptGameObject::SetCharacterCommunity	(LPCSTR comm, int squad, int group)
{
	MakeObj(CInventoryOwner,owner);
	MakeObj(CEntity,entity);
	CHARACTER_COMMUNITY	community;
	community.set(comm);
	owner->SetCommunity(community.index());
	entity->ChangeTeam(community.team(), squad, group);
}

void CScriptGameObject::SetCharacterName(LPCSTR name)
{
	MakeObj(CInventoryOwner,pOurOwner);
	pOurOwner->SetName(name);
}

LPCSTR CScriptGameObject::sound_voice_prefix () const
{
	RMakeObj(CInventoryOwner,owner,NULL);
	return owner->SpecificCharacter().sound_voice_prefix();
}

#include "GameTaskManager.h"
ETaskState CScriptGameObject::GetGameTaskState	(LPCSTR task_id)
{
	CGameTask* t						= Level().GameTaskManager().HasGameTask(shared_str(task_id), true);
	
	if(NULL==t) 
		return eTaskStateDummy;

	return t->GetTaskState();

}

void CScriptGameObject::SetGameTaskState	(ETaskState state, LPCSTR task_id)
{
	Level().GameTaskManager().SetTaskState(shared_str(task_id), state);
}

//////////////////////////////////////////////////////////////////////////

void  CScriptGameObject::SwitchToTrade		()
{
	MakeObj(CActor,pActor);
	//только если находимся в режиме single
	CUIGameSP* pGameSP = smart_cast<CUIGameSP*>(HUD().GetUI()->UIGame());
	if(!pGameSP) return;

	if(pGameSP->TalkMenu->IsShown())
	{
		pGameSP->TalkMenu->SwitchToTrade();
	}
}

void  CScriptGameObject::SwitchToUpgrade		()
{
	MakeObj(CActor,pActor);

	//только если находимся в режиме single
	CUIGameSP* pGameSP = smart_cast<CUIGameSP*>(HUD().GetUI()->UIGame());
	if(!pGameSP) return;

	if(pGameSP->TalkMenu->IsShown())
	{
		pGameSP->TalkMenu->SwitchToUpgrade();
	}
}

void  CScriptGameObject::SwitchToTalk		()
{
	R_ASSERT("switch_to_talk called ;)");
}

void CScriptGameObject::AllowBreakTalkDialog(bool b)
{
	CUIGameSP* pGameSP = smart_cast<CUIGameSP*>(HUD().GetUI()->UIGame());
	if(!pGameSP) return;
	pGameSP->TalkMenu->b_disable_break = !b;
}

void  CScriptGameObject::RunTalkDialog(CScriptGameObject* pToWho, bool disable_break)
{
	MakeObj(CActor,pActor);
	MakeObj2(CInventoryOwner,pPartner,&pToWho->object());
	pActor->RunTalkDialog(pPartner, disable_break);
}

void CScriptGameObject::ActorLookAtPoint	(Fvector point)
{
	CCameraBase* c		= Actor()->cam_FirstEye();
	CCameraFirstEye* cf = smart_cast<CCameraFirstEye*>(c);
	cf->LookAtPoint		(point);
}

//////////////////////////////////////////////////////////////////////////

void construct_restriction_vector(shared_str restrictions, xr_vector<ALife::_OBJECT_ID> &result)
{
	result.clear();
	string64	temp;
	u32			n = _GetItemCount(*restrictions);
	for (u32 i=0; i<n; ++i) {
		CObject	*object = Level().Objects.FindObjectByName(_GetItem(*restrictions,i,temp));
		if (!object)
			continue;
		result.push_back(object->ID());
	}
}

void CScriptGameObject::add_restrictions		(LPCSTR out, LPCSTR in)
{
	MakeObj(CCustomMonster,monster);
//	xr_vector<ALife::_OBJECT_ID>			temp0;
//	xr_vector<ALife::_OBJECT_ID>			temp1;

//	construct_restriction_vector			(out,temp0);
//	construct_restriction_vector			(in,temp1);

//	if (!xr_strcmp(monster->cName(),"mil_freedom_stalker0004")) {
//		int i = 0;
//		if (!xr_strcmp(in,"mil_freedom_wall_restrictor")) {
//			int j = 0;
//		}
//	}
	
	monster->movement().restrictions().add_restrictions(out,in);
}

void CScriptGameObject::remove_restrictions		(LPCSTR out, LPCSTR in)
{
	MakeObj(CCustomMonster,monster);
//	xr_vector<ALife::_OBJECT_ID>			temp0;
//	xr_vector<ALife::_OBJECT_ID>			temp1;

//	construct_restriction_vector			(out,temp0);
//	construct_restriction_vector			(in,temp1);

	monster->movement().restrictions().remove_restrictions(out,in);
}

void CScriptGameObject::remove_all_restrictions	()
{
	MakeObj(CCustomMonster,monster);
	monster->movement().restrictions().remove_all_restrictions	();
}

LPCSTR CScriptGameObject::in_restrictions	()
{
	RMakeObj(CCustomMonster,monster,"");
	return									(*monster->movement().restrictions().in_restrictions());
}

LPCSTR CScriptGameObject::out_restrictions	()
{
	RMakeObj(CCustomMonster,monster,"");
	return									(*monster->movement().restrictions().out_restrictions());
}

LPCSTR CScriptGameObject::base_in_restrictions	()
{
	RMakeObj(CCustomMonster,monster,"");
	return									(*monster->movement().restrictions().base_in_restrictions());
}

LPCSTR CScriptGameObject::base_out_restrictions	()
{
	RMakeObj(CCustomMonster,monster,"");
	return									(*monster->movement().restrictions().base_out_restrictions());
}

bool CScriptGameObject::accessible_position	(const Fvector &position)
{
	RMakeObj(CCustomMonster,monster,false);
	return									(monster->movement().restrictions().accessible(position));
}

bool CScriptGameObject::accessible_vertex_id(u32 level_vertex_id)
{
	RMakeObj(CCustomMonster,monster,false);
	if(!ai().level_graph().valid_vertex_id(level_vertex_id))
		return false;
	THROW2									(ai().level_graph().valid_vertex_id(level_vertex_id),"Cannot check if level vertex id is accessible, because it is invalid");
	return									(monster->movement().restrictions().accessible(level_vertex_id));
}

u32	 CScriptGameObject::accessible_nearest	(const Fvector &position, Fvector &result)
{
	RMakeObj(CCustomMonster,monster,u32(-1));
	if (monster->movement().restrictions().accessible(position)) {
		Msg("! CRestrictedObject: you use accessible_nearest when position is already accessible");
		return								(u32(-1));
	}
	return									(monster->movement().restrictions().accessible_nearest(position,result));
}

void CScriptGameObject::enable_attachable_item	(bool value)
{
	MakeObj(CAttachableItem,attachable_item);
	attachable_item->enable					(value);
}

bool CScriptGameObject::attachable_item_enabled	() const
{
	RMakeObj(CAttachableItem,attachable_item,false);
	return									(attachable_item->enabled());
}

void CScriptGameObject::night_vision_allowed(bool value)
{
	MakeObj(CActor,pActor);
	pActor->SetNightVisionAllowed(value);
}

void CScriptGameObject::enable_night_vision	(bool value)
{
	MakeObj(CActor,pActor);
	pActor->SwitchNightVision(value);
}

bool CScriptGameObject::night_vision_enabled	() const
{
	RMakeObj(CActor,pActor,false);
	return									(pActor->GetNightVisionStatus());
}

void CScriptGameObject::enable_torch	(bool value)
{
	MakeObj(CTorch,torch);
	torch->Switch							(value);
}

bool CScriptGameObject::torch_enabled			() const
{
	RMakeObj(CTorch,torch,false);
	return									(torch->torch_active());
}

void  CScriptGameObject::RestoreWeapon(int mode)
{
	if(mode == 1)
		Actor()->SetWeaponHideState(INV_STATE_HIDE_WEAPON, false);
	else
		Actor()->SetWeaponHideState(INV_STATE_BLOCK_ALL, false);
}

void  CScriptGameObject::HideWeapon(int mode)
{
	if(mode == 1)
		Actor()->SetWeaponHideState(INV_STATE_HIDE_WEAPON, true);
	else
		Actor()->SetWeaponHideState(INV_STATE_BLOCK_ALL, true);
}

int CScriptGameObject::Weapon_GrenadeLauncher_Status()
{
	RMakeObj(CWeapon,weapon,false);
	return (int)weapon->get_GrenadeLauncherStatus();
}

int CScriptGameObject::Weapon_Scope_Status()
{
	RMakeObj(CWeapon,weapon,false);
	return (int)weapon->get_ScopeStatus();
}

int CScriptGameObject::Weapon_Silencer_Status()
{
	RMakeObj(CWeapon,weapon,false);
	return (int)weapon->get_SilencerStatus();
}

bool CScriptGameObject::Weapon_IsGrenadeLauncherAttached()
{
	RMakeObj(CWeapon,weapon,false);
	return weapon->IsGrenadeLauncherAttached();
}

bool CScriptGameObject::Weapon_IsScopeAttached()
{
	RMakeObj(CWeapon,weapon,false);
	return weapon->IsScopeAttached();
}

bool CScriptGameObject::Weapon_IsSilencerAttached()
{
	RMakeObj(CWeapon,weapon,false);
	return weapon->IsSilencerAttached();
}

void  CScriptGameObject::AllowSprint(bool b)
{
	Actor()->SetCantRunState(!b);
}

int	CScriptGameObject::animation_slot			() const
{
	RMakeObj(CHudItem,hud_item,u32(-1));
	return			(hud_item->animation_slot());
}

CScriptGameObject *CScriptGameObject::item_in_slot	(u32 slot_id) const
{
	RMakeObj(CInventoryOwner,owner,nullptr);
	if (owner->inventory().m_slots.size() <= slot_id) {
		Msg("! CInventoryOwner: invalid slot id for class member item_in_slot : %d",slot_id);
		return nullptr;
	}

	CInventoryItem	*result = owner->inventory().m_slots[slot_id].m_pIItem;
	return			(result ? result->object().lua_game_object() : nullptr);
}

CScriptGameObject *CScriptGameObject::active_detector() const
{
	RMakeObj(CInventoryOwner,owner,nullptr);
	CInventoryItem	*result = owner->inventory().ItemFromSlot(DETECTOR_SLOT);
	if (!result)
		return nullptr;
	CCustomDetector *detector = smart_cast<CCustomDetector*>(result);
	VERIFY(detector);
	return			(detector->IsWorking() ? result->object().lua_game_object() : nullptr);
}

void CScriptGameObject::GiveTaskToActor(CGameTask* t, u32 dt, bool bCheckExisting, u32 t_timer)
{
	Level().GameTaskManager().GiveGameTaskToActor(t, dt, bCheckExisting, t_timer);
}

void CScriptGameObject::SetActiveTask(CGameTask* t)
{
	VERIFY(t);
	Level().GameTaskManager().SetActiveTask(t);
}

bool CScriptGameObject::IsActiveTask(CGameTask* t)
{
	VERIFY(t);
	return Level().GameTaskManager().ActiveTask(t->GetTaskType())==t;
}

CGameTask* CScriptGameObject::GetTask(LPCSTR id, bool only_inprocess)
{
	return Level().GameTaskManager().HasGameTask(id, only_inprocess);
}

u32	CScriptGameObject::active_slot()
{
	RMakeObj(CInventoryOwner,owner,0);
	return owner->inventory().GetActiveSlot();
}

void CScriptGameObject::activate_slot	(u32 slot_id)
{
	MakeObj(CInventoryOwner,owner);
	owner->inventory().Activate(slot_id);
}

void CScriptGameObject::enable_movement	(bool enable)
{
	MakeObj(CCustomMonster,monster);
	monster->movement().enable_movement	(enable);
}

bool CScriptGameObject::movement_enabled	()
{
	RMakeObj(CCustomMonster,monster,false);
	return								(monster->movement().enabled());
}

bool CScriptGameObject::can_throw_grenades	() const
{
	RMakeObj(CAI_Stalker,stalker,false);
	return								(stalker->can_throw_grenades());
}

void CScriptGameObject::can_throw_grenades	(bool can_throw_grenades)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->can_throw_grenades			(can_throw_grenades);
}

u32 CScriptGameObject::throw_time_interval			() const
{
	RMakeObj(CAI_Stalker,stalker,0);
	return								(stalker->throw_time_interval());
}

void CScriptGameObject::throw_time_interval			(u32 throw_time_interval)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->throw_time_interval		(throw_time_interval);
}

u32 CScriptGameObject::group_throw_time_interval	() const
{
	RMakeObj(CAI_Stalker,stalker,0);
	return								(stalker->agent_manager().member().throw_time_interval());
}

void CScriptGameObject::group_throw_time_interval	(u32 throw_time_interval)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->agent_manager().member().throw_time_interval	(throw_time_interval);
}

void CScriptGameObject::aim_time					(CScriptGameObject *weapon, u32 aim_time)
{
	MakeObj(CAI_Stalker,stalker);
	MakeObj2(CWeapon,weapon_,&weapon->object());
	stalker->aim_time					(*weapon_, aim_time);
}

u32 CScriptGameObject::aim_time						(CScriptGameObject *weapon)
{
	RMakeObj(CAI_Stalker,stalker,u32(-1));
	RMakeObj2(CWeapon,weapon_,u32(-1),&weapon->object());
	return								(stalker->aim_time(*weapon_));
}

void CScriptGameObject::special_danger_move			(bool value)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->animation().special_danger_move	(value);
}

bool CScriptGameObject::special_danger_move			()
{
	RMakeObj(CAI_Stalker,stalker,false);
	return								(stalker->animation().special_danger_move());
}

void CScriptGameObject::sniper_update_rate			(bool value)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->sniper_update_rate			(value);
}

bool CScriptGameObject::sniper_update_rate			() const
{
	RMakeObj(CAI_Stalker,stalker,false);
	return								(stalker->sniper_update_rate());
}

void CScriptGameObject::sniper_fire_mode			(bool value)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->sniper_fire_mode			(value);
}

bool CScriptGameObject::sniper_fire_mode			() const
{
	RMakeObj(CAI_Stalker,stalker,false);
	return								(stalker->sniper_fire_mode());
}

void CScriptGameObject::aim_bone_id					(LPCSTR bone_id)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->aim_bone_id				(bone_id);
}

LPCSTR CScriptGameObject::aim_bone_id				() const
{
	RMakeObj(CAI_Stalker,stalker,false);
	return								(stalker->aim_bone_id().c_str());
}

void CScriptGameObject::register_in_combat						()
{
	MakeObj(CAI_Stalker,stalker);
	stalker->agent_manager().member().register_in_combat(stalker);
}

void CScriptGameObject::unregister_in_combat					()
{
	MakeObj(CAI_Stalker,stalker);
	stalker->agent_manager().member().unregister_in_combat(stalker);
}

CCoverPoint const* CScriptGameObject::find_best_cover			(Fvector position_to_cover_from)
{
	RMakeObj(CAI_Stalker,stalker,NULL);
	return								(stalker->find_best_cover(position_to_cover_from));
}

bool CScriptGameObject::suitable_smart_cover					(CScriptGameObject* object)
{
	if (!object) {
		Msg("! CAI_Stalker::suitable_smart_cover null smart cover specified");
		return							(false);
	}

	RMakeObj2(CAI_Stalker,stalker,false,&this->object());

	smart_cover::object const* const	smart_object = smart_cast<smart_cover::object const*>(&object->object());
	if (!smart_object) {
		RCAST_ERR(smart_cover::object const*,smart_object,false);
		return							(false);
	}

	smart_cover::cover const& cover		= smart_object->cover();
	if (!cover.is_combat_cover())
		return							(true);

	CInventoryItem const* inventory_item= stalker->inventory().ActiveItem();
	if (inventory_item)
		return							(inventory_item->GetSlot() == 2);

	CInventoryItem const* best_weapon	= stalker->best_weapon();
	if (!best_weapon)
		return							(false);

	return								(best_weapon->GetSlot() == 2);
}

void CScriptGameObject::take_items_enabled						(bool const value)
{
	MakeObj(CAI_Stalker,stalker);
	stalker->take_items_enabled			(value);
}

bool CScriptGameObject::take_items_enabled						() const
{
	RMakeObj(CAI_Stalker,stalker,false);
	return								( stalker->take_items_enabled() );
}

void CScriptGameObject::SetPlayShHdRldSounds(bool val)
{
	MakeObj(CInventoryOwner,owner);
	owner->SetPlayShHdRldSounds(val);
}

/*added by Ray Twitty (aka Shadows) START*/
float CScriptGameObject::GetActorMaxWeight() const
{
	RMakeObj(CActor,pActor,false);
	return				(pActor->inventory().GetMaxWeight());
}

void CScriptGameObject::SetActorMaxWeight(float max_weight)
{
	MakeObj(CActor,pActor);
	pActor->inventory().SetMaxWeight(max_weight);
}

float CScriptGameObject::GetActorMaxWalkWeight() const
{
	RMakeObj(CActor,pActor,false);
	return				(pActor->conditions().m_MaxWalkWeight);
}
void CScriptGameObject::SetActorMaxWalkWeight(float max_walk_weight)
{
	MakeObj(CActor,pActor);
	pActor->conditions().m_MaxWalkWeight = max_walk_weight;
}

float CScriptGameObject::GetAdditionalMaxWeight() const
{
	RMakeObj(CCustomOutfit,outfit,0.0f);
	return				(outfit->m_additional_weight2);
}
float CScriptGameObject::GetAdditionalMaxWalkWeight() const
{
	RMakeObj(CCustomOutfit,outfit,0.0f);
	return				(outfit->m_additional_weight);
}
void CScriptGameObject::SetAdditionalMaxWeight(float add_max_weight)
{
	MakeObj(CCustomOutfit,outfit);
	outfit->m_additional_weight2 = add_max_weight;
}
void CScriptGameObject::SetAdditionalMaxWalkWeight(float add_max_walk_weight)
{
	MakeObj(CCustomOutfit,outfit);
	outfit->m_additional_weight = add_max_walk_weight;
}

#include "InventoryBox.h"

float CScriptGameObject::GetTotalWeight() const
{
	RMakeObj(CInventoryOwner,owner,0.0f);
	return				(owner->inventory().TotalWeight());
}

float CScriptGameObject::Weight() const
{
	RMakeObj(CInventoryItem,inventory_item,0.0f);
	return				(inventory_item->Weight());
}
/*added by Ray Twitty (aka Shadows) END*/

void CScriptGameObject::SetWeight(float w)
{
	MakeObj(CInventoryItem,inventory_item);
	inventory_item->SetWeight(w);
}
/*added by Ray Twitty (aka Shadows) END*/

//Alundaio: Methods for exporting the ability to detach/attach addons for magazined weapons
void CScriptGameObject::Weapon_AddonAttach(CScriptGameObject* item)
{
	MakeObj(CWeaponMagazined,weapon);
	CInventoryItem* pItm = item->object().cast_inventory_item();
	if (!pItm)
	{
		Msg("! CWeaponMagazined: trying to attach non-CInventoryItem");
		return;
	}

	if (weapon->CanAttach(pItm))
		weapon->Attach(pItm, true);
}

void CScriptGameObject::Weapon_AddonDetach(pcstr item_section, bool b_spawn_item = true)
{
	MakeObj(CWeaponMagazined,weapon);
	if (weapon->CanDetach(item_section))
		weapon->Detach(item_section, b_spawn_item);
}

void CScriptGameObject::Weapon_SetCurrentScope(u8 type)
{
	MakeObj(CWeaponMagazined,weapon);
	weapon->m_cur_scope = type;
}

u8 CScriptGameObject::Weapon_GetCurrentScope()
{
	RMakeObj(CWeaponMagazined,weapon,255);
	return weapon->m_cur_scope;
}

LPCSTR CScriptGameObject::Weapon_GetAmmoSection(u8 ammo_type)
{
	RMakeObj(CWeaponMagazined,weapon,"");
	if (weapon->m_ammoTypes.empty() || ammo_type + 1 > weapon->m_ammoTypes.size())
		return "";

	return weapon->m_ammoTypes[ammo_type].c_str();

}

bool CScriptGameObject::InstallUpgrade(pcstr upgrade)
{
	if (!pSettings->section_exist(upgrade))
		return false;
	RMakeObj(CInventoryItem,item,false);
	item->pre_install_upgrade();

	shared_str upgrade_id(upgrade);
	return ai().alife().inventory_upgrade_manager().upgrade_install(*item, upgrade_id, true);
}

bool CScriptGameObject::HasUpgrade(pcstr upgrade) const
{
	if (!pSettings->section_exist(upgrade))
		return false;
	RMakeObj(CInventoryItem,item,false);
	return item->has_upgrade(upgrade);
}

void CScriptGameObject::IterateInstalledUpgrades(const luabind::functor<void>& functor)
{
	MakeObj(CInventoryItem,Item);
	for (auto upgrade : Item->get_upgrades())
		functor(upgrade.c_str(), object().lua_game_object());
}

CScriptGameObject* CScriptGameObject::ItemOnBelt(u32 item_id) const
{
	RMakeObj(CInventoryOwner,owner,nullptr);
	TIItemContainer* belt = &owner->inventory().m_belt;
	if (belt->size() < item_id)
	{
		Msg("! item_on_belt: item id outside belt");
		return nullptr;
	}

	CInventoryItem* result = belt->at(item_id);
	return result ? result->object().lua_game_object() : nullptr;
}

bool CScriptGameObject::IsOnBelt(CScriptGameObject* obj) const
{
	RMakeObj2(CInventoryItem,inventory_item,false,&obj->object());
	RMakeObj(CInventoryOwner,owner,false);
	return owner->inventory().InBelt(inventory_item);
}

u32 CScriptGameObject::BeltSize() const
{
	RMakeObj(CInventoryOwner,owner,0);
	return owner->inventory().m_belt.size();
}

float CScriptGameObject::GetActorJumpSpeed() const
{
	RMakeObj(CActor,pActor,0.0f);
	return pActor->m_fJumpSpeed;
}

void CScriptGameObject::SetActorJumpSpeed(float jump_speed)
{
	MakeObj(CActor,pActor);
	pActor->m_fJumpSpeed = jump_speed;
	//character_physics_support()->movement()->SetJumpUpVelocity(m_fJumpSpeed);  
}

float CScriptGameObject::GetActorSprintKoef() const
{
	RMakeObj(CActor,pActor,0.0f);
	return pActor->m_fSprintFactor;
}

void CScriptGameObject::SetActorSprintKoef(float sprint_koef)
{
	MakeObj(CActor,pActor);
	pActor->m_fSprintFactor = sprint_koef;
}

float CScriptGameObject::GetActorRunCoef() const
{
	RMakeObj(CActor,pActor,0.0f);
	return pActor->m_fRunFactor;
}

void CScriptGameObject::SetActorRunCoef(float run_coef)
{
	MakeObj(CActor,pActor);
	pActor->m_fRunFactor = run_coef;
}

float CScriptGameObject::GetActorRunBackCoef() const
{
	RMakeObj(CActor,pActor,0.0f);
	return pActor->m_fRunBackFactor;
}

void CScriptGameObject::SetActorRunBackCoef(float run_back_coef)
{
	MakeObj(CActor,pActor);
	pActor->m_fRunBackFactor = run_back_coef;
}
//-Alundaio

void CScriptGameObject::ItemAllowTrade(CScriptGameObject* pItem)
{
	MakeObj2(CInventoryItem,item,&pItem->object());
	item->AllowTrade();
}

void CScriptGameObject::ItemDenyTrade(CScriptGameObject* pItem)
{
	MakeObj(CInventoryOwner,owner);
	MakeObj2(CInventoryItem,item,&pItem->object());
	item->DenyTrade();
}

//Arkada [begin]
float CScriptGameObject::GetActorClimbCoef() const
{
	RMakeObj(CActor,pActor,0.0f);
	return pActor->m_fClimbFactor;
}

void CScriptGameObject::SetActorClimbCoef(float k)
{
	MakeObj(CActor,pActor);
	pActor->m_fClimbFactor = k;
}
//Arkada [end]