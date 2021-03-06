
#include "lavos/engine.h"
#include "lavos/material/unlit_material.h"
#include "lavos/material/material_instance.h"

#include "lavos/shader_load.h"

using namespace lavos;

UnlitMaterial::UnlitMaterial(lavos::Engine *engine) : Material(engine)
{
	CreateDescriptorSetLayouts();

	vert_shader_module = CreateShaderModule(engine->GetVkDevice(), "material/unlit.vert");
	frag_shader_module = CreateShaderModule(engine->GetVkDevice(), "material/unlit.frag");

	texture_default_image = Texture::CreateColor(engine, vk::Format::eR8G8B8Unorm, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f));
}

UnlitMaterial::~UnlitMaterial()
{
	auto &device = engine->GetVkDevice();

	engine->DestroyTexture(texture_default_image);

	device.destroyShaderModule(vert_shader_module);
	device.destroyShaderModule(frag_shader_module);
}

void UnlitMaterial::CreateDescriptorSetLayouts()
{
	std::vector<vk::DescriptorSetLayoutBinding> bindings = {
		vk::DescriptorSetLayoutBinding()
			.setBinding(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setPImmutableSamplers(nullptr)
			.setStageFlags(vk::ShaderStageFlagBits::eFragment),

		vk::DescriptorSetLayoutBinding()
			.setBinding(1)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(1)
			.setPImmutableSamplers(nullptr)
			.setStageFlags(vk::ShaderStageFlagBits::eFragment)
	};

	CreateDescriptorSetLayout(DescriptorSetLayoutIdDefault, bindings);
}

std::vector<vk::PipelineShaderStageCreateInfo> UnlitMaterial::GetShaderStageCreateInfos(RenderMode render_mode) const
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

void UnlitMaterial::WriteDescriptorSet(DescriptorSetId id, vk::DescriptorSet descriptor_set, MaterialInstance *instance)
{
	auto instance_data = reinterpret_cast<UBOInstanceData *>(instance->GetInstanceData(id));

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



	Texture *texture = instance->GetTexture(texture_slot_base_color);
	if(texture == nullptr)
		texture = &texture_default_image;

	auto image_info = vk::DescriptorImageInfo()
		.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		.setImageView(texture->image_view)
		.setSampler(texture->sampler);

	auto image_write = vk::WriteDescriptorSet()
		.setDstSet(descriptor_set)
		.setDstBinding(1)
		.setDstArrayElement(0)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setDescriptorCount(1)
		.setPImageInfo(&image_info);


	engine->GetVkDevice().updateDescriptorSets({image_write, ubo_write}, nullptr);
}

void *UnlitMaterial::CreateInstanceData(InstanceDataId id)
{
	auto buffer = engine->CreateBuffer(sizeof(UniformBuffer),
									   vk::BufferUsageFlagBits::eUniformBuffer,
									   VMA_MEMORY_USAGE_CPU_ONLY);

	return new UBOInstanceData(buffer);
}

void UnlitMaterial::DestroyInstanceData(InstanceDataId id, void *data_p)
{
	auto data = reinterpret_cast<UBOInstanceData *>(data_p);
	delete data;
}

void UnlitMaterial::UpdateInstanceData(InstanceDataId id, void *data_p, MaterialInstance *instance)
{
	auto data = reinterpret_cast<UBOInstanceData *>(data_p);

	UniformBuffer ubo;

	ubo.color_factor = instance->GetParameter(parameter_slot_base_color_factor, glm::vec3(1.0f, 1.0f, 1.0f));

	memcpy(data->uniform_buffer->Map(), &ubo, sizeof(ubo));
	data->uniform_buffer->UnMap();
}