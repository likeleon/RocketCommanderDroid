#include "RocketCommanderDroid.hpp"
#include "OgreAndroidBaseFramework.hpp"
#include "Log.hpp"
#include <math.h>
#include <EGL/egl.h>
#include "Android/OgreAndroidEGLWindow.h"

namespace rcd
{
	RocketCommanderDroid::RocketCommanderDroid(likeleon::Context& context, android_app* pApplication)
	: m_pApplication(pApplication), m_initialized(false)
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

		new likeleon::OgreAndroidBaseFramework();

		if (!likeleon::OgreAndroidBaseFramework::getSingletonPtr()->initOgreRoot())
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

	void RocketCommanderDroid::onCreateWindow()
	{
		likeleon::OgreAndroidBaseFramework* pFramework = likeleon::OgreAndroidBaseFramework::getSingletonPtr();

		if (!m_pApplication->window || !pFramework)
			return;

		AConfiguration* pConfig = AConfiguration_new();
		AConfiguration_fromAssetManager(pConfig, m_pApplication->activity->assetManager);

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
	}

	void RocketCommanderDroid::onDestroyWindow()
	{
		likeleon::OgreAndroidBaseFramework* pFramework = likeleon::OgreAndroidBaseFramework::getSingletonPtr();
		if (!pFramework)
			return;

		Ogre::RenderWindow* pRenderWindow = pFramework->getRenderWindow();
		if (pRenderWindow)
			static_cast<Ogre::AndroidEGLWindow*>(pRenderWindow)->_destroyInternalResources();
	}
}
