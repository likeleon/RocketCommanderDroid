#include "AndroidLogListener.hpp"
#include "Log.hpp"

namespace likeleon
{
	void AndroidLogListener::messageLogged(const Ogre::String& message, Ogre::LogMessageLevel lml, bool maskDebug, const Ogre::String& logName, bool& skipThisMessage)
	{
		if (lml < Ogre::LML_CRITICAL)
		{
			Log::info(message.c_str());
		}
		else
		{
			Log::error(message.c_str());
		}
	}
}
