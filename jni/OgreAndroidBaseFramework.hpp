#ifndef _OGRE_ANDROID_BASE_FRAMEWORK_HPP_
#define _OGRE_ANDROID_BASE_FRAMEWORK_HPP_

#include <Ogre.h>
#include <OgreGLES2Plugin.h>
#include <OgreRenderTargetListener.h>

namespace likeleon
{
	class AndroidLogListener;

	class OgreAndroidBaseFramework : public Ogre::Singleton<OgreAndroidBaseFramework>, Ogre::FrameListener
	{
	public:
		OgreAndroidBaseFramework();
		virtual ~OgreAndroidBaseFramework();

		bool initOgreRoot();
		void initRenderWindow(unsigned int windowHandle, unsigned int width, unsigned int height, unsigned int contextHandle);
		void destroyRenderWindow();
		Ogre::RenderWindow* getRenderWindow();
		void renderOneFrame();

		static OgreAndroidBaseFramework* getSingletonPtr();

		bool frameRenderingQueued(const Ogre::FrameEvent& evt);

	private:
		Ogre::Root* m_pRoot;
		Ogre::GLES2Plugin* m_pGles2Plugin;
		Ogre::RenderWindow* m_pRenderWindow;
		AndroidLogListener* m_pAndroidLogListener;
		Ogre::LogManager* m_pLogManager;
		Ogre::Timer m_timer;

		unsigned long m_lastTime;
	};
}

#endif
