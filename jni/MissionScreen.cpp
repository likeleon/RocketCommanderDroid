#include "MissionScreen.hpp"
#include "Game.hpp"
#include "Level.hpp"
#include "GameAsteroidManager.hpp"
#include "Player.hpp"
#include "Rocket.hpp"
#include "SpaceCamera.hpp"
#include "Texts.hpp"

using namespace Ogre;

namespace rcd
{
	MissionScreen::MissionScreen(Game& game, const Level& level)
	: m_game(game), m_quit(false), m_pOverlay(NULL)
	, m_pScoreText(NULL), m_pLifeText(NULL), m_pMessageText(NULL), m_pFuelText(NULL)
	, m_pHealthText(NULL), m_pSpeedText(NULL), m_pPositionText(NULL), m_pRankText(NULL), m_pTimeText(NULL)
	{
		// Set level for asteroid manager
		m_game.GetAsteroidManager().SetLevel(level);

		// Start new game
		m_game.GetPlayer().Reset(level.GetName());
	}

	void MissionScreen::Enter()
	{
		m_game.GetCamera().setFOVy(Game::InGameFieldOfView);

		//m_game.EnableCompositor("Radial Blur");

		Ogre::OverlayManager& overlayManager = Ogre::OverlayManager::getSingleton();
		m_pOverlay = overlayManager.getByName("MissionOverlay");
		assert(m_pOverlay != NULL);
		m_pOverlay->show();

		Ogre::PanelOverlayElement* pPanel = (Ogre::PanelOverlayElement*)m_pOverlay->getChild("Mission/Panel");
		assert(pPanel != NULL);

		m_pScoreText = (Ogre::TextAreaOverlayElement *)pPanel->getChild("Mission/Score");
		m_pLifeText = (Ogre::TextAreaOverlayElement *)pPanel->getChild("Mission/Life");
		m_pMessageText = (Ogre::TextAreaOverlayElement *)pPanel->getChild("Mission/Message");
		m_pFuelText = (Ogre::TextAreaOverlayElement *)pPanel->getChild("Mission/Fuel");
		m_pHealthText = (Ogre::TextAreaOverlayElement *)pPanel->getChild("Mission/Health");
		m_pSpeedText = (Ogre::TextAreaOverlayElement *)pPanel->getChild("Mission/Speed");
		m_pPositionText = (Ogre::TextAreaOverlayElement *)pPanel->getChild("Mission/Position");
		m_pRankText = (Ogre::TextAreaOverlayElement *)pPanel->getChild("Mission/Rank");
		m_pTimeText = (Ogre::TextAreaOverlayElement *)pPanel->getChild("Mission/Time");
	}

	void MissionScreen::Exit()
	{
		m_pOverlay->hide();

		//m_game.DisableCompositor("Radial Blur");
	}

	void MissionScreen::Run()
	{
		// Zoom into rocket for countdown time
		ZoomIntoRocket();

		// Show target in z direction
		ShowTarget(m_game.GetAsteroidManager().GetLevel().GetLength());

		// Show all hud controls
		ShowHudContorls();

		// Handle game stuff
		Player &player = m_game.GetPlayer();
		player.HandleGameLogic();

		// Show on screen helper messages
		ShowScreenMessages();
	}

	bool MissionScreen::touchMoved(const OIS::MultiTouchEvent &arg)
	{
		return true;
	}

	bool MissionScreen::touchPressed(const OIS::MultiTouchEvent &arg)
	{
		Player &player = m_game.GetPlayer();

		if (m_game.GetPlayer().IsGameOver())
		{
			player.SetGameOverAndUploadHighscore();

			// TOOD: m_game.GetSound().StopRocketMotorSound();

			m_game.GetCamera().setPosition(Vector3::ZERO);
			m_game.GetSpaceCamera().SetInGame(false);

			m_quit = true;
		}
		else if (player.GetExplosionTimeoutMs() >= 0)
		{
			player.SetExplosionTimeoutMs(-1);
			m_game.GetCamera().setFOVy(Game::InGameFieldOfView);
		}
		return true;
	}

