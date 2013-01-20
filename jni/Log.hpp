#ifndef _LOG_HPP_
#define _LOG_HPP_

namespace likeleon
{
	class Log
	{
	public:
		static void error(const char* pMessage, ...);
		static void warn(const char* pMessage, ...);
		static void info(const char* pMessage, ...);
		static void debug(const char* pMessage, ...);
	};
}

#ifndef NDEBUG
	#define likeleon_Log_debug(...)	likeleon::Log::debug(__VA_ARGS__)
#else
	#define likeleon_Log_debug(...)
#endif

#endif
