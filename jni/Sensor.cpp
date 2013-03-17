#include "Sensor.hpp"
#include "EventLoop.hpp"
#include "Log.hpp"

namespace likeleon
{
	Sensor::Sensor(EventLoop& eventLoop, int32_t sensorType)
	: m_eventLoop(eventLoop), m_pSensor(NULL), m_sensorType(sensorType)
	{
	}

	status Sensor::toggle()
	{
		return (m_pSensor != NULL) ? disable() : enable();
	}

	status Sensor::enable()
	{
		if (m_eventLoop.m_enabled)
		{
			m_pSensor = ASensorManager_getDefaultSensor(m_eventLoop.m_pSensorManager, m_sensorType);
			if (m_pSensor != NULL)
			{
				if (ASensorEventQueue_enableSensor(m_eventLoop.m_pSensorEventQueue, m_pSensor) < 0)
					goto ERROR;
			}
		}
		else
		{
			likeleon::Log::error("no sensor type %d", m_sensorType);
		}
		return STATUS_OK;

	ERROR:
		likeleon::Log::error("Error while activating sensor.");
		disable();
		return STATUS_KO;
	}

	status Sensor::disable()
	{
		if ((m_eventLoop.m_enabled) && (m_pSensor != NULL))
		{
			if (ASensorEventQueue_disableSensor(m_eventLoop.m_pSensorEventQueue, m_pSensor) < 0)
				goto ERROR;

			m_pSensor = NULL;
		}
		return STATUS_OK;

	ERROR:
		likeleon::Log::error("Error while deactivating sensor.");
		return STATUS_KO;
	}
}
