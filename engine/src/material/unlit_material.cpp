
#include "engine.h"
#include "material/unlit_material.h"

using namespace engine;

UnlitMaterial::UnlitMaterial(engine::Engine *engine) : Material(engine)
{
	CreateDescriptorSetLayout();
}

UnlitMaterial::~UnlitMaterial()
{
}

void UnlitMaterial::CreateDescriptorSetLayout()
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

std::vector<vk::DescriptorPoolSize> UnlitMaterial::GetDescriptorPoolSizes() const
{
	return {
		vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 1)
	};
}
