#include "MainMenuScreen.hpp"
#include "Game.hpp"
#include "SpaceCamera.hpp"

namespace rcd
{
	MainMenuScreen::MainMenuScreen(Game &game)
	: m_game(game), m_resetCamera(true)
	{
	}

	void MainMenuScreen::Enter()
	{
		if (m_resetCamera)
		{
			m_game.GetCamera().setPosition(Ogre::Vector3::ZERO);
			m_game.GetSpaceCamera().SetInGame(false);
			m_resetCamera = false;
		}
	}
}
