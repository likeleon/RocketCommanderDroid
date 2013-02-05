#include "GameAsteroidManager.hpp"
#include <OgreMeshManager.h>
#include <exception>
#include "MathHelper.hpp"
#include "ColourHelper.hpp"
#include "Goal.hpp"
#include "RandomHelper.hpp"
#include "Asteroid.hpp"
#include "Game.hpp"
#include "Player.hpp"
#include "SmallAsteroid.hpp"
#include "LensFlare.hpp"

using namespace Ogre;

namespace rcd
{
	const float GameAsteroidManager::SectorWidth = 200.0f;
	const float GameAsteroidManager::SectorHeight = 200.0f;
	const float GameAsteroidManager::SectorDepth = 200.0f;

	const float GameAsteroidManager::MinAsteroidSize = 32.0f;
	const float GameAsteroidManager::MaxAsteroidSize = 62.0f;

	const float GameAsteroidManager::SmallAsteroidSize = 129.0f;

	const float GameAsteroidManager::MinViewDepth = 0.33f;
	float GameAsteroidManager::MaxViewDepth = -1.0f;
	float GameAsteroidManager::HalfViewDepth = -1.0f;

	float GameAsteroidManager::GetMinViewDepth()
	{
		return MinViewDepth;
	}

	float GameAsteroidManager::GetMaxViewDepth()
	{
		if (MaxViewDepth == -1.0f)
		{
			MaxViewDepth = SectorDepth * NumberOfSectors;// - SectorDepth / 2.0f;
		}
		return MaxViewDepth;
	}

	float GameAsteroidManager::GetHalfViewDepth()
	{
		if (HalfViewDepth == -1.0f)
		{
			HalfViewDepth = GetMaxViewDepth() * 0.4f;
		}
		return HalfViewDepth;
	}

