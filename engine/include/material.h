
#ifndef VULKAN_MATERIAL_H
#define VULKAN_MATERIAL_H

#include <vulkan/vulkan.hpp>

namespace engine
{

class Engine;

class Material
{
	private:
		Engine *engine;

		vk::DescriptorSetLayout descriptor_set_layout;

		void CreateDescriptorSetLayout();
		void CreateSamplers();

	public:
		Material(Engine *engine);
		~Material();

		Engine *GetEngine() const							 		{ return engine; }

		vk::DescriptorSetLayout GetDescriptorSetLayout() const		{ return descriptor_set_layout; }
};

}

#endif //VULKAN_MATERIAL_H
