#include "SensorHandler.hpp"
#include "Log.hpp"
#include "Sensor.hpp"

namespace likeleon
{
	SensorHandler::SensorHandler(android_app* pApplication, Sensor* pAccelerometer)
	: m_pApplication(pApplication), m_horizontal(0.0f), m_vertical(0.0f), m_pAccelerometer(pAccelerometer)
	{
	}

	status SensorHandler::start()
	{
		if (m_pAccelerometer->toggle() != STATUS_OK)
			return STATUS_KO;

		return STATUS_OK;
	}

	void SensorHandler::stop()
	{
		m_pAccelerometer->disable();
	}

	bool SensorHandler::onAccelerometerEvent(ASensorEvent* pEvent)
	{
		const float GRAVITY = ASENSOR_STANDARD_GRAVITY / 2.0f;
		const float MIN_X = -1.0f;
		const float MAX_X = 1.0f;
		const float MIN_Y = 0.0f;
		const float MAX_Y = 2.0f;
		const float CENTER_X = (MAX_X + MIN_X) / 2.0f;
		const float CENTER_Y = (MAX_Y + MIN_Y) / 2.0f;

		float rawHorizontal = pEvent->vector.y / GRAVITY;
		if (rawHorizontal > MAX_X)
			rawHorizontal = MAX_X;
		else if (rawHorizontal < MIN_X)
			rawHorizontal = MIN_X;
		m_horizontal = CENTER_X - rawHorizontal;

		float rawVertical = pEvent->vector.z / GRAVITY;
		if (rawVertical > MAX_Y)
			rawVertical = MAX_Y;
		else if (rawVertical < MIN_Y)
			rawVertical = MIN_Y;
		m_vertical = rawVertical - CENTER_Y;

		return true;
	}
}
