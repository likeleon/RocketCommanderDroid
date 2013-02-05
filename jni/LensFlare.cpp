#include "LensFlare.hpp"
#include "Game.hpp"
#include "ColourHelper.hpp"
#include "Sprite.hpp"

using namespace Ogre;

namespace rcd
{
	const Vector3 LensFlare::DefaultSunPos = Vector3(-2500000, +2250000 , -1500000);

	Vector3 LensFlare::lensOrigin3D = Vector3(0, 0, 0);

	const String LensFlare::flareTextureNames[LensFlare::NumberOfFlareTypes] =
	{
		"Sun.dds",
		"Glow.dds",
		"Lens.dds",
		"Streaks.dds",
		"Ring.dds",
		"Halo.dds",
		"Circle.dds"
	};

	int LensFlare::screenFlareSize = 225;

	const LensFlare::FlareData LensFlare::flareDatas[] =
	{
		// Smal red/yellow/gray halo behind sun
		FlareData(FlareType_Circle, 1.2f, 0.55f, ColourValue(0.68f, 0.68f, 0.68f, 0.2f)),

		// The sun, sun + streaks + glow + red ring
		FlareData(FlareType_Sun, 1.0f, 0.9f, ColourValue(1.0f, 1.0f, 1.0f, 1.0f)),
		FlareData(FlareType_Streaks, 1.0f, 1.8f, ColourValue(1.0f, 1.0f, 1.0f, 0.5f)),
		FlareData(FlareType_Glow, 1.0f, 2.6f, ColourValue(1.0f, 1.0f, 0.78f, 0.58f)),

		// 3 Blue circles at 0.5 distance
		FlareData(FlareType_Circle, 0.5f, 0.12f, ColourValue(0.24f, 0.24f, 0.71f, 0.14f)),
		FlareData(FlareType_Circle, 0.45f, 0.46f, ColourValue(0.39f, 0.39f, 0.78f, 0.24f)),
		FlareData(FlareType_Circle, 0.4f, 0.17f, ColourValue(0.47f, 0.47f, 0.86f, 0.16f)),

		FlareData(FlareType_Ring, 0.15f, 0.2f, ColourValue(0.24f, 0.24f, 1.0f, 0.39f)),
		FlareData(FlareType_Lens, -0.5f, 0.2f, ColourValue(1.0f, 0.24f, 0.24f, 0.51f)),
		FlareData(FlareType_Lens, -0.15f, 0.15f, ColourValue(1.0f, 0.24f, 0.24f, 0.35f)),
		FlareData(FlareType_Halo, -0.3f, 0.6f, ColourValue(0.24f, 0.24f, 1.0f, 0.71f)),

		// 3 red halos and circles on the opposite side of the blue halos
		FlareData(FlareType_Halo, -0.4f, 0.2f, ColourValue(0.86f, 0.31f, 0.31f, 0.38f)),
		FlareData(FlareType_Circle, -0.5f, 0.1f, ColourValue(0.86f, 0.31f, 0.31f, 0.33f)),

		FlareData(FlareType_Halo, -0.6f, 0.5f, ColourValue(0.24f, 0.24f, 1.0f, 0.31f)),
		FlareData(FlareType_Ring, -0.8f, 0.3f, ColourValue(0.35f, 0.24f, 1.0f, 0.43f)),

		FlareData(FlareType_Halo, -0.95f, 0.5f, ColourValue(0.24f, 0.24f, 1.0f, 0.47f)),
		FlareData(FlareType_Circle, -1.0f, 0.15f, ColourValue(0.24f, 0.24f, 1.0f, 0.33f)),
	};

	LensFlare::LensFlare(Game &game, const Vector3 &lensOrigin3D)
	: m_game(game), sunIntensity(0.0f)
	{
		LensFlare::lensOrigin3D = lensOrigin3D;
		LoadTextures();
	}

	LensFlare::~LensFlare()
	{
		// TODO: Unload textures?
	}

	void LensFlare::LoadTextures()
	{
		for (int flareType = 0; flareType < NumberOfFlareTypes; ++flareType)
		{
			m_game.GetSprite().LoadSprite(LensFlare::flareTextureNames[flareType]);
		}
	}

	void LensFlare::Render(ColourValue sunColor)
	{
		const Camera &camera = m_game.GetCamera();

		LensFlare::screenFlareSize = 225 * m_game.GetWidth() / 1024;
		const Vector3 relativeLensPos = LensFlare::lensOrigin3D - camera.getRealPosition();

		// Only show lens flare if facing in the right direction
		if (m_game.IsInFrontOfCamera(relativeLensPos) == false)
			return;

		// Convert 3D point to 2D
		Vector2 lensOrigin = m_game.Convert3DPointTo2D(relativeLensPos);

		// Check sun occlusion intensity and fade it in and out
		float thisSunIntensity = 0.85f;
		sunIntensity = thisSunIntensity * 0.1f + sunIntensity * 0.9f;

		// We can skip rendering the sun if the intensity is to low
		if (sunIntensity < 0.01f)
			return;

		int resWidth = m_game.GetWidth();
		int resHeight = m_game.GetHeight();
		Vector2 center = Vector2((Real)resWidth / 2, (Real)resHeight / 2);
		Vector2 relOrigin = Vector2(center.x - lensOrigin.x, center.y - lensOrigin.y);

		// Check if origin is on screen, fade out at borders
		float alpha = 1.0f;
		float distance = std::abs(std::max(relOrigin.x, relOrigin.y));
		if (distance > resHeight / 1.75f)
		{
			distance -= resHeight / 1.75f;
			// If distance is more than half the resolution, don't show anything
			if (distance > resHeight / 1.75f)
				return;
			alpha = 1.0f - (distance / ((float)resHeight / 1.75f));
			if (alpha > 1.0f)
				alpha = 1.0f;
		}

		// Use square of sunIntensity for lens flares because we want
		// them to get very weak if sun is not fully visible.
		alpha *= sunIntensity * sunIntensity;
		sunColor = ColourHelper::InterpolateColour(ColourValue::White, sunColor, 0.5f);

		m_game.GetSprite().SetScreenMetric(Sprite::OSPRITE_METRIC_PIXELS);

		for (int i = 0; i < sizeof(LensFlare::flareDatas) / sizeof(LensFlare::flareDatas[0]); ++i)
		{
			const FlareData &data = LensFlare::flareDatas[i];

			float size = LensFlare::screenFlareSize * data.m_scale;
			const Real left = center.x - relOrigin.x * data.m_position - size / 2;
			const Real top = center.y - relOrigin.y * data.m_position - size / 2;
			Sprite::Rect destRect(left, top, left + size, top + size);

			ColourValue drawColour = sunColor * data.m_colour;
			drawColour.saturate();

			float drawAlpha = data.m_colour.a;
			if (data.m_type == FlareType_Sun || data.m_type == FlareType_Glow)
				drawAlpha *= sunIntensity;
			else
				drawAlpha *= alpha;

			m_game.GetSprite().DrawSprite(LensFlare::flareTextureNames[data.m_type],
				destRect, Sprite::FULL_SPRITE, drawColour, drawAlpha, true);
		}
	}

	Vector3 LensFlare::RotateSun(Radian rotation)
	{
		Vector3 sunPos = DefaultSunPos;
		const Quaternion q = Quaternion(Radian(rotation), Vector3::UNIT_X) *
			Quaternion(Radian(rotation), Vector3::UNIT_Z);
		return q * sunPos;
	}
}
