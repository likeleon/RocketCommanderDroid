#include "AndroidInputInjector.hpp"
#include "Game.hpp"
#include "IGameScreen.hpp"

namespace rcd
{
	AndroidInputInjector::AndroidMultiTouch::AndroidMultiTouch(Ogre::RenderWindow* pRenderWindow)
	: OIS::MultiTouch("DWM", false, 0, 0), m_pRenderWindow(pRenderWindow)
	{
		assert(m_pRenderWindow != NULL);
	}

	OIS::MultiTouchState& AndroidInputInjector::AndroidMultiTouch::getMultiTouchState(int i)
	{
		while(i >= mStates.size())
		{
			OIS::MultiTouchState state;
			state.width = m_pRenderWindow->getWidth();
			state.height = m_pRenderWindow->getHeight();
			mStates.push_back(state);
		}
		return mStates[i];
	}

	AndroidInputInjector::AndroidInputInjector(Ogre::RenderWindow* pRenderWindow)
	: m_pGameScreen(NULL), m_touch(pRenderWindow)
	{
	}

	void AndroidInputInjector::InjectTouchEvent(int action, float x, float y, int pointerId)
	{
		if (m_pGameScreen == NULL)
			return;

		OIS::MultiTouchState &state = m_touch.getMultiTouchState(pointerId);

		switch(action)
		{
			case 0:
				state.touchType = OIS::MT_Pressed;
				break;
			case 1:
				state.touchType = OIS::MT_Released;
				break;
			case 2:
				state.touchType = OIS::MT_Moved;
				break;
			case 3:
				state.touchType = OIS::MT_Cancelled;
				break;
			default:
				state.touchType = OIS::MT_None;
				break;
		}

		if(state.touchType != OIS::MT_None)
		{
			int last = state.X.abs;
			state.X.abs =  (int)x;
			state.X.rel = state.X.abs - last;

			last = state.Y.abs;
			state.Y.abs = (int)y;
			state.Y.rel = state.Y.abs - last;

			state.Z.abs = 0;
			state.Z.rel = 0;

			OIS::MultiTouchEvent evt(&m_touch, state);

			switch(state.touchType)
			{
				case OIS::MT_Pressed:
					m_pGameScreen->touchPressed(evt);
					break;
				case OIS::MT_Released:
					m_pGameScreen->touchReleased(evt);
					break;
				case OIS::MT_Moved:
					m_pGameScreen->touchMoved(evt);
					break;
				case OIS::MT_Cancelled:
					m_pGameScreen->touchCancelled(evt);
					break;
				default:
					break;
			}
		}
	}

	void AndroidInputInjector::SetGameScreen(IGameScreen* pGameScreen)
	{
		m_pGameScreen = pGameScreen;
	}
}
