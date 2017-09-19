
#include "material/material.h"
#include "material/material_instance.h"
#include "engine.h"

using namespace engine;

MaterialInstance::MaterialInstance(Material *material, vk::DescriptorPool descriptor_pool)
	: material(material), descriptor_pool(descriptor_pool)
{
	CreateDescriptorSet();
	CreateUniformBuffer();
}

MaterialInstance::~MaterialInstance()
{
	auto engine = material->GetEngine();

	material->DestroyInstanceData(instance_data);

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

void MaterialInstance::CreateUniformBuffer()
{
	instance_data = material->CreateInstanceData();
}

void MaterialInstance::WriteDescriptorSet()
{
	material->WriteDescriptorSet(descriptor_set, this);
}

void MaterialInstance::WriteUniformBuffer()
{
	material->UpdateInstanceData(instance_data, this);
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

void MaterialInstance::SetParameter(MaterialInstance::ParameterSlot slot, MaterialInstance::Parameter parameter)
{
	parameters.insert(std::make_pair(slot, parameter));
}

MaterialInstance::Parameter *MaterialInstance::GetParameter(MaterialInstance::ParameterSlot slot)
{
	auto it = parameters.find(slot);

	if(it != parameters.end())
		return &it->second;

	return nullptr;
}

float MaterialInstance::GetParameter(MaterialInstance::ParameterSlot slot, float default_value)
{
	auto it = parameters.find(slot);

	if(it != parameters.end())
		return it->second.f;

	return default_value;
}

glm::vec2 MaterialInstance::GetParameter(MaterialInstance::ParameterSlot slot, glm::vec2 default_value)
{
	auto it = parameters.find(slot);

	if(it != parameters.end())
		return it->second.v2;

	return default_value;
}

glm::vec3 MaterialInstance::GetParameter(MaterialInstance::ParameterSlot slot, glm::vec3 default_value)
{
	auto it = parameters.find(slot);

	if(it != parameters.end())
		return it->second.v3;

	return default_value;
}

glm::vec4 MaterialInstance::GetParameter(MaterialInstance::ParameterSlot slot, glm::vec4 default_value)
{
	auto it = parameters.find(slot);

	if(it != parameters.end())
		return it->second.v4;

	return default_value;
}

void MaterialInstance::WriteAllData()
{
	WriteUniformBuffer();
	WriteDescriptorSet();
}
