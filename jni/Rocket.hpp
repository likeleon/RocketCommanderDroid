#ifndef _ROCKET_HPP_
#define _ROCKET_HPP_

#include <Ogre.h>
#include "Model.hpp"

namespace rcd
{
	class Rocket : public Model
	{
	public:
		Rocket(Ogre::SceneManager &sceneMgr)
		 : Model(sceneMgr, "rocket", 0.00753780361f)
		{
		}
	};
}

#endif
