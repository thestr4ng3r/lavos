
#include "texture.h"
#include "engine.h"

using namespace lavos;

Texture Texture::CreateColor(Engine *engine, vk::Format format, glm::vec4 color)
{
	auto &device = engine->GetVkDevice();

	Image image = Image::CreateColor(engine, format, color);

	auto image_view_info = vk::ImageViewCreateInfo()
		.setImage(image.image)
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(image.format)
		.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

	auto image_view = device.createImageView(image_view_info);


	auto create_info = vk::SamplerCreateInfo()
		.setMagFilter(vk::Filter::eNearest)
		.setMinFilter(vk::Filter::eNearest)
		.setAddressModeU(vk::SamplerAddressMode::eRepeat)
		.setAddressModeV(vk::SamplerAddressMode::eRepeat)
		.setAddressModeW(vk::SamplerAddressMode::eRepeat)
		.setAnisotropyEnable(VK_FALSE)
		.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
		.setUnnormalizedCoordinates(VK_FALSE)
		.setCompareEnable(VK_FALSE)
		.setCompareOp(vk::CompareOp::eAlways)
		.setMipmapMode(vk::SamplerMipmapMode::eNearest)
		.setMipLodBias(0.0f)
		.setMinLod(0.0f)
		.setMaxLod(0.0f);

	auto sampler = device.createSampler(create_info);

	return Texture(image, image_view, sampler);
}