#ifndef _RANDOM_HELPER_HPP_
#define _RANDOM_HELPER_HPP_

namespace rcd
{
	class RandomHelper
	{
	public:
		static Ogre::Vector2 GetRandomVector2(Ogre::Real min, Ogre::Real max)
		{
			return Ogre::Vector2(Ogre::Math::RangeRandom(min, max),
				Ogre::Math::RangeRandom(min, max));
		}

		static Ogre::Vector3 GetRandomVector3(Ogre::Real min, Ogre::Real max)
		{
			return Ogre::Vector3(Ogre::Math::RangeRandom(min, max),
				Ogre::Math::RangeRandom(min, max),
				Ogre::Math::RangeRandom(min, max));
		}

		static Ogre::ColourValue GetRandomColour(float alpha = 1.0f)
		{
			return Ogre::ColourValue(Ogre::Math::RangeRandom(0, 1),
				Ogre::Math::RangeRandom(0, 1),
				Ogre::Math::RangeRandom(0, 1), alpha);
		}

		static int GetRandomInt(int min, int max)
		{
			const int randValue = (int)Ogre::Math::RangeRandom((Ogre::Real)min, (Ogre::Real)max + 1.0f);
			return std::min(max, randValue);
		}

		static Ogre::Quaternion GetRandomQuaternion()
		{
			Ogre::Radian angle = Ogre::Radian(Ogre::Math::RangeRandom(0.0f, Ogre::Math::PI * 2.0f));
			return Ogre::Quaternion(angle, Ogre::Vector3::UNIT_X)
				* Ogre::Quaternion(angle, Ogre::Vector3::UNIT_Y)
				* Ogre::Quaternion(angle, Ogre::Vector3::UNIT_Z);
		}
	};
}

#endif
