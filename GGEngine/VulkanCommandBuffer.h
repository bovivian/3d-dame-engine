#pragma once

#include "VulkanDevice.h"
#include "VulkanCommandPool.h"

class VulkanCommandBuffer
{
	private:
		VkFence fence;
		VkCommandBuffer commandBuffer;
		bool primary;
	public:
		VulkanCommandBuffer();
		~VulkanCommandBuffer();

		bool Init(VulkanDevice * vulkanDevice, VulkanCommandPool * vulkanCommandPool, bool primary);
		void Unload(VulkanDevice * vulkanDevice, VulkanCommandPool * vulkanCommandPool);
		void BeginRecording();
		void BeginRecordingSecondary(VkRenderPass renderPass, VkFramebuffer framebuffer);
		void EndRecording();
		void Execute(VulkanDevice * device, VkPipelineStageFlags flags, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, bool waitFence);
		void ExecuteSecondary(VulkanCommandBuffer * primaryCmdBuffer);
		VkCommandBuffer GetCommandBuffer();
};