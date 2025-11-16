#pragma once

#include "../../xrCore/net_utils.h"

struct GameEvent
{
	u16			type;
	u32			time;
	ClientID	sender;
	NET_Packet	P;
};

class  GameEventQueue
{
	xrCriticalSection		cs;
	xr_deque<GameEvent*>	ready;
	xr_vector<GameEvent*>	unused;
public:
	GameEventQueue();
	~GameEventQueue();

	typedef fastdelegate::FastDelegate1<GameEvent*, bool> event_predicate;

	GameEvent*			Create	();
	GameEvent*			Create	(NET_Packet& P, u16 type, u32 time, ClientID clientID);
	GameEvent*			Retreive();
	void				Release	();

	u32					EraseEvents(event_predicate to_del);
};
