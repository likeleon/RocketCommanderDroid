#include "Game.hpp"
#include "Log.hpp"
#include "GameAsteroidManager.hpp"
#include "IGameScreen.hpp"
#include "MainMenuScreen.hpp"
#include "SpaceCamera.hpp"
#include "Rocket.hpp"
#include "SdkTrays.h"
#include <OgreResourceGroupManager.h>
#include <OgreRTShaderSystem.h>
#include "ShaderGeneratorTechniqueResolverListener.hpp"

namespace rcd
{
	const Ogre::Radian Game::FieldOfView = Ogre::Radian(Ogre::Math::PI / 1.8f);
	const Ogre::Real Game::NearPlane = GameAsteroidManager::GetMinViewDepth();
	const Ogre::Real Game::FarPlane = GameAsteroidManager::GetMaxViewDepth();

	Game::Game(Ogre::Root& ogreRoot, Ogre::RenderWindow& renderWindow, Ogre::OverlaySystem& overlaySystem) :
		m_ogreRoot(ogreRoot), m_renderWindow(renderWindow), m_pSceneMgr(NULL), m_overlaySystem(overlaySystem)
		, m_pCamera(NULL), m_pViewport(NULL), m_pTrayMgr(NULL), m_pShaderGenerator(NULL), m_pMaterialMgrListener(NULL)
		, m_elapsedTimeThisFrameInMs(0.001f)
		, m_totalTimeMs(0.0f), m_inGame(false), m_pRocket(NULL)
	{
	}

	Game::~Game()
	{
		Cleanup();
	}

	void Game::Initialize()
	{
		likeleon::Log::info("Initialize game");

		m_pTrayMgr = new OgreBites::SdkTrayManager("GameControls", &m_renderWindow,	m_inputContext);

		CreateScene();

		if (m_ogreRoot.getRenderSystem()->getCapabilities()->hasCapability(Ogre::RSC_FIXED_FUNCTION) == false)
		{
			Ogre::RTShader::ShaderGenerator::getSingletonPtr()->addSceneManager(m_pSceneMgr);
		}

		LoadResources();

		Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

		m_ogreRoot.addFrameListener(this);
		m_ogreRoot.getRenderSystem()->_initRenderTargets();
		m_ogreRoot.getSingleton().clearEventTimes();

		// Skybox
		m_pSceneMgr->setSkyBox(true, "RocketCommander/SpaceSkyBox", GetCamera().getFarClipDistance() * 0.5f, true);

		// Rocket
		m_pRocket = new Rocket(*m_pSceneMgr);

		// Main menu screen
		AddGameScreen(new MainMenuScreen(*this));
	}

