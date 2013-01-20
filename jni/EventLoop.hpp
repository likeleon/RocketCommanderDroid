#ifndef _EVENT_LOOP_HPP_
#define _EVENT_LOOP_HPP_

#include "ActivityHandler.hpp"
#include "Types.hpp"
#include <android_native_app_glue.h>

namespace likeleon
{
	class EventLoop
	{
	public:
		EventLoop(android_app* pApplication);
		void run(ActivityHandler* pActivityHandler);

	protected:
		void activate();
		void deactivate();

		void processAppEvent(int32_t pCommand);

	private:
		static void callback_event(android_app* pApplication, int32_t pCommand);

	private:
		bool m_enabled;
		bool m_quit;
		ActivityHandler* m_pActivityHandler;
		android_app* m_pApplication;
	};
}

#endif
