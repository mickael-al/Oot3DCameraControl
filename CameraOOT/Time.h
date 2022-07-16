#ifndef __ENGINE_TIME__
#define __ENGINE_TIME__

#include <chrono>

class Time
{
public:
	Time();
	void startTime();
	void fixedUpdateTime();
	void updateTime();
	float getDeltaTime();
	float getFixedDeltaTime();
	float getTime();
private:
	std::chrono::steady_clock::time_point m_startTime;
	std::chrono::steady_clock::time_point m_currentTimeF;
	std::chrono::steady_clock::time_point m_lastTimeF;
	std::chrono::steady_clock::time_point m_currentTime;
	std::chrono::steady_clock::time_point m_lastTime;
	float m_time = 0.0f;
	float m_deltaTime = 0.0f;
	float m_fixedDeltaTime = 0.0f;
};


#endif // __ENGINE_TIME__
