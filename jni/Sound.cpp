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

	Sound::Sound()
	: m_pEngineObj(NULL), m_pEngine(NULL), m_pOutputMixObj(NULL)
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

ERROR:
		assert(!"Error while creating Sound");
	}

	Sound::~Sound()
	{
		likeleon::Log::debug("Destroying Sound");

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

	void Sound::Update()
	{
	}

	void Sound::Play(Sounds sound)
	{
	}

	void Sound::SetCurrentMusicMode(bool inGame)
	{
		Play(inGame? Sound::Sounds_GameMusic : Sound::Sounds_MenuMusic);
	}
}
