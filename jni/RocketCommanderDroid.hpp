#ifndef _ROCKET_COMMANDER_DROID_H_
#define _ROCKET_COMMANDER_DROID_H_

#include "ActivityHandler.hpp"
#include "Types.hpp"

namespace rcd
{
	class RocketCommanderDroid : public likeleon::ActivityHandler
	{
	public:
		RocketCommanderDroid();
		~RocketCommanderDroid();

	protected:
		likeleon::status onActivate();
		void onDeactivate();
		likeleon::status onStep();
	};
}

#endif
