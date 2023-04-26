#include "GUIElement.h"
#include "LogManager.h"
#include "StdInc.h"
#include "TextureManager.h"

extern LogManager * gLogManager;
extern TextureManager * gTextureManager;

GUIElement::GUIElement()
{
	name = "";
	texture = NULL;
	canvas = NULL;
}

void GUIElement::copy(const GUIElement &source, VulkanInterface* vulkan, VulkanCommandBuffer* cmdBuffer)
{
	this->name = source.name;
	name.append(".rct");
	this->texture = gTextureManager->RequestTexture((baseItemsDir + this->name), vulkan->GetVulkanDevice(), cmdBuffer);
	this->canvas = new Canvas();
	if (!canvas->Init(vulkan))
		gLogManager->AddMessage("ERROR: Failed to init canvas!");
}

bool GUIElement::Init(VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer, std::string filename)
{
	texture = gTextureManager->RequestTexture(filename, vulkan->GetVulkanDevice(), cmdBuffer);
	if (texture == nullptr)
		return false;

	canvas = new Canvas();
	if (!canvas->Init(vulkan))
	{
		gLogManager->AddMessage("ERROR: Failed to init canvas!");
		return false;
	}

	return true;
}

void GUIElement::Unload(VulkanInterface * vulkan)
{
	SAFE_UNLOAD(canvas, vulkan);
	gTextureManager->ReleaseTexture(texture, vulkan->GetVulkanDevice());
}

void GUIElement::Render(VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer, VulkanPipeline * pipeline,
	Camera * camera, int frameBufferId)
{
	canvas->Render(vulkan, cmdBuffer, pipeline, camera->GetOrthoMatrix(), texture->GetImageView(), frameBufferId);
}

void GUIElement::SetPosition(float x, float y)
{
	canvas->SetPosition(x, y);
}

void GUIElement::SetDimensions(float width, float height)
{
	canvas->SetDimensions(width, height);
}

void GUIElement::SetName(std::string _name)
{
	name = _name;
}

std::string GUIElement::GetName()
{
	return name;
}

float GUIElement::GetDimensionX()
{
	return canvas->GetDimensionX();
}

float GUIElement::GetDimensionY()
{
	return canvas->GetDimensionY();
}

float GUIElement::GetPositionX()
{
	return canvas->GetPositionX();
}

float GUIElement::GetPositionY()
{
	return canvas->GetPositionY();
}
