#pragma once
#include <string>
#include <vector>
#include"Item.h"

class Inventory
{
private:
	Item ** itemArray;
	unsigned numOfItems;
	unsigned capacity;

	//Private functions

	void nullify(const unsigned from = 0);
	void freeMemory();

public:
	Inventory();
	~Inventory();

	void Init(unsigned capacity);

	//Functions
	unsigned getCapacity();
	unsigned getNumOfItems();

	void clear();
	bool empty();

	bool add(Item* item);
	bool remove(unsigned index);

	std::vector<Item*> getAllItems();
};

