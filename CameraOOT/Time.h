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
		void release();		
		float getDeltaTime();
		float getFixedDeltaTime();
		float getTime();
		static float GetFixedDeltaTime();
		static float GetDeltaTime();
		static float GetTime();
	private:
		static Time* s_pInstance;
#ifdef _WIN32 //TODO tester le win32 sur unix
		std::chrono::steady_clock::time_point m_startTime;
		std::chrono::steady_clock::time_point m_currentTimeF;
		std::chrono::steady_clock::time_point m_lastTimeF;
		std::chrono::steady_clock::time_point m_currentTime;
		std::chrono::steady_clock::time_point m_lastTime;
#elif __unix__
		std::chrono::_V2::system_clock::time_point m_startTime;
		std::chrono::_V2::system_clock::time_point m_currentTimeF;
		std::chrono::_V2::system_clock::time_point m_lastTimeF;
		std::chrono::_V2::system_clock::time_point m_currentTime;
		std::chrono::_V2::system_clock::time_point m_lastTime;
#endif
		float m_time = 0.0f;
		float m_deltaTime = 0.0f;
		float m_fixedDeltaTime = 0.0f;
	};


#endif // __ENGINE_TIME__
