#ifndef _GAME_HPP_
#define _GAME_HPP_

#include <Ogre.h>
#include <stack>

namespace rcd
{
	class IGameScreen;
	class SpaceCamera;
	class Rocket;

	class Game : public Ogre::FrameListener
	{
	public:
		Game(Ogre::Root& ogreRoot, Ogre::RenderWindow& renderWindow);
		~Game();

		void Initialize();
		void Cleanup();

		Ogre::SceneManager& GetSceneManager();
		SpaceCamera& GetSpaceCamera();
		Ogre::Camera& GetCamera();
		float GetMoveFactorPerSecond() const;
		float GetTotalTimeMs() const;
		Rocket& GetRocket();

		void AddGameScreen(IGameScreen *gameScreen);
		void ChangeGameScreen(IGameScreen *gameScreen);
		void ExitCurrentGameScreen();

		static const Ogre::Radian FieldOfView;
		static const Ogre::Real NearPlane;
		static const Ogre::Real FarPlane;

	private:
		void Update(double timeSinceLastFrame);
		void EnterGameScreen(IGameScreen *gameScreen, bool push);
		void RemoveCurrentGameScreen();
		void ExitGame();

		// Overrides Ogre::FrameListener
		virtual bool frameStarted(const Ogre::FrameEvent& evt);
		virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
		virtual bool frameEnded(const Ogre::FrameEvent& evt);

		Ogre::Root& m_ogreRoot;
		Ogre::RenderWindow& m_renderWindow;
		Ogre::SceneManager* m_pSceneMgr;
		Ogre::Light* m_pLight;
		SpaceCamera* m_pCamera;
		Ogre::Viewport* m_pViewport;

		float m_elapsedTimeThisFrameInMs;
		float m_totalTimeMs;

		std::stack<IGameScreen *> m_gameScreens;
		bool m_inGame;

		Rocket* m_pRocket;
	};
}

#endif
