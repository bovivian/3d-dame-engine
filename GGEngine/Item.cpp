#include "StdInc.h"
#include "Item.h"

Item::Item()
{
	itemName = "";
	itemRarity = 0;
}

Item::~Item()
{
	itemName = "";
	itemRarity = NULL;
}

void Item::Init(std::string name, int rarity)
{
	itemName = name;
	itemRarity = rarity;
}

std::string Item::getItemName()
{
	return itemName;
}
int Item::getItemRarity()
{
	return itemRarity;
}
bool Item::getOnMap()
{
	return itemOnMap;
}

void Item::setItemName(std::string name)
{
	itemName = name;
}
void Item::setItemRarity(int rarity)
{
	itemRarity = rarity;
}
void Item::setOnMap(bool onMap)
{
	itemOnMap = onMap;
}