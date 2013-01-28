#ifndef _GAME_HPP_
#define _GAME_HPP_

#include <Ogre.h>

namespace rcd
{
	class Game : public Ogre::FrameListener
	{
	public:
		Game(Ogre::Root& ogreRoot, Ogre::RenderWindow& renderWindow);
		~Game();

		void Initialize();
		void Cleanup();

	private:
		void Update(double timeSinceLastFrame);

		// Overrides Ogre::FrameListener
		virtual bool frameStarted(const Ogre::FrameEvent& evt);
		virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
		virtual bool frameEnded(const Ogre::FrameEvent& evt);

		Ogre::Root& m_ogreRoot;
		Ogre::RenderWindow& m_renderWindow;
		Ogre::SceneManager* m_pSceneMgr;
		Ogre::Light* m_pLight;
		Ogre::Camera* m_pCamera;
		Ogre::Viewport* m_pViewport;
		Ogre::SceneNode* m_pMeshNode;
		Ogre::SceneNode* m_pLightNode;
	};
}

#endif
