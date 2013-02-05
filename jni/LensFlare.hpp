#ifndef _LENS_FLARE_HPP_
#define _LENS_FLARE_HPP_

#include <Ogre.h>

namespace rcd
{
	class Game;

	class LensFlare
	{
	public:
		LensFlare(Game &game, const Ogre::Vector3 &lensOrigin3D);
		~LensFlare();

		static Ogre::Vector3 RotateSun(Ogre::Radian rotation);
		static void SetOrigin3D(const Ogre::Vector3 origin) { lensOrigin3D = origin; }

		void Render(Ogre::ColourValue sunColor);

		static const Ogre::Vector3 DefaultSunPos;

	private:
		enum FlareTextureTypes
		{
			FlareType_Sun = 0,
			FlareType_Glow = 1,
			FlareType_Lens = 2,
			FlareType_Streaks = 3,
			FlareType_Ring = 4,
			FlareType_Halo = 5,
			FlareType_Circle = 6,
			NumberOfFlareTypes
		};

		struct FlareData
		{
			FlareTextureTypes m_type;	// type of flare
			float m_position;			// position of flare (1=origin, 0=center of screen, -1=other side)
			float m_scale;				// scale of flare in relatio to MaxFlareSize
			Ogre::ColourValue m_colour;	// color of this flare

			FlareData(FlareTextureTypes type, float position, float scale, const Ogre::ColourValue &colour)
			: m_type(type), m_position(position), m_scale(scale), m_colour(colour)
			{
			}
		};

		void LoadTextures();

		static Ogre::Vector3 lensOrigin3D;
		static const Ogre::String flareTextureNames[];
		static int screenFlareSize;
		static const FlareData flareDatas[];

		Game &m_game;
		float sunIntensity;
	};
}

#endif