	bool MissionScreen::touchReleased(const OIS::MultiTouchEvent &arg)
	{
		return true;
	}

	bool MissionScreen::touchCancelled(const OIS::MultiTouchEvent &arg)
	{
		return true;
	}

	void MissionScreen::ZoomIntoRocket()
	{
		const Player &player = m_game.GetPlayer();
		if (!player.IsGameOver())
			return;

		// Only start showing rocket after all explosions are nearly over
		if (player.GetExplosionTimeoutMs() < 400 &&
			player.GetExplosionTimeoutMs2() < 400 &&
			player.GetExplosionTimeoutMs3() < 400)
		{
			Rocket &rocket = m_game.GetRocket();

			// Scale in rocket
			m_rocketEndGameScale += m_game.GetMoveFactorPerSecond() * 5.0f;
			m_rocketEndGameScale = std::min(m_rocketEndGameScale, 10.0f);
			rocket.SetScale(Vector3(m_rocketEndGameScale, m_rocketEndGameScale, m_rocketEndGameScale));

			// Show rocket in middle of screen
			Vector3 inFrontOfCameraPos = Vector3(0, -0.3f, -1.75f) * 10.0f;
			inFrontOfCameraPos = m_game.GetCamera().getViewMatrix().inverse() * inFrontOfCameraPos;
			rocket.SetPosition(inFrontOfCameraPos);

			const Quaternion &rocketOrient = Quaternion(Radian(Math::PI / 4.0f), Vector3::UNIT_Z) *
				Quaternion(Radian(-Math::PI), Vector3::UNIT_X) *
				Quaternion(Radian(m_game.GetTotalTimeMs() / 2293.0f), Vector3::UNIT_Y) *
				Quaternion(Radian(Math::PI), Vector3::UNIT_X);
			rocket.SetOrientation(rocketOrient);
		}
		else
		{
			m_rocketEndGameScale = 0.0f;
		}
	}

	void MissionScreen::ShowHudContorls()
	{
		const Player &player = m_game.GetPlayer();

		// Fuel, health and speed
		assert(m_pFuelText && m_pHealthText && m_pSpeedText && m_pLifeText);
		m_pFuelText->setCaption("Fuel: " + StringConverter::toString((int)(player.GetFuel() * 100)) + "%");
		m_pHealthText->setCaption("Health: " + StringConverter::toString((int)(player.GetHealth() * 100)) + "%");
		m_pSpeedText->setCaption("Speed: " + StringConverter::toString((int)(player.GetSpeed() * 100)));
		m_pLifeText->setCaption("Life: " + StringConverter::toString(player.GetLifes()));

		// Show level position, score, rank ang game time
		assert(m_pPositionText && m_pRankText && m_pScoreText && m_pTimeText);
		int pos = static_cast<int>(101 * m_game.GetCamera().getPosition().z /
			(m_game.GetAsteroidManager().GetLevel().GetLength() * GameAsteroidManager::SectorDepth));
		pos = std::min(100, std::max(0, pos));
		m_pPositionText->setCaption("Pos: " + StringConverter::toString(pos) + "%");

		m_pRankText->setCaption("Rank: #1");

		m_pScoreText->setCaption("Score: " + StringConverter::toString(player.GetScore()));

		m_pTimeText->setCaption("Time: " +
			StringConverter::toString(static_cast<int>((player.GetGameTimeMs() / 1000) / 60), 2, '0') + ":" +
			StringConverter::toString(static_cast<int>(player.GetGameTimeMs() / 1000) % 60, 2, '0'));
	}

