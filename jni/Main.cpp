#include "RocketCommanderDroid.hpp"
#include "EventLoop.hpp"
#include "SensorHandler.hpp"
#include "Sensor.hpp"

void android_main(android_app* pApplication)
{
	likeleon::EventLoop eventLoop(pApplication);
	likeleon::Sensor accelerometer(eventLoop, ASENSOR_TYPE_ACCELEROMETER);

	likeleon::TimeService timeService;
	likeleon::SensorHandler sensorHandler(pApplication, &accelerometer);
	likeleon::Context context = { &timeService, &sensorHandler };

	rcd::RocketCommanderDroid rocketCommanderDroid(context, pApplication);
	eventLoop.run(&rocketCommanderDroid, &sensorHandler);
}
