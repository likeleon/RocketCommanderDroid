#include "SmallAsteroid.hpp"
#include "GameAsteroidManager.hpp"

using namespace Ogre;

namespace rcd
{
	const float SmallAsteroid::DownScalings[] =
	{
		0.00243701926f, 0.00213851035f, 0.00200786744f
	};

	SmallAsteroid::SmallAsteroid(SceneManager &sceneMgr, int type, Vector3 position)
		: Model(sceneMgr, "SmallAsteroid" + StringConverter::toString(type + 1), DownScalings[type]),
		  m_type(type),
		  m_size(GameAsteroidManager::SmallAsteroidSize + Math::RangeRandom(-2.5f, +5.0f))
	{
		SetPosition(position);
		SetScale(Vector3(m_size));

		// Set orientation
	}

	void SmallAsteroid::SetSize(float size)
	{
		m_size = size;
		SetScale(Vector3(m_size));
	}
}
