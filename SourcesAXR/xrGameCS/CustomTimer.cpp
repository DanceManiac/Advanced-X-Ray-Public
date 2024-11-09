////////////////////////////////////////////////////////////////////////////
//	Module 		: CustomTimer.cpp
//	Created 	: 15.08.2023
//  Modified 	: 15.08.2023
//	Author		: Dance Maniac (M.F.S. Team)
//	Description : Engine custom timer
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CustomTimer.h"
#include "object_broker.h"
#include "ai_space.h"
#include "alife_simulator.h"
#include "alife_time_manager.h"
#include "../xrEngine/device.h"

#include "script_engine.h"
#include <luabind/luabind.hpp>
#include <luabind/functor.hpp>

CCustomTimer::CCustomTimer()
{
	m_sTimerName = "";
	m_iTimerStartValue = 0;
	m_iTimerCurValue = 0;
	m_bIsActive = false;
}

CCustomTimer::~CCustomTimer()
{
	StopCustomTimer();
}

void CCustomTimer::StartCustomTimer()
{
	m_bIsActive = true;
	m_iStartTime = ai().alife().time_manager().game_time();

#ifdef DEBUG
	Msg("Custom Timer: %s : Started", m_sTimerName.c_str());
#endif
}

void CCustomTimer::StopCustomTimer()
{
	m_bIsActive = false;

#ifdef DEBUG
	Msg("Custom Timer: %s : Stopped (timer value: %d)", m_sTimerName.c_str(), m_iTimerCurValue);
#endif
}

void CCustomTimer::ResetCustomTimer()
{
#ifdef DEBUG
	Msg("Custom Timer: %s : Reset (timer value: %d)", m_sTimerName, m_iTimerCurValue);
#endif

	StopCustomTimer();

	m_iTimerCurValue = 0;

	StartCustomTimer();
}

void CCustomTimer::save(NET_Packet& packet)
{
	save_data(m_sTimerName, packet);
	save_data(m_iTimerStartValue, packet);
	save_data(m_iTimerCurValue, packet);
	save_data(m_iTimerMode, packet);
	save_data(m_iStartTime, packet);
	save_data(m_bIsActive, packet);
}

void CCustomTimer::load(IReader& packet)
{
	load_data(m_sTimerName, packet);
	load_data(m_iTimerStartValue, packet);
	load_data(m_iTimerCurValue, packet);
	load_data(m_iTimerMode, packet);
	load_data(m_iStartTime, packet);
	load_data(m_bIsActive, packet);
}

void CCustomTimer::Update()
{
	if (!m_bIsActive)
		return;

	ALife::_TIME_ID elapsedTime = (ai().alife().time_manager().game_time() - m_iStartTime);

	switch (m_iTimerMode)
	{
	case 1: // seconds
		m_iTimerCurValue = elapsedTime / 1000;
		break;
	case 2: // minutes
		m_iTimerCurValue = elapsedTime / (1000 * 60);
		break;
	case 3: // hours
		m_iTimerCurValue = elapsedTime / (1000 * 60 * 60);
		break;
	default: // milliseconds
		m_iTimerCurValue = elapsedTime;
	}

	if (m_iTimerCurValue >= m_iTimerStartValue)
	{
		OnTimerStop(m_sTimerName);

		m_bIsActive = false;

		luabind::functor<void> funct;
		if (ai().script_engine().functor("mfs_functions.on_custom_timer_end", funct))
			funct(m_sTimerName.c_str());
	}
}

void CTimerManager::CreateTimer(std::string name, ALife::_TIME_ID value, ETimerMode mode)
{
	for (auto& timer : Timers)
	{
		if ((*timer).getName() == name)
		{
			Msg("! Custom Timer with name [%s] already exists!");
			return;
		}
#ifdef DEBUG
		else
		{
			Msg("Custom Timer: %s : Created (start value: %d)", (*timer).getName().c_str(), (*timer).getValue());
		}
#endif
	}

	Timers.push_back(std::make_shared<CCustomTimer>(name, value, mode));
}

bool CTimerManager::DeleteTimer(std::string name)
{
	for (auto it = Timers.begin(); it != Timers.end(); ++it)
	{
		if ((*it)->getName() == name)
		{
			(*it)->StopCustomTimer();
			Timers.erase(it);

#ifdef DEBUG
			Msg("Custom Timer: %s : Deleted", (*it)->getName().c_str());
#endif
			return true;
		}
	}
	return false;
}

bool CTimerManager::ResetTimer(std::string name)
{
	for (auto& timer : Timers)
	{
		if ((*timer).getName() == name)
		{
			(*timer).ResetCustomTimer();
			return true;
		}
	}
	return false;
}

bool CTimerManager::StartTimer(std::string name, ALife::_TIME_ID start_time, ETimerMode mode)
{
	for (auto& timer : Timers)
	{
		if ((*timer).getName() == name)
		{
			(*timer).SetOnTimerStopCallback([this, name = (*timer).getName()](std::string stopped_name)
			{
				OnTimerStop(stopped_name);
			});

			if ((*timer).getCurValue() > 0)
				(*timer).ResetCustomTimer();

			if (mode)
				(*timer).setMode(mode);

			if (start_time > 0)
				(*timer).setValue(start_time);

			(*timer).StartCustomTimer();

			return true;
		}
	}
	return false;
}

bool CTimerManager::StopTimer(std::string name)
{
	for (auto& timer : Timers)
	{
		if ((*timer).getName() == name)
		{
			(*timer).StopCustomTimer();
			return true;
		}
	}
	return false;
}

void CTimerManager::save(NET_Packet& packet)
{
	u32 timer_count = static_cast<u32>(Timers.size());
	save_data(timer_count, packet);

	for (const auto& timer : Timers)
	{
		timer->save(packet);
	}
}

void CTimerManager::load(IReader& packet)
{
	u32 timer_count = 0;
	load_data(timer_count, packet);

	Timers.clear();

	for (u32 i = 0; i < timer_count; ++i)
	{
		auto timer = std::make_shared<CCustomTimer>();
		timer->load(packet);
		Timers.push_back(timer);
	}
}

ALife::_TIME_ID CTimerManager::GetTimerValue(std::string name) const
{
	for (const auto& timer : Timers)
	{
		if ((*timer).getName() == name)
		{
			return (*timer).getCurValue();
		}
	}

	return -1;
}

void CTimerManager::Update()
{
	for (auto& timer : Timers)
	{
		timer->Update();
	}
}