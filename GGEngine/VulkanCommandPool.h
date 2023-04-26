#pragma once

#include "VulkanDevice.h"

class VulkanCommandPool
{
	private:
		VkCommandPool commandPool;
	public:
		VulkanCommandPool();
		~VulkanCommandPool();

		bool Init(VulkanDevice * vulkanDevice);
		void Unload(VulkanDevice * vulkanDevice);
		VkCommandPool GetCommandPool();
};