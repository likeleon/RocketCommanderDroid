#include "MissionScreen.hpp"
#include "Game.hpp"
#include "Level.hpp"
#include "GameAsteroidManager.hpp"
#include "Player.hpp"

namespace rcd
{
	MissionScreen::MissionScreen(Game& game, const Level& level)
	: m_game(game), m_quit(false)
	{
		// Set level for asteroid manager
		m_game.GetAsteroidManager().SetLevel(level);

		// Start new game
		m_game.GetPlayer().Reset(level.GetName());
	}

	void MissionScreen::Enter()
	{
	}

	void MissionScreen::Exit()
	{
	}

	void MissionScreen::Run()
	{
	}

	bool MissionScreen::touchMoved(const OIS::MultiTouchEvent &arg)
	{
		return true;
	}

	bool MissionScreen::touchPressed(const OIS::MultiTouchEvent &arg)
	{
		m_quit = true;
	}

	bool MissionScreen::touchReleased(const OIS::MultiTouchEvent &arg)
	{
		return true;
	}

	bool MissionScreen::touchCancelled(const OIS::MultiTouchEvent &arg)
	{
		return true;
	}
}
