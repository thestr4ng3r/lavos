
#include "material.h"
#include "material_instance.h"
#include "engine.h"

using namespace engine;

MaterialInstance::MaterialInstance(Material *material, vk::DescriptorPool descriptor_pool)
	: material(material), texture(nullptr)
{
	CreateDescriptorSet(descriptor_pool);
}

MaterialInstance::~MaterialInstance()
{
	auto engine = material->GetEngine();
	engine->DestroyTexture(texture);

	// TODO: free descriptor set (would also be automatically freed with descriptor pool)
}

void MaterialInstance::CreateDescriptorSet(vk::DescriptorPool descriptor_pool)
{
	auto engine = material->GetEngine();

	vk::DescriptorSetLayout layouts[] = { material->GetDescriptorSetLayout() };

	auto alloc_info = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(descriptor_pool)
		.setDescriptorSetCount(1)
		.setPSetLayouts(layouts);

	descriptor_set = *engine->GetVkDevice().allocateDescriptorSets(alloc_info).begin();
}

void MaterialInstance::WriteDescriptorSet()
{
	auto image_info = vk::DescriptorImageInfo()
		.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		.setImageView(texture.image_view)
		.setSampler(texture.sampler);

	auto image_write = vk::WriteDescriptorSet()
		.setDstSet(descriptor_set)
		.setDstBinding(0)
		.setDstArrayElement(0)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setDescriptorCount(1)
		.setPImageInfo(&image_info);

	material->GetEngine()->GetVkDevice().updateDescriptorSets(image_write, nullptr);
}
