#include "RocketCommanderDroid.hpp"
#include "OgreAndroidBaseFramework.hpp"
#include "Log.hpp"
#include <math.h>
#include <EGL/egl.h>
#include "Android/OgreAndroidEGLWindow.h"
#include "Android/OgreAPKFileSystemArchive.h"
#include "Android/OgreAPKZipArchive.h"

namespace rcd
{
	RocketCommanderDroid::RocketCommanderDroid(likeleon::Context& context, android_app* pApplication)
	: m_pApplication(pApplication), m_initialized(false)
	{
		likeleon::Log::info("Creating RocketCommanderDroid");
	}

	RocketCommanderDroid::~RocketCommanderDroid()
	{
		likeleon::Log::info("Destructing RocketCommanderDroid");
	}

	void RocketCommanderDroid::onInit()
	{
		likeleon::Log::info("Initializing RocketCommanderDroid");

		if (m_initialized)
			return;

		new likeleon::OgreAndroidBaseFramework();

		if (!likeleon::OgreAndroidBaseFramework::getSingletonPtr()->initOgreRoot())
			return;

		m_initialized = true;
	}

	void RocketCommanderDroid::onDestroy()
	{
		likeleon::Log::info("Destroying RocketCommanderDroid");

		if (!m_initialized)
			return;

		m_initialized = false;

		delete likeleon::OgreAndroidBaseFramework::getSingletonPtr();
	}

	likeleon::status RocketCommanderDroid::onActivate()
	{
		likeleon::Log::info("Activating RocketCommanderDroid");
		return likeleon::STATUS_OK;
	}

	void RocketCommanderDroid::onDeactivate()
	{
		likeleon::Log::info("Deactivating RocketCommanderDroid");
	}

	static Ogre::SceneNode* s_pLightPivot = NULL;

	likeleon::status RocketCommanderDroid::onStep()
	{
		if (s_pLightPivot != NULL)
			s_pLightPivot->yaw(Ogre::Degree((Ogre::Real)33 / 30));

		likeleon::OgreAndroidBaseFramework* pFramework = likeleon::OgreAndroidBaseFramework::getSingletonPtr();
		if (pFramework)
			pFramework->renderOneFrame();

		return likeleon::STATUS_OK;
	}

	static Ogre::DataStreamPtr OpenAPKFile(AAssetManager* pAssetMgr, const Ogre::String& fileName)
	{
		Ogre::DataStreamPtr stream;
		AAsset* pAsset = AAssetManager_open(pAssetMgr, fileName.c_str(), AASSET_MODE_BUFFER);
		if (pAsset)
		{
			off_t length = AAsset_getLength(pAsset);
			void* membuf = OGRE_MALLOC(length, Ogre::MEMCATEGORY_GENERAL);
			memcpy(membuf, AAsset_getBuffer(pAsset), length);
			AAsset_close(pAsset);

			stream = Ogre::DataStreamPtr(new Ogre::MemoryDataStream(membuf, length, true, true));
		}
		return stream;
	}

	void RocketCommanderDroid::onCreateWindow()
	{
		likeleon::OgreAndroidBaseFramework* pFramework = likeleon::OgreAndroidBaseFramework::getSingletonPtr();

		if (!m_pApplication->window || !pFramework)
			return;

		AConfiguration* pConfig = AConfiguration_new();
		AAssetManager* pAssetMgr = m_pApplication->activity->assetManager;
		AConfiguration_fromAssetManager(pConfig, pAssetMgr);

		Ogre::RenderWindow* pRenderWindow = pFramework->getRenderWindow();
		if (!pRenderWindow)
		{
			pFramework->initRenderWindow(m_pApplication->window, pConfig);
		}
		else
		{
			static_cast<Ogre::AndroidEGLWindow*>(pRenderWindow)->_createInternalResources(m_pApplication->window, pConfig);
		}

		AConfiguration_delete(pConfig);

		Ogre::ArchiveManager::getSingleton().addArchiveFactory(new Ogre::APKFileSystemArchiveFactory(pAssetMgr));
		Ogre::ArchiveManager::getSingleton().addArchiveFactory(new Ogre::APKZipArchiveFactory(pAssetMgr));

		// Locate resources
		Ogre::ConfigFile cfg;
		cfg.load(OpenAPKFile(pAssetMgr, "resources.cfg"));
		Ogre::ConfigFile::SectionIterator seci = cfg.getSectionIterator();
		Ogre::String sec, type, arch;

		// go through all specified resource groups
		while (seci.hasMoreElements())
		{
			sec = seci.peekNextKey();
			Ogre::ConfigFile::SettingsMultiMap* settings = seci.getNext();
			Ogre::ConfigFile::SettingsMultiMap::iterator i;

			// go through all resource paths
			for (i = settings->begin(); i != settings->end(); i++)
			{
				type = i->first;
				arch = i->second;
				Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch, type, sec);
			}
		}

		Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

		// Create scene manager
		Ogre::SceneManager* pSceneMgr = pFramework->getOgreRoot()->createSceneManager(Ogre::ST_GENERIC, "SceneManager");
		pSceneMgr->setAmbientLight(Ogre::ColourValue::White);

		// Light
		Ogre::Light* pLight = pSceneMgr->createLight();
		pLight->setPosition(0, 0, -150);
		pLight->setDiffuseColour(Ogre::ColourValue::Black);
		pLight->setSpecularColour(Ogre::ColourValue::Black);

		s_pLightPivot = pSceneMgr->getRootSceneNode()->createChildSceneNode();
		s_pLightPivot->attachObject(pLight);

		// Camera
		Ogre::Camera* pCamera = pSceneMgr->createCamera("Camera");
		pCamera->setPosition(0, 0, -280);
		pCamera->lookAt(0, 0, 0);
		pCamera->setNearClipDistance(0.1);
		pCamera->setFarClipDistance(50000);

		// Viewport
		Ogre::Viewport* pViewport = pFramework->getRenderWindow()->addViewport(pCamera);
		pViewport->setBackgroundColour(Ogre::ColourValue(0, 0, 0));
		pCamera->setAspectRatio(Ogre::Real(pViewport->getActualWidth()) / Ogre::Real(pViewport->getActualHeight()));
		pViewport->setCamera(pCamera);

		// Skybox
		pSceneMgr->setSkyBox(true, "RocketCommander/SpaceSkyBox", 10, true);

		// Mesh
		pSceneMgr->createEntity("rocket", "rocket.mesh");
		Ogre::SceneNode* pMeshNode = pSceneMgr->getRootSceneNode()->createChildSceneNode();
		pMeshNode->attachObject(pSceneMgr->getEntity("rocket"));
		pMeshNode->scale(0.5, 0.5, 0.5);
		pMeshNode->yaw(Ogre::Radian(Ogre::Math::PI / 2.0f));
	}

	void RocketCommanderDroid::onDestroyWindow()
	{
		likeleon::OgreAndroidBaseFramework* pFramework = likeleon::OgreAndroidBaseFramework::getSingletonPtr();
		if (!pFramework)
			return;

		Ogre::RenderWindow* pRenderWindow = pFramework->getRenderWindow();
		if (pRenderWindow)
			static_cast<Ogre::AndroidEGLWindow*>(pRenderWindow)->_destroyInternalResources();
	}
}
