#include "RocketCommanderDroid.hpp"
#include "Log.hpp"

#include <math.h>

namespace rcd
{
	RocketCommanderDroid::RocketCommanderDroid(likeleon::Context& context, android_app* pApplication)
	: m_pApplication(pApplication), m_pTimeService(context.m_pTimeService), m_initialized(false)
	, m_posX(0), m_posY(0), m_size(24), m_speed(100.0f)
	{
		likeleon::Log::info("Creating RocketCommanderDroid");
	}

	RocketCommanderDroid::~RocketCommanderDroid()
	{
		likeleon::Log::info("Destructing RocketCommanderDroid");
	}

	likeleon::status RocketCommanderDroid::onActivate()
	{
		likeleon::Log::info("Activating RocketCommanderDroid");
		m_pTimeService->reset();

		ANativeWindow* pWindow = m_pApplication->window;
		if (ANativeWindow_setBuffersGeometry(pWindow, 0, 0, WINDOW_FORMAT_RGBX_8888) < 0)
			return likeleon::STATUS_KO;

		if (ANativeWindow_lock(pWindow, &m_windowBuffer, NULL) >= 0)
			ANativeWindow_unlockAndPost(pWindow);
		else
			return likeleon::STATUS_KO;

		if (!m_initialized)
		{
			m_posX = m_windowBuffer.width / 2;
			m_posY = m_windowBuffer.height / 2;
			m_initialized = true;
		}

		return likeleon::STATUS_OK;
	}

	void RocketCommanderDroid::onDeactivate()
	{
		likeleon::Log::info("Deactivating RocketCommanderDroid");
	}

	likeleon::status RocketCommanderDroid::onStep()
	{
		m_pTimeService->update();

		m_posX = fmod(m_posX + m_speed * m_pTimeService->elapsed(), m_windowBuffer.width);

		ANativeWindow* pWindow = m_pApplication->window;
		if (ANativeWindow_lock(pWindow, &m_windowBuffer, NULL) >= 0)
		{
			clear();
			drawCursor(m_size, m_posX, m_posY);
			ANativeWindow_unlockAndPost(pWindow);
			return likeleon::STATUS_OK;
		}
		else
		{
			return likeleon::STATUS_KO;
		}
	}

	void RocketCommanderDroid::clear()
	{
		memset(m_windowBuffer.bits, 0, m_windowBuffer.stride * m_windowBuffer.height * sizeof(uint32_t*));
	}

	void RocketCommanderDroid::drawCursor(int size, int x, int y)
	{
		const int halfSize = size / 2;

		const int upLeftX = x - halfSize;
		const int upLeftY = y - halfSize;
		const int downRightX = x + halfSize;
		const int downRightY = y + halfSize;

		uint32_t* pLine = ((uint32_t*)m_windowBuffer.bits) + (m_windowBuffer.stride * upLeftY);
		for (int iy = upLeftY; iy <= downRightY; ++iy)
		{
			for (int ix = upLeftX; ix <= downRightX; ++ix)
			{
				pLine[ix] = 255;
			}
			pLine = pLine + m_windowBuffer.stride;
		}
	}
}
