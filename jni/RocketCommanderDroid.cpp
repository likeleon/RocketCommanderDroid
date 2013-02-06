#include "RocketCommanderDroid.hpp"
#include "OgreAndroidBaseFramework.hpp"
#include "Log.hpp"
#include <math.h>
#include <EGL/egl.h>
#include "Android/OgreAndroidEGLWindow.h"
#include "Android/OgreAPKFileSystemArchive.h"
#include "Android/OgreAPKZipArchive.h"
#include "Game.hpp"
#include "AndroidInputInjector.hpp"

namespace rcd
{
	RocketCommanderDroid::RocketCommanderDroid(likeleon::Context& context, android_app* pApplication)
	: m_pApplication(pApplication), m_initialized(false), m_pGame(NULL), m_pInputInjector(NULL)
	{
		likeleon::Log::info("Creating RocketCommanderDroid");
	}

	RocketCommanderDroid::~RocketCommanderDroid()
	{
		likeleon::Log::info("Destructing RocketCommanderDroid");
	}

	void RocketCommanderDroid::onInit()
	{
		likeleon::Log::info("Initializing RocketCommanderDroid");

		if (m_initialized)
			return;

		likeleon::OgreAndroidBaseFramework* pFramework = new likeleon::OgreAndroidBaseFramework();
		if (!pFramework->initOgreRoot())
			return;

		m_initialized = true;
	}

	void RocketCommanderDroid::onDestroy()
	{
		likeleon::Log::info("Destroying RocketCommanderDroid");

		if (!m_initialized)
			return;

		m_initialized = false;

		delete likeleon::OgreAndroidBaseFramework::getSingletonPtr();
	}

	likeleon::status RocketCommanderDroid::onActivate()
	{
		likeleon::Log::info("Activating RocketCommanderDroid");
		return likeleon::STATUS_OK;
	}

	void RocketCommanderDroid::onDeactivate()
	{
		likeleon::Log::info("Deactivating RocketCommanderDroid");
	}

	likeleon::status RocketCommanderDroid::onStep()
	{
		likeleon::OgreAndroidBaseFramework* pFramework = likeleon::OgreAndroidBaseFramework::getSingletonPtr();
		if (pFramework)
			pFramework->renderOneFrame();

		return likeleon::STATUS_OK;
	}

	static Ogre::DataStreamPtr OpenAPKFile(AAssetManager* pAssetMgr, const Ogre::String& fileName)
	{
		Ogre::DataStreamPtr stream;
		AAsset* pAsset = AAssetManager_open(pAssetMgr, fileName.c_str(), AASSET_MODE_BUFFER);
		if (pAsset)
		{
			off_t length = AAsset_getLength(pAsset);
			void* membuf = OGRE_MALLOC(length, Ogre::MEMCATEGORY_GENERAL);
			memcpy(membuf, AAsset_getBuffer(pAsset), length);
			AAsset_close(pAsset);

			stream = Ogre::DataStreamPtr(new Ogre::MemoryDataStream(membuf, length, true, true));
		}
		return stream;
	}

	void RocketCommanderDroid::onCreateWindow()
	{
		likeleon::OgreAndroidBaseFramework* pFramework = likeleon::OgreAndroidBaseFramework::getSingletonPtr();

		if (!m_pApplication->window || !pFramework)
			return;

		AConfiguration* pConfig = AConfiguration_new();
		AAssetManager* pAssetMgr = m_pApplication->activity->assetManager;
		AConfiguration_fromAssetManager(pConfig, pAssetMgr);

		Ogre::RenderWindow* pRenderWindow = pFramework->getRenderWindow();
		if (!pRenderWindow)
		{
			pFramework->initRenderWindow(m_pApplication->window, pConfig);
		}
		else
		{
			static_cast<Ogre::AndroidEGLWindow*>(pRenderWindow)->_createInternalResources(m_pApplication->window, pConfig);
		}

		AConfiguration_delete(pConfig);

		Ogre::ArchiveManager::getSingleton().addArchiveFactory(new Ogre::APKFileSystemArchiveFactory(pAssetMgr));
		Ogre::ArchiveManager::getSingleton().addArchiveFactory(new Ogre::APKZipArchiveFactory(pAssetMgr));

		// Locate resources
		Ogre::ConfigFile cfg;
		cfg.load(OpenAPKFile(pAssetMgr, "resources.cfg"));
		Ogre::ConfigFile::SectionIterator seci = cfg.getSectionIterator();
		Ogre::String sec, type, arch;

		// go through all specified resource groups
		while (seci.hasMoreElements())
		{
			sec = seci.peekNextKey();
			Ogre::ConfigFile::SettingsMultiMap* settings = seci.getNext();
			Ogre::ConfigFile::SettingsMultiMap::iterator i;

			// go through all resource paths
			for (i = settings->begin(); i != settings->end(); i++)
			{
				type = i->first;
				arch = i->second;
				Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch, type, sec);
			}
		}

		Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Essential");

		m_pInputInjector = new AndroidInputInjector(pFramework->getRenderWindow());

		m_pGame = new Game(*pFramework->getOgreRoot(), *pFramework->getRenderWindow(), *pFramework->getOverlaySystem(), *pAssetMgr, *m_pInputInjector);
		m_pGame->Initialize();
	}

	void RocketCommanderDroid::onDestroyWindow()
	{
		if (m_pGame)
		{
			m_pGame->Cleanup();
			delete m_pGame;
			m_pGame = NULL;
		}

		if (m_pInputInjector)
		{
			delete m_pInputInjector;
			m_pInputInjector = NULL;
		}

		likeleon::OgreAndroidBaseFramework* pFramework = likeleon::OgreAndroidBaseFramework::getSingletonPtr();
		if (pFramework)
		{
			Ogre::RenderWindow* pRenderWindow = pFramework->getRenderWindow();
			if (pRenderWindow)
				static_cast<Ogre::AndroidEGLWindow*>(pRenderWindow)->_destroyInternalResources();
		}
	}

	int RocketCommanderDroid::onInputEvent(AInputEvent* pEvent)
	{
		if (m_pInputInjector != NULL)
		{
			if (AInputEvent_getType(pEvent) == AINPUT_EVENT_TYPE_MOTION)
			{
				int action = (int)(AMOTION_EVENT_ACTION_MASK & AMotionEvent_getAction(pEvent));

				if(action == 0)
					m_pInputInjector->InjectTouchEvent(2, AMotionEvent_getRawX(pEvent, 0), AMotionEvent_getRawY(pEvent, 0));

				m_pInputInjector->InjectTouchEvent(action, AMotionEvent_getRawX(pEvent, 0), AMotionEvent_getRawY(pEvent, 0));
			}

			return 1;
		}

		return 0;
	}
}
