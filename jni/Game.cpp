#include "Game.hpp"
#include "Log.hpp"
#include "GameAsteroidManager.hpp"
#include "IGameScreen.hpp"
#include "MainMenuScreen.hpp"
#include "SpaceCamera.hpp"
#include "Rocket.hpp"

namespace rcd
{
	const Ogre::Radian Game::FieldOfView = Ogre::Radian(Ogre::Math::PI / 1.8f);
	const Ogre::Real Game::NearPlane = GameAsteroidManager::GetMinViewDepth();
	const Ogre::Real Game::FarPlane = GameAsteroidManager::GetMaxViewDepth();

	Game::Game(Ogre::Root& ogreRoot, Ogre::RenderWindow& renderWindow)
	: m_ogreRoot(ogreRoot), m_renderWindow(renderWindow), m_pSceneMgr(NULL)
	, m_pCamera(NULL), m_pViewport(NULL)
	, m_elapsedTimeThisFrameInMs(0.001f), m_totalTimeMs(0.0f), m_inGame(false)
	, m_pRocket(NULL)
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

		// Skybox
		m_pSceneMgr->setSkyBox(true, "RocketCommander/SpaceSkyBox", GetCamera().getFarClipDistance() * 0.5f, true);

		// Rocket
		m_pRocket = new Rocket(*m_pSceneMgr);

		// Main menu screen
		AddGameScreen(new MainMenuScreen(*this));

		m_ogreRoot.addFrameListener(this);
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
		if (!m_gameScreens.empty() &&
			m_gameScreens.top()->GetQuit())
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
			likeleon::Log::error("Failed to execute %s\nError: %s", m_gameScreens.top()->GetName(), e.what());
		}

		m_elapsedTimeThisFrameInMs = (float)timeSinceLastFrame;
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

		likeleon::Log::info("Removing game screen %s", m_gameScreens.top()->GetName());

		m_gameScreens.top()->Exit();
		delete m_gameScreens.top();
		m_gameScreens.pop();
	}

	void Game::ExitGame()
	{
		m_ogreRoot.queueEndRendering();
	}
}
