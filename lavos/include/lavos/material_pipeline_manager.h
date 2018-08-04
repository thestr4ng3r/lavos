
#ifndef LAVOS_MATERIAL_PIPELINE_MANAGER_H
#define LAVOS_MATERIAL_PIPELINE_MANAGER_H

#include <vector>
#include <vulkan/vulkan.hpp>

#include "material/material.h"

namespace lavos
{

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

class MaterialPipelineManager
{
	private:
		Renderer * const renderer;
		std::vector<MaterialPipeline> material_pipelines;

		MaterialPipeline CreateMaterialPipeline(Material *material, Material::RenderMode render_mode);
		void DestroyMaterialPipeline(const MaterialPipeline &material_pipeline);

	public:
		explicit MaterialPipelineManager(Renderer *renderer);
		~MaterialPipelineManager();

		void DestroyAllMaterialPipelines();

		void AddMaterial(Material *material);
		void RemoveMaterial(Material *material);

		void RecreateAllMaterialPipelines();

		MaterialPipeline *GetMaterialPipeline(Material *material);
};

}

#endif //LAVOS_MATERIAL_PIPELINE_MANAGER_H
