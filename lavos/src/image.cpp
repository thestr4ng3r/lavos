
#include <lavos/engine.h>
#include <glm/ext/vector_float3.hpp>
#include "lavos/image.h"
#include "lavos/vk_util.h"

#include "stb_image.h"

using namespace lavos;
using namespace lavos::vk_util;

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

	vk::DeviceSize image_size = static_cast<vk::DeviceSize>(width * height * GetComponentsCount(actual_format) * GetComponentSize(actual_format));

	unsigned char *image_pixels = pixels;
	if(actual_format != format)
	{
		image_pixels = ReformatImageData(GetComponentsCount(format),
										 GetComponentsCount(actual_format), width * height, pixels);
	}


	auto staging_buffer = engine->CreateBuffer(image_size, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_CPU_ONLY);

	memcpy(staging_buffer->Map(), image_pixels, image_size);
	staging_buffer->UnMap();

	if(image_pixels != pixels)
		delete [] image_pixels;

	Image image = engine->Create2DImage(width, height, actual_format, vk::ImageTiling::eOptimal,
										vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
										VMA_MEMORY_USAGE_GPU_ONLY);


	vk::ImageAspectFlags aspect_mask = GetFormatIsDepth(format) ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;
	engine->TransitionImageLayout(image.image, actual_format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, aspect_mask);
	engine->CopyBufferTo2DImage(staging_buffer->GetVkBuffer(), image.image, static_cast<uint32_t>(width), static_cast<uint32_t>(height), aspect_mask);
	engine->TransitionImageLayout(image.image, actual_format, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, aspect_mask);

	delete staging_buffer;

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
	size_t components = GetComponentsCount(format);
	size_t component_size = GetComponentSize(format);

	// TODO: this all won't work if the format isn't uint
	uint8_t *pixels = new uint8_t[components * component_size];
	if(component_size == 1)
	{
		for(size_t i=0; i<components; i++)
			pixels[i] = static_cast<unsigned char>(color[i] * 255.0f);
	}
	else if(component_size == 2)
	{
		for(size_t i=0; i<components; i++)
			*((uint16_t *)pixels + i) = static_cast<unsigned char>(color[i] * 255.0f);
	}
	else
	{
		// TODO: add support for more formats
		throw std::runtime_error("unsupported format for creating single color image.");
	}

	Image image = LoadFromPixelData(engine, format, 1, 1, pixels);

	delete[] pixels;

	return image;
}
