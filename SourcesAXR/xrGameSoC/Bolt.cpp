#include "stdafx.h"
#include "bolt.h"
#include "ParticlesObject.h"
#include "PhysicsShell.h"
#include "xr_level_controller.h"
#include "actor.h"
#include "inventory.h"
#include "ai_sounds.h"
#include "AdvancedXrayGameConstants.h"

CBolt::CBolt(void) 
{
	m_thrower_id				=u16(-1);
}

CBolt::~CBolt(void) 
{
}

void CBolt::Load(LPCSTR section)
{
	inherited::Load(section);
	m_sounds.LoadSound(section, "snd_throw_start", "sndThrowStart", false, ESoundTypes(SOUND_TYPE_WEAPON_RECHARGING));
	m_sounds.LoadSound(section, "snd_throw", "sndThrow", false, ESoundTypes(SOUND_TYPE_WEAPON_RECHARGING));
}

void CBolt::OnH_A_Chield() 
{
	inherited::OnH_A_Chield();
	CObject* o= H_Parent()->H_Parent();
	if(o)SetInitiator(o->ID());
	
}

void CBolt::State(u32 state)
{
	auto actor = smart_cast<CActor*>(this->H_Parent());

	switch (GetState())
	{
	case eThrowStart:
	{
		if (!m_sounds.FindSoundItem("sndThrowStart", false) && !actor)
			return;

		PlaySound("sndThrowStart", actor->Position());
	} break;
	case eThrowEnd:
	{
		if (GameConstants::GetLimitedBolts() && actor && (Level().CurrentViewEntity() == H_Parent()))
		{
			if (m_pPhysicsShell) m_pPhysicsShell->Deactivate();
			xr_delete(m_pPhysicsShell);
			m_dwDestroyTime = 0xffffffff;
			PutNextToSlot();
			if (Local())
			{
#ifdef DEBUG
				Msg("Destroying local bolt[%d][%d]", ID(), Device.dwFrame);
#endif
				DestroyObject();
			}
		}
	}break;
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

	auto actor = smart_cast<CActor*>(this->H_Parent());
	if (!m_sounds.FindSoundItem("sndThrow", false) || !actor)
		return;

	PlaySound("sndThrow", actor->Position());
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

void CBolt::PutNextToSlot()
{
	if (OnClient()) return;

	VERIFY(!getDestroy());
	//выкинуть болт из инвентаря
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
		CBolt *pNext = smart_cast<CBolt*>(m_pCurrentInventory->Same(this, true));
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

void CBolt::OnAnimationEnd(u32 state)
{
	switch (state)
	{
	case eThrowEnd:
	{
		if (smart_cast<CActor*>(this->H_Parent()) && (Level().CurrentViewEntity() == H_Parent()))
			SwitchState(eHidden);
	}break;
	}
	inherited::OnAnimationEnd(state);
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

bool CBolt::GetBriefInfo(II_BriefInfo& info)
{
	info.clear();
	info.name = NameShort();
	info.icon = *cNameSect();

	return true;
}