#ifndef _SMALL_ASTEROID_HPP_
#define _SMALL_ASTEROID_HPP_

#include "Model.hpp"

namespace rcd
{
	class SmallAsteroid : public Model
	{
	public:
		SmallAsteroid(Ogre::SceneManager &sceneMgr, int type, Ogre::Vector3 position);

		void SetSize(float size);

	private:
		static const float DownScalings[];

		int m_type;
		float m_size;
	};
}

#endif
