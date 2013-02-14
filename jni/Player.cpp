#include "Player.hpp"
#include "GameAsteroidManager.hpp"
#include "Texts.hpp"
#include "Sound.hpp"

namespace rcd
{
	const float Player::DefaultSpeed = 0.5f;
	const float Player::MinSpeedWithoutItem = 0.5f;
	const float Player::MaxSpeedWithoutItem = 0.7f;
	const float Player::MinSpeedWithItem = 0.7f;
	const float Player::MaxSpeedWithItem = 1.0f;
	const float Player::HurtFactorIfFuelIsEmpty = 10.0f;
	const float Player::FuelRefillTime = 40.0f;
	const float Player::SlideFactor = 1.0f / 5.5f;
	const float Player::RotationSpeedPerMouseMovement = 0.0015f;

	Player::Player(Game &game)
	 : MovementSpeedPerSecond(1111.0f),
	   m_game(game),
	   m_asteroidManager(NULL),
	   m_gameTimeMs(0.0f),
	   m_levelName(""),
	   m_gameOver(false),
	   m_health(1.0f),
	   m_fuel(1.0f),
	   m_lifes(DefaultNumberOfLifes),
	   m_score(0),
	   m_cameraWobbelTimeoutMs(0.0f),
	   m_explosionTimeoutMs(-1.0f),
	   m_explosionTimeoutMs2(-1.0f),
	   m_explosionTimeoutMs3(-1.0f),
	   m_alreadyUploadedHighscore(false),
	   m_lifeTimeMs(0.0f),
	   m_speed(DefaultSpeed),
	   m_speedItemTimeout(0.0f),
	   m_numberOfBombItems(0),
	   m_itemMessageTimeoutMs(0.0f),
	   m_showHealthWarningTimeoutMs(0.0f),
	   m_victory(false),
	   m_cameraWobbelFactor(1.0f),
	   m_lastCollectedItem(Item::NumOfItemTypes),
	   m_currentItemMessage("")
	{
	}

	void Player::SetAsteroidManager(GameAsteroidManager *asteroidManager)
	{
		assert(m_asteroidManager == NULL);
		m_asteroidManager = asteroidManager;
	}

	void Player::HandleGameLogic()
	{
		assert(m_asteroidManager);

		if (m_gameTimeMs == 0)
		{
			//TODO: m_game.GetSound().PlayRocketMotorSound(0.86f);
		}

		if (m_explosionTimeoutMs >= 0)
		{
			m_explosionTimeoutMs -= m_game.GetElapsedTimeThisFrameInMs();
			if (m_explosionTimeoutMs < 0)
				m_explosionTimeoutMs = 0;
		}
		else if (IsGameOver() == false)
		{
			m_gameTimeMs += m_game.GetElapsedTimeThisFrameInMs();
			m_lifeTimeMs += m_game.GetElapsedTimeThisFrameInMs();
		}

		if (m_explosionTimeoutMs2 >= 0)
		{
			m_explosionTimeoutMs2 -= m_game.GetElapsedTimeThisFrameInMs();
			if (m_explosionTimeoutMs2 < 0)
				m_explosionTimeoutMs2 = 0;
		}

		if (m_explosionTimeoutMs3 >= 0)
		{
			m_explosionTimeoutMs3 -= m_game.GetElapsedTimeThisFrameInMs();
			if (m_explosionTimeoutMs3 < 0)
				m_explosionTimeoutMs3 = 0;
		}

		if (m_cameraWobbelTimeoutMs > 0)
		{
			m_cameraWobbelTimeoutMs -= m_game.GetElapsedTimeThisFrameInMs();
			if (m_cameraWobbelTimeoutMs < 0)
				m_cameraWobbelTimeoutMs = 0;
		}

		if (IsGameOver())
			return;

		const float oldHealth = m_health;

		//TODO: m_game.GetSound().ChangeRocketMotorPitchEffect(-0.24f + m_speed * 0.6f);

		// Check if too near to an asteroid. Check 3x3 sector in middle.
		if (CanControlRocket())
		{
			const float playerCollision = m_asteroidManager->PlayerAsteroidCollision(*this);
			if (playerCollision > 0.0f)
			{
				m_health -= 0.1f + playerCollision * 4.25f;
				if (oldHealth == 1.0f && m_health <= 0.0f)
				{
					m_health = 0.1f;
					m_game.GetSound().PlayExplosionSound();
				}
			}
		}

		// Fuel
		if (m_fuel < 0.0f)
		{
			m_fuel = 0.0f;
			m_health -= m_game.GetMoveFactorPerSecond() / HurtFactorIfFuelIsEmpty;
		}

		// Show health low warning if health is getting very low
		if ((oldHealth >= 0.25f && m_health < 0.25f) ||
			(oldHealth >= 0.1f && m_health < 0.1f))
			m_showHealthWarningTimeoutMs = 8 * 1000;
		if (m_showHealthWarningTimeoutMs > 0)
		{
			m_showHealthWarningTimeoutMs -= m_game.GetElapsedTimeThisFrameInMs();
			m_showHealthWarningTimeoutMs = std::max(0.0f, m_showHealthWarningTimeoutMs);
		}

		// Die if health is 0 or lower
		if (m_health <= 0.0f)
		{
			m_health = 0.0f;

			m_explosionTimeoutMs = MaxExplosionTimeoutMs;

			// Reset everything for the player, all items and stuff
			ResetLifeValues();

			if (m_lifes > 0)
			{
				TODO: m_game.GetSound().PlayExplosionSound();
				m_lifes--;
			}
			else
			{
				m_victory = false;
				m_explosionTimeoutMs2 = MaxExplosionTimeoutMs * 1.6f;
				m_explosionTimeoutMs3 = MaxExplosionTimeoutMs * 2.1f;
				SetGameOverAndUploadHighscore();
				//TODO: m_game.GetSound().StopRocketMotorSound();
				//TODO: m_game.GetSound().PlayDefeatSound();
			}

			m_speed = MinSpeedWithoutItem;

			m_asteroidManager->KillAllInnerSectorAsteroids();
		}

		// Reached target?
		if (m_game.GetCamera().getPosition().z >=
			m_asteroidManager->GetLevel().GetLength() * GameAsteroidManager::SectorDepth)
		{
			m_victory = true;

			// Life bonus
			m_score += m_lifes * 10000;

			// Time bonus
			const int maxGameTime = (m_asteroidManager->GetLevel().GetLength() / 2) * 1000;
			if (m_gameTimeMs < maxGameTime)
			{
				m_score += (int)(maxGameTime - m_gameTimeMs) / 10;
			}

			// Health & fuel bonus
			m_score += (int)(m_health * 3000);
			m_score += (int)(m_fuel * 4000);

			SetGameOverAndUploadHighscore();
			//TODO: m_game.GetSound().StopRocketMotorSound();
			//TODO: m_game.GetSound().PlayVictorySound();
		}
	}

