
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

		Buffer(nullptr_t = nullptr)
			: buffer(nullptr), allocation(nullptr) {}

		Buffer(vk::Buffer buffer, VmaAllocation allocation)
			: buffer(buffer), allocation(allocation) {}


		bool operator==(Buffer const &rhs) const
		{
			return buffer == rhs.buffer
				   && allocation == rhs.allocation;
		}

		bool operator!=(Buffer const &rhs) const
		{
			return buffer != rhs.buffer
				   || allocation != rhs.allocation;
		}
};

}

#endif //VULKAN_BUFFER_H
