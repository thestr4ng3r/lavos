
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

		vk::Sampler texture_sampler;

		void CreateDescriptorSetLayout();
		void CreateSamplers();

	public:
		Material(Engine *engine);
		~Material();

		Engine *GetEngine() const							 		{ return engine; }

		vk::DescriptorSetLayout GetDescriptorSetLayout() const		{ return descriptor_set_layout; }
		vk::Sampler GetTextureSampler() const 						{ return texture_sampler; }
};

}

#endif //VULKAN_MATERIAL_H
