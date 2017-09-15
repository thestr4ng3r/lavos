
#ifndef VULKAN_PHONG_MATERIAL_H
#define VULKAN_PHONG_MATERIAL_H

#include "material.h"

namespace engine
{

class PhongMaterial: public Material
{
	private:
		void CreateDescriptorSetLayout();

		vk::ShaderModule vert_shader_module;
		vk::ShaderModule frag_shader_module;

	public:
		PhongMaterial(Engine *engine);
		~PhongMaterial();

		virtual std::vector<vk::DescriptorPoolSize> GetDescriptorPoolSizes() const override;
		virtual std::vector<vk::PipelineShaderStageCreateInfo> GetShaderStageCreateInfos() const override;
};

}


#endif //VULKAN_PHONG_MATERIAL_H
