#include "RocketCommanderDroid.hpp"
#include "Log.hpp"

#include <unistd.h>

namespace rcd
{
	RocketCommanderDroid::RocketCommanderDroid()
	{
		likeleon::Log::info("Creating RocketCommanderDroid");
	}

	RocketCommanderDroid::~RocketCommanderDroid()
	{
		likeleon::Log::info("Destructing RocketCommanderDroid");
	}

	likeleon::status RocketCommanderDroid::onActivate()
	{
		likeleon::Log::info("Activating RocketCommanderDroid");
		return likeleon::STATUS_OK;
	}

	void RocketCommanderDroid::onDeactivate()
	{
		likeleon::Log::info("Deactivating RocketCommanderDroid");
	}

	likeleon::status RocketCommanderDroid::onStep()
	{
		likeleon::Log::info("Starting step");
		usleep(300000);
		likeleon::Log::info("Stepping done");
		return likeleon::STATUS_OK;
	}
}
