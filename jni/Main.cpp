#include "RocketCommanderDroid.hpp"
#include "EventLoop.hpp"

void android_main(android_app* pApplication)
{
	likeleon::EventLoop eventLoop(pApplication);
	rcd::RocketCommanderDroid rocketCommanderDroid;
	eventLoop.run(&rocketCommanderDroid);
}
