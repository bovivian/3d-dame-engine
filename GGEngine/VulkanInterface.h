#pragma once

#define VULKAN_DEBUG_MODE_ENABLED false

#define VK_USE_PLATFORM_WIN32_KHR

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanCommandPool.h"
#include "VulkanCommandBuffer.h"
#include "VulkanSwapchain.h"
#include "VulkanRenderpass.h"
#include "FrameBufferAttachment.h"

class VulkanInterface
{
	private:
		struct
		{
			VkFormat format;
			VkImage image;
			VkDeviceMemory mem;
			VkImageView view;
		} depthImage;

		VulkanInstance * vulkanInstance;
		VulkanDevice * vulkanDevice;
		VulkanCommandPool * vulkanCommandPool;
		VulkanCommandBuffer * initCommandBuffer;
		VulkanSwapchain * vulkanSwapchain;
		VulkanRenderpass * forwardRenderPass;
		VulkanRenderpass * deferredRenderPass;

		VkViewport viewport;
		VkRect2D scissor;

		VkSampler colorSampler;
		VkFramebuffer deferredFramebuffer;

		FrameBufferAttachment * positionAtt;
		FrameBufferAttachment * normalAtt;
		FrameBufferAttachment * albedoAtt;
		FrameBufferAttachment * materialAtt;
		FrameBufferAttachment * depthAtt;
		std::vector<FrameBufferAttachment*> attachmentsPtr;

		VkSemaphore imageReadySemaphore;
		VkSemaphore drawCompleteSemaphore;

		VkPipelineCache pipelineCache;
#if VULKAN_DEBUG_MODE_ENABLED
		VkDebugReportCallbackEXT debugReport;
#endif
	private:
		bool InitDepthBuffer();
		bool InitColorSampler();
		bool InitDeferredFramebuffer();
	
#if VULKAN_DEBUG_MODE_ENABLED
		bool InitVulkanDebugMode();
		void UnloadVulkanDebugMode();
#endif
	public:
		VulkanInterface();
		~VulkanInterface();

		bool Init(HWND hwnd);
		void BeginSceneDeferred(VulkanCommandBuffer * commandBuffer);
		void EndSceneDeferred(VulkanCommandBuffer * commandBuffer);
		void BeginSceneForward(VulkanCommandBuffer * commandBuffer, int frameId);
		void EndSceneForward(VulkanCommandBuffer * commandBuffer);
		void Present(std::vector<VulkanCommandBuffer*>& renderCommandBuffers);
		void InitViewportAndScissors(VulkanCommandBuffer * commandBuffer, float vWidth, float vHeight, uint32_t sWidth, uint32_t sHeight);
		VulkanCommandPool * GetVulkanCommandPool();
		VulkanDevice * GetVulkanDevice();
		VulkanRenderpass * GetForwardRenderpass();
		VulkanRenderpass * GetDeferredRenderpass();
		VulkanSwapchain * GetVulkanSwapchain();
		VkSampler GetColorSampler();
		FrameBufferAttachment * GetPositionAttachment();
		FrameBufferAttachment * GetNormalAttachment();
		FrameBufferAttachment * GetAlbedoAttachment();
		FrameBufferAttachment * GetMaterialAttachment();
		FrameBufferAttachment * GetDepthAttachment();
		VkFramebuffer GetDeferredFramebuffer();
		VkPipelineCache GetPipelineCache();
};