#ifndef _SOUND_HPP_
#define _SOUND_HPP_

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <Ogre.h>
#include <android/asset_manager.h>

namespace rcd
{
	class Sound
	{
	public:
		typedef enum
		{
			Sounds_Bomb = 0,
			Sounds_Click,
			Sounds_Defeat,
			Sounds_Explosion,
			Sounds_ExtraLife,
			Sounds_Fuel,
			Sounds_Health,
			Sounds_Highlight,
			Sounds_GameMusic,
			Sounds_MenuMusic,
			Sounds_RocketMotor,
			Sounds_SideHit,
			Sounds_Speed,
			Sounds_Victory,
			Sounds_Whosh1,
			Sounds_Whosh2,
			Sounds_Whosh3,
			Sounds_Whosh4,
			MaxSounds
		} Sounds;

		struct SoundSetting
		{
			SoundSetting(Sounds sound, const Ogre::String &fileName, bool looped)
				: m_sound(sound), m_fileName(fileName), m_looped(looped) {}

			Sounds			m_sound;
			Ogre::String	m_fileName;
			bool			m_looped;
		};

		struct SoundBuffer
		{
			char* m_pBuffer;
			size_t m_length;
		};

		Sound(AAssetManager& assetManager);
		~Sound();

		void Update();
		void Play(Sounds sound);
		void SetCurrentMusicMode(bool inGame);

	private:
		static const SoundSetting SoundSettings[];

		void LoadSoundBuffers();
		void UnloadSoundBuffers();
		bool PlayBGM(std::string filePath);
		void StopBGM();

		AAssetManager& m_assetManager;
		SLObjectItf m_pEngineObj;
		SLEngineItf m_pEngine;
		SLObjectItf m_pOutputMixObj;
		SLObjectItf m_pBGMPlayerObj;
		SLPlayItf m_pBGMPlayer;
		SLSeekItf m_pBGMPlayerSeek;
		SoundBuffer m_soundBuffers[MaxSounds];
	};
}

#endif
