#ifndef _PLAYER_HPP_
#define _PLAYER_HPP_

#include "Game.hpp"
#include "Item.hpp"

namespace rcd
{
	class GameAsteroidManager;

	class Player
	{
	public:
		Player(Game &game);

		void HandleGameLogic();
		void Reset(const std::string &levelName);
		bool CanControlRocket() const;
		void HandleItem(Item::ItemTypes itemType);
		void SetGameOverAndUploadHighscore();

		// Set methods
		void SetAsteroidManager(GameAsteroidManager *asteroidManager);
		void SetNumberOfBombItems(int numBombItems) { m_numberOfBombItems = numBombItems; }
		void SetScore(int score) { m_score = score; }
		void SetCameraWobbel(float wobbelFactor);
		void SetStartingSpeed(float percentage);
		void SetFuel(float fuel) { m_fuel = fuel; }
		void SetSpeed(float speed);
		void SetSpeedItemTimeout(float speedItemTimeout);
		void SetExplosionTimeoutMs(float explosionTimeoutMs);
		void SetItemMessage(const std::string &message);
		void SetItemMessageTimeoutMs(float itemMessageTimeoutMs);

		// Get methods
		float GetGameTimeMs() const { return m_gameTimeMs; }
		bool  IsGameOver() const	{ return m_gameOver; }
		float GetSpeed() const		{ return m_speed; }
		float GetMovementSpeedPerSecond() const { return MovementSpeedPerSecond; }
		int   GetNumberOfBombItems() const { return m_numberOfBombItems; }
		int   GetScore() const		{ return m_score; }
		float GetLifeTimeMs() const { return m_lifeTimeMs; }
		float GetFuel() const		{ return m_fuel; }
		float GetSpeedItemTimeout() const { return m_speedItemTimeout; }
		float GetExplosionTimeoutMs() const { return m_explosionTimeoutMs; }
		float GetExplosionTimeoutMs2() const { return m_explosionTimeoutMs2; }
		float GetExplosionTimeoutMs3() const { return m_explosionTimeoutMs3; }
		float GetHealth() const		{ return m_health; }
		int   GetLifes() const		{ return m_lifes; }
		bool  IsVictory() const     { return m_victory; }
		float GetItemMessageTimeoutMs() const { return m_itemMessageTimeoutMs; }
		const std::string& GetCurrentItemMessage() const { return m_currentItemMessage; }
		float GetShowHealthWarningTimeoutMs() const { return m_showHealthWarningTimeoutMs; }

		static const int	LifeTimeZoomAndAccelerateMs = 4000;
		static const float	FuelRefillTime;
		static const float	SlideFactor;
		static const float	MaxSpeedWithoutItem;
		static const float  RotationSpeedPerMouseMovement;

	private:
		static const int   DefaultNumberOfLifes = 3;
		static const float DefaultSpeed;
		static const float MinSpeedWithoutItem;
		static const float MinSpeedWithItem;
		static const float MaxSpeedWithItem;
		static const float HurtFactorIfFuelIsEmpty;
		static const int   MaxExplosionTimeoutMs = 1500;
		static const int   MaxCameraWobbelTimeoutMs = 700;
		static const int   MaxSpeedItemTimeout = 10 * 1000;
		static const int   MaxItemMessageTimeoutMs = 6000;

		const float MovementSpeedPerSecond;

		void ResetLifeValues();

		Game	&m_game;
		GameAsteroidManager *m_asteroidManager;
		float		m_gameTimeMs;
		std::string m_levelName;
		bool		m_gameOver;
		float		m_health;
		float		m_fuel;
		int			m_lifes;
		int			m_score;
		float		m_cameraWobbelTimeoutMs;
		float		m_explosionTimeoutMs;
		float		m_explosionTimeoutMs2;
		float		m_explosionTimeoutMs3;
		bool		m_alreadyUploadedHighscore;
		float		m_lifeTimeMs;
		float		m_speed;
		float		m_speedItemTimeout;
		int			m_numberOfBombItems;
		float		m_itemMessageTimeoutMs;
		float		m_showHealthWarningTimeoutMs;
		bool		m_victory;
		float		m_cameraWobbelFactor;
		Item::ItemTypes m_lastCollectedItem;
		std::string m_currentItemMessage;
	};
}

#endif
