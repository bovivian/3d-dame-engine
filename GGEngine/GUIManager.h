#include "GUIElement.h"
#include "Camera.h"
#include "Item.h"

#pragma once

class GUIManager
{
private:
	std::string baseDir;
	std::string baseItemsDir;
	std::string path = "data/items/GUI/";

	bool guiEnabled;
	bool cursorEnabled;
	bool guiInventory;
	bool guiMainMenu;

	float cursorPosX, cursorPosY;
	GUIElement * cursor;
	GUIElement * playerBagIcon;
	GUIElement * mainMenu;
	GUIElement * startbt;
	GUIElement * exitbt;
	GUIElement * inventory;
	std::vector<GUIElement*> guiItemList;
	std::vector<GUIElement*> inventoryGUI;

public:
	GUIManager();
	~GUIManager();

	bool Init(VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer, std::vector<std::string> itemList);
	void Unload(VulkanInterface * vulkan);
	void Update(VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer, VulkanPipeline * pipeline,
		Camera * camera, int frameBufferId, std::vector<Item*> itemList, bool changed);
	void ToggleGUI(bool toggle);
	void ToggleMainMenuGUI(bool toggle);
	void ToggleInventoryGUI(bool toggle);
	bool getToggleInventoryGUI();
	void ToggleCursor(bool toggle);
	void SetCursorPosition();
	float GetCursorPosX();
	float GetCursorPosY();
	bool CursorOnButton(GUIElement* element, GUIElement* cursor);
	GUIElement* getCursor();
	GUIElement* getMainMenu();
	GUIElement* getStartbt();
	GUIElement* getExitbt();
};
