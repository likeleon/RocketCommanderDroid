#ifndef _OGRE_ANDROID_BASE_FRAMEWORK_HPP_
#define _OGRE_ANDROID_BASE_FRAMEWORK_HPP_

#include <Ogre.h>
#include <OgreRenderTargetListener.h>

namespace Ogre
{
	class StaticPluginLoader;
}

namespace likeleon
{
	class AndroidLogListener;

	class OgreAndroidBaseFramework : public Ogre::Singleton<OgreAndroidBaseFramework>, Ogre::FrameListener
	{
	public:
		OgreAndroidBaseFramework();
		virtual ~OgreAndroidBaseFramework();

		bool initOgreRoot();
		void initRenderWindow(void* pWindow, void* pConfig);
		void destroyRenderWindow();

		Ogre::RenderWindow* getRenderWindow();
		Ogre::Root* getOgreRoot();
		void renderOneFrame();

		static OgreAndroidBaseFramework* getSingletonPtr();

		bool frameRenderingQueued(const Ogre::FrameEvent& evt);

	private:
		Ogre::Root* m_pRoot;
		Ogre::StaticPluginLoader* m_pStaticPluginLoader;
		Ogre::RenderWindow* m_pRenderWindow;
		Ogre::Timer m_timer;

		unsigned long m_lastTime;
	};
}

#endif