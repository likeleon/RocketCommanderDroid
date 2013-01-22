#ifndef _ANDROID_LOG_LISTENER_HPP_
#define _ANDROID_LOG_LISTENER_HPP_

#include <OgreLog.h>

namespace likeleon
{
	class AndroidLogListener : public Ogre::LogListener
	{
	public:
		virtual void messageLogged(const Ogre::String& message, Ogre::LogMessageLevel lml, bool maskDebug, const Ogre::String& logName, bool& skipThisMessage);
	};
}

#endif
