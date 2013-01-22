#ifndef _ROCKET_COMMANDER_DROID_H_
#define _ROCKET_COMMANDER_DROID_H_

#include "ActivityHandler.hpp"
#include "Context.hpp"
#include "TimeService.hpp"
#include "Types.hpp"
#include <android_native_app_glue.h>

namespace rcd
{
	class RocketCommanderDroid : public likeleon::ActivityHandler
	{
	public:
		RocketCommanderDroid(likeleon::Context& context, android_app* pApplication);
		~RocketCommanderDroid();

	private:
		// likeleon::ActivityHandler
		virtual likeleon::status onActivate();
		virtual void onDeactivate();
		virtual likeleon::status onStep();

		virtual void onInit();
		virtual void onDestroy();
		virtual void onCreateWindow();
		virtual void onDestroyWindow();

	private:
		android_app* m_pApplication;
		bool m_initialized;
	};
}

#endif
