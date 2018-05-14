
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
};

}

#endif //LAVOS_POINT_CLOUD_MATERIAL_H
