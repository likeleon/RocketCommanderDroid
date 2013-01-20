#include "TimeService.hpp"
#include "Log.hpp"

namespace likeleon
{
	TimeService::TimeService()
	: m_elapsed(0.0f), m_lastTime(0.0)
	{
	}

	void TimeService::reset()
	{
		Log::info("Resetting TimeService.");
		m_elapsed = 0.0f;
		m_lastTime = now();
	}

	void TimeService::update()
	{
		double currentTime = now();
		m_elapsed = (currentTime - m_lastTime);
		m_lastTime = currentTime;
	}

	double TimeService::now()
	{
		timespec timeVal;
		clock_gettime(CLOCK_MONOTONIC, &timeVal);
		return timeVal.tv_sec + (timeVal.tv_nsec * 1.0e-9);
	}

	float TimeService::elapsed()
	{
		return m_elapsed;
	}
}
