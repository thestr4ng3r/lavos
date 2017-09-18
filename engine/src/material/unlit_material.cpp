
#include "engine.h"
#include "material/unlit_material.h"
#include "material/material_instance.h"

#include "shader_load.h"

using namespace engine;

struct UniformBuffer
{
	glm::vec3 color_factor;
};

UnlitMaterial::UnlitMaterial(engine::Engine *engine) : Material(engine)
{
	CreateDescriptorSetLayout();

	vert_shader_module = CreateShaderModule(engine->GetVkDevice(), ReadSPIRVShader("material/unlit.vert"));
	frag_shader_module = CreateShaderModule(engine->GetVkDevice(), ReadSPIRVShader("material/unlit.frag"));

	texture_default_image = Texture::CreateColor(engine, vk::Format::eR8G8B8Unorm, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f));
}

UnlitMaterial::~UnlitMaterial()
{
	auto &device = engine->GetVkDevice();

	engine->DestroyTexture(texture_default_image);

	device.destroyShaderModule(vert_shader_module);
	device.destroyShaderModule(frag_shader_module);
}

void UnlitMaterial::CreateDescriptorSetLayout()
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

std::vector<vk::DescriptorPoolSize> UnlitMaterial::GetDescriptorPoolSizes() const
{
	return {
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1),
		vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 1)
	};
}

std::vector<vk::PipelineShaderStageCreateInfo> UnlitMaterial::GetShaderStageCreateInfos() const
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

void UnlitMaterial::WriteDescriptorSet(vk::DescriptorSet descriptor_set, MaterialInstance *instance)
{
	auto ubo_info = vk::DescriptorBufferInfo()
		.setBuffer(instance->GetUniformBuffer().buffer)
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

engine::Buffer UnlitMaterial::CreateUniformBuffer()
{
	return engine->CreateBuffer(sizeof(UniformBuffer),
								vk::BufferUsageFlagBits::eUniformBuffer,
								VMA_MEMORY_USAGE_CPU_ONLY);
}

void UnlitMaterial::WriteUniformBuffer(engine::Buffer buffer, MaterialInstance *instance)
{
	UniformBuffer ubo;

	ubo.color_factor = instance->GetParameter(parameter_slot_base_color_factor, glm::vec3(1.0f, 1.0f, 1.0f));

	void *data = engine->MapMemory(buffer.allocation);
	memcpy(data, &ubo, sizeof(ubo));
	engine->UnmapMemory(buffer.allocation);
}
