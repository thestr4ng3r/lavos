
#ifndef LAVOS_PHONG_MATERIAL_H
#define LAVOS_PHONG_MATERIAL_H

#include "material.h"

namespace lavos
{

class PhongMaterial: public Material
{
	private:
		struct alignas(sizeof(float)) UniformBuffer
		{
			glm::vec4 color_factor;
			float specular_exponent;
		};

		enum : Material::DescriptorSetId {
			DescriptorSetLayoutIdDefault,
			DescriptorSetLayoutIdShadow
		};

		vk::ShaderModule vert_shader_module;
		vk::ShaderModule frag_shader_module;

		vk::ShaderModule shadow_vert_shader_module;
		vk::ShaderModule shadow_frag_shader_module;

		Texture texture_default_base_color;
		Texture texture_default_normal;

		void CreateDescriptorSetLayouts();

	public:
		static const Material::ParameterSlot parameter_slot_specular_exponent = 1000;

		PhongMaterial(Engine *engine);
		~PhongMaterial();

		bool GetRenderModeSupport(RenderMode render_mode) const override
		{
			return render_mode == DefaultRenderMode::ColorForward
					|| render_mode == DefaultRenderMode::Shadow;
		}

		virtual std::vector<vk::PipelineShaderStageCreateInfo> GetShaderStageCreateInfos(Material::RenderMode render_mode) const override;

		virtual void WriteDescriptorSet(DescriptorSetId id, vk::DescriptorSet descriptor_set, MaterialInstance *instance) override;

		virtual void *CreateInstanceData(InstanceDataId id) override;
		virtual void DestroyInstanceData(InstanceDataId id, void *data) override;
		virtual void UpdateInstanceData(InstanceDataId id, void *data, MaterialInstance *instance) override;
};

}


#endif //VULKAN_PHONG_MATERIAL_H
