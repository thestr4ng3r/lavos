
#ifndef VULKAN_RENDERER_H
#define VULKAN_RENDERER_H

#include "engine.h"
#include "material.h"

namespace engine
{

class Renderer
{
	private:
		Engine *engine;

		Material *material;

		vk::DescriptorPool descriptor_pool;

		void CreateDescriptorPool();
		void CreateMaterial();

	public:
		Renderer(Engine *engine);
		~Renderer();

		vk::DescriptorPool GetDescriptorPool() const 	{ return descriptor_pool; }
		Material *GetMaterial() const 					{ return material; }
};

}

#endif //VULKAN_RENDERER_H
