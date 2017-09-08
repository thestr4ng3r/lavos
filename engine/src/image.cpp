
#include <engine.h>
#include "image.h"

#include "stb_image.h"

using namespace engine;


Image Image::LoadFromPixelDataRGBA8UI(Engine *engine, uint32_t width, uint32_t height, unsigned char *pixels)
{
	vk::DeviceSize image_size = static_cast<vk::DeviceSize>(width * height * 4);

	auto device = engine->GetVkDevice();

	auto staging_buffer = engine->CreateBuffer(image_size, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_CPU_ONLY);

	void *data = engine->MapMemory(staging_buffer.allocation);
	memcpy(data, pixels, image_size);
	engine->UnmapMemory(staging_buffer.allocation);


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

Image Image::LoadFromFile(Engine *engine, std::string file)
{
	int width, height, channels;
	stbi_uc *pixels = stbi_load(file.c_str(), &width, &height, &channels, STBI_rgb_alpha);

	if (!pixels)
		throw std::runtime_error(std::string("failed to load image \"" + file + "\": ") + stbi_failure_reason());

	Image r = LoadFromPixelDataRGBA8UI(engine, static_cast<uint32_t>(width), static_cast<uint32_t>(height), pixels);

	stbi_image_free(pixels);

	return r;
}

Image Image::LoadFromMemory(Engine *engine, unsigned char *data, size_t size)
{
	int width, height, channels;
	stbi_uc *pixels = stbi_load_from_memory(data, static_cast<int>(size), &width, &height, &channels, STBI_rgb_alpha);

	if (!pixels)
		throw std::runtime_error(std::string("failed to load image from memory: ") + stbi_failure_reason());

	Image r = LoadFromPixelDataRGBA8UI(engine, static_cast<uint32_t>(width), static_cast<uint32_t>(height), pixels);

	stbi_image_free(pixels);

	return r;
}
