#ifndef _ACTIVITY_HANDLER_HPP_
#define _ACTIVITY_HANDLER_HPP_

#include "Types.hpp"

namespace likeleon
{
	class ActivityHandler
	{
	public:
		virtual ~ActivityHandler() {};

		virtual status onActivate() = 0;
		virtual void onDeactivate() = 0;
		virtual status onStep() = 0;

		virtual void onInit() {}
		virtual void onStart() {};
		virtual void onResume() {};
		virtual void onPause() {};
		virtual void onStop() {};
		virtual void onDestroy() {};
		virtual void onSaveState(void **pData, size_t* pSize) {};
		virtual void onConfigurationChanged() {};
		virtual void onLowMemory() {};

		virtual void onCreateWindow() {};
		virtual void onDestroyWindow() {};
		virtual void onGainFocus() {};
		virtual void onLostFocus() {};
	};
}

#endif
