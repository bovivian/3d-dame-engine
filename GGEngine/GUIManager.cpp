#pragma once
#include <filesystem>
#include <iostream>
#include "GUIManager.h"
#include "LogManager.h"
#include "StdInc.h"
#include "Input.h"

namespace fs = std::experimental::filesystem;

extern LogManager * gLogManager;
extern Input * gInput;

GUIManager::GUIManager()
{
	cursor = NULL;
	cursorPosX = cursorPosY = 0.5f;
	guiEnabled = false;
	cursorEnabled = false;
	guiInventory = false;
	guiMainMenu = false;
}

GUIManager::~GUIManager()
{

}

bool GUIManager::Init(VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer, std::vector<std::string> itemList)
{
	baseDir = "data/textures/GUI/";
	baseItemsDir = "data/items/GUI/";

	cursor = new GUIElement();
	if (!cursor->Init(vulkan, cmdBuffer, baseDir + "cursor.rct"))
	{
		gLogManager->AddMessage("ERROR: Failed to init GUI cursor!");
		return false;
	}
	cursor->SetDimensions(0.02f, 0.04f);

	playerBagIcon = new GUIElement();
	if (!playerBagIcon->Init(vulkan, cmdBuffer, baseDir + "bag_t.rct"))
	{
		gLogManager->AddMessage("ERROR: Failed to init GUI invventory!");
		return false;
	}
	playerBagIcon->SetPosition(0.05f, 0.9f);
	playerBagIcon->SetDimensions(0.06f, 0.07f);

	mainMenu = new GUIElement();
	if (!mainMenu->Init(vulkan, cmdBuffer, baseDir + "mainMenu.rct"))
	{
		gLogManager->AddMessage("ERROR: Failed to init GUI mainMenu!");
		return false;
	}
	mainMenu->SetPosition(0.0f, 0.0f);
	mainMenu->SetDimensions(1.0f, 1.0f);

	startbt = new GUIElement();
	if (!startbt->Init(vulkan, cmdBuffer, baseDir + "startbt.rct"))
	{
		gLogManager->AddMessage("ERROR: Failed to init GUI startbt!");
		return false;
	}
	startbt->SetPosition(0.1f, 0.1f);
	startbt->SetDimensions(0.1f, 0.1f);

	exitbt = new GUIElement();
	if (!exitbt->Init(vulkan, cmdBuffer, baseDir + "exitbt.rct"))
	{
		gLogManager->AddMessage("ERROR: Failed to init GUI startbt!");
		return false;
	}
	exitbt->SetPosition(0.1f, 0.8f);
	exitbt->SetDimensions(0.1f, 0.1f);

	inventory = new GUIElement();
	if (!inventory->Init(vulkan, cmdBuffer, baseDir + "inventory.rct"))
	{
		gLogManager->AddMessage("ERROR: Failed to init GUI startbt!");
		return false;
	}
	inventory->SetPosition(0.1f, 0.1f);
	inventory->SetDimensions(0.8f, 0.8f);

	for (auto & p : fs::directory_iterator(path))
	{
		std::string itemName = p.path().filename().string();

		GUIElement * guiItem = new GUIElement();
		guiItem->SetName(itemName.substr(0, itemName.size() - 4));
		if (!guiItem->Init(vulkan, cmdBuffer, path + itemName))
		{
			gLogManager->AddMessage("ERROR: Failed to init GUI startbt!");
			return false;
		}
		guiItemList.push_back(guiItem);
	}

	return true;
}

void GUIManager::Unload(VulkanInterface * vulkan)
{
	SAFE_UNLOAD(cursor, vulkan);
	SAFE_UNLOAD(playerBagIcon, vulkan);
	SAFE_UNLOAD(mainMenu, vulkan);
	SAFE_UNLOAD(startbt, vulkan);
	SAFE_UNLOAD(exitbt, vulkan);
	SAFE_UNLOAD(inventory, vulkan);

	for (unsigned int i = 0; i < guiItemList.size(); i++)
		SAFE_UNLOAD(guiItemList[i], vulkan);
}

