#ifndef _GAME_HPP_
#define _GAME_HPP_

#include <Ogre.h>
#include <OgreOverlaySystem.h>
#include <stack>
#include "InputContext.h"
#include <OgreRTShaderSystem.h>
#include <vector>
#include "Level.hpp"
#include "AndroidInputInjector.hpp"

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
	class GameAsteroidManager;
	class Player;
	class Sprite;
	class LensFlare;
	class Sound;

	class Game : public Ogre::FrameListener
	{
	public:
		Game(Ogre::Root& ogreRoot, Ogre::RenderWindow& renderWindow, Ogre::OverlaySystem& overlaySystem, AAssetManager& assetManager, AndroidInputInjector& inputInjector);
		~Game();

		void Initialize();
		void Cleanup();

		Ogre::SceneManager& GetSceneManager();
		SpaceCamera& GetSpaceCamera();
		Ogre::Camera& GetCamera();
		float GetMoveFactorPerSecond() const;
		float GetTotalTimeMs() const;
		Rocket& GetRocket();
		const Level& GetLevel(int index) const;
		GameAsteroidManager& GetAsteroidManager();
		int GetTotalFrames() const;
		float GetElapsedTimeThisFrameInMs() const;
		Player& GetPlayer();
		Sprite& GetSprite();
		OIS::MultiTouch& GetMultiTouch();
		Sound& GetSound();

		void SetLensFlareColour(const Ogre::ColourValue &colour) { m_remLensFlareColor = colour; }
		void SetLightDirection(const Ogre::Vector3 &lightDir);
		void SetLightColour(const Ogre::ColourValue &lightColour);

		// Viewport size
		int GetWidth() const { return m_pViewport->getActualWidth(); }
		int GetHeight() const { return m_pViewport->getActualHeight(); }

		Ogre::Vector2 Convert3DPointTo2D(const Ogre::Vector3 &point);
		bool IsInFrontOfCamera(const Ogre::Vector3 &point);

		void AddGameScreen(IGameScreen *gameScreen);
		void ChangeGameScreen(IGameScreen *gameScreen);
		void ExitCurrentGameScreen();

		void EnableCompositor(const Ogre::String& compositorName);
		void DisableCompositor(const Ogre::String& compositorName);

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
		AAssetManager& m_assetManager;
		AndroidInputInjector& m_inputInjector;
		Ogre::SceneManager* m_pSceneMgr;
		Ogre::Light* m_pLight;
		SpaceCamera* m_pCamera;
		Ogre::Viewport* m_pViewport;
		OgreBites::SdkTrayManager* m_pTrayMgr;
		OgreBites::InputContext m_inputContext;
		Ogre::RTShader::ShaderGenerator* m_pShaderGenerator; // the Shader generator instance.
		ShaderGeneratorTechniqueResolverListener* m_pMaterialMgrListener; // shader generator material manager listener.
		Sprite* m_pSprite;
		LensFlare* m_pLensFlare;
		Ogre::ColourValue m_remLensFlareColor;
		Sound* m_pSound;

		float m_elapsedTimeThisFrameInMs;
		float m_totalTimeMs;
		int m_totalFrameCount;

		std::stack<IGameScreen *> m_gameScreens;
		bool m_inGame;

		Rocket* m_pRocket;
		std::vector<Level> m_levels;
		GameAsteroidManager* m_pAsteroidManager;
		Player* m_pPlayer;
	};
}

#endif
