
#include <engine.h>
#include <glm/vec4.hpp>
#include "image.h"

#include "stb_image.h"

using namespace engine;


inline unsigned int GetComponentsCount(vk::Format format)
{
	switch(format)
	{
		case vk::Format::eR8Unorm:				return 1;
		case vk::Format::eR8G8Unorm:			return 2;
		case vk::Format::eR8G8B8Unorm:			return 3;
		case vk::Format::eR8G8B8A8Unorm:		return 4;
		default:
			throw std::runtime_error("unsupported format.");
	}
}

inline size_t GetComponentSize(vk::Format format)
{
	switch(format)
	{
		case vk::Format::eR8Unorm:				return 1;
		case vk::Format::eR8G8Unorm:			return 1;
		case vk::Format::eR8G8B8Unorm:			return 1;
		case vk::Format::eR8G8B8A8Unorm:		return 1;
		default:
			throw std::runtime_error("unsupported format.");
	}
}


unsigned char *ReformatImageData(unsigned int src_components, unsigned int dst_components,
								 size_t pixel_count, unsigned char *pixels,
								 std::array<unsigned char, 4> default_values = { 0, 0, 0, 255 })
{
	unsigned char *dst_pixels = new unsigned char[dst_components * pixel_count];

	for(size_t i=0; i<pixel_count; i++)
	{
		for(unsigned int c=0; c<dst_components; c++)
		{
			dst_pixels[i * dst_components + c] = c < src_components
												 ? pixels[i * src_components + c]
												 : default_values[c];
		}
	}

	return dst_pixels;
}


Image Image::LoadFromPixelData(Engine *engine, vk::Format format, uint32_t width, uint32_t height, unsigned char *pixels)
{
	vk::Format actual_format = format;
	if(actual_format != vk::Format::eR8G8B8A8Unorm)
	{
		actual_format = engine->FindSupportedFormat({format, vk::Format::eR8G8B8A8Unorm},
													vk::ImageTiling::eOptimal,
													vk::FormatFeatureFlagBits::eSampledImage);
	}

	vk::DeviceSize image_size = static_cast<vk::DeviceSize>(width * height * GetComponentsCount(actual_format));

	unsigned char *image_pixels = pixels;
	if(actual_format != format)
	{
		image_pixels = ReformatImageData(GetComponentsCount(format),
										 GetComponentsCount(actual_format), width * height, pixels);
	}


	auto staging_buffer = engine->CreateBuffer(image_size, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_CPU_ONLY);

	void *data = engine->MapMemory(staging_buffer.allocation);
	memcpy(data, image_pixels, image_size);
	engine->UnmapMemory(staging_buffer.allocation);

	if(image_pixels != pixels)
		delete [] image_pixels;


	Image image = engine->Create2DImage(width, height, actual_format, vk::ImageTiling::eOptimal,
										vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
										VMA_MEMORY_USAGE_GPU_ONLY);


	engine->TransitionImageLayout(image.image, actual_format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
	engine->CopyBufferTo2DImage(staging_buffer.buffer, image.image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
	engine->TransitionImageLayout(image.image, actual_format, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);


	engine->DestroyBuffer(staging_buffer);

	return image;
}

Image Image::LoadFromFile(Engine *engine, std::string file)
{
	int width, height, channels;
	stbi_uc *pixels = stbi_load(file.c_str(), &width, &height, &channels, STBI_rgb_alpha);

	if (!pixels)
		throw std::runtime_error(std::string("failed to load image \"" + file + "\": ") + stbi_failure_reason());

	Image r = LoadFromPixelData(engine, vk::Format::eR8G8B8A8Unorm,
								static_cast<uint32_t>(width), static_cast<uint32_t>(height), pixels);

	stbi_image_free(pixels);

	return r;
}

Image Image::LoadFromMemory(Engine *engine, unsigned char *data, size_t size)
{
	int width, height, channels;
	stbi_uc *pixels = stbi_load_from_memory(data, static_cast<int>(size), &width, &height, &channels, STBI_rgb_alpha);

	if (!pixels)
		throw std::runtime_error(std::string("failed to load image from memory: ") + stbi_failure_reason());

	Image r = LoadFromPixelData(engine, vk::Format::eR8G8B8A8Unorm,
								static_cast<uint32_t>(width), static_cast<uint32_t>(height), pixels);

	stbi_image_free(pixels);

	return r;
}

Image Image::CreateColor(Engine *engine, vk::Format format, glm::vec4 color)
{
	unsigned int components = GetComponentsCount(format);
	size_t component_size = GetComponentSize(format);

	if(component_size != 1)
	{
		// TODO: add support for more formats
		throw std::runtime_error("unsupported format for creating single color image.");
	}

	unsigned char *pixels = new unsigned char[components * component_size];

	for(unsigned int i=0; i<components; i++)
		pixels[i] = static_cast<unsigned char>(color[i] * 255.0f);

	Image image = LoadFromPixelData(engine, format, 1, 1, pixels);

	delete[] pixels;

	return image;
}
