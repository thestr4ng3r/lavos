
#include "engine.h"
#include "material/phong_material.h"
#include "material/material_instance.h"

#include "shader_load.h"

using namespace engine;

PhongMaterial::PhongMaterial(engine::Engine *engine) : Material(engine)
{
	CreateDescriptorSetLayout();

	vert_shader_module = CreateShaderModule(engine->GetVkDevice(), ReadSPIRVShader("material/phong.vert"));
	frag_shader_module = CreateShaderModule(engine->GetVkDevice(), ReadSPIRVShader("material/phong.frag"));

	texture_default_image = Texture::CreateColor(engine, vk::Format::eR8G8B8Unorm, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f));
}

PhongMaterial::~PhongMaterial()
{
	auto &device = engine->GetVkDevice();

	engine->DestroyTexture(texture_default_image);

	device.destroyShaderModule(vert_shader_module);
	device.destroyShaderModule(frag_shader_module);
}

void PhongMaterial::CreateDescriptorSetLayout()
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


	auto create_info = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(static_cast<uint32_t>(bindings.size()))
		.setPBindings(bindings.data());

	descriptor_set_layout = engine->GetVkDevice().createDescriptorSetLayout(create_info);
}

std::vector<vk::DescriptorPoolSize> PhongMaterial::GetDescriptorPoolSizes() const
{
	return {
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1),
		vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 1)
	};
}

std::vector<vk::PipelineShaderStageCreateInfo> PhongMaterial::GetShaderStageCreateInfos() const
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


void PhongMaterial::WriteDescriptorSet(vk::DescriptorSet descriptor_set, MaterialInstance *instance)
{
	auto instance_data = reinterpret_cast<InstanceData *>(instance->GetInstanceData());

	auto ubo_info = vk::DescriptorBufferInfo()
		.setBuffer(instance_data->uniform_buffer.buffer)
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


void *PhongMaterial::CreateInstanceData()
{
	InstanceData *data = new InstanceData();

	data->uniform_buffer = engine->CreateBuffer(sizeof(UniformBuffer),
												vk::BufferUsageFlagBits::eUniformBuffer,
												VMA_MEMORY_USAGE_CPU_ONLY);

	return data;
}

void PhongMaterial::DestroyInstanceData(void *data_p)
{
	auto data = reinterpret_cast<InstanceData *>(data_p);
	engine->DestroyBuffer(data->uniform_buffer);
	delete data;
}

void PhongMaterial::UpdateInstanceData(void *data_p, MaterialInstance *instance)
{
	auto data = reinterpret_cast<InstanceData *>(data_p);

	UniformBuffer ubo;

	ubo.color_factor = instance->GetParameter(parameter_slot_base_color_factor, glm::vec3(1.0f, 1.0f, 1.0f));

	void *buffer_data = engine->MapMemory(data->uniform_buffer.allocation);
	memcpy(buffer_data, &ubo, sizeof(ubo));
	engine->UnmapMemory(data->uniform_buffer.allocation);
}