
#include "lavos/engine.h"
#include "lavos/material/phong_material.h"
#include "lavos/material/material_instance.h"

#include "lavos/shader_load.h"

using namespace lavos;

PhongMaterial::PhongMaterial(lavos::Engine *engine) : Material(engine)
{
	CreateDescriptorSetLayouts();

	vert_shader_module = CreateShaderModule(engine->GetVkDevice(), "material/phong.vert");
	frag_shader_module = CreateShaderModule(engine->GetVkDevice(), "material/phong.frag");
	shadow_vert_shader_module = CreateShaderModule(engine->GetVkDevice(), "material/shadow.vert");
	shadow_frag_shader_module = CreateShaderModule(engine->GetVkDevice(), "material/shadow.frag");

	texture_default_base_color = Texture::CreateColor(engine, vk::Format::eR8G8B8Unorm, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f));
	texture_default_normal = Texture::CreateColor(engine, vk::Format::eR8G8B8Unorm, glm::vec4(0.5f, 0.5f, 1.0f, 0.0f));
}

PhongMaterial::~PhongMaterial()
{
	auto &device = engine->GetVkDevice();

	engine->DestroyTexture(texture_default_base_color);
	engine->DestroyTexture(texture_default_normal);

	device.destroyShaderModule(vert_shader_module);
	device.destroyShaderModule(frag_shader_module);
	device.destroyShaderModule(shadow_vert_shader_module);
	device.destroyShaderModule(shadow_frag_shader_module);
}

void PhongMaterial::CreateDescriptorSetLayouts()
{
	CreateDescriptorSetLayout(DescriptorSetLayoutIdDefault, {
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
					.setStageFlags(vk::ShaderStageFlagBits::eFragment),

			vk::DescriptorSetLayoutBinding()
					.setBinding(2)
					.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
					.setDescriptorCount(1)
					.setPImmutableSamplers(nullptr)
					.setStageFlags(vk::ShaderStageFlagBits::eFragment)
	});
}

std::vector<vk::PipelineShaderStageCreateInfo> PhongMaterial::GetShaderStageCreateInfos(Material::RenderMode render_mode) const
{
	if(render_mode == DefaultRenderMode::ColorForward)
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
	else if(render_mode == DefaultRenderMode::Shadow)
	{
		return {
				vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(),
												  vk::ShaderStageFlagBits::eVertex,
												  shadow_vert_shader_module,
												  "main"),
				vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(),
												  vk::ShaderStageFlagBits::eFragment,
												  shadow_frag_shader_module,
												  "main")
		};
	}
	else
	{
		assert(false);
	}
}


void PhongMaterial::WriteDescriptorSet(Material::DescriptorSetId id, vk::DescriptorSet descriptor_set, MaterialInstance *instance)
{
	if(id == DefaultRenderMode::ColorForward)
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



		Texture *normal_texture = instance->GetTexture(texture_slot_normal);
		if(normal_texture == nullptr)
			normal_texture = &texture_default_normal;

		auto normal_image_info = vk::DescriptorImageInfo()
				.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
				.setImageView(normal_texture->image_view)
				.setSampler(normal_texture->sampler);

		auto normal_texture_write = vk::WriteDescriptorSet()
				.setDstSet(descriptor_set)
				.setDstBinding(2)
				.setDstArrayElement(0)
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				.setDescriptorCount(1)
				.setPImageInfo(&normal_image_info);


		engine->GetVkDevice().updateDescriptorSets({base_color_texture_write, normal_texture_write, ubo_write}, nullptr);
	}
	else if(id == DefaultRenderMode::Shadow)
	{
	}
	else
	{
		assert(false);
	}
}


void *PhongMaterial::CreateInstanceData(InstanceDataId id)
{
	if(id == DefaultRenderMode::ColorForward)
	{
		auto buffer = engine->CreateBuffer(sizeof(UniformBuffer),
										   vk::BufferUsageFlagBits::eUniformBuffer,
										   VMA_MEMORY_USAGE_CPU_ONLY);


		return new UBOInstanceData(buffer);
	}
	else if(id == DefaultRenderMode::Shadow)
	{
		return nullptr;
	}
	else
	{
		assert(false);
	}
}

void PhongMaterial::DestroyInstanceData(InstanceDataId id, void *data_p)
{
	if(id == DefaultRenderMode::ColorForward)
	{
		auto data = reinterpret_cast<UBOInstanceData *>(data_p);
		delete data;
	}
	else if(id == DefaultRenderMode::Shadow)
	{
	}
	else
	{
		assert(false);
	}
}

void PhongMaterial::UpdateInstanceData(InstanceDataId id, void *data_p, MaterialInstance *instance)
{
	if(id == DefaultRenderMode::ColorForward)
	{
		auto data = reinterpret_cast<UBOInstanceData *>(data_p);

		UniformBuffer ubo;
		ubo.color_factor = instance->GetParameter(parameter_slot_base_color_factor, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		ubo.specular_exponent = instance->GetParameter(parameter_slot_specular_exponent, 16.0f);

		memcpy(data->uniform_buffer->Map(), &ubo, sizeof(ubo));
		data->uniform_buffer->UnMap();
	}
	else if(id == DefaultRenderMode::Shadow)
	{
	}
	else
	{
		assert(false);
	}
}