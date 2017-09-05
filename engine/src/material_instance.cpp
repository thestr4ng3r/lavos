
#include "material.h"
#include "material_instance.h"
#include "engine.h"

using namespace engine;

MaterialInstance::MaterialInstance(Material *material, vk::DescriptorPool descriptor_pool, std::string texture_file)
	: material(material)
{
	auto engine = material->GetEngine();

	texture_image = engine::Image::LoadFromFile(engine, texture_file);

	auto create_info = vk::ImageViewCreateInfo()
		.setImage(texture_image.image)
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(vk::Format::eR8G8B8A8Unorm)
		.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

	texture_image_view = engine->GetVkDevice().createImageView(create_info);

	CreateDescriptorSet(descriptor_pool);
}

MaterialInstance::~MaterialInstance()
{
	auto engine = material->GetEngine();

	// TODO: free descriptor set (would also be automatically freed with descriptor pool)

	engine->GetVkDevice().destroyImageView(texture_image_view);
	engine->DestroyImage(texture_image);
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

	auto image_info = vk::DescriptorImageInfo()
		.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		.setImageView(texture_image_view)
		.setSampler(material->GetTextureSampler());

	auto image_write = vk::WriteDescriptorSet()
		.setDstSet(descriptor_set)
		.setDstBinding(0)
		.setDstArrayElement(0)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setDescriptorCount(1)
		.setPImageInfo(&image_info);

	engine->GetVkDevice().updateDescriptorSets(image_write, nullptr);
}
