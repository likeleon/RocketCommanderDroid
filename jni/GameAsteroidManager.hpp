#ifndef _GAME_ASTEROID_MANAGER_HPP_
#define _GAME_ASTEROID_MANAGER_HPP_

namespace rcd
{
	class GameAsteroidManager
	{
	public:
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

	private:
		static const float MinViewDepth;
		static float MaxViewDepth;
		static float HalfViewDepth;
	};
}

#endif
