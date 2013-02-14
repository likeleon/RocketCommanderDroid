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
#include "Player.hpp"
#include "Sprite.hpp"
#include "LensFlare.hpp"
#include "RadialBlurListener.hpp"
#include "Sound.hpp"

namespace rcd
{
	const Ogre::Radian Game::FieldOfView = Ogre::Radian(Ogre::Math::PI / 1.8f);
	const Ogre::Real Game::NearPlane = GameAsteroidManager::GetMinViewDepth();
	const Ogre::Real Game::FarPlane = GameAsteroidManager::GetMaxViewDepth();

	Game::Game(Ogre::Root& ogreRoot, Ogre::RenderWindow& renderWindow, Ogre::OverlaySystem& overlaySystem, AAssetManager& assetManager, AndroidInputInjector& inputInjector) :
		m_ogreRoot(ogreRoot), m_renderWindow(renderWindow), m_pSceneMgr(NULL), m_overlaySystem(overlaySystem), m_assetManager(assetManager), m_inputInjector(inputInjector)
		, m_pCamera(NULL), m_pViewport(NULL), m_pTrayMgr(NULL), m_pShaderGenerator(NULL), m_pMaterialMgrListener(NULL)
		, m_elapsedTimeThisFrameInMs(0.001f), m_totalTimeMs(0.0f), m_totalFrameCount(0), m_inGame(false), m_pSprite(NULL)
		, m_pLensFlare(NULL), m_pSound(NULL), m_remLensFlareColor(Ogre::ColourValue::White), m_pRocket(NULL), m_pAsteroidManager(NULL), m_pPlayer(NULL)
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
		m_pTrayMgr->showTrays();
		m_pTrayMgr->hideCursor();
		m_pTrayMgr->showFrameStats(OgreBites::TL_TOPLEFT);

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

		// Player
		m_pPlayer = new Player(*this);

		// Sprite
		m_pSceneMgr->getRenderQueue()->getQueueGroup(Ogre::RENDER_QUEUE_MAIN + 1)->setShadowsEnabled(false);
		m_pSprite = new Sprite();
		m_pSprite->Init(m_pSceneMgr, m_pViewport, Ogre::RENDER_QUEUE_MAIN + 1, true);
		m_pSprite->SetSpriteLocation("/sprites");

		// Lens flare
		m_pLensFlare = new LensFlare(*this, LensFlare::DefaultSunPos);

		//
		// Compositors
		//
		Ogre::CompositorManager &compMgr = Ogre::CompositorManager::getSingleton();

		// Radial blur
		Ogre::CompositorInstance *radialBlur = compMgr.addCompositor(m_pViewport, "Radial Blur");
		radialBlur->addListener(new RadialBlurListener(*m_pPlayer));

		m_pSound = new Sound(m_assetManager);

		// Skybox
		m_pSceneMgr->setSkyBox(true, "RocketCommander/SpaceSkyBox", GetCamera().getFarClipDistance() * 0.5f, true);

		// Load all available levels
		m_levels = Level::LoadAllLevels(m_assetManager);

		// Initialize asteroidmanager and use last available level.
		m_pAsteroidManager = new GameAsteroidManager(*this, m_levels[0]);
		GetPlayer().SetAsteroidManager(m_pAsteroidManager);

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

		m_pAsteroidManager->Update();

		m_pLensFlare->Render(m_remLensFlareColor);

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

		// Increase frame counter
		m_totalFrameCount++;

		// Update sound engine
		m_pSound->Update();
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

		if (m_pAsteroidManager)
		{
			delete m_pAsteroidManager;
			m_pAsteroidManager = NULL;
		}

		if (m_pTrayMgr)
		{
			delete m_pTrayMgr;
			m_pTrayMgr = NULL;
		}

		if (m_pSound)
		{
			delete m_pSound;
			m_pSound = NULL;
		}

		Ogre::CompositorManager *compMgr = Ogre::CompositorManager::getSingletonPtr();
		if (compMgr && compMgr->hasCompositorChain(m_pViewport))
		{
			compMgr->removeCompositorChain(m_pViewport);
		}

