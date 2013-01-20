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

	protected:
		likeleon::status onActivate();
		void onDeactivate();
		likeleon::status onStep();

	private:
		void clear();
		void drawCursor(int pSize, int pX, int pY);

	private:
		android_app* m_pApplication;
		ANativeWindow_Buffer m_windowBuffer;
		likeleon::TimeService* m_pTimeService;

		bool m_initialized;

		float m_posX;
		float m_posY;
		const int32_t m_size;
		const float m_speed;
	};
}

#endif
