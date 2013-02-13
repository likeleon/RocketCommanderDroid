#include "RadialBlurListener.hpp"

using namespace Ogre;

namespace rcd
{
	RadialBlurListener::RadialBlurListener(const Player &player)
	: m_player(player)
	{
	}


	void RadialBlurListener::notifyMaterialSetup(uint32 pass_id, MaterialPtr &mat)
	{
		if (pass_id == 700)
		{
			fpParams = mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
		}
	}

	void RadialBlurListener::notifyMaterialRender(uint32 pass_id, MaterialPtr &mat)
	{
		if (pass_id == 700)
		{
			fpParams->setNamedConstant("sampleStrength", m_player.GetSpeed() * 3.0f);
		}
	}
}
