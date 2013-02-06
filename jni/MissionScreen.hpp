#ifndef _MISSION_SCREEN_HPP_
#define _MISSION_SCREEN_HPP_

#include "IGameScreen.hpp"

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
		Game& m_game;
		bool m_quit;
	};
}

#endif
