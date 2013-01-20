#include "RocketCommanderDroid.hpp"
#include "EventLoop.hpp"

void android_main(android_app* pApplication)
{
	likeleon::TimeService timeService;
	likeleon::Context context = { &timeService };

	likeleon::EventLoop eventLoop(pApplication);
	rcd::RocketCommanderDroid rocketCommanderDroid(context, pApplication);
	eventLoop.run(&rocketCommanderDroid);
}
