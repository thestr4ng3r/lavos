
#ifndef VULKAN_PHONG_MATERIAL_H
#define VULKAN_PHONG_MATERIAL_H

#include "material.h"

namespace engine
{

class PhongMaterial: public Material
{
	private:
		struct UniformBuffer
		{
			glm::vec3 color_factor;
		};

		struct InstanceData
		{
			engine::Buffer uniform_buffer;
		};

		void CreateDescriptorSetLayout();

		vk::ShaderModule vert_shader_module;
		vk::ShaderModule frag_shader_module;

		Texture texture_default_image;

	public:
		PhongMaterial(Engine *engine);
		~PhongMaterial();

		virtual std::vector<vk::DescriptorPoolSize> GetDescriptorPoolSizes() const override;
		virtual std::vector<vk::PipelineShaderStageCreateInfo> GetShaderStageCreateInfos() const override;

		virtual void WriteDescriptorSet(vk::DescriptorSet descriptor_set, MaterialInstance *instance) override;

		virtual void *CreateInstanceData() override;
		virtual void DestroyInstanceData(void *data) override;
		virtual void UpdateInstanceData(void *data, MaterialInstance *instance) override;
};

}


#endif //VULKAN_PHONG_MATERIAL_H
