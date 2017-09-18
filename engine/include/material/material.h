
#ifndef VULKAN_MATERIAL_H
#define VULKAN_MATERIAL_H

#include <vulkan/vulkan.hpp>

#include "texture.h"
#include "material_instance.h"

namespace engine
{

class Engine;

class Material
{
	protected:
		Engine *engine;

		vk::DescriptorSetLayout descriptor_set_layout;

	public:
		static const MaterialInstance::TextureSlot texture_slot_base_color = 0;

		static const MaterialInstance::ParameterSlot parameter_slot_base_color_factor = 0;


		Material(Engine *engine);
		virtual ~Material();

		Engine *GetEngine() const							 		{ return engine; }

		vk::DescriptorSetLayout GetDescriptorSetLayout() const		{ return descriptor_set_layout; }

		virtual std::vector<vk::DescriptorPoolSize> GetDescriptorPoolSizes() const =0;
		virtual std::vector<vk::PipelineShaderStageCreateInfo> GetShaderStageCreateInfos() const =0;

		virtual void WriteDescriptorSet(vk::DescriptorSet descriptor_set, MaterialInstance *instance) =0;

		virtual engine::Buffer CreateUniformBuffer() 				{ return nullptr; }
		virtual void WriteUniformBuffer(engine::Buffer buffer, MaterialInstance *instance) {}

		static vk::ShaderModule CreateShaderModule(vk::Device device, const std::vector<char> &code);

		virtual Texture GetTextureDefaultImage() const 		{ return nullptr; };
};

}

#endif //VULKAN_MATERIAL_H
