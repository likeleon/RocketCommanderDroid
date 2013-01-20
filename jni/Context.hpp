#ifndef _CONTEXT_HPP_
#define _CONTEXT_HPP_

#include "Types.hpp"

namespace likeleon
{
	class TimeService;

	struct Context
	{
		TimeService* m_pTimeService;
	};
}

#endif
