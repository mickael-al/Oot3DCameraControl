#include "Time.h"


	Time::Time()
	{
		m_time = 0.0f;
	}

	void Time::startTime()
	{
		m_startTime = std::chrono::high_resolution_clock::now();
		m_currentTime = std::chrono::high_resolution_clock::now();
		m_currentTimeF = std::chrono::high_resolution_clock::now();
	}

	void Time::fixedUpdateTime()
	{
		m_lastTimeF = m_currentTimeF;
		m_currentTimeF = std::chrono::high_resolution_clock::now();
		m_time = std::chrono::duration<float, std::chrono::seconds::period>(m_currentTimeF - m_startTime).count();
		m_fixedDeltaTime = std::chrono::duration<float, std::chrono::seconds::period>(m_currentTimeF - m_lastTimeF).count();
	}

	void Time::updateTime()
	{
		m_lastTime = m_currentTime;
		m_currentTime = std::chrono::high_resolution_clock::now();
		m_deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(m_currentTime - m_lastTime).count();
	}

	float Time::getDeltaTime()
	{
		return m_deltaTime;
	}

	float Time::getFixedDeltaTime()
	{
		return m_fixedDeltaTime;
	}

	float Time::getTime()
	{
		return m_time;
	}






