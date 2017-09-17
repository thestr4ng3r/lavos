
#include "material/material.h"
#include "material/material_instance.h"
#include "engine.h"

using namespace engine;

MaterialInstance::MaterialInstance(Material *material, vk::DescriptorPool descriptor_pool)
	: material(material), descriptor_pool(descriptor_pool)
{
	CreateDescriptorSet();
}

MaterialInstance::~MaterialInstance()
{
	auto engine = material->GetEngine();

	for(auto &entry : textures)
		engine->DestroyTexture(entry.second);

	engine->GetVkDevice().freeDescriptorSets(descriptor_pool, descriptor_set);
}

void MaterialInstance::CreateDescriptorSet()
{
	auto engine = material->GetEngine();

	vk::DescriptorSetLayout layouts[] = { material->GetDescriptorSetLayout() };

	auto alloc_info = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(descriptor_pool)
		.setDescriptorSetCount(1)
		.setPSetLayouts(layouts);

	descriptor_set = *engine->GetVkDevice().allocateDescriptorSets(alloc_info).begin();
}

void MaterialInstance::WriteDescriptorSet()
{
	material->WriteDescriptorSet(descriptor_set, this);
}

void MaterialInstance::SetTexture(MaterialInstance::TextureSlot slot, Texture texture)
{
	if(texture == nullptr)
	{
		textures.erase(slot);
		return;
	}

	textures[slot] = texture;
}

Texture *MaterialInstance::GetTexture(MaterialInstance::TextureSlot slot)
{
	auto it = textures.find(slot);

	if(it != textures.end())
		return &it->second;

	return nullptr;
}
