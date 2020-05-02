
#ifndef LAVOS_MATERIAL_PIPELINE_MANAGER_H
#define LAVOS_MATERIAL_PIPELINE_MANAGER_H

#include <vector>
#include <vulkan/vulkan.hpp>

#include "material/material.h"
#include "vk_util.h"

namespace lavos
{

class Engine;
class Renderer;

struct MaterialPipeline
{
	Material *material;

	vk::PipelineLayout pipeline_layout;
	vk::Pipeline pipeline;

	int renderer_descriptor_set_index;
	int material_descriptor_set_index;

	MaterialPipeline(Material *material) :
			material(material) {}
};

struct MaterialPipelineConfiguration
{
	vk::Extent2D extent;
	vk::SampleCountFlagBits samples;
	vk::DescriptorSetLayout renderer_descriptor_set_layout;
	vk::RenderPass render_pass;
	Material::RenderMode render_mode;
	vk_util::PipelineColorBlendStateCreateInfo color_blend_state_info;

	MaterialPipelineConfiguration(vk::Extent2D extent,
			vk::SampleCountFlagBits samples,
			vk::DescriptorSetLayout renderer_descriptor_set_layout,
			vk::RenderPass render_pass,
			Material::RenderMode render_mode,
			const vk_util::PipelineColorBlendStateCreateInfo &color_blend_state_info)
			: extent(extent),
			samples(samples),
			renderer_descriptor_set_layout(renderer_descriptor_set_layout),
			render_pass(render_pass),
			render_mode(render_mode),
			color_blend_state_info(color_blend_state_info) {}
};

static inline bool operator==(const MaterialPipelineConfiguration &a, const MaterialPipelineConfiguration &b)
{
	return a.extent == b.extent
		&& a.renderer_descriptor_set_layout == b.renderer_descriptor_set_layout
		&& a.render_pass == b.render_pass;
}

class MaterialPipelineManager
{
	private:
		Engine * const engine;

		MaterialPipelineConfiguration config;

		std::vector<MaterialPipeline> material_pipelines;

		MaterialPipeline CreateMaterialPipeline(Material *material, Material::RenderMode render_mode);
		void DestroyMaterialPipeline(const MaterialPipeline &material_pipeline);

		void DestroyAllMaterialPipelines();
		void RecreateAllMaterialPipelines();

	public:
		MaterialPipelineManager(Engine *engine, const MaterialPipelineConfiguration &config);
		~MaterialPipelineManager();

		void AddMaterial(Material *material);
		void RemoveMaterial(Material *material);

		void SetConfiguration(const MaterialPipelineConfiguration &config);

		MaterialPipeline *GetMaterialPipeline(Material *material);
};

}

#endif //LAVOS_MATERIAL_PIPELINE_MANAGER_H
