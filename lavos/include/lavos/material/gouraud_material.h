
#ifndef LAVOS_GOURAUD_MATERIAL_H
#define LAVOS_GOURAUD_MATERIAL_H

#include "material.h"

namespace lavos
{

class GouraudMaterial: public Material
{
	private:
		struct alignas(sizeof(float)) UniformBuffer
		{
			glm::vec4 color_factor;
			float specular_exponent;
		};

		enum : Material::DescriptorSetId {
			DescriptorSetLayoutIdDefault
		};

		void CreateDescriptorSetLayouts();

		vk::ShaderModule vert_shader_module;
		vk::ShaderModule frag_shader_module;

		Texture texture_default_base_color;
		Texture texture_default_normal;

	public:
		static const Material::ParameterSlot parameter_slot_specular_exponent = 1000;

		GouraudMaterial(Engine *engine);
		~GouraudMaterial();

		bool GetRenderModeSupport(RenderMode render_mode) const override
		{
			return render_mode == DefaultRenderMode::ColorForward;
		}

		virtual std::vector<vk::PipelineShaderStageCreateInfo> GetShaderStageCreateInfos(Material::RenderMode render_mode) const override;

		void WriteDescriptorSet(DescriptorSetId id, vk::DescriptorSet descriptor_set, MaterialInstance *instance) override;

		void *CreateInstanceData(InstanceDataId id) override;
		void DestroyInstanceData(InstanceDataId id, void *data) override;
		void UpdateInstanceData(InstanceDataId id, void *data, MaterialInstance *instance) override;
};

}


#endif //LAVOS_GOURAUD_MATERIAL_H
