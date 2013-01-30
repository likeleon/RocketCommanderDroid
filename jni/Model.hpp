#ifndef _MODEL_HPP_
#define _MODEL_HPP_

#include <Ogre.h>

namespace rcd
{
	class Model
	{
	public:
		Model(Ogre::SceneManager &sceneMgr, const Ogre::String &name, float downScaling);
		virtual ~Model();

		void SetPosition(const Ogre::Vector3 &pos);
		void SetOrientation(const Ogre::Quaternion &q);
		void SetScale(const Ogre::Vector3 &scale);

		const Ogre::Vector3& GetPosition() const;

		void Pitch(const Ogre::Radian& angle, Ogre::Node::TransformSpace relativeTo = Ogre::Node::TS_LOCAL);
		void Yaw(const Ogre::Radian& angle, Ogre::Node::TransformSpace relativeTo = Ogre::Node::TS_LOCAL);
		void Roll(const Ogre::Radian& angle, Ogre::Node::TransformSpace relativeTo = Ogre::Node::TS_LOCAL);

	private:
		Ogre::SceneManager &m_sceneMgr;
		Ogre::String m_name;
		Ogre::Entity *m_entity;
		Ogre::SceneNode *m_node;

		const float m_downScaling;
		const Ogre::Quaternion m_objOrient;
	};
}

#endif
