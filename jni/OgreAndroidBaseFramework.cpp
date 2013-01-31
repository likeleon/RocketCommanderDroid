#include "OgreAndroidBaseFramework.hpp"
#include "AndroidLogListener.hpp"
#include "Log.hpp"
#include <OgrePlatform.h>
#include <OgreOverlaySystem.h>

// Static plugins declaration section
// Note that every entry in here adds an extra header / library dependency
#ifdef OGRE_STATIC_LIB
#  ifdef OGRE_BUILD_RENDERSYSTEM_GL
#    define OGRE_STATIC_GL
#  endif
#  ifdef OGRE_BUILD_RENDERSYSTEM_GLES
#    define OGRE_STATIC_GLES
#    undef USE_RTSHADER_SYSTEM
#  endif
#  ifdef OGRE_BUILD_RENDERSYSTEM_GLES2
#undef OGRE_STATIC_GLES
#    define USE_RTSHADER_SYSTEM
#    define OGRE_STATIC_GLES2
#  endif
#  if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
#    ifdef OGRE_BUILD_RENDERSYSTEM_D3D9
#		define OGRE_STATIC_Direct3D9
#    endif
// dx11 will only work on vista and above, so be careful about statically linking
#    ifdef OGRE_BUILD_RENDERSYSTEM_D3D11
#      define OGRE_STATIC_Direct3D11
#    endif
#  endif

#  ifdef OGRE_BUILD_PLUGIN_BSP
#  define OGRE_STATIC_BSPSceneManager
#  endif
#  ifdef OGRE_BUILD_PLUGIN_PFX
#  define OGRE_STATIC_ParticleFX
#  endif
#  ifdef OGRE_BUILD_PLUGIN_CG
#  define OGRE_STATIC_CgProgramManager
#  endif

#  ifdef OGRE_USE_PCZ
#    ifdef OGRE_BUILD_PLUGIN_PCZ
#    define OGRE_STATIC_PCZSceneManager
#    define OGRE_STATIC_OctreeZone
#    endif
#  else
#    ifdef OGRE_BUILD_PLUGIN_OCTREE
#    define OGRE_STATIC_OctreeSceneManager
#  endif
#     endif
#  include "OgreStaticPluginLoader.h"
#endif

namespace likeleon
{
	template <> OgreAndroidBaseFramework* Ogre::Singleton<OgreAndroidBaseFramework>::msSingleton = NULL;

	OgreAndroidBaseFramework* OgreAndroidBaseFramework::getSingletonPtr()
	{
		if (msSingleton)
			return msSingleton;
	}

	OgreAndroidBaseFramework::OgreAndroidBaseFramework()
	: m_pRoot(NULL), m_pStaticPluginLoader(NULL), m_pRenderWindow(NULL), m_pOverlaySystem(NULL), m_lastTime(0)
	{
	}

	OgreAndroidBaseFramework::~OgreAndroidBaseFramework()
	{
		destroyRenderWindow();

		if (m_pOverlaySystem)
		{
			delete m_pOverlaySystem;
		}

		if (m_pRoot)
		{
			delete m_pRoot;
		}

		if (m_pStaticPluginLoader)
		{
			m_pStaticPluginLoader->unload();
			delete m_pStaticPluginLoader;
		}
	}

	bool OgreAndroidBaseFramework::initOgreRoot()
	{
		try
		{
			// Create a root object
			m_pRoot = new Ogre::Root();

			m_pStaticPluginLoader = new Ogre::StaticPluginLoader();
			m_pStaticPluginLoader->load();

			m_pOverlaySystem = new Ogre::OverlaySystem();

			// Grab the available render systems
			const Ogre::RenderSystemList& renderSystemList = m_pRoot->getAvailableRenderers();
			if (renderSystemList.empty())
				return false;

			// Set the render system and init
			m_pRoot->setRenderSystem(renderSystemList.at(0));
			m_pRoot->initialise(false);

			m_lastTime = m_timer.getMilliseconds();
			return true;

		}
		catch (Ogre::Exception &e)
		{
			return false;
		}

		return false;
	}

	void OgreAndroidBaseFramework::destroyRenderWindow()
	{
		if (m_pRenderWindow)
		{
			m_pRenderWindow->destroy();
			Ogre::Root::getSingleton().detachRenderTarget(m_pRenderWindow);
			m_pRenderWindow = NULL;
		}
	}

	Ogre::RenderWindow* OgreAndroidBaseFramework::getRenderWindow()
	{
		return m_pRenderWindow;
	}

	Ogre::Root* OgreAndroidBaseFramework::getOgreRoot()
	{
		return m_pRoot;
	}

	Ogre::OverlaySystem* OgreAndroidBaseFramework::getOverlaySystem()
	{
		return m_pOverlaySystem;
	}

	void OgreAndroidBaseFramework::renderOneFrame()
	{
		m_pRoot->renderOneFrame();
	}

	void OgreAndroidBaseFramework::initRenderWindow(void* pWindow, void* pConfig)
	{
		if (!m_pRenderWindow)
		{
			Ogre::NameValuePairList params;
			params["externalWindowHandle"] = Ogre::StringConverter::toString((int)pWindow);
			params["androidConfig"] = Ogre::StringConverter::toString((int)pConfig);

			m_pRenderWindow = m_pRoot->createRenderWindow("OgreAndroidPrimary", 0, 0, false, &params);
		}
	}
}
