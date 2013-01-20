#include "EventLoop.hpp"
#include "Log.hpp"

namespace likeleon
{
	EventLoop::EventLoop(android_app* pApplication)
	: m_pApplication(pApplication)
	{
	}

	void EventLoop::run()
	{
		int32_t result;
		int32_t events;
		android_poll_source* source;

		app_dummy();

		likeleon::Log::info("Starting event loop");
		while (true)
		{
			result = ALooper_pollOnce(-1, NULL, &events, (void**)&source);
			while (result >= 0)
			{
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
		}
	}
}
