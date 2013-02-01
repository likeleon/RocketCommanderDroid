#ifndef _ASTEROID_HPP_
#define _ASTEROID_HPP_

#include "Model.hpp"

namespace rcd
{
	class Asteroid : public Model
	{
	public:
		static const float CollisionSizeCorrectionFactor;

		Asteroid(Ogre::SceneManager &sceneMgr, int type, const Ogre::Vector3 &position);

		void UpdateMovement(float moveFactor);

		// Set methods
		void SetMovement(const Ogre::Vector3 &movement) { m_movement = movement; }
		void SetSize(float size);
		void SetCollisionRadius(float radius) { m_collisionRadius = radius; }
		void SetRotationSpeed(const Ogre::Vector2 &rotationSpeed) { m_rotationSpeed = rotationSpeed; }

		// Get methods
		float GetCollisionRadius() const { return m_collisionRadius; }
		const Ogre::Vector3& GetMovement() const { return m_movement; }
		float GetSize() const { return m_size; }
		const Ogre::Vector2& GetRotationSpeed() { return m_rotationSpeed; }

	private:
		static const float DownScalings[];

		int m_type;
		float m_size;
		Ogre::Vector3 m_movement;
		Ogre::Vector3 m_rotation;
		Ogre::Vector2 m_rotationSpeed;
		float m_collisionRadius;
	};
}

#endif
