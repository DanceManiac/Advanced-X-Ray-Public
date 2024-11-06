#pragma once

#include <functional>
#include "../xrServerEntitiesCS/alife_space.h"

class CCustomTimer
{
	std::string			m_sTimerName;
	int					m_iTimerStartValue;
	int					m_iTimerCurValue;
	int					m_iTimerMode; //0 - milliseconds, 1 - seconds, 2 - minutes, 3 hours
	ALife::_TIME_ID		m_iStartTime;
	bool				m_bIsActive;

	std::function<void(std::string)> OnTimerStop = [](std::string) {};

	CCustomTimer(const CCustomTimer&) = delete;
	CCustomTimer& operator=(const CCustomTimer&) = delete;

public:
	CCustomTimer			();
	CCustomTimer			(std::string name, int value, int mode = 0) : m_sTimerName(name), m_iTimerStartValue(value), m_iTimerMode(mode), m_iTimerCurValue(0), m_iStartTime(0), m_bIsActive(false) {}

	~CCustomTimer			();

	void StartCustomTimer	();
	void StopCustomTimer	();
	void ResetCustomTimer	();
	void Update				();

	void setName			(std::string name)	{m_sTimerName = name;}
	std::string getName		() const			{return m_sTimerName;}

	void setValue			(int value)		{m_iTimerStartValue = value;}
	int  getValue			() const		{return m_iTimerStartValue;}

	void setCurValue		(int value)		{m_iTimerCurValue = value;}
	int  getCurValue		() const		{return m_iTimerCurValue;}

	void setMode			(int mode)		{m_iTimerMode = mode;}
	int  getMode			() const		{return m_iTimerMode;}

	void save				(NET_Packet& output_packet);
	void load				(IReader& input_packet);

	void SetOnTimerStopCallback(std::function<void(std::string)> callback)
	{
		OnTimerStop = callback;
	}
};

class CTimerManager
{
	std::vector<std::shared_ptr<CCustomTimer>> Timers;
	std::function<void(std::string)> OnTimerStop = [](std::string) {};

public:
	void CreateTimer	(std::string name, int value, int mode = 0);
	bool DeleteTimer	(std::string name);
	bool ResetTimer		(std::string name);
	bool StartTimer		(std::string name, int start_time = 0, int mode = 0);
	bool StopTimer		(std::string name);

	int  GetTimerValue	(std::string name) const;

	void save			(NET_Packet& output_packet);
	void load			(IReader& input_packet);

	void Update			();

	void SetOnTimerStopCallback(std::function<void(std::string)> callback)
	{
		OnTimerStop = callback;
	}
};

