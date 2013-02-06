#ifndef _MAIN_MENU_SCREEN_HPP_
#define _MAIN_MENU_SCREEN_HPP_

#include "IGameScreen.hpp"

namespace Ogre
{
	class Rectangle2D;
	class SceneNode;
	class Overlay;
}

namespace rcd
{
	class Game;

	class MainMenuScreen : public IGameScreen
	{
	public:
		MainMenuScreen(Game &game);

		// Overrides GameScreen
		virtual const char* GetName() const { return "Main Menu"; }
		virtual void Enter();
		virtual bool GetQuit() const { return false; }
		virtual void Exit();
		virtual void Run() {}
		virtual bool IsInGame() const { return false; }

		virtual bool touchMoved(const OIS::MultiTouchEvent &arg);
		virtual bool touchPressed(const OIS::MultiTouchEvent &arg);
		virtual bool touchReleased(const OIS::MultiTouchEvent &arg);
		virtual bool touchCancelled(const OIS::MultiTouchEvent &arg);

	private:
		Game& m_game;
		bool m_resetCamera;
		Ogre::Rectangle2D* m_pBackground;
		Ogre::SceneNode* m_pBackgroundNode;
		Ogre::Overlay* m_pOverlay;
	};
}

#endif
