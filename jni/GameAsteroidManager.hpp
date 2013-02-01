#ifndef _GAME_ASTEROID_MANAGER_HPP_
#define _GAME_ASTEROID_MANAGER_HPP_

#include "Level.hpp"
#include <list>

namespace rcd
{
	class Asteroid;
	class SmallAsteroid;
	class Player;
	class Goal;
	class Game;

	class GameAsteroidManager
	{
	public:
		GameAsteroidManager(Game& m_game, const Level& level);
		~GameAsteroidManager();

		static const float SectorWidth;
		static const float SectorHeight;
		static const float SectorDepth;

		static const float MinAsteroidSize;
		static const float MaxAsteroidSize;

		static const float SmallAsteroidSize;

		static float GetMinViewDepth();
		static float GetMaxViewDepth();
		static float GetHalfViewDepth();

		static const int NumberOfSectors = 19;
		static const int MiddleSector = NumberOfSectors / 2;
		static const int MinSector = -MiddleSector;
		static const int MaxSector = +MiddleSector;

		static const int NumberOfSmallSectors = 7;
		static const int SmallMiddleSector = NumberOfSmallSectors / 2;
		static const int SmallSectorAdd = MiddleSector - SmallMiddleSector;
		static const int MinSmallSector = -SmallMiddleSector;
		static const int MaxSmallSector = +SmallMiddleSector;

		const Level& GetLevel() const { return m_currentLevel; }
		const Ogre::Vector3& GetTargetPosition() const { return m_targetPos; }
		void SetLevel(const Level &level);

		void Update();

		void KillAllInnerSectorAsteroids();
		float PlayerAsteroidCollision(Player &player);

	private:
		void HandleSectorPhysics(int checkX, int checkZ, int cameraSectorPosX, int cameraSectorPosZ);
		void ShowAllItems();
		static Ogre::Vector3 ReflectVector(const Ogre::Vector3 &vec, const Ogre::Vector3 &normal);
		void HandleAsteroidCollision(Asteroid &asteroid1, Asteroid &asteroid2);
		void ClearAllItems();
		void ClearGoal();

		static const int MaxHitDirectionTimeoutMs = 2000;

		static const float MinViewDepth;
		static float MaxViewDepth;
		static float HalfViewDepth;

		Game& m_game;
		Ogre::Vector3 m_targetPos;

		typedef std::list<Asteroid *> Asteroids;
		std::vector<std::vector<Asteroids> > m_sectorAsteroids;
		std::vector<std::vector<bool> >	m_sectorIsVisible;

		typedef std::list<SmallAsteroid *> SmallAsteroids;
		std::vector<std::vector<SmallAsteroids> > m_sectorSmallerAsteroids;

		void GenerateSector(Asteroids &asteroids, int x, int z);
		void GenerateSmallerAsteroidsSector(SmallAsteroids &smallAsteroids, int numOfAsteroids, int x, int z);
		void CalculateSectors();
		void CalculateSectorMovement();
		void CalculateSectorVisibility();
		Ogre::Radian GetViewableFieldOfView() const;
		void SetHitDirectionEffect(const Ogre::Radian &direction);

		Level m_currentLevel;
		std::vector<std::vector<bool> >	m_sectorVisibleInRange;
		std::vector<std::vector<Ogre::Vector3> > m_sectorDirection;
		int m_centerSectorPositionX;
		int m_centerSectorPositionZ;
		int m_numAsteroidModels;
		int m_numSmallAsteroidModels;

		std::list<Item *> m_items;
		int m_hitDirectionEffectTimeoutMs;
		Ogre::Radian m_hitDirection;
		Goal *m_goal;
	};
}

#endif
