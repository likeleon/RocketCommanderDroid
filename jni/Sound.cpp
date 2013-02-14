#include "Sound.hpp"
#include "Log.hpp"

namespace rcd
{
	const Sound::SoundSetting Sound::SoundSettings[MaxSounds] =
	{
		Sound::SoundSetting(Sounds_Bomb,		"Bomb.wav",			false),
		Sound::SoundSetting(Sounds_Click,		"Click.wav",		false),
		Sound::SoundSetting(Sounds_Defeat,		"Defeat.wav",		false),
		Sound::SoundSetting(Sounds_Explosion,	"Explosion.wav",	false),
		Sound::SoundSetting(Sounds_ExtraLife,	"ExtraLife.wav",	false),
		Sound::SoundSetting(Sounds_Fuel,		"Fuel.wav",			false),
		Sound::SoundSetting(Sounds_Health,		"Health.wav",		false),
		Sound::SoundSetting(Sounds_Highlight,	"Highlight.wav",	false),
		Sound::SoundSetting(Sounds_GameMusic,	"Rocket Commander Game Music.wav", true),
		Sound::SoundSetting(Sounds_MenuMusic,	"Rocket Commander Menu Music.wav", true),
		Sound::SoundSetting(Sounds_RocketMotor,	"RocketMotor.wav",	true),
		Sound::SoundSetting(Sounds_SideHit,		"SideHit.wav",		false),
		Sound::SoundSetting(Sounds_Speed,		"Speed.wav",		false),
		Sound::SoundSetting(Sounds_Victory,		"Victory.wav",		false),
		Sound::SoundSetting(Sounds_Whosh1,		"Whosh1.wav",		false),
		Sound::SoundSetting(Sounds_Whosh2,		"Whosh2.wav",		false),
		Sound::SoundSetting(Sounds_Whosh3,		"Whosh3.wav",		false),
		Sound::SoundSetting(Sounds_Whosh4,		"Whosh4.wav",		false)
	};

	Sound::Sound(AAssetManager& assetManager)
	: m_assetManager(assetManager), m_pEngineObj(NULL), m_pEngine(NULL), m_pOutputMixObj(NULL)
	, m_pBGMPlayerObj(NULL), m_pBGMPlayer(NULL), m_pBGMPlayerSeek(NULL)
	{
		likeleon::Log::debug("Creating Sound");

		const SLuint32      lEngineMixIIDCount = 1;
		const SLInterfaceID lEngineMixIIDs[]   = {SL_IID_ENGINE};
		const SLboolean     lEngineMixReqs[]   = {SL_BOOLEAN_TRUE};
		const SLuint32      lOutputMixIIDCount = 0;
		const SLInterfaceID lOutputMixIIDs[]   = {};
		const SLboolean     lOutputMixReqs[]   = {};

		SLresult res = slCreateEngine(&m_pEngineObj, 0, NULL, lEngineMixIIDCount, lEngineMixIIDs, lEngineMixReqs);
		if (res != SL_RESULT_SUCCESS)
			goto ERROR;

		res = (*m_pEngineObj)->Realize(m_pEngineObj, SL_BOOLEAN_FALSE);
		if (res != SL_RESULT_SUCCESS)
			goto ERROR;

		res = (*m_pEngineObj)->GetInterface(m_pEngineObj, SL_IID_ENGINE, &m_pEngine);
		if (res != SL_RESULT_SUCCESS)
			goto ERROR;

		(*m_pEngine)->CreateOutputMix(m_pEngine, &m_pOutputMixObj, lOutputMixIIDCount, lOutputMixIIDs, lOutputMixReqs);
		(*m_pOutputMixObj)->Realize(m_pOutputMixObj, SL_BOOLEAN_FALSE);

		LoadSoundBuffers();
ERROR:
		assert(!"Error while creating Sound");
	}

	Sound::~Sound()
	{
		likeleon::Log::debug("Destroying Sound");

		StopBGM();

		UnloadSoundBuffers();

		if (m_pOutputMixObj != NULL)
		{
			(*m_pOutputMixObj)->Destroy(m_pOutputMixObj);
			m_pOutputMixObj = NULL;
		}

		if (m_pEngineObj != NULL)
		{
			(*m_pEngineObj)->Destroy(m_pEngineObj);
			m_pEngineObj = NULL;
			m_pEngine = NULL;
		}
	}

	void Sound::LoadSoundBuffers()
	{
		for (int i = 0; i < MaxSounds; ++i)
		{
			Sounds sounds = static_cast<Sounds>(i);

			std::string filePath = "sounds/" + SoundSettings[sounds].m_fileName;
			AAsset* m_pAsset = AAssetManager_open(&m_assetManager, filePath.c_str(), AASSET_MODE_UNKNOWN);
			assert(m_pAsset != NULL);

			SoundBuffer& soundBuffer = m_soundBuffers[sounds];
			soundBuffer.m_length = AAsset_getLength(m_pAsset);
			soundBuffer.m_pBuffer = new char[soundBuffer.m_length];
			int res = AAsset_read(m_pAsset, soundBuffer.m_pBuffer, soundBuffer.m_length);
			assert(res >= 0);

			AAsset_close(m_pAsset);
		}
	}

