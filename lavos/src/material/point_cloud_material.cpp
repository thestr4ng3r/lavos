
#include "lavos/engine.h"
#include "lavos/material/point_cloud_material.h"
#include "lavos/material/material_instance.h"

#include "lavos/shader_load.h"

using namespace lavos;

PointCloudMaterial::PointCloudMaterial(lavos::Engine *engine) : Material(engine)
{
	CreateDescriptorSetLayouts();

	vert_shader_module = CreateShaderModule(engine->GetVkDevice(), "material/point_cloud.vert");
	frag_shader_module = CreateShaderModule(engine->GetVkDevice(), "material/point_cloud.frag");
}

PointCloudMaterial::~PointCloudMaterial()
{
	auto &device = engine->GetVkDevice();

	device.destroyShaderModule(vert_shader_module);
	device.destroyShaderModule(frag_shader_module);
}

void PointCloudMaterial::CreateDescriptorSetLayouts()
{
}

std::vector<vk::PipelineShaderStageCreateInfo> PointCloudMaterial::GetShaderStageCreateInfos(RenderMode render_mode) const
{
	return {
			vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(),
											  vk::ShaderStageFlagBits::eVertex,
											  vert_shader_module,
											  "main"),
			vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(),
											  vk::ShaderStageFlagBits::eFragment,
											  frag_shader_module,
											  "main")
	};
}


void PointCloudMaterial::WriteDescriptorSet(DescriptorSetId id, vk::DescriptorSet descriptor_set, MaterialInstance *instance)
{
	return;
}

void *PointCloudMaterial::CreateInstanceData(InstanceDataId id)
{
	return nullptr;
}

void PointCloudMaterial::DestroyInstanceData(InstanceDataId id, void *data_p)
{
}

void PointCloudMaterial::UpdateInstanceData(InstanceDataId id, void *data_p, MaterialInstance *instance)
{
}