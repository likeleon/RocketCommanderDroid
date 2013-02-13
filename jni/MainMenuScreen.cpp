#include "MainMenuScreen.hpp"
#include "Game.hpp"
#include "SpaceCamera.hpp"
#include <OgreMaterialManager.h>
#include "MissionScreen.hpp"

namespace rcd
{
	MainMenuScreen::MainMenuScreen(Game &game)
	: m_game(game), m_resetCamera(true), m_pBackground(NULL), m_pOverlay(NULL)
	{
	}

	void MainMenuScreen::Enter()
	{
		if (m_resetCamera)
		{
			m_game.GetCamera().setPosition(Ogre::Vector3::ZERO);
			m_game.GetSpaceCamera().SetInGame(false);
			m_resetCamera = false;
		}

		// Create background material
		Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().create("Background", "GameResource");
		material->getTechnique(0)->getPass(0)->createTextureUnitState("Background.png");
		material->getTechnique(0)->getPass(0)->setDepthCheckEnabled(false);
		material->getTechnique(0)->getPass(0)->setDepthWriteEnabled(false);
		material->getTechnique(0)->getPass(0)->setLightingEnabled(false);
		material->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_DEST_COLOUR);

		// Create background rectangle covering the whole screen
		m_pBackground = new Ogre::Rectangle2D(true);
		m_pBackground->setCorners(-1.0, 1.0, 1.0, -1.0);
		m_pBackground->setMaterial("Background");

		// Render the background before everything else
		m_pBackground->setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY - 1);

		// Use infinite AAB to always stay visible
		Ogre::AxisAlignedBox aabInf;
		aabInf.setInfinite();
		m_pBackground->setBoundingBox(aabInf);

		// Attach background to the scene
		m_pBackgroundNode = m_game.GetSceneManager().getRootSceneNode()->createChildSceneNode("Background");
		m_pBackgroundNode->attachObject(m_pBackground);

		// Create a main menu panel
		Ogre::OverlayManager& overlayManager = Ogre::OverlayManager::getSingleton();
		m_pOverlay = overlayManager.getByName("MainMenuOverlay");
		m_pOverlay->show();
	}

	void MainMenuScreen::Exit()
	{
		m_pOverlay->hide();

		m_pBackgroundNode->detachAllObjects();
		assert(m_pBackgroundNode->getParentSceneNode() == m_game.GetSceneManager().getRootSceneNode());
		m_pBackgroundNode->getParentSceneNode()->removeAndDestroyChild(m_pBackgroundNode->getName());
		m_pBackgroundNode = NULL;

		delete m_pBackground;
		m_pBackground = NULL;
	}

	bool MainMenuScreen::touchMoved(const OIS::MultiTouchEvent &arg)
	{
		return true;
	}

	bool MainMenuScreen::touchPressed(const OIS::MultiTouchEvent &arg)
	{
		m_game.AddGameScreen(new MissionScreen(m_game, m_game.GetLevel(0)));
		return true;
	}

	bool MainMenuScreen::touchReleased(const OIS::MultiTouchEvent &arg)
	{
		return true;
	}

	bool MainMenuScreen::touchCancelled(const OIS::MultiTouchEvent &arg)
	{
		return true;
	}
}
