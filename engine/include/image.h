
#ifndef VULKAN_IMAGE_H
#define VULKAN_IMAGE_H

#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"

namespace engine
{

class Image
{
	public:
		vk::Image image;
		VmaAllocation allocation;

		Image()
			: image(nullptr), allocation(nullptr) {}

		Image(vk::Image image, VmaAllocation allocation)
			: image(image), allocation(allocation) {}
};

}

#endif //VULKAN_IMAGE_H
