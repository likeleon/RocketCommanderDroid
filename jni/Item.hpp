#ifndef _ITEM_HPP_
#define _ITEM_HPP_

#include "Model.hpp"

namespace rcd
{
	class Item : public Model
	{
	public:
		enum ItemTypes
		{
			ItemType_Fuel,
			ItemType_Health,
			ItemType_ExtraLife,
			ItemType_Speed,
			ItemType_Bomb,
			NumOfItemTypes
		};

		static const float ItemSize;

		Item(Ogre::SceneManager &sceneMgr, ItemTypes type, const Ogre::Vector3 &position);
		ItemTypes GetType() const { return m_type; }

	private:
		static Ogre::String GetMeshName(ItemTypes type);
		static float GetDownScaling(ItemTypes type);

		ItemTypes m_type;
	};
}

#endif
