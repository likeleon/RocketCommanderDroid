#include "Log.hpp"

#include <stdarg.h>
#include <android/log.h>

namespace likeleon
{
	void Log::error(const char* pMessage, ...)
	{
		va_list vargs;
		va_start(vargs, pMessage);
		__android_log_vprint(ANDROID_LOG_ERROR, "LIKELEON", pMessage, vargs);
		__android_log_print(ANDROID_LOG_ERROR, "LIKELEON", "\n");
		va_end(vargs);
	}

	void Log::warn(const char* pMessage, ...)
	{
		va_list vargs;
		va_start(vargs, pMessage);
		__android_log_vprint(ANDROID_LOG_WARN, "LIKELEON", pMessage, vargs);
		__android_log_print(ANDROID_LOG_WARN, "LIKELEON", "\n");
		va_end(vargs);
	}

	void Log::info(const char* pMessage, ...)
	{
		va_list vargs;
		va_start(vargs, pMessage);
		__android_log_vprint(ANDROID_LOG_INFO, "LIKELEON", pMessage, vargs);
		__android_log_print(ANDROID_LOG_INFO, "LIKELEON", "\n");
		va_end(vargs);
	}

	void Log::debug(const char* pMessage, ...)
	{
		va_list vargs;
		va_start(vargs, pMessage);
		__android_log_vprint(ANDROID_LOG_DEBUG, "LIKELEON", pMessage, vargs);
		__android_log_print(ANDROID_LOG_DEBUG, "LIKELEON", "\n");
		va_end(vargs);
	}
}
