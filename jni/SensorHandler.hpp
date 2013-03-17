#ifndef _SENSOR_HANDLER_HPP_
#define _SENSOR_HANDLER_HPP_

#include <android_native_app_glue.h>
#include <android/sensor.h>
#include "Types.hpp"

namespace likeleon
{
	class Sensor;

	class SensorHandler
	{
	public:
		SensorHandler(android_app* pApplication, Sensor* pAccelerometer);

		status start();
		void stop();

		float GetHorizontal() const { return m_horizontal; }
		float GetVertical() const { return m_vertical; }
		bool onAccelerometerEvent(ASensorEvent* pEvent);

	private:
		android_app* m_pApplication;
		float m_horizontal;
		float m_vertical;
		Sensor* m_pAccelerometer;
	};
}

#endif
