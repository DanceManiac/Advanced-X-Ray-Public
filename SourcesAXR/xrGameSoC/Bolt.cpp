#include "stdafx.h"
#include "bolt.h"
#include "ParticlesObject.h"
#include "PhysicsShell.h"
#include "xr_level_controller.h"
#include "actor.h"
#include "inventory.h"
#include "AdvancedXrayGameConstants.h"

CBolt::CBolt(void) 
{
	m_weight					= .1f;
	m_slot						= BOLT_SLOT;
	m_flags.set					(Fruck, FALSE);
	m_thrower_id				=u16(-1);
}

CBolt::~CBolt(void) 
{
}

void CBolt::OnH_A_Chield() 
{
	inherited::OnH_A_Chield();
	CObject* o= H_Parent()->H_Parent();
	if(o)SetInitiator(o->ID());
	
}

void CBolt::OnEvent(NET_Packet& P, u16 type) 
{
	inherited::OnEvent(P,type);
}

bool CBolt::Activate() 
{
	Show();
	return true;
}

void CBolt::Deactivate() 
{
	Hide();
}

void CBolt::State(u32 state)
{
	switch (GetState())
	{
	case MS_HIDDEN:
	{
		if (GameConstants::GetLimitedBolts())
		{
			if (m_pPhysicsShell) m_pPhysicsShell->Deactivate();
			xr_delete(m_pPhysicsShell);
			m_dwDestroyTime = 0xffffffff;
			PutNextToSlot();
			if (Local())
			{
				//Msg("Destroying local bolt[%d][%d]", ID(), Device.dwFrame);
				DestroyObject();
			}
		}
	}
	break;
	}
	inherited::State(state);
}

void CBolt::Throw() 
{
	CMissile					*l_pBolt = smart_cast<CMissile*>(m_fake_missile);
	if(!l_pBolt)				return;
	l_pBolt->set_destroy_time	(u32(m_dwDestroyTimeMax/phTimefactor));
	inherited::Throw			();
	spawn_fake_missile			();
}

bool CBolt::Useful() const
{
	if (GameConstants::GetLimitedBolts())
		return true;
	else
		return false;
}

bool CBolt::Action(s32 cmd, u32 flags) 
{
	if(inherited::Action(cmd, flags)) return true;
/*
	switch(cmd) 
	{
	case kDROP:
		{
			if(flags&CMD_START) 
			{
				m_throw = false;
				if(State() == MS_IDLE) State(MS_THREATEN);
			} 
			else if(State() == MS_READY || State() == MS_THREATEN) 
			{
				m_throw = true; 
				if(State() == MS_READY) State(MS_THROW);
			}
		} 
		return true;
	}
*/
	return false;
}

void CBolt::Destroy()
{
	inherited::Destroy();
}

void CBolt::PutNextToSlot()
{
	if (OnClient()) return;

	VERIFY(!getDestroy());

	NET_Packet						P;
	if (m_pCurrentInventory)
	{
		m_pCurrentInventory->Ruck(this);

		this->u_EventGen(P, GEG_PLAYER_ITEM2RUCK, this->H_Parent()->ID());
		P.w_u16(this->ID());
		this->u_EventSend(P);
	}
	else
		Msg("! PutNextToSlot : m_pInventory = NULL [%d][%d]", ID(), Device.dwFrame);

	if (smart_cast<CInventoryOwner*>(H_Parent()) && m_pCurrentInventory)
	{
		CBolt* pNext = smart_cast<CBolt*>(m_pCurrentInventory->Same(this, true));
		if (!pNext) pNext = smart_cast<CBolt*>(m_pCurrentInventory->SameSlot(GRENADE_SLOT, this, true));

		VERIFY(pNext != this);

		if (pNext && m_pCurrentInventory->Slot(pNext))
		{

			pNext->u_EventGen(P, GEG_PLAYER_ITEM2SLOT, pNext->H_Parent()->ID());
			P.w_u16(pNext->ID());
			pNext->u_EventSend(P);
			m_pCurrentInventory->SetActiveSlot(pNext->GetSlot());
		}
	}
}

void CBolt::activate_physic_shell	()
{
	inherited::activate_physic_shell	();
	m_pPhysicsShell->SetAirResistance	(.0001f);
}

void CBolt::SetInitiator			(u16 id)
{
	m_thrower_id=id;
}
u16	CBolt::Initiator				()
{
	return m_thrower_id;
}