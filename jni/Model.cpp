#include "Model.hpp"

using namespace Ogre;

namespace rcd
{
	Model::Model(SceneManager &sceneMgr, const Ogre::String &name, float downScaling)
	: m_sceneMgr(sceneMgr),
	  m_name(name),
	  m_entity(NULL),
	  m_node(NULL),
	  m_downScaling(downScaling),
	  m_objOrient(Quaternion(-Radian(Ogre::Math::PI / 2), Vector3::UNIT_X))
	{
		m_entity = m_sceneMgr.createEntity(name + ".mesh");
		assert(m_entity);

		m_node = m_sceneMgr.getRootSceneNode()->createChildSceneNode();
		assert(m_node);
		m_node->attachObject(m_entity);

		//// Calculate scailing for this object, used for distance comparisons.
		//m_downScaling = 1.0f / m_entity->getBoundingRadius();
		m_node->setScale(m_downScaling, m_downScaling, m_downScaling);

		m_node->setOrientation(m_objOrient);
	}

	Model::~Model()
	{
		if (m_node)
		{
			m_node->detachAllObjects();

			assert(m_node->getParentSceneNode() == m_sceneMgr.getRootSceneNode());
			m_node->getParentSceneNode()->removeAndDestroyChild(m_node->getName());
		}

		if (m_entity)
		{
			m_sceneMgr.destroyEntity(m_entity);
		}
	}

	void Model::SetScale(const Ogre::Vector3 &scale)
	{
		m_node->setScale(scale * m_downScaling);
	}

	void Model::SetPosition(const Ogre::Vector3& pos)
	{
		assert(m_node);
		m_node->setPosition(pos);
	}

	void Model::SetOrientation(const Ogre::Quaternion& q)
	{
		assert(m_node);
		m_node->setOrientation(q * m_objOrient);
	}

	void Model::Pitch(const Ogre::Radian& angle, Ogre::Node::TransformSpace relativeTo)
	{
		assert(m_node);
		m_node->pitch(angle, relativeTo);
	}

	void Model::Yaw(const Ogre::Radian& angle, Ogre::Node::TransformSpace relativeTo)
	{
		assert(m_node);
		m_node->yaw(angle, relativeTo);
	}

	void Model::Roll(const Ogre::Radian& angle, Ogre::Node::TransformSpace relativeTo)
	{
		assert(m_node);
		m_node->roll(angle, relativeTo);
	}

	const Ogre::Vector3& Model::GetPosition() const
	{
		assert(m_node);
		return m_node->getPosition();
	}
}