	void MissionScreen::ShowScreenMessages()
	{
		using namespace OIS;

		assert(m_pMessageText);
		m_pMessageText->setCaption("");
		m_pMessageText->setColour(Ogre::ColourValue::White);

		Player &player = m_game.GetPlayer();
		const Ogre::Vector3 &pos = m_game.GetCamera().getPosition();

		// If game is over, show end screen
		if (player.IsGameOver())
		{
			m_pMessageText->setCaption(player.IsVictory() ? Texts::Victory : Texts::GameOver + "\n" +
				Texts::YourHighscore + " " + StringConverter::toString(player.GetScore()) + " " +
				"(#1)");
		}

		// If we just lost a life, display message
		else if (player.GetExplosionTimeoutMs() >= 0)
		{
			m_pMessageText->setCaption(Texts::YouLostALife + "\n" +
				Texts::ClickToContinue);
		}

		// Show countdown if a new life started
		else if (player.GetLifeTimeMs() < Player::LifeTimeZoomAndAccelerateMs)
		{
			float alpha = 1.0f - ((static_cast<int>(player.GetLifeTimeMs()) % 1000) / 1000.0f);
			alpha *= 2.0f;
			alpha = std::min(1.0f, alpha);
			m_pMessageText->setColour(Ogre::ColourValue(1.0f, 1.0f, 1.0f, alpha));

			if (player.GetLifeTimeMs() < 3000)
			{
				m_pMessageText->setCaption(Texts::GetReady + "\n" +
					StringConverter::toString(3 - static_cast<int>(player.GetLifeTimeMs() / 1000)));
			}
			else
			{
				m_pMessageText->setCaption(Texts::Go);
			}
		}

		// Show item helper messages
		else if (player.GetItemMessageTimeoutMs() > 0 &&
			player.GetCurrentItemMessage().empty() == false)
		{
			player.SetItemMessageTimeoutMs(std::max(0.0f, player.GetItemMessageTimeoutMs() - m_game.GetElapsedTimeThisFrameInMs()));

			float alpha = 1.0f;
			if (player.GetItemMessageTimeoutMs() < 1000)
				alpha = player.GetItemMessageTimeoutMs() / 1000.0f;
			m_pMessageText->setColour(Ogre::ColourValue(0.83f, 0.83f, 0.83f, alpha));
			m_pMessageText->setCaption(player.GetCurrentItemMessage());
		}

		// Warning if nearly dead
		else if (player.GetShowHealthWarningTimeoutMs() > 0)
		{
			float alpha = 1.0f;
			if (player.GetShowHealthWarningTimeoutMs() < 1000)
				alpha = player.GetShowHealthWarningTimeoutMs() / 1000.0f;

			m_pMessageText->setColour(Ogre::ColourValue(1, 0, 0, 1));
			m_pMessageText->setCaption(Texts::WarningLowHealth);
		}

		// Warning if going out of fuel
		else if (player.GetFuel() < 0.1f)
		{
			m_pMessageText->setColour(Ogre::ColourValue(1, 0, 0, 1));
			m_pMessageText->setCaption(Texts::WarningOutOfFuel);
		}

		// Warning if out of playable area
		else if (pos.x / GameAsteroidManager::SectorWidth <
			-m_game.GetAsteroidManager().GetLevel().GetWidth() / 2 ||
			pos.x / GameAsteroidManager::SectorWidth >
			+m_game.GetAsteroidManager().GetLevel().GetWidth() / 2 ||
			pos.y / GameAsteroidManager::SectorHeight <
			-m_game.GetAsteroidManager().GetLevel().GetWidth() / 2 ||
			pos.y / GameAsteroidManager::SectorHeight >
			+m_game.GetAsteroidManager().GetLevel().GetWidth() / 2 ||
			pos.z < 0)
		{
			m_pMessageText->setColour(Ogre::ColourValue(1, 0, 0, 1));
			m_pMessageText->setCaption(Texts::WarningOutOfLevel);
		}
	}

	void MissionScreen::ShowTarget(int levelLength)
	{

	}
}
