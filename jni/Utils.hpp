#ifndef _UTILS_HPP_
#define _UTILS_HPP_

namespace rcd
{
	template <typename T, unsigned int i> inline unsigned int ArraySize(T (&a)[i])
	{
		return sizeof a / sizeof a[0];
	}
}

#endif
