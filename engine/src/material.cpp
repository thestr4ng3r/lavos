
#include "material.h"
#include "engine.h"

using namespace engine;

Material::Material(engine::Engine *engine)
	: engine(engine)
{
	CreateDescriptorSetLayout();
	CreateSamplers();
}

Material::~Material()
{
	engine->GetVkDevice().destroyDescriptorSetLayout(descriptor_set_layout);
}

void Material::CreateDescriptorSetLayout()
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

void Material::CreateSamplers()
{
}

std::vector<vk::DescriptorPoolSize> Material::GetDescriptorPoolSizes() const
{
	return {
		vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 1)
	};
}
