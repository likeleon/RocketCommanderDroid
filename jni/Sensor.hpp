#ifndef _SENSOR_HPP_
#define _SENSOR_HPP_

#include "Types.hpp"
#include <android/sensor.h>

namespace likeleon
{
	class EventLoop;

	class Sensor
	{
	public:
		Sensor(EventLoop& eventLoop, int32_t sensorType);

		status toggle();
		status enable();
		status disable();

	private:
		EventLoop& m_eventLoop;
		const ASensor* m_pSensor;
		int32_t m_sensorType;
	};
}

#endif