	void Player::Reset(const std::string &levelName)
	{
		m_levelName = levelName;
		m_gameOver = false;
		m_alreadyUploadedHighscore = false;
		m_gameTimeMs = 0.0f;
		m_health = 1.0f;
		m_fuel = 1.0f;
		m_lifes = DefaultNumberOfLifes;
		m_score = 0;
		m_cameraWobbelTimeoutMs = 0;
		m_explosionTimeoutMs = -1;

		ResetLifeValues();
	}

	void Player::ResetLifeValues()
	{
		m_lifeTimeMs = 0.0f;

		if (IsGameOver())
		{
			m_speed = 0.0f;
		}
		else
		{
			m_health = 1.0f;
			m_fuel = 1.0f;
			m_speed = DefaultSpeed;
		}

		m_speedItemTimeout = 0.0f;
		m_numberOfBombItems = 0;
		m_itemMessageTimeoutMs = 0;
		m_showHealthWarningTimeoutMs = 0;
	}

	bool Player::CanControlRocket() const
	{
		return (m_lifeTimeMs > 3000) &&
			(m_explosionTimeoutMs < 0) &&
			(IsGameOver() == false);
	}

	void Player::SetGameOverAndUploadHighscore()
	{
		m_lifes = 0;
		m_gameOver = true;
		m_speed = 0;

		// Upload highscore
		if (m_alreadyUploadedHighscore == false)
		{
			m_alreadyUploadedHighscore = true;
			//Highscores::SubmitHighscore(m_score, m_levelName);
		}
	}

	void Player::SetCameraWobbel(float wobbelFactor)
	{
		m_cameraWobbelTimeoutMs = (int)MaxCameraWobbelTimeoutMs;
		m_cameraWobbelFactor = wobbelFactor;
	}

	void Player::SetStartingSpeed(float percentage)
	{
		m_speed = percentage * DefaultSpeed;
	}

	void Player::SetSpeed(float speed)
	{
		m_speed = speed;

		if (m_speedItemTimeout == 0)
		{
			if (m_speed > MaxSpeedWithoutItem)
				m_speed = MaxSpeedWithoutItem;

			if (m_speed < MinSpeedWithoutItem &&
				m_lifeTimeMs > LifeTimeZoomAndAccelerateMs)
				m_speed = MinSpeedWithoutItem;
		}
		else
		{
			if (m_speed > MaxSpeedWithItem)
				m_speed = MaxSpeedWithItem;

			if (m_speed < MinSpeedWithItem)
				m_speed = MinSpeedWithItem;
		}
	}

	void Player::SetSpeedItemTimeout(float speedItemTimeout)
	{
		m_speedItemTimeout = speedItemTimeout;
	}

	void Player::HandleItem(Item::ItemTypes itemType)
	{
		m_lastCollectedItem = itemType;

		// Play item sound
		//TODO: m_game.GetSound().PlayItemSound(itemType);

		m_score += 2500;

		switch (itemType)
		{
		case Item::ItemType_Fuel:
			m_fuel = 1.0f;
			SetItemMessage(Texts::ItemRefilledFuel);
			break;

		case Item::ItemType_Health:
			m_health += 0.5f;
			if (m_health > 1.0f)
				m_health = 1.0f;
			SetItemMessage(Texts::ItemHealthRefreshed);
			break;

		case Item::ItemType_Speed:
			m_speedItemTimeout = MaxSpeedItemTimeout;
			m_speed = MaxSpeedWithItem;
			SetItemMessage(Texts::ItemSpeedMode);
			break;

		case Item::ItemType_Bomb:
			if (m_numberOfBombItems < 3)
				m_numberOfBombItems++;
			SetItemMessage(Texts::ItemBombActive);
			break;

		case Item::ItemType_ExtraLife:
			m_lifes++;
			SetItemMessage(Texts::ItemExtraLife);
			break;
		}
	}

	void Player::SetExplosionTimeoutMs(float explosionTimeoutMs)
	{
		m_explosionTimeoutMs = explosionTimeoutMs;
	}

	void Player::SetItemMessage(const std::string &message)
	{
		m_currentItemMessage = message;
		m_itemMessageTimeoutMs = MaxItemMessageTimeoutMs;
	}

	void Player::SetItemMessageTimeoutMs(float itemMessageTimeoutMs)
	{
		m_itemMessageTimeoutMs = itemMessageTimeoutMs;
	}
}
