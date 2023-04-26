#pragma once
#include <iostream>
#include "Inventory.h"


Inventory::Inventory()
{
	capacity = 0;
}

void Inventory::Init(unsigned capacity)
{
	this->capacity = capacity;
	this->numOfItems = 0;

	this->itemArray = new Item*[capacity];
	this->nullify();
}

void Inventory::nullify(const unsigned from)
{
	for (size_t i = from; i < this->capacity; i++)
	{
		this->itemArray[i] = nullptr;
	}
}

void Inventory::freeMemory()
{
	for (size_t i = 0; i < this->numOfItems; i++)
	{
		delete this->itemArray[i];
	}
}

Inventory::~Inventory()
{
	this->freeMemory();
}

//Accessors
unsigned Inventory::getCapacity()
{
	return capacity;
}

unsigned Inventory::getNumOfItems()
{
	return numOfItems;
}


//Functions

void Inventory::clear()
{
	for (size_t i = 0; i < numOfItems; i++)
	{
		delete this->itemArray[i];
	}

	numOfItems = 0;

	nullify();
}

bool Inventory::empty()
{
	return numOfItems == 0;
}

bool Inventory::add(Item * item)
{
	if (numOfItems < capacity)
	{
		itemArray[numOfItems++] = item;

		return true;
	}

	return false;
}

bool Inventory::remove(unsigned index)
{
	if (numOfItems > 0)
	{
		if (index < 0 || index >= capacity)
			return false;

		delete itemArray[index];
		itemArray[index] = nullptr;
		--numOfItems;

		return true;
	}

	return false;
}

std::vector<Item*> Inventory::getAllItems()
{
	std::vector<Item*> itemsInventory;
	for (size_t i = 0; i < this->numOfItems; i++)
	{
		itemsInventory.push_back(itemArray[i]);
	}
	return std::move(itemsInventory);
}