
#ifndef VULKAN_UNLIT_MATERIAL_H
#define VULKAN_UNLIT_MATERIAL_H

#include "material.h"

namespace engine
{

class UnlitMaterial: public Material
{
	private:
		void CreateDescriptorSetLayout();

		vk::ShaderModule vert_shader_module;
		vk::ShaderModule frag_shader_module;

		Texture texture_default_image;

	public:
		UnlitMaterial(Engine *engine);
		~UnlitMaterial();

		virtual std::vector<vk::DescriptorPoolSize> GetDescriptorPoolSizes() const override;
		virtual std::vector<vk::PipelineShaderStageCreateInfo> GetShaderStageCreateInfos() const override;

		Texture GetTextureDefaultImage() const override		{ return texture_default_image; }
};

}

#endif //VULKAN_UNLIT_MATERIAL_H
