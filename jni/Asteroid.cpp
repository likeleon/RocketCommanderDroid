#include "Asteroid.hpp"
#include "GameAsteroidManager.hpp"
#include "RandomHelper.hpp"

using namespace Ogre;

namespace rcd
{
	const float Asteroid::CollisionSizeCorrectionFactor = 0.9149594f;
	const float Asteroid::DownScalings[] =
	{
		0.0249557421f, 0.01308855f, 0.0136641478f, 0.0136658037f, 0.008148187f
	};

	Asteroid::Asteroid(SceneManager &sceneMgr, int type, const Vector3 &position)
	: Model(sceneMgr, "asteroid" + StringConverter::toString(type + 1), DownScalings[type]),
	  m_type(type),
	  m_size(Math::RangeRandom(GameAsteroidManager::MinAsteroidSize, GameAsteroidManager::MaxAsteroidSize))
	{
		SetPosition(position);
		SetScale(Vector3(m_size));

		m_collisionRadius = m_size * CollisionSizeCorrectionFactor;
		if (type > 0)
			m_collisionRadius *= CollisionSizeCorrectionFactor;
		if (type == 4)
			m_collisionRadius *= CollisionSizeCorrectionFactor;

		// 60%: slow, 35%: normal, 5% fast moving asteroids
		const float speedFactor = Math::RangeRandom(-20.0f, +20.0f);
		Vector3 flyDirection = RandomHelper::GetRandomVector3(-1.0f, +1.0f);
		flyDirection.normalise();
		m_movement = flyDirection * speedFactor;

		// Rotation
		const float pi = (float)Math::PI;
		m_rotation = RandomHelper::GetRandomVector3(-pi, +pi);
		m_rotationSpeed = Vector2(Math::RangeRandom(-0.25f, +0.6f), Math::RangeRandom(-0.75f, +1.0f));

		// Randomly create fast flying asteroids
		const float highSpeedNumber = Math::RangeRandom(0.0f, 1.0f);
		if (highSpeedNumber < 0.05)
		{
			m_movement *= 5;
			m_rotationSpeed.y = Math::RangeRandom(1.28f, 4.28f);
		}

		if (highSpeedNumber < 0.4)
		{
			m_movement *= 2;
		}

		// Make donuts asteroids very big
		if (m_type == 4)
		{
			m_size += GameAsteroidManager::MaxAsteroidSize;
			SetScale(Vector3(m_size));
			m_rotationSpeed = Vector2(m_rotationSpeed.x / 3, m_rotationSpeed.y / 4);
		}
	}

	void Asteroid::UpdateMovement(float moveFactor)
	{
		// Apply rotation
		Pitch(Radian(m_rotationSpeed.x * moveFactor));
		Yaw(Radian(m_rotationSpeed.y * moveFactor));

		// Movement
		SetPosition(GetPosition() + m_movement * moveFactor);
	}

	void Asteroid::SetSize(float size)
	{
		m_size = size;
		SetScale(Vector3(m_size));
	}
}
