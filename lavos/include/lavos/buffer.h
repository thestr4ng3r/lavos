
#ifndef LAVOS_BUFFER_H
#define LAVOS_BUFFER_H

#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"

namespace lavos
{

class Engine;

class Buffer
{
	private:
		Engine * const engine;

		vk::Buffer buffer;
		VmaAllocation allocation;
		void *map = nullptr;

	public:
		Buffer(Engine *engine, vk::Buffer buffer, VmaAllocation allocation)
			: engine(engine), buffer(buffer), allocation(allocation) {}

		~Buffer();

		vk::Buffer GetVkBuffer()	{ return buffer; }

		void *Map();
		void UnMap();
};

}

#endif //VULKAN_BUFFER_H
