#pragma once

#include <string>

class Item 
{
private:

	std::string itemName;
	int itemRarity;
	bool itemOnMap = 1;

public:

	Item();
	~Item();

	void Init(std::string name, int rarity);

	std::string getItemName();
	int getItemRarity();
	bool getOnMap();

	void setItemName(std::string name);
	void setItemRarity(int rarity);
	void setOnMap(bool onMap);
};