	void Game::CreateScene()
	{
		m_renderWindow.removeAllViewports();
		m_pSceneMgr = m_ogreRoot.createSceneManager(Ogre::ST_GENERIC, "GameScene");
		m_pSceneMgr->setAmbientLight(Ogre::ColourValue::White);
		m_pSceneMgr->addRenderQueueListener(&m_overlaySystem);

		// Light
		m_pLight = m_pSceneMgr->createLight();
		m_pLight->setType(Ogre::Light::LT_DIRECTIONAL);
		m_pLight->setDirection(1.0f, 0.0f, 0.0f);

		// Space camera
		m_pCamera = new SpaceCamera(*this, Ogre::Vector3(0, 0, -5));
		GetCamera().setFOVy(FieldOfView);
		GetCamera().setNearClipDistance(NearPlane);
		GetCamera().setFarClipDistance(FarPlane);
		GetCamera().setPosition(Ogre::Vector3(0, 0, 0));

		m_pViewport = m_renderWindow.addViewport(&GetCamera());
		m_pViewport->setBackgroundColour(Ogre::ColourValue::Black);

		GetCamera().setAspectRatio(Ogre::Real(m_pViewport->getActualWidth()) / Ogre::Real(m_pViewport->getActualHeight()));
		m_pViewport->setCamera(&GetCamera());

		// Initialize shader generator.
		// Must be before resource loading in order to allow parsing extended material attributes.
		bool success = InitializeRTShaderSystem();
		if (!success)
		{
			OGRE_EXCEPT(Ogre::Exception::ERR_FILE_NOT_FOUND,
				"Shader Generator Initialization failed - Core shader libs path not found",
				"SampleBrowser::createDummyScene");
		}

		if (m_ogreRoot.getRenderSystem()->getCapabilities()->hasCapability(Ogre::RSC_FIXED_FUNCTION) == false)
		{
			//newViewport->setMaterialScheme(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);

			// creates shaders for base material BaseWhite using the RTSS
			Ogre::MaterialPtr baseWhite = Ogre::MaterialManager::getSingleton().getByName("BaseWhite", Ogre::ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
			baseWhite->setLightingEnabled(false);
			m_pShaderGenerator->createShaderBasedTechnique(
				"BaseWhite",
				Ogre::MaterialManager::DEFAULT_SCHEME_NAME,
				Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
			m_pShaderGenerator->validateMaterial(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME,
				"BaseWhite");
			if(baseWhite->getNumTechniques() > 1)
			{
				baseWhite->getTechnique(0)->getPass(0)->setVertexProgram(
				baseWhite->getTechnique(1)->getPass(0)->getVertexProgram()->getName());
				baseWhite->getTechnique(0)->getPass(0)->setFragmentProgram(
				baseWhite->getTechnique(1)->getPass(0)->getFragmentProgram()->getName());
			}

			// creates shaders for base material BaseWhiteNoLighting using the RTSS
			m_pShaderGenerator->createShaderBasedTechnique(
				"BaseWhiteNoLighting",
				Ogre::MaterialManager::DEFAULT_SCHEME_NAME,
				Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);
			m_pShaderGenerator->validateMaterial(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME,
				"BaseWhiteNoLighting");
			Ogre::MaterialPtr baseWhiteNoLighting = Ogre::MaterialManager::getSingleton().getByName("BaseWhiteNoLighting", Ogre::ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
			if(baseWhite->getNumTechniques() > 1)
			{
				baseWhiteNoLighting->getTechnique(0)->getPass(0)->setVertexProgram(
				baseWhiteNoLighting->getTechnique(1)->getPass(0)->getVertexProgram()->getName());
				baseWhiteNoLighting->getTechnique(0)->getPass(0)->setFragmentProgram(
				baseWhiteNoLighting->getTechnique(1)->getPass(0)->getFragmentProgram()->getName());
			}
		}
	}

	void Game::DestroyScene()
	{
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
			m_pShaderGenerator->removeSceneManager(m_pSceneMgr);
			m_pSceneMgr->removeRenderQueueListener(&m_overlaySystem);
			m_renderWindow.removeAllViewports();
			m_ogreRoot.destroySceneManager(m_pSceneMgr);
			m_pSceneMgr = NULL;
		}
	}

	bool Game::InitializeRTShaderSystem()
	{
		if (Ogre::RTShader::ShaderGenerator::initialize())
		{
			m_pShaderGenerator = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
			assert(m_pShaderGenerator != NULL);

			m_pShaderGenerator->addSceneManager(m_pSceneMgr);

			// Create and register the material manager listener if it doesn't exist yet.
			if (m_pMaterialMgrListener == NULL)
			{
				m_pMaterialMgrListener = new ShaderGeneratorTechniqueResolverListener(m_pShaderGenerator);
				Ogre::MaterialManager::getSingleton().addListener(m_pMaterialMgrListener);
			}
		}

		return true;
	}

	void Game::FinalizeRTShaderSystem()
	{
		// Restore default scheme.
		Ogre::MaterialManager::getSingleton().setActiveScheme(Ogre::MaterialManager::DEFAULT_SCHEME_NAME);

		// Unregister the material manager listener.
		if (m_pMaterialMgrListener != NULL)
		{
			Ogre::MaterialManager::getSingleton().removeListener(m_pMaterialMgrListener);
			delete m_pMaterialMgrListener;
			m_pMaterialMgrListener = NULL;
		}

		// Finalize RTShader system.
		if (m_pShaderGenerator != NULL)
		{
			Ogre::RTShader::ShaderGenerator::finalize();
			m_pShaderGenerator = NULL;
		}
	}

	void Game::Update(double timeSinceLastFrame)
	{
		using namespace Ogre;

		GetSpaceCamera().Update(timeSinceLastFrame);

		// Render rocket in front of view in menu mode
		if (GetSpaceCamera().IsInGame() == false)
		{
			 Vector3 inFrontOfCameraPos = Vector3(0, -1.33f, -2.5f);
			 inFrontOfCameraPos = GetCamera().getViewMatrix().inverse() * inFrontOfCameraPos;
			 m_pRocket->SetPosition(inFrontOfCameraPos);

			 const Quaternion rocketOrient = GetCamera().getOrientation() *
			 Quaternion(Radian(GetTotalTimeMs() / 8400.0f), Vector3::UNIT_Z) *
			 Quaternion(-Radian(Math::PI / 2.2f), Vector3::UNIT_X);
			 m_pRocket->SetOrientation(rocketOrient);
		 }

		// If that game screen should be quitted, remove it from stack
		if (!m_gameScreens.empty() && m_gameScreens.top()->GetQuit())
			ExitCurrentGameScreen();

		// If no more game screens are left, it is time to quit
		if (m_gameScreens.empty())
			ExitGame();

		try
		{
			// Execute the game screen on top
			if (!m_gameScreens.empty())
				m_gameScreens.top()->Run();
		}
		catch (std::exception &e)
		{
			likeleon::Log::error("Failed to execute %s\nError: %s",	m_gameScreens.top()->GetName(), e.what());
		}

		m_elapsedTimeThisFrameInMs = (float) timeSinceLastFrame;
		m_totalTimeMs += m_elapsedTimeThisFrameInMs;

		// Make sure m_elapsedTimeThisFrameInMs is never 0
		if (m_elapsedTimeThisFrameInMs <= 0)
			m_elapsedTimeThisFrameInMs = 0.001f;
	}

	void Game::Cleanup()
	{
		likeleon::Log::info("Cleanup game");

		while (!m_gameScreens.empty())
		{
			delete m_gameScreens.top();
			m_gameScreens.pop();
		}

		if (m_pRocket)
		{
			delete m_pRocket;
			m_pRocket = NULL;
		}

		if (m_pTrayMgr)
		{
			delete m_pTrayMgr;
			m_pTrayMgr = NULL;
		}

		DestroyScene();

		FinalizeRTShaderSystem();
	}

	bool Game::frameStarted(const Ogre::FrameEvent& evt)
	{
		return true;
	}

	bool Game::frameRenderingQueued(const Ogre::FrameEvent& evt)
	{
		m_pTrayMgr->frameRenderingQueued(evt);
		Update(evt.timeSinceLastFrame * 1000.0);
		return true;
	}

	bool Game::frameEnded(const Ogre::FrameEvent& evt)
	{
		return !(m_renderWindow.isClosed());
	}

	Ogre::SceneManager& Game::GetSceneManager()
	{
		assert(m_pSceneMgr);
		return *m_pSceneMgr;
	}

	float Game::GetMoveFactorPerSecond() const
	{
		return m_elapsedTimeThisFrameInMs / 1000.0f;
	}

	float Game::GetTotalTimeMs() const
	{
		return m_totalTimeMs;
	}

	SpaceCamera& Game::GetSpaceCamera()
	{
		assert(m_pCamera);
		return *m_pCamera;
	}

	Ogre::Camera& Game::GetCamera()
	{
		return GetSpaceCamera().GetCamera();
	}

	Rocket& Game::GetRocket()
	{
		assert(m_pRocket);
		return *m_pRocket;
	}

	void Game::AddGameScreen(IGameScreen *gameScreen)
	{
		assert(gameScreen);

		if (!m_gameScreens.empty())
		{
			m_gameScreens.top()->Exit();
		}
		EnterGameScreen(gameScreen, true);
	}

	void Game::ChangeGameScreen(IGameScreen *gameScreen)
	{
		assert(gameScreen);
		assert(!m_gameScreens.empty());

		RemoveCurrentGameScreen();
		EnterGameScreen(gameScreen, true);
	}

	void Game::EnterGameScreen(IGameScreen *gameScreen, bool push)
	{
		assert(gameScreen);

		likeleon::Log::info("Entering game screen %s", gameScreen->GetName());

		m_inGame = gameScreen->IsInGame();

		// Space camera
		GetSpaceCamera().SetInGame(m_inGame);

		// Enter game screen
		gameScreen->Enter();

		if (push)
			m_gameScreens.push(gameScreen);
	}

	void Game::ExitCurrentGameScreen()
	{
		RemoveCurrentGameScreen();

		if (!m_gameScreens.empty())
			EnterGameScreen(m_gameScreens.top(), false);
	}

	void Game::RemoveCurrentGameScreen()
	{
		assert(!m_gameScreens.empty());

		likeleon::Log::info("Removing game screen %s",	m_gameScreens.top()->GetName());

		m_gameScreens.top()->Exit();
		delete m_gameScreens.top();
		m_gameScreens.pop();
	}

	void Game::ExitGame()
	{
		m_ogreRoot.queueEndRendering();
	}

	void Game::LoadResources()
	{
		m_pTrayMgr->showLoadingBar(1, 0);
		Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("GameResource");
		m_pTrayMgr->hideLoadingBar();
	}
}
