#ifndef _CONTEXT_HPP_
#define _CONTEXT_HPP_

#include "Types.hpp"

namespace likeleon
{
	class TimeService;
	class SensorHandler;

	struct Context
	{
		TimeService* m_pTimeService;
		SensorHandler* m_pSensorHandler;
	};
}

#endif