		if (m_pLensFlare)
		{
			delete m_pLensFlare;
			m_pLensFlare = NULL;
		}

		if (m_pSprite)
		{
			m_pSprite->Shutdown();
			delete m_pSprite;
			m_pSprite = NULL;
		}

		if (m_pPlayer)
		{
			delete m_pPlayer;
			m_pPlayer = NULL;
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


		// Sound
		GetSound().SetCurrentMusicMode(m_inGame);

		// OIS
		m_inputInjector.SetGameScreen(gameScreen);

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

		m_inputInjector.SetGameScreen(NULL);

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

	const Level& Game::GetLevel(int index) const
	{
		return m_levels[index];
	}

	GameAsteroidManager& Game::GetAsteroidManager()
	{
		assert(m_pAsteroidManager);
		return *m_pAsteroidManager;
	}

	void Game::SetLightDirection(const Ogre::Vector3 &lightDir)
	{
		assert(m_pLight);
		m_pLight->setDirection(lightDir.normalisedCopy());
	}

	void Game::SetLightColour(const Ogre::ColourValue &lightColour)
	{
		assert(m_pLight);
		m_pLight->setDiffuseColour(lightColour);
		m_pLight->setSpecularColour(lightColour);
	}

	int Game::GetTotalFrames() const
	{
		return m_totalFrameCount;
	}

	float Game::GetElapsedTimeThisFrameInMs() const
	{
		return m_elapsedTimeThisFrameInMs;
	}

	Player& Game::GetPlayer()
	{
		assert(m_pPlayer);
		return *m_pPlayer;
	}

	Ogre::Vector2 Game::Convert3DPointTo2D(const Ogre::Vector3 &point)
	{
		const Ogre::Vector3 worldView = GetCamera().getViewMatrix() * point;

		// Homogeneous clip space, between -1, 1 is in frusttrum
		const Ogre::Vector3 hcsPos = GetCamera().getProjectionMatrix() * worldView;

		const float halfWidth = (float)GetWidth() / 2;
		const float halfHeight = (float)GetHeight() / 2;
		return Ogre::Vector2(halfWidth + halfWidth * hcsPos.x,
			halfHeight + halfHeight * (-hcsPos.y));
	}

	bool Game::IsInFrontOfCamera(const Ogre::Vector3 &point)
	{
		// Not work, why?
		const Ogre::Matrix4 viewProjMatrix = GetCamera().getViewMatrix() * GetCamera().getProjectionMatrix();
		Ogre::Vector4 result = Ogre::Vector4(point.x, point.y, point.z, 1) * viewProjMatrix;

		// Is result in front?
		return result.z > result.w - GetCamera().getNearClipDistance();

		/*const Ogre::Vector3 eyeSpacePos = GetCamera().getViewMatrix() * point;
		if (eyeSpacePos.z >= 0)
			return false;

		const Ogre::Vector3 hcsPos = GetCamera().getProjectionMatrix() * eyeSpacePos;
		if ((hcsPos.x < -1.0f) || (hcsPos.x > 1.0f) || (hcsPos.y < -1.0f) || (hcsPos.y > 1.0f))
			return false;

		return true;*/
	}

	Sprite& Game::GetSprite()
	{
		assert(m_pSprite);
		return *m_pSprite;
	}

	OIS::MultiTouch& Game::GetMultiTouch()
	{
		return m_inputInjector.GetMultiTouch();
	}

	void Game::EnableCompositor(const Ogre::String& compositorName)
	{
		Ogre::CompositorManager::getSingleton().setCompositorEnabled(m_pViewport, compositorName, true);
	}

	void Game::DisableCompositor(const Ogre::String& compositorName)
	{
		Ogre::CompositorManager::getSingleton().setCompositorEnabled(m_pViewport, compositorName, false);
	}

	Sound& Game::GetSound()
	{
		assert(m_pSound);
		return *m_pSound;
	}
}
