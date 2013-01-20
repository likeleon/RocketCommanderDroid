#ifndef _TIME_SERVICE_HPP_
#define _TIME_SERVICE_HPP_

#include "Types.hpp"
#include <time.h>

namespace likeleon
{
	class TimeService
	{
	public:
		TimeService();

		void reset();
		void update();

		double now();
		float elapsed();

	private:
		float m_elapsed;
		double m_lastTime;
	};
}

#endif
