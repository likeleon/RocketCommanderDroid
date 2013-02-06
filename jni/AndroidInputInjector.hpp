#ifndef _ANDROID_INPUT_INJECTOR_HPP_
#define _ANDROID_INPUT_INJECTOR_HPP_

#include <OISMultiTouch.h>
#include <Ogre.h>

namespace rcd
{
	class IGameScreen;

	class AndroidInputInjector
	{
	public:
		AndroidInputInjector(Ogre::RenderWindow* pRenderWindow);
		void InjectTouchEvent(int action, float x, float y, int pointerId = 0);
		void SetGameScreen(IGameScreen* pGameScreen);

		OIS::MultiTouch& GetMultiTouch() { return m_touch; }

	private:
		class AndroidMultiTouch : public OIS::MultiTouch
		{
		public:
			AndroidMultiTouch(Ogre::RenderWindow* pRenderWindow);

			virtual void setBuffered(bool buffered) {}
			virtual void capture(){}
			virtual OIS::Interface* queryInterface(OIS::Interface::IType type) { return 0; }
			virtual void _initialize() {}
		    OIS::MultiTouchState& getMultiTouchState(int i);

		private:
			Ogre::RenderWindow* m_pRenderWindow;
		};

		IGameScreen* m_pGameScreen;
		AndroidMultiTouch m_touch;
	};
}

#endif
