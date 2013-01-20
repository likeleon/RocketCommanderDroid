#include "EventLoop.hpp"

void android_main(android_app* pApplication)
{
	likeleon::EventLoop eventLoop(pApplication);
	eventLoop.run();
}
