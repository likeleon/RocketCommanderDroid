#include "Item.hpp"

using namespace Ogre;

namespace rcd
{
	const float Item::ItemSize = 75.0f;

	Item::Item(SceneManager &sceneMgr, ItemTypes type, const Vector3 &position)
	: Model(sceneMgr, GetMeshName(type), GetDownScaling(type)), m_type(type)
	{
		SetPosition(position);
		SetScale(Vector3(ItemSize));
	}

	Ogre::String Item::GetMeshName(ItemTypes type)
	{
		assert(type >= ItemType_Fuel && type < NumOfItemTypes);
		static const Ogre::String meshNames[NumOfItemTypes] =
		{
			"Fuel",
			"Health",
			"ExtraLife",
			"Speed",
			"Bomb"
		};
		return meshNames[type];
	}

	float Item::GetDownScaling(ItemTypes type)
	{
		assert(type >= ItemType_Fuel && type < NumOfItemTypes);
		static const float downScalings[NumOfItemTypes] =
		{
			0.006876214f,
			0.009066273f,
			0.0125944568f,
			0.0146259107f,
			0.0168181341f
		};
		return downScalings[type];
	}
}
