
#ifndef LAVOS_POINT_CLOUD_MATERIAL_H
#define LAVOS_POINT_CLOUD_MATERIAL_H

#include "material.h"

namespace lavos
{

class PointCloudMaterial : public Material
{
	private:
		void CreateDescriptorSetLayouts();

		vk::ShaderModule vert_shader_module;
		vk::ShaderModule frag_shader_module;

	public:
		PointCloudMaterial(Engine *engine);
		~PointCloudMaterial();

		bool GetRenderModeSupport(RenderMode render_mode) const override
		{
			return render_mode == DefaultRenderMode::ColorForward;
		}

		std::vector<vk::PipelineShaderStageCreateInfo> GetShaderStageCreateInfos(RenderMode render_mode) const override;

		void WriteDescriptorSet(DescriptorSetId id, vk::DescriptorSet descriptor_set, MaterialInstance *instance) override;
		void *CreateInstanceData(InstanceDataId id) override;
		void DestroyInstanceData(InstanceDataId id, void *data) override;
		void UpdateInstanceData(InstanceDataId id, void *data, MaterialInstance *instance) override;

		vk::PrimitiveTopology GetPrimitiveTopology() override	{ return vk::PrimitiveTopology::ePointList; }

		std::vector<vk::VertexInputBindingDescription> GetVertexInputBindingDescriptions() override
		{
			return {
				vk::VertexInputBindingDescription()
						.setBinding(0)
						.setStride(sizeof(glm::vec3))
						.setInputRate(vk::VertexInputRate::eVertex)
			};
		}

		std::vector<vk::VertexInputAttributeDescription> GetVertexInputAttributeDescriptions() override
		{
			return {
				vk::VertexInputAttributeDescription()
						.setBinding(0)
						.setLocation(0)
						.setFormat(vk::Format::eR32G32B32Sfloat)
						.setOffset(0),
			};
		}
};

}

#endif //LAVOS_POINT_CLOUD_MATERIAL_H
