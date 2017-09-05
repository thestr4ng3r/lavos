
#include <engine.h>
#include "image.h"

#include "stb_image.h"

using namespace engine;

Image Image::LoadFromFile(Engine *engine, std::string file)
{
	int width, height, channels;
	stbi_uc *pixels = stbi_load(file.c_str(), &width, &height, &channels, STBI_rgb_alpha);

	vk::DeviceSize image_size = static_cast<vk::DeviceSize>(width * height * 4);

	if (!pixels)
		throw std::runtime_error("failed to load image!");


	auto device = engine->GetVkDevice();

	auto staging_buffer = engine->CreateBuffer(image_size, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_CPU_ONLY);

	void *data = engine->MapMemory(staging_buffer.allocation);
	memcpy(data, pixels, image_size);
	engine->UnmapMemory(staging_buffer.allocation);

	stbi_image_free(pixels);


	vk::Format format = vk::Format::eR8G8B8A8Unorm;

	Image image = engine->Create2DImage(width, height, format, vk::ImageTiling::eOptimal,
										  vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
										  VMA_MEMORY_USAGE_GPU_ONLY);


	engine->TransitionImageLayout(image.image, format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
	engine->CopyBufferTo2DImage(staging_buffer.buffer, image.image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
	engine->TransitionImageLayout(image.image, format, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);


	engine->DestroyBuffer(staging_buffer);

	return image;
}