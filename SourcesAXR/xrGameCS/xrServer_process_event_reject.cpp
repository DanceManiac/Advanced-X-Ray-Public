#include "stdafx.h"
#include "xrserver.h"
#include "xrserver_objects.h"
#include "xrServer_svclient_validation.h"

bool xrServer::Process_event_reject	(NET_Packet& P, const ClientID sender, const u32 time, const u16 id_parent, const u16 id_entity, bool send_message)
{
	// Parse message
	CSE_Abstract*		e_parent	= game->get_entity_from_eid	(id_parent);
	CSE_Abstract*		e_entity	= game->get_entity_from_eid	(id_entity);

//	R_ASSERT2( e_entity, make_string( "entity not found. parent_id = [%d], entity_id = [%d], frame = [%d]", id_parent, id_entity, Device.dwFrame ).c_str() );
	VERIFY2  ( e_entity, make_string( "entity not found. parent_id = [%d], entity_id = [%d], frame = [%d]", id_parent, id_entity, Device.dwFrame ).c_str() );
//	R_ASSERT2( e_parent, make_string( "parent not found. parent_id = [%d], entity_id = [%d], frame = [%d]", id_parent, id_entity, Device.dwFrame ).c_str() );
	VERIFY2  ( e_parent, make_string( "parent not found. parent_id = [%d], entity_id = [%d], frame = [%d]", id_parent, id_entity, Device.dwFrame ).c_str() );
	if ( !e_parent ) {
		Msg                ( "! ERROR on rejecting: parent not found. parent_id = [%d], entity_id = [%d], frame = [%d]", id_parent, id_entity, Device.dwFrame );
		return false;
	}
	if ( !e_entity ) {
		Msg                ( "! ERROR on rejecting: entity not found. parent_id = [%d], entity_id = [%d], frame = [%d]", id_parent, id_entity, Device.dwFrame );
		return false;
	}
	game->OnDetach(id_parent,id_entity);

#ifdef MP_LOGGING
	Msg ( "--- SV: Process reject: parent[%d][%s], item[%d][%s]", id_parent, e_parent->name_replace(), id_entity, e_entity->name());
#endif // MP_LOGGING

	if (0xffff == e_entity->ID_Parent) 
	{
#ifndef MASTER_GOLD
		Msg	("! ERROR: can't detach independent object. entity[%s][%d], parent[%s][%d], section[%s]",
			e_entity->name_replace(), id_entity, e_parent->name_replace(), id_parent, e_entity->s_name.c_str() );
#endif // #ifndef MASTER_GOLD
		return			(false);
	}

	// Rebuild parentness
	if (e_entity->ID_Parent != id_parent)
	{
		//it can't be !!!

		Msg("! ERROR: e_entity->ID_Parent = [%d]  parent = [%d][%s]  entity_id = [%d]  frame = [%d]",
			e_entity->ID_Parent, id_parent, e_parent->name_replace(), id_entity, Device.dwFrame);
	}



	auto& children		= e_parent->children;
	const auto child	= std::find	(children.begin(),children.end(),id_entity);
	if (child == children.end())
	{
		Msg("! ERROR: SV: can't find children [%d] of parent [%d]", id_entity, e_parent);
		return false;
	}

	e_entity->ID_Parent		= 0xffff; 
	children.erase			(child);

	// Signal to everyone (including sender)
	if (send_message)
	{
		DWORD MODE		= net_flags(TRUE,TRUE, FALSE, TRUE);
		SendBroadcast	(BroadcastCID,P,MODE);
	}
	
	return				(true);
}
