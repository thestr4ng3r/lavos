
#include "renderer.h"

using namespace engine;

Renderer::Renderer(Engine *engine)
	: engine(engine)
{
	CreateDescriptorPool();
	CreateMaterial();
}

Renderer::~Renderer()
{
	engine->GetVkDevice().destroyDescriptorPool(descriptor_pool);
}

void Renderer::CreateDescriptorPool()
{
	std::vector<vk::DescriptorPoolSize> pool_sizes = {
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1),
		vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 1),
	};

	auto create_info = vk::DescriptorPoolCreateInfo()
		.setPoolSizeCount(static_cast<uint32_t>(pool_sizes.size()))
		.setPPoolSizes(pool_sizes.data())
		.setMaxSets(2);

	descriptor_pool = engine->GetVkDevice().createDescriptorPool(create_info);
}

void Renderer::CreateMaterial()
{
	material = new Material(engine);
}
