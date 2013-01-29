#ifndef _MAIN_MENU_SCREEN_HPP_
#define _MAIN_MENU_SCREEN_HPP_

#include "IGameScreen.hpp"

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
		virtual void Exit() {}
		virtual void Run() {}
		virtual bool IsInGame() const { return false; }

	private:
		Game& m_game;
		bool m_resetCamera;
	};
}

#endif
