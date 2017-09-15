
#ifndef VULKAN_IMAGE_H
#define VULKAN_IMAGE_H

#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"

namespace engine
{

class Engine;

class Image
{
	public:
		vk::Image image;
		VmaAllocation allocation;

		Image(nullptr_t = nullptr)
			: image(nullptr), allocation(nullptr) {}

		Image(vk::Image image, VmaAllocation allocation)
			: image(image), allocation(allocation) {}


		static Image LoadFromPixelData(Engine *engine, vk::Format format, uint32_t width, uint32_t height, unsigned char *pixels);
		static Image LoadFromFile(Engine *engine, std::string file);
		static Image LoadFromMemory(Engine *engine, unsigned char *data, size_t size);

		bool operator==(Image const &rhs) const
		{
			return image == rhs.image
				   && allocation == rhs.allocation;
		}

		bool operator!=(Image const &rhs) const
		{
			return image != rhs.image
				   || allocation != rhs.allocation;
		}
};



}

#endif //VULKAN_IMAGE_H
