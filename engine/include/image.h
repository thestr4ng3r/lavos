
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

		Image()
			: image(nullptr), allocation(nullptr) {}

		Image(vk::Image image, VmaAllocation allocation)
			: image(image), allocation(allocation) {}


		static Image LoadFromFile(Engine *engine, std::string file);
};

}

#endif //VULKAN_IMAGE_H
