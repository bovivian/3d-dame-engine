#pragma once

#include "VulkanInterface.h"
#include <string>

class Shader
{
	private:
		VkPipelineShaderStageCreateInfo * shaderStages;
		uint32_t stageCount;
	public:
		Shader();
		~Shader();

		bool Init(VulkanDevice * vulkanDevice, std::string shaderName, bool hasGeometryShader);
		void Unload(VulkanDevice * vulkanDevice);
		VkPipelineShaderStageCreateInfo * GetShaderStages();
		uint32_t GetStageCount();
};