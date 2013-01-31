#ifndef _GAME_HPP_
#define _GAME_HPP_

#include <Ogre.h>
#include <OgreOverlaySystem.h>
#include <stack>
#include "InputContext.h"
#include <OgreRTShaderSystem.h>

namespace OgreBites
{
	class SdkTrayManager;
}

namespace rcd
{
	class IGameScreen;
	class SpaceCamera;
	class Rocket;
	class ShaderGeneratorTechniqueResolverListener;

	class Game : public Ogre::FrameListener
	{
	public:
		Game(Ogre::Root& ogreRoot, Ogre::RenderWindow& renderWindow, Ogre::OverlaySystem& overlaySystem);
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
		void LoadResources();
		void CreateScene();
		void DestroyScene();
		bool InitializeRTShaderSystem();
		void FinalizeRTShaderSystem();

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
		Ogre::OverlaySystem& m_overlaySystem;
		Ogre::SceneManager* m_pSceneMgr;
		Ogre::Light* m_pLight;
		SpaceCamera* m_pCamera;
		Ogre::Viewport* m_pViewport;
		OgreBites::SdkTrayManager* m_pTrayMgr;
		OgreBites::InputContext m_inputContext;
		Ogre::RTShader::ShaderGenerator* m_pShaderGenerator; // the Shader generator instance.
		ShaderGeneratorTechniqueResolverListener* m_pMaterialMgrListener; // shader generator material manager listener.

		float m_elapsedTimeThisFrameInMs;
		float m_totalTimeMs;

		std::stack<IGameScreen *> m_gameScreens;
		bool m_inGame;

		Rocket* m_pRocket;
	};
}

#endif
