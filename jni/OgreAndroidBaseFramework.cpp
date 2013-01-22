#include "OgreAndroidBaseFramework.hpp"
#include "AndroidLogListener.hpp"

namespace likeleon
{
	template <> OgreAndroidBaseFramework* Ogre::Singleton<OgreAndroidBaseFramework>::msSingleton = NULL;

	OgreAndroidBaseFramework* OgreAndroidBaseFramework::getSingletonPtr()
	{
		if (msSingleton)
			return msSingleton;
	}

	OgreAndroidBaseFramework::OgreAndroidBaseFramework()
	: m_pRoot(NULL), m_pGles2Plugin(NULL), m_pRenderWindow(NULL), m_pAndroidLogListener(NULL), m_pLogManager(NULL)
	, m_lastTime(0)
	{
	}

	OgreAndroidBaseFramework::~OgreAndroidBaseFramework()
	{
		destroyRenderWindow();

		if (m_pRoot)
			delete m_pRoot;
		if (m_pGles2Plugin)
			delete m_pGles2Plugin;
		if (m_pLogManager)
			delete m_pLogManager;
	}

	bool OgreAndroidBaseFramework::initOgreRoot()
	{
		try
		{
			// Create logs that funnel to android logs
			m_pLogManager = OGRE_NEW Ogre::LogManager();
			Ogre::Log* pLog = m_pLogManager->createLog("AndroidLog", true, true, true);
			m_pAndroidLogListener = new AndroidLogListener();
			pLog->addListener(m_pAndroidLogListener);

			// Create a root object
			m_pRoot = new Ogre::Root("", "", "");

			// Register the ES2 plugin
			m_pGles2Plugin = new Ogre::GLES2Plugin();
			Ogre::Root::getSingleton().installPlugin(m_pGles2Plugin);

			// Grab the available render systems
			const Ogre::RenderSystemList& renderSystemList = m_pRoot->getAvailableRenderers();
			if (renderSystemList.empty())
				return false;

			// Set the render system and init
			Ogre::RenderSystem* pSystem = renderSystemList.front();
			m_pRoot->setRenderSystem(pSystem);
			m_pRoot->initialise(false);

			m_lastTime = m_timer.getMilliseconds();

			m_pRoot->addFrameListener(this);
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

	void OgreAndroidBaseFramework::renderOneFrame()
	{
		m_pRoot->renderOneFrame();
	}

	bool OgreAndroidBaseFramework::frameRenderingQueued(const Ogre::FrameEvent& evt)
	{
		return true;
	}

	void OgreAndroidBaseFramework::initRenderWindow(unsigned int windowHandle, unsigned int width, unsigned int height, unsigned int contextHandle)
	{
		if (m_pRenderWindow)
		{
			Ogre::NameValuePairList params;
			params["externalWindowHandle"] = Ogre::StringConverter::toString(windowHandle);
			params["externalGLContext"] = Ogre::StringConverter::toString(contextHandle);

			m_pRenderWindow = m_pRoot->createRenderWindow("OgreAndroidPrimary", width, height, true, &params);
		}
	}
}
