#ifndef _GOAL_HPP_
#define _GOAL_HPP_

#include "Model.hpp"

namespace rcd
{
	class Goal : public Model
	{
	public:
		Goal(Ogre::SceneManager &sceneMgr, const Ogre::Vector3 &position);

	private:
		static const float DownScaling;
	};
}

#endif
