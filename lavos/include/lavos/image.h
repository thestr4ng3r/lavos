
#ifndef LAVOS_IMAGE_H
#define LAVOS_IMAGE_H

#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"

#include "glm_config.h"

namespace lavos
{

class Engine;

class Image
{
	public:
		vk::Image image;
		VmaAllocation allocation;
		vk::Format format;

		Image(std::nullptr_t = nullptr)
			: image(nullptr), allocation(nullptr), format(vk::Format::eUndefined) {}

		Image(vk::Image image, VmaAllocation allocation, vk::Format format)
			: image(image), allocation(allocation), format(format) {}


		static Image LoadFromPixelData(Engine *engine, vk::Format format, uint32_t width, uint32_t height, unsigned char *pixels);
		static Image LoadFromFile(Engine *engine, std::string file);
		static Image LoadFromMemory(Engine *engine, unsigned char *data, size_t size);
		static Image CreateColor(Engine *engine, vk::Format format, glm::vec4 color);

		bool operator==(Image const &rhs) const
		{
			return image == rhs.image
				   && allocation == rhs.allocation
				   && format == rhs.format;
		}

		bool operator!=(Image const &rhs) const
		{
			return image != rhs.image
				   || allocation != rhs.allocation
				   || format != rhs.format;
		}
};



}

#endif //VULKAN_IMAGE_H
