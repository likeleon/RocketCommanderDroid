#include "EventLoop.hpp"
#include "Log.hpp"

namespace likeleon
{
	EventLoop::EventLoop(android_app* pApplication)
	: m_enabled(false), m_quit(false), m_pApplication(pApplication), m_pActivityHandler(NULL),
	  m_pSensorManager(NULL), m_pSensorEventQueue(NULL)
	{
		m_pApplication->userData = this;
		m_pApplication->onAppCmd = callback_event;
		m_pApplication->onInputEvent = handle_input;
	}

	void EventLoop::run(ActivityHandler* pActivityHandler, SensorHandler* pSensorHandler)
	{
		int32_t events;
		android_poll_source* source;

		app_dummy();
		m_pActivityHandler = pActivityHandler;
		m_pSensorHandler = pSensorHandler;

		m_pActivityHandler->onInit();

		likeleon::Log::info("Starting event loop");
		while (true)
		{
			while (ALooper_pollAll(m_enabled ? 0 : -1, NULL, &events, (void**)&source) >= 0)
			{
				likeleon::Log::info("pollAll returned positive value");

				if (source != NULL)
				{
					likeleon::Log::info("Processing an event");
					source->process(m_pApplication, source);
				}
				if (m_pApplication->destroyRequested)
				{
					likeleon::Log::info("Exiting event loop");
					return;
				}
			}

			if (m_enabled && !m_quit)
			{
				if (m_pActivityHandler->onStep() != STATUS_OK)
				{
					m_quit = true;

					ANativeActivity_finish(m_pApplication->activity);
				}
			}
		}
	}

	void EventLoop::activate()
	{
		if (!m_enabled && m_pApplication != NULL)
		{
			m_sensorPollSource.id = LOOPER_ID_USER;
			m_sensorPollSource.app = m_pApplication;
			m_sensorPollSource.process = callback_sensor;
			m_pSensorManager = ASensorManager_getInstance();
			if (m_pSensorManager != NULL)
			{
				m_pSensorEventQueue = ASensorManager_createEventQueue(m_pSensorManager, m_pApplication->looper, LOOPER_ID_USER, NULL, &m_sensorPollSource);
				if (m_pSensorEventQueue == NULL)
					goto ERROR;
			}

			m_quit = false;
			m_enabled = true;
			if (m_pActivityHandler->onActivate() != STATUS_OK)
				goto ERROR;
		}
		return;

	ERROR:
		m_quit = true;
		deactivate();
		ANativeActivity_finish(m_pApplication->activity);
	}

	void EventLoop::deactivate()
	{
		if (m_enabled)
		{
			m_pActivityHandler->onDeactivate();
			m_enabled = false;

			if (m_pSensorEventQueue != NULL)
			{
				ASensorManager_destroyEventQueue(m_pSensorManager, m_pSensorEventQueue);
				m_pSensorEventQueue = NULL;
			}
			m_pSensorManager = NULL;
		}
	}

	void EventLoop::processAppEvent(int32_t pCommand)
	{
		switch (pCommand)
		{
		case APP_CMD_CONFIG_CHANGED:
			m_pActivityHandler->onConfigurationChanged();
			break;

		case APP_CMD_INIT_WINDOW:
			m_pActivityHandler->onCreateWindow();
			break;

		case APP_CMD_DESTROY:
			m_pActivityHandler->onDestroy();
			break;

		case APP_CMD_GAINED_FOCUS:
			activate();
			m_pActivityHandler->onGainFocus();
			break;

		case APP_CMD_LOST_FOCUS:
			m_pActivityHandler->onLostFocus();
			deactivate();
			break;

		case APP_CMD_LOW_MEMORY:
			m_pActivityHandler->onLowMemory();
			break;

		case APP_CMD_PAUSE:
			m_pActivityHandler->onPause();
			deactivate();
			break;

		case APP_CMD_RESUME:
			m_pActivityHandler->onResume();
			break;


		case APP_CMD_SAVE_STATE:
			m_pActivityHandler->onSaveState(&m_pApplication->savedState, &m_pApplication->savedStateSize);
			break;

		case APP_CMD_START:
			m_pActivityHandler->onStart();
			break;

		case APP_CMD_STOP:
			m_pActivityHandler->onStop();
			break;

		case APP_CMD_TERM_WINDOW:
			m_pActivityHandler->onDestroyWindow();
			deactivate();
			break;

		default:
			break;
		}
	}

	int32_t EventLoop::handleInputEvent(AInputEvent* pEvent)
	{
		return m_pActivityHandler->onInputEvent(pEvent);
	}

	void EventLoop::processSensorEvent()
	{
		ASensorEvent lEvent;
		while (ASensorEventQueue_getEvents(m_pSensorEventQueue, &lEvent, 1) > 0)
		{
			switch (lEvent.type)
			{
			case ASENSOR_TYPE_ACCELEROMETER:
				m_pSensorHandler->onAccelerometerEvent(&lEvent);
				break;
			}
		}
	}

	void EventLoop::callback_event(android_app* pApplication, int32_t pCommand)
	{
		EventLoop& eventLoop = *(EventLoop*)pApplication->userData;
		eventLoop.processAppEvent(pCommand);
	}

	int32_t EventLoop::handle_input(android_app* pApplication, AInputEvent* event)
	{
		EventLoop& eventLoop = *(EventLoop*)pApplication->userData;
		return eventLoop.handleInputEvent(event);
	}

	void EventLoop::callback_sensor(android_app* pApplication, android_poll_source* pSource)
	{
		EventLoop& lEventLoop = *(EventLoop*)pApplication->userData;
		lEventLoop.processSensorEvent();
	}
}
