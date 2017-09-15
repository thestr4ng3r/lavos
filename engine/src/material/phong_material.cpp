
#include "engine.h"
#include "material/phong_material.h"

#include "shader_load.h"

using namespace engine;

PhongMaterial::PhongMaterial(engine::Engine *engine) : Material(engine)
{
	CreateDescriptorSetLayout();

	vert_shader_module = CreateShaderModule(engine->GetVkDevice(), ReadSPIRVShader("phong.vert"));
	frag_shader_module = CreateShaderModule(engine->GetVkDevice(), ReadSPIRVShader("phong.frag"));
}

PhongMaterial::~PhongMaterial()
{
	auto &device = engine->GetVkDevice();

	device.destroyShaderModule(vert_shader_module);
	device.destroyShaderModule(frag_shader_module);
}

void PhongMaterial::CreateDescriptorSetLayout()
{
	std::vector<vk::DescriptorSetLayoutBinding> bindings = {
		vk::DescriptorSetLayoutBinding()
			.setBinding(0)
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
