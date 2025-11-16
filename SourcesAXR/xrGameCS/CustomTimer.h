#pragma once

#include <functional>
#include "../xrServerEntitiesCS/alife_space.h"

enum ETimerMode
{
	eTimerModeMilliseconds = 0,
	eTimerModeSeconds,
	eTimerModeMinutes,
	eTimerModeHours,
};

class CCustomTimer
{
	std::string			m_sTimerName;
	ALife::_TIME_ID		m_iTimerStartValue;
	ALife::_TIME_ID		m_iTimerCurValue;
	ETimerMode			m_iTimerMode; //0 - milliseconds, 1 - seconds, 2 - minutes, 3 hours
	ALife::_TIME_ID		m_iStartTime;
	bool				m_bIsActive;

	std::function<void(std::string)> OnTimerStop = [](std::string) {};

	CCustomTimer(const CCustomTimer&) = delete;
	CCustomTimer& operator=(const CCustomTimer&) = delete;

public:
	CCustomTimer					();
	CCustomTimer					(std::string name, int value, ETimerMode mode = eTimerModeMilliseconds) : m_sTimerName(name), m_iTimerStartValue(value), m_iTimerMode(mode), m_iTimerCurValue(0), m_iStartTime(0), m_bIsActive(false) {}

	~CCustomTimer					();

	void StartCustomTimer			();
	void StopCustomTimer			();
	void ResetCustomTimer			();
	void Update						();

	void setName					(std::string name)			{m_sTimerName = name;}
	std::string getName				() const					{return m_sTimerName;}

	void setValue					(int value)					{m_iTimerStartValue = value;}
	ALife::_TIME_ID getValue		() const					{return m_iTimerStartValue;}

	void setCurValue				(ALife::_TIME_ID value)		{m_iTimerCurValue = value;}
	ALife::_TIME_ID getCurValue		() const					{return m_iTimerCurValue;}

	void setMode					(ETimerMode mode)			{m_iTimerMode = mode;}
	ETimerMode  getMode				() const					{return m_iTimerMode;}

	void save						(NET_Packet& output_packet);
	void load						(IReader& input_packet);

	void SetOnTimerStopCallback		(std::function<void(std::string)> callback)
	{
		OnTimerStop = callback;
	}
};

class CTimerManager
{
	std::vector<std::shared_ptr<CCustomTimer>> Timers;
	std::function<void(std::string)> OnTimerStop = [](std::string) {};

public:
	void CreateTimer	(std::string name, ALife::_TIME_ID value, ETimerMode mode = eTimerModeMilliseconds);
	bool DeleteTimer	(std::string name);
	bool ResetTimer		(std::string name);
	bool StartTimer		(std::string name, ALife::_TIME_ID start_time = 0, ETimerMode mode = eTimerModeMilliseconds);
	bool StopTimer		(std::string name);

	ALife::_TIME_ID GetTimerValue	(std::string name) const;

	void save			(NET_Packet& output_packet);
	void load			(IReader& input_packet);

	void Update			();

	void SetOnTimerStopCallback(std::function<void(std::string)> callback)
	{
		OnTimerStop = callback;
	}
};

