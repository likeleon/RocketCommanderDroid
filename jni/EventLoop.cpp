#include "EventLoop.hpp"
#include "Log.hpp"

namespace likeleon
{
	EventLoop::EventLoop(android_app* pApplication)
	: m_enabled(false), m_quit(false), m_pApplication(pApplication), m_pActivityHandler(NULL)
	{
		m_pApplication->userData = this;
		m_pApplication->onAppCmd = callback_event;
	}

	void EventLoop::run(ActivityHandler* pActivityHandler)
	{
		int32_t events;
		android_poll_source* source;

		app_dummy();
		m_pActivityHandler = pActivityHandler;

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
			m_quit = false;
			m_enabled = true;
			if (m_pActivityHandler->onActivate() != STATUS_OK)
			{
				m_quit = true;
				deactivate();
				ANativeActivity_finish(m_pApplication->activity);
			}
		}
	}

	void EventLoop::deactivate()
	{
		if (m_enabled)
		{
			m_pActivityHandler->onDeactivate();
			m_enabled = false;
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

	void EventLoop::callback_event(android_app* pApplication, int32_t pCommand)
	{
		EventLoop& eventLoop = *(EventLoop*)pApplication->userData;
		eventLoop.processAppEvent(pCommand);
	}
}
