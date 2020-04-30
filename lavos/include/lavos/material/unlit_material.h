
#ifndef LAVOS_UNLIT_MATERIAL_H
#define LAVOS_UNLIT_MATERIAL_H

#include "material.h"

#include "../glm_config.h"
#include <glm/ext/vector_float3.hpp>

namespace lavos
{

class UnlitMaterial: public Material
{
	private:
		struct UniformBuffer
		{
			glm::vec3 color_factor;
		};

		enum : Material::DescriptorSetId {
			DescriptorSetLayoutIdDefault
		};

		void CreateDescriptorSetLayouts();

		vk::ShaderModule vert_shader_module;
		vk::ShaderModule frag_shader_module;

		Texture texture_default_image;

	public:
		UnlitMaterial(Engine *engine);
		~UnlitMaterial();

		bool GetRenderModeSupport(RenderMode render_mode) const override
		{
			return render_mode == DefaultRenderMode::ColorForward;
		}

		std::vector<vk::PipelineShaderStageCreateInfo> GetShaderStageCreateInfos(RenderMode render_mode) const override;

		void WriteDescriptorSet(DescriptorSetId id, vk::DescriptorSet descriptor_set, MaterialInstance *instance) override;

		void *CreateInstanceData(InstanceDataId id) override;
		void DestroyInstanceData(InstanceDataId id, void *data) override;
		void UpdateInstanceData(InstanceDataId id, void *data, MaterialInstance *instance) override;
};

}

#endif //VULKAN_UNLIT_MATERIAL_H
