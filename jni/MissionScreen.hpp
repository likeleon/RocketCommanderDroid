#ifndef _MISSION_SCREEN_HPP_
#define _MISSION_SCREEN_HPP_

#include "IGameScreen.hpp"
#include <OgreOverlay.h>
#include <OgreTextAreaOverlayElement.h>

namespace rcd
{
	class Game;
	class Level;

	class MissionScreen: public IGameScreen
	{
	public:
		MissionScreen(Game& game, const Level& level);

		// Overrides GameScreen
		virtual const char* GetName() const { return "Mission"; }
		virtual void Enter();
		virtual bool GetQuit() const { return m_quit; }
		virtual void Exit();
		virtual void Run();
		virtual bool IsInGame() const { return true; }

		virtual bool touchMoved(const OIS::MultiTouchEvent &arg);
		virtual bool touchPressed(const OIS::MultiTouchEvent &arg);
		virtual bool touchReleased(const OIS::MultiTouchEvent &arg);
		virtual bool touchCancelled(const OIS::MultiTouchEvent &arg);

	private:
		void ZoomIntoRocket();
		void ShowHudContorls();
		void ShowScreenMessages();
		void ShowTarget(int levelLength);

		Game& m_game;
		bool m_quit;
		float m_rocketEndGameScale;

		Ogre::Overlay* m_pOverlay;
		Ogre::TextAreaOverlayElement* m_pScoreText;
		Ogre::TextAreaOverlayElement* m_pLifeText;
		Ogre::TextAreaOverlayElement* m_pMessageText;
		Ogre::TextAreaOverlayElement* m_pFuelText;
		Ogre::TextAreaOverlayElement* m_pHealthText;
		Ogre::TextAreaOverlayElement* m_pSpeedText;
		Ogre::TextAreaOverlayElement* m_pPositionText;
		Ogre::TextAreaOverlayElement* m_pRankText;
		Ogre::TextAreaOverlayElement* m_pTimeText;
		Ogre::TextAreaOverlayElement* m_pSensorText;
	};
}

#endif
