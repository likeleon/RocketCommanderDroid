#ifndef _EVENT_LOOP_HPP_
#define _EVENT_LOOP_HPP_

#include "ActivityHandler.hpp"
#include "Types.hpp"
#include <android_native_app_glue.h>
#include "SensorHandler.hpp"

namespace likeleon
{
	class EventLoop
	{
	public:
		EventLoop(android_app* pApplication);
		void run(ActivityHandler* pActivityHandler, SensorHandler* pSensorHandler);

	protected:
		void activate();
		void deactivate();

		void processAppEvent(int32_t pCommand);
		int32_t handleInputEvent(AInputEvent* pEvent);
		void processSensorEvent();

	private:
		static void callback_event(android_app* pApplication, int32_t pCommand);
		static int32_t handle_input(android_app* pApplication, AInputEvent* event);
		static void callback_sensor(android_app* pApplication, android_poll_source* pSource);

	private:
		friend class Sensor;

		bool m_enabled;
		bool m_quit;
		ActivityHandler* m_pActivityHandler;
		android_app* m_pApplication;
		SensorHandler* m_pSensorHandler;
		ASensorManager* m_pSensorManager;
		ASensorEventQueue* m_pSensorEventQueue;
		android_poll_source m_sensorPollSource;
	};
}

#endif
