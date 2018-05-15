
#ifndef LAVOS_POINT_CLOUD_MATERIAL_H
#define LAVOS_POINT_CLOUD_MATERIAL_H

#include "material.h"

namespace lavos
{

class PointCloudMaterial : public Material
{
	private:
		void CreateDescriptorSetLayout();

		vk::ShaderModule vert_shader_module;
		vk::ShaderModule frag_shader_module;

	public:
		PointCloudMaterial(Engine *engine);
		~PointCloudMaterial();

		virtual std::vector<vk::DescriptorPoolSize> GetDescriptorPoolSizes() const override;

		virtual std::vector<vk::PipelineShaderStageCreateInfo> GetShaderStageCreateInfos() const override;

		virtual void WriteDescriptorSet(vk::DescriptorSet descriptor_set, MaterialInstance *instance) override;
		virtual void *CreateInstanceData() override;
		virtual void DestroyInstanceData(void *data) override;
		virtual void UpdateInstanceData(void *data, MaterialInstance *instance) override;

		virtual vk::PrimitiveTopology GetPrimitiveTopology()	{ return vk::PrimitiveTopology::ePointList; }

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
