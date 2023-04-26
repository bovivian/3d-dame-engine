#include "Texture.h"
#include "Canvas.h"
#include "Camera.h"

#pragma once

class GUIElement
{
	private:
		std::string name;
		Texture * texture;
		Canvas * canvas;
	public:
		std::string baseItemsDir = "data/items/GUI/";
		GUIElement();
		void copy(const GUIElement &source, VulkanInterface* vulkan, VulkanCommandBuffer* cmdBuffer);
		GUIElement& operator=(const GUIElement& rhs) {};

		bool Init(VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer, std::string filename);
		void Unload(VulkanInterface * vulkan);
		void Render(VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer, VulkanPipeline * pipeline,
			Camera * camera, int frameBufferId);
		void SetPosition(float x, float y);
		void SetDimensions(float width, float height);
		void GUIElement::SetName(std::string _name);
		std::string GUIElement::GetName();

		float GetDimensionX();
		float GetDimensionY();
		float GetPositionX();
		float GetPositionY();
};
