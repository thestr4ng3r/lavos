
#include "lavos/engine.h"
#include "lavos/material/gouraud_material.h"
#include "lavos/material/material_instance.h"

#include "lavos/shader_load.h"

using namespace lavos;

GouraudMaterial::GouraudMaterial(lavos::Engine *engine) : Material(engine)
{
	CreateDescriptorSetLayout();

	vert_shader_module = CreateShaderModule(engine->GetVkDevice(), "material/gouraud.vert");
	frag_shader_module = CreateShaderModule(engine->GetVkDevice(), "material/gouraud.frag");

	texture_default_base_color = Texture::CreateColor(engine, vk::Format::eR8G8B8Unorm, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f));
	texture_default_normal = Texture::CreateColor(engine, vk::Format::eR8G8B8Unorm, glm::vec4(0.5f, 0.5f, 1.0f, 0.0f));
}

GouraudMaterial::~GouraudMaterial()
{
	auto &device = engine->GetVkDevice();

	engine->DestroyTexture(texture_default_base_color);
	engine->DestroyTexture(texture_default_normal);

	device.destroyShaderModule(vert_shader_module);
	device.destroyShaderModule(frag_shader_module);
}

void GouraudMaterial::CreateDescriptorSetLayout()
{
	descriptor_set_layout = nullptr;
}

std::vector<vk::DescriptorPoolSize> GouraudMaterial::GetDescriptorPoolSizes(Material::RenderMode render_mode) const
{
	return {
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1),
		vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 1)
	};
}

std::vector<vk::PipelineShaderStageCreateInfo> GouraudMaterial::GetShaderStageCreateInfos(Material::RenderMode render_mode) const
{
	return {
		vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(),
										  vk::ShaderStageFlagBits::eVertex,
										  vert_shader_module,
										  "main"),
		vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(),
										  vk::ShaderStageFlagBits::eFragment,
										  frag_shader_module,
										  "main")
	};
}


void GouraudMaterial::WriteDescriptorSet(Material::RenderMode render_mode, vk::DescriptorSet descriptor_set, MaterialInstance *instance)
{
	auto instance_data = reinterpret_cast<InstanceData *>(instance->GetInstanceData(render_mode));

	auto ubo_info = vk::DescriptorBufferInfo()
		.setBuffer(instance_data->uniform_buffer->GetVkBuffer())
		.setOffset(0)
		.setRange(sizeof(UniformBuffer));

	auto ubo_write = vk::WriteDescriptorSet()
		.setDstSet(descriptor_set)
		.setDstBinding(0)
		.setDstArrayElement(0)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1)
		.setPBufferInfo(&ubo_info);



	Texture *base_color_texture = instance->GetTexture(texture_slot_base_color);
	if(base_color_texture == nullptr)
		base_color_texture = &texture_default_base_color;

	auto base_color_image_info = vk::DescriptorImageInfo()
		.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		.setImageView(base_color_texture->image_view)
		.setSampler(base_color_texture->sampler);

	auto base_color_texture_write = vk::WriteDescriptorSet()
		.setDstSet(descriptor_set)
		.setDstBinding(1)
		.setDstArrayElement(0)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setDescriptorCount(1)
		.setPImageInfo(&base_color_image_info);


	engine->GetVkDevice().updateDescriptorSets({base_color_texture_write, ubo_write}, nullptr);
}


void *GouraudMaterial::CreateInstanceData(Material::RenderMode render_mode)
{
	InstanceData *data = new InstanceData();

	data->uniform_buffer = engine->CreateBuffer(sizeof(UniformBuffer),
												vk::BufferUsageFlagBits::eUniformBuffer,
												VMA_MEMORY_USAGE_CPU_ONLY);

	return data;
}

void GouraudMaterial::DestroyInstanceData(Material::RenderMode render_mode, void *data_p)
{
	auto data = reinterpret_cast<InstanceData *>(data_p);
	delete data->uniform_buffer;
	delete data;
}

void GouraudMaterial::UpdateInstanceData(Material::RenderMode render_mode, void *data_p, MaterialInstance *instance)
{
	auto data = reinterpret_cast<InstanceData *>(data_p);

	UniformBuffer ubo;
	ubo.color_factor = instance->GetParameter(parameter_slot_base_color_factor, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	ubo.specular_exponent = instance->GetParameter(parameter_slot_specular_exponent, 16.0f);

	memcpy(data->uniform_buffer->Map(), &ubo, sizeof(ubo));
	data->uniform_buffer->UnMap();
}