	GameAsteroidManager::GameAsteroidManager(Game &game, const Level &level)
		: m_game(game),
		  m_targetPos(0, 0, 10000),
		  m_sectorAsteroids(NumberOfSectors, std::vector<Asteroids>(NumberOfSectors)),
		  m_sectorIsVisible(NumberOfSectors, std::vector<bool>(NumberOfSectors, false)),
		  m_sectorSmallerAsteroids(NumberOfSmallSectors, std::vector<SmallAsteroids>(NumberOfSmallSectors)),
		  m_sectorVisibleInRange(NumberOfSectors, std::vector<bool>(NumberOfSectors)),
		  m_sectorDirection(NumberOfSectors, std::vector<Ogre::Vector3>(NumberOfSectors, Ogre::Vector3::ZERO)),
		  m_centerSectorPositionX(0),
		  m_centerSectorPositionZ(0),
		  m_numAsteroidModels(5),
		  m_numSmallAsteroidModels(3),
		  m_hitDirectionEffectTimeoutMs(0),
		  m_hitDirection(0.0f),
		  m_goal(NULL)
	{
		// Set level
		SetLevel(level);

		// Load all available asteroids
		for (int i = 0; i < m_numAsteroidModels; ++i)
		{
			const std::string meshName = "asteroid" + StringConverter::toString(i + 1);
			const std::string meshFile = meshName + ".mesh";
			const std::string lowMeshFile = meshName + "Low.mesh";

			MeshPtr meshPtr = MeshManager::getSingleton().load(meshFile, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
			MeshPtr lowMeshPtr = MeshManager::getSingleton().load(lowMeshFile, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
			meshPtr->createManualLodLevel(GetHalfViewDepth(), lowMeshPtr->getName());
		}

		// Create all asteroids
		for (int z = MinSector; z <= MaxSector; ++z)
		{
			for (int x = MinSector; x <= MaxSector; ++x)
			{
				const int iz = z + MiddleSector;
				const int ix = x + MiddleSector;
				GenerateSector(m_sectorAsteroids[iz][ix], x, z);
			}
		}

		// Create smaller asteroids
		for (int z = MinSmallSector; z <= MaxSmallSector; ++z)
		{
			for (int x = MinSmallSector; x <= MaxSmallSector; ++x)
			{
				const int iz = z + SmallMiddleSector;
				const int ix = x + SmallMiddleSector;
				GenerateSmallerAsteroidsSector(m_sectorSmallerAsteroids[iz][ix],
					m_sectorAsteroids[iz + SmallSectorAdd][ix + SmallSectorAdd].size(), x, z);
			}
		}

		// Precalculate visible sector stuff
		for (int z = MinSector; z <= MaxSector; ++z)
		{
			for (int x = MinSector; x <= MaxSector; ++x)
			{
				const int iz = z + MiddleSector;
				const int ix = x + MiddleSector;

				// Check if distance is smaller than the max view depth and
				// add a small offset to include nearly visible sectors.
				m_sectorVisibleInRange[iz][ix] = Math::Sqrt((Real)(x * x + z * z)) < MiddleSector + 0.25f;

				// Calculate direction (just normalize relative position)
				m_sectorDirection[iz][ix] = Ogre::Vector3((Real)x, 0, (Real)z);
				m_sectorDirection[iz][ix].normalise();
			}
		}

		// Calculate sectors and visibility
		CalculateSectors();
	}

	GameAsteroidManager::~GameAsteroidManager()
	{
		ClearAllItems();
		ClearGoal();

		for (std::vector<std::vector<Asteroids> >::iterator it1 = m_sectorAsteroids.begin(); it1 != m_sectorAsteroids.end(); ++it1)
		{
			for (std::vector<Asteroids>::iterator it2 = it1->begin(); it2 != it1->end(); ++it2)
			{
				for (Asteroids::iterator asteroid = it2->begin(); asteroid != it2->end(); ++asteroid)
				{
					delete *asteroid;
				}
			}
		}

		for (std::vector<std::vector<SmallAsteroids> >::iterator it1 = m_sectorSmallerAsteroids.begin(); it1 != m_sectorSmallerAsteroids.end(); ++it1)
		{
			for (std::vector<SmallAsteroids>::iterator it2 = it1->begin(); it2 != it1->end(); ++it2)
			{
				for (SmallAsteroids::iterator smallAsteroid = it2->begin(); smallAsteroid != it2->end(); ++smallAsteroid)
				{
					delete *smallAsteroid;
				}
			}
		}
	}

	void GameAsteroidManager::GenerateSector(Asteroids &asteroids, int x, int z)
	{
		// Make sure there are no asteroids in sector 0, 0, 0 and the surrounding sectors
		if (std::abs(x) < 2 && std::abs(z) < 2)
			return;

		// Check out how much asteroids we got in this sector.
		// If this is not a valid sector, use a medium density.
		float density = 0.1f;
		const int levelX = x + m_currentLevel.GetWidth() / 2;
		const int levelZ = z;
		if (levelX >= 0 && levelX < m_currentLevel.GetWidth() &&
			levelZ >= 0 && levelZ < m_currentLevel.GetLength())
		{
			density += m_currentLevel.GetDensity(levelX, levelZ);
		}

		const int numOfAsteroids = RandomHelper::GetRandomInt(0, (int)(2 + density * 10));

		for (int num = 0; num < numOfAsteroids; ++num)
		{
			const int type = RandomHelper::GetRandomInt(0, m_numAsteroidModels - 1);
			Vector3 position(x * SectorWidth,
				m_game.GetCamera().getPosition().y + Math::RangeRandom(-SectorWidth * 3.15f, +SectorWidth * 3.15f),
				z * SectorDepth);
			position += Vector3(RandomHelper::GetRandomVector3(-SectorWidth * 0.42f, +SectorWidth * 0.42f));
			asteroids.push_back(new Asteroid(m_game.GetSceneManager(), type, position));
		}
	}

	void GameAsteroidManager::GenerateSmallerAsteroidsSector(SmallAsteroids &smallAsteroids, int numOfAsteroids, int x, int z)
	{
		// Always create at least 1 smaller asteroid instance per sector
		const int numOfSmallerAsteroids = 2 + RandomHelper::GetRandomInt(0, 4 + numOfAsteroids);

		for (int num = 0; num < numOfSmallerAsteroids; ++num)
		{
			const int type = RandomHelper::GetRandomInt(0, m_numSmallAsteroidModels - 1);
			Vector3 position(x * SectorWidth, 0, z * SectorDepth);
			position += Vector3(Math::RangeRandom(-SectorWidth / 2, +SectorWidth / 2),
				Math::RangeRandom(-SectorWidth * 2.1f, +SectorWidth * 2.1f),
				Math::RangeRandom(-SectorWidth / 2, +SectorWidth / 2));
			smallAsteroids.push_back(new SmallAsteroid(m_game.GetSceneManager(), type, position));
		}
	}

	void GameAsteroidManager::CalculateSectors()
	{
		CalculateSectorMovement();
		CalculateSectorVisibility();
	}

	void GameAsteroidManager::CalculateSectorMovement()
	{
		const Vector3 &cameraPos = m_game.GetCamera().getPosition();
		const int cameraSectorPosX = (int)MathHelper::Round(cameraPos.x / SectorWidth);
		const int cameraSectorPosZ = (int)MathHelper::Round(cameraPos.z / SectorWidth);

		// Changed sector?
		if (m_centerSectorPositionX != cameraSectorPosX || m_centerSectorPositionZ != cameraSectorPosZ)
		{
			// Move sectors over, check out how much we are moving
			const int movedXSectors = cameraSectorPosX - m_centerSectorPositionX;
			const int movedZSectors = cameraSectorPosZ - m_centerSectorPositionZ;

			//
			// Normal asteroids
			//
			std::vector<std::vector<Asteroids> > helperCopyAsteroids(m_sectorAsteroids);
			std::vector<std::vector<bool> > helperCopyUsed(NumberOfSectors, std::vector<bool>(NumberOfSectors, false));

			for (int z = 0; z < NumberOfSectors; ++z)
			{
				for (int x = 0; x < NumberOfSectors; ++x)
				{
					if (x >= -movedXSectors && x < NumberOfSectors - movedXSectors &&
						z >= -movedZSectors && z < NumberOfSectors - movedZSectors)
					{
						m_sectorAsteroids[z][x] = helperCopyAsteroids[z + movedZSectors][x + movedXSectors];
						helperCopyUsed[z + movedZSectors][x + movedXSectors] = true;
					}
					else
					{
						m_sectorAsteroids[z][x].clear();
						GenerateSector(m_sectorAsteroids[z][x],
							x - MiddleSector + cameraSectorPosX,
							z - MiddleSector + cameraSectorPosZ);
					}
				}
			}

			for (int z = 0; z < NumberOfSectors; ++z)
			{
				for (int x = 0; x < NumberOfSectors; ++x)
				{
					if (helperCopyUsed[z][x])
						continue;

					for (Asteroids::iterator asteroid = helperCopyAsteroids[z][x].begin(); asteroid != helperCopyAsteroids[z][x].end(); ++asteroid)
					{
						delete *asteroid;
					}
				}
			}

			//
			// Smaller asteroids
			//
			std::vector<std::vector<SmallAsteroids> > helperCopySmallerAsteroids(m_sectorSmallerAsteroids);
			std::vector<std::vector<bool> > helperCopyUsedSmaller(NumberOfSmallSectors, std::vector<bool>(NumberOfSmallSectors, false));

			for (int z = 0; z < NumberOfSmallSectors; ++z)
			{
				for (int x = 0; x < NumberOfSmallSectors; ++x)
				{
					if (x >= -movedXSectors && x < NumberOfSmallSectors - movedXSectors &&
						z >= -movedZSectors && z < NumberOfSmallSectors - movedZSectors)
					{
						m_sectorSmallerAsteroids[z][x] = helperCopySmallerAsteroids[z + movedZSectors][x + movedXSectors];
						helperCopyUsedSmaller[z + movedZSectors][x + movedXSectors] = true;
					}
					else
					{
						m_sectorSmallerAsteroids[z][x].clear();
						GenerateSmallerAsteroidsSector(m_sectorSmallerAsteroids[z][x],
							m_sectorAsteroids[z + SmallSectorAdd][x + SmallSectorAdd].size(),
							x - SmallMiddleSector + cameraSectorPosX,
							z - SmallMiddleSector + cameraSectorPosZ);
					}
				}
			}

			for (int z = 0; z < NumberOfSmallSectors; ++z)
			{
				for (int x = 0; x < NumberOfSmallSectors; ++x)
				{
					if (helperCopyUsedSmaller[z][x])
						continue;

					for (SmallAsteroids::iterator smallAsteroid = helperCopySmallerAsteroids[z][x].begin(); smallAsteroid != helperCopySmallerAsteroids[z][x].end(); ++smallAsteroid)
					{
						delete *smallAsteroid;
					}
				}
			}

			m_centerSectorPositionX = cameraSectorPosX;
			m_centerSectorPositionZ = cameraSectorPosZ;
		}
	}

	void GameAsteroidManager::CalculateSectorVisibility()
	{
		for (int z = MinSector; z <= MaxSector; ++z)
		{
			for (int x = MinSector; x <= MaxSector; ++x)
			{
				const int iz = z + MiddleSector;
				const int ix = x + MiddleSector;

				bool isVisible = m_sectorVisibleInRange[iz][ix];

				// Allow all if looking up or down now.
				Vector3 camDir = m_game.GetCamera().getDirection();
				if (isVisible && std::abs(camDir.y) < 0.75f)
				{
					Radian angle = camDir.angleBetween(m_sectorDirection[iz][ix]);
					isVisible = camDir.angleBetween(m_sectorDirection[iz][ix]) < GetViewableFieldOfView();
				}

				if (std::abs(x) + std::abs(z) <= 2)
					isVisible = true;

				m_sectorIsVisible[iz][ix] = isVisible;
			}
		}
	}

	void GameAsteroidManager::Update()
	{
		//
		// Initialize and cacluate sectors
		//

		//Get current sector we are in.
		const Vector3 &cameraPos = m_game.GetCamera().getPosition();
		const int cameraSectorPosX = (int)MathHelper::Round(cameraPos.x / SectorWidth);
		const int cameraSectorPosZ = (int)MathHelper::Round(cameraPos.z / SectorWidth);

		CalculateSectors();

		//
		// Prepare level position and colors
		//

		// Get level position for sun color
		int levelPos = cameraSectorPosZ;
		if (levelPos < 0)
			levelPos = 0;
		if (levelPos >= m_currentLevel.GetLength())
			levelPos = m_currentLevel.GetLength() - 1;

		// Update sun color, also for lighting and lens flare.
		const ColourValue sunColour = ColourHelper::InterpolateColour(ColourValue::White, m_currentLevel.GetSunColour(levelPos), 0.75f);
		m_game.SetLensFlareColour(sunColour);

		// Set colour to lighting
		//m_game.SetLightColour(sunColour);
		m_game.SetLightColour(ColourValue::White);

		// Also update sun position
		const Vector3 sunPos = LensFlare::RotateSun(Radian(Math::PI / 4.0f + m_game.GetPlayer().GetGameTimeMs() / 50000.0f));
		LensFlare::SetOrigin3D(sunPos);
		m_game.SetLightDirection(-sunPos);

		//
		// Update asteroids
		//
		for (int z = 0; z < NumberOfSectors; ++z)
		{
			for (int x = 0; x < NumberOfSectors; ++x)
			{
				if (!m_sectorIsVisible[z][x])
					continue;

				// Update rotation and movement for all asteroids
				for (Asteroids::iterator asteroid = m_sectorAsteroids[z][x].begin(); asteroid != m_sectorAsteroids[z][x].end(); ++asteroid)
				{
					// Update rotation and position
					(*asteroid)->UpdateMovement(m_game.GetMoveFactorPerSecond());
				}

				if ((z + x) % 2 == m_game.GetTotalFrames() % 2)
					HandleSectorPhysics(x, z, cameraSectorPosX, cameraSectorPosZ);
			}
		}

		// Show all items
		ShowAllItems();
	}

	Radian GameAsteroidManager::GetViewableFieldOfView() const
	{
		return Game::FieldOfView * 0.75f;
	}

	void GameAsteroidManager::HandleSectorPhysics(int checkX, int checkZ, int cameraSectorPosX, int cameraSectorPosZ)
	{
		Asteroids &thisSectorAsteroids = m_sectorAsteroids[checkZ][checkX];

		if (thisSectorAsteroids.empty())
			return;

		for (Asteroids::iterator astIter = thisSectorAsteroids.begin(); astIter != thisSectorAsteroids.end();)
		{
			Asteroid * const asteroid = *astIter;

			// Get position in percentage
			const float xp = asteroid->GetPosition().x / SectorWidth;
			const float zp = asteroid->GetPosition().z / SectorWidth;

			// Get sector position
			int ix = (int)MathHelper::Round(xp);
			int iz = (int)MathHelper::Round(zp);

			// -0.5: left border, +0.5: right border
			// -0.4 ~ +0.4: inside the sector
			const float borderX = xp - ix;
			const float borderZ = zp - iz;

			// Subtract current position from sector position
			ix = ix - cameraSectorPosX + MiddleSector;
			iz = iz - cameraSectorPosZ + MiddleSector;

			if (ix < 0)
				ix = 0;
			if (iz < 0)
				iz = 0;
			if (ix >= NumberOfSectors)
				ix = NumberOfSectors - 1;
			if (iz >= NumberOfSectors)
				iz = NumberOfSectors - 1;

			// Change asteroid sector if it is not longer in the correct sector.
			if (ix != checkX || iz != checkZ)
			{
				thisSectorAsteroids.erase(astIter++);
				m_sectorAsteroids[iz][ix].push_back(asteroid);
			}
			else
			{
				astIter++;
			}

			if (borderX > -0.4f && borderX < 0.4f &&
				borderZ > -0.4f && borderZ < 0.4f)
			{
				// Only check this ssecctor
				for (Asteroids::iterator otherAstIter = thisSectorAsteroids.begin(); otherAstIter != thisSectorAsteroids.end(); ++otherAstIter)
				{
					Asteroid * const otherAsteroid = *otherAstIter;
					if (asteroid != otherAsteroid)
					{
						const float maxAllowedDistance = otherAsteroid->GetCollisionRadius() +
							asteroid->GetCollisionRadius();

						if ((otherAsteroid->GetPosition() - asteroid->GetPosition()).length() < maxAllowedDistance)
						{
							HandleAsteroidCollision(*asteroid, *otherAsteroid);
						}
					}
				}
			}
			else
			{
				// Check neighbor sectors too
				for (int z = checkZ - 1; z <= checkZ + 1; ++z)
				{
					if (z < 0 || z >= NumberOfSectors)
						continue;

					for (int x = checkX - 1; x <= checkX + 1; ++x)
					{
						if (x < 0 || x >= NumberOfSectors || !m_sectorIsVisible[z][x])
							continue;

						for (Asteroids::iterator otherAstIter = thisSectorAsteroids.begin(); otherAstIter != thisSectorAsteroids.end(); ++otherAstIter)
						{
							Asteroid * const otherAsteroid = *otherAstIter;
							if (asteroid != otherAsteroid)
							{
								const float maxAllowedDistance = otherAsteroid->GetCollisionRadius() +
									asteroid->GetCollisionRadius();

								if ((otherAsteroid->GetPosition() - asteroid->GetPosition()).length() < maxAllowedDistance)
								{
									HandleAsteroidCollision(*asteroid, *otherAsteroid);
								}
							}
						}
					}
				}
			}
		}
	}

	void GameAsteroidManager::HandleAsteroidCollision(Asteroid &asteroid1, Asteroid &asteroid2)
	{
		const float maxAllowedDistance = asteroid1.GetCollisionRadius() + asteroid2.GetCollisionRadius();
		//const float distance = (asteroid2.GetPosition() - asteroid1.GetPosition()).length();

		// Collision point
		const Vector3 middle = asteroid1.GetPosition() * (asteroid2.GetCollisionRadius() / maxAllowedDistance) +
			asteroid2.GetPosition() * (asteroid1.GetCollisionRadius() / maxAllowedDistance);

		// Relative positions to middle
		Vector3 relPos1 = asteroid1.GetPosition() - middle;
		relPos1.normalise();
		Vector3 relPos2 = asteroid2.GetPosition() - middle;
		relPos2.normalise();

		asteroid1.SetPosition(middle + relPos1 * asteroid1.GetCollisionRadius() * 1.015f);
		asteroid2.SetPosition(middle + relPos2 * asteroid2.GetCollisionRadius() * 1.015f);

		// Move speed
		const float speed1 = asteroid1.GetMovement().length();
		const float speed2 = asteroid2.GetMovement().length();

		// Mass = size * size
		const float mass1 = asteroid1.GetSize() * asteroid1.GetSize();
		const float mass2 = asteroid2.GetSize() * asteroid2.GetSize();
		const float bothMasses = mass1 + mass2;

		// Force = speed * mass
		const float force1 = speed1 * mass1;
		const float force2 = speed2 * mass2;
		const float bothForces = force1 + force2;

		// Copy over normals
		const Vector3 normal1 = relPos1;
		const Vector3 normal2 = relPos2;

		// Normalize movement
		Vector3 direction1 = asteroid1.GetMovement();
		if (direction1 != Vector3::ZERO)
			direction1.normalise();
		Vector3 direction2 = asteroid2.GetMovement();
		if (direction2 != Vector3::ZERO)
			direction2.normalise();

		// Collision strength (1 = same dir, 0 = 90 degree)
		float collisionStrength1 = std::abs(direction1.dotProduct(normal1));
		float collisionStrength2 = std::abs(direction2.dotProduct(normal2));

		Vector3 reflection1 = ReflectVector(direction1, normal1);
		Vector3 reflection2 = ReflectVector(direction2, normal2);
		if (direction1.length() <= 0.01f)
		{
			collisionStrength1 = collisionStrength2;
			reflection1 = normal1;
		}
		if (direction2.length() <= 0.01f)
		{
			collisionStrength2 = collisionStrength1;
			reflection2 = normal2;
		}

		float movement1 = (1 - collisionStrength1) * speed1;
		movement1 += collisionStrength1 * (std::abs(mass1 - mass2) * speed1 + (2 * mass2 * speed2)) / bothMasses;
		asteroid1.SetMovement(reflection1 * movement1);

		float movement2 = (1 - collisionStrength2) * speed2;
		movement2 += collisionStrength2 * (std::abs(mass2 - mass1) * speed2 + (2 * mass1 * speed1)) / bothMasses;
		asteroid2.SetMovement(reflection2 * movement2);

		asteroid1.SetRotationSpeed(Vector2(asteroid1.GetRotationSpeed().x,
			(asteroid1.GetRotationSpeed().y + Math::RangeRandom(-0.75f, +1.0f)) / 2.0f));
		asteroid2.SetRotationSpeed(Vector2(asteroid2.GetRotationSpeed().x,
			(asteroid2.GetRotationSpeed().y + Math::RangeRandom(-0.75f, +1.0f)) / 2.0f));
	}

	Vector3 GameAsteroidManager::ReflectVector(const Ogre::Vector3 &vec, const Ogre::Vector3 &normal)
	{
		return (vec - normal * 2.0f * vec.dotProduct(normal));
	}

	void GameAsteroidManager::KillAllInnerSectorAsteroids()
	{
		for (int z = -1; z <= +1; ++z)
		{
			for (int x = -1; x <= +1; ++x)
			{
				// Kill asteroids
				Asteroids &asteroids = m_sectorAsteroids[MiddleSector + z][MiddleSector + x];
				for (Asteroids::iterator asteroid = asteroids.begin(); asteroid != asteroids.end(); ++asteroid)
				{
					delete (*asteroid);
				}
				asteroids.clear();

				// Kill small asteroids
				SmallAsteroids &smallAsteroids = m_sectorSmallerAsteroids[SmallMiddleSector + z][SmallMiddleSector + x];
				for (SmallAsteroids::iterator smallAsteroid = smallAsteroids.begin(); smallAsteroid != smallAsteroids.end(); ++smallAsteroid)
				{
					delete (*smallAsteroid);
				}
				smallAsteroids.clear();
			}
		}
	}

	// 0 if no collision happened, 1 for a frontal collsion.
	float GameAsteroidManager::PlayerAsteroidCollision(Player &player)
	{
		const Vector3 &camPos = m_game.GetCamera().getPosition();
		Vector3 camDir = Vector3(0, 0, -1);
		//camDir = m_game.GetCamera().getViewMatrix().transformAffine(camDir);
		Matrix3 viewMat3;
		m_game.GetCamera().getViewMatrix().extract3x3Matrix(viewMat3);
		camDir = viewMat3 * camDir;
		const Vector3 nextCamPos1 = camPos +
			camDir * 10.0f;
		const Vector3 nextCamPos2 = camPos +
			camDir * (10.0f + player.GetSpeed() * m_game.GetMoveFactorPerSecond() * player.GetMovementSpeedPerSecond());
		float damageFactor = 0.0f;
		float remToPlayWhoshVolume = 0.0f;

		for (int z = -1; z <= +1; ++z)
		{
			for (int x = -1; x <= +1; ++x)
			{
				Asteroids &thisSector = m_sectorAsteroids[MiddleSector + z][MiddleSector + x];
				for (Asteroids::iterator astIter = thisSector.begin(); astIter != thisSector.end();)
				{
					Asteroid &asteroid = *(*astIter);
					bool removeThisAsteroid = false;

					// Check distance to camera
					const float distance = (asteroid.GetPosition() - camPos).length();

					// If distance is smaller than collision radius, we could have
					// a hit. Now do polygon based collision testing.
					if (distance <= asteroid.GetSize() * 0.825157789f)
					{
						// Add 25% than just flying by very closely
						float thisDamageFactor = 0.175f +
							0.25f * std::max(0.0f, 1.0f - (distance / (asteroid.GetCollisionRadius() * 1.025f)));

						// Big asteroids to more damage
						thisDamageFactor *= 0.5f + 0.5f * (asteroid.GetSize() / MaxAsteroidSize);

						// Check highest damange factor
						if (thisDamageFactor > damageFactor)
							damageFactor = thisDamageFactor;

						// Play side hit sound
						// Play only once (asteroid is killed after that)
						//TODO: m_game.GetSound().Play(Sound::Sounds_SideHit);

						// Remeber to show hit direction effect
						Vector3 distVector = asteroid.GetPosition() - camPos;
						distVector = m_game.GetCamera().getViewMatrix().transformAffine(distVector);
						SetHitDirectionEffect(Math::ATan2(distVector.x, distVector.y));

						if (player.GetNumberOfBombItems() > 0)
						{
							player.SetNumberOfBombItems(player.GetNumberOfBombItems() - 1);
							damageFactor = 0.0f;
						}

						// Remove asteroid we hit
						removeThisAsteroid = true;
					} // if (distance)

					const float distanceSoon = (asteroid.GetPosition() - nextCamPos1).length();
					// Check if some asteroids are getting really close
					if (distanceSoon < MaxAsteroidSize * 3.5f)
					{
						// Find out if the asteroid is not longer moving closer
						if (distanceSoon < (asteroid.GetPosition() - nextCamPos2).length())
						{
							// Play one of the whosh sounds
							float loudness = 1.0f -
								((distanceSoon - asteroid.GetCollisionRadius() * 1.5f) /
								(MaxAsteroidSize * 3.5f - asteroid.GetCollisionRadius() * 1.5f));
							loudness *= 1.25f;
							if (loudness > 1.0f)
								loudness = 1.0f;

							if (loudness > 0.1f)
							{
								// Play whosh sound
								const float newVolume = 0.5f + 0.5f * loudness;
								if (newVolume > remToPlayWhoshVolume)
									remToPlayWhoshVolume = newVolume;

								if (distance <= asteroid.GetCollisionRadius() * 2.5f)
								{
									player.SetCameraWobbel(0.33f * (1.0f - (distance / (asteroid.GetCollisionRadius() * 2.0f))));
								}

								// Add a little score for flying so close
								if (m_game.GetTotalFrames() % 20 == 0)
									player.SetScore(player.GetScore() + (int)m_game.GetElapsedTimeThisFrameInMs());
							}
						}
					} // if (distanceSoon)

					if (removeThisAsteroid)
					{
						thisSector.erase(astIter++);
						delete &asteroid;
					}
					else
					{
						astIter++;
					}
				}
			} // for (int x)
		} // for (int z)

		// Max. check every 10 frames, else we have to many whosh sounds
		// TODO
		/*if (remToPlayWhoshVolume > 0 &&
			m_game.GetTotalFrames() % 5 == 0)
			m_game.GetSound().PlayWhosh(remToPlayWhoshVolume);*/
		return damageFactor;
	}

	void GameAsteroidManager::SetLevel(const Level &level)
	{
		m_currentLevel = level;

		// Update target position
		m_targetPos = Vector3(0, 0, (level.GetLength() + 1) * SectorDepth);

		ClearAllItems();
		ClearGoal();

		// Copy over all items
		const Level::Items &items = level.GetItems();
		for (Level::Items::const_iterator itemIter = items.begin(); itemIter != items.end(); ++itemIter)
		{
			const Item::ItemTypes itemType = itemIter->first;
			const Level::ItemPositions &itemPositions = itemIter->second;

			for (Level::ItemPositions::const_iterator itemPosIter = itemPositions.begin(); itemPosIter != itemPositions.end(); ++itemPosIter)
			{
				const Vector3 &itemPos = (*itemPosIter);
				m_items.push_back(new Item(m_game.GetSceneManager(), itemType, itemPos));
			}
		}

		// Goal at target position
		m_goal = new Goal(m_game.GetSceneManager(), m_targetPos);
	}

	void GameAsteroidManager::SetHitDirectionEffect(const Ogre::Radian &direction)
	{
		m_hitDirectionEffectTimeoutMs = MaxHitDirectionTimeoutMs;
		m_hitDirection = direction;
	}

	void GameAsteroidManager::ShowAllItems()
	{
		for (std::list<Item *>::iterator itemIter = m_items.begin(); itemIter != m_items.end();)
		{
			Item &item = *(*itemIter);

			const Vector3 &itemPos = item.GetPosition();

			// Distance to viewer
			const float distance = (itemPos - m_game.GetCamera().getPosition()).length();

			bool itemRemoved = false;

			// Skip if out of visible range
			if (distance > GetMaxViewDepth() * 6)
				goto proceed_to_next_item;

			// If very close to item, collect it
			if (distance <= Item::ItemSize * 2.15f)
			{
				m_game.GetPlayer().HandleItem(item.GetType());

				// Remove item
				delete &item;
				m_items.erase(itemIter++);
				itemRemoved = true;
			}

proceed_to_next_item:
			if (!itemRemoved)
				++itemIter;
		}
	}

	void GameAsteroidManager::ClearAllItems()
	{
		for (std::list<Item *>::iterator item = m_items.begin(); item != m_items.end(); ++item)
		{
			delete *item;
		}
		m_items.clear();
	}

	void GameAsteroidManager::ClearGoal()
	{
		if (m_goal)
			delete m_goal;
		m_goal = NULL;
	}
}
