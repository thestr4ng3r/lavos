
#include "lavos/material/material.h"
#include "lavos/material/material_instance.h"
#include "lavos/engine.h"

using namespace lavos;

MaterialInstance::MaterialInstance(Material *material, vk::DescriptorPool descriptor_pool)
	: material(material), descriptor_pool(descriptor_pool)
{
	CreateDescriptorSet(material->GetDescriptorSetId(Material::DefaultRenderMode::ColorForward)); // TODO: depend on context
	CreateInstanceData(material->GetInstanceDataId(Material::DefaultRenderMode::ColorForward)); // TODO: depend on context
}

MaterialInstance::~MaterialInstance()
{
	auto engine = material->GetEngine();

	for(auto it : instance_data)
		material->DestroyInstanceData(it.first, it.second);

	for(auto &entry : textures)
		engine->DestroyTexture(entry.second);

	for(auto it : descriptor_sets)
		engine->GetVkDevice().freeDescriptorSets(descriptor_pool, it.second);
}

void MaterialInstance::CreateDescriptorSet(Material::RenderMode render_mode)
{
	auto engine = material->GetEngine();
	auto descriptor_set_layout_id = material->GetDescriptorSetId(render_mode);
	auto layout = material->GetDescriptorSetLayout(descriptor_set_layout_id);

	if(!layout)
		return;

	vk::DescriptorSetLayout layouts[] = { layout->layout };

	auto alloc_info = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(descriptor_pool)
			.setDescriptorSetCount(1)
			.setPSetLayouts(layouts);

	auto descriptor_set = *engine->GetVkDevice().allocateDescriptorSets(alloc_info).begin();
	if(descriptor_set)
		descriptor_sets[render_mode] = descriptor_set;
}

void MaterialInstance::CreateInstanceData(Material::RenderMode render_mode)
{
	instance_data[render_mode] = material->CreateInstanceData(render_mode);
}

void MaterialInstance::WriteDescriptorSet(Material::RenderMode render_mode)
{
	auto descriptor_set = GetDescriptorSet(render_mode);
	if(descriptor_set)
		material->WriteDescriptorSet(render_mode, descriptor_set, this);
}

void MaterialInstance::WriteInstanceData(Material::RenderMode render_mode)
{
	auto data = instance_data[render_mode];
	material->UpdateInstanceData(render_mode, data, this);
}

void MaterialInstance::SetTexture(Material::TextureSlot slot, Texture texture)
{
	if(texture == nullptr)
	{
		textures.erase(slot);
		return;
	}

	textures[slot] = texture;
}

Texture *MaterialInstance::GetTexture(Material::TextureSlot slot)
{
	auto it = textures.find(slot);

	if(it != textures.end())
		return &it->second;

	return nullptr;
}

void MaterialInstance::SetParameter(Material::ParameterSlot slot, MaterialInstance::Parameter parameter)
{
	parameters.insert(std::make_pair(slot, parameter));
}

MaterialInstance::Parameter *MaterialInstance::GetParameter(Material::ParameterSlot slot)
{
	auto it = parameters.find(slot);

	if(it != parameters.end())
		return &it->second;

	return nullptr;
}

float MaterialInstance::GetParameter(Material::ParameterSlot slot, float default_value)
{
	auto it = parameters.find(slot);

	if(it != parameters.end())
		return it->second.f;

	return default_value;
}

glm::vec2 MaterialInstance::GetParameter(Material::ParameterSlot slot, glm::vec2 default_value)
{
	auto it = parameters.find(slot);

	if(it != parameters.end())
		return it->second.v2;

	return default_value;
}

glm::vec3 MaterialInstance::GetParameter(Material::ParameterSlot slot, glm::vec3 default_value)
{
	auto it = parameters.find(slot);

	if(it != parameters.end())
		return it->second.v3;

	return default_value;
}

glm::vec4 MaterialInstance::GetParameter(Material::ParameterSlot slot, glm::vec4 default_value)
{
	auto it = parameters.find(slot);

	if(it != parameters.end())
		return it->second.v4;

	return default_value;
}

void MaterialInstance::WriteAllData()
{
	for(auto it : instance_data)
		WriteInstanceData(it.first);

	for(auto it : descriptor_sets)
		WriteDescriptorSet(it.first);
}
