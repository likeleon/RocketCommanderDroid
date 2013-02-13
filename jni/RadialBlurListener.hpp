#ifndef _RADIAL_BLUR_LISTENER_HPP_
#define _RADIAL_BLUR_LISTENER_HPP_

#include <OgreCompositorLogic.h>
#include "Player.hpp"

namespace rcd
{
	class RadialBlurListener : public Ogre::CompositorInstance::Listener
	{
	public:
		RadialBlurListener(const Player &player);

		virtual void notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);
	    virtual void notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);

	private:
		Ogre::GpuProgramParametersSharedPtr fpParams;
		const Player &m_player;
	};
}

#endif
