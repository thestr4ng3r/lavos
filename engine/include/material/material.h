
#ifndef VULKAN_MATERIAL_H
#define VULKAN_MATERIAL_H

#include <vulkan/vulkan.hpp>

namespace engine
{

class Engine;

class Material
{
	protected:
		Engine *engine;

		vk::DescriptorSetLayout descriptor_set_layout;

	public:
		Material(Engine *engine);
		virtual ~Material();

		Engine *GetEngine() const							 		{ return engine; }

		vk::DescriptorSetLayout GetDescriptorSetLayout() const		{ return descriptor_set_layout; }

		virtual std::vector<vk::DescriptorPoolSize> GetDescriptorPoolSizes() const =0;
		virtual std::vector<vk::PipelineShaderStageCreateInfo> GetShaderStageCreateInfos() const =0;

		static vk::ShaderModule CreateShaderModule(vk::Device device, const std::vector<char> &code);
};

}

#endif //VULKAN_MATERIAL_H
