#include "GameAsteroidManager.hpp"

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
}