	void Sound::UnloadSoundBuffers()
	{
		for (int i = 0; i < MaxSounds; ++i)
		{
			Sounds sounds = static_cast<Sounds>(i);
			delete m_soundBuffers[sounds].m_pBuffer;
			m_soundBuffers[sounds].m_pBuffer = NULL;
			m_soundBuffers[sounds].m_length = 0;
		}
	}

	bool Sound::PlayBGM(std::string filePath)
	{
		AAsset* m_pAsset = AAssetManager_open(&m_assetManager, filePath.c_str(), AASSET_MODE_UNKNOWN);
		assert(m_pAsset != NULL);

		struct ResourceDescriptor
		{
			int m_descriptor;
			off_t m_start;
			off_t m_length;
		};

		ResourceDescriptor descriptor;
		descriptor.m_descriptor = AAsset_openFileDescriptor(m_pAsset, &descriptor.m_start, &descriptor.m_length);
		assert(descriptor.m_descriptor >= 0);

		SLDataLocator_AndroidFD dataLocatorIn;
		dataLocatorIn.locatorType = SL_DATALOCATOR_ANDROIDFD;
		dataLocatorIn.fd = descriptor.m_descriptor;
		dataLocatorIn.offset = descriptor.m_start;
		dataLocatorIn.length = descriptor.m_length;

		SLDataFormat_MIME dataFormat;
		dataFormat.formatType = SL_DATAFORMAT_MIME;
		dataFormat.mimeType = NULL;
		dataFormat.containerType = SL_CONTAINERTYPE_UNSPECIFIED;

		SLDataSource dataSource;
		dataSource.pLocator = &dataLocatorIn;
		dataSource.pFormat  = &dataFormat;

		SLDataLocator_OutputMix dataLocatorOut;
		dataLocatorOut.locatorType = SL_DATALOCATOR_OUTPUTMIX;
		dataLocatorOut.outputMix   = m_pOutputMixObj;

		SLDataSink dataSink;
		dataSink.pLocator = &dataLocatorOut;
		dataSink.pFormat  = NULL;

		// Creates BGM player and retrieves its interfaces.
		const SLuint32 BGMPlayerIIDCount = 2;
		const SLInterfaceID BGMPlayerIIDs[] = { SL_IID_PLAY, SL_IID_SEEK };
		const SLboolean BGMPlayerReqs[] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };

		SLresult res = (*m_pEngine)->CreateAudioPlayer(m_pEngine, &m_pBGMPlayerObj, &dataSource, &dataSink, BGMPlayerIIDCount, BGMPlayerIIDs, BGMPlayerReqs);
		if (res != SL_RESULT_SUCCESS)
			goto ERROR;

		res = (*m_pBGMPlayerObj)->Realize(m_pBGMPlayerObj, SL_BOOLEAN_FALSE);
		if (res != SL_RESULT_SUCCESS)
			goto ERROR;

		res = (*m_pBGMPlayerObj)->GetInterface(m_pBGMPlayerObj, SL_IID_PLAY, &m_pBGMPlayer);
		if (res != SL_RESULT_SUCCESS)
			goto ERROR;

		res = (*m_pBGMPlayerObj)->GetInterface(m_pBGMPlayerObj, SL_IID_SEEK, &m_pBGMPlayerSeek);
		if (res != SL_RESULT_SUCCESS)
			goto ERROR;

		// Enables looping and starts playing.
		res = (*m_pBGMPlayerSeek)->SetLoop(m_pBGMPlayerSeek, SL_BOOLEAN_TRUE, 0, SL_TIME_UNKNOWN);
		if (res != SL_RESULT_SUCCESS)
			goto ERROR;

		res = (*m_pBGMPlayer)->SetPlayState(m_pBGMPlayer, SL_PLAYSTATE_PLAYING);
		if (res != SL_RESULT_SUCCESS)
			goto ERROR;

		AAsset_close(m_pAsset);

ERROR:
		return false;
	}

	void Sound::StopBGM()
	{
		if (m_pBGMPlayer != NULL)
		{
			SLuint32 BGMPlayerState = 0;
			(*m_pBGMPlayerObj)->GetState(m_pBGMPlayerObj, &BGMPlayerState);

			if (BGMPlayerState == SL_OBJECT_STATE_REALIZED)
			{
				(*m_pBGMPlayer)->SetPlayState(m_pBGMPlayer, SL_PLAYSTATE_PAUSED);

				(*m_pBGMPlayerObj)->Destroy(m_pBGMPlayerObj);
				m_pBGMPlayerObj = NULL;
				m_pBGMPlayer = NULL;
				m_pBGMPlayerSeek = NULL;
			}
		}
	}

	void Sound::Update()
	{
	}

	void Sound::Play(Sounds sound)
	{
		const SoundSetting& setting = SoundSettings[sound];
		if (setting.m_looped)
		{
			StopBGM();
			PlayBGM(std::string("sounds/") + setting.m_fileName);
		}
		else
		{
		}
	}

	void Sound::SetCurrentMusicMode(bool inGame)
	{
		Play(inGame? Sound::Sounds_GameMusic : Sound::Sounds_MenuMusic);
	}
}
