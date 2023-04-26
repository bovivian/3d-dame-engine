#pragma once

#include <string>

#include "VulkanCommandBuffer.h"

class Texture
{
	private:
		VkImage textureImage;
		VkImageView textureImageView;
		VkDeviceMemory textureMemory;
		int mipMapsCount;
	public:
		Texture();
		~Texture();

		bool Init(VulkanDevice * device, VulkanCommandBuffer * cmdBuffer, std::string filename);
		void Unload(VulkanDevice * vulkanDevice);
		VkImageView * GetImageView();
		int GetMipMapCount();
};