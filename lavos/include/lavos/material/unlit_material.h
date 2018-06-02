
#ifndef LAVOS_UNLIT_MATERIAL_H
#define LAVOS_UNLIT_MATERIAL_H

#include "material.h"

namespace lavos
{

class UnlitMaterial: public Material
{
	private:
		struct UniformBuffer
		{
			glm::vec3 color_factor;
		};

		struct InstanceData
		{
			lavos::Buffer *uniform_buffer;
		};

		void CreateDescriptorSetLayout();

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

		virtual std::vector<vk::DescriptorPoolSize> GetDescriptorPoolSizes(RenderMode render_mode) const override;
		virtual std::vector<vk::PipelineShaderStageCreateInfo> GetShaderStageCreateInfos(RenderMode render_mode) const override;

		virtual void WriteDescriptorSet(RenderMode render_mode, vk::DescriptorSet descriptor_set, MaterialInstance *instance) override;

		virtual void *CreateInstanceData(RenderMode render_mode) override;
		virtual void DestroyInstanceData(RenderMode render_mode, void *data) override;
		virtual void UpdateInstanceData(RenderMode render_mode, void *data, MaterialInstance *instance) override;
};

}

#endif //VULKAN_UNLIT_MATERIAL_H
