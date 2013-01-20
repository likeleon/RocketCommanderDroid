#ifndef _EVENT_LOOP_H_
#define _EVENT_LOOP_H_

#include "Types.hpp"
#include <android_native_app_glue.h>

namespace likeleon
{
	class EventLoop
	{
	public:
		EventLoop(android_app* pApplication);
		void run();

	private:
		android_app* m_pApplication;
	};
}

#endif