void GUIManager::Update(VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer, VulkanPipeline * pipeline,
	Camera * camera, int frameBufferId, std::vector<Item*> itemList, bool changed)
{
	if (changed)
	{
		inventoryGUI.clear();
		for (int i = 0; i < itemList.size(); i++)
		{
			for (int j = 0; j < guiItemList.size(); j++)
			{
				if (itemList[i]->getItemName() == guiItemList[j]->GetName())
				{
					GUIElement* copyOfItem = new GUIElement();
					copyOfItem->copy(*guiItemList[j], vulkan, cmdBuffer);
					inventoryGUI.push_back(copyOfItem);
				}
			}
		}
	}

	if (cursorEnabled)
	{
		if (gInput->GetCursorRelativeX() != 0)
			cursorPosX += 0.001f * gInput->GetCursorRelativeX();
		if (gInput->GetCursorRelativeY() != 0)
			cursorPosY += 0.001f * gInput->GetCursorRelativeY();

		// Clamp cursor to screen
		if (cursorPosX > 1.0f)
			cursorPosX = 1.0f;
		else if (cursorPosX < 0.0f)
			cursorPosX = 0.0f;
		if (cursorPosY > 1.0f)
			cursorPosY = 1.0f;
		else if (cursorPosY < 0.0f)
			cursorPosY = 0.0f;
		cursor->Render(vulkan, cmdBuffer, pipeline, camera, frameBufferId);
		cursor->SetPosition(cursorPosX, cursorPosY);

	}
	if (guiMainMenu)
	{
		mainMenu->Render(vulkan, cmdBuffer, pipeline, camera, frameBufferId);
		startbt->Render(vulkan, cmdBuffer, pipeline, camera, frameBufferId);
		exitbt->Render(vulkan, cmdBuffer, pipeline, camera, frameBufferId);
		if (gInput->GetCursorRelativeX() != 0)
			cursorPosX += 0.001f * gInput->GetCursorRelativeX();
		if (gInput->GetCursorRelativeY() != 0)
			cursorPosY += 0.001f * gInput->GetCursorRelativeY();

		// Clamp cursor to screen
		if (cursorPosX > 1.0f)
			cursorPosX = 1.0f;
		else if (cursorPosX < 0.0f)
			cursorPosX = 0.0f;
		if (cursorPosY > 1.0f)
			cursorPosY = 1.0f;
		else if (cursorPosY < 0.0f)
			cursorPosY = 0.0f;
		cursor->Render(vulkan, cmdBuffer, pipeline, camera, frameBufferId);
		cursor->SetPosition(cursorPosX, cursorPosY);
	}
	if (guiEnabled)
	{
		playerBagIcon->Render(vulkan, cmdBuffer, pipeline, camera, frameBufferId);
	}
	if (guiInventory)
	{
		inventory->Render(vulkan, cmdBuffer, pipeline, camera, frameBufferId);

		for (int i = 0, j = 0, k=0, o=0; i < inventoryGUI.size(); i++)
		{
			if (i > 4 && i <= 9)
			{
				inventoryGUI[i]->SetPosition(inventory->GetPositionX() + (float)(j / 7.0f + 0.05f), inventory->GetPositionY() + 0.25f);
				j++;
			}
			else if (i > 9 && i <= 14)
			{
				inventoryGUI[i]->SetPosition(inventory->GetPositionX() + (float)(k / 7.0f + 0.05f), inventory->GetPositionY() + 0.45f);
				k++;
			}
			else if (i > 14 && i <= 19)
			{
				inventoryGUI[i]->SetPosition(inventory->GetPositionX() + (float)(o / 7.0f + 0.05f), inventory->GetPositionY() + 0.65f);
				o++;
			}
			else
			{
				inventoryGUI[i]->SetPosition(inventory->GetPositionX() + (float)(i / 7.0f + 0.05f), inventory->GetPositionY() + 0.05f);
			}
			inventoryGUI[i]->SetDimensions(0.1f, 0.1f);
			inventoryGUI[i]->Render(vulkan, cmdBuffer, pipeline, camera, frameBufferId);
		}
	}
}

void GUIManager::ToggleGUI(bool toggle)
{
	guiEnabled = toggle;
}

void GUIManager::ToggleMainMenuGUI(bool toggle) {
	guiMainMenu = toggle;
}

void GUIManager::ToggleInventoryGUI(bool toggle) {
	guiInventory = toggle;
}

bool GUIManager::getToggleInventoryGUI() {
	return guiInventory;
}

void GUIManager::ToggleCursor(bool toggle)
{
	cursor->SetPosition(0.5f, 0.5f);
	cursorEnabled = toggle;
}

float GUIManager::GetCursorPosX()
{
	return cursorPosX;
}

void GUIManager::SetCursorPosition()
{
	cursorPosX = cursorPosY = 0.5f;
}

float GUIManager::GetCursorPosY()
{
	return cursorPosY;
}

bool GUIManager::CursorOnButton(GUIElement* element, GUIElement* cursor)
{
	if ((GetCursorPosX() > element->GetPositionX()) && (GetCursorPosX() < (element->GetDimensionX() + element->GetPositionX())))
		if ((GetCursorPosY() > element->GetPositionY()) && (GetCursorPosY() < (element->GetDimensionY() + element->GetPositionY())))
			if (gInput->WasKeyPressed(MOUSE_KEY_L))
				return true;

	return false;
}

GUIElement* GUIManager::getCursor()
{
	return cursor;
}

GUIElement* GUIManager::getMainMenu()
{
	return mainMenu;
}

GUIElement* GUIManager::getStartbt()
{
	return startbt;
}

GUIElement* GUIManager::getExitbt()
{
	return exitbt;
}