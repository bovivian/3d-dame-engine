#pragma once

#include "VulkanInterface.h"
#include "VulkanPipeline.h"
#include "VulkanBuffer.h"

class Canvas
{
	private:
		struct Vertex {
			float x, y, z;
			float u, v;
		};
		unsigned int vertexCount;

		struct VertexUniformBuffer
		{
			glm::mat4 MVP;
		};
		VertexUniformBuffer vertexUniformBuffer;

		VulkanBuffer * vsUBO;
		VulkanBuffer * vertexBuffer;
		Vertex * vertexData;

		float posX, posY, width, height;
		bool updateVertexBuffer;

		std::vector<VulkanCommandBuffer*> drawCmdBuffers;
	private:
		void UpdateVertexData();
		void UpdateDescriptorSet(VulkanInterface * vulkan, VulkanPipeline * vulkanPipeline, VkImageView * imageView);
	public:
		Canvas();
		~Canvas();

		bool Init(VulkanInterface * vulkan);
		void Unload(VulkanInterface * vulkan);
		void Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer, VulkanPipeline * vulkanPipeline,
			glm::mat4 orthoMatrix, VkImageView * imageView, int frameBufferId);
		void SetPosition(float x, float y);
		void SetDimensions(float width, float height);
		float GetDimensionX();
		float GetDimensionY();
		float GetPositionX();
		float GetPositionY();
};