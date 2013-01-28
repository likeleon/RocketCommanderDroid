#include "Game.hpp"
#include "Log.hpp"

namespace rcd
{
	Game::Game(Ogre::Root& ogreRoot, Ogre::RenderWindow& renderWindow)
	: m_ogreRoot(ogreRoot), m_renderWindow(renderWindow), m_pSceneMgr(NULL)
	, m_pCamera(NULL), m_pViewport(NULL), m_pMeshNode(NULL), m_pLightNode(NULL)
	{
	}

	Game::~Game()
	{
		Cleanup();
	}

	void Game::Initialize()
	{
		likeleon::Log::info("Initialize game");

		// Create scene manager
		m_pSceneMgr = m_ogreRoot.createSceneManager(Ogre::ST_GENERIC, "SceneManager");
		m_pSceneMgr->setAmbientLight(Ogre::ColourValue::White);

		// Light
		m_pLight = m_pSceneMgr->createLight();
		m_pLight->setPosition(0, 0, -150);
		m_pLight->setDiffuseColour(Ogre::ColourValue::Black);
		m_pLight->setSpecularColour(Ogre::ColourValue::Black);

		m_pLightNode = m_pSceneMgr->getRootSceneNode()->createChildSceneNode();
		m_pLightNode->attachObject(m_pLight);

		// Camera
		m_pCamera = m_pSceneMgr->createCamera("Camera");
		m_pCamera->setPosition(0, 0, -280);
		m_pCamera->lookAt(0, 0, 0);
		m_pCamera->setNearClipDistance(0.1);
		m_pCamera->setFarClipDistance(50000);

		// Viewport
		m_pViewport = m_renderWindow.addViewport(m_pCamera);
		m_pViewport->setBackgroundColour(Ogre::ColourValue(0, 0, 0));
		m_pCamera->setAspectRatio(Ogre::Real(m_pViewport->getActualWidth()) / Ogre::Real(m_pViewport->getActualHeight()));
		m_pViewport->setCamera(m_pCamera);

		// Skybox
		m_pSceneMgr->setSkyBox(true, "RocketCommander/SpaceSkyBox", 10, true);

		// Mesh
		m_pSceneMgr->createEntity("rocket", "rocket.mesh");
		m_pMeshNode = m_pSceneMgr->getRootSceneNode()->createChildSceneNode();
		m_pMeshNode->attachObject(m_pSceneMgr->getEntity("rocket"));
		m_pMeshNode->scale(0.5, 0.5, 0.5);
		m_pMeshNode->yaw(Ogre::Radian(Ogre::Math::PI / 2.0f));

		m_ogreRoot.addFrameListener(this);
	}

	void Game::Update(double timeSinceLastFrame)
	{
		m_pLightNode->yaw(Ogre::Degree((Ogre::Real)timeSinceLastFrame / 30));
	}

	void Game::Cleanup()
	{
		likeleon::Log::info("Cleanup game");

		if (m_pViewport)
		{
			m_renderWindow.removeViewport(m_pViewport->getZOrder());
			m_pViewport = NULL;
		}

		if (m_pCamera)
		{
			delete m_pCamera;
			m_pCamera = NULL;
		}

		if (m_pLight)
		{
			m_pSceneMgr->destroyLight(m_pLight);
			m_pLight = NULL;
		}

		if (m_pSceneMgr)
		{
			m_ogreRoot.destroySceneManager(m_pSceneMgr);
			m_pSceneMgr = NULL;
		}
	}

	bool Game::frameStarted(const Ogre::FrameEvent& evt)
	{
		return true;
	}

	bool Game::frameRenderingQueued(const Ogre::FrameEvent& evt)
	{
		Update(evt.timeSinceLastFrame * 1000.0);
		return true;
	}

	bool Game::frameEnded(const Ogre::FrameEvent& evt)
	{
		return !(m_renderWindow.isClosed());
	}
}
