#ifndef _TYPES_HPP_
#define _TYPES_HPP_

#include <stdint.h>

namespace likeleon
{
	typedef int32_t status;

	const status STATUS_OK = 0;
	const status STATUS_KO = -1;
	const status STATUS_EXIT = -2;
}
#endif
