
#include "material.h"
#include "engine.h"

using namespace engine;

Material::Material(Engine *engine, std::string texture_file):
	engine(engine)
{
	texture_image = engine::Image::LoadFromFile(engine, texture_file);

	auto create_info = vk::ImageViewCreateInfo()
		.setImage(texture_image.image)
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(vk::Format::eR8G8B8A8Unorm)
		.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

	texture_image_view = engine->GetVkDevice().createImageView(create_info);
}

Material::~Material()
{
	engine->GetVkDevice().destroyImageView(texture_image_view);
	engine->DestroyImage(texture_image);
}
