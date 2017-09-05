
#ifndef VULKAN_BUFFER_H
#define VULKAN_BUFFER_H

#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"

namespace engine
{

class Buffer
{
	public:
		vk::Buffer buffer;
		VmaAllocation allocation;

		Buffer()
			: buffer(nullptr), allocation(nullptr) {}

		Buffer(vk::Buffer buffer, VmaAllocation allocation)
			: buffer(buffer), allocation(allocation) {}
};

}

#endif //VULKAN_BUFFER_H
