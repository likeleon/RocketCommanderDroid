#ifndef _COLOUR_HELPER_HPP_
#define _COLOUR_HELPER_HPP_

namespace rcd
{
	class ColourHelper
	{
	public:
		static Ogre::ColourValue InterpolateColour(const Ogre::ColourValue &col1, const Ogre::ColourValue &col2, Ogre::Real percent)
		{
			return (col1 * (1.0f - percent) + col2 * percent);
		}
	};
}

#endif
