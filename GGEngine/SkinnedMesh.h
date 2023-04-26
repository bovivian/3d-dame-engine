#pragma once

#include "VulkanInterface.h"
#include "VulkanPipeline.h"
#include "VulkanBuffer.h"
#include "Material.h"

class SkinnedMesh
{
	private:
		struct Vertex {
			float x, y, z;
			float u, v;
			float nx, ny, nz;
			float boneWeights[4];
			uint32_t boneIDs[4];
			float tx, ty, tz;
			float bx, by, bz;
		};

		unsigned int vertexCount;
		unsigned int indexCount;

		struct MaterialUniformBuffer
		{
			float hasNormalMap;
			float metallicOffset;
			float roughnessOffset;
			float padding;
		};
		MaterialUniformBuffer materialUniformBuffer;

		VulkanBuffer * vertexBuffer;
		VulkanBuffer * indexBuffer;
		VulkanBuffer * materialUBO;

		Material * material;
	public:
		SkinnedMesh();
		~SkinnedMesh();

		bool Init(VulkanInterface * vulkan, FILE * modelFile, std::string meshName);
		void Unload(VulkanInterface * vulkan);
		void Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer);
		void UpdateUniformBuffer(VulkanInterface * vulkan);
		void SetMaterial(Material * material);
		Material * GetMaterial();
		VkDescriptorBufferInfo * GetMaterialBufferInfo();
};